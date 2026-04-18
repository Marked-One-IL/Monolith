#include <Parser/AST/Base.hpp>
#include <Parser/AST/Stmt/If.hpp>
#include <Parser/AST/Stmt/While.hpp>
#include <Parser/AST/Stmt/For.hpp>
#include <Parser/AST/Stmt/Break.hpp>
#include <Parser/AST/Stmt/Continue.hpp>
#include <Parser/AST/Stmt/Return.hpp>
#include <Parser/AST/Stmt/Assign.hpp>
#include <Parser/AST/Decl/Var.hpp>
#include <Parser/AST/Decl/Func.hpp>
#include <Parser/AST/Decl/Struct.hpp>
#include <Parser/AST/Decl/Typedef.hpp>
#include <Parser/AST/Expr/ArrAccess.hpp>
#include <Parser/AST/Expr/ArrInit.hpp>
#include <Parser/AST/Expr/StructInit.hpp>
#include <Parser/AST/Expr/Bin.hpp>
#include <Parser/AST/Expr/Call.hpp>
#include <Parser/AST/Expr/Cast.hpp>
#include <Parser/AST/Expr/StructAccess.hpp>
#include <Parser/AST/Expr/Identifier.hpp>
#include <Parser/AST/Expr/Literal.hpp>
#include <Parser/AST/Expr/Sizeof.hpp>
#include <Parser/AST/Expr/Unary.hpp>
#include <Helper/Assert.hpp>
#include <Helper/Helper.hpp>
#include <format>

Parser::AST::Base::Base(Parser::AST::Tag new_tag, const Lexer::LocationData& new_locationData) :
    tag(new_tag), locationData(Parser::AST::LocationData(new_locationData))
{
}

Parser::AST::LocationData::LocationData(const Lexer::LocationData& new_locationData) :
    line(new_locationData.line), lineNum(new_locationData.lineNum), filename(new_locationData.filename)
{
}

void Parser::AST::Base::print(std::ostream& stream, Uint depth) const
{
    if (this == nullptr) 
    {
        stream << "nullptr";
        return;
    }

    switch (this->tag)
    {
    case Parser::AST::Tag::STMT_IF: 
    {
        const Parser::AST::Stmt::If& castedType = *(static_cast<const Parser::AST::Stmt::If*>(this));
        Helper::printDepth(stream, depth) << "STMT_IF:\n";
        Helper::printDepth(stream, depth) << "cond:\n"; castedType.cond->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "then: \n"; castedType.then->print(stream, depth + 1);
        if (castedType.els != nullptr)
        {
            Helper::printDepth(stream, depth) << "els: \n";
            castedType.els->print(stream, depth + 1);
        }
        break;
    }
    case Parser::AST::Tag::STMT_WHILE: 
    {
        const Parser::AST::Stmt::While& castedType = *(static_cast<const Parser::AST::Stmt::While*>(this));

        Helper::printDepth(stream, depth) << "STMT_WHILE:\n";
        Helper::printDepth(stream, depth) << "cond:\n"; castedType.cond->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "then:";  castedType.then->print(stream, depth + 1);
        break;
    }
    case Parser::AST::Tag::STMT_FOR:
    {
        const Parser::AST::Stmt::For& castedType = *(static_cast<const Parser::AST::Stmt::For*>(this));

        Helper::printDepth(stream, depth) << "STMT_FOR:\n";
        Helper::printDepth(stream, depth) << "init:\n"; castedType.init->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "cond:\n"; castedType.cond->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "iter:\n"; castedType.iter->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "then:"; 
        castedType.then->print(stream, depth + 1);
        break;
    }
    case Parser::AST::Tag::STMT_BREAK:
    {
        Helper::printDepth(stream, depth) << "STMT_BREAK\n";
        break;
    }
    case Parser::AST::Tag::STMT_CONTINUE:
    {
        Helper::printDepth(stream, depth) << "STMT_CONTINUE\n";
        break;
    }
    case Parser::AST::Tag::STMT_RETURN:
    {
        const Parser::AST::Stmt::Return& castedType = *(static_cast<const Parser::AST::Stmt::Return*>(this));

        Helper::printDepth(stream, depth) << "STMT_RETURN\n";
        if (castedType.expr) castedType.expr->print(stream, depth + 1);
        break;
    }
    case Parser::AST::Tag::STMT_ASSIGN:
    {
        const Parser::AST::Stmt::Assign& castedType = *(static_cast<const Parser::AST::Stmt::Assign*>(this));

        Helper::printDepth(stream, depth) << "STMT_ASSIGN:\n";
        Helper::printDepth(stream, depth) << "dst:\n"; castedType.dst->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "src:\n"; castedType.src->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "operation: " << castedType.operation << '\n';
        break;
    }

    case Parser::AST::Tag::DECL_VAR:
    {
        const Parser::AST::Decl::Var& castedType = *(static_cast<const Parser::AST::Decl::Var*>(this));

        Helper::printDepth(stream, depth) << "DECL_VAR:\n";
        Helper::printDepth(stream, depth) << "name: " << castedType.name << '\n';
        Helper::printDepth(stream, depth) << "type:\n"; castedType.type->print(stream, depth); stream << '\n';
        Helper::printDepth(stream, depth) << "init:\n"; castedType.init->print(stream, depth + 1);
        break;
    }
    case Parser::AST::Tag::DECL_FUNC:
    {
        const Parser::AST::Decl::Func& castedType = *(static_cast<const Parser::AST::Decl::Func*>(this));

        Helper::printDepth(stream, depth) << "DECL_FUNC:\n";
        Helper::printDepth(stream, depth) << "name: " << castedType.name << '\n';
        Helper::printDepth(stream, depth) << "type: "; castedType.data->print(stream, depth);

        for (Uint i = 0; i < castedType.paramNames.size(); i++)
        {
            Helper::printDepth(stream, depth) << castedType.paramNames[i] << ":";
            castedType.data->params[i]->print(stream, depth);
        }

        Helper::printDepth(stream, depth) << "block:\n"; castedType.block->print(stream, depth);
        break;
    }
    case Parser::AST::Tag::DECL_STRUCT:
    {
        const Parser::AST::Decl::Struct &castedType = *(static_cast<const Parser::AST::Decl::Struct*>(this));

        Helper::printDepth(stream, depth) << "DECL_STRUCT:\n";
        Helper::printDepth(stream, depth) << "name: " << castedType.name << '\n';
        Helper::printDepth(stream, depth) << "type:\n"; castedType.data->print(stream, depth);
        break;
    }
    case Parser::AST::Tag::DECL_TYPEDEF:
    {
        const Parser::AST::Decl::Typedef& castedType = *(static_cast<const Parser::AST::Decl::Typedef*>(this));

        Helper::printDepth(stream, depth) << "DECL_TYPEDEF:\n";
        Helper::printDepth(stream, depth) << "name: " << castedType.name << '\n';
        Helper::printDepth(stream, depth) << "type:\n"; castedType.typeDef->print(stream, depth);
        break;
    }

    case Parser::AST::Tag::EXPR_LITERAL:
    {
        const Parser::AST::Expr::Literal& castedType = *(static_cast<const Parser::AST::Expr::Literal*>(this));

        Helper::printDepth(stream, depth) << "EXPR_LITERAL:\n";
        Helper::printDepth(stream, depth) << "literal: " << castedType.literal;
        break;
    }
    case Parser::AST::Tag::EXPR_IDENTIFIER:
    {
        const Parser::AST::Expr::Identifier& castedType = *(static_cast<const Parser::AST::Expr::Identifier*>(this));
        Assert_Message(castedType.origin, "origin is a nullptr");

        Helper::printDepth(stream, depth) << "EXPR_IDENTIFIER:\n";
        Helper::printDepth(stream, depth) << "name: " << castedType.name << '\n';
        break;
    }
    case Parser::AST::Tag::EXPR_BIN:
    {
        const Parser::AST::Expr::Bin& castedType = *(static_cast<const Parser::AST::Expr::Bin*>(this));

        Helper::printDepth(stream, depth) << "EXPR_BIN:\n";
        Helper::printDepth(stream, depth) << "left:\n"; castedType.left->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "right:\n"; castedType.right->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "operation: " << castedType.operation;
        break;
    }
    case Parser::AST::Tag::EXPR_UNARY:
    {
        const Parser::AST::Expr::Unary& castedType = *(static_cast<const Parser::AST::Expr::Unary*>(this));

        Helper::printDepth(stream, depth) << "EXPR_UNARY:\n";
        Helper::printDepth(stream, depth) << "right:\n"; castedType.right->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "operation: " << castedType.operation;
        break;
    }
    case Parser::AST::Tag::EXPR_CALL:
    {
        const Parser::AST::Expr::Call& castedType = *(static_cast<const Parser::AST::Expr::Call*>(this));

        Helper::printDepth(stream, depth) << "EXPR_CALL:\n";
        Helper::printDepth(stream, depth) << "func:\n"; castedType.func->print(stream, depth + 1);

        Helper::printDepth(stream, depth) << "params:\n";
        for (auto& param : castedType.params)
        {
            Helper::printDepth(stream, depth); param->print(stream, depth + 1);
        }
        break;
    }
    case Parser::AST::Tag::EXPR_ARR_ACCESS:
    {
        const Parser::AST::Expr::ArrAccess& castedType = *(static_cast<const Parser::AST::Expr::ArrAccess*>(this));

        Helper::printDepth(stream, depth) << "EXPR_ARR_ACCESS:\n";
        Helper::printDepth(stream, depth) << "location:\n"; castedType.location->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "index:\n"; castedType.index->print(stream, depth + 1);
        break;
    }
    case Parser::AST::Tag::EXPR_ARR_INIT:
    {
        const Parser::AST::Expr::ArrInit& castedType = *(static_cast<const Parser::AST::Expr::ArrInit*>(this));

        Helper::printDepth(stream, depth) << "EXPR_ARR_INIT:\n";
        for (auto& element : castedType.elements)
        {
            Helper::printDepth(stream, depth); element->print(stream, depth + 1);
        }
        break;
    }
    case Parser::AST::Tag::EXPR_STRUCT_INIT:
    {
        const Parser::AST::Expr::StructInit& castedType = *(static_cast<const Parser::AST::Expr::StructInit*>(this));

        Helper::printDepth(stream, depth) << "EXPR_STRUCT_INIT:\n";
        for (auto& field : castedType.fields)
        {
            Helper::printDepth(stream, depth); field->print(stream, depth + 1);
        }
        break;
    }
    case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
    {
        const Parser::AST::Expr::StructAccess& castedType = *(static_cast<const Parser::AST::Expr::StructAccess*>(this));

        Helper::printDepth(stream, depth) << "EXPR_STRUCT_ACCESS:\n";
        Helper::printDepth(stream, depth) << "location:\n"; castedType.location->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "field: " << castedType.field;
        break;
    }
    case Parser::AST::Tag::EXPR_CAST:
    {
        const Parser::AST::Expr::Cast& castedType = *(static_cast<const Parser::AST::Expr::Cast*>(this));

        Helper::printDepth(stream, depth) << "EXPR_CAST:\n";
        Helper::printDepth(stream, depth) << "expr:\n"; castedType.expr->print(stream, depth + 1);
        Helper::printDepth(stream, depth) << "as:\n"; castedType.as->print(stream, depth + 1);
        break;
    }
    case Parser::AST::Tag::EXPR_SIZEOF:
    {
        const Parser::AST::Expr::Sizeof& castedType = *(static_cast<const Parser::AST::Expr::Sizeof*>(this));

        Helper::printDepth(stream, depth) << "EXPR_SIZEOF:\n";
        Helper::printDepth(stream, depth) << "of:\n"; castedType.of->print(stream, depth + 1);
        break;
    }
    case Parser::AST::Tag::DECL_PARAM:
    case Parser::AST::Tag::DECL_FIELD:
        break;

    default: Assert_Message(ASSERT_ALWAYS, "Node have an unknown tag");
    }

    stream << '\n';
}
