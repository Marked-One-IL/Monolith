#include <Helper/Helper.hpp>
#include <fstream>
#include <charconv>
#include <stdexcept>
#include <Lexer/Generator.hpp>
#include <sstream>

std::string Helper::extractFileContent(std::string filename)
{
    std::ifstream file(filename.data(), std::ios::in);
    if (not file) throw std::runtime_error("Could not extract file content");

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}
std::ostream& Helper::printDepth(std::ostream& stream, Uint depth)
{
    for (Uint i = 0; i < depth; i++) 
    {
        stream << "  "; 
    }
    return stream;
}
void Helper::applyDepth(std::string& x, Uint depth)
{
    Uint originalSize = x.size();
    std::stringstream tempBuffer (std::move(x));
    std::string currentString;
    std::string newString;
    newString.reserve(originalSize);

    while (std::getline(tempBuffer, currentString))
    {
        for (Uint i = 0; i < depth * 4; i++) newString.push_back(' ');
        newString += std::move(currentString);
        newString.push_back('\n');
    }

    x = newString;
}
