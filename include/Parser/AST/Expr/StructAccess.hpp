#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Expr
{
    struct StructAccess : public Parser::AST::Expr::Base
    {
        StructAccess(std::unique_ptr<const Parser::AST::Expr::Base> new_location,
                     std::string_view new_field,
                     const Lexer::LocationData& new_locationData);

        mutable std::unique_ptr<const Parser::AST::Expr::Base> location;
        const std::string_view field;
    };
}
