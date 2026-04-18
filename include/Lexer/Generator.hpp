#pragma once
#include "Token.hpp"
#include <Helper/Types.hpp>
#include <vector>
#include <string_view>
#include <stack>
#include <list>
#include <optional>

namespace Lexer
{
    class Generator
    {
    public:
        Generator(std::string file, std::string filename);

        // Wrapper for m_tokens.
        bool empty(void) const;
        Uint size(void) const;
        const Lexer::Token& operator [] (Uint index) const;
        std::vector<Lexer::Token>::const_iterator begin(void) const;
        std::vector<Lexer::Token>::const_iterator end(void) const;
        std::vector<Lexer::Token> copy(void) const;
        const char* getFilename(void) const;

        friend std::ostream& operator << (std::ostream& stream, const Lexer::Generator& generator);

        struct Error
        {
            Error(const char* new_error);
            const char* error = nullptr;
        };

    private:
        // Note:
        // A method that takes a std::string_view& and not const std::string_view& will increment the string it didn't throw or returned std::nullopt.

        static void skipSpaces(std::string_view& view);
        static void incrementToNextLine(std::string_view& view);
        static Lexer::LocationData getLocationData(const char* filename, std::string_view line, std::string_view current, Uint lineNum);

        static std::optional<Uint> extractSpacesLevel(const std::string_view& view);
        static std::optional<std::string_view> extractUntilNewLine(const std::string_view& view);
        static std::optional<std::string_view> extractUntilNotAlnum(const std::string_view& view);
        
        static std::optional<Lexer::Token> extractString3Literal(std::string_view& view);
        static std::optional<Lexer::Token> extractStringLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractCharLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractHexLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractBinLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractOctLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractSciLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractFloatLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractIntLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractBoolLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractNoneLiteral(std::string_view& view);
        static std::optional<Lexer::Token> extractSymbol(std::string_view& view, Uint& depthClosingCountParentheses, Uint& depthClosingCountBracket, Uint& depthClosingCountBrace);
        static std::optional<Lexer::Token> extractKeyword(std::string_view& view);
        static std::optional<Lexer::Token> extractNewLine(std::string_view& view);
        static std::optional<std::vector<Lexer::Token>> extractInDedent(std::string_view& view, std::stack<Uint>& identLevels);
        static std::optional<Lexer::Token> extractIdentifier(std::string_view& view);
        static std::optional<std::vector<Lexer::Token>> extractImport(std::string_view& view);

        std::string m_file;
        std::string m_filename;
        std::vector<Lexer::Token> m_tokens;
        static std::list<Lexer::Generator> lexers;
        static std::list<std::string> filenames;
    };
}
