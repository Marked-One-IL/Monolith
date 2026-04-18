#include <Parser/AST/Expr/Call.hpp>

Parser::AST::Expr::Call::Call(std::unique_ptr<const Parser::AST::Expr::Base> new_func,
                              std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> new_params,
                              const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_CALL, new_locationData), func(std::move(new_func)), params(std::move(new_params))
{
}
