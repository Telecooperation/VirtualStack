#pragma once

#include <array>
#include <string>
#include "../model/TransportProtocolEnum.h"

//#####################
//ACHTUNG: Änderungen der Reihenfolge oder hinzufügen müssenb mit der darunter liegenden "StackEnumString" UND "StackEnumSize" synchronisiert werden
//ACHTUNG: Änderungen der Reihenfolge oder hinzufügen müssenb mit der darunter liegenden "StackEnumString" UND "StackEnumSize" synchronisiert werden
//ACHTUNG: Änderungen der Reihenfolge oder hinzufügen müssenb mit der darunter liegenden "StackEnumString" UND "StackEnumSize" synchronisiert werden
//#####################
enum class StackEnum : uint8_t
{
    TCPIPv4,
    UDPIPv4,
    UDPLITEIPv4,
    SCTPIPv4,
    DCCPIPv4,
    UDPPlusIPv4,
    UDPPlusNoFecIPv4,
    UDPPlusNoFlowFecIPv4,
    UDPPlusNoFlowIPv4,
    UDPPlusOnlyFecIPv4,
    UDPPlusOnlyFlowIPv4,
    UDPPlusOnlyFlowFecIPv4,
    SoftwareLoop,
    Invalid
};
constexpr size_t StackEnumSize = 14;

struct StackEnumInfo
{
    StackEnumInfo(StackEnum stack,
                  TransportProtocolEnum transportProtocol,
                  uint16_t headerSize,
                  bool isReliable,
                  const std::string& name,
                  StackEnum baseStack = StackEnum::Invalid) :
            Stack(stack),
            BaseStack(baseStack == StackEnum::Invalid ? stack : baseStack),
            TransportProtocol(transportProtocol),
            HeaderSize(headerSize),
            IsReliable(isReliable),
            Name(name)
    {
    }

    const StackEnum Stack;
    const StackEnum BaseStack;
    const TransportProtocolEnum TransportProtocol;
    const uint16_t HeaderSize;
    const bool IsReliable;
    const std::string Name;
};

const std::array<StackEnumInfo, StackEnumSize> StackEnumInfos{
        StackEnumInfo(StackEnum::TCPIPv4, TransportProtocolEnum::TCP, 44, true, "tcpipv4"),
        StackEnumInfo(StackEnum::UDPIPv4, TransportProtocolEnum::UDP, 8, false, "udpipv4"),
        StackEnumInfo(StackEnum::UDPLITEIPv4, TransportProtocolEnum::UDPLITE, 8, false, "udpliteipv4"),
        StackEnumInfo(StackEnum::SCTPIPv4, TransportProtocolEnum::SCTP, 44, true, "sctpipv4"),
        StackEnumInfo(StackEnum::DCCPIPv4, TransportProtocolEnum::DCCP, 32, false, "dccpipv4"),
        StackEnumInfo(StackEnum::UDPPlusIPv4, TransportProtocolEnum::UDP, 20, true, "udpplusipv4"),
        StackEnumInfo(StackEnum::UDPPlusNoFecIPv4, TransportProtocolEnum::UDP, 10, true, "udpplusnofecipv4", StackEnum::UDPPlusIPv4),
        StackEnumInfo(StackEnum::UDPPlusNoFlowFecIPv4, TransportProtocolEnum::UDP, 10, true, "udpplusnoflowfecipv4", StackEnum::UDPPlusIPv4),
        StackEnumInfo(StackEnum::UDPPlusNoFlowIPv4, TransportProtocolEnum::UDP, 20, true, "udpplusnoflowipv4", StackEnum::UDPPlusIPv4),
        StackEnumInfo(StackEnum::UDPPlusOnlyFecIPv4, TransportProtocolEnum::UDP, 20, false, "udpplusonlyfecipv4", StackEnum::UDPPlusIPv4),
        StackEnumInfo(StackEnum::UDPPlusOnlyFlowFecIPv4, TransportProtocolEnum::UDP, 10, false, "udpplusonlyflowipv4", StackEnum::UDPPlusIPv4),
        StackEnumInfo(StackEnum::UDPPlusOnlyFlowIPv4, TransportProtocolEnum::UDP, 20, false, "udpplusonlyflowfecipv4", StackEnum::UDPPlusIPv4),
        StackEnumInfo(StackEnum::SoftwareLoop, TransportProtocolEnum::NONE, 0, true, "softwareloop"),
        StackEnumInfo(StackEnum::Invalid, TransportProtocolEnum::RAW, 0, false, "invalid")
};

static_assert((static_cast<uint16_t>(StackEnum::Invalid) + 1u) == StackEnumSize,
              "StackEnum and StackEnumSize have to be of same size");


