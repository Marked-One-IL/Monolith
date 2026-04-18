#include <Parser/AST/Expr/Unary.hpp>

Parser::AST::Expr::Unary::Unary(std::string_view new_operation,
                                std::unique_ptr<const Parser::AST::Expr::Base> new_right,
                                const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_UNARY, new_locationData), operation(new_operation), right(std::move(new_right))
{
}
