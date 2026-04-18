#include <Parser/AST/Type/Ptr.hpp>

Parser::AST::Type::Ptr::Ptr(std::unique_ptr<const Parser::AST::Type::Base> new_to) :
    Parser::AST::Type::Base(Parser::AST::Type::Tag::PTR), to(std::move(new_to))
{
}

const Parser::AST::Type::Base* Parser::AST::Type::Ptr::getTo(void) const
{
    if (this->to.get() == nullptr)
        return this->toView;
    return this->to.get();
}
