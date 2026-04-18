#pragma once
#include <Parser/AST/Decl/Base.hpp>
#include <Parser/AST/Type/Base.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Decl
{
    struct Var : public Parser::AST::Decl::Base
    {
        Var(std::string_view new_name,
            std::unique_ptr<const Parser::AST::Type::Base> new_type,
            std::unique_ptr<const Parser::AST::Expr::Base> new_init,
            const Lexer::LocationData& new_locationData);

        const std::string_view name;
        std::unique_ptr<const Parser::AST::Type::Base> type;
        std::unique_ptr<const Parser::AST::Expr::Base> init;
    };
}
