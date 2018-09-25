#pragma once


#include "app/SimulationRunCapture.h"
#include "transport/BaseOverTransport.h"
#include <ns3/dce-module.h>
#include <ns3/internet-module.h>
#include <ns3/names.h>
#include <ns3/node-container.h>
#include <ns3/point-to-point-module.h>
#include <ns3/string.h>

namespace simulations
{
    class SimulationSetup
    {
    public:
        SimulationSetup(std::unique_ptr<BaseOverTransport>&& transport);

        void preRun(double packetLoss,
                    const std::string& senderDataRate, const std::string& receiverDataRate,
                    const std::string& delay, uint32_t packetSize);

        void postRun(const std::string& path, const std::string& prefixName);

        void setSimRuntime(uint64_t runtime);
        uint64_t getSimRuntime() const;

        void setSimSocketType(ns3::TypeId simSocketType);
        ns3::TypeId getSimSocketType() const;

        ns3::Ptr<ns3::Node> getSender();
        ns3::Ptr<ns3::Node> getReceiver();

        ns3::StringValue getSendDataRate() const;

        ns3::Ipv4Address getSenderAddress() const;
        ns3::Ipv4Address getReceiverAddress() const;

        static ns3::TypeId getTcpTypeId();
        static ns3::TypeId getUdpTypeId();
        static ns3::TypeId getUdpLiteTypeId();
        static ns3::TypeId getSctpTypeId();
        static ns3::TypeId getDccpTypeId();        
        static ns3::TypeId getRdsTypeId();

        static std::string getTcpTypeStr();
        static std::string getUdpTypeStr();
        static std::string getSctpTypeStr();
        static std::string getDccpTypeStr();        

        ns3::NodeContainer& getNodes();
        ns3::DceManagerHelper& getDceManager();
        ns3::LinuxStackHelper& getStack();
        ns3::NetDeviceContainer& getD0D1();
        ns3::Ipv4InterfaceContainer& getI0I1();
        SimulationRunCapture& getRunCapture();

        uint32_t payloadSize = 0;

private:

        ns3::TypeId simulationSocketType;
        mutable uint64_t simRuntime = 10;

        std::unique_ptr<BaseOverTransport> transport;
        ns3::NodeContainer nodes;
        ns3::DceManagerHelper dceManager;
        ns3::LinuxStackHelper stack;

        ns3::Ipv4InterfaceContainer i0i1;
        SimulationRunCapture runCapture;
    };
}


