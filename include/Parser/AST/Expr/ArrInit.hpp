#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <vector>
#include <memory>

namespace Parser::AST::Expr
{
    struct ArrInit : public Parser::AST::Expr::Base
    {
        ArrInit(std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> new_elements, const Lexer::LocationData& new_locationData);
        std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> elements;
    };
}
