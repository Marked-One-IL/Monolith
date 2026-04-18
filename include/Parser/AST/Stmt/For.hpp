#pragma once
#include <Parser/AST/Stmt/Base.hpp>
#include <Parser/AST/Stmt/Assign.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Block.hpp>
#include <memory>

namespace Parser::AST::Stmt
{
    struct For : public Parser::AST::Stmt::Base
    {
        For(std::unique_ptr<const Parser::AST::Stmt::Assign> new_init,
            std::unique_ptr<const Parser::AST::Expr::Base> new_cond,
            std::unique_ptr<const Parser::AST::Stmt::Assign> new_iter,
            std::unique_ptr<const Parser::AST::Block> new_then,
            const Lexer::LocationData& new_locationData);

        std::unique_ptr<const Parser::AST::Stmt::Assign> init;
        std::unique_ptr<const Parser::AST::Expr::Base> cond;
        std::unique_ptr<const Parser::AST::Stmt::Assign> iter;
        std::unique_ptr<const Parser::AST::Block> then;
    };
}
