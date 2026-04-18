#pragma once
#include <string>

namespace x86Gen::Context
{
	struct Loop
	{
		Loop(void) = default;
		Loop(std::string new_labelStart, std::string new_labelExit);

		void operator = (const x86Gen::Context::Loop& other);

		std::string labelStart;
		std::string labelExit;
		std::string labelIter; // Only for 'for'.
	};
}
