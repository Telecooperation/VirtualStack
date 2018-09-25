#include "../app/SimulationRunCapture.h"
#include "../app/sequentialOnOffApplication.h"
#include "../simulationSetup.h"
#include "discretSmallPacketSim.h"
#include "../app/sequentialPacketSink.h"

#include <ns3/applications-module.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>

void simulations::p2p::discretSmallPacketSim::run(SimulationSetup& sim)
{
    sim.setSimRuntime(10);

    uint16_t port = 9;   // Discard port (RFC 863)
    uint32_t packetSize = 210;
    std::string sendDataRate = std::to_string(packetSize * 2) + "Bps"; //every 0.5s a packet


    sim.getRunCapture().setPacketSize(packetSize);


    auto onOffApp = app::SequentialOnOffApplication::Install(sim.getSender(),
                                                             sim.getSimSocketType(),
                                                             ns3::Address(ns3::InetSocketAddress(sim.getReceiverAddress(), port)),
                                                             sendDataRate,
                                                             packetSize,
                                                             sim.getRunCapture().getAddSentCallback());

    auto sinkApp = app::SequentialPacketSink::Install(sim.getReceiver(),
                                       sim.getSimSocketType(),
                                       ns3::Address(ns3::InetSocketAddress(sim.getReceiverAddress(), port)),
                                       sim.getRunCapture().getAddReceivedCallback());

    //start by 1.0s because of initialization phase of ns3 dce-cradle
    onOffApp.Start(ns3::Seconds(1.0));
    sinkApp.Start(ns3::Seconds(0.9));
    sim.getRunCapture().setRuntime(sim.getSimRuntime() - 1);

//    onOffApp.Stop(ns3::Seconds(sim.getSimRuntime() - 1));
}

