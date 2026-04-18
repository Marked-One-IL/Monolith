#include <Semantic/IdentifierLinker.hpp>
#include <Parser/AST/Expr/ArrAccess.hpp>
#include <Parser/AST/Expr/Bin.hpp>
#include <Parser/AST/Expr/Call.hpp>
#include <Parser/AST/Expr/StructAccess.hpp>
#include <Parser/AST/Expr/Identifier.hpp>
#include <Parser/AST/Expr/Literal.hpp>
#include <Parser/AST/Expr/Unary.hpp>
#include <Parser/AST/Expr/Cast.hpp>
#include <Parser/AST/Expr/Sizeof.hpp>
#include <Parser/AST/Expr/ArrInit.hpp>
#include <Parser/AST/Expr/StructInit.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <Parser/AST/Type/Primitive.hpp>
#include <Parser/AST/Type/Typedef.hpp>
#include <Parser/AST/Type/Ptr.hpp>
#include <Parser/AST/Type/Arr.hpp>
#include <Parser/AST/Type/Struct.hpp>
#include <Parser/AST/Type/Func.hpp>
#include <Helper/Assert.hpp>

void Semantic::IdentifierLinker::link(Parser::Generator& parser)
{
    try
    {
        std::map<std::string_view, const Parser::AST::Decl::Base*> table;
        Semantic::IdentifierLinker::linkerBlock(parser.getHeadBlock(), table, false, false);
    }
    catch (const Semantic::IdentifierLinker::Error& error)
    {
        Assert(error.ast);

        throw std::runtime_error(std::format("Semantic Identifier Linker:\nAt file: {}\nLine: {}\nError: {}\n{}",
                                             error.ast->locationData.filename, error.ast->locationData.lineNum, error.error,
                                             error.ast->locationData.line));
    }
}

Semantic::IdentifierLinker::Error::Error(const char* new_error, const Parser::AST::Base* new_ast) :
    error(new_error), ast(new_ast)
{
}

// Don't let the 'const' Parser::AST...Base* fool you.
// What I change has a mutable keyword to it.
// This is just to avoid annoying casting.
void Semantic::IdentifierLinker::linkerBlock(const Parser::AST::Block* block,
                                             std::map<std::string_view, const Parser::AST::Decl::Base*> table,
                                             bool isInLoopStmt, bool isInFunc) // "Your not supposed to check for this in this section actually". Fuck you.
{
    if (block == nullptr) return;

    for (auto& ast : block->commands)
    {
        if (ast.get() == nullptr) continue;

        switch (ast->tag)
        {
        case Parser::AST::Tag::STMT_IF:
        {
            const Parser::AST::Stmt::If* castedType = static_cast<const Parser::AST::Stmt::If*>(ast.get());

            while (castedType)
            {
                const Parser::AST::Expr::Base* cond = castedType->cond.get();
                Semantic::IdentifierLinker::linkerExpr(cond, table);

                const Parser::AST::Block* then = castedType->then.get();
                Semantic::IdentifierLinker::linkerBlock(then, table, isInLoopStmt, isInFunc);

                castedType = castedType->els.get();
            }
            break;
        }
        case Parser::AST::Tag::STMT_WHILE:
        {
            const Parser::AST::Stmt::While* castedType = static_cast<const Parser::AST::Stmt::While*>(ast.get());
            
            const Parser::AST::Expr::Base* cond = castedType->cond.get();
            Semantic::IdentifierLinker::linkerExpr(cond, table);

            const Parser::AST::Block* then = castedType->then.get();
            Semantic::IdentifierLinker::linkerBlock(then, table, true, isInFunc);
            break;
        }
        case Parser::AST::Tag::STMT_FOR:
        {
            const Parser::AST::Stmt::For* castedType = static_cast<const Parser::AST::Stmt::For*>(ast.get());

            const Parser::AST::Stmt::Assign* init = castedType->init.get();
            Semantic::IdentifierLinker::linkerExpr(init->dst.get(), table);
            Semantic::IdentifierLinker::linkerExpr(init->src.get(), table);

            const Parser::AST::Expr::Base* cond = castedType->cond.get();
            Semantic::IdentifierLinker::linkerExpr(cond, table);

            const Parser::AST::Stmt::Assign* iter = castedType->iter.get();
            Semantic::IdentifierLinker::linkerExpr(iter->dst.get(), table);
            Semantic::IdentifierLinker::linkerExpr(iter->src.get(), table);

            const Parser::AST::Block* then = castedType->then.get();
            Semantic::IdentifierLinker::linkerBlock(then, table, true, isInFunc);
            break;
        }
        case Parser::AST::Tag::STMT_BREAK:
        {
            if (not isInLoopStmt) throw Semantic::IdentifierLinker::Error("Inappropriate location of break", ast.get());

            break;
        }
        case Parser::AST::Tag::STMT_CONTINUE:
        {
            if (not isInLoopStmt) throw Semantic::IdentifierLinker::Error("Inappropriate location of continue", ast.get());

            break;
        }

        case Parser::AST::Tag::STMT_RETURN:
        {
            if (not isInFunc) throw Semantic::IdentifierLinker::Error("Inappropriate location of return", ast.get());

            const Parser::AST::Stmt::Return* castedType = static_cast<const Parser::AST::Stmt::Return*>(ast.get());

            Semantic::IdentifierLinker::linkerExpr(castedType->expr.get(), table);
            break;
        }
        case Parser::AST::Tag::STMT_ASSIGN:
        {
            const Parser::AST::Stmt::Assign* castedType = static_cast<const Parser::AST::Stmt::Assign*>(ast.get());

            Semantic::IdentifierLinker::linkerExpr(castedType->dst.get(), table);
            Semantic::IdentifierLinker::linkerExpr(castedType->src.get(), table);
            break;
        }

        case Parser::AST::Tag::DECL_VAR:
        {
            const Parser::AST::Decl::Var* castedType = static_cast<const Parser::AST::Decl::Var*>(ast.get());

            if (table.find(castedType->name) != table.end()) throw Semantic::IdentifierLinker::Error("Variable redefination", castedType);
            Semantic::IdentifierLinker::linkerType(castedType->type.get(), table, castedType);
            Semantic::IdentifierLinker::linkerExpr(castedType->init.get(), table);
            table.emplace(castedType->name, castedType);
            break;
        }
        case Parser::AST::Tag::DECL_FUNC:
        {
            const Parser::AST::Decl::Func* castedType = static_cast<const Parser::AST::Decl::Func*>(ast.get());

            if (table.find(castedType->name) != table.end()) throw Semantic::IdentifierLinker::Error("Function redefination", castedType);
            table.emplace(castedType->name, castedType); // Early emplace to allow recursion.
            Semantic::IdentifierLinker::linkerType(castedType->data.get(), table, castedType);
            std::map<std::string_view, const Parser::AST::Decl::Base*> newMap = table;

            for (auto& param : castedType->paramsDecl)
            {
                if (newMap.find(param.name) != newMap.end()) throw Semantic::IdentifierLinker::Error("Parameter redefination", castedType);
                newMap.emplace(param.name, &param);
            }

            Semantic::IdentifierLinker::linkerBlock(castedType->block.get(), newMap, isInLoopStmt, true);
            break;
        }
        case Parser::AST::Tag::DECL_STRUCT:
        {
            const Parser::AST::Decl::Struct* castedType = static_cast<const Parser::AST::Decl::Struct*>(ast.get());

            if (table.find(castedType->name) != table.end()) throw Semantic::IdentifierLinker::Error("Struct redefination", castedType);
            table.emplace(castedType->name, castedType);
            std::map<std::string_view, const Parser::AST::Decl::Base*> newMap;

            for (auto& field : castedType->fieldsDecl)
            {
                if (newMap.find(field.name) != newMap.end()) throw Semantic::IdentifierLinker::Error("Field redefination", castedType);
                Semantic::IdentifierLinker::linkerType(field.type, table, castedType);
                newMap.emplace(field.name, &field);
            }
            break;
        }
        case Parser::AST::Tag::DECL_TYPEDEF:
        {
            const Parser::AST::Decl::Typedef* castedType = static_cast<const Parser::AST::Decl::Typedef*>(ast.get());

            if (table.find(castedType->name) != table.end()) throw Semantic::IdentifierLinker::Error("Typedef redefination", castedType);
            Semantic::IdentifierLinker::linkerType(castedType->typeDef.get(), table, castedType);
            table.emplace(castedType->name, castedType);
            break;
        }

        case Parser::AST::Tag::EXPR_LITERAL:
        case Parser::AST::Tag::EXPR_IDENTIFIER:
        case Parser::AST::Tag::EXPR_BIN:
        case Parser::AST::Tag::EXPR_UNARY:
        case Parser::AST::Tag::EXPR_CALL:
        case Parser::AST::Tag::EXPR_ARR_ACCESS:
        case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
        case Parser::AST::Tag::EXPR_SIZEOF:
        case Parser::AST::Tag::EXPR_CAST:
        case Parser::AST::Tag::EXPR_ARR_INIT:
        case Parser::AST::Tag::EXPR_STRUCT_INIT:
            Semantic::IdentifierLinker::linkerExpr(ast.get(), table);
            break;
        }
    }
}

void Semantic::IdentifierLinker::linkerExpr(const Parser::AST::Base* expr, const std::map<std::string_view, const Parser::AST::Decl::Base*>& table)
{
    if (expr == nullptr) return;

    switch (expr->tag)
    {
        case Parser::AST::Tag::EXPR_LITERAL: break;
        case Parser::AST::Tag::EXPR_IDENTIFIER:
        {
            const Parser::AST::Expr::Identifier* castedType = static_cast<const Parser::AST::Expr::Identifier*>(expr);
            auto it = table.find(castedType->name);

            if (it == table.end()) throw Semantic::IdentifierLinker::Error("Unknown expression identifier", castedType);

            if (it->second->tag != Parser::AST::Tag::DECL_VAR and it->second->tag != Parser::AST::Tag::DECL_FUNC and it->second->tag != Parser::AST::Tag::DECL_PARAM)
                throw Semantic::IdentifierLinker::Error("Identifier Type must be a variable or a function", castedType);

            castedType->origin = it->second;
            break;
        }
        case Parser::AST::Tag::EXPR_BIN:
        {
            const Parser::AST::Expr::Bin* castedType = static_cast<const Parser::AST::Expr::Bin*>(expr);

            Semantic::IdentifierLinker::linkerExpr(castedType->left.get(), table);
            Semantic::IdentifierLinker::linkerExpr(castedType->right.get(), table);
            break;
        }
        case Parser::AST::Tag::EXPR_UNARY:
        {
            const Parser::AST::Expr::Unary* castedType = static_cast<const Parser::AST::Expr::Unary*>(expr);

            Semantic::IdentifierLinker::linkerExpr(castedType->right.get(), table);
            break;
        }
        case Parser::AST::Tag::EXPR_CALL:
        {
            const Parser::AST::Expr::Call* castedType = static_cast<const Parser::AST::Expr::Call*>(expr);

            Semantic::IdentifierLinker::linkerExpr(castedType->func.get(), table);
            for (auto& param : castedType->params)
            {
                Semantic::IdentifierLinker::linkerExpr(param.get(), table);
            }
            break;
        }
        case Parser::AST::Tag::EXPR_ARR_ACCESS:
        {
            const Parser::AST::Expr::ArrAccess* castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(expr);

            Semantic::IdentifierLinker::linkerExpr(castedType->location.get(), table);
            Semantic::IdentifierLinker::linkerExpr(castedType->index.get(), table);
            break;
        }
        case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
        {
            const Parser::AST::Expr::StructAccess* castedType = static_cast<const Parser::AST::Expr::StructAccess*>(expr);
            Semantic::IdentifierLinker::linkerExpr(castedType->location.get(), table);
            break;
        }
        case Parser::AST::Tag::EXPR_SIZEOF:
        {
            const Parser::AST::Expr::Sizeof* castedType = static_cast<const Parser::AST::Expr::Sizeof*>(expr);

            Semantic::IdentifierLinker::linkerType(castedType->of.get(), table, castedType);
            break;
        }
        case Parser::AST::Tag::EXPR_CAST:
        {
            const Parser::AST::Expr::Cast* castedType = static_cast<const Parser::AST::Expr::Cast*>(expr);

            Semantic::IdentifierLinker::linkerExpr(castedType->expr.get(), table);
            Semantic::IdentifierLinker::linkerType(castedType->as.get(), table, castedType);
            break;
        }
        case Parser::AST::Tag::EXPR_ARR_INIT:
        {
            const Parser::AST::Expr::ArrInit* castedType = static_cast<const Parser::AST::Expr::ArrInit*>(expr);

            for (auto& element : castedType->elements)
            {
                Semantic::IdentifierLinker::linkerExpr(element.get(), table);
            }
            break;
        }
        case Parser::AST::Tag::EXPR_STRUCT_INIT:
        {
            const Parser::AST::Expr::StructInit* castedType = static_cast<const Parser::AST::Expr::StructInit*>(expr);

            Semantic::IdentifierLinker::linkerType(castedType->structName.get(), table, castedType);
            for (auto& field : castedType->fields)
            {
                Semantic::IdentifierLinker::linkerExpr(field.get(), table);
            }
            break;
        }
    }
}

void Semantic::IdentifierLinker::linkerType(const Parser::AST::Type::Base* type, const std::map<std::string_view, const Parser::AST::Decl::Base*>& table,
                                            const Parser::AST::Base* origin) // For throwing.
{
    if (type == nullptr) return;

    switch (type->tag)
    {
        case Parser::AST::Type::Tag::PRIMITIVE:
            break;

        case Parser::AST::Type::Tag::IDENTIFIER:
        {
            const Parser::AST::Type::Identifier* castedType = static_cast<const Parser::AST::Type::Identifier*>(type);

            auto it = table.find(castedType->name);
            if (it == table.end()) throw Semantic::IdentifierLinker::Error("Unknown type", origin);
            if (it->second->tag == Parser::AST::Tag::DECL_VAR or it->second->tag == Parser::AST::Tag::DECL_FUNC) 
                throw Semantic::IdentifierLinker::Error("Identifier Type cannot be a variable or a function", origin);

            castedType->origin = it->second;
            break;
        }
        case Parser::AST::Type::Tag::TYPEDEF:
        {
            const Parser::AST::Type::Typedef* castedType = static_cast<const Parser::AST::Type::Typedef*>(type);

            Semantic::IdentifierLinker::linkerType(castedType->from.get(), table, origin);
            break;
        }
        case Parser::AST::Type::Tag::PTR:
        {
            const Parser::AST::Type::Ptr* castedType = static_cast<const Parser::AST::Type::Ptr*>(type);

            Semantic::IdentifierLinker::linkerType(castedType->to.get(), table, origin);
            break;
        }
        case Parser::AST::Type::Tag::ARR:
        {
            const Parser::AST::Type::Arr* castedType = static_cast<const Parser::AST::Type::Arr*>(type);

            Semantic::IdentifierLinker::linkerType(castedType->type.get(), table, origin);
            Semantic::IdentifierLinker::linkerExpr(castedType->size.get(), table);
            break;
        }
        case Parser::AST::Type::Tag::FUNC:
        {
            const Parser::AST::Type::Func* castedType = static_cast<const Parser::AST::Type::Func*>(type);

            Semantic::IdentifierLinker::linkerType(castedType->returnType.get(), table, origin);
            for (auto& param : castedType->params)
            {
                Semantic::IdentifierLinker::linkerType(param.get(), table, origin);
            }
            break;
        }
        case Parser::AST::Type::Tag::STRUCT:
        {
            const Parser::AST::Type::Struct* castedType = static_cast<const Parser::AST::Type::Struct*>(type);

            for (auto& field : castedType->fields)
            {
                Semantic::IdentifierLinker::linkerType(field.type.get(), table, origin);
            }
            break;
        }
    }
}
