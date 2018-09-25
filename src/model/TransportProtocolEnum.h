#pragma once

#include <cstdint>
#include <string>

enum class TransportProtocolEnum : uint8_t {
	RAW = 0,
	UDP = 1,
    UDPLITE = 2,
	TCP = 3,
	SCTP = 4,
    DCCP = 5,
	ROUTE = 6,
    NONE = 7
};

class TransportProtocolWrapper
{
public:
    static std::string toString(TransportProtocolEnum transportProtocol)
    {
        switch (transportProtocol)
        {
            case TransportProtocolEnum::RAW: return "raw";
            case TransportProtocolEnum::UDP: return "udp";
            case TransportProtocolEnum::UDPLITE: return "udplite";
            case TransportProtocolEnum::TCP: return "tcp";
            case TransportProtocolEnum::SCTP: return "sctp";
            case TransportProtocolEnum::DCCP: return "dccp";
            case TransportProtocolEnum::ROUTE: return "route";
            case TransportProtocolEnum::NONE: return "none";
        }
        return "Missing TransportProtocolEnum toString()";
    }

    static TransportProtocolEnum fromString(const std::string& transportProtocol)
    {
        if (transportProtocol == "raw")
            return TransportProtocolEnum::RAW;
        if (transportProtocol == "udp")
            return TransportProtocolEnum::UDP;
        if (transportProtocol == "udplite")
            return TransportProtocolEnum::UDPLITE;
        if (transportProtocol == "tcp")
            return TransportProtocolEnum::TCP;
        if (transportProtocol == "sctp")
            return TransportProtocolEnum::SCTP;
        if (transportProtocol == "dccp")
            return TransportProtocolEnum::DCCP;
        if (transportProtocol == "route")
            return TransportProtocolEnum::ROUTE;
        if (transportProtocol == "none")
            return TransportProtocolEnum::NONE;

        return TransportProtocolEnum::NONE;
    }
};
