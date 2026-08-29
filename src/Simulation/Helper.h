#pragma once
#include <string>
#include <vector>
#include <sstream>

namespace DIQKD_ns
{
	namespace Helper
	{
		template <typename T>
		static std::string GetNumericString(const std::vector<T>& data)
		{
			std::stringstream ss;
			for (auto x : data)
				ss << std::max(0, (int)x);

			return ss.str();
		}
	};
}