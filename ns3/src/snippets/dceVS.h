#pragma once

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/dce-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/internet-module.h>
#include <fstream>

using namespace ns3;

int runVs (int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    NodeContainer nodes;
    nodes.Create(2);

    NetDeviceContainer devices;

    //SimpleNetDeviceHelper p2p;
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));
    devices = p2p.Install(nodes);
    //p2p.EnablePcapAll ("dce-udp-simple");

    DceManagerHelper processManager;
    processManager.SetTaskManagerAttribute("FiberManagerType",
                                           StringValue("PthreadFiberManager"));
     processManager.SetLoader ("ns3::CoojaLoaderFactory");
//    processManager.SetNetworkStack("ns3::LinuxSocketFdFactory", "Library", StringValue("libsim-linux.so"));
//    LinuxStackHelper stack;

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    processManager.Install(nodes);


//    for (int n=0; n < 2; n++)
//    {
//        RunIp (nodes.Get (n), Seconds (0.2), "link show");
//        RunIp (nodes.Get (n), Seconds (0.3), "route show table all");
//        RunIp (nodes.Get (n), Seconds (0.4), "addr list");
//    }

    DceApplicationHelper process;
    ApplicationContainer apps;

    std::stringstream dummyStream;
    interfaces.GetAddress(0).Print(dummyStream);
    auto serverIp = dummyStream.str();

    dummyStream.str("");
    dummyStream.clear();
    interfaces.GetAddress(1).Print(dummyStream);
    auto clientIp = dummyStream.str();


    process.SetBinary("ns3-vsServer");
    process.SetStackSize(1 << 24);
    process.ResetArguments();
    process.AddArgument(serverIp);
    process.AddArgument("11000");
    apps = process.Install(nodes.Get(0));
    apps.Start(Seconds(0));

    process.SetBinary("ns3-vsClient");
    process.SetStackSize(1 << 24);
    process.ResetArguments();
    process.AddArgument(clientIp);
    process.AddArgument(serverIp);
    process.AddArgument("10000");
    process.AddArgument("11000");
    apps = process.Install(nodes.Get(1));
    apps.Start(Seconds(0.1));

    Simulator::Stop(Seconds(12.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
