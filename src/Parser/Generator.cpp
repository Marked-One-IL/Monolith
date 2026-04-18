#include <Parser/Generator.hpp>
#include <Parser/SubParser/Type.hpp>
#include <Parser/SubParser/Expr.hpp>
#include <Helper/Assert.hpp>
#include <Parser/AST/Type/Primitive.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <format>

Parser::Generator::Generator(const Lexer::Generator& lexer)
try
    : m_headBlock(std::move(Parser::Generator::parseBlock(lexer, 0, 0, false).first))
{
    bool foundMain = false;
    
    for (auto& ast : this->m_headBlock.get()->commands)
    {
        if (ast.get()->tag == Parser::AST::Tag::DECL_FUNC)
        {
            auto possibleMainFunc = static_cast<const Parser::AST::Decl::Func*>(ast.get());
            if (possibleMainFunc->name == "main")
            {
                foundMain = true;
                break;
            }
        }
    }
    
    if (not foundMain) throw std::runtime_error(std::format("Parser:\nAt file: {}\nmain function was not found", lexer.getFilename()));
}
catch (const Parser::Generator::Error& error)
{
    Assert(error.tokPos < lexer.size());

    throw std::runtime_error(std::format("Parser:\nAt file: {}\nLine: {}\nError: {}\n{}",
        lexer[error.tokPos].locationData.filename, lexer[error.tokPos].locationData.lineNum, error.error,
        lexer[error.tokPos].locationData.line));
}

const Parser::AST::Block* Parser::Generator::getHeadBlock(void) const
{
    return this->m_headBlock.get();
}

Parser::Generator::Error::Error(const char* new_error, Uint new_tokPos) :
    error(new_error), tokPos(new_tokPos)
{
}


namespace Parser
{
    std::ostream& operator << (std::ostream& stream, const Parser::Generator& parser)
    {
        if (parser.m_headBlock != nullptr and not parser.m_headBlock->commands.empty())
        {
            parser.m_headBlock->print(stream, NPOS); // This will overflow from NPOS to 0. Dirty but works.
        }
        return stream;
    }
}

Uint Parser::Generator::findEndOff(const Lexer::Generator& lexer, Uint startOff)
{
    Uint endOff = NPOS;

    for (Uint i = startOff; i < lexer.size(); i++)
    {
        const Lexer::Token& token = lexer[i];

        if (token.tag == Lexer::Tag::NEW_LINE)
        {
            endOff = i - 1;
            break;
        }
    }
    Assert_Message(endOff != NPOS, "endOff == NPOS");

    return endOff;
}
std::pair<std::unique_ptr<const Parser::AST::Block>, Uint> Parser::Generator::parseBlock(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel, bool exceptIndent)
{
    if (startOff >= lexer.size()) throw Parser::Generator::Error("Block with not content", lexer.size() - 1);
    while ((startOff < lexer.size()) and (lexer[startOff].tag == Lexer::Tag::NEW_LINE)) startOff++;
    if (startOff >= lexer.size()) throw Parser::Generator::Error("Block with not content", lexer.size() - 1);
    const bool isIndent = lexer[startOff].tag == Lexer::Tag::INDENT;
    const bool isDedent = lexer[startOff].tag == Lexer::Tag::DEDENT;
    if (isIndent or isDedent) startOff++;

    if (not exceptIndent and isIndent) throw Parser::Generator::Error("Unexpected indent", startOff);
    if (not exceptIndent and isDedent) throw Parser::Generator::Error("Unexpected dedent", startOff);
    if (exceptIndent and not isIndent) throw Parser::Generator::Error("Expected indent is missing", startOff);
    if (isIndent) indentLevel++;
    if (isDedent)
    {
        Assert_Message(indentLevel > 0, "Dedent is found at level 0");
        indentLevel--;
    }

    std::vector<std::unique_ptr<const Parser::AST::Base>> commands;
    
    Uint i = startOff;
    while (i < lexer.size()) 
    {
        const Lexer::Token& token = lexer[i];

        if (token.tag == Lexer::Tag::INDENT)
        {
            throw Parser::Generator::Error("Unexpected indent", startOff);
        }
        if (token.tag == Lexer::Tag::DEDENT) 
        {
            i++;
            break; 
        }
        if (token.tag == Lexer::Tag::NEW_LINE)
        { 
            i++;
            continue;
        } 

        auto ast = Parser::Generator::parseGeneral(lexer, i, indentLevel);
        commands.push_back(std::move(ast.first));
        i = ast.second;
    }

    return std::make_pair(std::make_unique<const Parser::AST::Block>(std::move(commands)), i);
}
std::pair<std::unique_ptr<const Parser::AST::Base>, Uint> Parser::Generator::parseGeneral(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    {
        auto pair = Parser::Generator::extractIf(lexer, startOff, indentLevel);
        if (pair.first)
        {
            if (startOff < lexer.size() and lexer[startOff].tag == Lexer::Tag::KEYWORD and
                lexer[startOff].content != "if") // If is elif or else.
            {
                throw Parser::Generator::Error("An elif/else statement before an if statement", startOff);
            }
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractWhile(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractFor(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractVar(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractTypedef(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractFunc(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractStruct(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractAssign(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractBreak(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractContinue(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractReturn(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    {
        auto pair = Parser::Generator::extractExpr(lexer, startOff, indentLevel);
        if (pair.first)
        {
            return pair;
        }
    }
    throw Parser::Generator::Error("Unknown command", startOff);
}

std::pair<std::unique_ptr<const Parser::AST::Stmt::If>, Uint> Parser::Generator::extractIf(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    const Lexer::Token& token = lexer[startOff];
    if (token.tag != Lexer::Tag::KEYWORD or (token.content != "if" and token.content != "elif" and token.content != "else")) return std::make_pair(nullptr, startOff);;
    if (indentLevel == 0) throw Parser::Generator::Error("If cannot be in the global scope", startOff);
    bool isIf = token.content == "if";
    bool isElif = token.content == "elif";
    bool isElse = token.content == "else";

    const Uint colonPos = endOff;
    if (lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":") throw Parser::Generator::Error("An if statement does not end with an ending ':'", startOff);
    if (not isElse and startOff + 1 >= colonPos) throw Parser::Generator::Error("If statement without an expression", startOff);

    std::unique_ptr<const Parser::AST::Expr::Base> cond = (not isElse) ? Parser::SubParser::Expr::parse(lexer, startOff + 1, colonPos - 1) : nullptr;
    std::pair<std::unique_ptr<const Parser::AST::Block>, Uint> pair = Parser::Generator::parseBlock(lexer, colonPos + 1, indentLevel, true);
    std::unique_ptr<const Parser::AST::Block> thenBlock = std::move(pair.first);
    std::unique_ptr<const Parser::AST::Stmt::If> elseBlock;

    while (pair.second < lexer.size() and lexer[pair.second].tag == Lexer::Tag::NEW_LINE) pair.second++;
    bool isNextElifOrElse = pair.second < lexer.size() and lexer[pair.second].tag == Lexer::Tag::KEYWORD and (lexer[pair.second].content == "elif" or
                                                          (lexer[pair.second].content == "else" and not isElse));

    if ((isIf or isElif) and isNextElifOrElse)
    {
        auto elsePair = Parser::Generator::extractIf(lexer, pair.second, indentLevel);
        elseBlock = std::move(elsePair.first);
        return std::make_pair(std::make_unique<const Parser::AST::Stmt::If>(std::move(cond), std::move(thenBlock), std::move(elseBlock), lexer[startOff].locationData),
                                                                            elsePair.second);
    }

    return std::make_pair(std::make_unique<const Parser::AST::Stmt::If>(std::move(cond), std::move(thenBlock), std::move(elseBlock), lexer[startOff].locationData), pair.second);
}
std::pair<std::unique_ptr<const Parser::AST::Stmt::While>, Uint> Parser::Generator::extractWhile(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    const Lexer::Token& token = lexer[startOff];
    if (token.tag != Lexer::Tag::KEYWORD or token.content != "while") return std::make_pair(nullptr, startOff);;
    if (indentLevel == 0) throw Parser::Generator::Error("While cannot be in the global scope", startOff);

    const Uint colonPos = endOff;
    if (lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":") throw Parser::Generator::Error("An while statement does not end with an ending ':'", startOff);
    if (startOff + 1 >= colonPos) throw Parser::Generator::Error("While statement without an expression", startOff);

    std::unique_ptr<const Parser::AST::Expr::Base> cond = Parser::SubParser::Expr::parse(lexer, startOff + 1, colonPos - 1);
    std::pair<std::unique_ptr<const Parser::AST::Block>, Uint> pair = Parser::Generator::parseBlock(lexer, colonPos + 1, indentLevel, true);
    std::unique_ptr<const Parser::AST::Block> thenBlock = std::move(pair.first);

    return std::make_pair(std::make_unique<const Parser::AST::Stmt::While>(std::move(cond), std::move(thenBlock), lexer[startOff].locationData), pair.second);
}
std::pair<std::unique_ptr<const Parser::AST::Stmt::For>, Uint> Parser::Generator::extractFor(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    const Lexer::Token& token = lexer[startOff];
    if (token.tag != Lexer::Tag::KEYWORD or token.content != "for") return std::make_pair(nullptr, startOff);;
    if (indentLevel == 0) throw Parser::Generator::Error("For cannot be in the global scope", startOff);

    std::vector<Uint> commaPositions;
    Uint depthBracket = 0;
    Uint depthParentheses = 0;
    Uint depthBrace = 0;
    for (Uint i = startOff + 1; i <= endOff; i++)
    {
        const Lexer::Token& tok = lexer[i];

        // Depth tracker.
        if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "[") depthBracket++;
        else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "]" and depthBracket > 0) depthBracket--;
        else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "(") depthParentheses++;
        else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == ")" and depthParentheses > 0) depthParentheses--;
        else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "{") depthBrace++;
        else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "}" and depthBrace > 0) depthBrace--;

        // Actual finding.
        else if (tok.tag == Lexer::Tag::SYMBOL and tok.content == "," and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0) commaPositions.push_back(i);
    }
    const Uint colonPos = endOff;
    if (lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":") throw Parser::Generator::Error("An for statement does not end with an ending ':'", startOff);
    if (commaPositions.size() != 2) throw Parser::Generator::Error("A for statement does not have 2 ',' separators", startOff);
    if ((startOff + 1) > (commaPositions[0] - 1)) throw Parser::Generator::Error("The initialization expression of a for statement is empty", startOff);
    if ((commaPositions[0] + 1) > (commaPositions[1] - 1)) throw Parser::Generator::Error("The condition expression of a for statement is empty", startOff);
    if ((commaPositions[1] + 1) > (colonPos - 1)) throw Parser::Generator::Error("The increment expression of a for statement is empty", startOff);

    std::unique_ptr<const Parser::AST::Stmt::Assign> init = Parser::Generator::extractAssignStrict(lexer, startOff + 1, commaPositions[0] - 1);
    std::unique_ptr<const Parser::AST::Expr::Base> cond = Parser::SubParser::Expr::parse(lexer, commaPositions[0] + 1, commaPositions[1] - 1);
    std::unique_ptr<const Parser::AST::Stmt::Assign> inc = Parser::Generator::extractAssignStrict(lexer, commaPositions[1] + 1, colonPos - 1);

    std::pair<std::unique_ptr<const Parser::AST::Block>, Uint> pair = Parser::Generator::parseBlock(lexer, colonPos + 1, indentLevel, true);
    std::unique_ptr<const Parser::AST::Block> thenBlock = std::move(pair.first);

    return std::make_pair(std::make_unique<const Parser::AST::Stmt::For>(std::move(init), std::move(cond), std::move(inc), std::move(thenBlock), lexer[startOff].locationData),
                          pair.second);
}
std::pair<std::unique_ptr<const Parser::AST::Stmt::Assign>, Uint> Parser::Generator::extractAssign(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    static constexpr auto operations = std::to_array<std::string_view>(
    {
        "<<=", ">>=", "+=", "-=", "*=", "/=", "%=", "|=", "&=", "^=", "="
    });

    Uint oprPos = NPOS;
    for (Uint i = startOff; i <= endOff; i++)
    {
        const Lexer::Token& token = lexer[i];

        if (token.tag == Lexer::Tag::SYMBOL)
        {
            auto it = std::find(operations.begin(), operations.end(), token.content);
            if (it != operations.end())
            {
                oprPos = i;
                break;
            }
        }
    }
    if (oprPos == NPOS) return std::make_pair(nullptr, startOff);;
    if (indentLevel == 0) throw Parser::Generator::Error("Assignment cannot be in the global scope", startOff);
    if (startOff >= oprPos) throw Parser::Generator::Error("No type before '='", startOff);
    if (oprPos >= endOff) throw Parser::Generator::Error("No type after '='", startOff);

    std::unique_ptr<const Parser::AST::Expr::Base> dst = Parser::SubParser::Expr::parse(lexer, startOff, oprPos - 1);
    const std::string_view opr = lexer[oprPos].content;
    std::unique_ptr<const Parser::AST::Expr::Base> src = Parser::SubParser::Expr::parse(lexer, oprPos + 1, endOff);

    return std::make_pair(std::make_unique<const Parser::AST::Stmt::Assign>(std::move(dst), std::move(src), opr, lexer[startOff].locationData), endOff + 1);
}
std::unique_ptr<const Parser::AST::Stmt::Assign> Parser::Generator::extractAssignStrict(const Lexer::Generator& lexer, Uint startOff, Uint endOff)
{
    static constexpr auto operations = std::to_array<std::string_view>(
    {
        "<<=", ">>=", "+=", "-=", "*=", "/=", "%=", "|=", "&=", "^=", "="
    });

    Uint oprPos = NPOS;
    for (Uint i = startOff; i <= endOff; i++)
    {
        const Lexer::Token& token = lexer[i];

        if (token.tag == Lexer::Tag::SYMBOL)
        {
            auto it = std::find(operations.begin(), operations.end(), token.content);
            if (it != operations.end())
            {
                oprPos = i;
                break;
            }
        }
    }
    if (oprPos == NPOS) throw Parser::Generator::Error("No assignment operation in an assignment", startOff);
    if (startOff >= oprPos) throw Parser::Generator::Error("No expression before assignment operation", startOff);
    if (oprPos >= endOff) throw Parser::Generator::Error("No expression after assignment operation", startOff);

    std::unique_ptr<const Parser::AST::Expr::Base> dst = Parser::SubParser::Expr::parse(lexer, startOff, oprPos - 1);
    const std::string_view opr = lexer[oprPos].content;
    std::unique_ptr<const Parser::AST::Expr::Base> src = Parser::SubParser::Expr::parse(lexer, oprPos + 1, endOff);

    return std::make_unique<const Parser::AST::Stmt::Assign>(std::move(dst), std::move(src), opr, lexer[startOff].locationData);
}
std::pair<std::unique_ptr<const Parser::AST::Stmt::Break>, Uint> Parser::Generator::extractBreak(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    if (endOff - startOff > 1) return std::make_pair(nullptr, startOff);;

    const Lexer::Token& token = lexer[startOff];
    if (token.tag != Lexer::Tag::KEYWORD or token.content != "break") return std::make_pair(nullptr, startOff);;
    if (indentLevel == 0) throw Parser::Generator::Error("Break cannot be in the global scope", startOff);

    return std::make_pair(std::make_unique<const Parser::AST::Stmt::Break>(lexer[startOff].locationData), endOff + 1);
}
std::pair<std::unique_ptr<const Parser::AST::Stmt::Continue>, Uint> Parser::Generator::extractContinue(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    if (endOff - startOff > 1) return std::make_pair(nullptr, startOff);;

    const Lexer::Token& token = lexer[startOff];
    if (token.tag != Lexer::Tag::KEYWORD or token.content != "continue") return std::make_pair(nullptr, startOff);;
    if (indentLevel == 0) throw Parser::Generator::Error("Continue cannot be in the global scope", startOff);

    return std::make_pair(std::make_unique<const Parser::AST::Stmt::Continue>(lexer[startOff].locationData), endOff + 1);
}
std::pair<std::unique_ptr<const Parser::AST::Stmt::Return>, Uint> Parser::Generator::extractReturn(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    const Lexer::Token& token = lexer[startOff];
    if (token.tag != Lexer::Tag::KEYWORD or token.content != "return") return std::make_pair(nullptr, startOff);;
    if (indentLevel == 0) throw Parser::Generator::Error("Returns cannot be in the global scope", startOff);

    std::unique_ptr<const Parser::AST::Expr::Base> expr = nullptr;
    if (startOff + 1 <= endOff) expr = std::move(Parser::SubParser::Expr::parse(lexer, startOff + 1, endOff));
    return std::make_pair(std::make_unique<const Parser::AST::Stmt::Return>(std::move(expr), lexer[startOff].locationData), endOff + 1);
}
std::pair<std::unique_ptr<const Parser::AST::Decl::Var>, Uint> Parser::Generator::extractVar(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    Uint endOff = Parser::Generator::findEndOff(lexer, startOff);
    const Uint namePos = startOff;
    const Uint colonPos = namePos + 1;

    if (namePos + 1 > endOff) return std::make_pair(nullptr, startOff);;
    if (lexer[namePos].tag != Lexer::Tag::IDENTIFIER) return std::make_pair(nullptr, startOff);;
    if (lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":")
        return std::make_pair(nullptr, startOff);

    Uint eqPos = NPOS;
    for (Uint i = namePos; i <= endOff; i++)
    {
        const Lexer::Token& token = lexer[i];

        if (token.tag == Lexer::Tag::SYMBOL and token.content == "=")
        {
            eqPos = i;
            break;
        }
    }
    if (eqPos == NPOS) throw Parser::Generator::Error("A variable declaration without an '='", startOff);
    if (eqPos + 1 > endOff) throw Parser::Generator::Error("An '=' without an expression after it", startOff);
    if (eqPos - 1 == namePos) throw Parser::Generator::Error("A variable without a type", startOff);
    if (indentLevel == 0) throw Parser::Generator::Error("Variables cannot be in the global scope", startOff);

    const std::string_view name = lexer[namePos].content;
    auto retType = Parser::SubParser::Type::parse(lexer, colonPos + 1, eqPos - 1);
    std::unique_ptr<const Parser::AST::Type::Base> type = std::move(retType.type);
    if ((retType.endOff + 1) < eqPos) throw Parser::Generator::Error("Type have left overs", startOff);
    std::unique_ptr<const Parser::AST::Expr::Base> expr = Parser::SubParser::Expr::parse(lexer, eqPos + 1, endOff);;
    
    std::unique_ptr<const Parser::AST::Decl::Var> varDecl = std::make_unique<const Parser::AST::Decl::Var>(name, std::move(type), std::move(expr),
                                                                                                           lexer[startOff].locationData);
    return std::make_pair(std::move(varDecl), endOff + 1);
}
std::pair<Parser::Generator::MutableField, Uint> Parser::Generator::extractFieldMutable(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    Uint endOff = Parser::Generator::findEndOff(lexer, startOff);
    const Uint namePos = startOff;
    Uint colonPos = namePos + 1;

    if (namePos + 2 > endOff) throw Parser::Generator::Error("Field declaration is too short", startOff);
    if (lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":")
        throw Parser::Generator::Error("A field declaration without an '=' or ':'", startOff);

    const std::string_view name = lexer[namePos].content;

    auto retType = Parser::SubParser::Type::parse(lexer, colonPos + 1, endOff);
    if ((retType.endOff + 1) < endOff) throw Parser::Generator::Error("Type have left overs", startOff);
    return std::make_pair(Parser::Generator::MutableField(name, std::move(retType.type)), endOff + 1);
}
std::pair<std::unique_ptr<const Parser::AST::Decl::Typedef>, Uint> Parser::Generator::extractTypedef(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    Uint endOff = Parser::Generator::findEndOff(lexer, startOff);
    const Uint namePos = startOff + 1;
    const Uint typePos = startOff + 3;
    
    if (endOff - startOff < 3 or
        lexer[startOff].content != "typedef") return std::make_pair(nullptr, startOff);;
    if (indentLevel >= 1) throw Parser::Generator::Error("Typedefs can only be in the global scope", startOff);
    if (lexer[namePos].tag != Lexer::Tag::IDENTIFIER) throw Parser::Generator::Error("Name is not valid", startOff);

    Uint eqPos = NPOS;
    for (Uint i = namePos; i <= endOff; i++)
    {
        const Lexer::Token& token = lexer[i];
    
        if (token.tag == Lexer::Tag::SYMBOL and token.content == "=")
        {
            eqPos = i;
            break;
        }
    }
    if (eqPos == NPOS) throw Parser::Generator::Error("A variable declaration without an '='", startOff);
    
    const std::string_view name = lexer[namePos].content;

    auto retType = Parser::SubParser::Type::parse(lexer, eqPos + 1, endOff);
    std::unique_ptr<const Parser::AST::Type::Base> type = std::move(retType.type);
    if ((retType.endOff + 1) < endOff) throw Parser::Generator::Error("Type have left overs", startOff);
    
    std::unique_ptr<const Parser::AST::Type::Typedef> typedefType = std::make_unique<const Parser::AST::Type::Typedef>(std::move(type));
    std::unique_ptr<const Parser::AST::Decl::Typedef> typedefDecl = std::make_unique<const Parser::AST::Decl::Typedef>(name, std::move(typedefType),
                                                                                                                       lexer[startOff].locationData);

    return std::make_pair(std::move(typedefDecl), endOff + 1);
}
std::pair<std::unique_ptr<const Parser::AST::Decl::Func>, Uint> Parser::Generator::extractFunc(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);
    const Uint colonPos = endOff;
    bool isExtern = false;

    if ((startOff + 2 > endOff) or (lexer[startOff].tag != Lexer::Tag::KEYWORD) or (lexer[startOff].content != "def" and lexer[startOff].content != "def_extern"))
        return std::make_pair(nullptr, startOff);
    if (lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":")
        return std::make_pair(nullptr, startOff);;

    isExtern = lexer[startOff].content == "def_extern";

    if (indentLevel >= 1) throw Parser::Generator::Error("Functions can only be in the global scope", startOff);
    const Uint namePos = startOff + 1;
    const Uint parenthesisStartPos = startOff + 2;
    if (lexer[namePos].tag != Lexer::Tag::IDENTIFIER) throw Parser::Generator::Error("Name is not valid", startOff);
    if (lexer[parenthesisStartPos].tag != Lexer::Tag::SYMBOL or lexer[parenthesisStartPos].content != "(") throw Parser::Generator::Error("Func does not have a starting '('", startOff);

    std::vector<Uint> commaPositions;
    Uint parenthesisEndPos = NPOS;
    Uint depthBracket = 0;
    Uint depthParentheses = 0;
    Uint depthBrace = 0;
    for (Uint i = parenthesisStartPos; i <= endOff; i++)
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
        if (token.tag == Lexer::Tag::SYMBOL and token.content == ")" and depthBracket == 0 and depthParentheses == 0 and depthBrace == 0)
        {
            parenthesisEndPos = i;
            break;
        }
        if (token.tag == Lexer::Tag::SYMBOL and token.content == "," and depthBracket <= 0 and depthParentheses <= 1) commaPositions.push_back(i);
    } 
    if (parenthesisEndPos == NPOS) throw Parser::Generator::Error("A function declaration without an ending ')'", startOff);
    Uint arrowPos = NPOS;
    Parser::SubParser::Type::Return returnType (nullptr, parenthesisEndPos);
    if (parenthesisEndPos == NPOS) throw Parser::Generator::Error("Could not have found an ending ')'", startOff);
    if (parenthesisEndPos + 1 <= endOff and lexer[parenthesisEndPos + 1].tag == Lexer::Tag::SYMBOL and lexer[parenthesisEndPos + 1].content == "->")
    {
        arrowPos = parenthesisEndPos + 1;
        if (arrowPos + 1 > endOff) throw Parser::Generator::Error("Return type without a type", startOff);
        returnType = Parser::SubParser::Type::parse(lexer, arrowPos + 1, colonPos - 1);
        if ((returnType.endOff + 1) < colonPos) throw Parser::Generator::Error("Type have left overs", startOff);
    }
    else if (parenthesisEndPos + 1 <= endOff and (lexer[parenthesisEndPos + 1].tag != Lexer::Tag::SYMBOL or lexer[parenthesisEndPos + 1].content != ":"))
        throw Parser::Generator::Error("Invalid token after ending ')'", startOff);

    auto extractParam = [&lexer] (Uint startOff, Uint endOff) -> std::pair<std::string_view, std::unique_ptr<const Parser::AST::Type::Base>>
    {
        const Uint namePos = startOff;
        const Uint colonPos = startOff + 1;
    
        const Lexer::Token& Cname = lexer[namePos];
        const Lexer::Token& Ccolon = lexer[colonPos];

        if (endOff - startOff < 2) throw Parser::Generator::Error("Parameter is not complete", startOff);
        if (lexer[namePos].tag != Lexer::Tag::IDENTIFIER or
            lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":")
            throw Parser::Generator::Error("Parameter have no valid tokens", startOff);
    
        const std::string_view name = lexer[startOff].content;

        auto retType = Parser::SubParser::Type::parse(lexer, colonPos + 1, endOff);
        std::unique_ptr<const Parser::AST::Type::Base> type = std::move(retType.type);
        if ((retType.endOff + 1) < endOff) throw Parser::Generator::Error("Type have left overs", startOff);
        return std::make_pair(name, std::move(type));
    };
    
    // They must be separated.
    std::vector<std::string_view> names;
    std::vector<std::unique_ptr<const Parser::AST::Type::Base>> types;
    Uint paramStart = namePos + 2;
    bool isEllipsis = false;
    for (Uint comma : commaPositions)
    {
        Uint paramEnd = comma - 1;

        auto pair = extractParam(paramStart, paramEnd);
        names.push_back(pair.first);
        types.push_back(std::move(pair.second));

        paramStart = comma + 1;
    }
    if (not commaPositions.empty() or parenthesisEndPos - parenthesisStartPos > 1)
    {
        if (lexer[paramStart].tag == Lexer::Tag::SYMBOL and lexer[paramStart].content == "...")
        {
            if (paramStart + 1 >= parenthesisEndPos)
            {
                isEllipsis = true;
            }
            else throw Parser::Generator::Error("'...' must be at the end of the function parameter type", startOff);
        }
        else
        {
            auto lastPair = extractParam(paramStart, parenthesisEndPos - 1);
            names.push_back(lastPair.first);
            types.push_back(std::move(lastPair.second));
        }
    }
    if (arrowPos == NPOS and (arrowPos + 1 >= colonPos)) throw Parser::Generator::Error("An '->' with no return type", startOff);

    const std::string_view name = lexer[namePos].content;

    auto pair = (not isExtern) ? Parser::Generator::parseBlock(lexer, colonPos + 1, indentLevel, true) : std::make_pair(nullptr, colonPos + 1);
    std::unique_ptr<const Parser::AST::Block> block = std::move(pair.first);
    std::unique_ptr<const Parser::AST::Type::Func> funcType = std::make_unique<const Parser::AST::Type::Func>(std::move(returnType.type), std::move(types), isEllipsis);
    std::unique_ptr<const Parser::AST::Decl::Func> funcDecl = std::make_unique<const Parser::AST::Decl::Func>(name, std::move(funcType), std::move(names), std::move(block),
                                                                                                              lexer[startOff].locationData);
    return std::make_pair(std::move(funcDecl), pair.second);
}
std::pair<std::unique_ptr<const Parser::AST::Decl::Struct>, Uint> Parser::Generator::extractStruct(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);
    
    const Uint sigPos = startOff;
    const Uint namePos = startOff + 1;
    const Uint colonPos = startOff + 2;
    Uint blockStart = colonPos + 1;

    if (endOff - sigPos < 2 or
        lexer[sigPos].tag != Lexer::Tag::KEYWORD or
        lexer[sigPos].content != "struct") return std::make_pair(nullptr, startOff);;
    if (indentLevel >= 1) throw Parser::Generator::Error("Structs can only be in the global scope", startOff);

    if (lexer[namePos].tag != Lexer::Tag::IDENTIFIER) throw Parser::Generator::Error("Name is not valid", startOff);
    if (lexer[colonPos].tag != Lexer::Tag::SYMBOL or lexer[colonPos].content != ":") throw Parser::Generator::Error("Struct does not have an ending ':'", startOff);

    const std::string_view name = lexer[namePos].content;
    std::vector<Parser::AST::Type::Struct::Field> fields;
    Uint i = colonPos + 1;

    while (i < lexer.size() and lexer[i].tag == Lexer::Tag::NEW_LINE) i++;
    if (i < lexer.size() and lexer[i++].tag != Lexer::Tag::INDENT) throw Parser::Generator::Error("Expected indent is missing", i);
    if (i >= lexer.size()) throw Parser::Generator::Error("Struct has no fields", i);

    while (i < lexer.size())
    {
        const Lexer::Token& token = lexer[i];

        if (token.tag == Lexer::Tag::INDENT) throw Parser::Generator::Error("Unexpected indent", i);
        if (token.tag == Lexer::Tag::DEDENT)
        {
            i++;
            break;
        }
        if (token.tag == Lexer::Tag::NEW_LINE)
        {
            i++;
            continue;
        }
        auto pair = Parser::Generator::extractFieldMutable(lexer, i, indentLevel + 1);
        
        const std::string_view fname = pair.first.name;
        std::unique_ptr<const Parser::AST::Type::Base> ftype = std::move(pair.first.type);
        if (ftype->tag == Parser::AST::Type::Tag::IDENTIFIER and
            static_cast<const Parser::AST::Type::Identifier*>(ftype.get())->name == name)
            throw Parser::Generator::Error("A struct field type cannot be same type of his own struct", i);

        i = pair.second;
        fields.emplace_back(fname, std::move(ftype));
    }

    std::unique_ptr<const Parser::AST::Type::Struct> structType = std::make_unique<const Parser::AST::Type::Struct>(std::move(fields));
    std::unique_ptr<const Parser::AST::Decl::Struct> structDecl = std::make_unique<const Parser::AST::Decl::Struct>(name, std::move(structType), lexer[startOff].locationData);

    return std::make_pair(std::move(structDecl), i);
}
std::pair<std::unique_ptr<const Parser::AST::Expr::Base>, Uint> Parser::Generator::extractExpr(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel)
{
    const Uint endOff = Parser::Generator::findEndOff(lexer, startOff);

    Parser::SubParser::Expr::Return ret = Parser::SubParser::Expr::parseEndOff(lexer, startOff, endOff);
    if (indentLevel == 0) throw Parser::Generator::Error("Expressions cannot be in the global scope", startOff);
    return std::make_pair(std::move(ret.expr), ret.endOff + 1);    
}

Parser::Generator::MutableField::MutableField(std::string_view new_name, std::unique_ptr<const Parser::AST::Type::Base> new_type) :
    name(new_name), type(std::move(new_type))
{
}
