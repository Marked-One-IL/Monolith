#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Expr
{
    struct Bin : public Parser::AST::Expr::Base
    {
        Bin(std::unique_ptr<const Parser::AST::Expr::Base> new_left,
            std::unique_ptr<const Parser::AST::Expr::Base> new_right,
            std::string_view new_operation,
            const Lexer::LocationData& new_locationData);

        std::unique_ptr<const Parser::AST::Expr::Base> left;
        std::unique_ptr<const Parser::AST::Expr::Base> right;
        const std::string_view operation;
    };
}
