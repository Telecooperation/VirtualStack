#pragma once

#include "../simulationSetup.h"
#include "SimOptions.h"
#include <functional>

namespace simulations
{
    namespace options
    {
        class SctpOptions final : public SimOptions
        {
        public:
            explicit SctpOptions(bool noDelay = false);

            void run(SimulationSetup& sim) override;

        private:
            bool _noDelay;
        };
    }
}