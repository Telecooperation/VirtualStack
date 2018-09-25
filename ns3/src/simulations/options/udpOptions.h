#pragma once

#include "../simulationSetup.h"
#include "SimOptions.h"
#include <functional>

namespace simulations
{
    namespace options
    {
        class UdpOptions final : public SimOptions
        {
        public:
            explicit UdpOptions();

            void run(SimulationSetup& sim) override;
        };
    }
}