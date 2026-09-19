#include <Lexer/Generator.hpp>
#include <Helper/Helper.hpp>
#include <Helper/Assert.hpp>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <array>
#include <iterator>
#include <cctype>
#include <cmath>
#include <list>
#include <filesystem>

// Blessed be You, O my Lord. Our God, King of the world.
// That he shall protect this code from bugs and undefined behavior. Amen :)

std::list<Lexer::Generator> Lexer::Generator::lexers;
std::list<std::string> Lexer::Generator::filenames;

Lexer::Generator::Generator(std::string file, std::string filename) : 
    m_file(file)
{
    auto normalized = std::filesystem::path(filename);
    normalized.make_preferred();
    this->m_filename = normalized.string();

    Lexer::Generator::filenames.emplace_back(this->m_filename);

    // Scan for weird chars.
    for (unsigned char c : this->m_file)
    {
        if (not std::isprint(c) and not std::iscntrl(c))
        {
            throw std::runtime_error("Unprintable chars");
        }
    }

    // For lexering.
    std::string_view view = this->m_file;
    std::stack<Uint> identLevels;
    Uint depthClosingCountParentheses = 0;
    Uint depthClosingCountBracket = 0;
    Uint depthClosingCountBrace = 0;
    bool shouldCheckIndentFlag = true;
    
    // For errors.
    Uint linesCount = 1;
    std::string_view currentLine;
    if (auto opt2 = Lexer::Generator::extractUntilNewLine(view)) currentLine = opt2.value();

    // Guidelines: 
    // 1. Every (with exceptions) extract'XTag' must start with this code chunk.
    // std::string_view fixedView;
    // if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    // else return std::nullopt;
    // And later code only use fixedView and not view. Only after 100% of extracting/scanning everything you are allowed to do view.remove_prefix(x);
    // This is to avoid chars after the \n. (Please even if it seams not unnecessary or slow or unoptimized keep it).
    // 2. If you perform multiple scans in one function keep a variable named Number totalSize = 0; as the first chunk above them.
    // This will make it easier to know at which column to report errors or just calculate sizes. This is not a must but recommended.
    // Also make sure that you do 'totalSize++' next to something like 'i++' eg: for (...; ...; i++, totalSize++).

    // How this lexer work?
    // Basic concept: I first skip all the spaces and scan the current word I have to matching possible tokens one by one.
    // If it match I append to the tokens. Else I check other tokens.
    // Extra more complex concept: Every '\n' I check for indention level (number of spaces and tabs).
    // If I ecounter a (... or [... I stop this checking until I find an ending ...) or ...].

    while (not view.empty())
    {
        try
        {
            if (depthClosingCountParentheses or depthClosingCountBracket or depthClosingCountBrace)
            {
                if (auto opt = Lexer::Generator::extractNewLine(view))
                {
                    if (auto opt2 = Lexer::Generator::extractUntilNewLine(view)) currentLine = opt2.value();
                    linesCount++;
                    continue;
                }
            }

            // Newline must be first.
            // This avoids the other extract function getting a string like that "\nx = 1234" and converting it into "".
            // This could prevent bugs.
            if (auto opt = Lexer::Generator::extractNewLine(view))
            {
                if (auto opt2 = Lexer::Generator::extractUntilNewLine(view)) currentLine = opt2.value();
                linesCount++;
                shouldCheckIndentFlag = true;

                if (not depthClosingCountParentheses and not depthClosingCountBracket and not depthClosingCountBrace)
                {
                    opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                    this->m_tokens.emplace_back(opt.value());
                }
                continue;
            }

            // Sorry for this ugly nesting. It's required and there is no better way to do it.
            if (shouldCheckIndentFlag) // If newline and no closing depth.
            {
                if (not depthClosingCountParentheses and not depthClosingCountBracket and not depthClosingCountBrace) // This is nested here and not if (shouldCheckIndentFlag and not depthClosingCount) above. To make shouldCheckIndentFlag = false;.
                {
                    if (auto opt = Lexer::Generator::extractInDedent(view, identLevels))
                    {
                        for (auto& token : opt.value())
                        {
                            token.locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                            this->m_tokens.emplace_back(std::move(token));
                        }
                    }
                }
                shouldCheckIndentFlag = false;
                continue;
            }

            Lexer::Generator::skipSpaces(view);
            if (view.empty()) break;

            if (auto opt = Lexer::Generator::extractImport(view))
            {
                for (auto& token : opt.value())
                    this->m_tokens.emplace_back(token);
                continue;
            }

            // This ugly if .., continue is to keep 'auto opt' in the scope of the single 'if'.
            if (auto opt = Lexer::Generator::extractString3Literal(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractStringLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractCharLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractHexLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractBinLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractOctLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractSciLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            // Float before int because a string like this "1234.1234" will become: [INT_LITERAL: '1234'], [SYMBOL: '.'], [INT_LITERAL: '1234']
            if (auto opt = Lexer::Generator::extractFloatLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractIntLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractBoolLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractNoneLiteral(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractSymbol(view, depthClosingCountParentheses, depthClosingCountBracket, depthClosingCountBrace))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractKeyword(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }
            if (auto opt = Lexer::Generator::extractIdentifier(view))
            {
                opt.value().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
                this->m_tokens.emplace_back(opt.value());
                continue;
            }

            // This section happens if it's a comment. 
            Lexer::Generator::incrementToNextLine(view);
            if (auto opt2 = Lexer::Generator::extractUntilNewLine(view)) currentLine = opt2.value();
            // This must happen because in incrementToNextLine we skip the newline so we must acknowledge it directly.
            linesCount++;
            shouldCheckIndentFlag = true;
            if (not depthClosingCountParentheses and not depthClosingCountBracket and not depthClosingCountBrace)
            {
                this->m_tokens.emplace_back(Lexer::Tag::NEW_LINE);
                this->m_tokens.back().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
            }
        }
        catch (const Lexer::Generator::Error& error)
        {
            Lexer::LocationData locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
            throw std::runtime_error(std::format("Lexer:\nAt file: {}\nLine: {}\nError: {}", filename, locationData.lineNum, error.error, locationData.line));
        }
    }

    if (depthClosingCountParentheses != 0)
    {
        throw std::runtime_error("Invalid parentheses depth");
    }
    if (depthClosingCountBracket != 0)
    {
        throw std::runtime_error("Invalid bracket depth");
    }
    if (depthClosingCountBrace != 0)
    {
        throw std::runtime_error("Invalid brace depth");
    }

    if (not this->m_tokens.empty() and this->m_tokens.back().tag != Lexer::Tag::NEW_LINE)
    {
        this->m_tokens.emplace_back(Lexer::Tag::NEW_LINE);
        this->m_tokens.back().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
    }
    while (not identLevels.empty())
    {
        identLevels.pop();
        this->m_tokens.emplace_back(Lexer::Tag::DEDENT);
        this->m_tokens.back().locationData = Lexer::Generator::getLocationData(this->m_filename.data(), view, currentLine, linesCount);
    }

    bool isRelavent = false;
    for (auto& token : this->m_tokens)
    {
        if (token.tag != Lexer::Tag::NEW_LINE and
            token.tag != Lexer::Tag::INDENT and
            token.tag != Lexer::Tag::DEDENT)
        {
            isRelavent = true;
            break;
        }
    }
    if (not isRelavent) throw std::runtime_error("There is no code");
}

bool Lexer::Generator::empty(void) const
{
    return this->m_tokens.empty();
}
Uint Lexer::Generator::size(void) const
{
    return this->m_tokens.size();
}
const Lexer::Token& Lexer::Generator::operator [] (Uint index) const
{
    return this->m_tokens[index];
}
std::vector<Lexer::Token>::const_iterator Lexer::Generator::begin(void) const
{
    return this->m_tokens.begin();
}
std::vector<Lexer::Token>::const_iterator Lexer::Generator::end(void) const
{
    return this->m_tokens.end();
}
std::vector<Lexer::Token> Lexer::Generator::copy(void) const
{
    return this->m_tokens;
}
const char* Lexer::Generator::getFilename(void) const
{
    return this->m_filename.data();
}

void Lexer::Generator::skipSpaces(std::string_view& view)
{
    while (not view.empty() and (view.front() == ' ' or view.front() == '\t')) 
        view.remove_prefix(1);
}
void Lexer::Generator::incrementToNextLine(std::string_view& view)
{
    while (not view.empty() and (view.front() != '\n')) view.remove_prefix(1);
    if (not view.empty()) view.remove_prefix(1);
}
Lexer::LocationData Lexer::Generator::getLocationData(const char* filename, std::string_view line, std::string_view current, Uint lineNum)
{
    Lexer::Generator::skipSpaces(line);
    Lexer::Generator::skipSpaces(current);
    return Lexer::LocationData{ filename, current, lineNum };
}

std::optional<Uint> Lexer::Generator::extractSpacesLevel(const std::string_view& view)
{
    // Scan.
    std::string_view temp = view;
    Uint level = 0;
    while (not temp.empty() and (temp.front() == ' ' or temp.front() == '\t'))
    {
        level += temp.front() == ' ';
        level += static_cast<Uint>(temp.front() == '\t') * 4;
        temp.remove_prefix(1);
    }
    // If empty string or is a comment.
    if (temp.empty()) return std::nullopt;
    if (not temp.empty() and (temp.front() == '\n' or temp.front() == '#')) return std::nullopt;

    // Return.
    return level;
}
std::optional<std::string_view> Lexer::Generator::extractUntilNewLine(const std::string_view& view)
{
    // Scan.
    Uint newLinePos = view.find('\n');
    std::string_view content = view;
    if (newLinePos != std::string_view::npos)
    {
        content = view.substr(0, newLinePos);
    }
    if (content.empty()) return std::nullopt;

    // Return.
    return content;
}
std::optional<std::string_view> Lexer::Generator::extractUntilNotAlnum(const std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;
    
    // Scan.
    Uint i = 0;
    for (i = 0; i < fixedView.size(); i++)
    {
        unsigned char c = static_cast<unsigned char>(fixedView[i]);

        // Exit checks.
        if (not std::isalnum(c) and c != '_') break;
    }
    if (i == 0) return std::nullopt;

    // Return.
    return fixedView.substr(0, i);
}

std::optional<Lexer::Token> Lexer::Generator::extractString3Literal(std::string_view& view)
{
    // Early return.
    if (not view.starts_with("\"\"\"")) return std::nullopt;

    // Scan.
    Uint slashCount = 0;
    Uint quoteCount = 0;
    auto endPosIt = std::find_if(view.begin() + std::strlen("\"\"\""), view.end(), [&slashCount, &quoteCount] (char c) -> bool
    {
        // Normal Checks.
        if (c == '\"' and (slashCount % 2 == 0))
        {
            quoteCount++;

            // Exit Checks.
            if (quoteCount >= 3) return true;
        }
        else if (c == '\\') slashCount++;
        else
        {
            quoteCount = 0;
            slashCount = 0;
        }

        return false;
    });
    if (endPosIt == view.end())
    {
        throw Lexer::Generator::Error("Triple string literal does not end");
    }
    Uint endPos = std::distance(view.begin(), endPosIt) + std::strlen("\"");
    std::string_view string3Literal = view.substr(0, endPos);

    // Incrementation & return.
    std::string_view content = string3Literal;
    view.remove_prefix(endPos);
    return Lexer::Token(Lexer::Tag::STRING3_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractStringLiteral(std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    if (not (fixedView.front() == '\"')) return std::nullopt;

    // Scan.
    Uint slashCount = 0;
    auto endPosIt = std::find_if(fixedView.begin() + 1, fixedView.end(), [&slashCount] (char c) -> bool
    {
        // Exit checks.
        if (c == '\"' and (slashCount % 2) == 0) return true;

        // Normal Checks.
        if (c == '\\') slashCount++;
        else slashCount = 0;

        return false;
    });
    if (endPosIt == fixedView.end()) throw Lexer::Generator::Error("String literal does not end at current line");
    Uint endPos = std::distance(fixedView.begin(), endPosIt) + std::strlen("\"");
    std::string_view stringLiteral = fixedView.substr(0, endPos);

    // Incrementation & return.
    std::string_view content = stringLiteral;
    view.remove_prefix(endPos);
    return Lexer::Token(Lexer::Tag::STRING_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractCharLiteral(std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    if (not (fixedView.front() == '\'')) return std::nullopt;

    // Scan.
    Uint slashCount = 0;
    auto endPosIt = std::find_if(fixedView.begin() + 1, fixedView.end(), [&slashCount](char c) -> bool
    {
        // Exit checks.
        if (c == '\'' and (slashCount % 2) == 0) return true;

        // Normal Checks.
        if (c == '\\') slashCount++;
        else slashCount = 0;

        return false;
    });
    if (endPosIt == fixedView.end())
    {
        throw Lexer::Generator::Error("Character literal does not end at current line");
    }
    Uint endPos = std::distance(fixedView.begin(), endPosIt) + std::strlen("\'");
    std::string_view charLiteral = fixedView.substr(0, endPos);
    if (charLiteral.size() >= 2 // Bounds checking.
        and charLiteral.size() > (std::strlen("'a'") + (charLiteral[1] == '\\'))) // Checks if it's bypassing the length of 'a' or the length of '\n' if there was a '\' before.
        throw Lexer::Generator::Error("Character literal is more than character"); // I did not mistake with "'a" it doesn't end with an ' on purpose.

    std::string_view strippedCharLiteral = charLiteral.substr(1, charLiteral.size() - 2);
    if (strippedCharLiteral.size() == 2 and strippedCharLiteral[0] == '\\')
    {
        switch (strippedCharLiteral[1])
        {
        case 'a':  break;
        case 'b':  break;
        case 'f':  break;
        case 'n':  break;
        case 'r':  break;
        case 't':  break;
        case 'v':  break;
        case '\'': break;
        case '"':  break;
        case '?':  break;
        case '\\': break;
        case '0':  break;
        default:
            throw Lexer::Generator::Error("Invalid escape character");
        }
    }

    // Incrementation & return.
    std::string_view content = charLiteral;
    view.remove_prefix(endPos);
    return Lexer::Token(Lexer::Tag::CHAR_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractHexLiteral(std::string_view& view)
{
    // Forced Code.
    Uint totalSize = 0;
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early errors/return.
    if ((not fixedView.starts_with("0x") and not fixedView.starts_with("0X"))) return std::nullopt;
    if (fixedView.size() <= 2) throw Lexer::Generator::Error("Invalid hexadecimal literal");
    totalSize += std::strlen("0x");

    // Scan.
    std::string_view afterSignature = fixedView.substr(totalSize);
    for (Uint i = 0; i < afterSignature.size(); i++, totalSize++)
    {
        unsigned char c = static_cast<unsigned char>(afterSignature[i]);

        // Exit checks.
        if (not std::isalnum(c)) break;

        // Error checks.
        if (std::string_view("0123456789ABCDEFabcdef").find(c) == std::string_view::npos) throw Lexer::Generator::Error("Invalid hexadecimal literal");
    }

    // Incrementation & return.
    std::string_view content = fixedView.substr(0, totalSize);
    view.remove_prefix(content.size());
    return Lexer::Token(Lexer::Tag::HEX_LITERAL, content);
}

std::optional<Lexer::Token> Lexer::Generator::extractBinLiteral(std::string_view& view)
{
    // Forced Code.
    Uint totalSize = 0;
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    if ((not fixedView.starts_with("0b") and not fixedView.starts_with("0B"))) return std::nullopt;
    if (fixedView.size() <= 2) throw Lexer::Generator::Error("Invalid binary literal");
    totalSize += std::strlen("0b");

    // Scan.
    std::string_view afterSignature = fixedView.substr(totalSize);
    for (Uint i = 0; i < afterSignature.size(); i++, totalSize++)
    {
        unsigned char c = static_cast<unsigned char>(afterSignature[i]);

        // Exit checks.
        if (not std::isalnum(c)) break;

        // Error checks.
        if (c != '0' and c != '1') throw Lexer::Generator::Error("Invalid binary literal");
    }

    // Incrementation & return.
    std::string_view content = fixedView.substr(0, totalSize);
    view.remove_prefix(content.size());
    return Lexer::Token(Lexer::Tag::BIN_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractOctLiteral(std::string_view& view)
{
    // Forced Code.
    Uint totalSize = 0;
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    if ((not fixedView.starts_with("0o") and not fixedView.starts_with("0O"))) return std::nullopt;
    if (fixedView.size() <= 2) throw Lexer::Generator::Error("Invalid octal literal");
    totalSize += std::strlen("0o");

    // Scan.
    std::string_view afterSignature = fixedView.substr(totalSize);
    for (Uint i = 0; i < afterSignature.size(); i++, totalSize++)
    {
        unsigned char c = static_cast<unsigned char>(afterSignature[i]);

        // Exit checks.
        if (not std::isalnum(c)) break;

        // Error checks.
        if (std::string_view("01234567").find(c) == std::string_view::npos) throw Lexer::Generator::Error("Invalid octal literal");
    }

    // Incrementation & return.
    std::string_view content = fixedView.substr(0, totalSize);
    view.remove_prefix(content.size());
    return Lexer::Token(Lexer::Tag::OCT_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractSciLiteral(std::string_view& view)
{
    // Forced Code.
    Uint totalSize = 0;
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    if (fixedView.size() <= 2) return std::nullopt;
    if (fixedView.front() == '.' and not std::isdigit(static_cast<unsigned char>(fixedView[1]))) return std::nullopt; // '.a'
    if (fixedView[1] == '.' and not std::isdigit(static_cast<unsigned char>(fixedView.front()))) return std::nullopt; // 'a.'
    if (fixedView.front() != '.' and not std::isdigit(static_cast<unsigned char>(fixedView.front()))) return std::nullopt; // 'a'

    // Scan 1.
    bool seenE = false;
    bool seenDot = false;
    for (Uint i = 0; i < fixedView.size(); i++, totalSize++)
    {
        unsigned char c = std::tolower(static_cast<unsigned char>(fixedView[i]));

        // Normal checks.
        if (c == 'e')
        {
            seenE = true;
            break;
        }
        if (c == '.')
        {
            if (seenDot)
            {
                throw Lexer::Generator::Error("Invalid float literal");
            }
            seenDot = true;
            continue;
        }

        // Exit checks.
        if (not std::isalnum(c)) break;

        // Error checks.
        if (std::isalpha(c))
        {
            if (seenDot)
            {
                throw Lexer::Generator::Error("Invalid float literal");
            }
            throw Lexer::Generator::Error("Invalid integer literal"); // If there was no dot it means it's an int literal.
        }
    }
    if (not seenE) return std::nullopt;
    totalSize += std::strlen("e");

    std::string_view afterE = fixedView.substr(totalSize);
    if (afterE.empty()) throw Lexer::Generator::Error("Invalid scientific notation literal");

    // Scan 2.
    if (afterE.front() == '+' or afterE.front() == '-')
    {
        afterE.remove_prefix(1);
        totalSize++;
    }
    bool seen = false;
    for (Uint i = 0; i < afterE.size(); i++, totalSize++)
    {
        unsigned char c = std::tolower(static_cast<unsigned char>(afterE[i]));

        // Exit checks.
        if (not std::isalnum(c)) break;

        // Error checks.
        if (std::isalpha(c)) throw Lexer::Generator::Error("Invalid scientific notation literal");

        seen = true;
    }
    if (not seen) throw Lexer::Generator::Error("Invalid scientific notation literal");

    // Incrementation & return.
    std::string_view content = fixedView.substr(0, totalSize);
    view.remove_prefix(content.size());
    return Lexer::Token(Lexer::Tag::SCI_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractFloatLiteral(std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    if (fixedView.size() <= 1) return std::nullopt;
    if (fixedView.front() == '.' and not std::isdigit(static_cast<unsigned char>(fixedView[1]))) return std::nullopt; // '.a'
    if (fixedView[1] == '.' and not std::isdigit(static_cast<unsigned char>(fixedView.front()))) return std::nullopt; // 'a.'
    if (fixedView.front() != '.' and not std::isdigit(static_cast<unsigned char>(fixedView.front()))) return std::nullopt; // 'a'

    // Scan.
    bool seenDot = false;
    Uint i = 0;
    for (i = 0; i < fixedView.size(); i++)
    {
        unsigned char c = static_cast<unsigned char>(fixedView[i]);

        // Normal checks.
        if (c == '.')
        {
            if (seenDot)
            {
                throw Lexer::Generator::Error("Invalid float literal");
            }
            seenDot = true;
            continue;
        }

        // Exit checks.
        if (not std::isalnum(c)) break;

        // Error checks.
        if (std::isalpha(c))
        {
            if (seenDot)
            {
                throw Lexer::Generator::Error("Invalid float literal");
            }
            throw Lexer::Generator::Error("Invalid integer literal"); // If there was no dot it means it's an int literal.
        }
    }
    if (not seenDot) return std::nullopt;

    // Incrementation & return.
    std::string_view content = fixedView.substr(0, i);
    view.remove_prefix(content.size());
    return Lexer::Token(Lexer::Tag::FLOAT_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractIntLiteral(std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    if (not std::isdigit(static_cast<unsigned char>(fixedView.front()))) return std::nullopt;

    // Scan.
    Uint i = 0;
    for (i = 0; i < fixedView.size(); i++)
    {
        unsigned char c = static_cast<unsigned char>(fixedView[i]);

        // Error checks.
        if (std::isalpha(c)) throw Lexer::Generator::Error("Invalid integer literal");

        // Exit checks.
        if (not std::isalnum(c)) break;
    }

    // Incrementation & return.
    std::string_view content = fixedView.substr(0, i);
    view.remove_prefix(content.size());
    return Lexer::Token(Lexer::Tag::INT_LITERAL, content);
}
std::optional<Lexer::Token> Lexer::Generator::extractBoolLiteral(std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    std::string_view possibleNone;
    if (auto opt = Lexer::Generator::extractUntilNotAlnum(fixedView)) possibleNone = opt.value();
    else return std::nullopt;

    // Scan.
    if (possibleNone == "True" or possibleNone == "False")
    {
        /// Incrementation & return.
        std::string_view content = possibleNone;
        view.remove_prefix(content.size());
        return Lexer::Token(Lexer::Tag::BOOL_LITERAL, content);
    }

    // Return default.
    return std::nullopt;
}
std::optional<Lexer::Token> Lexer::Generator::extractNoneLiteral(std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    std::string_view possibleNone;
    if (auto opt = Lexer::Generator::extractUntilNotAlnum(fixedView)) possibleNone = opt.value();
    else return std::nullopt;

    // Scan.
    if (possibleNone == "None")
    {
        // Incrementation & return.
        std::string_view content = possibleNone;
        view.remove_prefix(content.size());
        return Lexer::Token(Lexer::Tag::NONE_LITERAL, content);
    }

    // Return default.
    return std::nullopt;
}
std::optional<Lexer::Token> Lexer::Generator::extractSymbol(std::string_view& view, Uint& depthClosingCountParentheses, Uint& depthClosingCountBracket, Uint& depthClosingCountBrace)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Data.
    static constexpr auto wordSymbols = std::to_array<std::string_view>(
    {
        "and", "or", "not", "is", "as", "sizeof", "ref", "dref"
    });
    static constexpr auto punctuator3Symbols = std::to_array<std::string_view>(
    {
        "<<=", ">>=", "..."
    });
    static constexpr auto punctuator2Symbols = std::to_array<std::string_view>(
    {
        "+=", "-=", "*=", "/=", "%=", ">=", "<=", ">>", "<<", "|=", "&=", "^=", "==", "!=", "->"
    });
    static constexpr auto punctuator1symbols = std::to_array<char>(
    {
        '+', '-', '*', '/', '%', '<', '>', '|', '&', '^', '~', '=', '.', ',', '(', ')', '[', ']', '{', '}', ':'
    });

    // Scan 1.
    std::string_view possibleWordSymbol;
    if (auto opt = Lexer::Generator::extractUntilNotAlnum(fixedView))
    {
        possibleWordSymbol = opt.value();
        for (auto& wordSymbol : wordSymbols)
        {
            if (possibleWordSymbol == wordSymbol)
            {
                // Incrementation & return.
                std::string_view content = possibleWordSymbol;
                view.remove_prefix(content.size());
                return Token(Lexer::Tag::SYMBOL, content);
            }
        }
        return std::nullopt;
    }
    // Scan 2.
    for (auto& punctuator3Symbol : punctuator3Symbols)
    {
        if (fixedView.starts_with(punctuator3Symbol))
        {
            constexpr Uint OFFSET = 3;

            // Incrementation & return.
            std::string_view content = fixedView.substr(0, OFFSET);
            view.remove_prefix(OFFSET);
            return Token(Lexer::Tag::SYMBOL, content);
        }
    }
    // Scan 3.
    for (auto& punctuator2Symbol : punctuator2Symbols)
    {
        if (fixedView.starts_with(punctuator2Symbol))
        {
            constexpr Uint OFFSET = 2;

            // Incrementation & return.
            std::string_view content = fixedView.substr(0, OFFSET);
            view.remove_prefix(OFFSET);
            return Token(Lexer::Tag::SYMBOL, content);
        }
    }
    // Scan 4.
    for (char punctuator1Symbol : punctuator1symbols)
    {
        if (fixedView.starts_with(punctuator1Symbol))
        {
            if (punctuator1Symbol == '(') depthClosingCountParentheses++;
            else if (punctuator1Symbol == ')') 
            {
                if (depthClosingCountParentheses <= 0) throw Lexer::Generator::Error("Invalid depth of Parentheses");
                depthClosingCountParentheses--; 
            }
            else if (punctuator1Symbol == '[') depthClosingCountBracket++;
            else if (punctuator1Symbol == ']') 
            {
                if (depthClosingCountBracket <= 0) throw Lexer::Generator::Error("Invalid depth of Brackets");
                depthClosingCountBracket--;
            }
            else if (punctuator1Symbol == '{') depthClosingCountBrace++;
            else if (punctuator1Symbol == '}')
            {
                if (depthClosingCountBrace <= 0) throw Lexer::Generator::Error("Invalid depth of Brackets");
                depthClosingCountBrace--;
            }

            constexpr Uint OFFSET = 1;

            // Incrementation & return.
            std::string_view content = fixedView.substr(0, OFFSET);
            view.remove_prefix(OFFSET);
            return Token(Lexer::Tag::SYMBOL, content);
        }
    }

    // Return default.
    return std::nullopt;
}
std::optional<Lexer::Token> Lexer::Generator::extractKeyword(std::string_view& view)
{
    // Forced code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Data.
    static constexpr auto keywords = std::to_array<std::string_view>(
    {
        "if", "elif", "else",
        "for", "while", "break", "continue", "return",
        "def", "struct", "typedef", "def_extern",
        "bool", "char",
        "int", "float", "ptr",
    });

    // Early return.
    std::string_view possibleKeyword;
    if (auto opt = Lexer::Generator::extractUntilNotAlnum(fixedView)) possibleKeyword = opt.value();
    else return std::nullopt;

    // Scan.
    for (Uint i = 0; i < keywords.size(); i++)
    {
        if (keywords[i] == possibleKeyword)
        {
            // Incrementation & return.
            view.remove_prefix(possibleKeyword.size());
            return Token(Lexer::Tag::KEYWORD, possibleKeyword);
        }
    }

    // Return default.
    return std::nullopt;
}
std::optional<Lexer::Token> Lexer::Generator::extractNewLine(std::string_view& view)
{
    std::string_view tempView = view;
    bool isValid = false;

    // Scan 1.
    while (tempView.size() > 0)
    {
        if (tempView.front() == '\n')
        {
            tempView.remove_prefix(1);
            isValid = true;
            break;
        }
        if (tempView.front() != ' ' and tempView.front() != '\t')
        {
            break;
        }

        tempView.remove_prefix(1);
    }

    // Scan 2.
    if (isValid)
    {
        // Incrementation & return.
        view = tempView;
        return Lexer::Token(Lexer::Tag::NEW_LINE);
    }

    return std::nullopt;
}
std::optional<std::vector<Lexer::Token>> Lexer::Generator::extractInDedent(std::string_view& view, std::stack<Uint>& identLevels)
{
    // Early return.
    Uint newLevel;
    if (auto opt = Lexer::Generator::extractSpacesLevel(view)) newLevel = opt.value();
    else return std::nullopt;
    if (identLevels.empty() and newLevel == 0)
    {
        return std::nullopt;
    }

    // Scan 1.
    if (identLevels.empty() or newLevel > identLevels.top()) 
    {
        identLevels.push(newLevel);
        return std::vector<Lexer::Token>{ Lexer::Token(Lexer::Tag::INDENT) };
    }
    else if (identLevels.top() == newLevel)
    {
        return std::nullopt;
    }

    // Scan 2.
    std::vector<Lexer::Token> dedents;
    while (not identLevels.empty() and identLevels.top() > newLevel) 
    {
        identLevels.pop();
        dedents.emplace_back(Lexer::Tag::DEDENT);
    }
    if ((identLevels.empty() or identLevels.top() != newLevel) and newLevel != 0) throw Lexer::Generator::Error("Indent (spacing) doesn't match previous indents");

    // Return.
    return dedents;
}
std::optional<Lexer::Token> Lexer::Generator::extractIdentifier(std::string_view& view)
{
    // Forced code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return/errors.
    if (fixedView.front() == '#') return std::nullopt;
    if (not std::isalnum(static_cast<unsigned char>(fixedView.front())) and fixedView.front() != '_')
        throw Lexer::Generator::Error("Invalid character");

    // Extraction.
    std::string_view content;
    if (auto opt = Lexer::Generator::extractUntilNotAlnum(fixedView)) content = opt.value();
    else return std::nullopt;

    // Incrementation & return.
    view.remove_prefix(content.size());
    return Lexer::Token(Lexer::Tag::IDENTIFIER, content);
}
std::optional<std::vector<Lexer::Token>> Lexer::Generator::extractImport(std::string_view& view)
{
    // Forced Code.
    std::string_view fixedView;
    if (auto opt = Lexer::Generator::extractUntilNewLine(view)) fixedView = opt.value();
    else return std::nullopt;

    // Early return.
    std::string_view possibleImport;
    if (auto opt = Lexer::Generator::extractUntilNotAlnum(fixedView)) possibleImport = opt.value();
    else return std::nullopt;

    if (possibleImport == "import")
    {
        std::string_view tempView = fixedView.substr(std::strlen("import"));
        Lexer::Generator::skipSpaces(tempView);
        if (auto opt = Lexer::Generator::extractStringLiteral(tempView))
        {
            Lexer::Generator::skipSpaces(tempView);
            if (not tempView.empty() and not tempView.starts_with("#"))
                throw Lexer::Generator::Error("Import have left overs");
            std::string_view filenameView = opt.value().content;
            filenameView.remove_prefix(1);
            filenameView.remove_suffix(1);

            auto normalized = std::filesystem::path(filenameView);
            normalized.make_preferred();
            std::string normalizedFilename = normalized.string();
            if (std::find(Lexer::Generator::filenames.begin(), Lexer::Generator::filenames.end(), normalizedFilename) == Lexer::Generator::filenames.end())
            {
                Lexer::Generator::lexers.emplace_back(Helper::extractFileContent(normalizedFilename), normalizedFilename);

                // Incrementation & return.
                while (not view.empty() and view.front() != '\n') view.remove_prefix(1);
                return lexers.back().copy();
            }

            while (not view.empty() and view.front() != '\n') view.remove_prefix(1);
            return std::nullopt;
        }
    }

    return std::nullopt;
}

namespace Lexer
{
    std::ostream& operator << (std::ostream& stream, const Lexer::Generator& generator)
    {
        std::copy(generator.begin(), generator.end(), std::ostream_iterator<Lexer::Token>(stream));

        // Return.
        return stream;
    }
}

Lexer::Generator::Error::Error(const char* new_error)
    : error(new_error)
{
}
