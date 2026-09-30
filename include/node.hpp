#pragma once

#include "error.hpp"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <Eigen/Dense>

using namespace Eigen;

namespace TreeAsset{
    class Node{
        friend class Market;
    private:
        std::string _state_name;
        double _prob;
        double _art_prob;
        double _sdf;
        int _num_children;

        ArrayXd _prices;
        std::vector<std::shared_ptr<Node>> _children;
    public:
        // Branch nodes start with zero probability; terminal nodes supply it.
        Node(const std::string& state_name, std::vector<double> prices);
        Node(const std::string& state_name, const ArrayXd& prices);
        Node(const std::string& state_name, double prob, std::vector<double> prices);
        Node(const std::string& state_name, double prob, const ArrayXd& prices);

        virtual ~Node() = default;

        inline ArrayXd price() const { return _prices; }
        inline int num_children() const { return _num_children; }
        inline double art_prob() const { return _art_prob; }
        inline void art_prob(double val) { _art_prob = val; }
        // Attach completed subtrees bottom-up, accumulating their probabilities.
        void assign(const std::shared_ptr<Node>& child);
        
        // Unchecked cumulative SDF solve; call top-down on non-leaf nodes only.
        // Requires valid children, matching finite prices, positive probabilities,
        // and a consistent system with full column rank.
        void cal_children_sdf();

        double pricing(std::vector<double> children_payoff) const;
    };
}
