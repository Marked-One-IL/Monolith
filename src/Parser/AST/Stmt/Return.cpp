#include <Parser/AST/Stmt/Return.hpp>

Parser::AST::Stmt::Return::Return(std::unique_ptr<const Parser::AST::Expr::Base> new_expr, const Lexer::LocationData& new_locationData) :
    Parser::AST::Stmt::Base(Parser::AST::Tag::STMT_RETURN, new_locationData), expr(std::move(new_expr))
{
}
