#include <Parser/AST/Stmt/If.hpp>

Parser::AST::Stmt::If::If(std::unique_ptr<const Parser::AST::Expr::Base> new_cond,
                          std::unique_ptr<const Parser::AST::Block> new_then,
                          std::unique_ptr<const Parser::AST::Stmt::If> new_els,
                          const Lexer::LocationData& new_locationData) :
    Parser::AST::Stmt::Base(Parser::AST::Tag::STMT_IF, new_locationData), cond(std::move(new_cond)), then(std::move(new_then)), els(std::move(new_els))
{
}
