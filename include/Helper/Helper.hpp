#pragma once
#include <string>
#include <ostream>
#include <Lexer/Token.hpp>
#include <Helper/Types.hpp>

namespace Helper
{
    std::string extractFileContent(std::string filename);
    std::ostream& printDepth(std::ostream& stream, Uint depth);
    void applyDepth(std::string& x, Uint depth);
}
