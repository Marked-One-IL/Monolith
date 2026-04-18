#pragma once
#include <Parser/AST/Base.hpp>
#include <memory>

namespace Parser::AST::Stmt
{
    struct Base : public Parser::AST::Base
    {
        using Parser::AST::Base::Base;
    };
}
