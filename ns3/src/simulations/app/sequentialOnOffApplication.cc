/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationOnOff in GTNetS.

#include "sequentialOnOffApplication.h"
#include "../simulationSetup.h"
#include <ns3/network-module.h>
#include <common/Helper/make_unique.h>
#include <common/Helper/StringExtensions.h>
#include <ns3/dce-module.h>

#include <netinet/in.h>
#include "sctp_custom.h"

NS_LOG_COMPONENT_DEFINE ("SequentialOnOffApplication");

simulations::app::SequentialOnOffApplication::SequentialOnOffApplication(ns3::TypeId socketType, ns3::Address remote,
                                   std::string dataRate, uint32_t packetSize,
                                   ns3::Callback<void, size_t, ns3::Time> txCallback)
        : m_socket(0),
          m_connected(false),
          m_residualBits(0),
          m_lastStartTime(ns3::Seconds(0)),
          m_totBytes(0)
{
    NS_LOG_FUNCTION (this);

    m_cbrRate = ns3::DataRate(dataRate);
    m_pktSize = packetSize;
    m_peer = remote;
    m_onTime = ns3::CreateObject<ns3::ConstantRandomVariable>();
    m_onTime->SetAttribute("Constant", ns3::DoubleValue(1.0));

    m_offTime = ns3::CreateObject<ns3::ConstantRandomVariable>();
    m_offTime->SetAttribute("Constant", ns3::DoubleValue(1.0));
    m_tid = socketType;
    sequenceNumber = 0;
    payloadBuffer = std::unique_ptr<uint8_t []>(new uint8_t[packetSize]{});
    m_txCallback = txCallback;
}

simulations::app::SequentialOnOffApplication::~SequentialOnOffApplication()
{
    NS_LOG_FUNCTION (this);
}

ns3::Ptr<ns3::Socket> simulations::app::SequentialOnOffApplication::GetSocket(void) const
{
    NS_LOG_FUNCTION (this);
    return m_socket;
}

void simulations::app::SequentialOnOffApplication::DoDispose(void)
{
    NS_LOG_FUNCTION (this);

    m_socket = 0;
    // chain up
    Application::DoDispose();
}

// Application Methods
void simulations::app::SequentialOnOffApplication::StartApplication() // Called at time specified by Start
{
    NS_LOG_FUNCTION (this);

    // Create the socket if not already
    if (!m_socket)
    {
        m_socket = ns3::Socket::CreateSocket(GetNode(), m_tid);
        if (ns3::Inet6SocketAddress::IsMatchingType(m_peer))
        {
            if (m_socket->Bind6() == -1)
            {
                NS_FATAL_ERROR ("Failed to bind socket");
            }
        } else if (ns3::InetSocketAddress::IsMatchingType(m_peer) ||
                   ns3::PacketSocketAddress::IsMatchingType(m_peer))
        {
            if (m_socket->Bind() == -1)
            {
                NS_FATAL_ERROR ("Failed to bind socket");
            }
        }
        if(!m_txCallback.IsNull())
            m_txCallback(0, ns3::Simulator::Now());

        m_socket->Connect(m_peer);
        m_socket->SetAllowBroadcast(true);
        m_socket->ShutdownRecv();

        m_socket->SetConnectCallback(
                MakeCallback(&simulations::app::SequentialOnOffApplication::ConnectionSucceeded, this),
                MakeCallback(&simulations::app::SequentialOnOffApplication::ConnectionFailed, this));
    }
    m_cbrRateFailSafe = m_cbrRate;

    // Insure no pending event
    CancelEvents();

    if(StringExtensions::toLower(m_tid.GetName()).find("udp") != m_tid.GetName().npos)
        ConnectionSucceeded(m_socket);
}

void simulations::app::SequentialOnOffApplication::StopApplication() // Called at time specified by Stop
{
    NS_LOG_FUNCTION (this);

    CancelEvents();
    if (m_socket != nullptr)
    {
        m_socket->Close();
    } else
    {
        NS_LOG_WARN ("OnOffApplication found null socket to close in StopApplication");
    }
}

void simulations::app::SequentialOnOffApplication::CancelEvents()
{
    NS_LOG_FUNCTION (this);

    if (m_sendEvent.IsRunning() && m_cbrRateFailSafe == m_cbrRate)
    { // Cancel the pending send packet event
        // Calculate residual bits since last packet sent
        ns3::Time delta(ns3::Simulator::Now() - m_lastStartTime);
        ns3::int64x64_t bits = delta.To(ns3::Time::S) * m_cbrRate.GetBitRate();
        m_residualBits += bits.GetHigh();
    }
    m_cbrRateFailSafe = m_cbrRate;
    ns3::Simulator::Cancel(m_sendEvent);
}

// Private helpers
void simulations::app::SequentialOnOffApplication::ScheduleNextTx()
{
    NS_LOG_FUNCTION (this);

    uint32_t bits = m_pktSize * 8 - m_residualBits;
    NS_LOG_LOGIC ("bits = " << bits);
    auto next = bits /
                static_cast<double>(m_cbrRate.GetBitRate());
    ns3::Time nextTime(ns3::Seconds(next)); // Time till next packet
    NS_LOG_LOGIC ("nextTime = " << nextTime);
    m_sendEvent = ns3::Simulator::Schedule(nextTime,
                                           &simulations::app::SequentialOnOffApplication::SendPacket, this);
}

void simulations::app::SequentialOnOffApplication::SendPacket()
{
    NS_LOG_FUNCTION (this);

    NS_ASSERT (m_sendEvent.IsExpired());

    //add sequence number
    if(packet == nullptr)
    {
        ++sequenceNumber;
//        std::cout << sequenceNumber << std::endl;
        memcpy(payloadBuffer.get(), &sequenceNumber, sizeof(sequenceNumber));
        packet = ns3::Create<ns3::Packet>(payloadBuffer.get(), m_pktSize);

        if(!m_txCallback.IsNull())
            m_txCallback(sequenceNumber, ns3::Simulator::Now());
    }

//    for(size_t i=0; i<m_pktSize; ++i)
//    {
//        std::cout << static_cast<int>(payloadBuffer[i]);
//    }
//    std::cout << std::endl;

    int retVal = m_socket->Send(packet);
    if(retVal == -1)
    {
        ScheduleNextTx();
        return;
    }

    packet->RemoveAtStart(static_cast<uint32_t>(retVal));
    m_totBytes += static_cast<size_t>(retVal);

    m_lastStartTime = ns3::Simulator::Now();

    auto packetSize = packet->GetSize();
    m_residualBits = packetSize * 8;

//    std::cout <<sequenceNumber << ", retVal: " << retVal << ", packetSize: " << packetSize << std::endl;
    if(m_residualBits == 0)
    {
        packet = nullptr;
    }

    ScheduleNextTx();
}

/* UDP-Lite socket options */
#define UDPLITE_SEND_CSCOV   10 /* sender partial coverage (as sent)      */
//#define UDPLITE_RECV_CSCOV 11 /* receiver partial coverage (threshold ) */

void simulations::app::SequentialOnOffApplication::ConnectionSucceeded(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    m_connected = true;

    //if(socket->GetTypeId() == SimulationSetup::getSctpTypeId())
    //{
        struct sctp_sndrcvinfo defaultSendOpts{};
        //bzero(&defaultSendOpts, sizeof(defaultSendOpts));

        defaultSendOpts.sinfo_flags = SCTP_UNORDERED;
        //defaultSendOpts.sinfo_stream = 1;
        auto* sendSocket = reinterpret_cast<ns3::LinuxSocketImpl*>(ns3::PeekPointer(socket));

        sendSocket->Setsockopt(SOL_SCTP, SCTP_DEFAULT_SEND_PARAM, &defaultSendOpts, sizeof(defaultSendOpts));

        int flag = 1;
        sendSocket->Setsockopt(SOL_SCTP, SCTP_NODELAY, &flag, sizeof(flag));
        sendSocket->Setsockopt(SOL_SCTP, SCTP_DISABLE_FRAGMENTS, &flag, sizeof(flag));
        flag = 8;
        sendSocket->Setsockopt(IPPROTO_UDPLITE, UDPLITE_SEND_CSCOV, &flag, sizeof(flag));

   // }


    if(!m_txCallback.IsNull())
        m_txCallback(0, ns3::Simulator::Now());

    ScheduleNextTx();
}

void simulations::app::SequentialOnOffApplication::ConnectionFailed(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
}
