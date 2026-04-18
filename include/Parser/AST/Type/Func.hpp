#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <vector>
#include <memory>

namespace Parser::AST::Type
{
    struct Func : Parser::AST::Type::Base
    {
        Func(std::unique_ptr<const Parser::AST::Type::Base> new_returnType,
             std::vector<std::unique_ptr<const Parser::AST::Type::Base>> new_params,
             bool new_isEllipsis);

        std::unique_ptr<const Parser::AST::Type::Base> returnType;
        std::vector<std::unique_ptr<const Parser::AST::Type::Base>> params;
        bool isEllipsis = false; // printf(const char* fmt, ...) 
    };
}
