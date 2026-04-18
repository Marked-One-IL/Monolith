#include <Parser/AST/Stmt/For.hpp>

Parser::AST::Stmt::For::For(std::unique_ptr<const Parser::AST::Stmt::Assign> new_init,
                            std::unique_ptr<const Parser::AST::Expr::Base> new_cond,
                            std::unique_ptr<const Parser::AST::Stmt::Assign> new_iter,
                            std::unique_ptr<const Parser::AST::Block> new_then,
                            const Lexer::LocationData& new_locationData) :
    Parser::AST::Stmt::Base(Parser::AST::Tag::STMT_FOR, new_locationData),
    init(std::move(new_init)), cond(std::move(new_cond)), iter(std::move(new_iter)), then(std::move(new_then))
{
}
