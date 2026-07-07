#include <x86Gen/Generator.hpp>
#include <x86Gen/Context/Global.hpp>
#include <Parser/AST/Type/Identifier.hpp>
#include <Parser/AST/Type/Primitive.hpp>
#include <Parser/AST/Type/Typedef.hpp>
#include <Parser/AST/Type/Ptr.hpp>
#include <Parser/AST/Type/Arr.hpp>
#include <Parser/AST/Type/Struct.hpp>
#include <Parser/AST/Type/Func.hpp>
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
#include <Parser/AST/Stmt/If.hpp>
#include <Parser/AST/Stmt/While.hpp>
#include <Parser/AST/Stmt/For.hpp>
#include <Parser/AST/Stmt/Break.hpp>
#include <Parser/AST/Stmt/Continue.hpp>
#include <Parser/AST/Stmt/Return.hpp>
#include <Parser/AST/Stmt/Assign.hpp>
#include <Helper/Assert.hpp>
#include <Helper/Helper.hpp>
#include <format>
#include <array>
#include <algorithm>
#include <string_view>
#include <iostream>
#include <queue>
#include <format>

// If I will finish this project in time.
// It's a proof that God has not forsaken me, and there is hope.
// Please God don't forsake me.
// https://youtu.be/vtYiwVpf9Mo
// But this won't prevent from me making hacky and ugly shit.

// Also I don't follow the abi call conventions 100%.
// But only enough to call basic external functions.
// Why? Because it's hard and fuck you.

x86Gen::Generator::Generator(const Parser::Generator& parser)
{
    std::list<std::string> stringLiterals;

    constexpr const char* format =
R"(.686
.model flat, c
option casemap:none
public main
{}

.data
{}
{}

.code
{}
end)";

    std::string functions;
    for (auto& possibleFunc : parser.getHeadBlock()->commands)
    {
        if (possibleFunc->tag == Parser::AST::Tag::DECL_FUNC)
        {
            auto castedType = static_cast<const Parser::AST::Decl::Func*>(possibleFunc.get());
            if (castedType->block.get()) functions += x86Gen::Generator::extractFunc(castedType, 1);
            else x86Gen::Context::Global::appendExternalFunction(castedType->name);
        }
    }
    std::string externFuncsAsm;
    for (auto& externFunc : x86Gen::Context::Global::getExternalFunctions())
    {
        externFuncsAsm += std::format("extern {}:PROC\n", externFunc);
    }
    std::string strLiterals;
    Uint i = 0;
    for (auto& strLiteral : x86Gen::Context::Global::getStringLiterals())
    {
        const Uint literalSize = std::format("stringLiteral_{} ", i).size();
        std::string temp = std::format("stringLiteral_{} db ", i++);

        Uint j = 0;
        for (char c : strLiteral)
        {
            temp += std::format("{}", static_cast<Uint>(c));
            if (j != 0 and j % 30 == 0 and ((j + 1) < strLiteral.size())) // Prevent empty db ...
            {
                temp += "\ndb ";
            }
            else
            {
                temp += ',';
            }
            j++;
        }
        if (not temp.empty()) temp.pop_back();
        temp.push_back('\n');

        strLiterals += temp;
    }
    Uint j = 0;
    std::string floatLiterals;
    for (auto& floatLiteral : x86Gen::Context::Global::getFloats())
    {
        floatLiterals += std::format("floatLiteral_{} REAL4 {}\n", j++, floatLiteral);
    }

    this->m_asm = std::format(format, externFuncsAsm, strLiterals, floatLiterals, functions);
}

x86Gen::Generator::ExprReturn::ExprReturn(std::string new_asmCalc, std::string new_location) :
    asmCalc(std::move(new_asmCalc)), location(std::move(new_location))
{
}

const std::string& x86Gen::Generator::getAsm(void) const
{
	return this->m_asm;
}

void x86Gen::Generator::allocateExpr(const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext)
{
    if (not expr) return;

    switch (expr->tag)
    {
    case Parser::AST::Tag::EXPR_SIZEOF:
    {
        funcContext.allocateTemp(4);
        break;
    }
    case Parser::AST::Tag::EXPR_IDENTIFIER:
    case Parser::AST::Tag::EXPR_LITERAL:
    {
        funcContext.allocateTemp(x86Gen::Generator::getSize(expr->getType()));
        break;
    }
    case Parser::AST::Tag::EXPR_ARR_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(expr);

        x86Gen::Generator::allocateExpr(castedType->location.get(), funcContext);
        x86Gen::Generator::allocateExpr(castedType->index.get(), funcContext);
        funcContext.allocateTemp(x86Gen::Generator::getSize(expr->getType()));
        break;
    }
    case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::StructAccess*>(expr);

        x86Gen::Generator::allocateExpr(castedType->location.get(), funcContext);
        funcContext.allocateTemp(x86Gen::Generator::getSize(expr->getType()));
        break;
    }
    case Parser::AST::Tag::EXPR_BIN:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Bin*>(expr);

        x86Gen::Generator::allocateExpr(castedType->left.get(), funcContext);
        x86Gen::Generator::allocateExpr(castedType->right.get(), funcContext);
        funcContext.allocateTemp(x86Gen::Generator::getSize(expr->getType()));
        break;
    }
    case Parser::AST::Tag::EXPR_UNARY:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Unary*>(expr);

        if (castedType->operation == "ref")
        {
            funcContext.allocateTemp(4);
            x86Gen::Generator::allocateRefAndAssign(castedType->right.get(), funcContext);
            break;
        }

        x86Gen::Generator::allocateExpr(castedType->right.get(), funcContext);
        funcContext.allocateTemp(x86Gen::Generator::getSize(expr->getType()));
        break;
    }
    case Parser::AST::Tag::EXPR_CALL:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Call*>(expr);

        x86Gen::Generator::allocateExpr(castedType->func.get(), funcContext); // getFunc()(10)
        funcContext.allocateTemp(x86Gen::Generator::getSize(expr->getType())); // x = foo()
        for (auto it = castedType->params.rbegin(); it != castedType->params.rend(); it++) // foo(1, 2, 3, 4)
        {
            x86Gen::Generator::allocateExpr(it->get(), funcContext);
        }
        break;
    }
    case Parser::AST::Tag::EXPR_CAST:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Cast*>(expr);

        x86Gen::Generator::allocateExpr(castedType->expr.get(), funcContext);
        funcContext.allocateTemp(x86Gen::Generator::getSize(castedType->as.get()));
        break;
    }
    case Parser::AST::Tag::EXPR_ARR_INIT:
    {
        auto castedType = static_cast<const Parser::AST::Expr::ArrInit*>(expr);

        funcContext.allocateTemp(x86Gen::Generator::getSize(castedType->getType()));
        for (auto& element : castedType->elements)
        {
            x86Gen::Generator::allocateExpr(element.get(), funcContext);
        }
        break;
    }
    case Parser::AST::Tag::EXPR_STRUCT_INIT:
    {
        auto castedType = static_cast<const Parser::AST::Expr::StructInit*>(expr);

        funcContext.allocateTemp(x86Gen::Generator::getSize(castedType->getType()));
        for (auto& field : castedType->fields)
        {
            x86Gen::Generator::allocateExpr(field.get(), funcContext);
        }
        break;
    }
    }
}
void x86Gen::Generator::allocateRefAndAssign(const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext)
{
    switch (expr->tag)
    {
    case Parser::AST::Tag::EXPR_IDENTIFIER:
        break;
    case Parser::AST::Tag::EXPR_ARR_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(expr);
        x86Gen::Generator::allocateRefAndAssign(castedType->location.get(), funcContext);
        x86Gen::Generator::allocateExpr(castedType->index.get(), funcContext);
        break;
    }
    case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::StructAccess*>(expr);
        x86Gen::Generator::allocateRefAndAssign(castedType->location.get(), funcContext);
        break;
    }
    case Parser::AST::Tag::EXPR_UNARY:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Unary*>(expr);
        x86Gen::Generator::allocateExpr(castedType->right.get(), funcContext);
        break;
    }
    }
}
void x86Gen::Generator::allocateBlock(const Parser::AST::Block* block, x86Gen::Context::Func& funcContext)
{
    if (not block) return;

    // Allocate variables and expressions (temps).
    for (auto it = block->commands.begin(); it != block->commands.end(); it++)
    {
        if (it->get()->tag == Parser::AST::Tag::DECL_VAR)
        {
            auto castedType = static_cast<const Parser::AST::Decl::Var*>(it->get());
            x86Gen::Generator::allocateExpr(castedType->init.get(), funcContext);
            funcContext.allocateVar(castedType->name, x86Gen::Generator::getSize(castedType->type.get()));
        }
        else if ((it->get()->tag >= Parser::AST::Tag::EXPR_LITERAL) and (it->get()->tag <= Parser::AST::Tag::EXPR_STRUCT_INIT)) // Is any expression.
        {
            x86Gen::Generator::allocateExpr(static_cast<const Parser::AST::Expr::Base*>(it->get()), funcContext);
        }
        else if (it->get()->tag == Parser::AST::Tag::STMT_ASSIGN)
        {
            auto castedType = static_cast<const Parser::AST::Stmt::Assign*>(it->get());

            x86Gen::Generator::allocateExpr(castedType->src.get(), funcContext);
            funcContext.allocateTemp(4); x86Gen::Generator::allocateRefAndAssign(castedType->dst.get(), funcContext);
        }
        else if (it->get()->tag == Parser::AST::Tag::STMT_IF)
        {
            auto castedType = static_cast<const Parser::AST::Stmt::If*>(it->get());

            for (const Parser::AST::Stmt::If* currentIf = castedType; currentIf != nullptr; currentIf = currentIf->els.get())
            {
                x86Gen::Generator::allocateExpr(currentIf->cond.get(), funcContext);
                x86Gen::Generator::allocateBlock(currentIf->then.get(), funcContext);
            }
        }
        else if (it->get()->tag == Parser::AST::Tag::STMT_WHILE)
        {
            auto castedType = static_cast<const Parser::AST::Stmt::While*>(it->get());
            x86Gen::Generator::allocateExpr(castedType->cond.get(), funcContext);
            x86Gen::Generator::allocateBlock(castedType->then.get(), funcContext);
        }
        else if (it->get()->tag == Parser::AST::Tag::STMT_FOR)
        {
            auto castedType = static_cast<const Parser::AST::Stmt::For*>(it->get());

            // i = 0
            x86Gen::Generator::allocateExpr(castedType->init->src.get(), funcContext);
            funcContext.allocateTemp(4); x86Gen::Generator::allocateRefAndAssign(castedType->init->dst.get(), funcContext);

            // i < 10
            x86Gen::Generator::allocateExpr(castedType->cond.get(), funcContext);

            // i += 1
            x86Gen::Generator::allocateExpr(castedType->iter->src.get(), funcContext);
            funcContext.allocateTemp(4); x86Gen::Generator::allocateRefAndAssign(castedType->iter->dst.get(), funcContext);

            x86Gen::Generator::allocateBlock(castedType->then.get(), funcContext);
        }
        else if (it->get()->tag == Parser::AST::Tag::STMT_RETURN)
        {
            auto castedType = static_cast<const Parser::AST::Stmt::Return*>(it->get());
            x86Gen::Generator::allocateExpr(castedType->expr.get(), funcContext);
        }
    }
}

x86Gen::Context::Func x86Gen::Generator::getFuncContext(const Parser::AST::Decl::Func* func)
{
    if (func == nullptr) return x86Gen::Context::Func("");
    x86Gen::Context::Func funcContext(func->name);

    Uint returnSize = x86Gen::Generator::getSize(func->data->returnType.get());
    auto underType = func->data->returnType.get() != nullptr ? Parser::AST::Type::Base::getUnderlyingType(func->data->returnType.get()) : nullptr;

    bool isStuctOrArr = false;
    if (underType != nullptr)
    {
        isStuctOrArr = underType->tag == Parser::AST::Type::Tag::STRUCT or
                       underType->tag == Parser::AST::Type::Tag::ARR;
    }

    if (isStuctOrArr) funcContext.allocateParam("", 4);

    // Allocate parameters.
    for (auto it = func->paramsDecl.begin(); it != func->paramsDecl.end(); it++)
    {
        funcContext.allocateParam(it->name, x86Gen::Generator::getSize(it->type));
    }
    x86Gen::Generator::allocateBlock(func->block.get(), funcContext);

    return funcContext;
}
x86Gen::Context::Loop x86Gen::Generator::getLoopContext(const Parser::AST::Stmt::Base* loop)
{
    if (not loop) return x86Gen::Context::Loop();

    static Uint whileCount = 0;
    static Uint forCount = 0;

    switch (loop->tag)
    {
    case Parser::AST::Tag::STMT_WHILE:
    {
        std::string sectionName = std::format("while_{}", whileCount++);
        return x86Gen::Context::Loop(sectionName + "_start", sectionName + "_exit");
    }
    case Parser::AST::Tag::STMT_FOR:
    {
        std::string sectionName = std::format("for_{}", forCount++);
        x86Gen::Context::Loop loop(sectionName + "_start", sectionName + "_exit");
        loop.labelIter = sectionName + "_iter";
        return loop;
    }
    }
    
    return x86Gen::Context::Loop();
}
// Before calling set do mov dword ptr {}, 0
// Because I will be adding to the location.
void x86Gen::Generator::appendCalculationsForRefAndAssign(const std::string& dst, std::string& asmCalc, const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext)
{
    switch (expr->tag)
    {
    case Parser::AST::Tag::EXPR_IDENTIFIER:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Identifier*>(expr);
        auto internalType = castedType->origin->getType();

        constexpr const char* format =
R"(
lea eax, {}
add dword ptr {}, eax
)";

        asmCalc += std::format(format, funcContext.getVarOrParamLocation(castedType->name), dst);
        break;
    }
    case Parser::AST::Tag::EXPR_ARR_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(expr);
        x86Gen::Generator::appendCalculationsForRefAndAssign(dst, asmCalc, castedType->location.get(), funcContext);

        if (castedType->location->getType()->tag == Parser::AST::Type::Tag::PTR)
        {
            asmCalc += std::format(
R"(
mov eax, dword ptr {}
mov eax, dword ptr [eax]
mov dword ptr {}, eax
)", dst, dst);
        }

        constexpr const char* format =
R"(
mov eax, dword ptr {}
imul eax, {}
add dword ptr {}, eax
)";

        Uint elementSize = x86Gen::Generator::getSize(castedType->getType());
        auto index = x86Gen::Generator::extractExpr(castedType->index.get(), funcContext);
        asmCalc += index.asmCalc;
        asmCalc += std::format(format, index.location, elementSize, dst);
        break;
    }
    case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::StructAccess*>(expr);
        auto structType = static_cast<const Parser::AST::Type::Struct*>(castedType->location->getType());
        constexpr const char* format =
R"(
mov eax, {}
add dword ptr {}, eax
)";

        Uint offset = 0;
        for (auto& field : structType->fields)
        {
            if (field.name == castedType->field)
            {
                break;
            }
            offset += x86Gen::Generator::getSize(field.type.get());
        }

        x86Gen::Generator::appendCalculationsForRefAndAssign(dst, asmCalc, castedType->location.get(), funcContext);
        asmCalc += std::format(format, offset, dst);
        break;
    }
    case Parser::AST::Tag::EXPR_UNARY: // Dref only.
    {
        auto castedType = static_cast<const Parser::AST::Expr::Unary*>(expr);

        constexpr const char* format =
R"(
mov eax, {}
add dword ptr {}, eax
)";

        auto srcFull = x86Gen::Generator::extractExpr(castedType->right.get(), funcContext);
        asmCalc += srcFull.asmCalc;
        asmCalc += std::format(format, srcFull.location, dst);
        break;
    }
    }
}

std::string x86Gen::Generator::extractFunc(const Parser::AST::Decl::Func* func, Uint depth)
{
    if ((not func) or (not func->block.get())) return "";

    constexpr const char* format =
R"(
_{} PROC
    push ebp
    mov ebp, esp
    sub esp, {} 

    {}
    
    {}_return:
    mov esp, ebp
    pop ebp
    ret
_{} ENDP
)";

    constexpr const char* formatMain =
R"(
{} PROC
    push ebp
    mov ebp, esp
    sub esp, {}

{}
    {}_return:
    mov esp, ebp
    pop ebp
    ret
{} ENDP
)";

    x86Gen::Context::Func funcContext = x86Gen::Generator::getFuncContext(func);
    x86Gen::Context::Loop currentLoopContext;
    std::string body = x86Gen::Generator::extractBlock(func->block.get(), funcContext, currentLoopContext, depth);
    Helper::applyDepth(body, depth);

    if (func->name == "main")
    {
        return std::format(formatMain, func->name, funcContext.getTotalStackSize(), body, func->name, func->name);
    }
    return std::format(format, func->name, funcContext.getTotalStackSize(), body, func->name, func->name);
}
std::string x86Gen::Generator::extractBlock(const Parser::AST::Block* block, x86Gen::Context::Func& funcContext, const x86Gen::Context::Loop& prevLoopContext, Uint depth)
{
    if (not block) return "";

    std::string asmCode;
    x86Gen::Context::Loop currentLoopContext;

    for (auto& command : block->commands)
    {
        switch (command->tag)
        {
        case Parser::AST::Tag::DECL_VAR:
        {
            auto castedType = static_cast<const Parser::AST::Decl::Var*>(command.get());

            std::string dst = funcContext.getVarOrParamLocation(castedType->name);
            auto srcFull = x86Gen::Generator::extractExpr(castedType->init.get(), funcContext);
            asmCode += srcFull.asmCalc;
            asmCode += x86Gen::Generator::generateMov(dst, srcFull.location, x86Gen::Generator::getSize(castedType->init->getType()), false);
            break;
        }
        case Parser::AST::Tag::STMT_IF:
        {
            auto castedType = static_cast<const Parser::AST::Stmt::If*>(command.get());
            static Uint chainCount = NPOS;
            chainCount++;
            Uint savedChainCount = chainCount;

            constexpr const char* ifOrElifFormat = 
R"(
if_{}_{}:
{}
mov al, byte ptr {}
cmp al, 0
je {}
{}
jmp {}
)";

            constexpr const char* elseFormat =
R"(
if_{}_{}:
{}
)";

            const Parser::AST::Stmt::If* currentIf = castedType;

            
            for (Uint ifCount = 0; currentIf != nullptr; ifCount++)
            {
                bool isIfOrElif = currentIf->cond.get() != nullptr;
                bool isElse = not isIfOrElif;

                if (isIfOrElif)
                {
                    bool doesHaveNext = currentIf->els.get() != nullptr;
                    auto cond = x86Gen::Generator::extractExpr(currentIf->cond.get(), funcContext);
                    std::string block = x86Gen::Generator::extractBlock(currentIf->then.get(), funcContext, prevLoopContext, depth);

                    std::string exitLabel = (doesHaveNext) ? std::format("if_{}_{}", savedChainCount, ifCount + 1) : std::format("if_{}_exit", savedChainCount);
                    asmCode += std::format(ifOrElifFormat,
                                           savedChainCount, ifCount,
                                           cond.asmCalc,
                                           cond.location,
                                           exitLabel,
                                           block,
                                           std::format("if_{}_exit", savedChainCount));
                }
                else
                {
                    std::string block = x86Gen::Generator::extractBlock(currentIf->then.get(), funcContext, currentLoopContext, depth);

                    asmCode += std::format(elseFormat,
                                           savedChainCount, ifCount,
                                           block);
                }

                currentIf = currentIf->els.get();
            }
            asmCode += std::format("if_{}_exit:", savedChainCount);
            break;
        }
        case Parser::AST::Tag::STMT_WHILE:
        {
            auto castedType = static_cast<const Parser::AST::Stmt::While*>(command.get());

            currentLoopContext = x86Gen::Generator::getLoopContext(castedType);
            constexpr const char* whileFormat =
R"(
{}:
{}
mov al, byte ptr {}
cmp al, 0
je {}
{}
jmp {}
{}:
)";
            auto condSrc = x86Gen::Generator::extractExpr(castedType->cond.get(), funcContext);
            std::string block = x86Gen::Generator::extractBlock(castedType->then.get(), funcContext, currentLoopContext, depth);
            asmCode += std::format(whileFormat, currentLoopContext.labelStart, condSrc.asmCalc, condSrc.location, currentLoopContext.labelExit, block, currentLoopContext.labelStart, currentLoopContext.labelExit);
            break;
        }
        case Parser::AST::Tag::STMT_FOR:
        {
            auto castedType = static_cast<const Parser::AST::Stmt::For*>(command.get());

            currentLoopContext = x86Gen::Generator::getLoopContext(castedType);
            constexpr const char* forFormat = 
R"(
{}
{}:
{}
mov al, byte ptr {}
cmp al, 0
je {}
{}
{}:
{}
jmp {}
{}:
)";

            std::string init = x86Gen::Generator::generateAssign(castedType->init.get(), funcContext);
            auto cond = x86Gen::Generator::extractExpr(castedType->cond.get(), funcContext);
            std::string iter = x86Gen::Generator::generateAssign(castedType->iter.get(), funcContext);
            std::string block = x86Gen::Generator::extractBlock(castedType->then.get(), funcContext, currentLoopContext, depth);
            
            asmCode += std::format(forFormat, init, currentLoopContext.labelStart, cond.asmCalc, cond.location, currentLoopContext.labelExit, block,
                currentLoopContext.labelIter, iter, currentLoopContext.labelStart, currentLoopContext.labelExit);
            break;
        }
        case Parser::AST::Tag::STMT_BREAK:
        {
            asmCode += std::format("jmp {}", prevLoopContext.labelExit);
            break;
        }
        case Parser::AST::Tag::STMT_CONTINUE:
        {
            if (prevLoopContext.labelIter.empty()) asmCode += std::format("jmp {}", prevLoopContext.labelStart);
            else asmCode += std::format("jmp {}", prevLoopContext.labelIter);
            break;
        }
        case Parser::AST::Tag::STMT_RETURN:
        {
            auto castedType = static_cast<const Parser::AST::Stmt::Return*>(command.get());
            std::string exit = std::format("jmp {}_return", funcContext.getName());

            if (castedType->expr.get())
            {
                auto castedExprType = castedType->expr->getType();
                auto expr = x86Gen::Generator::extractExpr(castedType->expr.get(), funcContext);
                Uint typeSize = x86Gen::Generator::getSize(castedExprType);
                bool isInt = castedExprType->tag == Parser::AST::Type::Tag::PRIMITIVE and static_cast<const Parser::AST::Type::Primitive*>(castedExprType)->name == "int";
                bool isFloat = castedExprType->tag == Parser::AST::Type::Tag::PRIMITIVE and static_cast<const Parser::AST::Type::Primitive*>(castedExprType)->name == "float";
                bool isStructOrArr = castedExprType->tag == Parser::AST::Type::Tag::STRUCT or castedExprType->tag == Parser::AST::Type::Tag::ARR;

                asmCode += expr.asmCalc;

                if (typeSize == 1 and not isStructOrArr)
                {
                    constexpr const char* format1Byte =
R"(
xor eax, eax
mov al, byte ptr {}
)";

                    asmCode += std::format(format1Byte, expr.location);
                }
                else if (typeSize == 4 and not isFloat and not isStructOrArr)
                {
                    constexpr const char* format4Byte =
R"(
mov eax, dword ptr {}
)";

                    asmCode += std::format(format4Byte, expr.location);
                }
                else if (typeSize == 4 and isFloat and not isStructOrArr)
                {
                    constexpr const char* formatFloat =
R"(
fld dword ptr {}
)";

                    asmCode += std::format(formatFloat, expr.location);
                }
                else
                {
                    constexpr const char* formatAll =
R"(
lea esi, {}
mov edi, dword ptr [ebp+8]
mov ecx, {}
cld
rep movsb
)";
                    asmCode += std::format(formatAll, expr.location, typeSize);
                }
            }
            asmCode += exit;
            break;
        }
        case Parser::AST::Tag::STMT_ASSIGN:
        {
            auto castedType = static_cast<const Parser::AST::Stmt::Assign*>(command.get());
            asmCode += x86Gen::Generator::generateAssign(castedType, funcContext);
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
            asmCode += x86Gen::Generator::extractExpr(static_cast<const Parser::AST::Expr::Base*>(command.get()), funcContext).asmCalc;
            break;
        }
    }

    return asmCode;
}
x86Gen::Generator::ExprReturn x86Gen::Generator::extractExpr(const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext)
{
    switch (expr->tag)
    {
    case Parser::AST::Tag::EXPR_LITERAL:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Literal*>(expr);

        Uint size = 0;
        std::string src;

        if (castedType->literal.isBool())
        {
            size = 1;
            if (castedType->literal.content == "True") src = "1";
            else /*if (castedType->literal.content == "False")*/ src = "0";
        }
        else if (castedType->literal.isChar())
        {
            size = 1;

            std::string_view tempView = castedType->literal.content;
            tempView.remove_prefix(1);
            tempView.remove_suffix(1);
            char c = tempView.front();
            if (tempView.size() >= 2 and c == '\\')
            {
                char nextC = tempView[1];
                switch (nextC)
                {
                case 'a':  src = std::to_string(static_cast<int>('\a')); break;
                case 'b':  src = std::to_string(static_cast<int>('\b')); break;
                case 'f':  src = std::to_string(static_cast<int>('\f')); break;
                case 'n':  src = std::to_string(static_cast<int>('\n')); break;
                case 'r':  src = std::to_string(static_cast<int>('\r')); break;
                case 't':  src = std::to_string(static_cast<int>('\t')); break;
                case 'v':  src = std::to_string(static_cast<int>('\v')); break;
                case '\'': src = std::to_string(static_cast<int>('\'')); break;
                case '"':  src = std::to_string(static_cast<int>('\"')); break;
                case '?':  src = std::to_string(static_cast<int>('\?')); break;
                case '\\': src = std::to_string(static_cast<int>('\\')); break;
                case '0':  src = std::to_string(static_cast<int>('\0')); break;
                default:
                    src = std::to_string(static_cast<int>(c));
                }
            }
            else src = std::to_string(static_cast<int>(c));
        }
        else if (castedType->literal.isInt())
        {
            size = 4;
            Uint value = 0;
            Uint offset = (castedType->literal.tag == Lexer::Tag::INT_LITERAL) ? (0) : (2);
            std::from_chars(castedType->literal.content.data() + offset, castedType->literal.content.data() + castedType->literal.content.size(), value);
            src = std::to_string(value);
        }
        else if (castedType->literal.isFloat())
        {
            size = 4;
            src = x86Gen::Context::Global::appendAndGetFloatLiteral(castedType->literal); // I can't write mov eax, 1.3. I must keep as constant in the global section. MASM you suck dick.
        }
        else if (castedType->literal.isNone())
        {
            size = 4;
            src = "0";
        }
        else if (castedType->literal.isStr())
        {
            size = 4;
            src = x86Gen::Context::Global::appendAndGetStrLiteral(castedType->literal);
        }
        else Assert_Message(ASSERT_ALWAYS, "Invalid literal");

        std::string location = funcContext.getTempLocation();
        constexpr const char* format1Byte =
R"(
mov al, {}
mov byte ptr {}, al
)";

        constexpr const char* format4Byte =
R"(
mov eax, {}
mov dword ptr {}, eax
)";

        if (size == 1) return x86Gen::Generator::ExprReturn(std::format(format1Byte, src, location), location);
        /*else if (size == 4)*/ return x86Gen::Generator::ExprReturn(std::format(format4Byte, src, location), location);
    }
    case Parser::AST::Tag::EXPR_IDENTIFIER:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Identifier*>(expr);

        std::string src = funcContext.isGlobalIdentifier(castedType->name) ? std::string(castedType->name) : funcContext.getVarOrParamLocation(castedType->name);
        if (not x86Gen::Context::Global::isExternalFunction(castedType->name) and funcContext.isGlobalIdentifier(castedType->name))
        {
            src = '_' + src;
        }

        std::string location = funcContext.getTempLocation();
        std::string asmCode = x86Gen::Generator::generateMov(location, src, x86Gen::Generator::getSize(castedType->getType()), funcContext.isGlobalIdentifier(castedType->name));
        return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
    }
    case Parser::AST::Tag::EXPR_BIN:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Bin*>(expr);

        auto left = x86Gen::Generator::extractExpr(castedType->left.get(), funcContext);
        auto right = x86Gen::Generator::extractExpr(castedType->right.get(), funcContext);
        std::string prevCalc = left.asmCalc + right.asmCalc;
        std::string location = funcContext.getTempLocation();

        auto leftType = castedType->left->getType();
        auto simplefiedType = x86Gen::Generator::simplfyType(leftType);
        std::string asmCode = prevCalc;
        Assert(simplefiedType != x86Gen::Generator::PrimitiveType::NONE);

        Uint pad = NPOS;
        if (leftType->tag == Parser::AST::Type::Tag::PTR)
        {
            auto castedPtr = static_cast<const Parser::AST::Type::Ptr*>(leftType);
            pad = x86Gen::Generator::getSize(castedPtr->getTo());
        }

        if (castedType->operation == "+")
        {
            asmCode += x86Gen::Generator::generateAdd(left.location, right.location, location, simplefiedType, pad);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "-")
        {
            asmCode += x86Gen::Generator::generateSub(left.location, right.location, location, simplefiedType, pad);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "*")
        {
            asmCode += x86Gen::Generator::generateMul(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "/")
        {
            asmCode += x86Gen::Generator::generateDiv(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "%")
        {
            asmCode += x86Gen::Generator::generateMod(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "<<")
        {
            asmCode += x86Gen::Generator::generateShl(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == ">>")
        {
            asmCode += x86Gen::Generator::generateShr(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "==" or
            castedType->operation == "is")
        {
            asmCode += x86Gen::Generator::generateEq(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "!=")
        {
            asmCode += x86Gen::Generator::generateNeq(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "<")
        {
            asmCode += x86Gen::Generator::generateL(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "<=")
        {
            asmCode += x86Gen::Generator::generateLeq(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == ">")
        {
            asmCode += x86Gen::Generator::generateG(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == ">=")
        {
            asmCode += x86Gen::Generator::generateGeq(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "|")
        {
            asmCode += x86Gen::Generator::generateOrBit(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "^")
        {
            asmCode += x86Gen::Generator::generateXorBit(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "&")
        {
            asmCode += x86Gen::Generator::generateAndBit(left.location, right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "or")
        {
            std::string asmCode2 = left.asmCalc;
            asmCode2 += x86Gen::Generator::generateOr(left.location, right.location, right.asmCalc, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode2), std::move(location));
        }
        else if (castedType->operation == "and")
        {
            std::string asmCode2 = left.asmCalc;
            asmCode2 += x86Gen::Generator::generateAnd(left.location, right.location, right.asmCalc, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode2), std::move(location));
        }
        else Assert_Message(ASSERT_ALWAYS, "Invalid binary operation");
        break;
    }
    case Parser::AST::Tag::EXPR_UNARY:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Unary*>(expr);

        if (castedType->operation == "ref")
        {
            std::string dst = funcContext.getTempLocation();
            std::string asmCalc = std::format(
R"(
mov dword ptr {}, 0)", dst);

            x86Gen::Generator::appendCalculationsForRefAndAssign(dst, asmCalc, castedType->right.get(), funcContext);
            return x86Gen::Generator::ExprReturn(std::move(asmCalc), std::move(dst));
        }

        auto right = x86Gen::Generator::extractExpr(castedType->right.get(), funcContext);
        std::string prevCalc = right.asmCalc;
        std::string location = funcContext.getTempLocation();
        auto simplefiedType = x86Gen::Generator::simplfyType(castedType->right->getType());
        std::string asmCode = prevCalc;

        if (castedType->operation == "+")
        {
            asmCode += x86Gen::Generator::generatePlus(right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "-")
        {
            asmCode += x86Gen::Generator::generateMinus(right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "not")
        {
            asmCode += x86Gen::Generator::generateNot(right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "~")
        {
            asmCode += x86Gen::Generator::generateNotBit(right.location, location, simplefiedType);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }
        else if (castedType->operation == "dref")
        {
            auto innerPtrType = Parser::AST::Type::Base::getUnderlyingType(static_cast<const Parser::AST::Type::Ptr*>(castedType->right->getType())->getTo());
            auto simplefiedInnerPtrType = x86Gen::Generator::simplfyType(innerPtrType);
            asmCode += x86Gen::Generator::generateDref(right.location, location, simplefiedInnerPtrType, x86Gen::Generator::getSize(innerPtrType));
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
        }

        Assert_Message(ASSERT_ALWAYS, "Invalid unary operation");
        break;
    }
    case Parser::AST::Tag::EXPR_CALL:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Call*>(expr);
        auto func = x86Gen::Generator::extractExpr(castedType->func.get(), funcContext);

        constexpr const char* format =
R"(
{}
mov eax, dword ptr {}
call eax
add esp, {}
)";

        std::string paramsAsmCode;
        
        auto funcType = static_cast<const Parser::AST::Type::Func*>(castedType->func->getType());
        auto returnType = funcType->returnType.get();
        Uint rSize = returnType ? x86Gen::Generator::getSize(returnType) : 0;
        std::string location;
        if (returnType)
        {
            location = funcContext.getTempLocation();
        }
        
        Uint totalSize = 0;
        Uint i = castedType->params.size();
        for (auto it = castedType->params.rbegin(); it != castedType->params.rend(); it++)
        {
            bool isInEllipsis = funcType->isEllipsis and (i-- > funcType->params.size());
            auto param = it->get();
            auto paramType = param->getType();
            if (not paramType) continue;

            Uint pSize = x86Gen::Generator::getSize(paramType);
            
            auto p = x86Gen::Generator::extractExpr(param, funcContext);
            paramsAsmCode += p.asmCalc;

            if (not isInEllipsis or paramType->tag != Parser::AST::Type::Tag::PRIMITIVE)
            {
                totalSize += pSize;

                if (pSize == 1)
                {
                    constexpr const char* alloc1Byte =
R"(
sub esp, 1
mov al, byte ptr {}
mov byte ptr [esp], al
)";

                    paramsAsmCode += std::format(alloc1Byte, p.location);
                }
                else if (pSize == 4)
                {
                    constexpr const char* alloc4Byte =
R"(
sub esp, 4
mov eax, dword ptr {}
mov dword ptr [esp], eax
)";

                    paramsAsmCode += std::format(alloc4Byte, p.location);
                }
                else
                {
                    constexpr const char* allocAll =
R"(
sub esp, {}
lea esi, {}
mov edi, esp
mov ecx, {}
cld
rep movsb
)";

                    paramsAsmCode += std::format(allocAll, pSize, p.location, pSize);
                }
            }
            else // If inside the ... section I must cast those stuff. Fuck you C conventions bs.
            {
                if (pSize == 1)
                {
                    totalSize += 4;
                    constexpr const char* format1to4 =
R"(
sub esp, 4
movsx eax, byte ptr {}
mov dword ptr [esp], eax
)";

                    paramsAsmCode += std::format(format1to4, p.location);
                }
                else if (pSize == 4)
                {
                    if (paramType->tag != Parser::AST::Type::Tag::PRIMITIVE)
                    {
                        totalSize += 4;
                        constexpr const char* alloc4Byte =
R"(
sub esp, 4
mov eax, dword ptr {}
mov dword ptr [esp], eax
)";

                        paramsAsmCode += std::format(alloc4Byte, p.location);
                    }
                    else
                    {
                        auto castedTypePrimitive = static_cast<const Parser::AST::Type::Primitive*>(paramType);

                        if (castedTypePrimitive->name == "int")
                        {
                            totalSize += 4;
                            constexpr const char* alloc4Byte =
R"(
sub esp, 4
mov eax, dword ptr {}
mov dword ptr [esp], eax
)";

                            paramsAsmCode += std::format(alloc4Byte, p.location);
                        }
                        else if (castedTypePrimitive->name == "float")
                        {
                            totalSize += 8;
                            constexpr const char* allocFloatToDouble =
R"(
sub esp, 8
fld dword ptr {}
fstp qword ptr [esp]
)";

                            paramsAsmCode += std::format(allocFloatToDouble, p.location);
                        }
                    }
                }
            }
        }


        auto fullUnderType = Parser::AST::Type::Base::getUnderlyingType(returnType);
        if ((fullUnderType != nullptr) and (fullUnderType->tag == Parser::AST::Type::Tag::STRUCT or fullUnderType->tag == Parser::AST::Type::Tag::ARR))
        {
            constexpr const char* allocPtr =
R"(
sub esp, 4
lea eax, {}
mov dword ptr [esp], eax
)";
            paramsAsmCode += std::format(allocPtr, location);
            totalSize += 4;
        }

        std::string funcLocation = func.location;
        std::string fullAsmCode = func.asmCalc;
        fullAsmCode += std::format(format, paramsAsmCode, funcLocation, totalSize);

        if (returnType)
        {
            auto fullUnderType = Parser::AST::Type::Base::getUnderlyingType(returnType);

            switch (fullUnderType->tag)
            {
            case Parser::AST::Type::Tag::PRIMITIVE:
            {
                auto castedType = static_cast<const Parser::AST::Type::Primitive*>(fullUnderType);

                if (castedType->name == "float")
                {
                    constexpr const char* floatFormat =
R"(
fstp dword ptr {}
)";
                    fullAsmCode += std::format(floatFormat, location);
                }
                else
                {
                    constexpr const char* IntFormat =
R"(
mov dword ptr {}, eax
)";

                    fullAsmCode += std::format(IntFormat, location);
                }

                break;
            }
            case Parser::AST::Type::Tag::FUNC:
            case Parser::AST::Type::Tag::PTR:
            {
            constexpr const char* ptrFormat =
R"(
mov dword ptr {}, eax
)";

            fullAsmCode += std::format(ptrFormat, location);
            break;
            }
            case Parser::AST::Type::Tag::ARR:
            case Parser::AST::Type::Tag::STRUCT:       
                break;
            }
        }

        return x86Gen::Generator::ExprReturn(std::move(fullAsmCode), std::move(location));
    }
    case Parser::AST::Tag::EXPR_ARR_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::ArrAccess*>(expr);
        constexpr const char* format1ByteArr =
R"(
lea eax, {}
add eax, dword ptr {}
mov al, byte ptr [eax]
mov byte ptr {}, al
)";

        constexpr const char* format4ByteArr =
R"(
mov eax, dword ptr {}
imul eax, 4
lea ecx, {}
add eax, ecx
mov eax, dword ptr [eax]
mov dword ptr {}, eax
)";

        constexpr const char* formatAllArr =
R"(
mov eax, dword ptr {}
imul eax, {}
lea ecx, {}
add eax, ecx
mov esi, eax
lea edi, {}
mov ecx, {}
cld
rep movsb
)";
        constexpr const char* format1BytePtr =
R"(
mov eax, dword ptr {}
add eax, dword ptr {}
mov al, byte ptr [eax]
mov byte ptr {}, al
)";

        constexpr const char* format4BytePtr =
R"(
mov eax, dword ptr {}
imul eax, 4
mov ecx, dword ptr {}
add eax, ecx
mov eax, dword ptr [eax]
mov dword ptr {}, eax
)";

        constexpr const char* formatAllPtr =
R"(
mov eax, dword ptr {}
imul eax, {}
mov ecx, dword ptr {}
add eax, ecx
mov esi, eax
lea edi, {}
mov ecx, {}
cld
rep movsb
)";

        Uint size = x86Gen::Generator::getSize(castedType->getType());
        auto arrLoc = x86Gen::Generator::extractExpr(castedType->location.get(), funcContext);
        auto index = x86Gen::Generator::extractExpr(castedType->index.get(), funcContext);
        std::string location = funcContext.getTempLocation();
        std::string asmCode = arrLoc.asmCalc + index.asmCalc;

        if (castedType->location->getType()->tag == Parser::AST::Type::Tag::ARR)
        {
            if (size == 1)
            {
                asmCode += std::format(format1ByteArr, arrLoc.location, index.location, location);
            }
            else if (size == 4)
            {
                asmCode += std::format(format4ByteArr, index.location, arrLoc.location, location);
            }
            else
            {
                asmCode += std::format(formatAllArr, index.location, size, arrLoc.location, location, size);
            }
        }
        else /* if (castedType->location->getType()->tag == Parser::AST::Type::Tag::PTR) */
        {
            if (size == 1)
            {
                asmCode += std::format(format1BytePtr, arrLoc.location, index.location, location);
            }
            else if (size == 4)
            {
                asmCode += std::format(format4BytePtr, index.location, arrLoc.location, location);
            }
            else
            {
                asmCode += std::format(formatAllPtr, index.location, size, arrLoc.location, location, size);
            }
        }

        return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(location));
    }
    case Parser::AST::Tag::EXPR_STRUCT_ACCESS:
    {
        auto castedType = static_cast<const Parser::AST::Expr::StructAccess*>(expr);
        auto structType = static_cast<const Parser::AST::Type::Struct*>(castedType->location->getType());
        auto src = x86Gen::Generator::extractExpr(castedType->location.get(), funcContext);

        Sint offset = 0;
        std::string_view srcView = src.location;
        srcView.remove_prefix(4);
        srcView.remove_suffix(1);
        std::from_chars(srcView.data(), srcView.data() + srcView.size(), offset);

        Uint size = 0;
        for (auto& field : structType->fields)
        {
            if (field.name == castedType->field)
            {
                size = x86Gen::Generator::getSize(field.type.get());
                break;
            }
            offset += x86Gen::Generator::getSize(field.type.get());
        }

        src.location = std::format("[ebp{}{}]", (offset >= 0) ? '+' : '-', std::abs(offset));
        std::string dst = funcContext.getTempLocation();

        std::string asmCode = src.asmCalc;
        asmCode += x86Gen::Generator::generateMov(dst, src.location, size, false);
        return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
    }
    case Parser::AST::Tag::EXPR_SIZEOF:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Sizeof*>(expr);
        std::string dst = funcContext.getTempLocation();

        constexpr const char* format = 
R"(
mov eax, {}
mov dword ptr {}, eax
)";

        std::string asmCode = std::format(format, x86Gen::Generator::getSize(castedType->of.get()), dst);
        return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
    }
    case Parser::AST::Tag::EXPR_CAST:
    {
        auto castedType = static_cast<const Parser::AST::Expr::Cast*>(expr);

        auto leftType = castedType->expr->getType();
        auto rightType = castedType->getType();
        Uint leftTypeSize = x86Gen::Generator::getSize(leftType);
        Uint rightTypeSize = x86Gen::Generator::getSize(rightType);
        auto src = x86Gen::Generator::extractExpr(castedType->expr.get(), funcContext);
        std::string dst = funcContext.getTempLocation();
        std::string asmCode = src.asmCalc;
        bool isLeftFloat = (leftType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(leftType)->name == "float";
        bool isRightFloat = (rightType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(rightType)->name == "float";
        bool isLeftInt = (leftType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(leftType)->name == "int";
        bool isRightInt = (rightType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(rightType)->name == "int";
        bool isLeftChar = (leftType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(leftType)->name == "char";
        bool isRightChar = (rightType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(rightType)->name == "char";
        bool isLeftBool = (leftType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(leftType)->name == "bool";
        bool isRightBool = (rightType->tag == Parser::AST::Type::Tag::PRIMITIVE) and static_cast<const Parser::AST::Type::Primitive*>(rightType)->name == "bool";

        if (Parser::AST::Type::Base::compare(leftType, rightType)) // same type
        {
            asmCode += x86Gen::Generator::generateMov(dst, src.location, leftTypeSize, false);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (leftType->tag == Parser::AST::Type::Tag::PTR or rightType->tag == Parser::AST::Type::Tag::PTR)
        {
            asmCode += x86Gen::Generator::generateMov(dst, src.location, leftTypeSize, false);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftFloat and isRightInt) // float -> int
        {
            constexpr const char* format = 
R"(
fld dword ptr {}
fistp dword ptr {}
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftInt and isRightFloat) // int -> float
        {
            constexpr const char* format =
R"(
fild dword ptr {}
fstp dword ptr {}
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftInt and isRightChar) // int -> char
        {
            constexpr const char* format =
R"(
mov eax, dword ptr {}
mov byte ptr {}, al
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftChar and isRightInt) // char -> int
        {
            constexpr const char* format =
R"(
movsx eax, byte ptr {}
mov dword ptr {}, eax
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftInt and isRightBool) // int -> bool
        {
            constexpr const char* format =
R"(
mov eax, dword ptr {}
cmp eax, 0
setne al
mov byte ptr {}, al
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftBool and isRightInt) // bool -> int
        {
            constexpr const char* format =
R"(
xor eax, eax
mov al, byte ptr {}
mov dword ptr {}, eax
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftChar and isRightBool) // char -> bool
        {
            constexpr const char* format =
R"(
mov al, byte ptr {}
cmp al, 0
setne al
mov byte ptr {}, al
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        if (isLeftBool and isRightChar) // bool -> char
        {
            constexpr const char* format =
R"(
mov al, byte ptr {}
mov byte ptr {}, al
)";

            asmCode += std::format(format, src.location, dst);
            return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
        }
        Assert_Message(ASSERT_ALWAYS, "Invalid casting");

        break;
    }
    case Parser::AST::Tag::EXPR_ARR_INIT:
    {
        auto castedType = static_cast<const Parser::AST::Expr::ArrInit*>(expr);
        std::string dst = funcContext.getTempLocation();

        std::string asmCode;
        Sint offset = 0;
        std::string_view srcView = dst;
        srcView.remove_prefix(4);
        srcView.remove_suffix(1);
        std::from_chars(srcView.data(), srcView.data() + srcView.size(), offset);
        Uint jumpSize = x86Gen::Generator::getSize(castedType->elements[0]->getType());
        char sign = offset >= 0 ? '+' : '-';
        offset = std::abs(offset);

        for (auto& element : castedType->elements)
        {
            std::string currentDst = std::format("[ebp{}{}]", sign, offset);
            auto src = x86Gen::Generator::extractExpr(element.get(), funcContext);

            asmCode += src.asmCalc;
            asmCode += x86Gen::Generator::generateMov(currentDst, src.location, jumpSize, false);
            offset -= jumpSize;
        }

        return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
    }
    case Parser::AST::Tag::EXPR_STRUCT_INIT:
    {
        auto castedType = static_cast<const Parser::AST::Expr::StructInit*>(expr);
        std::string dst = funcContext.getTempLocation();

        std::string asmCode;
        Sint offset = 0;
        std::string_view srcView = dst;
        srcView.remove_prefix(4);
        srcView.remove_suffix(1);
        std::from_chars(srcView.data(), srcView.data() + srcView.size(), offset);
        char sign = offset >= 0 ? '+' : '-';
        offset = std::abs(offset);

        for (auto& field : castedType->fields)
        {
            std::string currentDst = std::format("[ebp{}{}]", sign, offset);
            auto src = x86Gen::Generator::extractExpr(field.get(), funcContext);
            Uint size = x86Gen::Generator::getSize(field->getType());

            asmCode += src.asmCalc;
            asmCode += x86Gen::Generator::generateMov(currentDst, src.location, size, false);
            offset -= size;
        }

        return x86Gen::Generator::ExprReturn(std::move(asmCode), std::move(dst));
    }
    }

    Assert_Message(ASSERT_ALWAYS, "Invalid expression");
    return x86Gen::Generator::ExprReturn({}, {});
}
std::string x86Gen::Generator::generateAssign(const Parser::AST::Stmt::Assign* assign, x86Gen::Context::Func& funcContext)
{
    std::string dst;
    std::string src;
    std::string asmCode;
    const Parser::AST::Expr::Base* base = assign->dst.get();

    auto srcFull = x86Gen::Generator::extractExpr(assign->src.get(), funcContext);
    std::string tmpDst = funcContext.getTempLocation();
    asmCode += std::format("mov dword ptr {}, 0", tmpDst);
    x86Gen::Generator::appendCalculationsForRefAndAssign(tmpDst, asmCode, assign->dst.get(), funcContext);

    constexpr const char* formatDst =
R"(
mov ebx, dword ptr {}
)";
    asmCode += srcFull.asmCalc;
    asmCode += std::format(formatDst, tmpDst);
    dst = "[ebx]";
    src = srcFull.location;

    if (assign->operation == "=")
    {
        asmCode += x86Gen::Generator::generateMovAssign(src, x86Gen::Generator::getSize(base->getType()));
    }
    else if (assign->operation == "+=")
    {
        Uint pad = NPOS;
        auto dstType = assign->dst->getType();
        if (dstType->tag == Parser::AST::Type::Tag::PTR)
        {
            pad = x86Gen::Generator::getSize(static_cast<const Parser::AST::Type::Ptr*>(dstType)->getTo());
            if (pad == 0) pad = 1;
        }

        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateAdd(dst, src, dst, simplfiedType, pad);
    }
    else if (assign->operation == "-=")
    {
        Uint pad = NPOS;
        auto dstType = assign->dst->getType();
        if (dstType->tag == Parser::AST::Type::Tag::PTR)
        {
            pad = x86Gen::Generator::getSize(static_cast<const Parser::AST::Type::Ptr*>(dstType)->getTo());
            if (pad == 0) pad = 1;
        }

        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateSub(dst, src, dst, simplfiedType, pad);
    }
    else if (assign->operation == "*=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateMul(dst, src, dst, simplfiedType);
    }
    else if (assign->operation == "/=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateDiv(dst, src, dst, simplfiedType);
    }
    else if (assign->operation == "%=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateMod(dst, src, dst, simplfiedType);
    }
    else if (assign->operation == "|=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateOrBit(dst, src, dst, simplfiedType);
    }
    else if (assign->operation == "&=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateAndBit(dst, src, dst, simplfiedType);
    }
    else if (assign->operation == "^=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateXorBit(dst, src, dst, simplfiedType);
    }
    else if (assign->operation == "<<=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateShl(dst, src, dst, simplfiedType);
    }
    else if (assign->operation == ">>=")
    {
        auto simplfiedType = x86Gen::Generator::simplfyType(base->getType());
        asmCode += x86Gen::Generator::generateShr(dst, src, dst, simplfiedType);
    }
    else Assert_Message(ASSERT_ALWAYS, "Invalid assign operation");

    return asmCode;
}
std::string x86Gen::Generator::generateMov(const std::string& dst, const std::string& src, Uint size, bool isFunc)
{
    constexpr const char* format1Byte = // (bool, char, struct with the same size).
R"(
mov al, byte ptr {}
mov byte ptr {}, al
)";

    constexpr const char* format4Byte = // (int, float, struct with the same size).
R"(
mov eax, dword ptr {}
mov dword ptr {}, eax
)";

    constexpr const char* format4ByteFunc =
R"(
mov eax, offset {}
mov dword ptr {}, eax
)";

    constexpr const char* formatAll = // (struct that their size differ 1 and 4).
R"(
lea esi, {}
lea edi, {}
mov ecx, {}
cld
rep movsb
)";

    std::string asmCode;
    switch (size)
    {
    case 1:
    {
        asmCode = std::format(format1Byte, src, dst);
        break;
    }
    case 4:
    {
        if (isFunc) asmCode = std::format(format4ByteFunc, src, dst);
        else asmCode = std::format(format4Byte, src, dst);
        break;
    }
    default:
    {
        asmCode = std::format(formatAll, src, dst, size); // This could be done for all types. But the optimization is too easy so why not.
        break;
    }
    }

    return asmCode;
}
std::string x86Gen::Generator::generateMovAssign(const std::string& src, Uint size)
{
    constexpr const char* format1Byte =
R"(
mov al, byte ptr {}
mov byte ptr [ebx], al
)";

    constexpr const char* format4Byte =
R"(
mov eax, dword ptr {}
mov dword ptr [ebx], eax
)";

    constexpr const char* formatAll =
R"(
lea esi, {}
mov edi, ebx
mov ecx, {}
cld
rep movsb
)";

    switch (size)
    {
    case 1:
        return std::format(format1Byte, src);
    case 4:
        return std::format(format4Byte, src);
    }

    return std::format(formatAll, src, size);
}
std::string x86Gen::Generator::generateAdd(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type, Uint padSize)
{
    if (padSize != NPOS)
    {
        // (int*)(10) + (int*)(2) = 10 + 2 * sizeof(int)
        constexpr const char* formatInt32Pad =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
imul ecx, {}
add eax, ecx
mov dword ptr {}, eax
)";

        return std::format(formatInt32Pad, left, right, padSize, dst);
    }

    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
add al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
add eax, ecx
mov dword ptr {}, eax
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
faddp 
fstp dword ptr {}
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateSub(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type, Uint padSize)
{
    if (padSize != NPOS)
    {
        // (int*)(10) + (int*)(2) = 10 + 2 * sizeof(int)
        constexpr const char* formatInt32Pad =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
imul ecx, {}
sub eax, ecx
mov dword ptr {}, eax
)";

        return std::format(formatInt32Pad, left, right, padSize, dst);
    }

    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
sub al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
sub eax, ecx
mov dword ptr {}, eax
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fsubp
fstp dword ptr {}
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateMul(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
imul al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
imul eax, ecx
mov dword ptr {}, eax
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fmulp
fstp dword ptr {}
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateDiv(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
cbw
mov cl, byte ptr {}
idiv cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
cdq
mov ecx, dword ptr {}
idiv ecx
mov dword ptr {}, eax
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fdivp
fstp dword ptr {}
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateMod(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
cbw
mov cl, byte ptr {}
idiv cl
mov byte ptr {}, ah
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
cdq
mov ecx, dword ptr {}
idiv ecx
mov dword ptr {}, edx
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateShl(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
sal al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
sal eax, cl
mov dword ptr {}, eax
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateShr(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
sar al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
sar eax, cl
mov dword ptr {}, eax
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateEq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
cmp al, cl
sete al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
cmp eax, ecx
sete al
mov byte ptr {}, al
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fucom
fstsw ax
sahf
sete al
mov byte ptr {}, al
fstp st(0)
fstp st(0)
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, left, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateNeq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
cmp al, cl
setne al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
cmp eax, ecx
setne al
mov byte ptr {}, al
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fucom
fstsw ax
sahf
setne al
mov byte ptr {}, al
fstp st(0)
fstp st(0)
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, left, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateL(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
cmp al, cl
setl al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
cmp eax, ecx
setl al
mov byte ptr {}, al
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fucom
fstsw ax
sahf
setb al
mov byte ptr {}, al
fstp st(0)
fstp st(0)
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, left, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateLeq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
cmp al, cl
setle al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
cmp eax, ecx
setle al
mov byte ptr {}, al
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fucom
fstsw ax
sahf
setbe al
mov byte ptr {}, al
fstp st(0)
fstp st(0)
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, left, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateG(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
cmp al, cl
setg al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
cmp eax, ecx
setg al
mov byte ptr {}, al
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fucom
fstsw ax
sahf
seta al
mov byte ptr {}, al
fstp st(0)
fstp st(0)
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, left, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateGeq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
cmp al, cl
setge al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
cmp eax, ecx
setge al
mov byte ptr {}, al
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fld dword ptr {}
fucom
fstsw ax
sahf
setae al
mov byte ptr {}, al
fstp st(0)
fstp st(0)
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, left, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateOrBit(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
or al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
or eax, ecx
mov dword ptr {}, eax
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateXorBit(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
xor al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
xor eax, ecx
mov dword ptr {}, eax
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateAndBit(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov cl, byte ptr {}
and al, cl
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov ecx, dword ptr {}
and eax, ecx
mov dword ptr {}, eax
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, left, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateOr(const std::string& left, const std::string& right, const std::string& rightCalc, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
// This expression for instance. myBool or foo().
// If myBool is True foo() will not be called.
// That's why more complex asm is required here.
    static Uint shortCircuitOrCount = 0;
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
cmp al, 0
jne or_short_circuit_{}
{}
mov al, byte ptr {}
cmp al, 0
setne al
mov byte ptr {}, al
jmp or_short_circuit_{}_exit
or_short_circuit_{}:
mov al, 1
mov byte ptr {}, al
or_short_circuit_{}_exit:
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, shortCircuitOrCount, rightCalc, right, dst, shortCircuitOrCount, shortCircuitOrCount, dst, shortCircuitOrCount);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    shortCircuitOrCount++;
    return asmCode;
}
std::string x86Gen::Generator::generateAnd(const std::string& left, const std::string& right, const std::string& rightCalc, const std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    // This expression for instance. myBool and foo().
    // If myBool is False foo() will not be called.
    // That's why more complex asm is required here.
    static Uint shortCircuitAndCount = 0;
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
cmp al, 0
jz and_short_circuit_{}
{}
mov al, byte ptr {}
cmp al, 0
setne al
mov byte ptr {}, al
jmp and_short_circuit_{}_exit
and_short_circuit_{}:
mov al, 0
mov byte ptr {}, al
and_short_circuit_{}_exit:
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, left, shortCircuitAndCount, rightCalc, right, dst, shortCircuitAndCount, shortCircuitAndCount, dst, shortCircuitAndCount);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    shortCircuitAndCount++;
    return asmCode;
}
std::string x86Gen::Generator::generatePlus(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type)
{
// Does nothing but move.
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
mov dword ptr {}, eax
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fstp dword ptr {}
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateMinus(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
neg al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
neg eax
mov dword ptr {}, eax
)";

    constexpr const char* formatFloat =
R"(
fld dword ptr {}
fchs
fstp dword ptr {}
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatFloat, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateNot(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
xor al, 1
mov byte ptr {}, al
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateNotBit(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type)
{
    constexpr const char* formatInt8 =
R"(
mov al, byte ptr {}
not al
mov byte ptr {}, al
)";

    constexpr const char* formatInt32 =
R"(
mov eax, dword ptr {}
not eax
mov dword ptr {}, eax
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    {
        asmCode = std::format(formatInt32, right, dst);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}
std::string x86Gen::Generator::generateDref(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type, Uint size)
{
    constexpr const char* formatInt8 =
R"(
mov eax, dword ptr {}
mov al, byte ptr [eax]
mov byte ptr {}, al
)";

    constexpr const char* formatInt32NFloat =
R"(
mov eax, dword ptr {}
mov eax, dword ptr [eax]
mov dword ptr {}, eax
)";


        constexpr const char* formatAll =
R"(
mov esi, {}
lea edi, {}
mov ecx, {}
cld
rep movsb
)";

    std::string asmCode;
    switch (type)
    {
    case x86Gen::Generator::PrimitiveType::INT8:
    {
        asmCode = std::format(formatInt8, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::INT32:
    case x86Gen::Generator::PrimitiveType::FLOAT:
    {
        asmCode = std::format(formatInt32NFloat, right, dst);
        break;
    }
    case x86Gen::Generator::PrimitiveType::NONE:
    {
        asmCode = std::format(formatAll, right, dst, size);
        break;
    }
    default:
        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
    }

    return asmCode;
}

Uint x86Gen::Generator::getSize(const Parser::AST::Type::Base* type)
{
    if (not type) return 0;

    switch (type->tag)
    {
    case Parser::AST::Type::Tag::IDENTIFIER:
    {
        auto castedType = static_cast<const Parser::AST::Type::Identifier*>(type);
        return x86Gen::Generator::getSize(castedType->origin->getType());
        break;
    }
    case Parser::AST::Type::Tag::PRIMITIVE:
    {
        auto castedType = static_cast<const Parser::AST::Type::Primitive*>(type);

        if (castedType->name == "bool") return 1;
        if (castedType->name == "char") return 1;
        if (castedType->name == "int") return 4;
        if (castedType->name == "float") return 4;
        Assert_Message(ASSERT_ALWAYS, "Invalid type");
        return 0;
        break;
    }
    case Parser::AST::Type::Tag::TYPEDEF:
    {
        auto castedType = static_cast<const Parser::AST::Type::Typedef*>(type);
        return x86Gen::Generator::getSize(castedType->from.get());
        break;
    }
    case Parser::AST::Type::Tag::PTR:
    {
        return 4;
        break;
    }
    case Parser::AST::Type::Tag::ARR:
    {
        auto castedType = static_cast<const Parser::AST::Type::Arr*>(type);

        const Lexer::Token& token = static_cast<const Parser::AST::Expr::Literal*>(castedType->size.get())->literal;
        const std::string_view leftLiteral = token.content;
        Uint size = 0;
        Uint offset = (token.tag == Lexer::Tag::INT_LITERAL) ? (0) : (2);
        std::from_chars(leftLiteral.data() + offset, leftLiteral.data() + leftLiteral.size(), size);

        return size * x86Gen::Generator::getSize(castedType->getType());
        break;
    }
    case Parser::AST::Type::Tag::FUNC:
    {
        return 4;
        break;
    }
    case Parser::AST::Type::Tag::STRUCT:
    {
        auto castedType = static_cast<const Parser::AST::Type::Struct*>(type);

        // What about padding. The cdecl abi requires it when dealing with structs? ()
        // No I ain't doing this shit. printf will work fine and it's all I need for the showcase.
        Uint sum = 0;
        for (auto& field : castedType->fields)
        {
            sum += x86Gen::Generator::getSize(field.type.get());
        }

        return sum;
        break;
    }
    {
        return 4;
        break;
    }
    }

    Assert_Message(ASSERT_ALWAYS, "Invalid type");
    return 0;
}
x86Gen::Generator::PrimitiveType x86Gen::Generator::simplfyType(const Parser::AST::Type::Base* type)
{
    if (not type) return x86Gen::Generator::PrimitiveType::NONE;

    switch (type->tag)
    {
    case Parser::AST::Type::Tag::IDENTIFIER:
    {
        auto castedType = static_cast<const Parser::AST::Type::Identifier*>(type);
        return x86Gen::Generator::simplfyType(castedType->origin->getType());
    }
    case Parser::AST::Type::Tag::PRIMITIVE:
    {
        auto castedType = static_cast<const Parser::AST::Type::Primitive*>(type);

        if (castedType->name == "bool") return x86Gen::Generator::PrimitiveType::INT8;
        if (castedType->name == "char") return x86Gen::Generator::PrimitiveType::INT8;
        if (castedType->name == "int") return x86Gen::Generator::PrimitiveType::INT32;
        if (castedType->name == "float") return x86Gen::Generator::PrimitiveType::FLOAT;

        Assert_Message(ASSERT_ALWAYS, "Invalid primitive type");
        return x86Gen::Generator::PrimitiveType::NONE;
    }
    case Parser::AST::Type::Tag::TYPEDEF:
    {
        auto castedType = static_cast<const Parser::AST::Type::Typedef*>(type);
        return x86Gen::Generator::simplfyType(castedType->from.get());
    }
    case Parser::AST::Type::Tag::PTR:
    case Parser::AST::Type::Tag::FUNC:
        return x86Gen::Generator::PrimitiveType::INT32;

    case Parser::AST::Type::Tag::ARR:
    case Parser::AST::Type::Tag::STRUCT:
        return x86Gen::Generator::PrimitiveType::NONE;
    }

    return x86Gen::Generator::PrimitiveType::NONE;
}