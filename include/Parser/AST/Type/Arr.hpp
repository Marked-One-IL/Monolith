#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <Parser/AST/Expr/Base.hpp>
#include <memory>

namespace Parser::AST::Type
{
    struct Arr : Parser::AST::Type::Base
    {
        Arr(std::unique_ptr<const Parser::AST::Type::Base> new_type,
            std::unique_ptr<const Parser::AST::Expr::Base> new_size);

        const Parser::AST::Type::Base* getType(void) const;

        std::unique_ptr<const Parser::AST::Type::Base> type;
        std::unique_ptr<const Parser::AST::Expr::Base> size;
        mutable const Parser::AST::Type::Base* typeView = nullptr;
    };
}
