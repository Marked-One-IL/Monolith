#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <string_view>
#include <memory>

namespace Parser::AST::Type
{
    struct Typedef : Parser::AST::Type::Base
    {
        Typedef(std::unique_ptr<const Parser::AST::Type::Base> new_from);
        std::unique_ptr<const Parser::AST::Type::Base> from;
    };
}
