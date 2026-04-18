#include <Parser/AST/Type/Base.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <Parser/AST/Type/Primitive.hpp>
#include <Parser/AST/Type/Typedef.hpp>
#include <Parser/AST/Type/Ptr.hpp>
#include <Parser/AST/Type/Arr.hpp>
#include <Parser/AST/Type/Struct.hpp>
#include <Parser/AST/Type/Func.hpp>
#include <Parser/AST/Decl/Var.hpp>
#include <Parser/AST/Decl/Func.hpp>
#include <Parser/AST/Decl/Struct.hpp>
#include <Parser/AST/Decl/Typedef.hpp>
#include <Parser/AST/Expr/Literal.hpp>
#include <Helper/Helper.hpp>
#include <Helper/Assert.hpp>
#include <format>
#include <array>

Parser::AST::Type::Base::Base(Parser::AST::Type::Tag new_tag) :
    tag(new_tag)
{
}

void Parser::AST::Type::Base::print(std::ostream& stream, Uint depth) const
{
    if (this == nullptr) return;

    switch (this->tag)
    {
    case Parser::AST::Type::Tag::IDENTIFIER:
    {
        const Parser::AST::Type::Identifier* castedType = static_cast<const Parser::AST::Type::Identifier*>(this);

        Assert_Message(castedType->origin, "origin is a nullptr");
        
        Helper::printDepth(stream, depth); stream << std::format("Identifier()\n");
        Helper::printDepth(stream, depth); castedType->origin->print(stream, depth + 1);

        return;
    }
    case Parser::AST::Type::Tag::PRIMITIVE:
    {
        const Parser::AST::Type::Primitive* castedType = static_cast<const Parser::AST::Type::Primitive*>(this);
        Helper::printDepth(stream, depth); stream << std::format("Primitive({})\n", castedType->name);
        return;
    }
    case Parser::AST::Type::Tag::TYPEDEF:
    {
        const Parser::AST::Type::Typedef* castedType = static_cast<const Parser::AST::Type::Typedef*>(this);
        Helper::printDepth(stream, depth); stream << std::format("Typedef() ->\n");
        Helper::printDepth(stream, depth); castedType->from->print(stream, depth + 1);
        return;
    }
    case Parser::AST::Type::Tag::PTR:
    {
        const Parser::AST::Type::Ptr* castedType = static_cast<const Parser::AST::Type::Ptr*>(this);
        Helper::printDepth(stream, depth); stream << std::format("Ptr() ->\n");
        Helper::printDepth(stream, depth); castedType->to->print(stream, depth + 1);
        return;
    }
    case Parser::AST::Type::Tag::ARR:
    {
        const Parser::AST::Type::Arr* castedType = static_cast<const Parser::AST::Type::Arr*>(this);
        Helper::printDepth(stream, depth); stream << std::format("Arr() ->\n");
        Helper::printDepth(stream, depth); castedType->type->print(stream, depth + 1);
        Helper::printDepth(stream, depth); stream << "Size:\n"; castedType->size->print(stream, depth + 1);
        return;
    }
    case Parser::AST::Type::Tag::FUNC:
    {
        const Parser::AST::Type::Func* castedType = static_cast<const Parser::AST::Type::Func*>(this);
        Helper::printDepth(stream, depth); stream << std::format("Func() ->\n");
        Helper::printDepth(stream, depth); stream << "Return:\n"; castedType->returnType->print(stream, depth + 2);
        Helper::printDepth(stream, depth); stream << "Params:\n";
        for (auto& param : castedType->params)
        {
            Helper::printDepth(stream, depth); param->print(stream, depth + 2);
        }
        break;
    }
    case Parser::AST::Type::Tag::STRUCT:
    {
        const Parser::AST::Type::Struct* castedType = static_cast<const Parser::AST::Type::Struct*>(this);
        Helper::printDepth(stream, depth); stream << std::format("Struct() ->\n");
        Helper::printDepth(stream, depth); stream << "Field:\n";
        for (auto& field : castedType->fields)
        {
            Helper::printDepth(stream, depth); stream << "name: " << field.name << '\n';
            field.type->print(stream, depth);
        }
        break;
    }
    };

    stream << '\n';
}

bool Parser::AST::Type::Base::compare(const Parser::AST::Type::Base* left, const Parser::AST::Type::Base* right)
{
    const Parser::AST::Type::Base* newLeft = Parser::AST::Type::Base::getUnderlyingType(left);
    const Parser::AST::Type::Base* newRight = Parser::AST::Type::Base::getUnderlyingType(right);
    if (not newLeft and not newRight) return true;
    if (not newLeft or not newRight) return false;
    if (newLeft->tag != newRight->tag) return false;

    switch (newLeft->tag) // Could be also newRight->tag
    {
        case Parser::AST::Type::Tag::IDENTIFIER: Assert_Message(ASSERT_ALWAYS, "Tag cannot be identifier");
        case Parser::AST::Type::Tag::TYPEDEF: Assert_Message(ASSERT_ALWAYS, "Tag cannot be typedef");
        
        case Parser::AST::Type::Tag::PRIMITIVE:
        {
            const Parser::AST::Type::Primitive* castedLeft = static_cast<const Parser::AST::Type::Primitive*>(newLeft);
            const Parser::AST::Type::Primitive* castedRight = static_cast<const Parser::AST::Type::Primitive*>(newRight);

            return castedLeft->name == castedRight->name;
        }
        
        case Parser::AST::Type::Tag::PTR:
        {
            const Parser::AST::Type::Ptr* castedLeft = static_cast<const Parser::AST::Type::Ptr*>(newLeft);
            const Parser::AST::Type::Ptr* castedRight = static_cast<const Parser::AST::Type::Ptr*>(newRight);

            return Parser::AST::Type::Base::compare(castedLeft->getTo(), castedRight->getTo());
        }

        case Parser::AST::Type::Tag::ARR:
        {
            const Parser::AST::Type::Arr* castedLeft = static_cast<const Parser::AST::Type::Arr*>(newLeft);
            const Parser::AST::Type::Arr* castedRight = static_cast<const Parser::AST::Type::Arr*>(newRight);

            const Lexer::Token& leftToken = static_cast<const Parser::AST::Expr::Literal*>(castedLeft->size.get())->literal;
            const std::string_view leftLiteral = leftToken.content;
            Uint leftValue = 0;
            Uint leftOffset = (leftToken.tag == Lexer::Tag::INT_LITERAL) ? (0) : (2);
            std::from_chars(leftLiteral.data() + leftOffset, leftLiteral.data() + leftLiteral.size(), leftValue);

            const Lexer::Token& rightToken = static_cast<const Parser::AST::Expr::Literal*>(castedRight->size.get())->literal;
            const std::string_view rightLiteral = rightToken.content;
            Uint rightValue = 0;
            Uint rightOffset = (rightToken.tag == Lexer::Tag::INT_LITERAL) ? (0) : (2);
            std::from_chars(rightLiteral.data() + rightOffset, rightLiteral.data() + rightLiteral.size(), rightValue);
            if (leftValue != rightValue) return false;

            return Parser::AST::Type::Base::compare(castedLeft->getType(), castedRight->getType());
        }

        case Parser::AST::Type::Tag::FUNC:
        {
            const Parser::AST::Type::Func* castedLeft = static_cast<const Parser::AST::Type::Func*>(newLeft);
            const Parser::AST::Type::Func* castedRight = static_cast<const Parser::AST::Type::Func*>(newRight);

            if (not Parser::AST::Type::Base::compare(castedLeft->returnType.get(), castedRight->returnType.get())) return false;
            if (castedLeft->params.size() != castedRight->params.size()) return false;

            for (Uint i = 0; i < castedLeft->params.size(); i++)
            {
                if (not Parser::AST::Type::Base::compare(castedLeft->params[i].get(), castedRight->params[i].get())) return false;
            }

            return true;
        }

        // Structs are only the same if they came from the same struct declaration (And evantually type data in the declaration).
        case Parser::AST::Type::Tag::STRUCT:
            return newLeft == newRight; // Memory location comparsion. Ooo scary.
    }

    Assert_Message(ASSERT_ALWAYS, "Unknown tag");
    return false; // Shut the compiler up.
}

bool Parser::AST::Type::Base::isBinaryValid(std::string_view opr) const
{
    if (this == nullptr) return false;

    switch (this->tag)
    {
    case Parser::AST::Type::Tag::IDENTIFIER:
    {
        return Parser::AST::Type::Base::getUnderlyingType(this)->isBinaryValid(opr);
    }
    case Parser::AST::Type::Tag::TYPEDEF:
    {
        return Parser::AST::Type::Base::getUnderlyingType(this)->isBinaryValid(opr);
    }

    case Parser::AST::Type::Tag::PRIMITIVE:
    {
        auto castedType = static_cast<const Parser::AST::Type::Primitive*>(this);

        // All operations.
        // static constexpr auto validOperations = std::to_array<std::string_view>(
        // {
        //     "or", "and", "|", "^", "&", "<", ">", "<=", ">=", "==", "!=", "is", "<<", ">>", "+", "-", "*", "/", "%"
        // });
        if (castedType->name == "char")
        {
            static constexpr auto validOperations = std::to_array<std::string_view>(
            {
                "|", "^", "&", "<", ">", "<=", ">=", "==", "!=", "is", "<<", ">>", "+", "-", "*", "/", "%"
            });

            return std::find(validOperations.begin(), validOperations.end(), opr) != validOperations.end();
        }
        if (castedType->name == "bool")
        {
            static constexpr auto validOperations = std::to_array<std::string_view>(
            {
                "or", "and", "==", "!=", "is"
            });

            return std::find(validOperations.begin(), validOperations.end(), opr) != validOperations.end();
        }
        if (castedType->name == "int")
        {
            static constexpr auto validOperations = std::to_array<std::string_view>(
            {
               "|", "^", "&", "<", ">", "<=", ">=", "==", "!=", "is", "<<", ">>", "+", "-", "*", "/", "%"
            });

            return std::find(validOperations.begin(), validOperations.end(), opr) != validOperations.end();
        }
        if (castedType->name == "float")
        {
            static constexpr auto validOperations = std::to_array<std::string_view>(
            {
                "<", ">", "<=", ">=", "==", "!=", "is", "+", "-", "*", "/"
            });

            return std::find(validOperations.begin(), validOperations.end(), opr) != validOperations.end();
        }
    }

    case Parser::AST::Type::Tag::PTR:
    {
        static constexpr auto validOperations = std::to_array<std::string_view>(
        {
            "==", "!=", "is", "<", ">", "<=", ">=", "+", "-"
        });

        return std::find(validOperations.begin(), validOperations.end(), opr) != validOperations.end();
    }

    case Parser::AST::Type::Tag::ARR:
    case Parser::AST::Type::Tag::FUNC:
    case Parser::AST::Type::Tag::STRUCT:
        return false;
    }

    Assert_Message(ASSERT_ALWAYS, "Unknown type");
    return false;
}

bool Parser::AST::Type::Base::isUnaryValid(std::string_view opr) const
{
    if (this == nullptr) return false;

    switch (this->tag)
    {
    case Parser::AST::Type::Tag::IDENTIFIER:
    case Parser::AST::Type::Tag::TYPEDEF:
        return Parser::AST::Type::Base::getUnderlyingType(this)->isUnaryValid(opr);

    case Parser::AST::Type::Tag::PRIMITIVE:
    {
        auto castedType = static_cast<const Parser::AST::Type::Primitive*>(this);

        if (castedType->name == "char")
        {
            static constexpr auto unaries = std::to_array<std::string_view>(
            {
                "+", "-", "~"
            });

            return std::find(unaries.begin(), unaries.end(), opr) != unaries.end();
        }
        if (castedType->name == "bool")
        {
            static constexpr auto unaries = std::to_array<std::string_view>(
            {
                "not"
            });

            return std::find(unaries.begin(), unaries.end(), opr) != unaries.end();
        }
        if (castedType->name == "int")
        {
            static constexpr auto unaries = std::to_array<std::string_view>(
            {
                "+", "-", "~"
            });

            return std::find(unaries.begin(), unaries.end(), opr) != unaries.end();
        }
        if (castedType->name == "float")
        {
            static constexpr auto unaries = std::to_array<std::string_view>(
            {
                "+", "-"
            });

            return std::find(unaries.begin(), unaries.end(), opr) != unaries.end();
        }
    }

    case Parser::AST::Type::Tag::PTR:
    {
        auto castedType = static_cast<const Parser::AST::Type::Ptr*>(this);

        static constexpr auto unaries = std::to_array<std::string_view>(
        {
            "dref"
        });

        bool isPtr = std::find(unaries.begin(), unaries.end(), opr) != unaries.end();
        return isPtr and (castedType->getTo() != nullptr);
    }

    case Parser::AST::Type::Tag::FUNC:
    {
        auto castedType = static_cast<const Parser::AST::Type::Func*>(this);
        return castedType->returnType->Parser::AST::Type::Base::isUnaryValid(opr);
    }

    case Parser::AST::Type::Tag::ARR:
    case Parser::AST::Type::Tag::STRUCT:
        return false;
    }

    Assert_Message(ASSERT_ALWAYS, "Unknown type");
    return false;
}

const Parser::AST::Type::Base* Parser::AST::Type::Base::getUnderlyingType(const Parser::AST::Type::Base* origin)
{
    if (not origin) return nullptr;

    switch (origin->tag)
    {
        case Parser::AST::Type::Tag::IDENTIFIER:
        {
            const Parser::AST::Type::Identifier* castedType = static_cast<const Parser::AST::Type::Identifier*>(origin);
            return Parser::AST::Type::Base::getUnderlyingType(castedType->origin->getType());
        }
        case Parser::AST::Type::Tag::TYPEDEF:
        {
            const Parser::AST::Type::Typedef* castedType = static_cast<const Parser::AST::Type::Typedef*>(origin);
            return Parser::AST::Type::Base::getUnderlyingType(castedType->from.get());
        }

        case Parser::AST::Type::Tag::STRUCT:
        case Parser::AST::Type::Tag::FUNC:
        case Parser::AST::Type::Tag::ARR:
        case Parser::AST::Type::Tag::PTR:
        case Parser::AST::Type::Tag::PRIMITIVE:
            return origin;   
    }

    Assert_Message(ASSERT_ALWAYS, "Unknown tag");
    return nullptr; // Shut the compiler up.
}

bool Parser::AST::Type::Base::isCastable(const Parser::AST::Type::Base* left, const Parser::AST::Type::Base* right)
{
    if (not left and not right) return true;
    if (not left or not right) return false;
    if (Parser::AST::Type::Base::compare(left, right)) return true;
    
    auto leftDeep = Parser::AST::Type::Base::getUnderlyingType(left);
    auto rightDeep = Parser::AST::Type::Base::getUnderlyingType(right);

    if (left->tag == Parser::AST::Type::Tag::PRIMITIVE and right->tag == Parser::AST::Type::Tag::PRIMITIVE)
    {
        // Int to anything int type allowed.
        // int to float and reversed must be between only float and int.
        auto leftP = static_cast<const Parser::AST::Type::Primitive*>(left);
        auto rightP = static_cast<const Parser::AST::Type::Primitive*>(right);
        Uint leftSize = 0;
        Uint rightSize = 0;
        bool isLeftFloat = leftP->name == "float";
        bool isRightFloat = rightP->name == "float";

        if (leftP->name == "bool" or leftP->name == "char") leftSize = 1;
        else leftSize = 4;
        if (rightP->name == "bool" or rightP->name == "char") rightSize = 1;
        else rightSize = 4;

        if ((isLeftFloat and rightSize == 1) or
            (leftSize == 1 and isRightFloat)) return false;
        return true;
    }
    if (left->tag == Parser::AST::Type::Tag::PTR and right->tag == Parser::AST::Type::Tag::PTR)
    {
        return true;
    }
    // Is one of the types is an int and the other is an pointer.
    if ((left->tag == Parser::AST::Type::Tag::PRIMITIVE and 
         static_cast<const Parser::AST::Type::Primitive*>(left)->name == "int" and
         right->tag == Parser::AST::Type::Tag::PTR) or
        (right->tag == Parser::AST::Type::Tag::PRIMITIVE and
            static_cast<const Parser::AST::Type::Primitive*>(right)->name == "int" and
            left->tag == Parser::AST::Type::Tag::PTR))
    {
        return true;
    }

    return false;
}
