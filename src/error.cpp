#include "error.hpp"


namespace Exceptions{
    TreePricingException::TreePricingException(const std::string& name) : _name(name) {}
    ArbitrageException::ArbitrageException(const std::string& name) : TreePricingException(name) {}
    NotcompleteException::NotcompleteException(const std::string& name) : TreePricingException(name) {}

    NegativeSDFException::NegativeSDFException(const std::string& name) : ArbitrageException(name) {}
    NosulutionException::NosulutionException(const std::string& name) : ArbitrageException(name) {}


    ArbitrageException::~ArbitrageException() {}
    NotcompleteException::~NotcompleteException() {}

    NegativeSDFException::~NegativeSDFException() {}
    NosulutionException::~NosulutionException() {}

    void ArbitrageException::handle() const{
        std::stringstream oss;
        oss << "[warning] the market has arbitrage, the SDF of node ";
        oss << _name << " has no SDF solution\n";
        std::cout << oss.str();
    }

    void NotcompleteException::handle() const{
        std::stringstream oss;
        oss << "[warning] LOP holds, no arbitrage, but incomplete. The SDF of node ";
        oss << _name << " has infinitely many soltions\n";
        std::cout << oss.str();
    }

    void NegativeSDFException::handle() const{
        std::stringstream oss;
        oss << "[warning] LOP holds, but arbitrage exists, the SDF of node ";
        oss << _name << " has no positive SDF solution\n";
        std::cout << oss.str();
    }

    void NosulutionException::handle() const{
        std::stringstream oss;
        oss << "[warning] LOP violation, the SDF of node ";
        oss << _name << " has no SDF solution\n";
        std::cout << oss.str();
    }
}