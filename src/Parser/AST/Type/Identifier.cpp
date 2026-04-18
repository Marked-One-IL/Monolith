#include <Parser/AST/Type/Identifier.hpp>

Parser::AST::Type::Identifier::Identifier(std::string_view new_name) :
    Parser::AST::Type::Base(Parser::AST::Type::Tag::IDENTIFIER), name(new_name)
{
}
