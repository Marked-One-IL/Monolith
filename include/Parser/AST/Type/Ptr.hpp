#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <memory>

namespace Parser::AST::Type
{
    struct Ptr : Parser::AST::Type::Base
    {
        Ptr(std::unique_ptr<const Parser::AST::Type::Base> new_to);

        const Parser::AST::Type::Base* getTo(void) const;

        std::unique_ptr<const Parser::AST::Type::Base> to;
        mutable const Parser::AST::Type::Base* toView = nullptr;
    };
}
