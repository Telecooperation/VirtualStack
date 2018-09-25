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

#pragma once

#include <memory>
#include <ns3/applications-module.h>
#include <ns3/core-module.h>
#include <ns3/dce-module.h>
/**
 * \ingroup applications 
 * \defgroup onoff OnOffApplication
 *
 * This traffic generator follows an On/Off pattern: after 
 * Application::StartApplication
 * is called, "On" and "Off" states alternate. The duration of each of
 * these states is determined with the onTime and the offTime random
 * variables. During the "Off" state, no traffic is generated.
 * During the "On" state, cbr traffic is generated. This cbr traffic is
 * characterized by the specified "data rate" and "packet size".
 */
/**
* \ingroup onoff
*
* \brief Generate traffic to a single destination according to an
*        OnOff pattern.
*
* This traffic generator follows an On/Off pattern: after
* Application::StartApplication
* is called, "On" and "Off" states alternate. The duration of each of
* these states is determined with the onTime and the offTime random
* variables. During the "Off" state, no traffic is generated.
* During the "On" state, cbr traffic is generated. This cbr traffic is
* characterized by the specified "data rate" and "packet size".
*
* Note:  When an application is started, the first packet transmission
* occurs _after_ a delay equal to (packet size/bit rate).  Note also,
* when an application transitions into an off state in between packet
* transmissions, the remaining time until when the next transmission
* would have occurred is cached and is used when the application starts
* up again.  Example:  packet size = 1000 bits, bit rate = 500 bits/sec.
* If the application is started at time 3 seconds, the first packet
* transmission will be scheduled for time 5 seconds (3 + 1000/500)
* and subsequent transmissions at 2 second intervals.  If the above
* application were instead stopped at time 4 seconds, and restarted at
* time 5.5 seconds, then the first packet would be sent at time 6.5 seconds,
* because when it was stopped at 4 seconds, there was only 1 second remaining
* until the originally scheduled transmission, and this time remaining
* information is cached and used to schedule the next transmission
* upon restarting.
*
* If the underlying socket type supports broadcast, this application
* will automatically enable the SetAllowBroadcast(true) socket option.
*/

namespace simulations
{
    namespace app
    {
        class SequentialOnOffApplication : public ns3::Application
        {
        public:
            SequentialOnOffApplication(ns3::TypeId socketType, ns3::Address remote,
                                       std::string dataRate, uint32_t packetSize,
                                       ns3::Callback<void, size_t, ns3::Time> txCallback = ns3::MakeNullCallback<void, size_t, ns3::Time>());

            virtual ~SequentialOnOffApplication();

            static ns3::ApplicationContainer Install (ns3::Ptr<ns3::Node> node,
                                 ns3::TypeId socketType, ns3::Address remote,
                                 std::string dataRate, uint32_t packetSize,
                                 ns3::Callback<void, size_t, ns3::Time> txCallback = ns3::MakeNullCallback<void, size_t, ns3::Time>())
            {
                auto app = ns3::Create<SequentialOnOffApplication>(socketType, remote, dataRate, packetSize, txCallback);

                node->AddApplication(app);

                return ns3::ApplicationContainer(app);
            }

            /**
             * \brief Return a pointer to associated socket.
             * \return pointer to associated socket
             */
            ns3::Ptr<ns3::Socket> GetSocket(void) const;


            // inherited from Application base class.
            virtual void StartApplication(void);    // Called at time specified by Start
            virtual void StopApplication(void);     // Called at time specified by Stop

            //helpers
            /**
             * \brief Cancel all pending events.
             */
            void CancelEvents();

            /**
             * \brief Send a packet
             */
            void SendPacket();

            ns3::Ptr<ns3::Packet> packet = nullptr;
            ns3::Ptr<ns3::Socket> m_socket;       //!< Associated socket
            ns3::Address m_peer;         //!< Peer address
            bool m_connected;    //!< True if connected
            ns3::Ptr<ns3::RandomVariableStream> m_onTime;       //!< rng for On Time
            ns3::Ptr<ns3::RandomVariableStream> m_offTime;      //!< rng for Off Time
            ns3::DataRate m_cbrRate;      //!< Rate that data is generated
            ns3::DataRate m_cbrRateFailSafe;      //!< Rate that data is generated (check copy)
            uint32_t m_pktSize;      //!< Size of packets
            uint32_t m_residualBits; //!< Number of generated, but not sent, bits
            ns3::Time m_lastStartTime; //!< Time last packet sent
            uint64_t m_totBytes;     //!< Total bytes sent so far
            ns3::EventId m_sendEvent;    //!< Event id of pending "send packet" event
            ns3::TypeId m_tid;          //!< Type of the socket used
            size_t sequenceNumber;
            std::unique_ptr<uint8_t[]> payloadBuffer;

            /// Traced Callback: transmitted packets.
            ns3::Callback<void, size_t, ns3::Time> m_txCallback;

        protected:
            virtual void DoDispose(void);

        private:
            /**
             * \brief Schedule the next packet transmission
             */
            void ScheduleNextTx();

            /**
             * \brief Schedule the next On period start
             */
            void ScheduleStartEvent();

            /**
             * \brief Schedule the next Off period start
             */
            void ScheduleStopEvent();

            /**
             * \brief Handle a Connection Succeed event
             * \param socket the connected socket
             */
            void ConnectionSucceeded(ns3::Ptr<ns3::Socket> socket);

            /**
             * \brief Handle a Connection Failed event
             * \param socket the not connected socket
             */
            void ConnectionFailed(ns3::Ptr<ns3::Socket> socket);

            void setSockOpt(int level, int optname, const void* optval, socklen_t optlen)
            {
                auto* tmpSocket = ns3::PeekPointer(m_socket);
                auto* sendSocket = reinterpret_cast<ns3::LinuxSocketImpl*>(tmpSocket);
                sendSocket->Setsockopt(level, optname, optval, optlen);
            }
        };
    }
}

