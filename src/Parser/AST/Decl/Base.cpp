#include <Parser/AST/Decl/Base.hpp>
#include <Parser/AST/Decl/Var.hpp>
#include <Parser/AST/Decl/Typedef.hpp>
#include <Parser/AST/Decl/Func.hpp>
#include <Parser/AST/Decl/Struct.hpp>

const Parser::AST::Type::Base* Parser::AST::Decl::Base::getType(void) const
{
    const Parser::AST::Type::Base* type = nullptr;

    switch (this->tag)
    {
    case Parser::AST::Tag::DECL_VAR:
    {
        auto castedType = static_cast<const Parser::AST::Decl::Var*>(this);
        type = castedType->type.get();
        break;
    }
    case Parser::AST::Tag::DECL_FUNC:
    {
        auto castedType = static_cast<const Parser::AST::Decl::Func*>(this);
        type = castedType->data.get();
        break;
    }
    case Parser::AST::Tag::DECL_STRUCT:
    {
        auto castedType = static_cast<const Parser::AST::Decl::Struct*>(this);
        type = castedType->data.get();
        break;
    }
    case Parser::AST::Tag::DECL_TYPEDEF:
    {
        auto castedType = static_cast<const Parser::AST::Decl::Typedef*>(this);
        type = castedType->typeDef.get();
        break;
    }
    case Parser::AST::Tag::DECL_PARAM:
    {
        auto castedType = static_cast<const Parser::AST::Decl::Func::Param*>(this);
        type = castedType->type;
        break;
    }
    case Parser::AST::Tag::DECL_FIELD:
    {
        auto castedType = static_cast<const Parser::AST::Decl::Struct::Field*>(this);
        type = castedType->type;
        break;
    }
    }
    
    return Parser::AST::Type::Base::getUnderlyingType(type);
}
