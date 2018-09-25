#pragma once

#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/dce-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/tap-bridge-module.h>
#include <future>
#include <string>
#include "TapNetworkConfig.h"

namespace tapComplexNetwork {
    using namespace ns3;

    const std::string simName = "tapComplexNetwork";
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

    void BlockLTEConnection(Ptr<ns3::RandomVariableStream> delayTime, double lteModifier, Ptr<NetDevice> csmaNetDevice);
    void UnblockConnection(Ptr<ns3::RandomVariableStream> delayTime, double lteModifier, Ptr<NetDevice> csmaNetDevice);

    void TapComplexNetwork(uint8_t startIPRange, const ComplexNetworkConfig& config) {
        //Set Realtime Mode
        GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
        //Enable Protocol-Checksum as otherwise the LinuxKernel will discard the packets
        GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

        /*       /-----LTE----\
         * client             invs--Cable---server
         *       \----Cable---/
         */


        /*                    /-p2p-LTENODE-csma-\
         * LEFT-csma-INNERLEFT                   INVS-p2p-INNERRIGHT-csma-RIGHT
         *                   \-p2p-DSLNODE-csma-/
         */
        CsmaHelper csmaLeft;
        CsmaHelper csmaLte;
        CsmaHelper csmaDsl;
        CsmaHelper csmaRight;

        PointToPointHelper p2pLte;
        PointToPointHelper p2pDsl;
        PointToPointHelper p2pRight;


        //Parameter:
        // Params: DataRate, Delay
        // LEFT-innerLeft -> 100Gbps, 0
        // innerLeft-p2pLTE -> 100Gbps, 50ms
        // csmaLTE-invs -> 25Mbps, 0
        // innerLeft-p2pDSL -> 100Gbps, 30ms
        // csmaDSL-invs -> 6Mbps, 0
        // p2pInvs-p2pInnerRight -> 100Gbps, 1
        // csmaInnerRight-RIGHT -> 1Gbit, 0


        //nicht anfassen! DefaultWerte für alles was nicht überschrieben wird
        p2pLte.SetDeviceAttribute("DataRate", StringValue("100Gbps"));
        p2pDsl.SetDeviceAttribute("DataRate", StringValue("100Gbps"));
        p2pRight.SetDeviceAttribute("DataRate", StringValue("100Gbps"));

        p2pLte.SetChannelAttribute("Delay", TimeValue(MilliSeconds(config.lteLatency)));
        p2pDsl.SetChannelAttribute("Delay", TimeValue(MilliSeconds(config.dslLatency)));
        p2pRight.SetChannelAttribute("Delay", TimeValue(MilliSeconds(config.tapPathLatency)));

        //nicht anfassen! DefaultWerte für alles was nicht überschrieben wird
        csmaLeft.SetChannelAttribute ("DataRate", StringValue("100Gbps"));
        csmaLte.SetChannelAttribute ("DataRate", StringValue(config.lteDataRate));
        csmaDsl.SetChannelAttribute ("DataRate", StringValue(config.dslDataRate));
        csmaRight.SetChannelAttribute ("DataRate", StringValue(config.tapPathDataRate));

        //nicht anfassen! DefaultWerte für alles was nicht überschrieben wird
        csmaLeft.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(0)));
        csmaLte.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(0)));
        csmaDsl.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(0)));
        csmaRight.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(0)));

        InternetStackHelper internet;

        NodeContainer leftRightNodes;
        leftRightNodes.Create(2);
        auto tapLeft = leftRightNodes.Get(0);
        auto tapRight = leftRightNodes.Get(1);

        NodeContainer innerNodes;
        innerNodes.Create(2);
        auto innerLeft = innerNodes.Get(0);
        auto innerRight = innerNodes.Get(1);

        NodeContainer csmaIn;
        csmaIn.Add(tapLeft);
        csmaIn.Add(innerLeft);

        NodeContainer LTELeftNodes;
        LTELeftNodes.Add(innerLeft);
        LTELeftNodes.Create(1);
        auto lte = LTELeftNodes.Get(1);

        NodeContainer DSLLeftNodes;
        DSLLeftNodes.Add(innerLeft);
        DSLLeftNodes.Create(1);
        auto dsl = DSLLeftNodes.Get(1);

        NodeContainer lteINVSNodes;
        lteINVSNodes.Add(lte);
        lteINVSNodes.Create(1);
        auto invs = lteINVSNodes.Get(1);

        NodeContainer dslINVSNodes;
        dslINVSNodes.Add(dsl);
        dslINVSNodes.Add(invs);

        NodeContainer invsRightNodes;
        invsRightNodes.Add(invs);
        invsRightNodes.Add(innerRight);

        NodeContainer csmaOut;
        csmaOut.Add(innerRight);
        csmaOut.Add(tapRight);

        Names::Add(config.inDeviceName, tapLeft);
        Names::Add(config.outDeviceName, tapRight);
        Names::Add("innerLeft", innerLeft);
        Names::Add("innerRight", innerRight);
        Names::Add("lte", lte);
        Names::Add("dsl", dsl);
        Names::Add("invs", invs);


        auto csmaInDev = csmaLeft.Install(csmaIn);
        auto leftLteP2PDev = p2pLte.Install(innerLeft, lte);
        auto leftDslP2PDev = p2pDsl.Install(innerLeft, dsl);
        auto csmaLteInvs = csmaLte.Install(lteINVSNodes);
        auto csmaDslInvs = csmaDsl.Install(dslINVSNodes);
        auto p2pInvsRight = p2pRight.Install(invs, innerRight);
        auto csmaOutDev = csmaRight.Install(csmaOut);


        //auto nodes = std::ve
        for(auto& el : {tapLeft, innerLeft, lte, dsl, invs, innerRight, tapRight})
            internet.Install(el);

        Ipv4AddressHelper ipv4AddressHelper;

        std::vector<Ipv4InterfaceContainer> ips{};
        for(auto& a : {csmaInDev, leftLteP2PDev, leftDslP2PDev, csmaLteInvs, csmaDslInvs, p2pInvsRight, csmaOutDev})
        {
            ipv4AddressHelper.SetBase(getNextIPRange(startIPRange).c_str(), "255.255.255.0");
            ips.emplace_back(ipv4AddressHelper.Assign(a));
            //std::cout << a.Get(0)->GetObject<Ipv4>()->GetAddress(0,0) << std::endl;
//            std::cout << Names::FindName(a.Get(0)->GetNode()) << ": " << ips[ips.size() - 1].GetAddress(0)
//                      << " " << Names::FindName(a.Get(1)->GetNode()) << ": "
//                      << ips[ips.size() - 1].GetAddress(1) << std::endl;
        }

        TapBridgeHelper tapBridge;
        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue(config.inDeviceName));
        tapBridge.Install(tapLeft, csmaInDev.Get(0));

        tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
        tapBridge.SetAttribute("DeviceName", StringValue(config.outDeviceName));
        tapBridge.Install(tapRight, csmaOutDev.Get(1));

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
        LinuxStackHelper::PopulateRoutingTables();

//        csma.EnablePcapAll("sim/csma_" + simName, false);
//        p2p.EnablePcapAll("sim/p2p_" + simName, false);

        auto tapLeftIp = ips[0];
        auto tapRightIp = ips[ips.size() - 1];
        Simulator::Schedule(Seconds(1), printTapIps, tapLeftIp, tapRightIp, startIPRange);

        auto delayTime = ns3::CreateObject<ns3::NormalRandomVariable>();
        delayTime->SetAttribute ("Mean", ns3::DoubleValue (0.0065));
        delayTime->SetAttribute ("Variance", ns3::DoubleValue (0.001*0.001));
        delayTime->SetAttribute ("Bound", ns3::DoubleValue (0.003));

        if(config.lteModifier != 0)
            auto lteEvent = ns3::Simulator::Schedule(ns3::Seconds(1), &UnblockConnection, delayTime, config.lteModifier, csmaLteInvs.Get(1));

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

    void BlockLTEConnection(Ptr<ns3::RandomVariableStream> delayTime, double lteModifier, Ptr<NetDevice> csmaNetDevice)
    {
        csmaNetDevice->SetAttribute("ReceiveEnable", ns3::BooleanValue(false));

        auto newDelay = ns3::Seconds(delayTime->GetValue()* lteModifier);
        ns3::Simulator::Schedule(newDelay, &UnblockConnection, delayTime, lteModifier, csmaNetDevice);
    }

    void UnblockConnection(Ptr<ns3::RandomVariableStream> delayTime, double lteModifier, Ptr<NetDevice> csmaNetDevice)
    {
        csmaNetDevice->SetAttribute("ReceiveEnable", ns3::BooleanValue(true));
        ns3::Simulator::Schedule(ns3::MilliSeconds(10),  &BlockLTEConnection, delayTime, lteModifier, csmaNetDevice);
    }
}