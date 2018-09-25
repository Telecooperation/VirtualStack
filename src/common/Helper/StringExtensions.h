
#pragma once

#include <string>
#include <algorithm>

namespace StringExtensions
{
	static inline std::string& toLower(std::string& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
		{
			return std::tolower(c);
		});

		return str;
	}
	
	static inline std::string toLower(const std::string& str)
	{
		std::string tmpReturn = str;
		return StringExtensions::toLower(tmpReturn);
	}
}
