#include "BaseOverTransport.h"

BaseOverTransport::~BaseOverTransport()
{

}

ns3::NetDeviceContainer &BaseOverTransport::getNetDeviceContainer()
{
    return netDeviceContainer;
}

ns3::Ipv4InterfaceContainer &BaseOverTransport::getInterfaceContainer()
{
    return interfaceContainer;
}
