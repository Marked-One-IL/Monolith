#include <x86Gen/Context/Func.hpp>
#include <Helper/Assert.hpp>

x86Gen::Context::Func::Func(std::string_view name) : m_name(name)
{
}

void x86Gen::Context::Func::allocateParam(std::string_view name, Uint size)
{
	if (size == 0) return;

	this->m_varsAndParams[name] = this->m_paramOffset;
	this->m_paramOffset += size;
}
void x86Gen::Context::Func::allocateVar(std::string_view name, Uint size)
{
	if (size == 0) return;

	this->m_localOffset -= size;
	this->m_varsAndParams[name] = this->m_localOffset;
	this->m_totalStackSize += size;
}
void x86Gen::Context::Func::allocateTemp(Uint size)
{
	if (size == 0) return;

	this->m_localOffset -= size;
	this->m_tmps.emplace_back(this->m_localOffset);
	this->m_totalStackSize += size;
}

bool x86Gen::Context::Func::isGlobalIdentifier(std::string_view name) const
{
	return this->m_varsAndParams.find(name) == this->m_varsAndParams.end();
}
std::string x86Gen::Context::Func::getVarOrParamLocation(std::string_view name) const
{
	const auto it = this->m_varsAndParams.find(name);
	Assert(it != this->m_varsAndParams.end());

	const Sint offset = it->second;
	const char sign = (offset >= 0) ? ('+') : ('-');
	return std::format("[ebp{}{}]", sign, std::abs(offset));
}
std::string x86Gen::Context::Func::getTempLocation(void)
{
	Assert(this->m_tempCount < this->m_tmps.size());

	const Sint offset = this->m_tmps[this->m_tempCount++];
	const char sign = (offset >= 0) ? ('+') : ('-');
	return std::format("[ebp{}{}]", sign, std::abs(offset));
}
Uint x86Gen::Context::Func::getTotalStackSize(void) const
{
	return this->m_totalStackSize;
}

std::string_view x86Gen::Context::Func::getName(void) const
{
	return this->m_name;
}
