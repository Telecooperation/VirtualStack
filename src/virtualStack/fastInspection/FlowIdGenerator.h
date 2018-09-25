#pragma once

#include "../../common/DataStructures/Model/Storage.h"
#include "../../model/InspectionStruct.h"

class FlowIdGenerator final
{
public:
	static void process(StoragePoolPtr &storage);
	
	static flowid_t createFlowId(const TransportProtocolEnum transportProtocol, const sockaddr_storage &destination,
								 const uint16_t sourcePort, const uint16_t destPort);
	static flowid_t createFlowIdForRouter(const TransportProtocolEnum transportProtocol,
										  const sockaddr_storage &destination,
										  const sockaddr_storage &source,
								 const uint16_t sourcePort, const uint16_t destPort);
	static flowid_t createFlowId(const TransportProtocolEnum transportProtocol, const uint64_t destination,
								 const uint16_t sourcePort, const uint16_t destPort);
	static void addSourceToFlowId(flowid_t& flowid, const sockaddr_storage& source);

	/***
	 * Removes the ports from the flowId and so generates a catch all flowId for one ip
	 * @param flowid The normal flowId
	 * @return The catch all flowId only containing the ip and not the ports
	 */
	static flowid_t convertToCatchAllFlowId(const flowid_t flowid);

	static flowid_t generateCatchAllFlowId(const sockaddr_storage &destination);

	static flowid_t generateCatchAllSubFlowId(const sockaddr_storage &destination, uint8_t id);

    static uint64_t getIp(const sockaddr_storage& addr);
private:
	inline static uint64_t getDestIp(const InspectionStruct* inspectionStruct, const uint8_t* data);
	inline static uint64_t foldToUint64(const struct in6_addr& addr);
};


