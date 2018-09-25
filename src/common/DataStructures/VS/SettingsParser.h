
#pragma once

#include "../../Helper/Logger.h"
#include "../../Helper/StringExtensions.h"
#include <fstream>
#include <map>
#include <string>
#include <vector>

class SettingsParser
{
public:
	std::map<std::string, int> IntegerStorage;
	std::map<std::string, size_t> SizeTStorage;
	std::map<std::string, std::string> StringStorage;
	std::map<std::string, float> FloatStorage;

	bool Parse(std::istream& stream)
	{
		std::string line;
		while (std::getline (stream,line) )
		{
			auto tmpParsedLine = parseLine(line);
			//Wenn Zeile ungültig, überspringe sie (oder exception oder so?)
			if(tmpParsedLine.first.length() == 0 || tmpParsedLine.second.length() == 0)
				continue;

			StringExtensions::toLower(tmpParsedLine.first);

			if(isString(tmpParsedLine.second))
				StringStorage.emplace(tmpParsedLine.first, trimCharacter(tmpParsedLine.second, '"'));
			else if(isInteger(tmpParsedLine.second))
				IntegerStorage.emplace(tmpParsedLine.first, std::stoi(tmpParsedLine.second));
			else if(isSizeT(tmpParsedLine.second))
				SizeTStorage.emplace(tmpParsedLine.first, std::stoull(tmpParsedLine.second));
			else if(isFloat(tmpParsedLine.second))
				FloatStorage.emplace(tmpParsedLine.first, std::stod(tmpParsedLine.second));
			else
				Logger::Log(Logger::WARNING, "The following line could not be parsed by settingsParser: ", line);
		}

		return true;
	}

	static std::pair<std::string, std::string> parseLine(std::string& line)
	{
		auto firstIndexOfSeparator = line.find_first_of('=');
		if(firstIndexOfSeparator == line.npos)
			return std::pair<std::string, std::string>("", "");

		return std::pair<std::string, std::string>(
				line.substr(0, firstIndexOfSeparator),
				line.substr(firstIndexOfSeparator+1));
	}
	
	static std::vector<std::string> splitString(std::string& line, char separator)
	{
		std::vector<std::string> tmpResult;
		
		unsigned long currentIndexOf = 0;
		auto indexOfSeparator = line.find_first_of(separator, currentIndexOf);
		while(indexOfSeparator != line.npos)
		{
			auto tmpTokenStr = line.substr(currentIndexOf, (indexOfSeparator - currentIndexOf));
			tmpResult.push_back(trimCharacter(tmpTokenStr, ' '));
			
			currentIndexOf = indexOfSeparator + 1;
			indexOfSeparator = line.find_first_of(separator, currentIndexOf);
		}
		
		auto tmpTokenStr = line.substr(currentIndexOf);
		tmpResult.push_back(trimCharacter(tmpTokenStr, ' '));
		
		//remove all empty entries
		tmpResult.erase(std::remove_if(tmpResult.begin(), tmpResult.end(), std::mem_fn(&std::string::empty)), tmpResult.end());
		
		return tmpResult;
	}

	static bool isString(std::string& value)
	{
        return startsWith(value, '"') && endsWith(value, '"');
	}

	static bool isInteger(std::string& value)
	{
		return isNumber(value);
	}

	static bool isSizeT(std::string& value)
	{
		if(value.size() < 3)
			return false;
		auto it = value.find("ul");
		if(it == value.npos)
			return false;
		return isNumber(value.substr(0, value.size() - 2));
	}

	static bool isFloat(std::string& value)
	{
		auto split = splitString(value, '.');
		if(split.size() != 2)
			return false;
		return isNumber(split[0]) && isNumber(split[1]);
	}

	static std::string trimCharacter(std::string &str, char character)
	{
		size_t first = str.find_first_not_of(character);
		if(first == str.npos) //str besteht nur aus character oder ist selber leer
			return "";
		size_t last = str.find_last_not_of(character);

		auto val = str.substr(first, (last-first+1));
		return val;
	}

	static bool isNumber(const std::string& s)
	{
		std::string::const_iterator it = s.begin();
		while (it != s.end() && std::isdigit(*it)) ++it;
		return !s.empty() && it == s.end();
	}

    static bool startsWith(std::string& value, char character)
    {
        if(value.size() == 0)
            return false;

        return value[0] == character;
    }

    static bool endsWith(std::string& value, char character)
    {
        if(value.size() == 0)
            return false;

        return value[value.size() - 1] == character;
    }
};
