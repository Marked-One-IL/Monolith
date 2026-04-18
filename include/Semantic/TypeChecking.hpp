#pragma once
#include <Parser/Generator.hpp>
#include <Parser/AST/Decl/Func.hpp>
#include <Parser/AST/Stmt/Assign.hpp>

namespace Semantic
{
    class TypeChecking
    {
    public:
        static void check(const Parser::Generator& parser);

        struct Error
        {
            Error(const char* new_error, const Parser::AST::Base* new_ast);

            const char* error = nullptr;
            const Parser::AST::Base* ast = nullptr;
        };

    private:
        static void checkBlock(const Parser::AST::Block* block, const Parser::AST::Decl::Func* currentFunc, bool isInLoopStmt = false);
        static bool isLeftAssignCorrect(const Parser::AST::Expr::Base* expr);
        static void checkSingleAssign(const Parser::AST::Stmt::Assign* assign);
    };
}
