#pragma once
#include <string>
#include "../simulationSetup.h"

namespace simulations
{
    namespace p2p
    {
        class discretSmallPacketSim
        {

        public:
            static void run(SimulationSetup& sim);
        };
    }
}