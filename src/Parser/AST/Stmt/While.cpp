#include <Parser/AST/Stmt/While.hpp>

Parser::AST::Stmt::While::While(std::unique_ptr<const Parser::AST::Expr::Base> new_cond,
                                std::unique_ptr<const Parser::AST::Block> new_then,
                                const Lexer::LocationData& new_locationData) :
    Parser::AST::Stmt::Base(Parser::AST::Tag::STMT_WHILE, new_locationData), cond(std::move(new_cond)), then(std::move(new_then))
{
}
