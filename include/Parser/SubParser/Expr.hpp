#pragma once
#include <Parser/AST/Expr/Base.hpp>
#include <Lexer/Generator.hpp>
#include <Lexer/Token.hpp>
#include <Helper/Types.hpp>
#include <optional>
#include <memory>

namespace Parser::SubParser
{
    class Expr
    {
    public:
        struct Return
        {
            Return(std::unique_ptr<const Parser::AST::Expr::Base> new_expr, Uint new_endOff);
            Return(Parser::SubParser::Expr::Return&& other);
            Parser::SubParser::Expr::Return& operator = (Parser::SubParser::Expr::Return&& other);
            operator bool(void) const;

            std::unique_ptr<const Parser::AST::Expr::Base> expr;
            Uint endOff = 0;
        };

        static std::unique_ptr<const Parser::AST::Expr::Base> parse(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Expr::Return parseEndOff(const Lexer::Generator& lexer, Uint startOff, Uint endOff);

    private:
        struct Error
        {
            Error(const char* new_error, Uint new_tokPos);

            const char* error = nullptr;
            const Uint tokPos = 0;
        };

        enum class Level : uint8_t
        {
            NONE,
            LEVEL0,
            LEVEL1,
            LEVEL2,
            LEVEL3,
            LEVEL4,
            LEVEL5,
            LEVEL6,
            LEVEL7,
            LEVEL8,
            LEVEL9
        };

        static Parser::SubParser::Expr::Return parseBase(const Lexer::Generator& lexer, Uint startOff, Uint endOff, Parser::SubParser::Expr::Level prevLevel);
        static Parser::SubParser::Expr::Level getOperatorLevel(const Lexer::Generator& lexer, Uint tokPos);

        static Parser::SubParser::Expr::Return handleSingle(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Expr::Return handleSubExpr(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Expr::Return handleValue(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Expr::Return handleUnary(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static Parser::SubParser::Expr::Return handleSizeof(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
    };
}
