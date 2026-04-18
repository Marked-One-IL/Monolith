#include <Parser/AST/Type/Struct.hpp>

Parser::AST::Type::Struct::Field::Field(std::string_view new_name, std::unique_ptr<const Parser::AST::Type::Base> new_type) :
    name(new_name), type(std::move(new_type))
{
}

Parser::AST::Type::Struct::Struct(std::vector<Parser::AST::Type::Struct::Field> new_fields) :
    Parser::AST::Type::Base(Parser::AST::Type::Tag::STRUCT), fields(std::move(new_fields))
{
}
