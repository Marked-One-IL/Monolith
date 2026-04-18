#include <Parser/AST/Type/Primitive.hpp>

Parser::AST::Type::Primitive::Primitive(std::string_view new_name) :
    Parser::AST::Type::Base(Parser::AST::Type::Tag::PRIMITIVE), name(new_name)
{
}
