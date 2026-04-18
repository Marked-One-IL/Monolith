#pragma once
#include <string>
#include <vector>
#include <list>
#include <Lexer/Token.hpp>

namespace x86Gen::Context
{
    class Global
    {
    public:
        static std::string appendAndGetStrLiteral(const Lexer::Token& token);
        static std::string appendAndGetFloatLiteral(const Lexer::Token& token);
        static void appendExternalFunction(std::string_view name);
        static bool isExternalFunction(std::string_view name);

        static const std::list<std::vector<char>>& getStringLiterals(void);
        static const std::list<std::string>& getFloats(void);
        static const std::vector<std::string_view>& getExternalFunctions(void);

    private:
        static std::list<std::vector<char>> m_strLiterals;
        static std::list<std::string> m_floats;
        static std::vector<std::string_view> m_externFuncs;
    };
}
