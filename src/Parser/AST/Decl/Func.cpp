#include <Parser/AST/Decl/Func.hpp>

Parser::AST::Decl::Func::Param::Param(std::string_view new_name,
                                      const Parser::AST::Type::Base* new_type,
                                      const Lexer::LocationData& new_locationData) :
    Parser::AST::Decl::Base(Parser::AST::Tag::DECL_PARAM, new_locationData), name(new_name), type(new_type)
{
}

Parser::AST::Decl::Func::Func(std::string_view new_name,
                              std::unique_ptr<const Parser::AST::Type::Func> new_data,
                              std::vector<std::string_view> new_paramNames,
                              std::unique_ptr<const Parser::AST::Block> new_block,
                              const Lexer::LocationData& new_locationData) :
    Parser::AST::Decl::Base(Parser::AST::Tag::DECL_FUNC, new_locationData), name(new_name), data(std::move(new_data)), paramNames(new_paramNames), block(std::move(new_block))
{
    this->paramsDecl.reserve(paramNames.size());

    for (Uint i = 0; i < paramNames.size(); i++)
    {
        std::string_view name = paramNames[i];
        const Parser::AST::Type::Base* type = data.get()->params[i].get();
        this->paramsDecl.emplace_back(name, type, new_locationData);
    }
}
