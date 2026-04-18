#pragma once
#include <ostream>
#include <Helper/Types.hpp>
#include <Lexer/Token.hpp>

namespace Parser::AST
{
    enum class Tag : uint8_t
    {
        // Statements.
        STMT_IF,
        STMT_WHILE,
        STMT_FOR,
        STMT_BREAK,
        STMT_CONTINUE,
        STMT_RETURN,
        STMT_ASSIGN,

        // Declarations.
        DECL_VAR,
        DECL_FUNC,
        DECL_STRUCT,
        DECL_TYPEDEF,
        // Specific Declarations.
        DECL_PARAM,
        DECL_FIELD,

        // Expressions.
        EXPR_LITERAL,
        EXPR_IDENTIFIER,
        EXPR_BIN,
        EXPR_UNARY,
        EXPR_CALL,
        EXPR_ARR_ACCESS,
        EXPR_STRUCT_ACCESS,
        EXPR_SIZEOF,
        EXPR_CAST,
        EXPR_ARR_INIT,
        EXPR_STRUCT_INIT
    };

    struct LocationData
    {
        LocationData(const Lexer::LocationData& new_locationData);

        const std::string_view line;
        const Uint lineNum = 0;
        const char* filename = nullptr;
    };

    struct Base
    {
        Base(Parser::AST::Tag new_tag, const Lexer::LocationData& new_locationData);
        virtual ~Base(void) = default;

        void print(std::ostream& stream, Uint depth) const;
        const Parser::AST::Tag tag;
        const Parser::AST::LocationData locationData;
    };
}
