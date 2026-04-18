#pragma once
#include <Parser/AST/Stmt/Base.hpp>
#include <memory>

namespace Parser::AST::Stmt
{
    struct Break : public Parser::AST::Stmt::Base
    {
        Break(const Lexer::LocationData& new_locationData);
    };
}
