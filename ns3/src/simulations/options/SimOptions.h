#pragma once

#include <string>
#include <ns3/type-id.h>
#include "../simulationSetup.h"

namespace simulations
{
    namespace options
    {
        class SimOptions
        {
        public:
            SimOptions(const ns3::TypeId &socketType);
            virtual ~SimOptions();

            void baseRun(simulations::SimulationSetup &sim);
            virtual void run(simulations::SimulationSetup &sim) = 0;

            const ns3::TypeId socketType;
        };
    }
}


