#include <Parser/AST/Expr/Bin.hpp>

Parser::AST::Expr::Bin::Bin(std::unique_ptr<const Parser::AST::Expr::Base> new_left,
                            std::unique_ptr<const Parser::AST::Expr::Base> new_right,
                            std::string_view new_operation,
                            const Lexer::LocationData& new_locationData) :
    Parser::AST::Expr::Base(Parser::AST::Tag::EXPR_BIN, new_locationData), left(std::move(new_left)), right(std::move(new_right)), operation(std::move(new_operation))
{
}
