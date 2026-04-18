#pragma once
#include <Parser/AST/Base.hpp>
#include <Lexer/Generator.hpp>
#include <Parser/AST/Decl/Var.hpp>
#include <Parser/AST/Decl/Typedef.hpp>
#include <Parser/AST/Decl/Func.hpp>
#include <Parser/AST/Decl/Struct.hpp>
#include <Parser/AST/Stmt/Assign.hpp>
#include <Parser/AST/Stmt/If.hpp>
#include <Parser/AST/Stmt/While.hpp>
#include <Parser/AST/Stmt/For.hpp>
#include <Parser/AST/Stmt/Break.hpp>
#include <Parser/AST/Stmt/Continue.hpp>
#include <Parser/AST/Stmt/Return.hpp>
#include <Helper/Types.hpp>
#include <vector>
#include <ostream>
#include <utility>

namespace Parser
{
    class Generator
    {
    public:
        Generator(const Lexer::Generator& lexer);

        const Parser::AST::Block* getHeadBlock(void) const;
        friend std::ostream& operator << (std::ostream& stream, const Parser::Generator& parser);

    private:
        struct Error
        {
            Error(const char* new_error, Uint new_tokPos);

            const char* error = nullptr;
            const Uint tokPos = 0;
        };

        struct MutableField
        {
            MutableField(std::string_view new_name, std::unique_ptr<const Parser::AST::Type::Base> new_type);

            const std::string_view name;
            std::unique_ptr<const Parser::AST::Type::Base> type;
        };

        static Uint findEndOff(const Lexer::Generator& lexer, Uint startOff);

        static std::pair<std::unique_ptr<const Parser::AST::Block>, Uint> parseBlock(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel, bool exceptIndent);
        static std::pair<std::unique_ptr<const Parser::AST::Base>, Uint> parseGeneral(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
                         
        static std::pair<std::unique_ptr<const Parser::AST::Stmt::If>, Uint> extractIf(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Stmt::While>, Uint> extractWhile(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Stmt::For>, Uint> extractFor(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Stmt::Assign>, Uint> extractAssign(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::unique_ptr<const Parser::AST::Stmt::Assign> extractAssignStrict(const Lexer::Generator& lexer, Uint startOff, Uint endOff);
        static std::pair<std::unique_ptr<const Parser::AST::Stmt::Break>, Uint> extractBreak(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Stmt::Continue>, Uint> extractContinue(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Stmt::Return>, Uint> extractReturn(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Decl::Var>, Uint> extractVar(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<Parser::Generator::MutableField, Uint> extractFieldMutable(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Decl::Typedef>, Uint> extractTypedef(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Decl::Func>, Uint> extractFunc(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Decl::Struct>, Uint> extractStruct(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel);
        static std::pair<std::unique_ptr<const Parser::AST::Expr::Base>, Uint> extractExpr(const Lexer::Generator& lexer, Uint startOff, Uint indentLevel); // Let users do random expressions or function calls.

        std::unique_ptr<const Parser::AST::Block> m_headBlock;
    };
}
