#pragma once

#include "SettingsEntry.h"
#include <functional>

template<typename T>
class SettingsEntryFactory
{
public:
    template<typename TParserType>
    static SettingsEntry<std::vector<T>> byVectorConvert(std::string keyName, SettingsProvider &provider, bool &readFailed)
    {
        StringExtensions::toLower(keyName);
        bool tmpReadFailed = false;
        auto tmpValues = provider.get<std::vector<TParserType>>(keyName, tmpReadFailed);
        readFailed &= tmpReadFailed;

        if (tmpReadFailed)
            return SettingsEntry<std::vector<T>>(keyName, std::vector<T>());

        std::vector<T> tmpConvertedValues;
        std::transform(tmpValues.begin(), tmpValues.end(), std::back_inserter(tmpConvertedValues),
                       [](const TParserType &item) { return T(item); });
        return SettingsEntry<std::vector<T>>(keyName, tmpConvertedValues);
    }

    template<typename TParserType>
    static SettingsEntry<std::vector<T>> byVectorConvert(std::string keyName, SettingsProvider &provider, bool &readFailed, T (*convertFn)(const TParserType&))
    {
        StringExtensions::toLower(keyName);
        bool tmpReadFailed = false;
        auto tmpValues = provider.get<std::vector<TParserType>>(keyName, tmpReadFailed);
        readFailed &= tmpReadFailed;

        if (tmpReadFailed)
            return SettingsEntry<std::vector<T>>(keyName, std::vector<T>());

        std::vector<T> tmpConvertedValues;
        std::transform(tmpValues.begin(), tmpValues.end(), std::back_inserter(tmpConvertedValues), convertFn);
        return SettingsEntry<std::vector<T>>(keyName, tmpConvertedValues);
    }


    template <typename TParserType>
    static SettingsEntry<T> byConvert(std::string keyName, SettingsProvider &provider,
                                      bool &readFailed,
                                      std::function<T(const TParserType&)> convertFn)
    {
        StringExtensions::toLower(keyName);
        bool tmpReadFailed = false;
        auto tmpValue = provider.get<TParserType>(keyName, tmpReadFailed);
        readFailed &= tmpReadFailed;

        if (tmpReadFailed)
            return SettingsEntry<T>(keyName, convertFn(TParserType()));

        return SettingsEntry<T>(keyName, convertFn(tmpValue));
    }

    template <typename TParserType>
    static SettingsEntry<T> byConvert(std::string keyName, SettingsProvider &provider, bool &readFailed)
    {
        auto convertFn = [](const TParserType& item) { return static_cast<T>(item); };
        return byConvert<TParserType>(keyName, provider, readFailed, convertFn);
    }

    static SettingsEntry<T> get(std::string keyName, SettingsProvider &provider, bool &readFailed)
    {
        auto convertFn = [](const T& item) { return item; };
        return byConvert<T>(keyName, provider, readFailed, convertFn);
    }

    template <typename TParserType>
    static SettingsEntry<T> byConvert(std::string keyName, SettingsProvider &provider, bool &readFailed,
                                      std::function<T(const TParserType&)> convertFn,
                                      TParserType defaultValue)
    {
        StringExtensions::toLower(keyName);
        bool tmpReadFailed = false;
        auto tmpValue = provider.get<TParserType>(keyName, tmpReadFailed);
        //readFailed &= tmpReadFailed;

        if (tmpReadFailed)
            return SettingsEntry<T>(keyName, convertFn(defaultValue));

        return SettingsEntry<T>(keyName, convertFn(tmpValue));
    }

    template <typename TParserType>
    static SettingsEntry<T> byConvert(std::string keyName, SettingsProvider &provider, bool &readFailed, TParserType defaultValue)
    {
        return byConvert<TParserType>(keyName, provider, readFailed, [](const TParserType& item) { return static_cast<T>(item); }, defaultValue);
    }

    static SettingsEntry<T> get(std::string keyName, SettingsProvider &provider, bool &readFailed, T defaultValue)
    {
        return byConvert<T>(keyName, provider, readFailed, [](const T& item) { return item; }, defaultValue);
    }
};
