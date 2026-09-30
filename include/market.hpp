#pragma once

#include "node.hpp"
#include <string>
#include <ostream>
#include <sstream>

namespace TreeAsset
{
    enum class MarketState{
        COMPLETE,
        NOTCOMPLETE,
        ARBITRAGE,
    };

    static std::unordered_map<MarketState,std::string> StateDescription = {
        {MarketState::COMPLETE, "Complete"},
        {MarketState::NOTCOMPLETE, "Not complete but no arbitrage"},
        {MarketState::ARBITRAGE, "Arbitrage exists"}
    };

    class Market
    {
    private:
        MarketState _state;
        int _asset_num;
        int _step;
        std::vector<std::vector<std::shared_ptr<Node>>> _tree;

        // The contributions that is used for constructing the artificial probability
        std::unordered_map<const Node*, double> _arti_contributions;

        std::string _description() const;
        void _check_prob() const;
        void _cal_sdf_layer(int layer);
    public:
        Market(int asset_num, int step);
        virtual ~Market() = default;


        void add_node(int step_num, const std::shared_ptr<Node>& node);
        std::shared_ptr<Node> add_node(int step_num, const std::string& name,
                                       std::vector<double> prices, double prob = 0.0);
        void connect(const std::shared_ptr<Node>& parent,
                     const std::shared_ptr<Node>& child);
        void cal_sdf();

        std::tuple<double, std::unordered_map<const Node*, double>> 
                price_iterated(std::vector<double> final_payoffs) const;

        friend std::ostream& operator << (std::ostream& oss, const Market& market);

        void assign_artprob(std::vector<double> final_payoffs);
        void assign_artprob(int asset_num);

        double pricing_artprob(std::vector<double> final_payoffs) const;
    };
    
} 
