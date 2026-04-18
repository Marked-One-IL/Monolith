#include <Parser/AST/Decl/Struct.hpp>

Parser::AST::Decl::Struct::Field::Field(std::string_view new_name,
                                        const Parser::AST::Type::Base* new_type,
                                        const Lexer::LocationData& new_locationData) :
    Parser::AST::Decl::Base(Parser::AST::Tag::DECL_FIELD, new_locationData), name(new_name), type(new_type)
{
}

Parser::AST::Decl::Struct::Struct(std::string_view new_name,
                                  std::unique_ptr<const Parser::AST::Type::Struct> new_data,
                                  const Lexer::LocationData& new_locationData) :
    Parser::AST::Decl::Base(Parser::AST::Tag::DECL_STRUCT, new_locationData), name(new_name), data(std::move(new_data))
{
    this->fieldsDecl.reserve(data->fields.size());

    for (Uint i = 0; i < data->fields.size(); i++)
    {
        std::string_view name = data.get()->fields[i].name;
        const Parser::AST::Type::Base* type = data.get()->fields[i].type.get();

        this->fieldsDecl.emplace_back(name, type, new_locationData);
    }
}
