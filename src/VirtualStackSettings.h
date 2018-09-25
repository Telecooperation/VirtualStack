#pragma once

#include "common/DataStructures/VS/ISettingsBase.h"
#include "common/DataStructures/VS/SettingsMacros.h"
#include "common/Helper/make_unique.h"
#include "stacks/StackEnum.h"
#include <functional>
#include <string>
#include <functional>
#include "stacks/StackEnumHelper.h"
#include "common/Helper/UtilizationPlan.h"


class VirtualStackSettings final : public ISettingsBase
{
public:
	explicit VirtualStackSettings(SettingsProvider&& provider);
	
	~VirtualStackSettings() override;

	STRING_KEY(NorthboundDeviceName);
	STRING_VECTOR_KEY(SouthboundInterfaces);
	SIZE_T_VECTOR_KEY(SouthboundInterfacesMTU);
	STRING_VECTOR_KEY(SouthboundInterfaceIPv4Address);
    UINT16_T_KEY(DefaultInterface);

    BOOL_KEY(IsLocalhost);
	BOOL_KEY(IsRouter);
    GENERIC_KEY_EXPLICIT(RouterOutStack, StackEnum, std::string, &StackEnumHelper::convert);
	STRING_VECTOR_KEY(RoutingTable);

	UINT16_T_KEY(ManagementPort);
	STRING_KEY(ManagementBindAddress);
	STRING_KEY(ManagementInterface);

    GENERIC_VECTOR_KEY_EXPLICIT(Stacks, StackEnum, std::string, &StackEnumHelper::convert);
	GENERIC_KEY_EXPLICIT(DefaultStack, StackEnum, std::string, &StackEnumHelper::convert);

	STRING_KEY(TunDeviceName);
	SIZE_T_KEY(StackEngineMaxStacksCount);
	SIZE_T_KEY(WaitForStackCreationTimeoutMilliseconds);

    SIZE_T_KEY(SizeOfStackCreationBuffer);
    SIZE_T_KEY(SizeOfManagementSessionsBuffer);
    SIZE_T_KEY(SizeOfManagementConfigurationsBuffer);
    SIZE_T_KEY(SizeOfRemoteControlCommandsBuffer);
    SIZE_T_KEY(SizeOfInspectionEventsBuffer);

    SIZE_T_KEY(SizeOfNorthboundPoolBuffer);
    SIZE_T_KEY(SizeOfFromNorthboundBuffer);
    SIZE_T_KEY(SizeOfStackEngineSequenceNumberBuffer);
    SIZE_T_KEY(SizeOfStackBuffer);

    SIZE_T_KEY(SizeOfUdpSequenceNumberBuffer);
    SIZE_T_KEY(SizeOfUdpPlusFecRestoreBuffer);
    SIZE_T_KEY(SizeOfUdpPlusResendingBuffer);
    SIZE_T_KEY(SizeOfUdpPlusInitialFlowWindowSize);
    SIZE_T_KEY(SizeOfUdpPlusFecGroupSize);
    SIZE_T_KEY(SizeOfUdpPlusControlFrameTimeThreshold);
    SIZE_T_KEY(SizeOfUdpPlusControlFrameRecvThreshold);
    SIZE_T_KEY(SizeOfUdpPlusControlFrameFlowWindowReserve);

    SIZE_T_KEY(SizeOfKernelBuffer);
    SIZE_T_KEY(SizeOfToNorthboundBuffer);

    SIZE_T_KEY(WeightedRoundRobinWeight);

	GENERIC_KEY_EXPLICIT(VirtualStackUtilizationPlan, UtilizationPlan, std::string, &UtilizationPlanWrapper::convert);
	GENERIC_KEY_EXPLICIT(NetworkMessageControllerUtilizationPlan, UtilizationPlan, std::string, &UtilizationPlanWrapper::convert);
	GENERIC_KEY_EXPLICIT(SouthboundStackUtilizationPlan, UtilizationPlan, std::string, &UtilizationPlanWrapper::convert);
	GENERIC_KEY_EXPLICIT(GenericStackUtilizationPlan, UtilizationPlan, std::string, &UtilizationPlanWrapper::convert);
	GENERIC_KEY_EXPLICIT(UdpPlusUtilizationPlan, UtilizationPlan, std::string, &UtilizationPlanWrapper::convert);
	GENERIC_KEY_EXPLICIT(StackEngineReceiveControllerUtilizationPlan, UtilizationPlan, std::string, &UtilizationPlanWrapper::convert);

	SIZE_T_KEY(UtilizationPlanLowThreshold);
	SIZE_T_KEY(UtilizationPlanMidThreshold);
	SIZE_T_KEY(UtilizationPlanHighThreshold);
	SIZE_T_KEY(DebouncerSleepInNanoseconds);

	SIZE_T_KEY(DefaultSchedulerIndex);

	const std::string& getDefaultInterface() const;
	const std::string& getDefaultIPv4() const;
	
	template <typename T, typename U>
	bool isSizeDifferent(SettingsEntry<std::vector<T>> first, SettingsEntry<std::vector<U>> second);
	
	template<typename T>
	bool notContainsValue(SettingsEntry<std::vector<T>> list, SettingsEntry<T> value);
	
	template<typename T>
	bool notContainsValue(SettingsEntry<std::vector<T>> list, SettingsEntry<T> value, const std::string& (*convertFn)(const T&));

    ALLOW_MOVE_SEMANTICS_ONLY(VirtualStackSettings);
};


