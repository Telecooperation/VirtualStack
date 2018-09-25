#pragma once

#include "StackEnum.h"


///Ermöglich das automatische konvertieren von StackEnum in einer string repräsentationStack
class StackEnumHelper
{
public:
    StackEnumHelper(StackEnum stack);

    StackEnumHelper(const std::string &name);

    operator const std::string &();

    operator const StackEnumInfo &();

    StackEnum &operator=(const StackEnum &item);

    operator const StackEnum &() const;

    operator StackEnum &();

    bool operator==(const StackEnum &rhs) const;

    bool operator!=(const StackEnum &rhs) const;

    static StackEnum convert(const std::string &name);

    static const std::string &toString(const StackEnum& value);

    static const std::string &toString(const StackEnumInfo& value);

    static const StackEnumInfo& getInfo(const StackEnum value);

    StackEnum value = StackEnum::Invalid;
};


