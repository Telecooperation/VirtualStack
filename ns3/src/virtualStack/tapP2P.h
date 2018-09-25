#pragma once

#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/dce-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/tap-bridge-module.h>
#include <future>
#include "TapNetworkConfig.h"

namespace tapP2P {
    using namespace ns3;

    const std::string simName = "tapP2P";
    NS_LOG_COMPONENT_DEFINE (simName);

    void printTapIps(Ipv4InterfaceContainer leftIpv4, Ipv4InterfaceContainer rightIpv4, uint8_t nextFreeIpIndex)
    {
        auto leftAddress = leftIpv4.GetAddress(0);
        auto leftGateway = leftIpv4.GetAddress(1);

        auto rightAddress = rightIpv4.GetAddress(1);
        auto rightGateway = rightIpv4.GetAddress(0);

        std::cout << leftAddress.CombineMask(Ipv4Mask("/24")) << "," << leftAddress << "," << leftGateway << std::endl;
        std::cout << rightAddress.CombineMask(Ipv4Mask("/24")) << "," << rightAddress << "," << rightGateway << std::endl;
        std::cout << static_cast<int>(nextFreeIpIndex) << std::endl;
        std::cout << "Press any key to stop " << std::flush;
    }

    std::string getNextIPRange(uint8_t& ipRange)
    {
        std::stringstream strStream;
        strStream << "10.0." << static_cast<int>(ipRange) << ".0";
        ++ipRange;
        return strStream.str();
    }

    void TapP2P(uint8_t startIPRange, const ComplexNetworkConfig& config) {
        //Set Realtime Mode
        GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
        //Enable Protocol-Checksum as otherwise the LinuxKernel will discard the packets
        GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

        CsmaHelper csma;
        PointToPointHelper p2p;

        //nicht anfassen!
        p2p.SetDeviceAttribute("DataRate", StringValue("100Gbps"));
        csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(0)));

        //kann angefasst werden
        csma.SetChannelAttribute ("DataRate", StringValue(config.tapPathDataRate));
        p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(config.tapPathLatency)));

        InternetStackHelper internet;

        NodeContainer tapNodes;
        tapNodes.Create(2);
        NodeContainer innerNodes;
        innerNodes.Create(2);

        Names::Add(config.inDeviceName, tapNodes.Get(0));
        Names::Add(config.outDeviceName, tapNodes.Get(1));
        Names::Add("innerNodeIn", innerNodes.Get(0));
        Names::Add("innerNodeOut", innerNodes.Get(1));

        NodeContainer csmaIn;
        csmaIn.Add(tapNodes.Get(0));
        csmaIn.Add(innerNodes.Get(0));

        NodeContainer csmaOut;
        csmaOut.Add(innerNodes.Get(1));
        csmaOut.Add(tapNodes.Get(1));

        auto csmaInDev = csma.Install(csmaIn);
        auto innerDev = p2p.Install(innerNodes);
        auto csmaOutDev = csma.Install(csmaOut);

        internet.Install(tapNodes);
        internet.Install(innerNodes);

        const std::string inIpNetwork = getNextIPRange(startIPRange);
        const std::string innerIpNetwork = getNextIPRange(startIPRange);
        const std::string outIpNetwork = getNextIPRange(startIPRange);

        Ipv4AddressHelper ipv4AddressHelper;
        ipv4AddressHelper.SetBase(inIpNetwork.c_str(), "255.255.255.0");
        auto csmaInIp = ipv4AddressHelper.Assign(csmaInDev);

        ipv4AddressHelper.SetBase(innerIpNetwork.c_str(), "255.255.255.0");
        auto innerIp = ipv4AddressHelper.Assign(innerDev);

        ipv4AddressHelper.SetBase(outIpNetwork.c_str(), "255.255.255.0");
        auto cmsaOutIp = ipv4AddressHelper.Assign(csmaOutDev);


        TapBridgeHelper tapBridge;
        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue(config.inDeviceName));
        tapBridge.Install(tapNodes.Get(0), csmaInDev.Get(0));

        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue(config.outDeviceName));
        tapBridge.Install(tapNodes.Get(1), csmaOutDev.Get(1));

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
        LinuxStackHelper::PopulateRoutingTables();

//        csma.EnablePcapAll("sim/csma_" + simName, false);
//        p2p.EnablePcapAll("sim/p2p_" + simName, false);

        //auto printIpCallback = ns3::MakeBoundCallback(printTapIps, csmaIp);
        Simulator::Schedule(ns3::Seconds(1), printTapIps, csmaInIp, cmsaOutIp, startIPRange);

        auto run = std::async(std::launch::async, [&]()
                                      {
                                          Simulator::Run();
                                          Simulator::Destroy();
                                      });

        std::string line;
        std::getline(std::cin, line);
        Simulator::Stop();

        /*
         * #!/bin/bash
           ip netns add right
           ip link set dev right netns right
           ip netns exec right ip link set right up
           ip netns exec right ip addr add 10.0.0.2/24 dev right
         */
    }
}