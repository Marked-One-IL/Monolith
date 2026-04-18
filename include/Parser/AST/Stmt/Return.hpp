#pragma once
#include <Parser/AST/Stmt/Base.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <memory>

namespace Parser::AST::Stmt
{
    struct Return : public Parser::AST::Stmt::Base
    {
        Return(std::unique_ptr<const Parser::AST::Expr::Base> new_expr, const Lexer::LocationData& new_locationData);
        std::unique_ptr<const Parser::AST::Expr::Base> expr;
    };
}
