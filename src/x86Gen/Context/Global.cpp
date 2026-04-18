#include <x86Gen/Context/Global.hpp>
#include <Helper/Assert.hpp>

std::list<std::vector<char>> x86Gen::Context::Global::m_strLiterals;
std::list<std::string> x86Gen::Context::Global::m_floats;
std::vector<std::string_view> x86Gen::Context::Global::m_externFuncs;

std::string x86Gen::Context::Global::appendAndGetStrLiteral(const Lexer::Token& token)
{
    std::string_view tmpStrView = token.content;
    if (token.tag == Lexer::Tag::STRING_LITERAL)
    {
        // "God forsaken me" -> God forsaken me
        tmpStrView.remove_prefix(1);
        tmpStrView.remove_suffix(1);
    }
    else // if (token.tag == Lexer::Tag::STRING3_LITERAL)
    {
        // """God forsaken
        // """me -> God forsaken me
        tmpStrView.remove_prefix(3);
        tmpStrView.remove_suffix(3);
    }

    std::vector<char> buffer;
    buffer.reserve(tmpStrView.size());
    while (not tmpStrView.empty())
    {
        char c = tmpStrView.front();

        if (tmpStrView.size() >= 2 and c == '\\')
        {
            char nextC = tmpStrView[1];

            switch (nextC)
            {
            case 'a':  buffer.emplace_back('\a'); break;
            case 'b':  buffer.emplace_back('\b'); break;
            case 'f':  buffer.emplace_back('\f'); break;
            case 'n':  buffer.emplace_back('\n'); break;
            case 'r':  buffer.emplace_back('\r'); break;
            case 't':  buffer.emplace_back('\t'); break;
            case 'v':  buffer.emplace_back('\v'); break;
            case '\'': buffer.emplace_back('\''); break;
            case '"':  buffer.emplace_back('\"'); break;
            case '?':  buffer.emplace_back('\?'); break;
            case '\\': buffer.emplace_back('\\'); break;
            case '0':  buffer.emplace_back('\0'); break;
            default: 
                buffer.emplace_back('\\');
                buffer.emplace_back(nextC);
                tmpStrView.remove_prefix(2);
                continue;
            }

            tmpStrView.remove_prefix(2);
        }
        else
        {
            buffer.emplace_back(c);
            tmpStrView.remove_prefix(1);
        }
    }

    buffer.emplace_back('\0');
    x86Gen::Context::Global::m_strLiterals.emplace_back(std::move(buffer));
    return std::format("offset stringLiteral_{}", x86Gen::Context::Global::m_strLiterals.size() - 1);
}
std::string x86Gen::Context::Global::appendAndGetFloatLiteral(const Lexer::Token& token)
{
    // 1e-3 -> 0.001
    std::string tmpStr (token.content);
    float value = std::strtof(tmpStr.c_str(), nullptr);
    x86Gen::Context::Global::m_floats.emplace_back(std::to_string(value));
    return std::format("floatLiteral_{}", x86Gen::Context::Global::m_floats.size() - 1);
}
void x86Gen::Context::Global::appendExternalFunction(std::string_view name)
{
    x86Gen::Context::Global::m_externFuncs.emplace_back(name);
}
bool x86Gen::Context::Global::isExternalFunction(std::string_view name)
{
    return std::find(x86Gen::Context::Global::m_externFuncs.begin(), x86Gen::Context::Global::m_externFuncs.end(), name) != x86Gen::Context::Global::m_externFuncs.end();
}

const std::list<std::vector<char>>& x86Gen::Context::Global::getStringLiterals(void)
{
    return x86Gen::Context::Global::m_strLiterals;
}
const std::list<std::string>& x86Gen::Context::Global::getFloats(void)
{
    return x86Gen::Context::Global::m_floats;
}
const std::vector<std::string_view>& x86Gen::Context::Global::getExternalFunctions(void)
{
    return x86Gen::Context::Global::m_externFuncs;
}
