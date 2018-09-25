#include "SettingsProvider.h"
#include "SettingsParser.h"

template<typename T>
T SettingsProvider::get(std::string name, bool &readFailed)
{
    return T();
}


template <>
std::string SettingsProvider::get<std::string>(std::string name, bool &readFailed)
{
	return getString(name, readFailed);
}

template <>
std::vector<std::string> SettingsProvider::get<std::vector<std::string>>(std::string name, bool &readFailed)
{
	auto tmpValue = getString(name, readFailed);
	if(readFailed)
		return std::vector<std::string>();
		
	return SettingsParser::splitString(tmpValue, ',');
}

template <>
std::vector<int> SettingsProvider::get<std::vector<int>>(std::string name, bool &readFailed)
{
	auto tmpValue = getString(name, readFailed);
	if(readFailed)
		return std::vector<int>();
	
	auto tmpSplitStrings = SettingsParser::splitString(tmpValue, ',');
	std::vector<int> tmpResult;
	tmpResult.reserve(tmpSplitStrings.size());
	
	if(!std::all_of(tmpSplitStrings.begin(), tmpSplitStrings.end(), &SettingsParser::isInteger))
	{
		readFailed = true;
		Logger::Log(Logger::ERROR, "At least one entry for: ", name, " is no integer value");
		return std::vector<int>();
	}
	
	std::transform(tmpSplitStrings.begin(), tmpSplitStrings.end(), std::back_inserter(tmpResult), [](const std::string& v) { return std::stoi(v); });
	
	return tmpResult;
}

template <>
std::vector<size_t> SettingsProvider::get<std::vector<size_t>>(std::string name, bool &readFailed)
{
	auto tmpValue = getString(name, readFailed);
	if(readFailed)
		return std::vector<size_t>();
	
	auto tmpSplitStrings = SettingsParser::splitString(tmpValue, ',');
	std::vector<size_t> tmpResult;
	tmpResult.reserve(tmpSplitStrings.size());
	
	if(!std::all_of(tmpSplitStrings.begin(), tmpSplitStrings.end(), &SettingsParser::isSizeT))
	{
		readFailed = true;
		Logger::Log(Logger::ERROR, "At least one entry for: ", name, " is no size_t value");
		return std::vector<size_t>();
	}
	
	std::transform(tmpSplitStrings.begin(), tmpSplitStrings.end(), std::back_inserter(tmpResult), [](const std::string& v) { return std::stoull(v); });
	
	return tmpResult;
}

template <>
int SettingsProvider::get<int>(std::string name, bool &readFailed)
{
	return getInteger(name, readFailed);
}

template <>
size_t SettingsProvider::get<size_t>(std::string name, bool &readFailed)
{
	return getSizeT(name, readFailed);
}

template <>
float SettingsProvider::get<float>(std::string name, bool &readFailed)
{
	return getFloat(name, readFailed);
}

std::string SettingsProvider::getString(std::string name, bool &readFailed)
{
	auto tmpIndex = _stringStorage.find(name);
	if(tmpIndex == _stringStorage.end())
	{
		readFailed = true;
		printKeyNotFoundMessage("string", name);
		return ""; //what to return?
	}
	return tmpIndex->second;
}

int SettingsProvider::getInteger(std::string name, bool &readFailed)
{
	auto tmpIndex = _integerStorage.find(name);
	if(tmpIndex == _integerStorage.end())
	{
		readFailed = true;
		printKeyNotFoundMessage("integer", name);
		return 0; //what to return?
	}
	return tmpIndex->second;
}

size_t SettingsProvider::getSizeT(std::string name, bool &readFailed)
{
	auto tmpIndex = _sizeTStorage.find(name);
	if(tmpIndex == _sizeTStorage.end())
	{
		readFailed = true;
		printKeyNotFoundMessage("size_t", name);
		return 0; //what to return?
	}
	return tmpIndex->second;
}

float SettingsProvider::getFloat(std::string name, bool &readFailed)
{
	auto tmpIndex = _floatStorage.find(name);
	if(tmpIndex == _floatStorage.end())
	{
		readFailed = true;
		printKeyNotFoundMessage("float", name);
		return 0.0f; //what to return?
	}
	return tmpIndex->second;
}

bool SettingsProvider::ReadSettings(std::istream& file)
{
	SettingsParser tmpParser;
	if(!tmpParser.Parse(file))
		return false;

	_integerStorage = std::move(tmpParser.IntegerStorage);
	_sizeTStorage = std::move(tmpParser.SizeTStorage);
	_stringStorage = std::move(tmpParser.StringStorage);
	_floatStorage = std::move(tmpParser.FloatStorage);
	return true;
}

bool SettingsProvider::ReadSettings(std::string file)
{
	std::ifstream fileStream (file);
	if (!fileStream.is_open())
		return false;

	bool tmpResult = ReadSettings(fileStream);

	fileStream.close();

	return tmpResult;
}

void inline SettingsProvider::printKeyNotFoundMessage(std::string keyType, std::string& key)
{
	Logger::Log(Logger::ERROR, keyType, " with name \"", key, "\" was not found in settings");
}
