#include <Semantic/TypeChecking.hpp>
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
#include <Parser/AST/Stmt/Assign.hpp>
#include <Parser/AST/Stmt/If.hpp>
#include <Parser/AST/Stmt/While.hpp>
#include <Parser/AST/Stmt/For.hpp>
#include <Parser/AST/Stmt/Break.hpp>
#include <Parser/AST/Stmt/Continue.hpp>
#include <Parser/AST/Stmt/Return.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <Parser/AST/Type/Primitive.hpp>
#include <Parser/AST/Type/Typedef.hpp>
#include <Parser/AST/Type/Ptr.hpp>
#include <Parser/AST/Type/Arr.hpp>
#include <Parser/AST/Type/Struct.hpp>
#include <Parser/AST/Type/Func.hpp>
#include <Helper/Assert.hpp>

void Semantic::TypeChecking::check(const Parser::Generator& parser)
{
    try
    {
        Semantic::TypeChecking::checkBlock(parser.getHeadBlock(), nullptr);
    }
    catch (const Semantic::TypeChecking::Error& error)
    {
        Assert(error.ast);

        throw std::runtime_error(std::format("Semantic Type Checker:\nAt file: {}\nLine: {}\nError: {}\n{}",
            error.ast->locationData.filename, error.ast->locationData.lineNum, error.error,
            error.ast->locationData.line));
    }
}

void Semantic::TypeChecking::checkBlock(const Parser::AST::Block* block, const Parser::AST::Decl::Func* currentFunc, bool isInLoopStmt)
{
    if (block == nullptr) return;

    bool wasLastCommandReturn = false;
    for (auto& ast : block->commands)
    {
        if (ast.get() == nullptr) continue;
        wasLastCommandReturn = false;

        switch (ast->tag)
        {
        case Parser::AST::Tag::STMT_IF:
        {
            static auto boolType = std::make_unique<const Parser::AST::Type::Primitive>("bool");
            auto castedType = static_cast<const Parser::AST::Stmt::If*>(ast.get());
            if (not castedType) return;

            auto condType = castedType->cond.get()->getType();
            if (not Parser::AST::Type::Base::compare(condType, boolType.get()))
                throw Semantic::TypeChecking::Error("If statement condition is not a bool type", castedType);

            while (castedType)
            {
                Semantic::TypeChecking::checkBlock(castedType->then.get(), currentFunc, true);
                castedType = castedType->els.get();
            }

            break;
        }
        case Parser::AST::Tag::STMT_WHILE:
        {
            static auto boolType = std::make_unique<const Parser::AST::Type::Primitive>("bool");
            auto castedType = static_cast<const Parser::AST::Stmt::While*>(ast.get());

            if (not Parser::AST::Type::Base::compare(castedType->cond.get()->getType(), boolType.get()))
                throw Semantic::TypeChecking::Error("While statement condition is not a bool type", castedType);

            Semantic::TypeChecking::checkBlock(castedType->then.get(), currentFunc, true);
            break;
        }
        case Parser::AST::Tag::STMT_FOR:
        {
            auto castedType = static_cast<const Parser::AST::Stmt::For*>(ast.get());

            Semantic::TypeChecking::checkSingleAssign(static_cast<const Parser::AST::Stmt::Assign*>
                (castedType->init.get()));
            castedType->cond->getType();


            Semantic::TypeChecking::checkSingleAssign(static_cast<const Parser::AST::Stmt::Assign*>
                (castedType->iter.get()));

            Semantic::TypeChecking::checkBlock(castedType->then.get(), currentFunc, true);
            break;
        }
        case Parser::AST::Tag::STMT_BREAK:
        case Parser::AST::Tag::STMT_CONTINUE:
            break;
        case Parser::AST::Tag::STMT_RETURN:
        {
            auto castedType = static_cast<const Parser::AST::Stmt::Return*>(ast.get());

            auto returnValueType = castedType->expr->getType();
            auto returnType = (currentFunc) ? (currentFunc->data.get()->returnType.get()) : nullptr;
            if (not Parser::AST::Type::Base::compare(returnValueType, returnType))
                throw Semantic::TypeChecking::Error("Return value type does not match the function's return type",  castedType);
            wasLastCommandReturn = true;
            break;
        }
        case Parser::AST::Tag::STMT_ASSIGN:
        {
            Semantic::TypeChecking::checkSingleAssign(static_cast<const Parser::AST::Stmt::Assign*>(ast.get()));
            break;
        }
        case Parser::AST::Tag::DECL_VAR:
        {
            auto castedType = static_cast<const Parser::AST::Decl::Var*>(ast.get());

            auto leftType = castedType->type.get();
            auto rightType = castedType->init->getType();

            if (not Parser::AST::Type::Base::compare(leftType, rightType))
                throw Semantic::TypeChecking::Error("Variable type does not match his initialization type", castedType);

            break;
        }
        case Parser::AST::Tag::DECL_FUNC:
        {
            auto castedType = static_cast<const Parser::AST::Decl::Func*>(ast.get());
            Semantic::TypeChecking::checkBlock(castedType->block.get(), castedType);
            break;
        }
        case Parser::AST::Tag::DECL_STRUCT:
        {
            break;
        }
        case Parser::AST::Tag::DECL_TYPEDEF:
            break;

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
        {
            auto castedType = static_cast<const Parser::AST::Expr::Base*>(ast.get());
            castedType->getType(); // Verify single expressions.
            break;
        }
        }
    }

    if (not wasLastCommandReturn and
        not isInLoopStmt and
        currentFunc != nullptr and
        currentFunc->data->returnType.get() != nullptr)
    {
        throw Semantic::TypeChecking::Error("Last command of a function block with a return type must end with a return", currentFunc);
    }
}

bool Semantic::TypeChecking::isLeftAssignCorrect(const Parser::AST::Expr::Base* expr)
{
    switch (expr->tag)
    {
    case Parser::AST::Tag::EXPR_LITERAL:
    case Parser::AST::Tag::EXPR_BIN:
    case Parser::AST::Tag::EXPR_CALL:
    case Parser::AST::Tag::EXPR_SIZEOF:
    case Parser::AST::Tag::EXPR_CAST:
    case Parser::AST::Tag::EXPR_ARR_INIT:
    case Parser::AST::Tag::EXPR_STRUCT_INIT:
        return false;

    case Parser::AST::Tag::EXPR_IDENTIFIER:
    {
        const auto castedType = static_cast<const Parser::AST::Expr::Identifier*>(expr);
        return castedType->origin->tag == Parser::AST::Tag::DECL_VAR;
    }
    case Parser::AST::Tag::EXPR_UNARY:
    {
        const auto castedType = static_cast<const Parser::AST::Expr::Unary*>(expr);
        return castedType->operation == "dref";
    }
    case Parser::AST::Tag::EXPR_ARR_ACCESS:
    {
        const auto castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(expr);
        return Semantic::TypeChecking::isLeftAssignCorrect(castedType->location.get());
    }
    case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
    {
        const auto castedType = static_cast<const Parser::AST::Expr::StructAccess*>(expr);
        return Semantic::TypeChecking::isLeftAssignCorrect(castedType->location.get());
    }
    }

    return false;
}

void Semantic::TypeChecking::checkSingleAssign(const Parser::AST::Stmt::Assign* assign)
{
    if (not Parser::AST::Type::Base::compare(assign->dst->getType(), assign->src->getType()))
        throw Semantic::TypeChecking::Error("Left and right of the assignment type mismatch", assign);
    if (not Semantic::TypeChecking::isLeftAssignCorrect(assign->dst.get()))
        throw Semantic::TypeChecking::Error("Invalid assign expression", assign);

    // Ugly stinky hack coming out fresh out my ass. Enjoy.
    if (assign->operation == "=") return;
    std::string_view opr;
    if (assign->operation == "+=") opr = "+";
    else if (assign->operation == "-=") opr = "-";
    else if (assign->operation == "*=") opr = "*";
    else if (assign->operation == "/=") opr = "/";
    else if (assign->operation == "%=") opr = "%";
    else if (assign->operation == "|=") opr = "|";
    else if (assign->operation == "&=") opr = "&";
    else if (assign->operation == "^=") opr = "^";
    else if (assign->operation == "<<=") opr = "<<";
    else if (assign->operation == ">>=") opr = ">>";
    else Assert_Message(ASSERT_ALWAYS, "Invalid assign operation");
    // Rebuild of bin node.
    std::unique_ptr<const Parser::AST::Expr::Base> tempLeft = std::move(assign->dst);
    std::unique_ptr<const Parser::AST::Expr::Base> tempRight = std::move(assign->src);
    Lexer::LocationData ld(assign->locationData.filename, assign->locationData.line, assign->locationData.lineNum);
    Parser::AST::Expr::Bin bin(std::move(tempLeft), std::move(tempRight), opr, ld);
    bin.getType(); // Verify the operation without creating a new entire check system specifically for assign.
    // Taking back ownership. I could have done this hack before but it's too late.
    assign->dst = std::move(bin.left);
    assign->src = std::move(bin.right);
    // I'm starting to think that this garbage could be art in some way.
    // Most hacks come from not understanding what's going on.
    // My hacks come from actually understanding what I'm doing knowning that some stuff are work that can be done in seconds.
    // Like think about it instead of over-engineering a new function for assign like a faggot you can just reuse the old ugly system.
    // And yeah I will jerk myself off fuck you. This garbage should be new Mona Lisa. It's pure garbage and pure genius.
    // Maybe Neriah you should stop talking to yourself and lose some fucking weight you human garbage sack of walking pure fat.
    // And I will say hmmm ... shit.
    // This is ass. I hate my life. I hate this project. God why have you forsaken me?
    // Niggas say that they would rather have meaning and authenticity then be plugged to 'Experience Machine' and gay shit like that.
    // Nigga get me some fucking heroin and shut up. Meaning is meaningless the meaning of life is null and everything we do is for pure pleasure.
    // If you think otherwise you are an idiot.
    // Just saying this project made an alcoholic I ain't kidding. If I had unlimited 40% alcohol supply I would be taking this shit daily. Shut up you fucking idiot.
    // No fuck you. No fuck you.
}

Semantic::TypeChecking::Error::Error(const char* new_error, const Parser::AST::Base* new_ast) :
    error(new_error), ast(new_ast)
{
}
