#include <Parser/AST/Expr/Identifier.hpp>

Parser::AST::Expr::Identifier::Identifier(std::string_view new_name, const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_IDENTIFIER, new_locationData), name(new_name)
{
}
