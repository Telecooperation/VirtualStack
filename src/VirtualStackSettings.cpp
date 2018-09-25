#include "VirtualStackSettings.h"
#include "common/Helper/Logger.h"
#include "common/Helper/VectorExtensions.h"
#include "stacks/updPlus/addons/UdpFecRestore.h"

VirtualStackSettings::VirtualStackSettings(SettingsProvider&& provider) : ISettingsBase(std::move(provider))
{
	if(VectorExtensions::contains(Stacks.value, StackEnum::Invalid))
	{
		Logger::Log(Logger::ERROR, "Stacks contains elements not to be found in StackFactory::GetStackList: ",
					VectorExtensions::toString(Stacks.value, &StackEnumHelper::toString),
					" has to be a subset of ",
					VectorExtensions::toString(StackEnumInfos, &StackEnumHelper::toString), " without ", StackEnumHelper::toString(StackEnum::Invalid));
		_settingsReadFailed = true;
	}
	
	if(DefaultStack.value == StackEnum::Invalid)
	{
		Logger::Log(Logger::ERROR, "Default-Stack was not valid, has to be one of: ", VectorExtensions::toString(StackEnumInfos, &StackEnumHelper::toString));
		_settingsReadFailed = true;
	}
	
	if(SouthboundInterfaces.value.size() <= DefaultInterface)
	{
		Logger::Log(Logger::ERROR, "DefaultInterface has to be between: ", 0, " and ", std::max(1ul, SouthboundInterfaces.value.size()) - 1, " but was: ", DefaultInterface);
		_settingsReadFailed = true;
	}
	
	_settingsReadFailed |= notContainsValue(Stacks, DefaultStack, &StackEnumHelper::toString);
	
	_settingsReadFailed |= isSizeDifferent(SouthboundInterfaces, SouthboundInterfacesMTU);
	_settingsReadFailed |= isSizeDifferent(SouthboundInterfaces, SouthboundInterfaceIPv4Address);

    if(SizeOfKernelBuffer < SizeOfStackEngineSequenceNumberBuffer)
    {
        Logger::Log(Logger::ERROR, "SizeOfKernelBuffer has to be equal or bigger than SizeOfStackEngineSequenceNumberBuffer");
        _settingsReadFailed = true;
    }

    if(SizeOfUdpPlusFecGroupSize < 1)
    {
        Logger::Log(Logger::ERROR, "FECGroupSize has to be 1 or more");
        _settingsReadFailed = true;
    }

    if(SizeOfUdpPlusResendingBuffer < SizeOfUdpSequenceNumberBuffer)
    {
        Logger::Log(Logger::ERROR, "SizeOfUdpPlusResendingBuffer has to be equal or bigger than SizeOfUdpSequenceNumberBuffer");
        _settingsReadFailed = true;
    }

    if(SizeOfKernelBuffer < SizeOfUdpSequenceNumberBuffer)
    {
        Logger::Log(Logger::ERROR, "SizeOfKernelBuffer has to be equal or bigger than SizeOfUdpSequenceNumberBuffer");
        _settingsReadFailed = true;
    }

    if(SizeOfKernelBuffer < SizeOfUdpPlusInitialFlowWindowSize)
    {
        Logger::Log(Logger::ERROR, "SizeOfUdpPlusInitialFlowWindowSize has to be equal or smaller than SizeOfKernelBuffer");
        _settingsReadFailed = true;
    }

    if(SizeOfStackBuffer < SizeOfFromNorthboundBuffer)
    {
        Logger::Log(Logger::ERROR, "SizeOfStackBuffer has to be equal or bigger than SizeOfFromNorthboundBuffer");
        _settingsReadFailed = true;
    }

    if(SizeOfNorthboundPoolBuffer < SizeOfFromNorthboundBuffer)
    {
        Logger::Log(Logger::ERROR, "SizeOfNorthboundPoolBuffer has to be equal or bigger than SizeOfFromNorthboundBuffer");
        _settingsReadFailed = true;
    }

	if((RoutingTable.value.size() % 2) != 0)
	{
		Logger::Log(Logger::ERROR, "RoutingTable has to be of even size {source, destination}");
		_settingsReadFailed = true;
	}

    if(SizeOfUdpPlusFecGroupSize > (sizeof(UdpFecRestore::FecRestoreInfo::seenFlag) * 8))
    {
        Logger::Log(Logger::ERROR, "SizeOfUdpPlusFecGroupSize has to be smaller or equal than ", (sizeof(UdpFecRestore::FecRestoreInfo::seenFlag) * 8));
        _settingsReadFailed = true;
    }
}

VirtualStackSettings::~VirtualStackSettings() = default;

template<typename T, typename U>
bool VirtualStackSettings::isSizeDifferent(SettingsEntry<std::vector<T>> first, SettingsEntry<std::vector<U>> second)
{
	if(first.value.size() != second.value.size())
	{
		Logger::Log(Logger::ERROR, first.key, " and ", second.key, " must be same size.");
		return true;
	}
	
	return false;
}

template<typename T>
bool VirtualStackSettings::notContainsValue(SettingsEntry<std::vector<T>> list, SettingsEntry<T> value, const std::string& (*convertFn)(const T&))
{
	if(!VectorExtensions::contains(list.value, value.value))
	{
		Logger::Log(Logger::ERROR, value.key, " \"", convertFn(value.value), "\" was not found in ", list.key, " ", VectorExtensions::toString<T>(list.value, convertFn));
		return true;
	}
	return false;
}

template<typename T>
bool VirtualStackSettings::notContainsValue(SettingsEntry<std::vector<T>> list, SettingsEntry<T> value)
{
	if(!VectorExtensions::contains(list.value, value.value))
	{
		Logger::Log(Logger::ERROR, value.key, " \"", value.value, "\" was not found in ", list.key, " ", VectorExtensions::toString<T>(list.value));
		return true;
	}
	return false;
}

const std::string &VirtualStackSettings::getDefaultInterface() const
{
	return SouthboundInterfaces.value[DefaultInterface.value];
}

const std::string &VirtualStackSettings::getDefaultIPv4() const
{
	return SouthboundInterfaceIPv4Address.value[DefaultInterface.value];
}