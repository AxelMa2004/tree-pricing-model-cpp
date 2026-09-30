#include "market.hpp"
#include <iomanip>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <utility>

namespace TreeAsset{
    Market::Market(int asset_num, int step) : 
        _state(MarketState::COMPLETE), _asset_num(asset_num), _step(step) {
            if (asset_num < 0 || step < 0)
                throw std::invalid_argument("Asset and step counts must be nonnegative");
            _tree.resize(step);
    }


    void Market::add_node(int step_num, const std::shared_ptr<Node>& node){
        if (step_num < 0 || step_num >= _step)
            throw std::out_of_range("Step index is outside the market");
        if (!node) throw std::invalid_argument("Cannot add a null node");
        _tree[step_num].push_back(node);
    }

    std::shared_ptr<Node> Market::add_node(int step_num, const std::string& name,
                                          std::vector<double> prices, double prob){
        auto node = std::make_shared<Node>(name, prob, std::move(prices));
        add_node(step_num, node);
        return node;
    }

    void Market::connect(const std::shared_ptr<Node>& parent,
                         const std::shared_ptr<Node>& child){
        if (!parent) throw std::invalid_argument("Cannot connect a null parent");
        parent->assign(child);
    }

    void Market::_cal_sdf_layer(int layer){
        for (const auto& node : _tree[layer])
            if (!node->_children.empty()) {
                try{
                    node->cal_children_sdf();
                }
                catch (Exceptions::NotcompleteException& e){
                    e.handle();
                    _state = MarketState::NOTCOMPLETE;
                }
                catch (Exceptions::NosulutionException& e){
                    e.handle();
                    _state = MarketState::ARBITRAGE;
                }
                catch (Exceptions::NegativeSDFException& e){
                    e.handle();
                    MarketState::ARBITRAGE;
                }
                catch (...){
                    throw std::runtime_error("unhandling error");
                }
            }
    }


    void Market::cal_sdf(){
        for (int layer=0; layer<_step-1; ++layer){
            _cal_sdf_layer(layer);
        }
    }

    std::tuple<double, std::unordered_map<const Node*, double>> 
        Market::price_iterated(std::vector<double> final_payoffs) const{
        if (_tree.empty() || _tree.front().size() != 1 || _tree.back().empty())
            throw std::invalid_argument("Pricing requires a nonempty, single-root tree");
        if (final_payoffs.size() != _tree.back().size())
            throw std::invalid_argument("Provide one payoff per terminal node");

        // Carry p_n M_n V_n instead of V_n, avoiding division by intermediate SDFs.
        std::unordered_map<const Node*, double> contributions;

        // Initialize the terminal time step of the map
        for (size_t i=0; i<final_payoffs.size(); ++i){
            const auto& node = _tree.back()[i];
            contributions.emplace(node.get(), node->_prob*node->_sdf*final_payoffs[i]);
        }

        for (int layer=_step-2; layer>=0; --layer){
            for (const auto& node : _tree[layer]){
                double total{0.0};
                for (const auto& child : node->_children){
                    total += contributions.at(child.get());
                }
                contributions.emplace(node.get(), total);
            }
        }

        const auto& root = _tree.front().front();
        double final_price = contributions.at(root.get()) / (root->_prob * root->_sdf);
        return std::make_tuple(final_price, contributions);
    }

    std::string Market::_description() const {
        std::ostringstream out;
        out << std::setprecision(6);
        out << "Market | assets=" << _asset_num << " | layers=" << _step << '\n';
        out << "Market state: " << StateDescription.at(_state) << '\n';

        // Find roots from actual edges, independent of insertion order.
        std::unordered_set<const Node*> children;
        for (const auto& layer : _tree)
            for (const auto& node : layer)
                for (const auto& child : node->_children)
                    children.insert(child.get());

        const auto print_tree = [&](int asset) {
            std::unordered_set<const Node*> printed;
            const auto visit = [&](const auto& self, const Node& node,
                                   const std::string& prefix, bool last,
                                   bool root, int depth) -> void {
                out << prefix << (root ? "" : (last ? "└── " : "├── "))
                    << node._state_name << " [t=" << depth
                    << ", artificial probability=" << node._art_prob << "] = ";
                if (asset < 0)
                    out << node._sdf;
                else if (asset < node._prices.size())
                    out << node._prices[asset];
                else
                    out << "N/A";

                if (!printed.insert(&node).second) {
                    out << " (shared node; shown above)\n";
                    return;
                }
                out << '\n';
                const std::string next = prefix + (root ? "" : (last ? "    " : "│   "));
                for (std::size_t i = 0; i < node._children.size(); ++i)
                    self(self, *node._children[i], next,
                         i + 1 == node._children.size(), false, depth + 1);
            };

            for (int t = 0; t < _step; ++t)
                for (const auto& node : _tree[t])
                    if (!children.count(node.get()) && !printed.count(node.get()))
                        visit(visit, *node, "", true, true, t);

            // Include any remaining registered nodes, even in cyclic input.
            for (int t = 0; t < _step; ++t)
                for (const auto& node : _tree[t])
                    if (!printed.count(node.get()))
                        visit(visit, *node, "", true, true, t);

            if (printed.empty()) out << "(empty)\n";
        };

        for (int asset = 0; asset < _asset_num; ++asset) {
            out << "\nAsset " << asset << " price tree\n";
            print_tree(asset);
        }
        out << "\nCumulative SDF tree\n";
        print_tree(-1);
        return out.str();
    }

    std::ostream& operator << (std::ostream& oss, const Market& market){
        return oss << market._description();
    }



    void Market::assign_artprob(std::vector<double> final_payoffs){
        auto [init_price, contributions] = price_iterated(final_payoffs);
        _arti_contributions = std::move(contributions);

        double devident = init_price * _tree[0][0]->_sdf;
        for (auto& node_vec : _tree){
            for (auto& node : node_vec){
                node -> art_prob(_arti_contributions.at(node.get()) / devident);
            }
        }
    }


    double Market::pricing_artprob(std::vector<double> final_payoffs) const{
        if (_tree.empty() || _tree.front().size() != 1 || _tree.back().empty())
            throw std::invalid_argument("Pricing requires a nonempty, single-root tree");
        if (final_payoffs.size() != _tree.back().size())
            throw std::invalid_argument("Provide one payoff per terminal node");
        if (_arti_contributions.empty()) {
            throw std::invalid_argument(
                "Artificial probabilities are not initialized. "
                "Call assign_artprob(reference_payoffs) first.");
        }
        std::unordered_map<const Node*, double> containers;  // Storing Y/Z

        // Assign the last nodes
        int j = 0;
        for (const auto& node : _tree.back()){
            double z = _arti_contributions.at(node.get()) / (node->_prob*node->_sdf);
            containers.emplace(node.get(), final_payoffs[j]/z);
            j++;
        }

        for (int layer=_step-2; layer>=0; --layer){
            for (const auto& node : _tree[layer]){
                // double z = _arti_contributions.at(node.get()) / (node->_prob*node->_sdf);
                // std::cout << z << std::endl;
                double Y_Z{0.0};
                for (const auto& child : node->_children){
                    // The children have already been evaluated; the parent has
                    // not.  Use the child's artificial probability and value.
                    Y_Z += child->art_prob() * containers.at(child.get());
                }
                containers.emplace(node.get(), Y_Z / node->art_prob());
            }
        }


        const auto& root = _tree.front().front();

        return containers.at(root.get()) * _arti_contributions.at(root.get()) / (root->_prob*root->_sdf);
    }



}
