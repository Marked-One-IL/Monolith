#include <Parser/AST/Decl/Typedef.hpp>

Parser::AST::Decl::Typedef::Typedef(std::string_view new_name,
                                    std::unique_ptr<const Parser::AST::Type::Typedef> new_typeDef,
                                    const Lexer::LocationData& new_locationData) :
    Parser::AST::Decl::Base(Parser::AST::Tag::DECL_TYPEDEF, new_locationData), name(new_name), typeDef(std::move(new_typeDef))
{
}
