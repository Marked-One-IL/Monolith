#include <Parser/AST/Stmt/Assign.hpp>

Parser::AST::Stmt::Assign::Assign(std::unique_ptr<const Parser::AST::Expr::Base> new_dst,
                                  std::unique_ptr<const Parser::AST::Expr::Base> new_src,
                                  std::string_view new_operation,
                                  const Lexer::LocationData& new_locationData) :
    Parser::AST::Stmt::Base(Parser::AST::Tag::STMT_ASSIGN, new_locationData), dst(std::move(new_dst)), src(std::move(new_src)), operation(new_operation)
{
}
