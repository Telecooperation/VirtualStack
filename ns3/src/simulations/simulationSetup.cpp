#include "SequentialDeterministicErrorModel.h"
#include "simulationSetup.h"
#include "sockets/udpLite/linux-udp-lite-socket-factory.h"
#include "sockets/rds/linux-rds-socket-factory.h"
#include <ns3/error-model.h>
#include <ns3/pointer.h>

simulations::SimulationSetup::SimulationSetup(std::unique_ptr<BaseOverTransport>&& pTransport) :
        transport(std::move(pTransport))
{

}

void simulations::SimulationSetup::preRun(double packetLoss,
                                          const std::string &senderDataRate,
                                          const std::string &receiverDataRate,
                                          const std::string &delay,
                                          uint32_t payloadSize)
{
    //ns3::GlobalValue::Bind("SimulatorImplementationType", ns3::StringValue("ns3::RealtimeSimulatorImpl"));
    ns3::GlobalValue::Bind ("ChecksumEnabled", ns3::BooleanValue (true));
    nodes.Create(2);

    //Add DCE to simulation. Use Sockets from liblinux.so kernel
    dceManager.SetNetworkStack("ns3::LinuxSocketFdFactory", "Library", ns3::StringValue("liblinux.so"));
    dceManager.SetTaskManagerAttribute("FiberManagerType", ns3::StringValue("UcontextFiberManager"));
    dceManager.Install(nodes);

    stack.Install(nodes);

    transport->setup(nodes.Get(0), nodes.Get(1));
    auto& d0d1 = getD0D1();

    //Set name on netDevice instead of Node, to name the connection
    //ns3::Names::Add("sender", d0d1.Get(0));
    //ns3::Names::Add("receiver", d0d1.Get(1));

    //Configure PacketLoss for receiver
    transport->setPacketLoss(packetLoss);
    transport->setDataRate(senderDataRate, receiverDataRate);
    transport->setDelay(delay);

    stack.PopulateRoutingTables();

    stack.RunIp(nodes.Get(0), ns3::Seconds(0.3), "route show");
    stack.RunIp(nodes.Get(1), ns3::Seconds(0.3), "route show");

    this->payloadSize = payloadSize;
}

void simulations::SimulationSetup::postRun(const std::string &path, const std::string& prefixName)
{
    ns3::Simulator::Stop(ns3::Seconds(simRuntime));
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    dceManager.Dispose();

    ns3::Names::Clear();
    ns3::Config::Reset();
    runCapture.saveToFile(path + prefixName  + ".csv");
}

ns3::Ptr<ns3::Node> simulations::SimulationSetup::getSender()
{
    return nodes.Get(0);
}

ns3::Ptr<ns3::Node> simulations::SimulationSetup::getReceiver()
{
    return nodes.Get(1);
}

ns3::Ipv4Address simulations::SimulationSetup::getSenderAddress() const
{
    return transport->getInterfaceContainer().GetAddress(0);
}

ns3::Ipv4Address simulations::SimulationSetup::getReceiverAddress() const
{
    return transport->getInterfaceContainer().GetAddress(1);;
}

ns3::TypeId simulations::SimulationSetup::getTcpTypeId()
{
    return ns3::LinuxTcpSocketFactory::GetTypeId();
}

ns3::TypeId simulations::SimulationSetup::getUdpTypeId()
{
    return ns3::LinuxUdpSocketFactory::GetTypeId();
}

ns3::TypeId simulations::SimulationSetup::getUdpLiteTypeId()
{
    return ns3::LinuxUdpLiteSocketFactory::GetTypeId();
}

ns3::TypeId simulations::SimulationSetup::getSctpTypeId()
{
    return ns3::LinuxSctpSocketFactory::GetTypeId();
}

ns3::NodeContainer &simulations::SimulationSetup::getNodes()
{
    return nodes;
}

ns3::DceManagerHelper &simulations::SimulationSetup::getDceManager()
{
    return dceManager;
}

ns3::LinuxStackHelper& simulations::SimulationSetup::getStack()
{
    return stack;
}

ns3::NetDeviceContainer &simulations::SimulationSetup::getD0D1()
{
    return transport->getNetDeviceContainer();
}

ns3::Ipv4InterfaceContainer &simulations::SimulationSetup::getI0I1()
{
    return  transport->getInterfaceContainer();
}

ns3::StringValue simulations::SimulationSetup::getSendDataRate() const
{
    return transport->getSendDataRate();
}

std::string simulations::SimulationSetup::getTcpTypeStr()
{
    return getTcpTypeId().GetName();
}

std::string simulations::SimulationSetup::getUdpTypeStr()
{
    return getUdpTypeId().GetName();
}

std::string simulations::SimulationSetup::getSctpTypeStr()
{
    return getSctpTypeId().GetName();
}

uint64_t simulations::SimulationSetup::getSimRuntime() const
{
    return simRuntime;
}

ns3::TypeId simulations::SimulationSetup::getSimSocketType() const
{
    return simulationSocketType;
}

void simulations::SimulationSetup::setSimRuntime(uint64_t runtime)
{
    simRuntime = runtime;
}

void simulations::SimulationSetup::setSimSocketType(ns3::TypeId simSocketType)
{
    simulationSocketType = simSocketType;
}

SimulationRunCapture &simulations::SimulationSetup::getRunCapture()
{
    return runCapture;
}

ns3::TypeId simulations::SimulationSetup::getDccpTypeId() {
    return ns3::LinuxDccpSocketFactory::GetTypeId();
}

ns3::TypeId simulations::SimulationSetup::getRdsTypeId() {
    return ns3::LinuxRdsSocketFactory::GetTypeId();
}

std::string simulations::SimulationSetup::getDccpTypeStr() {
    return getDccpTypeId().GetName();
}
