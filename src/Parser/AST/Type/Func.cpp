#include <Parser/AST/Type/Func.hpp>

Parser::AST::Type::Func::Func(std::unique_ptr<const Parser::AST::Type::Base> new_returnType,
                              std::vector<std::unique_ptr<const Parser::AST::Type::Base>> new_params,
                              bool new_isEllipsis) :
    Parser::AST::Type::Base(Parser::AST::Type::Tag::FUNC), returnType(std::move(new_returnType)), params(std::move(new_params)), isEllipsis(new_isEllipsis)
{
}
