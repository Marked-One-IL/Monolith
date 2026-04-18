#pragma once
#include <Parser/AST/Stmt/Base.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Block.hpp>
#include <memory>

namespace Parser::AST::Stmt
{
    // This can represent if, elif and else at once.
    struct If : public Parser::AST::Stmt::Base
    {
        If(std::unique_ptr<const Parser::AST::Expr::Base> new_cond,
           std::unique_ptr<const Parser::AST::Block> new_then,
           std::unique_ptr<const Parser::AST::Stmt::If> new_els,
           const Lexer::LocationData& new_locationData);

        std::unique_ptr<const Parser::AST::Expr::Base> cond;
        std::unique_ptr<const Parser::AST::Block> then;
        std::unique_ptr<const Parser::AST::Stmt::If> els;
    };
}
