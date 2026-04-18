#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Type/Base.hpp>
#include <memory>

namespace Parser::AST::Expr
{
    struct Sizeof : public Parser::AST::Expr::Base
    {
        Sizeof(std::unique_ptr<const Parser::AST::Type::Base> new_of, const Lexer::LocationData& new_locationData);
        std::unique_ptr<const Parser::AST::Type::Base> of;
    };
}
