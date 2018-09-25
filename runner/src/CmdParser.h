#pragma once

#include <common/Helper/DomainExtensions.h>
#include <iostream>
#include <stacks/StackEnum.h>
#include <stacks/StackEnumHelper.h>
#include <string>
#include <vector>

namespace CmdParser
{
#define CMD_LINE "usage: {(invs routerStackEnum), (vs stackEnum), (raw transportProtocol)} [route routingTable] {(send bwMbps packetSize [additional stackEnum connectOverIp]), (recv)} sendIP recvIP [runtime]"

    enum class RunType
    {
        Raw,
        Vs,
        Invs,
        Invalid
    };

    struct RunnerConfig
    {
        RunType runType = RunType::Raw;
        bool isSender = false;
        TransportProtocolEnum transportProtocol = TransportProtocolEnum::NONE;
        std::string routingTable = "";
        std::string sendIp = "";
        std::string recvIp = "";
        double bandwidth = 0.0;
        size_t packetSize = 0;
        StackEnum stack = StackEnum::TCPIPv4;
        StackEnum routerStack = StackEnum::Invalid;
        size_t runtime = 10;
        StackEnum additionalStack = StackEnum::Invalid;
        std::string connectOverIp = "";
        size_t defaultSchedulerIndex = 0;
        std::vector<std::string> nameAppendixList{};

        static RunType runTypeFromString(const std::string& val)
        {
            if(val == "vs")
                return RunType::Vs;
            if(val == "raw")
                return RunType::Raw;
            if(val == "invs")
                return RunType::Invs;
            return RunType::Invalid;
        }
    };

    RunnerConfig parseCmd(int argc, char **argv)
    {
        if (argc < 4)
        {
            std::cout << CMD_LINE << std::endl;
            exit(1);
        }

        RunnerConfig config{};
        int nextParameterIndex = 1;
        config.runType = RunnerConfig::runTypeFromString(argv[nextParameterIndex]);
        ++nextParameterIndex;

        if(config.runType == RunType::Invalid)
        {
            std::cout << CMD_LINE << std::endl;
            exit(2);
        }

        if(config.runType == RunType::Raw)
        {
            config.transportProtocol = TransportProtocolWrapper::fromString(std::string(argv[nextParameterIndex]));
            if(config.transportProtocol == TransportProtocolEnum::NONE)
            {
                std::cout << CMD_LINE << std::endl;
                exit(2);
            }

            ++nextParameterIndex;
        }
        else if(config.runType == RunType::Vs)
        {
            config.stack = StackEnumHelper::convert(argv[nextParameterIndex]);
            if (config.stack == StackEnum::Invalid)
            {
                std::cout << "Stack is invalid" << std::endl << CMD_LINE << std::endl;
                exit(2);
            }

            ++nextParameterIndex;
        }
        else if(config.runType == RunType::Invs)
        {
            config.routerStack = StackEnumHelper::convert(argv[nextParameterIndex]);
            ++nextParameterIndex;
        }

        if (std::string(argv[nextParameterIndex]) == "route")
        {
            ++nextParameterIndex;
            config.routingTable = argv[nextParameterIndex];
            ++nextParameterIndex;
        }

        if(config.runType != RunType::Invs)
        {
            if ((std::string(argv[nextParameterIndex]) != "send" && std::string(argv[nextParameterIndex]) != "recv"))
            {
                std::cout << CMD_LINE << std::endl;
                exit(3);
            }
            config.isSender = std::string(argv[nextParameterIndex]) == "send";
            ++nextParameterIndex;


            if (config.isSender)
            {
                if (argc < (nextParameterIndex + 2))
                {
                    std::cout << CMD_LINE << std::endl;
                    exit(4);
                }

                config.bandwidth = std::stod(argv[nextParameterIndex]);
                config.packetSize = static_cast<size_t>(std::stoi(argv[nextParameterIndex + 1]));

                nextParameterIndex += 2;

                if(std::string(argv[nextParameterIndex]) == "additional")
                {
                    ++nextParameterIndex;
                    config.additionalStack = StackEnumHelper::convert(argv[nextParameterIndex]);
                    config.connectOverIp = argv[nextParameterIndex + 1];
                    nextParameterIndex += 2;
                }
            }
        }

        config.sendIp = argv[nextParameterIndex];
        ++nextParameterIndex;
        config.recvIp = argv[nextParameterIndex];
        ++nextParameterIndex;

        if(argc >= (nextParameterIndex + 1))
        {
            config.runtime = static_cast<size_t>(std::stoi(argv[nextParameterIndex]));
            ++nextParameterIndex;
        }

        if(argc >= (nextParameterIndex + 1))
        {
            config.defaultSchedulerIndex = static_cast<size_t>(std::stoi(argv[nextParameterIndex]));
            ++nextParameterIndex;
        }


        for (int i = nextParameterIndex; i < argc; ++i)
            config.nameAppendixList.emplace_back(argv[i]);

        return config;
    };
}