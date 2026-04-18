#include <Parser/AST/Type/Arr.hpp>

Parser::AST::Type::Arr::Arr(std::unique_ptr<const Parser::AST::Type::Base> new_type,
                            std::unique_ptr<const Parser::AST::Expr::Base> new_size) :
    Parser::AST::Type::Base(Parser::AST::Type::Tag::ARR), type(std::move(new_type)), size(std::move(new_size))
{
}

const Parser::AST::Type::Base* Parser::AST::Type::Arr::getType(void) const
{
    if (not this->type.get())
        return this->typeView;
    return this->type.get();
}
