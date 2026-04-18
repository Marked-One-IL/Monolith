#include <Parser/AST/Decl/Var.hpp>

Parser::AST::Decl::Var::Var(std::string_view new_name,
                            std::unique_ptr<const Parser::AST::Type::Base> new_type,
                            std::unique_ptr<const Parser::AST::Expr::Base> new_init,
                            const Lexer::LocationData& new_locationData) :
    Parser::AST::Decl::Base(Parser::AST::Tag::DECL_VAR, new_locationData), name(new_name), type(std::move(new_type)), init(std::move(new_init))
{

}
