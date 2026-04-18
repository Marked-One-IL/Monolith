#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Expr
{
    struct Unary : public Parser::AST::Expr::Base
    {
        Unary(std::string_view new_operation,
              std::unique_ptr<const Parser::AST::Expr::Base> new_right,
              const Lexer::LocationData& new_locationData = {});

        const std::string_view operation;
        mutable std::unique_ptr<const Parser::AST::Expr::Base> right;
    };
}
