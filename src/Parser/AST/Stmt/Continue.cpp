#include <Parser/AST/Stmt/Continue.hpp>

Parser::AST::Stmt::Continue::Continue(const Lexer::LocationData& new_locationData) :
    Parser::AST::Stmt::Base(Parser::AST::Tag::STMT_CONTINUE, new_locationData)
{
}
