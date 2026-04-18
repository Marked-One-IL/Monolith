#pragma once
#include <Parser/AST/Decl/Base.hpp>
#include <Parser/AST/Type/Typedef.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Decl
{
    struct Typedef : public Parser::AST::Decl::Base
    {
        Typedef(std::string_view new_name,
                std::unique_ptr<const Parser::AST::Type::Typedef> new_typeDef,
                const Lexer::LocationData& new_locationData);

        const std::string_view name;
        std::unique_ptr<const Parser::AST::Type::Typedef> typeDef;
    };
}
