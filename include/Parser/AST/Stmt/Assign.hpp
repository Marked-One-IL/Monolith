#pragma once
#include <Parser/AST/Stmt/Base.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Stmt
{
    struct Assign : public Parser::AST::Stmt::Base
    {
        Assign(std::unique_ptr<const Parser::AST::Expr::Base> new_dst,
               std::unique_ptr<const Parser::AST::Expr::Base> new_src,
               std::string_view new_operation,
               const Lexer::LocationData& new_locationData);

        mutable std::unique_ptr<const Parser::AST::Expr::Base> dst;
        mutable std::unique_ptr<const Parser::AST::Expr::Base> src;
        const std::string_view operation;
    };
}
