
#pragma once

#include "ClassMacros.h"
#include "make_unique.h"
#include "VectorExtensions.h"
#include <fstream>
#include <map>
#include <sstream>

class SettingsStreamFactory
{
public:
	static SettingsStreamFactory New(){
		return SettingsStreamFactory();
	}
	
	std::unique_ptr<std::stringstream> Build() const{
		auto stream = std::make_unique<std::stringstream>();
		for (const auto& el : _keyMap) {
			*stream << el.first << "=" << el.second << "\n";
		}
		return stream;
	}
	
	SettingsStreamFactory& SaveToFile(const std::string& filename){
		auto stream = Build();
        std::ofstream tmpFile(filename);
		tmpFile << stream->rdbuf()->str() << std::flush;
		tmpFile.close();
		
		return *this;
	}
	
	SettingsStreamFactory& AddString(const std::string& key, const std::string& value){
		_keyMap[key] = "\"" + value + "\"";
		return *this;
	}
	
	SettingsStreamFactory& AddInt(const std::string& key, int value){
		_keyMap[key] = std::to_string(value);
		return *this;
	}
	
	SettingsStreamFactory& AddUInt(const std::string& key, unsigned int value){
		_keyMap[key] = std::to_string(value) + "ul";
		return *this;
	}
	
	SettingsStreamFactory& AddBool(const std::string& key, bool value){
		_keyMap[key] = (value ? "1" : "0");
		return *this;
	}
	
	SettingsStreamFactory& AddSizeT(const std::string& key, size_t value){
		_keyMap[key] = std::to_string(value) + "ul";
		return *this;
	}
	
	SettingsStreamFactory& AddFloat(const std::string& key, float value){
		_keyMap[key] = std::to_string(value);
		return *this;
	}
	
	SettingsStreamFactory& AddStringVector(const std::string& key, const std::vector<std::string>& value){
		_keyMap[key] = "\"" + VectorExtensions::toStringForSettings(value) + "\"";
		return *this;
	}
	
	SettingsStreamFactory& AddIntVector(const std::string& key, const std::vector<int>& value){
		_keyMap[key] = "\"" + VectorExtensions::toStringForSettings(value, &std::to_string) + "\"";
		return *this;
	}
	
	SettingsStreamFactory& AddSizeTVector(const std::string& key, const std::vector<size_t>& value){
		auto tmpConvertFn = static_cast<std::string(*)(size_t)>([](const size_t item) { return std::to_string(item) + "ul"; });

		_keyMap[key] = "\"" + VectorExtensions::toStringForSettings(value, tmpConvertFn) + "\"";
		return *this;
	}
	
	ALLOW_MOVE_SEMANTICS_ONLY(SettingsStreamFactory);
private:
	std::map<std::string, std::string> _keyMap;
public:
	SettingsStreamFactory() : _keyMap() {}
};
