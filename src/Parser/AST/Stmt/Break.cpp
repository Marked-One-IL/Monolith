#include <Parser/AST/Stmt/Break.hpp>

Parser::AST::Stmt::Break::Break(const Lexer::LocationData& new_locationData) :
    Parser::AST::Stmt::Base(Parser::AST::Tag::STMT_BREAK, new_locationData)
{
}
