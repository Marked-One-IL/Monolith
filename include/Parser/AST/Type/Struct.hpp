#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <string_view>
#include <vector>
#include <memory>

namespace Parser::AST::Type
{
    struct Struct : Parser::AST::Type::Base
    {
        struct Field
        {
            Field(std::string_view new_name,
                  std::unique_ptr<const Parser::AST::Type::Base> new_type);

            const std::string_view name;
            std::unique_ptr<const Parser::AST::Type::Base> type;
        };

        Struct(std::vector<Parser::AST::Type::Struct::Field> new_fields);
        std::vector<Parser::AST::Type::Struct::Field> fields;
    };
}
