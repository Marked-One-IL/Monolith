#include <Parser/AST/Type/Typedef.hpp>

Parser::AST::Type::Typedef::Typedef(std::unique_ptr<const Parser::AST::Type::Base> new_from) :
    Parser::AST::Type::Base(Parser::AST::Type::Tag::TYPEDEF), from(std::move(new_from))
{
}
