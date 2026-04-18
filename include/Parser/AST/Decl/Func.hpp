#pragma once
#include <Parser/AST/Decl/Base.hpp>
#include <Parser/AST/Type/Func.hpp>
#include <Parser/AST/Block.hpp>
#include <vector>
#include <string_view>
#include <memory>

namespace Parser::AST::Decl
{
    struct Func : public Parser::AST::Decl::Base
    {
        struct Param : public Parser::AST::Decl::Base
        {
            Param(std::string_view new_name,
                  const Parser::AST::Type::Base* new_type,
                  const Lexer::LocationData& new_locationData);

            const std::string_view name;
            const Parser::AST::Type::Base* type = nullptr;
        };

        Func(std::string_view new_name,
             std::unique_ptr<const Parser::AST::Type::Func> new_data,
             std::vector<std::string_view> new_paramNames,
             std::unique_ptr<const Parser::AST::Block> new_block,
             const Lexer::LocationData& new_locationData);

        const std::string_view name;
        std::unique_ptr<const Parser::AST::Type::Func> data;
        std::vector<std::string_view> paramNames;
        std::unique_ptr<const Parser::AST::Block> block;

        std::vector<Parser::AST::Decl::Func::Param> paramsDecl; // This is a view of the data above. Not taking ownership of them. This is just for easy use.
    };
}
