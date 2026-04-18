#pragma once
#include <Parser/AST/Stmt/Base.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Block.hpp>
#include <memory>

namespace Parser::AST::Stmt
{
    struct While : public Parser::AST::Stmt::Base
    {
        While(std::unique_ptr<const Parser::AST::Expr::Base> new_cond, std::unique_ptr<const Parser::AST::Block> new_then, const Lexer::LocationData& new_locationData);

        std::unique_ptr<const Parser::AST::Expr::Base> cond;
        std::unique_ptr<const Parser::AST::Block> then;
    };
}
