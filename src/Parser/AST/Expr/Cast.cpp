#include <Parser/AST/Expr/Cast.hpp>

Parser::AST::Expr::Cast::Cast(std::unique_ptr<const Parser::AST::Expr::Base> new_expr,
                              std::unique_ptr<const Parser::AST::Type::Base> new_as,
                              const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_CAST, new_locationData), expr(std::move(new_expr)), as(std::move(new_as))
{
}
