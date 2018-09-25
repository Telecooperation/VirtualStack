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

#pragma once

#include <ns3/applications-module.h>
#include <ns3/core-module.h>


/**
 * \ingroup applications 
 * \defgroup packetsink PacketSink
 *
 * This application was written to complement OnOffApplication, but it
 * is more general so a PacketSink name was selected.  Functionally it is
 * important to use in multicast situations, so that reception of the layer-2
 * multicast frames of interest are enabled, but it is also useful for
 * unicast as an example of how you can write something simple to receive
 * packets at the application layer.  Also, if an IP stack generates 
 * ICMP Port Unreachable errors, receiving applications will be needed.
 */

/**
 * \ingroup packetsink
 *
 * \brief Receive and consume traffic generated to an IP address and port
 *
 * This application was written to complement OnOffApplication, but it
 * is more general so a PacketSink name was selected.  Functionally it is
 * important to use in multicast situations, so that reception of the layer-2
 * multicast frames of interest are enabled, but it is also useful for
 * unicast as an example of how you can write something simple to receive
 * packets at the application layer.  Also, if an IP stack generates 
 * ICMP Port Unreachable errors, receiving applications will be needed.
 *
 * The constructor specifies the Address (IP address and port) and the 
 * transport protocol to use.   A virtual Receive () method is installed 
 * as a callback on the receiving socket.  By default, when logging is
 * enabled, it prints out the size of packets and their address.
 * A tracing source to Receive() is also available.
 */

namespace simulations
{
    namespace app
    {
        class SequentialPacketSink : public ns3::Application
        {
        public:
            SequentialPacketSink(ns3::TypeId socketType, ns3::Address local,
                                 ns3::Callback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address&> rxCallback =
                                    ns3::MakeNullCallback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address&>());

            virtual ~SequentialPacketSink();

            static ns3::ApplicationContainer Install (ns3::Ptr<ns3::Node> node,
                                                      ns3::TypeId socketType, ns3::Address local,
                                                      ns3::Callback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address&> rxCallback =
                                                        ns3::MakeNullCallback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address&>())
            {
                auto app = ns3::Create<SequentialPacketSink>(socketType, local, rxCallback);
                node->AddApplication(app);
                return ns3::ApplicationContainer(app);
            }

            /**
             * \return the total bytes received in this sink app
             */
            uint64_t GetTotalRx() const;

            /**
             * \return pointer to listening socket
             */
            ns3::Ptr<ns3::Socket> GetListeningSocket(void) const;

            /**
             * \return list of pointers to accepted sockets
             */
            std::list<ns3::Ptr<ns3::Socket>> GetAcceptedSockets(void) const;

        protected:
            virtual void DoDispose(void);

        private:
            // inherited from Application base class.
            virtual void StartApplication(void);    // Called at time specified by Start
            virtual void StopApplication(void);     // Called at time specified by Stop

            /**
             * \brief Handle a packet received by the application
             * \param socket the receiving socket
             */
            void HandleRead(ns3::Ptr<ns3::Socket> socket);

            /**
             * \brief Handle an incoming connection
             * \param socket the incoming connection socket
             * \param from the address the connection is from
             */
            void HandleAccept(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from);

            /**
             * \brief Handle an connection close
             * \param socket the connected socket
             */
            void HandlePeerClose(ns3::Ptr<ns3::Socket> socket);

            /**
             * \brief Handle an connection error
             * \param socket the connected socket
             */
            void HandlePeerError(ns3::Ptr<ns3::Socket> socket);

            // In the case of TCP, each socket accept returns a new socket, so the
            // listening socket is stored separately from the accepted sockets
            ns3::Ptr<ns3::Socket> m_socket;       //!< Listening socket
            std::list<ns3::Ptr<ns3::Socket> >
                    m_socketList; //!< the accepted sockets

            ns3::Address m_local;        //!< Local address to bind to
            uint64_t m_totalRx;      //!< Total bytes received
            ns3::TypeId m_tid;          //!< Protocol TypeId

            /// Traced Callback: received packets, source address.
            ns3::Callback<void, ns3::Ptr<const ns3::Packet>, ns3::Time, const ns3::Address&> m_rxCallback;

        };
    }
}

