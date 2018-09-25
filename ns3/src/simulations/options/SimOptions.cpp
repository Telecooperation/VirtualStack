#include "SimOptions.h"

simulations::options::SimOptions::SimOptions(const ns3::TypeId &pSocketType) :
        socketType(pSocketType)
{}

simulations::options::SimOptions::~SimOptions()
{

}

void simulations::options::SimOptions::baseRun(simulations::SimulationSetup &sim)
{
    sim.setSimSocketType(socketType);
}
