#include "simulations/simulationRunner.h"
#include "virtualStack/TapNetworkConfig.h"
#include "virtualStack/tapComplexNetwork.h"
#include "virtualStack/tapP2P.h"
#include "virtualStack/tapPath.h"

int main (int argc, char *argv[])
{
    if(argc <= 1)
    {
        simulations::SimulationRunner::run("sim/");
        return 0;
    }

    int readIndex = 1;
    auto simType = std::string(argv[readIndex++]);
    auto startIpIndex = static_cast<uint8_t>(std::atoi(argv[readIndex++]));
    ComplexNetworkConfig config{argc - readIndex, argv + readIndex};

    if (simType == "-tap")
        tapP2P::TapP2P(startIpIndex, config);
    else if (simType == "-taplex")
        tapComplexNetwork::TapComplexNetwork(startIpIndex, config);
    else if (simType == "-tappath")
        tapPath::TapPath(startIpIndex, config);

    return 0;
}
