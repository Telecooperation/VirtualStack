#include "StackEnumHelper.h"

StackEnumHelper::StackEnumHelper(StackEnum stack) : value(stack) {}

StackEnumHelper::StackEnumHelper(const std::string &name) : value(convert(name)) {}

StackEnumHelper::operator const std::string &()
{
    return toString(value);
}

StackEnumHelper::operator const StackEnumInfo &()
{
    return getInfo(value);
}

StackEnum &StackEnumHelper::operator=(const StackEnum &item)
{
    value = item;
    return value;
}

StackEnumHelper::operator const StackEnum &() const
{
    return value;
}

StackEnumHelper::operator StackEnum &()
{
    return value;
}

bool StackEnumHelper::operator==(const StackEnum &rhs) const
{
    return value == rhs;
}

bool StackEnumHelper::operator!=(const StackEnum &rhs) const
{
    return rhs != value;
}

StackEnum StackEnumHelper::convert(const std::string &name)
{
    for (size_t i = 0; i < StackEnumInfos.size(); ++i)
    {
        if (name != StackEnumInfos[i].Name)
            continue;

        return static_cast<StackEnum>(i);
    }
    return StackEnum::Invalid;
}

const StackEnumInfo &StackEnumHelper::getInfo(const StackEnum value)
{
    return StackEnumInfos[static_cast<size_t>(value)];
}

const std::string &StackEnumHelper::toString(const StackEnum& value)
{
    return StackEnumInfos[static_cast<size_t>(value)].Name;
}

const std::string &StackEnumHelper::toString(const StackEnumInfo& value)
{
    return value.Name;
}