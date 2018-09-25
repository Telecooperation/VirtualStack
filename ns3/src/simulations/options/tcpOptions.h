#pragma once

#include "../simulationSetup.h"
#include "SimOptions.h"
#include <functional>

namespace simulations
{
    namespace options
    {
        class TcpOptions final : public SimOptions
        {
        public:
            explicit TcpOptions(const std::string &_congestion, bool sack = true);

            void run(SimulationSetup& sim) override;
        private:
            void toggleSACK(simulations::SimulationSetup &sim, bool newValue);
            const bool _sack;
            const std::string _congestion;

        };
    }
}