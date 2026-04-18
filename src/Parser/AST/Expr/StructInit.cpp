#include <Parser/AST/Expr/StructInit.hpp>

Parser::AST::Expr::StructInit::StructInit(std::unique_ptr<const Parser::AST::Type::Identifier> new_structName,
                                          std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> new_fields,
                                          const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_STRUCT_INIT, new_locationData), structName(std::move(new_structName)), fields(std::move(new_fields))
{
}
