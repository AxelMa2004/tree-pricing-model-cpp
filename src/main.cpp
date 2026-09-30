#include "market.hpp"
#include <iostream>


void in_class_exp(){
    using TreeAsset::Market;

    Market market(2, 4); // Two assets, time layers t = 0, 1, 2, 3.

    // Terminal nodes carry unconditional probabilities.
    auto A = market.add_node(3, "A", {975, 1202.04}, 0.125);
    auto B = market.add_node(3, "B", {600, 1202.04}, 0.25);
    auto C = market.add_node(3, "C", {624, 1179.78}, 0.125);
    auto D = market.add_node(3, "D", {384, 1179.78}, 0.25);
    auto E = market.add_node(3, "E", {650, 1168.44}, 1.0/24.0);
    auto F = market.add_node(3, "F", {400, 1168.44}, 1.0/12.0);
    auto G = market.add_node(3, "G", {416, 1145.97}, 1.0/24.0);
    auto H = market.add_node(3, "H", {256, 1145.97}, 1.0/12.0);

    // Complete each subtree before attaching it to its parent.
    auto AB = market.add_node(2, "AB", {750, 1113});
    market.connect(AB, A);
    market.connect(AB, B);

    auto CD = market.add_node(2, "CD", {480, 1113});
    market.connect(CD, C);
    market.connect(CD, D);

    auto EF = market.add_node(2, "EF", {500, 1123.5});
    market.connect(EF, E);
    market.connect(EF, F);

    auto GH = market.add_node(2, "GH", {320, 1123.5});
    market.connect(GH, G);
    market.connect(GH, H);

    auto ABCD = market.add_node(1, "ABCD", {600, 1050});
    market.connect(ABCD, AB);
    market.connect(ABCD, CD);

    auto EFGH = market.add_node(1, "EFGH", {400, 1050});
    market.connect(EFGH, EF);
    market.connect(EFGH, GH);

    auto root = market.add_node(0, "ABCDEFGH", {500, 1000});
    market.connect(root, ABCD);
    market.connect(root, EFGH);

    market.cal_sdf();
    market.assign_artprob({1202.04,1202.04,1179.78,1179.78,1168.44,1168.44,1145.97,1145.97});
    std::cout << market;

    auto [price, contributions] = market.price_iterated({975, 600, 624, 384, 650, 400, 416, 256});
    // auto [price, contributions] = market.price_iterated({1000,1000,1000,1000,1000,1000,1000,1000});
    std::cout << price << std::endl;

    std::cout << "Using formula: " << market.pricing_artprob({975, 600, 624, 384, 650, 400, 416, 256}) << std::endl;
    
    std::cout << "Using formula for bond with no coupon: " << 
    market.pricing_artprob(std::vector<double>(8, 1000)) << std::endl;


}


void three_assets(){
    using TreeAsset::Market;
    Market market(3, 3); // Assets {X, Y, Z}; time layers t = 0, 1, 2.
    constexpr double terminal_prob = 1.0 / 9.0;

    auto A = market.add_node(2, "A", {1170, 14040, 7020}, terminal_prob);
    auto B = market.add_node(2, "B", {1170, 12285, 5265}, terminal_prob);
    auto C = market.add_node(2, "C", {1170, 10530, 3510}, terminal_prob);
    auto D = market.add_node(2, "D", {1170, 8775, 1755}, terminal_prob);
    auto E = market.add_node(2, "E", {1170, 7020, 0}, terminal_prob);
    auto F = market.add_node(2, "F", {1170, 5265, 1755}, terminal_prob);
    auto G = market.add_node(2, "G", {1170, 3510, 3510}, terminal_prob);
    auto H = market.add_node(2, "H", {1170, 1755, 5265}, terminal_prob);
    auto I = market.add_node(2, "I", {1170, 0, 7020}, terminal_prob);

    // Complete each subtree before attaching it to the root.
    auto ABC = market.add_node(1, "ABC", {930, 10350, 4770});
    market.connect(ABC, A);
    market.connect(ABC, B);
    market.connect(ABC, C);

    auto DEF = market.add_node(1, "DEF", {1040, 6760, 1040});
    market.connect(DEF, D);
    market.connect(DEF, E);
    market.connect(DEF, F);

    auto GHI = market.add_node(1, "GHI", {936, 2574, 3042});
    market.connect(GHI, G);
    market.connect(GHI, H);
    market.connect(GHI, I);

    auto root = market.add_node(0, "ABCDEFGHI", {871, 6942, 2886});
    market.connect(root, ABC);
    market.connect(root, DEF);
    market.connect(root, GHI);

    market.cal_sdf();
    std::cout << market;
}

void two_assets(){
    using TreeAsset::Market;
    Market market(2, 3); // Assets {X, Y}; time layers t = 0, 1, 2.
    constexpr double terminal_prob = 1.0 / 4.0;

    // Terminal states have unconditional probability 0.25 each.
    auto A = market.add_node(2, "A", {13230.00, 24261.12}, terminal_prob);
    auto B = market.add_node(2, "B", {13230.00, 16174.08}, terminal_prob);
    auto C = market.add_node(2, "C", {13230.00, 15874.56}, terminal_prob);
    auto D = market.add_node(2, "D", {13230.00, 10583.04}, terminal_prob);

    auto AB = market.add_node(1, "AB", {12600.00, 18720.00});
    market.connect(AB, A);
    market.connect(AB, B);

    auto CD = market.add_node(1, "CD", {12600.00, 12480.00});
    market.connect(CD, C);
    market.connect(CD, D);

    auto ABCD = market.add_node(0, "ABCD", {12000.00, 15000.00});
    market.connect(ABCD, AB);
    market.connect(ABCD, CD);

    market.cal_sdf();
    market.assign_artprob({1.0,1.0,1.0,1.0});
    std::cout << market;
}

int main(){
    // in_class_exp();
    two_assets();
    return 0;
}
