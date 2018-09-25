#pragma once

#include "../common/DataStructures/Model/Storage.h"
#include "InternetProtocolEnum.h"
#include "TransportProtocolEnum.h"
#include <cstdint>

struct flowid_t
{
	TransportProtocolEnum transportProtocol;
	uint8_t subFlow;
	uint16_t sourcePort;
	uint16_t destinationPort;
	uint64_t destination;
	uint64_t source;

	flowid_t()
	{
		bzero(this, sizeof(flowid_t));
	}

	bool operator==(const flowid_t &o) const
	{
		return memcmp(this, &o, sizeof(flowid_t)) == 0;
	}

	bool operator!=(const flowid_t &o) const
	{
		return memcmp(this, &o, sizeof(flowid_t)) != 0;
	}

	bool operator<(const flowid_t &o) const
	{
		return memcmp(this, &o, sizeof(flowid_t)) < 0;
	}

	operator std::string() const noexcept {
		return std::to_string(static_cast<int>(transportProtocol)) +
			   std::to_string(subFlow) +
			   std::to_string(sourcePort) +
			   std::to_string(destinationPort) +
			   std::to_string(destination) +
			   std::to_string(source);
	}

	inline friend std::ostream & operator<< (std::ostream &out, flowid_t const &t)
	{
		return out << static_cast<int>(t.transportProtocol)
				   << static_cast<int>(t.subFlow)
				   << t.sourcePort
				   << t.destinationPort
				   << t.destination
				   << t.source;
	}

    inline std::ostream & operator<<(std::ostream & str)
	{
		// print something from v to str, e.g: Str << v.getX();
		return str << static_cast<int>(transportProtocol)
				   << static_cast<int>(subFlow)
				   << sourcePort
				   << destinationPort
				   << destination
				   << source;
	}
} __attribute__((packed));

//typedef uint64_t flowid_t;

typedef struct InspectionStruct
{
	flowid_t flowId;
	uint32_t transportProtocolStartIndex;
	
	TransportProtocolEnum northboundTransportProtocol;
	InternetProtocolEnum internetProtocol;
	
	static InspectionStruct *getInspectionStruct(Storage &storage)
	{
		return reinterpret_cast<InspectionStruct *>(storage.data(0));
	}
	
	static const InspectionStruct *getInspectionStruct(const Storage &storage)
	{
		return reinterpret_cast<const InspectionStruct *>(storage.constData(0));
	}

	ALLOW_MOVE_SEMANTICS_ONLY(InspectionStruct);
} /*__attribute__((packed))*/ InspectionStruct;



