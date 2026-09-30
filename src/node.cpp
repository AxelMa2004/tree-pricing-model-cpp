#include "node.hpp"
#include <stdexcept>
#include <utility>
#include <cmath>

namespace TreeAsset{
    Node::Node(const std::string& state_name, std::vector<double> prices) :
        Node(state_name, 0.0, std::move(prices)) {}

    Node::Node(const std::string& state_name, const ArrayXd& prices) :
        Node(state_name, 0.0, prices) {}

    Node::Node(const std::string& state_name, double prob, std::vector<double> prices) :
        _state_name(state_name), _prob(prob), _art_prob(0.0),
        _sdf(1.0), _num_children(0), _prices(prices.size()){
        for (Eigen::Index i = 0; i < _prices.size(); ++i){
            _prices[i] = prices[i];
        }
    }

    Node::Node(const std::string& state_name, double prob, const ArrayXd& prices) :
        _state_name(state_name), _prob(prob), _art_prob(0.0),
        _sdf(1.0), _num_children(0), _prices(prices) {}

        
    void Node::assign(const std::shared_ptr<Node>& child){
        if (!child)
            throw std::invalid_argument("Cannot assign a null child");
        _children.push_back(child);
        _prob += child->_prob;
        _num_children++;
    }

    void Node::cal_children_sdf(){
        const auto child_count = static_cast<Eigen::Index>(_children.size());
        const double inv_prob = 1.0 / _prob;
        MatrixXd weighted_prices(_prices.size(), child_count);
        for (Eigen::Index i = 0; i < child_count; ++i){
            const auto& child = _children[i];
            weighted_prices.col(i) = (child->_prob * inv_prob) * child->_prices.matrix();
        }

        const VectorXd rhs = _sdf * _prices.matrix();

        if (weighted_prices.rows() == weighted_prices.cols()){
            const auto lu = weighted_prices.fullPivLu();
            const auto rank_A = lu.rank();
            MatrixXd augmented(weighted_prices.rows(),
                            weighted_prices.cols() + 1);
            augmented << weighted_prices, rhs;

            const auto rank_augmented = augmented.fullPivLu().rank();

            if (rank_A == weighted_prices.cols()) {
                const VectorXd sdf = lu.solve(rhs);
                constexpr double tolerance = 1e-12;

                // Here we need to justify whether all the sdfs are strictly positive!
                const bool strictly_positive =
                    sdf.array().isFinite().all() &&
                    (sdf.array() > tolerance).all();
                
                if (!strictly_positive) {
                    throw Exceptions::NegativeSDFException(_state_name);
                }

                for (Eigen::Index i = 0; i < child_count; ++i)
                    _children[i]->_sdf = sdf[i];
            } else if (rank_A == rank_augmented) {
                // QR returns one valid SDF among the infinitely many choices.
                const VectorXd sdf = weighted_prices.colPivHouseholderQr().solve(rhs);
                for (Eigen::Index i = 0; i < child_count; ++i)
                    _children[i]->_sdf = sdf[i];

                throw Exceptions::NotcompleteException(_state_name);
            } else {
                throw Exceptions::NosulutionException(_state_name);
            }
        }
        else{
            const VectorXd sdf = weighted_prices.colPivHouseholderQr().solve(
                _sdf * _prices.matrix());

            for (Eigen::Index i = 0; i < child_count; ++i)
                _children[i]->_sdf = sdf[i];
        }
    }

    double Node::pricing(std::vector<double> children_payoff) const{
        if (children_payoff.size() != _num_children){
            throw std::invalid_argument("The size of children payoff cannot match the number of children.");
        }

        double price{0.0};

        for (int i=0; i<_num_children; ++i){
            price += children_payoff[i] * _children[i]->_sdf * _children[i] -> _prob;
        }
        return price/(_prob * _sdf);
    }
 
}
