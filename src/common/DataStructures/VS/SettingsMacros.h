
#pragma once

#include "SettingsEntry.h"
#include "SettingsEntryFactory.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#define STRING_KEY(keyName) SettingsEntry<std::string> keyName = SettingsEntryFactory<std::string>::get(#keyName, _provider, _settingsReadFailed)
#define STRING_KEY_DEFAULT(keyName, defaultValue) SettingsEntry<std::string> keyName = SettingsEntryFactory<std::string>::get(#keyName, _provider, _settingsReadFailed, defaultValue)
#define STRING_VECTOR_KEY(keyName) SettingsEntry<std::vector<std::string>> keyName = SettingsEntryFactory<std::vector<std::string>>::get(#keyName, _provider, _settingsReadFailed)

#define INT_KEY(keyName) SettingsEntry<int> keyName = SettingsEntryFactory<int>::get(#keyName, _provider, _settingsReadFailed)
#define INT_VECTOR_KEY(keyName) SettingsEntry<std::vector<int>> keyName = SettingsEntryFactory<std::vector<int>>::get(#keyName, _provider, _settingsReadFailed)

#define SIZE_T_KEY(keyName) SettingsEntry<size_t> keyName = SettingsEntryFactory<size_t>::get(#keyName, _provider, _settingsReadFailed)
#define SIZE_T_VECTOR_KEY(keyName) SettingsEntry<std::vector<size_t>> keyName = SettingsEntryFactory<std::vector<size_t>>::get(#keyName, _provider, _settingsReadFailed)

#define FLOAT_KEY(keyName) SettingsEntry<float> keyName = SettingsEntryFactory<float>::get(#keyName, _provider, _settingsReadFailed)

#define GENERIC_KEY(keyName, type, parserType) SettingsEntry<type> keyName = SettingsEntryFactory<type>::byConvert<parserType>(#keyName, _provider, _settingsReadFailed)
#define GENERIC_KEY_EXPLICIT(keyName, type, parserType, convertFn) SettingsEntry<type> keyName = SettingsEntryFactory<type>::byConvert<parserType>(#keyName, _provider, _settingsReadFailed, convertFn)
#define GENERIC_KEY_EXPLICIT_DEFAULT(keyName, type, parserType, convertFn, defaultValue) SettingsEntry<type> keyName = SettingsEntryFactory<type>::byConvert<parserType>(#keyName, _provider, _settingsReadFailed, convertFn, defaultValue)
#define GENERIC_VECTOR_KEY(keyName, parserType, type) SettingsEntry<std::vector<type>> keyName =\
								 SettingsEntryFactory<type>::byVectorConvert<parserType>(#keyName, _provider, _settingsReadFailed)
#define GENERIC_VECTOR_KEY_EXPLICIT(keyName, type, parserType, convertFn) SettingsEntry<std::vector<type>> keyName =\
								 SettingsEntryFactory<type>::byVectorConvert<parserType>(#keyName, _provider, _settingsReadFailed, convertFn)

#define UINT16_T_KEY(keyName) GENERIC_KEY(keyName, uint16_t, size_t)
#define BOOL_KEY(keyName) GENERIC_KEY_EXPLICIT(keyName, bool, int, [](const int& value) { return value != 0; })
#define BOOL_KEY_DEFAULT(keyName, defaultValue) GENERIC_KEY_EXPLICIT_DEFAULT(keyName, bool, int, [](const int& value) { return value != 0; }, defaultValue)