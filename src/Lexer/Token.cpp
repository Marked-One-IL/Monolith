#include <Lexer/Token.hpp>
#include <Helper/Types.hpp>
#include <Helper/Assert.hpp>

Lexer::Token::Token(Lexer::Tag new_tag, std::string_view new_content)
    : tag(new_tag), content(new_content)
{
    Assert_Message(this->tag >= Tag::STRING3_LITERAL and this->tag <= Tag::IDENTIFIER, std::format("Unknown Tag: {}", static_cast<int>(this->tag)));
}

bool Lexer::Token::isNone(void) const
{
    switch (this->tag)
    {
    case Lexer::Tag::NONE_LITERAL:
        return true;
    }

    return false;
}
bool Lexer::Token::isBool(void) const
{
    switch (this->tag)
    {
    case Lexer::Tag::BOOL_LITERAL:
        return true;
    }

    return false;
}
bool Lexer::Token::isChar(void) const
{
    switch (this->tag)
    {
    case Lexer::Tag::CHAR_LITERAL:
        return true;
    }

    return false;
}
bool Lexer::Token::isInt(void) const
{
    switch (this->tag)
    {
    case Lexer::Tag::HEX_LITERAL:
    case Lexer::Tag::BIN_LITERAL:
    case Lexer::Tag::OCT_LITERAL:
    case Lexer::Tag::INT_LITERAL:
        return true;
    }

    return false;
}
bool Lexer::Token::isFloat(void) const
{
    switch (this->tag)
    {
    case Lexer::Tag::SCI_LITERAL:
    case Lexer::Tag::FLOAT_LITERAL:
        return true;
    }

    return false;
}
bool Lexer::Token::isStr(void) const
{
    switch (this->tag)
    {
    case Lexer::Tag::STRING3_LITERAL:
    case Lexer::Tag::STRING_LITERAL:
        return true;
    }

    return false;
}

namespace Lexer
{
    std::ostream& operator << (std::ostream& stream, const Lexer::Token& token)
    {
        // Format: [Tag: 'Content']
        static thread_local Uint indentCount = 0; // If I ever do multi-threading. (I won't).
        static thread_local bool shouldPrintIndents = false;

        if (shouldPrintIndents)
        {
            if (token.tag == Lexer::Tag::INDENT) indentCount++;
            else if (token.tag == Lexer::Tag::DEDENT) indentCount--;

            for (Uint i = 0; i < indentCount; i++)
            {
                stream << '\t';
            }
        }

        switch (token.tag)
        {
        case Lexer::Tag::NEW_LINE:
            shouldPrintIndents = true;
            stream << "[NEWLINE]\n";
            return stream;
        case Lexer::Tag::INDENT:
            shouldPrintIndents = true;
            stream << "[INDENT]\n";
            return stream;
        case Lexer::Tag::DEDENT:
            shouldPrintIndents = true;
            stream << "[DEDENT]\n";
            return stream;
        }

        shouldPrintIndents = false;

        stream << '[';
        switch (token.tag)
        {
        case Lexer::Tag::STRING3_LITERAL:
            stream << "STRING3_LITERAL";
            break;
        case Lexer::Tag::STRING_LITERAL:
            stream << "STRING_LITERAL";
            break;
        case Lexer::Tag::CHAR_LITERAL:
            stream << "CHAR_LITERAL";
            break;
        case Lexer::Tag::HEX_LITERAL:
            stream << "HEX_LITERAL";
            break;
        case Lexer::Tag::BIN_LITERAL:
            stream << "BIN_LITERAL";
            break;
        case Lexer::Tag::OCT_LITERAL:
            stream << "OCT_LITERAL";
            break;
        case Lexer::Tag::SCI_LITERAL:
            stream << "SCI_LITERAL";
            break;
        case Lexer::Tag::FLOAT_LITERAL:
            stream << "FLOAT_LITERAL";
            break;
        case Lexer::Tag::INT_LITERAL:
            stream << "INT_LITERAL";
            break;
        case Lexer::Tag::BOOL_LITERAL:
            stream << "BOOL_LITERAL";
            break;
        case Lexer::Tag::NONE_LITERAL:
            stream << "NONE_LITERAL";
            break;
        case Lexer::Tag::SYMBOL:
            stream << "SYMBOL";
            break;
        case Lexer::Tag::KEYWORD:
            stream << "KEYWORD";
            break;
        case Lexer::Tag::INDENT:
            stream << "INDENT";
            break;
        case Lexer::Tag::DEDENT:
            stream << "DEDENT";
            break;
        case Lexer::Tag::IDENTIFIER:
            stream << "IDENTIFIER";
            break;

        default:
            Assert_Message(ASSERT_ALWAYS, std::format("Unknown Tag: {}", static_cast<int>(token.tag)));
        }
        stream << ": '" << token.content << "'] ";

        return stream;
    }
}
