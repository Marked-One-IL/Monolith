#include <Parser/SubParser/Type.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <Parser/AST/Type/Primitive.hpp>
#include <Parser/AST/Type/Typedef.hpp>
#include <Parser/AST/Type/Ptr.hpp>
#include <Parser/AST/Type/Arr.hpp>
#include <Parser/AST/Type/Struct.hpp>
#include <Parser/AST/Type/Func.hpp>
#include <Parser/AST/Expr/Literal.hpp>
#include <Parser/SubParser/Expr.hpp>
#include <Lexer/Token.hpp>
#include <Helper/Assert.hpp>
#include <Helper/Types.hpp>
#include <array>
#include <algorithm>

// God let this be the last refactor.

Parser::SubParser::Type::Return Parser::SubParser::Type::parse(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    try
    {
        if (startOff > endOff) throw Parser::SubParser::Type::Error("There is no type", startOff);
        auto ret = parseGeneral(lexer, startOff, endOff);
        if (not ret.type.get()) throw Parser::SubParser::Type::Error("There is no type", startOff);
        return ret;
    }
    catch (const Parser::SubParser::Type::Error& error)
    {
        throw std::runtime_error(std::format("Type Parser:\nAt file: {}\nLine: {}\nError: {}\n{}",
            lexer[error.tokPos].locationData.filename, lexer[error.tokPos].locationData.lineNum, error.error,
            lexer[error.tokPos].locationData.line));
    }
}

Parser::SubParser::Type::Error::Error(const char* new_error, Uint new_tokPos) :
    error(new_error), tokPos(new_tokPos)
{
}

Parser::SubParser::Type::Return Parser::SubParser::Type::parseGeneral(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    if (startOff > endOff) return Parser::SubParser::Type::Return(nullptr, endOff);

    Parser::SubParser::Type::Return ret(nullptr, startOff);
    if (ret = Parser::SubParser::Type::extractPrimitive(lexer, startOff, endOff)) return ret;
    if (ret = Parser::SubParser::Type::extractIdentifier(lexer, startOff, endOff)) return ret;
    if (ret = Parser::SubParser::Type::extractPointer(lexer, startOff, endOff)) return ret;
    if (ret = Parser::SubParser::Type::extractArray(lexer, startOff, endOff)) return ret;
    if (ret = Parser::SubParser::Type::extractFunction(lexer, startOff, endOff)) return ret;
    
    return Parser::SubParser::Type::Return(nullptr, endOff);
}
Parser::SubParser::Type::Return Parser::SubParser::Type::extractPrimitive(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    if (startOff > endOff) return Parser::SubParser::Type::Return(nullptr, endOff);

    static constexpr auto primitiveTypes = std::to_array<std::string_view>(
    {
        "bool", "char",
        "int", "float"
    });
    if (lexer[startOff].tag == Lexer::Tag::KEYWORD and std::find(primitiveTypes.begin(), primitiveTypes.end(), lexer[startOff].content) != primitiveTypes.end())
    {
        return Parser::SubParser::Type::Return(std::make_unique<const Parser::AST::Type::Primitive>(lexer[startOff].content), startOff);
    }

    return Parser::SubParser::Type::Return(nullptr, endOff);
}
Parser::SubParser::Type::Return Parser::SubParser::Type::extractIdentifier(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    if (startOff > endOff) return Parser::SubParser::Type::Return(nullptr, endOff);

    if (lexer[startOff].tag == Lexer::Tag::IDENTIFIER)
    {
        return Parser::SubParser::Type::Return(std::make_unique<const Parser::AST::Type::Identifier>(lexer[startOff].content), startOff);
    }

    return Parser::SubParser::Type::Return(nullptr, endOff);
}
Parser::SubParser::Type::Return Parser::SubParser::Type::extractPointer(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    if (startOff > endOff) return Parser::SubParser::Type::Return(nullptr, endOff);

    if (lexer[startOff].tag == Lexer::Tag::KEYWORD and lexer[startOff].content == "ptr")
    {
        Parser::SubParser::Type::Return internal = Parser::SubParser::Type::parseGeneral(lexer, startOff + 1, endOff);
        return Parser::SubParser::Type::Return(std::make_unique<const Parser::AST::Type::Ptr>(std::move(internal.type)), internal.endOff);
    }

    return Parser::SubParser::Type::Return(nullptr, endOff);
}
Parser::SubParser::Type::Return Parser::SubParser::Type::extractArray(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    if (startOff > endOff) return Parser::SubParser::Type::Return(nullptr, endOff);

    if (lexer[startOff].tag == Lexer::Tag::SYMBOL and lexer[startOff].content == "[")
    {
        Uint depthBracket = 0;
        Uint depthParentheses = 0;
        Uint depthBrace = 0;
        Uint bracketPos = NPOS;
        for (Uint i = startOff + 1; i <= endOff; i++)
        {
            const Lexer::Token& token = lexer[i];

            // Depth tracker.
            if (token.tag == Lexer::Tag::SYMBOL and token.content == "[") depthBracket++;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == "]" and depthBracket > 0) depthBracket--;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == "(") depthParentheses++;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == ")" and depthParentheses > 0) depthParentheses--;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == "{") depthBrace++;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == "}" and depthBrace > 0) depthBrace--;

            // Actual finding.
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == "]" and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0 and bracketPos == NPOS) bracketPos = i;
        }
        if (bracketPos == NPOS) throw Parser::SubParser::Type::Error("Could not find a closing ']'", startOff);

        Parser::SubParser::Type::Return arrType = Parser::SubParser::Type::parseGeneral(lexer, startOff + 1, endOff);
        const Uint colonPos = arrType.endOff + 1;
        if (colonPos > endOff or lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":") 
            throw Parser::SubParser::Type::Error("Could not find a closing ':' separator", startOff);

        const Uint sizeStart = colonPos + 1;
        const Uint sizeEnd = bracketPos - 1;
        if (colonPos + 1 >= bracketPos) throw Parser::SubParser::Type::Error("Array size expression is empty", startOff);

        std::unique_ptr<const Parser::AST::Expr::Base> expr = Parser::SubParser::Expr::parse(lexer, colonPos + 1, bracketPos - 1);
        if (expr->tag != Parser::AST::Tag::EXPR_LITERAL) throw Parser::SubParser::Type::Error("Array size must be a literal", startOff);

        const Lexer::Token& literal = static_cast<const Parser::AST::Expr::Literal*>(expr.get())->literal;
        if (not literal.isInt()) throw Parser::SubParser::Type::Error("Array size must be an int literal", startOff);

        Uint size = 0;
        Uint offset = (literal.tag == Lexer::Tag::INT_LITERAL) ? (0) : (2);
        std::string_view view = literal.content;
        std::from_chars(view.data() + offset, view.data() + view.size(), size);
        if (size == 0) throw Parser::SubParser::Type::Error("Array size cannot be 0", startOff);

        return Parser::SubParser::Type::Return(std::make_unique<const Parser::AST::Type::Arr>(std::move(arrType.type), std::move(expr)), bracketPos);
    }

    return Parser::SubParser::Type::Return(nullptr, endOff);
}
Parser::SubParser::Type::Return Parser::SubParser::Type::extractFunction(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    if (startOff > endOff) return Parser::SubParser::Type::Return(nullptr, endOff);

    if (lexer[startOff].tag == Lexer::Tag::SYMBOL and lexer[startOff].content == "(")
    {
        // Getting end parentheses.
        Uint parenthesisPos = NPOS;
        Uint depthBracket = 0;
        Uint depthParentheses = 1;
        for (Uint i = startOff + 1; i <= endOff; i++)
        {
            const Lexer::Token& token = lexer[i];

            // Depth tracker.
            if (token.tag == Lexer::Tag::SYMBOL and token.content == "[") depthBracket++;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == "]" and depthBracket > 0) depthBracket--;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == "(") depthParentheses++;
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == ")" and depthParentheses > 1) depthParentheses--;

            // Actual finding.
            else if (token.tag == Lexer::Tag::SYMBOL and token.content == ")" and depthBracket == 0 and depthParentheses <= 1 and parenthesisPos == NPOS)
            {
                parenthesisPos = i;
                break;
            }
        }

        // Getting return type.
        Parser::SubParser::Type::Return returnType(nullptr, parenthesisPos);
        Uint arrowPos = NPOS;
        if (parenthesisPos == NPOS) throw Parser::SubParser::Type::Error("Could not have found an ending ')'", startOff);
        if (parenthesisPos + 1 <= endOff and lexer[parenthesisPos + 1].tag == Lexer::Tag::SYMBOL and lexer[parenthesisPos + 1].content == "->")
        {
            arrowPos = parenthesisPos + 1;
            if (arrowPos + 1 > endOff) throw Parser::SubParser::Type::Error("Return type without a type", startOff);
            returnType = Parser::SubParser::Type::parseGeneral(lexer, arrowPos + 1, endOff);
        }

        // Extracting parameters.
        std::vector<std::unique_ptr<const Parser::AST::Type::Base>> params;
        Parser::SubParser::Type::Return currentParameter(nullptr, startOff + 1);
        bool isEllipsis = false;
        while (currentParameter.endOff < parenthesisPos)
        {
            if (lexer[currentParameter.endOff].tag == Lexer::Tag::SYMBOL and lexer[currentParameter].content == "...")
            {
                if (currentParameter.endOff + 1 >= parenthesisPos)
                {
                    isEllipsis = true;
                    break;
                }
                throw Parser::SubParser::Type::Error("'...' must be at the end of the function parameter type", startOff);
            }

            currentParameter = Parser::SubParser::Type::parseGeneral(lexer, currentParameter.endOff, parenthesisPos - 1);
            params.emplace_back(std::move(currentParameter.type));
            const Uint commaPos = currentParameter.endOff + 1;
            if ((commaPos < parenthesisPos) and (lexer[commaPos].tag != Lexer::Tag::SYMBOL or lexer[commaPos].content != ","))
                throw Parser::SubParser::Type::Error("Each parameter must be separated by a ','", startOff);
            currentParameter.endOff = commaPos + 1;
        }

        return Parser::SubParser::Type::Return(std::make_unique<const Parser::AST::Type::Func>(std::move(returnType.type), std::move(params), isEllipsis), returnType.endOff);
    }

    return Parser::SubParser::Type::Return(nullptr, endOff);
}

Parser::SubParser::Type::Return::Return(std::unique_ptr<const Parser::AST::Type::Base> new_type, Uint new_endOff) :
    type(std::move(new_type)), endOff(new_endOff)
{
}
Parser::SubParser::Type::Return::Return(Parser::SubParser::Type::Return&& other) :
    type(std::move(other.type)), endOff(other.endOff)
{
}
Parser::SubParser::Type::Return& Parser::SubParser::Type::Return::operator = (Parser::SubParser::Type::Return&& other)
{
    this->type = std::move(other.type); 
    this->endOff = other.endOff;
    return *this;
}
Parser::SubParser::Type::Return::operator bool(void) const
{
    return static_cast<bool>(this->type.get());
}
