#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <vector>
#include <memory>

namespace Parser::AST::Expr
{
    struct StructInit : public Parser::AST::Expr::Base
    {
        StructInit(std::unique_ptr<const Parser::AST::Type::Identifier> new_structName,
                   std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> new_fields,
                   const Lexer::LocationData& new_locationData);

        std::unique_ptr<const Parser::AST::Type::Identifier> structName;
        std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> fields;
    };
}
