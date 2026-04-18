#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Type/Base.hpp>
#include <memory>

namespace Parser::AST::Expr
{
    struct Cast : public Parser::AST::Expr::Base
    {
        Cast(std::unique_ptr<const Parser::AST::Expr::Base> new_expr,
             std::unique_ptr<const Parser::AST::Type::Base> new_as,
             const Lexer::LocationData& new_locationData);

        std::unique_ptr<const Parser::AST::Expr::Base> expr;
        std::unique_ptr<const Parser::AST::Type::Base> as;
    };
}
