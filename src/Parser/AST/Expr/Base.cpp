#include <Parser/AST/Expr/Base.hpp>
#include <Parser/AST/Expr/ArrAccess.hpp>
#include <Parser/AST/Expr/Bin.hpp>
#include <Parser/AST/Expr/Call.hpp>
#include <Parser/AST/Expr/StructAccess.hpp>
#include <Parser/AST/Expr/Identifier.hpp>
#include <Parser/AST/Expr/Literal.hpp>
#include <Parser/AST/Expr/Unary.hpp>
#include <Parser/AST/Expr/Cast.hpp>
#include <Parser/AST/Expr/Sizeof.hpp>
#include <Parser/AST/Expr/ArrInit.hpp>
#include <Parser/AST/Expr/StructInit.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <Parser/AST/Type/Primitive.hpp>
#include <Parser/AST/Type/Typedef.hpp>
#include <Parser/AST/Type/Ptr.hpp>
#include <Parser/AST/Type/Arr.hpp>
#include <Parser/AST/Type/Struct.hpp>
#include <Parser/AST/Type/Func.hpp>
#include <Semantic/TypeChecking.hpp>
#include <Helper/Assert.hpp>
#include <memory>
#include <array>
#include <algorithm>
#include <string>
#include <list>

const Parser::AST::Type::Base* Parser::AST::Expr::Base::getType(void) const
{
    // God forgive me.
    // What I do not to leak stuff.
    static auto noneType = std::make_unique<const Parser::AST::Type::Ptr>(nullptr);
    static auto boolType = std::make_unique<const Parser::AST::Type::Primitive>("bool");
    static auto charType = std::make_unique<const Parser::AST::Type::Primitive>("char");
    static auto intType = std::make_unique<const Parser::AST::Type::Primitive>("int");
    static auto floatType = std::make_unique<const Parser::AST::Type::Primitive>("float");
    static auto strType = std::make_unique<const Parser::AST::Type::Ptr>(std::make_unique<const Parser::AST::Type::Primitive>("char"));
    static std::vector<std::unique_ptr<const Parser::AST::Type::Base>> types;
    static std::list<std::string> strs;

    if (this == nullptr) return nullptr;

    switch (this->tag)
    {
        case Parser::AST::Tag::EXPR_LITERAL:
        {
            auto castedType = static_cast<const Parser::AST::Expr::Literal*>(this);

            if (castedType->literal.isNone()) return noneType.get();
            if (castedType->literal.isBool()) return boolType.get();
            if (castedType->literal.isChar()) return charType.get();
            if (castedType->literal.isInt()) return intType.get();
            if (castedType->literal.isFloat()) return floatType.get();
            if (castedType->literal.isStr()) return strType.get();

            Assert_Message(ASSERT_ALWAYS, "Invalid literal");
            return nullptr;
            break;
        }
        case Parser::AST::Tag::EXPR_IDENTIFIER:
        {
            auto castedType = static_cast<const Parser::AST::Expr::Identifier*>(this);
            return Parser::AST::Type::Base::getUnderlyingType(castedType->origin->getType());
            break;
        }
        case Parser::AST::Tag::EXPR_BIN:
        {
            auto castedType = static_cast<const Parser::AST::Expr::Bin*>(this);
            auto left = castedType->left.get()->getType();
            auto right = castedType->right.get()->getType();

            if (not Parser::AST::Type::Base::compare(left, right)) throw Semantic::TypeChecking::Error("Binary operation with different sides type", castedType);
            if (not left->Parser::AST::Type::Base::isBinaryValid(castedType->operation)) throw Semantic::TypeChecking::Error("Invalid Binary operation with used types", castedType);

            static constexpr auto boolOperations = std::to_array<std::string_view>(
            {
                "<", ">", "<=", ">=", "==", "!=", "is", "or", "and"
            });
            if (std::find(boolOperations.begin(), boolOperations.end(), castedType->operation) != boolOperations.end())
            {
                return boolType.get();
            }

            return Parser::AST::Type::Base::getUnderlyingType(left); // return right; is also valid.
            break;
        }
        case Parser::AST::Tag::EXPR_UNARY:
        {
            auto castedType = static_cast<const Parser::AST::Expr::Unary*>(this);
            auto dstType = castedType->right->getType();

            if (not castedType->right->isUnaryValid(castedType->operation, dstType)) throw Semantic::TypeChecking::Error("Invalid unary operation to used type", castedType);
            if (castedType->operation == "ref")
            {
                auto newPtr = std::make_unique<const Parser::AST::Type::Ptr>(nullptr);
                newPtr->toView = dstType;
                types.emplace_back(std::move(newPtr));
                return Parser::AST::Type::Base::getUnderlyingType(types.back().get());
            }
            if (castedType->operation == "dref")
            {
                auto castedDst = static_cast<const Parser::AST::Type::Ptr*>(dstType);
                return Parser::AST::Type::Base::getUnderlyingType(castedDst->getTo());
            }

            return Parser::AST::Type::Base::getUnderlyingType(dstType);
            break;
        }
        case Parser::AST::Tag::EXPR_CALL:
        {
            auto castedType = static_cast<const Parser::AST::Expr::Call*>(this);
            auto srcFunc = castedType->func.get()->getType();

            if (srcFunc->tag != Parser::AST::Type::Tag::FUNC) throw Semantic::TypeChecking::Error("Source function call is not valid", castedType);
            auto castedFunc = static_cast<const Parser::AST::Type::Func*>(srcFunc);

            if (castedFunc->params.size() != castedType->params.size() and not castedFunc->isEllipsis)
                throw Semantic::TypeChecking::Error("Source function call parameters amount is not the same", castedType);

            for (Uint i = 0; i < castedFunc->params.size(); i++)
            {
                if (not Parser::AST::Type::Base::compare(castedType->params[i]->getType(), castedFunc->params[i].get()))
                    throw Semantic::TypeChecking::Error("Function call parameter type mismatch", castedType);
            }
            if (castedFunc->isEllipsis)
            {
                for (Uint i = castedFunc->params.size(); i < castedType->params.size(); i++)
                {
                    auto type = castedType->params[i]->getType();
                    if (not type) continue;

                    if (type->tag == Parser::AST::Type::Tag::STRUCT or
                        type->tag == Parser::AST::Type::Tag::ARR or
                        type->tag == Parser::AST::Type::Tag::FUNC)
                    {
                        throw Semantic::TypeChecking::Error("Function parameter in the '...' must a primitve", castedType);
                    }
                }
            }

            return Parser::AST::Type::Base::getUnderlyingType(castedFunc->returnType.get());
            break;
        }
        case Parser::AST::Tag::EXPR_ARR_ACCESS:
        {
            auto castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(this);
            auto srcArr = castedType->location.get()->getType();
            auto indexType = castedType->index.get()->getType();
            if (not Parser::AST::Type::Base::compare(indexType, intType.get())) throw Semantic::TypeChecking::Error("Array index must be int", castedType);

            if (srcArr->tag == Parser::AST::Type::Tag::ARR)
            {
                auto castedSrcType = static_cast<const Parser::AST::Type::Arr*>(srcArr);
                return Parser::AST::Type::Base::getUnderlyingType(castedSrcType->type.get());
            }
            else if (srcArr->tag == Parser::AST::Type::Tag::PTR)
            {
                auto castedSrcType = static_cast<const Parser::AST::Type::Ptr*>(srcArr);
                return Parser::AST::Type::Base::getUnderlyingType(castedSrcType->getTo());
            }

            throw Semantic::TypeChecking::Error("Source array is not valid", castedType);
        }
        case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
        {
            auto castedType = static_cast<const Parser::AST::Expr::StructAccess*>(this);
            auto srcStruct = castedType->location.get()->getType();

            if (srcStruct->tag != Parser::AST::Type::Tag::STRUCT) throw Semantic::TypeChecking::Error("Source struct is not valid", castedType);
            auto castedStruct = static_cast<const Parser::AST::Type::Struct*>(srcStruct);

            for (auto& field : castedStruct->fields)
            {
                if (field.name == castedType->field)
                {
                    return Parser::AST::Type::Base::getUnderlyingType(field.type.get());
                }
            }

            throw Semantic::TypeChecking::Error("Field name of the struct is not valid", castedType);
            break;
        }
        case Parser::AST::Tag::EXPR_SIZEOF:
        {
            return Parser::AST::Type::Base::getUnderlyingType(intType.get());
            break;
        }
        case Parser::AST::Tag::EXPR_CAST:
        {
            auto castedType = static_cast<const Parser::AST::Expr::Cast*>(this);
            if (not Parser::AST::Type::Base::isCastable(castedType->expr->getType(), castedType->as.get()))
                throw Semantic::TypeChecking::Error("Invalid cast", castedType);

            return Parser::AST::Type::Base::getUnderlyingType(castedType->as.get());
            break;
        }
        case Parser::AST::Tag::EXPR_ARR_INIT:
        {
            auto castedType = static_cast<const Parser::AST::Expr::ArrInit*>(this);

            Assert(castedType->elements.size() > 0);
            auto firstElement = castedType->elements[0]->getType();
            for (auto& element : castedType->elements)
            {
                if (not Parser::AST::Type::Base::compare(firstElement, element.get()->getType()))
                    throw Semantic::TypeChecking::Error("Not all elements have the same type", castedType);
            }

            // God forgive me again.
            strs.emplace_back(std::to_string(castedType->elements.size()));
            Lexer::Token token(Lexer::Tag::INT_LITERAL, strs.back());
            token.locationData.filename = castedType->locationData.filename;
            token.locationData.lineNum = castedType->locationData.lineNum;
            token.locationData.line = castedType->locationData.line;

            auto arrType = std::make_unique<const Parser::AST::Type::Arr>(nullptr,
                           std::make_unique<const Parser::AST::Expr::Literal>(token, token.locationData));
            arrType->typeView = firstElement;
            types.emplace_back(std::move(arrType));
            return Parser::AST::Type::Base::getUnderlyingType(types.back().get());
            break;
        }
        case Parser::AST::Tag::EXPR_STRUCT_INIT:
        {
            auto castedType = static_cast<const Parser::AST::Expr::StructInit*>(this);
            auto srcStructType = castedType->structName.get()->origin;
            auto tempCastedStuctType = srcStructType->getType();
            if (tempCastedStuctType->tag != Parser::AST::Type::Tag::STRUCT) throw Semantic::TypeChecking::Error("Struct identifier does not point to an struct", castedType);
            auto castedSrcStructType = static_cast<const Parser::AST::Type::Struct*>(tempCastedStuctType);

            if (castedType->fields.size() != castedSrcStructType->fields.size()) 
                throw Semantic::TypeChecking::Error("Struct initialization fields amount does not match the amount of the original struct", castedType);

            for (Uint i = 0; i < castedType->fields.size(); i++)
            {
                if (not Parser::AST::Type::Base::compare(castedType->fields[i].get()->getType(), castedSrcStructType->fields[i].type.get()))
                    throw Semantic::TypeChecking::Error("Type mismatch between the Struct initialization fields and the original struct", castedType);
            }

            return Parser::AST::Type::Base::getUnderlyingType(castedSrcStructType);
            break;
        }
    }

    Assert_Message(ASSERT_ALWAYS, "Unknown expression");
    return nullptr;
}

bool Parser::AST::Expr::Base::isUnaryValid(std::string_view opr, const Parser::AST::Type::Base* type) const
{
    if (this == nullptr) return false;
    if (opr == "ref") return this->isDeepIdentifier();

    return type->isUnaryValid(opr);
}

bool Parser::AST::Expr::Base::isDeepIdentifier(void) const
{
    if (this == nullptr) return false;

    switch (this->tag)
    {
    case Parser::AST::Tag::EXPR_ARR_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(this);
        return castedType->location->isDeepIdentifier();
    }
    case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::StructAccess*>(this);
        return castedType->location->isDeepIdentifier();
    }
    case Parser::AST::Tag::EXPR_UNARY:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Unary*>(this);
        if (castedType->operation != "dref") return false;
        return castedType->right->isDeepIdentifier();
    }
    case Parser::AST::Tag::EXPR_IDENTIFIER:
        return true;

    case Parser::AST::Tag::EXPR_LITERAL:
    case Parser::AST::Tag::EXPR_BIN:
    case Parser::AST::Tag::EXPR_CALL:
    case Parser::AST::Tag::EXPR_SIZEOF:
    case Parser::AST::Tag::EXPR_CAST:
    case Parser::AST::Tag::EXPR_ARR_INIT:
        return false;
    }

    return false;
}
