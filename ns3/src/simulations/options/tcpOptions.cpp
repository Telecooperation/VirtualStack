#include "tcpOptions.h"

simulations::options::TcpOptions::TcpOptions(const std::string &_congestion, bool sack) :
        SimOptions(simulations::SimulationSetup::getTcpTypeId()),
        _sack(sack),
        _congestion(_congestion)
{}

void simulations::options::TcpOptions::run(simulations::SimulationSetup &sim)
{
    //    sim.getStack().SysctlGet(sim.getNodes().Get(0), ns3::Seconds(0.5), ".net.ipv4.tcp_available_congestion_control", [](std::string path, std::string val) { std::cout << path << ", " << val << std::endl; });

    sim.getStack().SysctlSet(sim.getNodes(), ".net.ipv4.tcp_congestion_control", _congestion);
    toggleSACK(sim, _sack);
}

void simulations::options::TcpOptions::toggleSACK(simulations::SimulationSetup &sim, bool newValue)
{
    sim.getStack().SysctlSet(sim.getNodes(), ".net.ipv4.tcp_sack", newValue ? "1" : "0");
    sim.getStack().SysctlSet(sim.getNodes(), ".net.ipv4.tcp_dsack", newValue ? "1" : "0");
    sim.getStack().SysctlSet(sim.getNodes(), ".net.ipv4.tcp_fack", newValue ? "1" : "0");
}
