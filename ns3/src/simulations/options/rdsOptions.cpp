#include "rdsOptions.h"
#include "../sockets/rds/linux-rds-socket-factory-impl.h"

simulations::options::RdsOptions::RdsOptions() :
        SimOptions(simulations::SimulationSetup::getRdsTypeId())
{}

void simulations::options::RdsOptions::run(simulations::SimulationSetup &sim)
{
    for(size_t i = 0; i < sim.getNodes().GetN(); ++i)
    {
        auto node = sim.getNodes().Get(i);
        if(!node->GetObject<ns3::Ipv4Linux>())
            continue;
        auto rdsFactory = ns3::CreateObject<ns3::LinuxRdsSocketFactoryImpl> ();
        node->AggregateObject (rdsFactory);
    }

}
