#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <Lexer/Token.hpp>
#include <memory>

namespace Parser::AST::Expr
{
    struct Literal : public Parser::AST::Expr::Base
    {
        Literal(Lexer::Token new_literal, const Lexer::LocationData& new_locationData);
        const Lexer::Token literal;
    };
}
