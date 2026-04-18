#pragma once
#include <Parser/AST/Stmt/Base.hpp>
#include <memory>

namespace Parser::AST::Stmt
{
    struct Continue : public Parser::AST::Stmt::Base
    {
        Continue(const Lexer::LocationData& new_locationData);
    };
}
