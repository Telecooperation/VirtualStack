#include "../app/sequentialOnOffApplication.h"
#include "../simulationSetup.h"
#include "throughputSim.h"
#include "../app/SimulationRunCapture.h"
#include "../app/sequentialPacketSink.h"

#include <ns3/applications-module.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/stats-module.h>

void simulations::p2p::throughputSim::run(SimulationSetup& sim)
{
    uint16_t port = 9; //sim.getPort();   // Discard port (RFC 863)
    uint32_t packetSize = sim.payloadSize;//1300;
    std::string sendDataRate = sim.getSendDataRate().Get();

    sim.getRunCapture().setPacketSize(packetSize);


    auto onOffApp = app::SequentialOnOffApplication::Install(sim.getSender(), sim.getSimSocketType(),
                                                             ns3::Address(ns3::InetSocketAddress(sim.getReceiverAddress(), port)),
                                                             sendDataRate, packetSize,
                                                             sim.getRunCapture().getAddSentCallback());

    auto sinkApp = app::SequentialPacketSink::Install(sim.getReceiver(),
                                                      sim.getSimSocketType(),
                                                      ns3::Address(ns3::InetSocketAddress(sim.getReceiverAddress(), port)),
                                                      sim.getRunCapture().getAddReceivedCallback());

   /* auto onOffApp = app::SequentialOnOffApplication::Install(sim.getReceiver(), sim.getSimSocketType(),
                                                             ns3::Address(ns3::InetSocketAddress(sim.getSenderAddress(), port)),
                                                             sendDataRate, packetSize,
                                                             sim.getRunCapture().getAddSentCallback());

    auto sinkApp = app::SequentialPacketSink::Install(sim.getSender(),
                                                      sim.getSimSocketType(),
                                                      ns3::Address(ns3::InetSocketAddress(sim.getSenderAddress(), port)),
                                                      sim.getRunCapture().getAddReceivedCallback());*/

    //start by 1.0s because of initialization phase of ns3 dce-cradle
    onOffApp.Start(ns3::Seconds(1.0));
    sinkApp.Start(ns3::Seconds(0.9));
    sim.getRunCapture().setRuntime(sim.getSimRuntime() - 1);
//    onOffApp.Stop(ns3::Seconds(sim.getSimRuntime() - 1));
//    sinkApp.Stop(ns3::Seconds(sim.getSimRuntime()));
}

