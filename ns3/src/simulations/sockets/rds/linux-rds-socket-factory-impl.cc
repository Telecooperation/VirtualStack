/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 NICT
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
 * Author: Hajime Tazaki <tazaki@nict.go.jp>
 */
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/linux-socket-impl.h"
#include "linux-rds-socket-factory-impl.h"
#include <netinet/in.h>
#include <arpa/inet.h>

NS_LOG_COMPONENT_DEFINE ("DceLinuxRdsSocketFactoryImpl");

namespace ns3 {

Ptr<Socket>
LinuxRdsSocketFactoryImpl::CreateSocket (void)
{

  Ptr<LinuxSocketImpl> socket = CreateObject<LinuxSocketImpl> ();
  Ptr<Node> node = this->GetObject<Node> ();
  socket->SetNode (node);
  socket->SetAttribute ("Family", UintegerValue (AF_RDS));
  socket->SetAttribute ("SockType", UintegerValue (SOCK_SEQPACKET));
  socket->SetAttribute ("Protocol", UintegerValue (0));
  socket->CreateSocket ();
  return socket;
}


} // namespace ns3
