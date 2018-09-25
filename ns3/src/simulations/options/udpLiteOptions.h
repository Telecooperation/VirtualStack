#pragma once

#include "../simulationSetup.h"
#include "SimOptions.h"
#include <functional>

namespace simulations
{
    namespace options
    {
        class UdpLiteOptions final : public SimOptions
        {
        public:
            explicit UdpLiteOptions();

            void run(SimulationSetup& sim) override;
        };
    }
}