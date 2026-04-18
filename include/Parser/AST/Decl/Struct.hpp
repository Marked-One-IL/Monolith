#pragma once
#include <Parser/AST/Decl/Base.hpp>
#include <Parser/AST/Type/Struct.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <string_view>
#include <vector>
#include <memory>

namespace Parser::AST::Decl
{
    struct Struct : public Parser::AST::Decl::Base
    {
        struct Field : public Parser::AST::Decl::Base
        {
            Field(std::string_view new_name,
                  const Parser::AST::Type::Base* new_type,
                  const Lexer::LocationData& new_locationData);

            const std::string_view name;
            const Parser::AST::Type::Base* type;
        };

        Struct(std::string_view new_name,
               std::unique_ptr<const Parser::AST::Type::Struct> new_data,
               const Lexer::LocationData& new_locationData);

        const std::string_view name;
        std::unique_ptr<const Parser::AST::Type::Struct> data;
        std::vector<Parser::AST::Decl::Struct::Field> fieldsDecl; // This is a view of the data above. Not taking ownership of them. This is just for easy use.
    };
}
