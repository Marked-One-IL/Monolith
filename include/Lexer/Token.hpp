#pragma once
#include <Helper/Types.hpp>
#include <string_view>
#include <ostream>

namespace Lexer
{
    enum class Tag : uint8_t
    {
        // Literals
        STRING3_LITERAL,// """Hello, World"""
        STRING_LITERAL, // "Hello, World!"
        CHAR_LITERAL,   // 'H'
        HEX_LITERAL,    // 0xABCD
        BIN_LITERAL,    // 0b0101
        OCT_LITERAL,    // 0o01234567
        SCI_LITERAL,    // 1e-3
        FLOAT_LITERAL,  // 1234.1234
        INT_LITERAL,    // 1234
        BOOL_LITERAL,   // True, False
        NONE_LITERAL,   // None

        // Constants
        SYMBOL,         // +=, -, *, >>, (, and, or and ect
        KEYWORD,        // if, for, def and ects

        // Other
        NEW_LINE,        // \n
        INDENT,          // Increasement in level (\n and \t)
        DEDENT,          // Decreasement in level (\n and \t)
        IDENTIFIER       // *any*
    };

    struct LocationData
    {
        const char* filename;
        std::string_view line = {};
        Uint lineNum = 0;
    };

    struct Token
    {
        Token(Lexer::Tag new_tag, std::string_view new_content = {});

        friend std::ostream& operator << (std::ostream& stream, const Lexer::Token& token);

        bool isNone(void) const;
        bool isBool(void) const;
        bool isChar(void) const;
        bool isInt(void) const;
        bool isFloat(void) const;
        bool isStr(void) const;

        const Lexer::Tag tag;
        const std::string_view content;
        Lexer::LocationData locationData;
    };
}
