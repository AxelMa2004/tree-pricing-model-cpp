#pragma once

#include <string>
#include <sstream>
#include <iostream>

namespace Exceptions{
    class TreePricingException{
    protected:
        std::string _name;

    public:
        TreePricingException(const std::string& name);
        virtual ~TreePricingException() = default;

        virtual void handle() const = 0;

    };


    class ArbitrageException : public TreePricingException{
    public:
        ArbitrageException(const std::string& name);
        virtual ~ArbitrageException();
        void handle() const;
    };

    class NotcompleteException : public TreePricingException{
    public:
        NotcompleteException(const std::string& name);
        virtual ~NotcompleteException();
        void handle() const;
    };

    class NegativeSDFException : public ArbitrageException{
    public:
        NegativeSDFException(const std::string& name);
        virtual ~NegativeSDFException();

        void handle() const;
    };


    class NosulutionException : public ArbitrageException{
    public:
        NosulutionException(const std::string& name);
        virtual ~NosulutionException();

        void handle() const;
    };

}