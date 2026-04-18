#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <memory>

namespace Parser::AST::Expr
{
    struct ArrAccess : public Parser::AST::Expr::Base
    {
        ArrAccess(std::unique_ptr<const Parser::AST::Expr::Base> new_location,
                  std::unique_ptr<const Parser::AST::Expr::Base> new_index,
                  const Lexer::LocationData& new_locationData);

        mutable std::unique_ptr<const Parser::AST::Expr::Base> location;
        std::unique_ptr<const Parser::AST::Expr::Base> index;
    };
}
