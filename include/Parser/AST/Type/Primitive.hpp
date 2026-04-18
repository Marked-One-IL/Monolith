#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Type
{
    struct Primitive : Parser::AST::Type::Base
    {
        Primitive(std::string_view new_name);
        const std::string_view name;
    };
}
