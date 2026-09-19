#include <Parser/AST/Expr/ArrInit.hpp>

Parser::AST::Expr::ArrInit::ArrInit(std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> new_elements, const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_ARR_INIT, new_locationData), elements(std::move(new_elements))
{
}
