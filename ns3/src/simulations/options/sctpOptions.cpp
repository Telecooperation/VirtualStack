#include "sctpOptions.h"

simulations::options::SctpOptions::SctpOptions(bool noDelay) :
        SimOptions(simulations::SimulationSetup::getSctpTypeId()),
        _noDelay(noDelay)
{}

void simulations::options::SctpOptions::run(simulations::SimulationSetup &sim)
{
    sim.getStack().SysctlSet(sim.getNodes(), ".net.sctp.max_burst", "16");
}
