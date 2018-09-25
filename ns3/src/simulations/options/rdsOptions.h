#pragma once

#include "../simulationSetup.h"
#include "SimOptions.h"
#include <functional>

namespace simulations
{
    namespace options
    {
        class RdsOptions final : public SimOptions
        {
        public:
            explicit RdsOptions();

            void run(SimulationSetup& sim) override;
        };
    }
}