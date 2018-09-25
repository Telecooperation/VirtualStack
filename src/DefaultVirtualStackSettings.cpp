#include "DefaultVirtualStackSettings.h"

std::unique_ptr<VirtualStackSettings> DefaultVirtualStackSettings::Default(StackEnum defaultStack, bool isLocalhost, bool isRouter)
{
    auto tmpStream = DefaultFactory(defaultStack, isLocalhost, isRouter).Build();

    SettingsProvider tmpProvider;
    tmpProvider.ReadSettings(*tmpStream);
    return std::make_unique<VirtualStackSettings>(std::move(tmpProvider));
}

VirtualStackSettings DefaultVirtualStackSettings::Default2(StackEnum defaultStack, bool isLocalhost, bool isRouter)
{
    auto tmpStream = DefaultFactory(defaultStack, isLocalhost, isRouter).Build();

    SettingsProvider tmpProvider;
    tmpProvider.ReadSettings(*tmpStream);
    return VirtualStackSettings{std::move(tmpProvider)};
}

std::unique_ptr<VirtualStackSettings> DefaultVirtualStackSettings::Default(const SettingsStreamFactory &factory)
{
    auto tmpStream = factory.Build();

    SettingsProvider tmpProvider;
    tmpProvider.ReadSettings(*tmpStream);
    return std::make_unique<VirtualStackSettings>(std::move(tmpProvider));
}

SettingsStreamFactory DefaultVirtualStackSettings::DefaultFactory(StackEnum defaultStack, bool isLocalhost, bool isRouter)
{
    auto tmpStreamFactory = SettingsStreamFactory::New();
    tmpStreamFactory
            .AddString("NorthboundDeviceName", "tunDevice")
            .AddString("SouthboundInterfaces", "lo")
            .AddSizeTVector("SouthboundInterfacesMTU", {1500})
            .AddStringVector("SouthboundInterfaceIPv4Address", {"127.0.0.1"})
            .AddUInt("DefaultInterface", 0)

            .AddBool("IsLocalhost", isLocalhost)
            .AddBool("IsRouter", isRouter)
            .AddString("RouterOutStack", StackEnumHelper::toString(StackEnum::Invalid))
            .AddStringVector("RoutingTable", {})

            .AddUInt("ManagementPort", 3000)
            .AddString("ManagementBindAddress", "0.0.0.0")
            .AddString("ManagementInterface", "")

            .AddStringVector("Stacks", {StackEnumHelper::toString(StackEnum::UDPIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPLITEIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPPlusIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPPlusNoFecIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPPlusNoFlowFecIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPPlusNoFlowIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPPlusOnlyFecIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPPlusOnlyFlowFecIPv4),
                                        StackEnumHelper::toString(StackEnum::UDPPlusOnlyFlowIPv4),
                                        StackEnumHelper::toString(StackEnum::SoftwareLoop),
                                        StackEnumHelper::toString(StackEnum::SCTPIPv4),
                                        StackEnumHelper::toString(StackEnum::DCCPIPv4),
                                        StackEnumHelper::toString(StackEnum::TCPIPv4)})
            .AddString("DefaultStack", StackEnumHelper::toString(defaultStack))

            .AddString("TunDeviceName", "tun0")
            .AddSizeT("StackEngineMaxStacksCount", 10)
            .AddSizeT("WaitForStackCreationTimeoutMilliseconds", 20000)

            .AddSizeT("SizeOfStackCreationBuffer", 1024)
            .AddSizeT("SizeOfManagementSessionsBuffer", 128)
            .AddSizeT("SizeOfManagementConfigurationsBuffer", 128)
            .AddSizeT("SizeOfRemoteControlCommandsBuffer", 128)
            .AddSizeT("SizeOfInspectionEventsBuffer", 128)

            .AddSizeT("SizeOfNorthboundPoolBuffer", 256)
            .AddSizeT("SizeOfFromNorthboundBuffer", 8)
            .AddSizeT("SizeOfStackEngineSequenceNumberBuffer", 256)
            .AddSizeT("SizeOfStackBuffer", 8)

            .AddSizeT("SizeOfUdpSequenceNumberBuffer", 256)
            .AddSizeT("SizeOfUdpPlusFecRestoreBuffer", 256)
            .AddSizeT("SizeOfUdpPlusResendingBuffer", 512) //sendeseite
            .AddSizeT("SizeOfUdpPlusInitialFlowWindowSize", 128)
            .AddSizeT("SizeOfUdpPlusFecGroupSize", 8)
            .AddSizeT("SizeOfUdpPlusControlFrameTimeThreshold", 100)
            .AddSizeT("SizeOfUdpPlusControlFrameRecvThreshold", 128)
            .AddSizeT("SizeOfUdpPlusControlFrameFlowWindowReserve", 2)

            .AddSizeT("SizeOfKernelBuffer", 256)
            .AddSizeT("SizeOfToNorthboundBuffer", 1024)

            .AddSizeT("WeightedRoundRobinWeight", 4)

            .AddString("NetworkMessageControllerUtilizationPlan", UtilizationPlanWrapper::toString(UtilizationPlan::High))
            .AddString("SouthboundStackUtilizationPlan", UtilizationPlanWrapper::toString(UtilizationPlan::High))

            .AddString("VirtualStackUtilizationPlan", UtilizationPlanWrapper::toString(UtilizationPlan::High))
            .AddString("GenericStackUtilizationPlan", UtilizationPlanWrapper::toString(UtilizationPlan::High))
            .AddString("UdpPlusUtilizationPlan", UtilizationPlanWrapper::toString(UtilizationPlan::High))
            .AddString("StackEngineReceiveControllerUtilizationPlan", UtilizationPlanWrapper::toString(UtilizationPlan::High))

            .AddSizeT("UtilizationPlanLowThreshold", 1ul << 60)
            .AddSizeT("UtilizationPlanMidThreshold", 1ul << 60)
            .AddSizeT("UtilizationPlanHighThreshold", 1ul << 60)
            .AddSizeT("DebouncerSleepInNanoseconds", 10)
            .AddSizeT("DefaultSchedulerIndex", 0);

    return tmpStreamFactory;
}
