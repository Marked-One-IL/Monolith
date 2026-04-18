#include <Parser/AST/Expr/Literal.hpp>

Parser::AST::Expr::Literal::Literal(Lexer::Token new_literal, const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_LITERAL, new_locationData), literal(new_literal)
{
}
