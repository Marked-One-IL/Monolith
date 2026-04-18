#pragma once
#include <Parser/Generator.hpp>
#include <string_view>
#include <map>
#include <memory>

// This doesn't just link identifiers but also act as an scope use of stuff.
namespace Semantic
{
    class IdentifierLinker
    {
    public:
        static void link(Parser::Generator& parser);

    private:
        struct Error
        {
            Error(const char* new_error, const Parser::AST::Base* new_ast);

            const char* error = nullptr;
            const Parser::AST::Base* ast = nullptr;
        };

        // Don't let the 'const' Parser::AST...Base* fool you.
        // What I change has a mutable keyword to it.
        // This is just to avoid annoying casting.
        static void linkerBlock(const Parser::AST::Block* block, std::map<std::string_view, const Parser::AST::Decl::Base*> table, bool isInLoopStmt, bool isInFunc);
        static void linkerExpr(const Parser::AST::Base* expr, const std::map<std::string_view, const Parser::AST::Decl::Base*>& table);
        static void linkerType(const Parser::AST::Type::Base* type, const std::map<std::string_view, const Parser::AST::Decl::Base*>& table,
                               const Parser::AST::Base* origin); // For throwing.
    };
}
