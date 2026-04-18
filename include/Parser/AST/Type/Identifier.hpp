#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <Parser/AST/Decl/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Type
{
    struct Identifier : public Parser::AST::Type::Base
    {
        Identifier(std::string_view new_name);

        const std::string_view name;
        mutable const Parser::AST::Decl::Base* origin = nullptr; // This doesn't not own origin just view it.
    };
}
