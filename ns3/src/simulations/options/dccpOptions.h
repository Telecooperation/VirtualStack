#pragma once

#include "../simulationSetup.h"
#include "SimOptions.h"
#include <functional>

namespace simulations
{
    namespace options
    {
        class DccpOptions final : public SimOptions
        {
        public:
            explicit DccpOptions();

            void run(SimulationSetup& sim) override;
        };
    }
}