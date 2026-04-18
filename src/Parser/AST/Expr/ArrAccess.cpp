#include <Parser/AST/Expr/ArrAccess.hpp>

Parser::AST::Expr::ArrAccess::ArrAccess(std::unique_ptr<const Parser::AST::Expr::Base> new_location,
                                        std::unique_ptr<const Parser::AST::Expr::Base> new_index,
                                        const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_ARR_ACCESS, new_locationData), location(std::move(new_location)), index(std::move(new_index))
{
}
