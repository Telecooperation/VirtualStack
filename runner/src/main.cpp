#include "CmdParser.h"
#include "RawServerClient.h"
#include "VSServerClient.h"
#include <DefaultVirtualStackSettings.h>
#include <VirtualStackLoader.h>
#include <common/Helper/Logger.h>
#include <common/Helper/StopWatch.h>
#include <cstring>
#include <iostream>
#include <stacks/StackEnum.h>
#include <string>
#include <tuple>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>
#include <virtualStack/device/northboundDevices/LoopbackNorthboundDevice.h>


std::vector<std::unique_ptr<LatencyResult>> runInvs(const CmdParser::RunnerConfig& config)
{
    auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(config.stack, true);
    settingsStream.AddStringVector("SouthboundInterfaceIPv4Address", {config.sendIp});
    settingsStream.AddString("SouthboundInterfaces", "backboneIn");
    settingsStream.AddSizeTVector("SouthboundInterfacesMTU", {1500});
    settingsStream.AddString("RoutingTable", config.routingTable);
    settingsStream.AddString("RouterOutStack", StackEnumHelper::toString(config.routerStack));
    settingsStream.AddBool("IsRouter", true);
    settingsStream.AddSizeT("DefaultSchedulerIndex", config.defaultSchedulerIndex);

    auto virtualStackSettings = DefaultVirtualStackSettings::Default(settingsStream);
    if (virtualStackSettings->SettingsReadFailed())
        return {};

    VirtualStackLoader<LoopbackNorthboundDevice> vsLoader;
    if (!vsLoader.Initialize(std::move(virtualStackSettings)))
        return {};

    std::cout << "Press Enter to exit" << std::flush;
    std::string l;
    std::getline(std::cin, l);

    return {};
}

std::vector<std::unique_ptr<LatencyResult>> runRaw(const CmdParser::RunnerConfig& config)
{
    if (config.isSender)
    {
        RawServerClient::sender(config.sendIp, config.recvIp, config.runtime,
                                config.bandwidth, config.packetSize, config.transportProtocol);
        return {};
    }
    std::vector<std::unique_ptr<LatencyResult>> list;
    list.emplace_back(RawServerClient::receiver(config.sendIp, config.recvIp, config.runtime, config.transportProtocol));
    return list;
}

std::vector<std::unique_ptr<LatencyResult>> runVs(const CmdParser::RunnerConfig& config)
{
    auto settingsStream = DefaultVirtualStackSettings::DefaultFactory(config.stack, true);
    auto runIp = config.isSender ? config.sendIp : config.recvIp;
    settingsStream.AddString("SouthboundInterfaceIPv4Address", runIp);
    settingsStream.AddString("RoutingTable", config.routingTable);
    settingsStream.AddBool("IsRouter", false);
    settingsStream.AddSizeT("DefaultSchedulerIndex", config.defaultSchedulerIndex);

    auto virtualStackSettings = DefaultVirtualStackSettings::Default(settingsStream);
    if (virtualStackSettings->SettingsReadFailed())
        return {};

    VirtualStackLoader<DummyNorthboundDevice> vsLoader;
    if (!vsLoader.Initialize(std::move(virtualStackSettings)))
        return {};

    auto &dummyDevice = *vsLoader.northboundDevice;

    if (config.isSender)
    {
        VSServerClient::sender(config.recvIp, config.runtime, config.bandwidth, config.packetSize,
                               *vsLoader.virtualStack, dummyDevice, config.additionalStack, config.connectOverIp);
        return {};
    }
    return VSServerClient::receiver(config.runtime, *vsLoader.virtualStack, dummyDevice);
}

int main(int argc, char** argv)
{
    auto config = CmdParser::parseCmd(argc, argv);

    if (config.recvIp == config.sendIp)
    {
        std::cout << "ServerIP and ClientIP have to be different" << std::endl;
        exit(-1);
    }

    config.packetSize = std::min(config.packetSize, 1400ul); //max VS packetSize

    Logger::setMinLogLevel(Logger::DEBUG);

    std::vector<std::unique_ptr<LatencyResult>> latencyResultList;

    if (config.runType == CmdParser::RunType::Vs)
        latencyResultList = runVs(config);
    else if (config.runType == CmdParser::RunType::Raw)
        latencyResultList = runRaw(config);
    else if (config.runType == CmdParser::RunType::Invs)
        latencyResultList = runInvs(config);
    else
        exit(-2);

    if(latencyResultList.empty())
        return 0;

    for (size_t j = 0; j < latencyResultList.size(); ++j)
    {
        auto& latencyResult = latencyResultList[j];
        std::stringstream fileName;
        fileName << latencyResult->inName;

        if (!config.nameAppendixList.empty())
        {
            for (size_t i = 0; i < config.nameAppendixList.size(); ++i)
                fileName << "_" << config.nameAppendixList[i];
        }

        const std::string metadataFilename = fileName.str() + ".vsmeta";
        const std::string bargraphFilename = fileName.str() + ".vsbar";
        const std::string latencyFilename = fileName.str() + ".vsdelay";

        std::cout << "##### NS3-Runner #####" << std::endl;
        std::cout << metadataFilename << ":" << bargraphFilename << ":" << latencyFilename << std::endl;
        latencyResult->printValues(std::cout, "\t");
        latencyResult->printAsMicroSeconds(std::cout);

        std::ofstream metadataFile{metadataFilename};
        latencyResult->printValues(metadataFile, "\t");
        latencyResult->printAsMicroSeconds(metadataFile);
        metadataFile.close();

        std::ofstream bargraphFile{bargraphFilename};
        latencyResult->dumpBargraphMetadata(bargraphFile, 1000);
        bargraphFile.close();

//        std::ofstream delayFile{latencyFilename};
//        latencyResult->dumpLatencies(delayFile);
//        delayFile.close();
    }
    return 0;
}