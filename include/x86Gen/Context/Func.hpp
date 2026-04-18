#pragma once
#include <vector>
#include <unordered_map>
#include <Parser/AST/Decl/Var.hpp>
#include <Parser/AST/Type/Func.hpp>
#include <list>

namespace x86Gen::Context
{
	class Func
	{
	public:
		Func(std::string_view name);

		void allocateParam(std::string_view name, Uint size);
		void allocateVar(std::string_view name, Uint size);
		void allocateTemp(Uint size);

		bool isGlobalIdentifier(std::string_view name) const;
		std::string getVarOrParamLocation(std::string_view name) const;
		std::string getTempLocation(void);
		Uint getTotalStackSize(void) const;

		std::string_view getName(void) const;

	private:
		Uint m_paramOffset = 8;
		Sint m_localOffset = 0;
		Uint m_totalStackSize = 0;
		Uint m_tempCount = 0;
		std::unordered_map<std::string_view, Sint> m_varsAndParams;
		std::vector<Sint> m_tmps;
		std::string_view m_name;
	};
}
