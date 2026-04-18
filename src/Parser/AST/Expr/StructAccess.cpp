#include <Parser/AST/Expr/StructAccess.hpp>

Parser::AST::Expr::StructAccess::StructAccess(std::unique_ptr<const Parser::AST::Expr::Base> new_location,
                                              std::string_view new_field,
                                              const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_STRUCT_ACCESS, new_locationData), location(std::move(new_location)), field(new_field)
{
}
