#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <vector>
#include <memory>

namespace Parser::AST::Expr
{
    struct Call : public Parser::AST::Expr::Base
    {
        Call(std::unique_ptr<const Parser::AST::Expr::Base> new_func,
             std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> new_params,
             const Lexer::LocationData& new_locationData);

        std::unique_ptr<const Parser::AST::Expr::Base> func;
        std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> params;
    };
}
