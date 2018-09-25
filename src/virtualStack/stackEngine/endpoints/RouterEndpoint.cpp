#include "../../../common/Helper/DomainExtensions.h"
#include "RouterEndpoint.h"
#include <netinet/ip.h>

RouterEndpoint::RouterEndpoint(const flowid_t& flowId, const flowid_t& partnerFlowId) :
        IEndpoint(flowId, partnerFlowId, TransportProtocolEnum::ROUTE,
                  sockaddr_storage{}, sockaddr_storage{})
{}

void RouterEndpoint::processNBIToStack(Storage &packet) const
{
    packet.incrementStartIndex(sizeof(InternetProtocolEnum::Route));
}

void RouterEndpoint::processStackToNBI(Storage &packet) const
{
//    Logger::Log(Logger::DEBUG, "Router: Put into NB flowId: ", flowId);
    auto inspectionStruct = InspectionStruct::getInspectionStruct(packet);
    inspectionStruct->flowId = partnerFlowId;
    inspectionStruct->internetProtocol = InternetProtocolEnum::Route;
    inspectionStruct->northboundTransportProtocol = TransportProtocolEnum::ROUTE;
    //Add the NONE header
    packet.prependDataScalarBeforeStart(DomainExtensions::convertToSystem(InternetProtocolEnum::Route));
}

RouterEndpoint::~RouterEndpoint()
{

}
