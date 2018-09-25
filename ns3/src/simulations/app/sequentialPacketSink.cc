/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */

#include "sequentialPacketSink.h"
#include <ns3/internet-module.h>
#include <ns3/network-module.h>

NS_LOG_COMPONENT_DEFINE ("SequentialPacketSink");

//TypeId
//PacketSink::GetTypeId (void)
//{
//  static TypeId tid = TypeId ("ns3::PacketSink")
//    .SetParent<Application> ()
//    .SetGroupName("Applications")
//    .AddConstructor<PacketSink> ()
//    .AddAttribute ("Local",
//                   "The Address on which to Bind the rx socket.",
//                   AddressValue (),
//                   MakeAddressAccessor (&PacketSink::m_local),
//                   MakeAddressChecker ())
//    .AddAttribute ("Protocol",
//                   "The type id of the protocol to use for the rx socket.",
//                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
//                   MakeTypeIdAccessor (&PacketSink::m_tid),
//                   MakeTypeIdChecker ())
//    .AddTraceSource ("Rx",
//                     "A packet has been received",
//                     MakeTraceSourceAccessor (&PacketSink::m_rxTrace),
//                     "ns3::Packet::AddressTracedCallback")
//  ;
//  return tid;
//}

simulations::app::SequentialPacketSink::SequentialPacketSink(ns3::TypeId socketType, ns3::Address local,
                                                             ns3::Callback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address&> rxCallback)
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;
    m_totalRx = 0;

    m_tid = socketType;
    m_local = local;
    m_rxCallback = rxCallback;
}

simulations::app::SequentialPacketSink::~SequentialPacketSink()
{
    NS_LOG_FUNCTION (this);
}

uint64_t simulations::app::SequentialPacketSink::GetTotalRx() const
{
    NS_LOG_FUNCTION (this);
    return m_totalRx;
}

ns3::Ptr<ns3::Socket> simulations::app::SequentialPacketSink::GetListeningSocket(void) const
{
    NS_LOG_FUNCTION (this);
    return m_socket;
}

std::list<ns3::Ptr<ns3::Socket> > simulations::app::SequentialPacketSink::GetAcceptedSockets(void) const
{
    NS_LOG_FUNCTION (this);
    return m_socketList;
}

void simulations::app::SequentialPacketSink::DoDispose(void)
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;
    m_socketList.clear();

    // chain up
    Application::DoDispose();
}


// Application Methods
void simulations::app::SequentialPacketSink::StartApplication()    // Called at time specified by Start
{
    NS_LOG_FUNCTION (this);
    // Create the socket if not already
    if (!m_socket)
    {
        m_socket = ns3::Socket::CreateSocket(GetNode(), m_tid);
        if (m_socket->Bind(m_local) == -1)
        {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        m_socket->Listen();
        m_socket->ShutdownSend();
        if (ns3::addressUtils::IsMulticast(m_local))
        {
            ns3::Ptr<ns3::UdpSocket> udpSocket = ns3::DynamicCast<ns3::UdpSocket>(m_socket);
            if (udpSocket)
            {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup(0, m_local);
            } else
            {
                NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

    m_socket->SetRecvCallback(ns3::MakeCallback(&simulations::app::SequentialPacketSink::HandleRead, this));
    m_socket->SetAcceptCallback(
            ns3::MakeNullCallback<bool, ns3::Ptr<ns3::Socket>, const ns3::Address &>(),
            ns3::MakeCallback(&simulations::app::SequentialPacketSink::HandleAccept, this));
    m_socket->SetCloseCallbacks(
            ns3::MakeCallback(&simulations::app::SequentialPacketSink::HandlePeerClose, this),
            ns3::MakeCallback(&simulations::app::SequentialPacketSink::HandlePeerError, this));
}

void simulations::app::SequentialPacketSink::StopApplication()     // Called at time specified by Stop
{
    NS_LOG_FUNCTION (this);
    while (!m_socketList.empty()) //these are accepted sockets, close them
    {
        ns3::Ptr<ns3::Socket> acceptedSocket = m_socketList.front();
        m_socketList.pop_front();
        acceptedSocket->Close();
    }
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(ns3::MakeNullCallback<void, ns3::Ptr<ns3::Socket> >());
    }
}

void simulations::app::SequentialPacketSink::HandleRead(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        { //EOF
            break;
        }
        m_totalRx += packet->GetSize();

        if(!m_rxCallback.IsNull())
            m_rxCallback(packet, ns3::Simulator::Now(), from);
    }
}


void simulations::app::SequentialPacketSink::HandlePeerClose(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
}

void simulations::app::SequentialPacketSink::HandlePeerError(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
}


void simulations::app::SequentialPacketSink::HandleAccept(ns3::Ptr<ns3::Socket> s, const ns3::Address &from)
{
    NS_LOG_FUNCTION (this << s << from);
    s->SetRecvCallback(MakeCallback(&simulations::app::SequentialPacketSink::HandleRead, this));
    m_socketList.push_back(s);
}