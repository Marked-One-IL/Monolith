#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Decl/Var.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Expr
{
    struct Identifier : public Parser::AST::Expr::Base
    {
        Identifier(std::string_view new_name, const Lexer::LocationData& new_locationData);

        const std::string_view name;
        mutable const Parser::AST::Decl::Base* origin = nullptr; // This doesn't not own origin just view it.
    };
}
