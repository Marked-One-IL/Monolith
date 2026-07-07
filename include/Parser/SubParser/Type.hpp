#pragma once
#include <Parser/AST/Type/Base.hpp>
#include <Lexer/Generator.hpp>
#include <Helper/Types.hpp>
#include <memory>

namespace Parser::SubParser
{
    class Type
    {
    public:
        struct Return
        {
            Return(std::unique_ptr<const Parser::AST::Type::Base> new_type, Uint new_endOff);
            Return(Parser::SubParser::Type::Return&& other);
            Parser::SubParser::Type::Return& operator = (Parser::SubParser::Type::Return&& other);
            operator bool(void) const;

            std::unique_ptr<const Parser::AST::Type::Base> type;
            Uint endOff = 0;
        };

        static Parser::SubParser::Type::Return parse(const Lexer::Generator& lexer, Uint startOff, Uint endOff);

    private:
        struct Error
        {
            Error(const char* new_error, Uint new_tokPos);

            const char* error = nullptr;
            const Uint tokPos = 0;
        };

        static Parser::SubParser::Type::Return parseGeneral(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Type::Return extractPrimitive(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Type::Return extractIdentifier(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Type::Return extractPointer(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Type::Return extractArray(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Type::Return extractFunction(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
    };
}
