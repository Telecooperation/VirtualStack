#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/dce-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/internet-module.h>
#include <fstream>

using namespace ns3;

int main_dce (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    NodeContainer nodes;
    nodes.Create (2);

    NetDeviceContainer devices;

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Gbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
    devices = p2p.Install (nodes);
    //p2p.EnablePcapAll ("dce-udp-simple");

    DceManagerHelper processManager;
    processManager.SetTaskManagerAttribute ("FiberManagerType",
                                            StringValue ("UcontextFiberManager"));
    // processManager.SetLoader ("ns3::DlmLoaderFactory");
    processManager.SetNetworkStack("ns3::LinuxSocketFdFactory", "Library", StringValue ("libsim-linux.so"));
    LinuxStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    processManager.Install (nodes);


//    for (int n=0; n < 2; n++)
//    {
//        RunIp (nodes.Get (n), Seconds (0.2), "link show");
//        RunIp (nodes.Get (n), Seconds (0.3), "route show table all");
//        RunIp (nodes.Get (n), Seconds (0.4), "addr list");
//    }

    DceApplicationHelper process;
    ApplicationContainer apps;

    process.SetBinary ("udp-server");
    process.ResetArguments ();
    process.SetStackSize (1<<16);
    apps = process.Install (nodes.Get (0));
    apps.Start (Seconds (1.0));

    process.SetBinary ("udp-client");
    process.ResetArguments ();
    process.ParseArguments ("10.0.0.1");
    apps = process.Install (nodes.Get (1));
    apps.Start (Seconds (1.5));

    Simulator::Stop (Seconds (2000000.0));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
