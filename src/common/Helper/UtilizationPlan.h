#pragma once

#include <array>
#include <cstddef>
#include <string>

//#####################
//ACHTUNG: Änderungen der Reihenfolge oder hinzufügen müssenb mit der darunter liegenden "UtilizationPlanString" UND "UtilizationPlanSize" synchronisiert werden
//ACHTUNG: Änderungen der Reihenfolge oder hinzufügen müssenb mit der darunter liegenden "UtilizationPlanString" UND "UtilizationPlanSize" synchronisiert werden
//ACHTUNG: Änderungen der Reihenfolge oder hinzufügen müssenb mit der darunter liegenden "UtilizationPlanString" UND "UtilizationPlanSize" synchronisiert werden
//#####################
enum class UtilizationPlan : size_t
{
    Low,
    Mid,
    High,
    Invalid
};

constexpr size_t UtilizationPlanSize = 4;

const std::array<std::string, UtilizationPlanSize> UtilizationPlanString = {"low",
                                                           "mid",
                                                           "high",
                                                           "invalid"};

static_assert((static_cast<size_t>(UtilizationPlan::Invalid) + 1u) == UtilizationPlanSize,
              "UtilizationPlan and UtilizationPlanString have to be of same size");

///Ermöglich das automatische konvertieren von UtilizationPlan in einer string repräsentationStack
struct UtilizationPlanWrapper
{
    UtilizationPlan value = UtilizationPlan::Invalid;

    UtilizationPlanWrapper(UtilizationPlan stack) : value(stack) {}

    UtilizationPlanWrapper(const std::string &name) : value(convert(name)) {}

    operator const std::string &()
    {
        return UtilizationPlanString[static_cast<size_t>(value)];
    }

    UtilizationPlan &operator=(const UtilizationPlan &item)
    {
        value = item;
        return value;
    }

    operator const UtilizationPlan &() const
    {
        return value;
    }

    operator UtilizationPlan &()
    {
        return value;
    }

    bool operator==(const UtilizationPlan &rhs) const
    {
        return value == rhs;
    }

    bool operator!=(const UtilizationPlan &rhs) const
    {
        return rhs != value;
    }

    static UtilizationPlan convert(const std::string &name)
    {
        for (size_t i = 0; i < UtilizationPlanString.size(); ++i)
        {
            if (name != UtilizationPlanString[i])
                continue;

            return static_cast<UtilizationPlan>(i);
        }
        return UtilizationPlan::Invalid;
    }

    static const std::string &toString(const UtilizationPlan &value)
    {
        return UtilizationPlanString[static_cast<size_t>(value)];
    }
};


