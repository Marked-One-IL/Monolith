#pragma once
#include <Parser/Generator.hpp>
#include <Parser/AST/Decl/Func.hpp>
#include <Parser/AST/Stmt/Base.hpp>
#include <x86Gen/Context/Func.hpp>
#include <x86Gen/Context/Loop.hpp>
#include <list>
#include <optional>

namespace x86Gen
{
	class Generator
	{
	public:
		Generator(const Parser::Generator& parser);

		const std::string& getAsm(void) const;
		
	private:
		enum class PrimitiveType
		{
			INT8,
			INT32,
			FLOAT,
			NONE
		};
		struct ExprReturn
		{
			ExprReturn(std::string new_asmCalc, std::string new_location);

			std::string asmCalc;
			std::string location;
		};

		static void allocateExpr(const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext);
		static void allocateRefAndAssign(const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext);
		static void allocateBlock(const Parser::AST::Block* block, x86Gen::Context::Func& funcContext);
		static x86Gen::Context::Func getFuncContext(const Parser::AST::Decl::Func* func);
		static x86Gen::Context::Loop getLoopContext(const Parser::AST::Stmt::Base* loop);
		static void appendCalculationsForRefAndAssign(const std::string& dst, std::string& asmCalc, const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext);

		static std::string extractFunc(const Parser::AST::Decl::Func* func, Uint depth);
		static std::string extractBlock(const Parser::AST::Block* block, x86Gen::Context::Func& funcContext, const x86Gen::Context::Loop& prevLoopContext, Uint depth);
		static x86Gen::Generator::ExprReturn extractExpr(const Parser::AST::Expr::Base* expr, x86Gen::Context::Func& funcContext);
		static std::string generateAssign(const Parser::AST::Stmt::Assign* assign, x86Gen::Context::Func& funcContext);
		static std::string generateMov(const std::string& dst, const std::string& src, Uint size, bool isFunc);
		static std::string generateMovAssign(const std::string& src, Uint size);
		static std::string generateAdd(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type, Uint padSize);
		static std::string generateSub(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type, Uint padSize);
		static std::string generateMul(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateDiv(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateMod(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateShl(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateShr(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateEq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateNeq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateL(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateLeq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateG(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateGeq(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateOrBit(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateXorBit(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateAndBit(const std::string& left, const std::string& right, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateOr(const std::string& left, const std::string& right, const std::string& rightCalc, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateAnd(const std::string& left, const std::string& right, const std::string& rightCalc, const std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generatePlus(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateMinus(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateNot(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateNotBit(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type);
		static std::string generateDref(const std::string& right, std::string& dst, x86Gen::Generator::PrimitiveType type, Uint size);

		static Uint getSize(const Parser::AST::Type::Base* type);
		static x86Gen::Generator::PrimitiveType simplfyType(const Parser::AST::Type::Base* type);

		std::string m_asm;
	};
}
