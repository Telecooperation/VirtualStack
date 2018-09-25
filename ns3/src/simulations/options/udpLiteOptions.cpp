#include "udpLiteOptions.h"
#include "../sockets/udpLite/linux-udp-lite-socket-factory-impl.h"

simulations::options::UdpLiteOptions::UdpLiteOptions() :
        SimOptions(simulations::SimulationSetup::getUdpLiteTypeId())
{}

void simulations::options::UdpLiteOptions::run(simulations::SimulationSetup &sim)
{
    for(size_t i = 0; i < sim.getNodes().GetN(); ++i)
    {
        auto node = sim.getNodes().Get(i);
        if(!node->GetObject<ns3::Ipv4Linux>())
            continue;
        auto udpFactory = ns3::CreateObject<ns3::LinuxUdpLiteSocketFactoryImpl> ();
        node->AggregateObject (udpFactory);
    }

}
