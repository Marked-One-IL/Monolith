#include <Parser/SubParser/Expr.hpp>
#include <Parser/SubParser/Type.hpp>
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
#include <Helper/Assert.hpp>
#include <array>
#include <string_view>
#include <algorithm>

std::unique_ptr<const Parser::AST::Expr::Base> Parser::SubParser::Expr::parse(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    try
    {
        if (startOff > endOff) throw Parser::SubParser::Expr::Error("There is no expression", lexer.size() - 1);
        Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::parseBase(lexer, startOff, endOff, Parser::SubParser::Expr::Level::NONE);
        if (ret.endOff != endOff) throw Parser::SubParser::Expr::Error("Unexpected token after expression", lexer.size() - 1); // Paranoid check.
        if (not ret) throw Parser::SubParser::Expr::Error("There is no expression", lexer.size() - 1);
        return std::move(ret.expr);
    }
    catch (const Parser::SubParser::Expr::Error& error)
    {
        Assert(error.tokPos < lexer.size());

        throw std::runtime_error(std::format("Expression Parser:\nAt file: {}\nLine: {}\nError: {}\n{}",
            lexer[error.tokPos].locationData.filename, lexer[error.tokPos].locationData.lineNum, error.error,
            lexer[error.tokPos].locationData.line));
    }
}
Parser::SubParser::Expr::Return Parser::SubParser::Expr::parseEndOff(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    try
    {
        if (startOff > endOff) throw Parser::SubParser::Expr::Error("There is no expression", lexer.size() - 1);
        Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::parseBase(lexer, startOff, endOff, Parser::SubParser::Expr::Level::NONE);
        if (ret.endOff != endOff) throw Parser::SubParser::Expr::Error("Unexpected token after expression", lexer.size() - 1); // Paranoid check.
        if (not ret) throw Parser::SubParser::Expr::Error("There is no expression", lexer.size() - 1);
        return ret;
    }
    catch (const Parser::SubParser::Expr::Error& error)
    {
        Assert(error.tokPos < lexer.size());

        throw std::runtime_error(std::format("Expression Parser:\nAt file: {}\nLine: {}\nError: {}\n{}",
            lexer[error.tokPos].locationData.filename, lexer[error.tokPos].locationData.lineNum, error.error,
            lexer[error.tokPos].locationData.line));
    }
}

Parser::SubParser::Expr::Return::Return(std::unique_ptr<const Parser::AST::Expr::Base> new_expr, Uint new_endOff) : expr(std::move(new_expr)), endOff(new_endOff)
{
}
Parser::SubParser::Expr::Return::Return(Parser::SubParser::Expr::Return&& other) : 
    expr(std::move(other.expr)), endOff(other.endOff)
{
}
Parser::SubParser::Expr::Return& Parser::SubParser::Expr::Return::operator = (Parser::SubParser::Expr::Return&& other)
{
    this->expr = std::move(other.expr);
    this->endOff = other.endOff;
    return *this;
}
Parser::SubParser::Expr::Return::operator bool(void) const
{
    return static_cast<bool>(this->expr.get());
}
Parser::SubParser::Expr::Error::Error(const char* new_error, Uint new_tokPos) :
    error(new_error), tokPos(new_tokPos)
{
}

Parser::SubParser::Expr::Return Parser::SubParser::Expr::parseBase(const Lexer::Generator& lexer, Uint startOff, Uint endOff, Parser::SubParser::Expr::Level prevLevel)
{
    if (startOff > endOff) return Parser::SubParser::Expr::Return(nullptr, endOff);

    Parser::SubParser::Expr::Return left = Parser::SubParser::Expr::handleSingle(lexer, startOff, endOff);
    if (not left) throw Parser::SubParser::Expr::Error("Unknown expression", startOff);

    startOff = left.endOff + 1;
    while (startOff <= endOff)
    {
        const Lexer::Token& opToken = lexer[startOff];
        Uint opTokenPos = startOff;

        if (opToken.tag == Lexer::Tag::SYMBOL and opToken.content == "as")
        {
            if (startOff + 1 > endOff) throw Parser::SubParser::Expr::Error("There is no type", startOff);

            Parser::SubParser::Type::Return ret = Parser::SubParser::Type::parse(lexer, startOff + 1, endOff);

            left.expr = std::make_unique<const Parser::AST::Expr::Cast>(std::move(left.expr), std::move(ret.type), lexer[startOff].locationData);
            left.endOff = ret.endOff;
            startOff = ret.endOff + 1;
            continue;
        }

        Parser::SubParser::Expr::Level currLevel = Parser::SubParser::Expr::getOperatorLevel(lexer, opTokenPos);
        if (currLevel <= prevLevel) break;

        Parser::SubParser::Expr::Return right = parseBase(lexer, startOff + 1, endOff, currLevel);
        if (not right) throw Parser::SubParser::Expr::Error("Incomplete binary operation", startOff);
        left.expr = std::make_unique<const Parser::AST::Expr::Bin>(std::move(left.expr), std::move(right.expr), opToken.content, lexer[startOff].locationData);
        left.endOff = right.endOff;
        startOff = right.endOff + 1;
    }

    return std::move(left);
}
Parser::SubParser::Expr::Level Parser::SubParser::Expr::getOperatorLevel(const Lexer::Generator& lexer, Uint tokPos)
{
    static constexpr auto level1 = std::to_array<std::string_view>(
    {
        "or"
    });
    static constexpr auto level2 = std::to_array<std::string_view>(
    {
        "and"
    });
    static constexpr auto level3 = std::to_array<std::string_view>(
    {
        "|"
    });
    static constexpr auto level4 = std::to_array<std::string_view>(
    {
        "^"
    });
    static constexpr auto level5 = std::to_array<std::string_view>(
    {
        "&"
    });
    static constexpr auto level6 = std::to_array<std::string_view>(
    {
        "<", ">", "<=", ">=", "==", "!=", "is"
    });
    static constexpr auto level7 = std::to_array<std::string_view>(
    {
        "<<", ">>"
    });
    static constexpr auto level8 = std::to_array<std::string_view>(
    {
        "+", "-"
    });
    static constexpr auto level9 = std::to_array<std::string_view>(
    {
        "*", "/", "%"
    });
    
    const Lexer::Token& token = lexer[tokPos];

    if (std::find(level1.begin(), level1.end(), token.content) != level1.end()) return Parser::SubParser::Expr::Level::LEVEL1;
    if (std::find(level2.begin(), level2.end(), token.content) != level2.end()) return Parser::SubParser::Expr::Level::LEVEL2;
    if (std::find(level3.begin(), level3.end(), token.content) != level3.end()) return Parser::SubParser::Expr::Level::LEVEL3;
    if (std::find(level4.begin(), level4.end(), token.content) != level4.end()) return Parser::SubParser::Expr::Level::LEVEL4;
    if (std::find(level5.begin(), level5.end(), token.content) != level5.end()) return Parser::SubParser::Expr::Level::LEVEL5;
    if (std::find(level6.begin(), level6.end(), token.content) != level6.end()) return Parser::SubParser::Expr::Level::LEVEL6;
    if (std::find(level7.begin(), level7.end(), token.content) != level7.end()) return Parser::SubParser::Expr::Level::LEVEL7;
    if (std::find(level8.begin(), level8.end(), token.content) != level8.end()) return Parser::SubParser::Expr::Level::LEVEL8;
    if (std::find(level9.begin(), level9.end(), token.content) != level9.end()) return Parser::SubParser::Expr::Level::LEVEL9;

    throw Parser::SubParser::Expr::Error("Invalid binary operation", tokPos);
}

Parser::SubParser::Expr::Return Parser::SubParser::Expr::handleSingle(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    Parser::SubParser::Expr::Return single(nullptr, endOff);
    if (Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::handleSizeof(lexer, startOff, endOff)) single = std::move(ret);
    else if (Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::handleSubExpr(lexer, startOff, endOff)) single = std::move(ret);
    else if (Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::handleUnary(lexer, startOff, endOff)) single = std::move(ret);
    else if (Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::handleValue(lexer, startOff, endOff)) single = std::move(ret);

    if (not single) throw Parser::SubParser::Expr::Error("Invalid expression", startOff);
    return std::move(single);
}
Parser::SubParser::Expr::Return Parser::SubParser::Expr::handleSubExpr(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    const Lexer::Token& token = lexer[startOff];

    if (token.tag == Lexer::Tag::SYMBOL and token.content == "(")
    {
        const Uint startParenthesesPos = startOff;
        const Uint start = startOff + 1;

        Uint depthBracket = 0;
        Uint depthParentheses = 0;
        Uint depthBrace = 0;
        Uint endParentesisPos = NPOS;
        for (Uint i = start; i <= endOff; i++)
        {
            const Lexer::Token& tok = lexer[i];

            if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "[") depthBracket++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket > 0) depthBracket--;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "(") depthParentheses++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthParentheses > 0) depthParentheses--;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "{") depthBrace++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "}" and depthBrace > 0) depthBrace--;
            
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
            {
                endParentesisPos = i;
                break;
            }
        }
        if (endParentesisPos == NPOS) throw Parser::SubParser::Expr::Error("Expected closing parentheses is missing", startOff);
        if (endParentesisPos - startParenthesesPos <= 1) throw Parser::SubParser::Expr::Error("Empty parentheses expression", startOff);

        Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::parseBase(lexer, start, endParentesisPos - 1, Parser::SubParser::Expr::Level::NONE);
        return Parser::SubParser::Expr::Return(std::move(ret.expr), endParentesisPos);
    }

    return Parser::SubParser::Expr::Return(nullptr, endOff);
}
Parser::SubParser::Expr::Return Parser::SubParser::Expr::handleValue(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    static constexpr auto operations = std::to_array<std::string_view>(
    {
        ".", "->", "(", "["
    });

    const Lexer::Token& token = lexer[startOff];
    bool isNextTokenBrace = ((startOff + 1) <= endOff) and lexer[startOff + 1].tag == Lexer::Tag::SYMBOL and lexer[startOff + 1].content == "{";

    bool isLiteral = token.tag >= Lexer::Tag::STRING3_LITERAL and token.tag <= Lexer::Tag::NONE_LITERAL;
    bool isIdentifier = token.tag == Lexer::Tag::IDENTIFIER and not isNextTokenBrace;
    bool isStructLiteral = token.tag == Lexer::Tag::IDENTIFIER and isNextTokenBrace;
    bool isArrLiteral = token.tag == Lexer::Tag::SYMBOL and token.content == "[";

    if (not isLiteral and not isIdentifier and not isArrLiteral and not isStructLiteral) return Parser::SubParser::Expr::Return(nullptr, endOff);

    std::unique_ptr<const Parser::AST::Expr::Base> currExpr;
    if (isLiteral) currExpr = std::make_unique<const Parser::AST::Expr::Literal>(token, lexer[startOff].locationData);
    else if (isIdentifier) currExpr = std::make_unique<const Parser::AST::Expr::Identifier>(token.content, lexer[startOff].locationData);
    else if (isStructLiteral)
    {
        const Uint start = startOff + 2;
        const std::string_view name = token.content;

        Uint depthBracket = 0;
        Uint depthParentheses = 0;
        Uint depthBrace = 0;
        Uint endBracePos = NPOS;
        std::vector<Uint> commaPositions;
        for (Uint i = start; i <= endOff; i++)
        {
            const Lexer::Token& tok = lexer[i];

            if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "[") depthBracket++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket > 0) depthBracket--;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "(") depthParentheses++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthParentheses > 0) depthParentheses--;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "{") depthBrace++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "}" and depthBrace > 0) depthBrace--;

            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "}" and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0 and endBracePos == NPOS)
            {
                endBracePos = i;
                break;
            }
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "," and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
                commaPositions.push_back(i);
        }
        if (endBracePos == NPOS) throw Parser::SubParser::Expr::Error("Expected closing brace is missing", startOff);

        std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> fields;
        Uint paramStart = start;
        for (Uint comma : commaPositions)
        {
            Uint paramEnd = comma - 1;

            if (paramStart > paramEnd) throw Parser::SubParser::Expr::Error("Field with no expression", startOff);
            fields.push_back(std::move(Parser::SubParser::Expr::parseBase(lexer, paramStart, paramEnd, Parser::SubParser::Expr::Level::NONE).expr));

            paramStart = comma + 1;
        }
        // Last param.
        if (paramStart >= endBracePos and not commaPositions.empty()) throw Parser::SubParser::Expr::Error("Field with no expression", startOff);
        if (paramStart < endBracePos) fields.push_back(std::move(Parser::SubParser::Expr::parseBase(lexer, paramStart, endBracePos - 1, Parser::SubParser::Expr::Level::NONE).expr));
        if (fields.size() == 0) throw Parser::SubParser::Expr::Error("No Field for an struct literal", startOff);

        currExpr = std::make_unique<const Parser::AST::Expr::StructInit>(std::make_unique<const Parser::AST::Type::Identifier>(name), std::move(fields), lexer[startOff].locationData);
        startOff = endBracePos;
    }
    else if (isArrLiteral)
    {
        const Uint start = startOff + 1;

        Uint depthBracket = 0;
        Uint depthParentheses = 0;
        Uint depthBrace = 0;
        Uint endBracketPos = NPOS;
        std::vector<Uint> commaPositions;
        for (Uint i = start; i <= endOff; i++)
        {
            const Lexer::Token& tok = lexer[i];

            if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "[") depthBracket++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket > 0) depthBracket--;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "(") depthParentheses++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthParentheses > 0) depthParentheses--;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "{") depthBrace++;
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "}" and depthBrace > 0) depthBrace--;

            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
            {
                endBracketPos = i;
                break;
            }
            else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "," and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
                commaPositions.push_back(i);
        }
        if (endBracketPos == NPOS) throw Parser::SubParser::Expr::Error("Expected closing bracket is missing", startOff);

        std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> elements;
        Uint paramStart = start;
        for (Uint comma : commaPositions)
        {
            Uint paramEnd = comma - 1;

            if (paramStart > paramEnd) throw Parser::SubParser::Expr::Error("Element with no expression", startOff);
            elements.push_back(std::move(Parser::SubParser::Expr::parseBase(lexer, paramStart, paramEnd, Parser::SubParser::Expr::Level::NONE).expr));

            paramStart = comma + 1;
        }
        // Last param.
        if (paramStart >= endBracketPos and not commaPositions.empty()) throw Parser::SubParser::Expr::Error("Element with no expression", startOff);
        if (paramStart < endBracketPos) elements.push_back(std::move(Parser::SubParser::Expr::parseBase(lexer, paramStart, endBracketPos - 1, Parser::SubParser::Expr::Level::NONE).expr));
        if (elements.size() == 0) throw Parser::SubParser::Expr::Error("No elements for an array literal", startOff);

        currExpr = std::make_unique<const Parser::AST::Expr::ArrInit>(std::move(elements), lexer[startOff].locationData);
        startOff = endBracketPos;
    }

    // Handles chaining values (eg: x.y()[i])
    while (startOff + 1 <= endOff and
           lexer[startOff + 1].tag == Lexer::Tag::SYMBOL and
           std::find(operations.begin(), operations.end(), lexer[startOff + 1].content) != operations.end())
    {
        if (isLiteral) throw Parser::SubParser::Expr::Error("Literal is treated as an identifier", startOff);
        if (isArrLiteral) throw Parser::SubParser::Expr::Error("Array literal cannot be used but assigned", startOff);
        if (isStructLiteral) throw Parser::SubParser::Expr::Error("Struct literal cannot be used but assigned", startOff);

        startOff++;
        if (startOff + 1 > endOff) throw Parser::SubParser::Expr::Error("Incomplete expression", startOff);

        const Lexer::Token& currToken = lexer[startOff];
        const Lexer::Token& nextToken = lexer[startOff + 1];

        if (currToken.content == ".")
        {
            if (nextToken.tag != Lexer::Tag::IDENTIFIER) throw Parser::SubParser::Expr::Error("Member access must an identifier", startOff);
            currExpr = std::make_unique<const Parser::AST::Expr::StructAccess>(std::move(currExpr), nextToken.content, lexer[startOff].locationData);
            startOff++;
        }
        else if (currToken.content == "->")
        {
            if (nextToken.tag != Lexer::Tag::IDENTIFIER) throw Parser::SubParser::Expr::Error("Member access must an identifier", startOff);
            currExpr = std::make_unique<const Parser::AST::Expr::StructAccess>(std::make_unique<const Parser::AST::Expr::Unary>("dref",
                                                                               std::move(currExpr), lexer[startOff].locationData), nextToken.content, lexer[startOff].locationData);
            startOff++;
        }
        else if (currToken.content == "(")
        {
            const Uint start = startOff + 1;

            Uint depthBracket = 0;
            Uint depthParentheses = 0;
            Uint depthBrace = 0;
            Uint endParentesisPos = NPOS;
            std::vector<Uint> commaPositions;
            for (Uint i = start; i <= endOff; i++)
            {
                const Lexer::Token& tok = lexer[i];

                if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "[") depthBracket++;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket > 0) depthBracket--;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "(") depthParentheses++;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthParentheses > 0) depthParentheses--;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "{") depthBrace++;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "}" and depthBrace > 0) depthBrace--;

                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
                {
                    endParentesisPos = i;
                    break;
                }
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "," and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
                    commaPositions.push_back(i);
            }
            if (endParentesisPos == NPOS) throw Parser::SubParser::Expr::Error("Expected closing parentheses is missing", startOff);
            
            std::vector<std::unique_ptr<const Parser::AST::Expr::Base>> params;
            Uint paramStart = startOff + 1;
            for (Uint comma : commaPositions)
            {
                Uint paramEnd = comma - 1;

                if (paramStart > paramEnd) throw Parser::SubParser::Expr::Error("Parameter with no expression", startOff);
                params.push_back(std::move(Parser::SubParser::Expr::parseBase(lexer, paramStart, paramEnd, Parser::SubParser::Expr::Level::NONE).expr));

                paramStart = comma + 1;
            }
            // Last param.
            if (paramStart >= endParentesisPos and not commaPositions.empty()) throw Parser::SubParser::Expr::Error("Parameter with no type", startOff);
            if (paramStart < endParentesisPos) params.push_back(std::move(Parser::SubParser::Expr::parseBase(lexer, paramStart, endParentesisPos - 1, Parser::SubParser::Expr::Level::NONE).expr));

            currExpr = std::make_unique<const Parser::AST::Expr::Call>(std::move(currExpr), std::move(params), lexer[startOff].locationData);
            startOff = endParentesisPos;
        }
        else if (currToken.content == "[")
        {
            const Uint startBracketPos = startOff;
            const Uint start = startOff + 1;

            Uint depthBracket = 0;
            Uint depthParentheses = 0;
            Uint depthBrace = 0;
            Uint endBracketPos = NPOS;
            for (Uint i = start; i <= endOff; i++)
            {
                const Lexer::Token& tok = lexer[i];

                if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "[") depthBracket++;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket > 0) depthBracket--;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "(") depthParentheses++;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthParentheses > 0) depthParentheses--;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "{") depthBrace++;
                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "}" and depthBrace > 0) depthBrace--;

                else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
                {
                    endBracketPos = i;
                    break;
                }
            }
            if (endBracketPos == NPOS) throw Parser::SubParser::Expr::Error("Expected closing bracket is missing", startOff);
            if (start == endBracketPos) throw Parser::SubParser::Expr::Error("Array access without any expression inside it", startOff);

            std::unique_ptr<const Parser::AST::Expr::Base> indexExpr = std::move(Parser::SubParser::Expr::parseBase(lexer, start, endBracketPos - 1,
                                                                                 Parser::SubParser::Expr::Level::NONE).expr);
            currExpr = std::make_unique<const Parser::AST::Expr::ArrAccess>(std::move(currExpr), std::move(indexExpr), lexer[startOff].locationData);
            startOff = endBracketPos;
        }
    }

    return Parser::SubParser::Expr::Return(std::move(currExpr), startOff);
}
Parser::SubParser::Expr::Return Parser::SubParser::Expr::handleUnary(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    static constexpr auto unaries = std::to_array<std::string_view>(
    {
        "+", "-", "~", "not", "ref", "dref"
    });

    const Lexer::Token& token = lexer[startOff];
    auto it = std::find(unaries.begin(), unaries.end(), token.content);

    if (token.tag != Lexer::Tag::SYMBOL or it == unaries.end()) return Parser::SubParser::Expr::Return(nullptr, endOff);
    if (startOff + 1 > endOff) throw Parser::SubParser::Expr::Error("Operation without anything leading after it", startOff);

    Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::handleSingle(lexer, startOff + 1, endOff);
    if (not ret) throw Parser::SubParser::Expr::Error("Unary is linked to nothing", startOff);
    return Parser::SubParser::Expr::Return(std::make_unique<const Parser::AST::Expr::Unary>(*it, std::move(ret.expr), lexer[startOff].locationData), ret.endOff);
}
Parser::SubParser::Expr::Return Parser::SubParser::Expr::handleSizeof(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    const Lexer::Token& token = lexer[startOff];
    if (token.tag != Lexer::Tag::SYMBOL or token.content != "sizeof") return Parser::SubParser::Expr::Return(nullptr, endOff);

    if (startOff + 1 > endOff) throw Parser::SubParser::Expr::Error("There is no type", startOff);
    Parser::SubParser::Type::Return ret = Parser::SubParser::Type::parse(lexer, startOff + 1, endOff);
    if (not ret.type) throw Parser::SubParser::Expr::Error("There is no type", startOff);
    return Parser::SubParser::Expr::Return(std::make_unique<const Parser::AST::Expr::Sizeof>(std::move(ret.type), lexer[startOff].locationData), ret.endOff);
}
