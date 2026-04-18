#include <Parser/AST/Expr/Sizeof.hpp>

Parser::AST::Expr::Sizeof::Sizeof(std::unique_ptr<const Parser::AST::Type::Base> new_of, const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_SIZEOF, new_locationData), of(std::move(new_of))
{
}
