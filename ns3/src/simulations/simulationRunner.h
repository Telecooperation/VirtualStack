#pragma once

#include "options/sctpOptions.h"
#include "options/tcpOptions.h"
#include "p2p/discretSmallPacketSim.h"
#include "p2p/throughputSim.h"
#include "simulationSetup.h"
#include "transport/OverCable.h"
#include "transport/OverLTE.h"
#include "transport/OverWifi.h"
#include "options/dccpOptions.h"
#include "options/udpOptions.h"
#include "options/udpLiteOptions.h"
#include "options/rdsOptions.h"
#include "transport/OverLTEReal.h"
#include "transport/OverComplexLTE.h"
#include "transport/OverComplexWifi.h"
#include <common/Helper/Logger.h>
#include <common/Helper/make_unique.h>
#include <functional>
#include <iomanip>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

namespace simulations
{
    class SimulationRunner
    {
    public:
        static void run(const std::string& path)
        {
            initializeLists();

            std::string linkLayers[] = {"cable",
//                                        "complex-wifi",
//                                        "complex-lte",
//                                        "lte",
//                                        "wifi",
//                                        "lte-real",
//                                        "lte",
//                                        "lte-real"
            };

            auto linkLayerCount = sizeof(linkLayers)/sizeof(linkLayers[0]);

            size_t iterationSize = linkLayerCount * lossList.size() * dataRateTimeList.size() * delayList.size() * simulationList.size() * payloadList.size();
            size_t iterationCounter = 0;

            printProgress(0,iterationCounter, iterationSize);


            for (const auto &linkLayerName : linkLayers)
            {
                for (const auto &loss : lossList)
                {
                    for (const auto &dataRateTime : dataRateTimeList)
                    {
                        for (const auto &delay : delayList)
                        {
                            struct stat st{};
                            for (auto &el: simulationList)
                            {
                                for (auto payloadSize: payloadList)
                                {
                                    std::cout << "\rRunning " << linkLayerName << " " << loss << "% "
                                           << dataRateTime.first << " " << dataRateTime.second << "s "
                                           << delay << " " << el.first << " " << payloadSize << "B" << std::endl;
                                    printProgress(
                                            static_cast<size_t>(static_cast<double>(iterationCounter) / iterationSize *
                                                                100.0), iterationCounter, iterationSize);

                                    //if(linkLayerName == "wifi" && loss == "0" && dataRateTime.first == "10Mbps" && delay == "0ms" && el.first == "udpLite-Throughput-Standard")
                                    //    continue;
                                    auto simSuitePath = pathJoin(path, linkLayerName);

                                    if (stat(simSuitePath.c_str(), &st) == -1)
                                    {
                                        ::mkdir(simSuitePath.c_str(), 0777);
                                    }


                                    /*simSuitePath = pathJoin(simSuitePath, el.first);

                                    if (stat(simSuitePath.c_str(), &st) == -1)
                                    {
                                        ::mkdir(simSuitePath.c_str(), 0777);
                                    }*/

                                    auto simName = el.first + "-" + dataRateTime.first + "-" + delay + "-" + loss + "-" + std::to_string(payloadSize);
                                    //auto fullPath = pathJoin(simSuitePath, simName);

//                            if (stat(fullPath.c_str(), &st) == 0) {
//                                if(::rmdir(fullPath.c_str()) < 0)
//                                    Logger::Log(Logger::WARNING, "Delete folder ", fullPath," failed: ", strerror(errno));
//                            }

                                    //::mkdir(fullPath.c_str(), 0777);

                                    auto &pair = el.second;

                                    SimulationSetup sim{getLinkLayer(linkLayerName)};
                                    sim.setSimRuntime(dataRateTime.second);
                                    pair.first->baseRun(sim);

                                    auto realLoss = std::stod(loss); //udpLite drops packet only if header is damaged -> lower loss rates
                                    if (el.first == "udpLite-Throughput-Standard" ||
                                        el.first == "udpLite-DiscretSmallPacket-Standard")
                                        realLoss = realLoss * (1.0 - 0.008);

                                    sim.preRun(realLoss, dataRateTime.first, dataRateTime.first, delay, payloadSize);

                                    pair.first->run(sim);
                                    pair.second(sim);
                                    auto fileName = "ns3_" + el.first + "_" + std::to_string(dataRateTime.second) + "_" + dataRateTime.first + "_" + std::to_string(payloadSize);
                                    sim.postRun(simSuitePath, fileName);
                                    //mkdir auf den namen mit CWD path
                                    //funktion mitgeben: path+name
                                    ++iterationCounter;

                                }
                            }
                        }
                    }
                }
            }
            printProgress(static_cast<size_t>(static_cast<double>(iterationCounter) / iterationSize * 100.0), iterationCounter, iterationSize);
        }

        static std::unique_ptr<BaseOverTransport> getLinkLayer(const std::string &name)
        {
            if(name == "cable")
                return std::make_unique<OverCable>();
            if(name == "wifi")
                return std::make_unique<OverWifi>();
            if(name == "lte")
                return std::make_unique<OverLTE>();
            if(name == "lte-real")
                return std::make_unique<OverLTEReal>();
            if(name == "complex-lte")
                return std::make_unique<OverComplexLTE>();
            if(name == "complex-wifi")
                return std::make_unique<OverComplexWifi>();
            std::cout << "Invalid link layer: " << name << std::endl;
            exit(-1);
        }

        static std::string pathJoin(const std::string& parent, const std::string& child)
        {
           if(parent.empty())
               return "./" + child + "/";
            return parent + "/" + child + "/";
        }

        static void printProgress(size_t progress, size_t done, size_t todo)
        {
            std::cout << "\rSimulation-Progress: " << std::setw(3) << progress << "% " << done << "/" << todo << std::flush;
        }

        static std::vector<std::pair<std::string, uint64_t>> dataRateTimeList;
        static std::vector<std::string> delayList;
        static std::vector<std::string> lossList;
        static std::vector<uint32_t> payloadList;
        static std::map<std::string,
                std::pair<std::unique_ptr<options::SimOptions>, std::function<void(SimulationSetup&)>>> simulationList;

        static void initializeLists()
        {
            dataRateTimeList = {{"1Mbps", 60},{"3Mbps", 60},{"6Mbps", 60},{"10Mbps", 60},{"25Mbps", 60},{"50Mbps", 60},{"100Mbps", 60}};
            delayList = {"0ms"/*,"1ms", "10ms", "100ms", "1000ms", "2000ms"*/};
            lossList = {"0"};
            payloadList = {100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400};

//            simulationList["tcp-DiscretSmallPacket-Cubic"] = std::make_pair(std::make_unique<options::TcpOptions>("cubic"), &p2p::discretSmallPacketSim::run);
            //simulationList["tcp-DiscretSmallPacket-Reno"] = std::make_pair(std::make_unique<options::TcpOptions>("reno"), &p2p::discretSmallPacketSim::run);
//            simulationList["tcp-DiscretSmallPacket-Vegas"] = std::make_pair(std::make_unique<options::TcpOptions>("vegas"), &p2p::discretSmallPacketSim::run);

            //simulationList["tcp-Throughput-Cubic-noFACK"] = std::make_pair(std::make_unique<options::TcpOptions>("cubic", false), &p2p::throughputSim::run);
            simulationList["tcp-Throughput-Cubic"] = std::make_pair(std::make_unique<options::TcpOptions>("cubic"), &p2p::throughputSim::run);
            //simulationList["tcp-Throughput-Reno"] = std::make_pair(std::make_unique<options::TcpOptions>("reno"), &p2p::throughputSim::run);
//            simulationList["tcp-Throughput-Vegas"] = std::make_pair(std::make_unique<options::TcpOptions>("vegas"), &p2p::throughputSim::run);
//
//
            simulationList["sctp-Throughput-Optimized"] = std::make_pair(std::make_unique<options::SctpOptions>(), &p2p::throughputSim::run);
            //simulationList["sctp-DiscretSmallPacket-Optimized"] = std::make_pair(std::make_unique<options::SctpOptions>(), &p2p::discretSmallPacketSim::run);
//
            simulationList["dccp-Throughput-Standard"] = std::make_pair(std::make_unique<options::DccpOptions>(), &p2p::throughputSim::run);
            //simulationList["dccp-DiscretSmallPacket-Standard"] = std::make_pair(std::make_unique<options::DccpOptions>(), &p2p::discretSmallPacketSim::run);
//
           // simulationList["udp-DiscretSmallPacket-Standard"] = std::make_pair(std::make_unique<options::UdpOptions>(), &p2p::discretSmallPacketSim::run);
            simulationList["udp-Throughput-Standard"] = std::make_pair(std::make_unique<options::UdpOptions>(), &p2p::throughputSim::run);
//            simulationList["udpLite-DiscretSmallPacket-Standard"] = std::make_pair(std::make_unique<options::UdpLiteOptions>(), &p2p::discretSmallPacketSim::run);
//            simulationList["udpLite-Throughput-Standard"] = std::make_pair(std::make_unique<options::UdpLiteOptions>(), &p2p::throughputSim::run);
            //simulationList["rds-Throughput-Standard"] = std::make_pair(std::make_unique<options::RdsOptions>(), &p2p::throughputSim::run);

        }
    };

    std::vector<std::pair<std::string, uint64_t>> SimulationRunner::dataRateTimeList{};
    std::vector<std::string> SimulationRunner::delayList{};
    std::vector<std::string> SimulationRunner::lossList{};
    std::vector<uint32_t> SimulationRunner::payloadList{};
    std::map<std::string, std::pair<std::unique_ptr<options::SimOptions>, std::function<void(SimulationSetup&)>>> SimulationRunner::simulationList{};
}
