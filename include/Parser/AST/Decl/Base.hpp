#pragma once
#include <Parser/AST/Base.hpp>
#include <Parser/AST/Type/Base.hpp>
#include <ostream>

namespace Parser::AST::Decl
{
    struct Base : public Parser::AST::Base
    {
        using Parser::AST::Base::Base;
        const Parser::AST::Type::Base* getType(void) const;
    };
}
