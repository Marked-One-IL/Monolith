#pragma once
#include <Parser/AST/Base.hpp>
#include <Parser/AST/Type/Base.hpp>

namespace Parser::AST::Expr
{
    struct Base : public Parser::AST::Base
    {
        using Parser::AST::Base::Base;
        const Parser::AST::Type::Base* getType(void) const;
        bool isUnaryValid(std::string_view opr, const Parser::AST::Type::Base* type) const;
        bool isDeepIdentifier(void) const;
    };
}
