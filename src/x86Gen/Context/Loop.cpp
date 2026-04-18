#include <x86Gen/Context/Loop.hpp>

x86Gen::Context::Loop::Loop(std::string new_labelStart, std::string new_labelExit) :
	labelStart(std::move(new_labelStart)), labelExit(std::move(new_labelExit))
{
}

void x86Gen::Context::Loop::operator = (const x86Gen::Context::Loop& other)
{
	this->labelStart = other.labelStart;
	this->labelExit = other.labelExit;
	this->labelIter = other.labelIter;
}
