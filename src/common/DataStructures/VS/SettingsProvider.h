
#pragma once

#include <assert.h>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include "../../Helper/ClassMacros.h"

class SettingsProvider
{
public:
	SettingsProvider() { }
	
	bool ReadSettings(std::string file);

	bool ReadSettings(std::istream& file);

	template<typename T>
	T get(std::string name, bool &readFailed);

	int getInteger(std::string name, bool &readFailed);
	
	size_t getSizeT(std::string name, bool &readFailed);

	std::string getString(std::string name, bool &readFailed);

	float getFloat(std::string name, bool &readFailed);

	ALLOW_MOVE_SEMANTICS_ONLY(SettingsProvider);
private:

	std::map<std::string, int> _integerStorage;
	std::map<std::string, size_t> _sizeTStorage;
	std::map<std::string, std::string> _stringStorage;
	std::map<std::string, float> _floatStorage;

	void inline printKeyNotFoundMessage(std::string keyType, std::string& key);
};

template <>
std::string SettingsProvider::get<std::string>(std::string name, bool &readFailed);

template <>
std::vector<std::string> SettingsProvider::get<std::vector<std::string>>(std::string name, bool &readFailed);

template <>
std::vector<int> SettingsProvider::get<std::vector<int>>(std::string name, bool &readFailed);

template <>
std::vector<size_t> SettingsProvider::get<std::vector<size_t>>(std::string name, bool &readFailed);

template <>
int SettingsProvider::get<int>(std::string name, bool &readFailed);

template <>
size_t SettingsProvider::get<size_t>(std::string name, bool &readFailed);

template <>
float SettingsProvider::get<float>(std::string name, bool &readFailed);