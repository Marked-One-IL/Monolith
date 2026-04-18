#pragma once
#include <ostream>
#include <Helper/Types.hpp>

namespace Parser::AST::Type
{
    enum class Tag : uint8_t
    {
        IDENTIFIER,
        PRIMITIVE,
        TYPEDEF,
        PTR,
        ARR,
        FUNC,
        STRUCT
    };

    struct Base
    {
        Base(Parser::AST::Type::Tag new_tag);

        void print(std::ostream& stream, Uint depth) const;
        static bool compare(const Parser::AST::Type::Base* left, const Parser::AST::Type::Base* right);
        bool isBinaryValid(std::string_view opr) const;
        bool isUnaryValid(std::string_view opr) const;
        static const Parser::AST::Type::Base* getUnderlyingType(const Parser::AST::Type::Base* origin);
        static bool isCastable(const Parser::AST::Type::Base* left, const Parser::AST::Type::Base* right);

        const Parser::AST::Type::Tag tag;
    };
}
