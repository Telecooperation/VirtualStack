
#pragma once

#include <string>
#include "SettingsProvider.h"
#include "../../Helper/StringExtensions.h"

template<typename T>
class SettingsEntry
{
public:
	SettingsEntry(const std::string &keyName, const T pValue) : key(StringExtensions::toLower(keyName)), value(pValue) {}

	const std::string key;
	const T value;

	template<typename TNewType>
	TNewType as() const
	{
		return static_cast<TNewType>(value);
	}
	
	operator const T&() const noexcept {
		return value;
	}
};
