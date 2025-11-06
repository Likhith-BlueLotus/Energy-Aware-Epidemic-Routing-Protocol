/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Energy-Aware Epidemic Routing Protocol Implementation
 */

#include "energy-aware-epidemic-routing.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/inet-socket-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-header.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/socket.h"
#include "ns3/output-stream-wrapper.h"
#include <iostream>
#include <algorithm>
#include <functional>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EnergyAwareEpidemicRouting");

namespace Epidemic {

NS_OBJECT_ENSURE_REGISTERED (EnergyAwareRoutingProtocol);

TypeId
EnergyAwareRoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Epidemic::EnergyAwareRoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .AddConstructor<EnergyAwareRoutingProtocol> ()
    .AddAttribute ("HopCount","Maximum number of times "
                   "a packet will be flooded.",
                   UintegerValue (64),
                   MakeUintegerAccessor (&EnergyAwareRoutingProtocol::m_hopCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QueueLength","Maximum number of "
                   "packets that a queue can hold.",
                   UintegerValue (64),
                   MakeUintegerAccessor (&EnergyAwareRoutingProtocol::m_maxQueueLen),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QueueEntryExpireTime","Maximum time a packet can live in "
                   "the epidemic queues since it's generated at the source.",
                   TimeValue (Seconds (100)),
                   MakeTimeAccessor (&EnergyAwareRoutingProtocol::m_queueEntryExpireTime),
                   MakeTimeChecker ())
    .AddAttribute ("HostRecentPeriod","Time in seconds for host recent period"
                   ", in which hosts can not re-exchange summary vectors.",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&EnergyAwareRoutingProtocol::m_hostRecentPeriod),
                   MakeTimeChecker ())
    .AddAttribute ("BeaconInterval","Time in seconds after which a "
                   "beacon packet is broadcast.",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&EnergyAwareRoutingProtocol::m_beaconInterval),
                   MakeTimeChecker ())
    .AddAttribute ("BeaconRandomness","Upper bound of the uniform distribution"
                   " random time added to avoid collisions. Measured in milliseconds",
                   UintegerValue (100),
                   MakeUintegerAccessor (&EnergyAwareRoutingProtocol::m_beaconMaxJitterMs),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EnergyThresholdLow",
                   "Low energy threshold as ratio (0.0-1.0)",
                   DoubleValue (0.2),
                   MakeDoubleAccessor (&EnergyAwareRoutingProtocol::m_energyThresholdLow),
                   MakeDoubleChecker<double> (0.0, 1.0))
    .AddAttribute ("EnergyThresholdCritical",
                   "Critical energy threshold as ratio (0.0-1.0)",
                   DoubleValue (0.1),
                   MakeDoubleAccessor (&EnergyAwareRoutingProtocol::m_energyThresholdCritical),
                   MakeDoubleChecker<double> (0.0, 1.0))
    .AddAttribute ("EnergyAwareFloodingFactor",
                   "Factor to reduce flooding when energy is low",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&EnergyAwareRoutingProtocol::m_energyAwareFloodingFactor),
                   MakeDoubleChecker<double> (0.1, 1.0))
    .AddAttribute ("SpeechPacketPriority",
                   "Priority level for speech packets (higher = more priority)",
                   UintegerValue (10),
                   MakeUintegerAccessor (&EnergyAwareRoutingProtocol::m_speechPacketPriority),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CompressionRatio",
                   "Dynamic compression ratio for multimedia packets",
                   DoubleValue (0.8),
                   MakeDoubleAccessor (&EnergyAwareRoutingProtocol::m_compressionRatio),
                   MakeDoubleChecker<double> (0.1, 1.0));
  return tid;
}

EnergyAwareRoutingProtocol::EnergyAwareRoutingProtocol ()
  : m_hopCount (0),
    m_maxQueueLen (0),
    m_queueEntryExpireTime (Seconds (0)),
    m_beaconInterval (Seconds (0)),
    m_hostRecentPeriod (Seconds (0)),
    m_beaconMaxJitterMs (0),
    m_dataPacketCounter (0),
    m_queue (m_maxQueueLen),
    m_energySource (nullptr),
    m_energyThresholdLow (0.2),
    m_energyThresholdCritical (0.1),
    m_energyAwareFloodingFactor (0.5),
    m_speechPacketPriority (10),
    m_compressionRatio (0.8)
{
  NS_LOG_FUNCTION (this);
  m_adaptiveBeaconInterval = m_beaconInterval;
  m_maxHopsEnergyAware = m_hopCount;
  m_beaconJitter = CreateObject<UniformRandomVariable> ();
}

EnergyAwareRoutingProtocol::~EnergyAwareRoutingProtocol ()
{
  NS_LOG_FUNCTION (this);
}

void
EnergyAwareRoutingProtocol::SetEnergySource (Ptr<energy::EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  m_energySource = source;
}

double
EnergyAwareRoutingProtocol::GetRemainingEnergyRatio () const
{
  if (!m_energySource)
    {
      return 1.0; // Assume full energy if no energy source
    }
  
  double remaining = m_energySource->GetRemainingEnergy ();
  double initial = m_energySource->GetInitialEnergy ();
  
  if (initial <= 0)
    {
      return 0.0;
    }
  
  return remaining / initial;
}

bool
EnergyAwareRoutingProtocol::ShouldForwardPacket (const EpidemicHeader& header) const
{
  NS_LOG_FUNCTION (this);
  
  double energyRatio = GetRemainingEnergyRatio ();
  
  // Critical energy: only forward high priority packets
  if (energyRatio <= m_energyThresholdCritical)
    {
      // Check if this is a speech packet (high priority)
      // This is a simplified check - in practice, you'd examine packet content
      return (header.GetPacketID () % 100) < m_speechPacketPriority;
    }
  
  // Low energy: reduce flooding probability
  if (energyRatio <= m_energyThresholdLow)
    {
      // Use energy-aware flooding factor to reduce transmission probability
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
      return rand->GetValue () < m_energyAwareFloodingFactor;
    }
  
  // Normal energy: forward all packets
  return true;
}

void
EnergyAwareRoutingProtocol::AdaptBeaconInterval ()
{
  NS_LOG_FUNCTION (this);
  
  double energyRatio = GetRemainingEnergyRatio ();
  
  if (energyRatio <= m_energyThresholdCritical)
    {
      // Critical energy: increase beacon interval significantly
      m_adaptiveBeaconInterval = m_beaconInterval * 4;
      m_maxHopsEnergyAware = std::max (1u, m_hopCount / 4);
    }
  else if (energyRatio <= m_energyThresholdLow)
    {
      // Low energy: increase beacon interval moderately
      m_adaptiveBeaconInterval = m_beaconInterval * 2;
      m_maxHopsEnergyAware = std::max (1u, m_hopCount / 2);
    }
  else
    {
      // Normal energy: use default intervals
      m_adaptiveBeaconInterval = m_beaconInterval;
      m_maxHopsEnergyAware = m_hopCount;
    }
  
  NS_LOG_DEBUG ("Energy ratio: " << energyRatio 
                << ", Adaptive beacon interval: " << m_adaptiveBeaconInterval.GetSeconds ()
                << ", Max hops: " << m_maxHopsEnergyAware);
}

void
EnergyAwareRoutingProtocol::HandleLowEnergy ()
{
  NS_LOG_FUNCTION (this);
  
  double energyRatio = GetRemainingEnergyRatio ();
  
  if (energyRatio <= m_energyThresholdCritical)
    {
      NS_LOG_WARN ("Critical energy level reached: " << energyRatio);
      // Purge non-essential packets from queue
      // This would require extending the PacketQueue class
    }
  else if (energyRatio <= m_energyThresholdLow)
    {
      NS_LOG_INFO ("Low energy level: " << energyRatio);
      // Reduce queue size and drop older packets more aggressively
    }
}

bool
EnergyAwareRoutingProtocol::IsEnergyAwareForwarding (const QueueEntry& entry) const
{
  NS_LOG_FUNCTION (this);
  
  // Calculate energy cost for this transmission
  double transmissionCost = CalculateTransmissionCost (entry.GetPacket ());
  double remainingEnergy = m_energySource ? m_energySource->GetRemainingEnergy () : 1000.0;
  
  // Don't forward if transmission would consume too much remaining energy
  if (transmissionCost > remainingEnergy * 0.1) // Don't use more than 10% of remaining energy
    {
      return false;
    }
  
  return true;
}

void
EnergyAwareRoutingProtocol::OptimizeMultimediaPacket (Ptr<Packet> packet, 
                                                      const EpidemicHeader& header)
{
  NS_LOG_FUNCTION (this << packet);
  
  double energyRatio = GetRemainingEnergyRatio ();
  
  // Apply compression based on energy level
  if (energyRatio <= m_energyThresholdLow)
    {
      // Simulate packet compression by reducing effective size
      // In practice, this would involve actual compression algorithms
      uint32_t originalSize = packet->GetSize ();
      uint32_t compressedSize = static_cast<uint32_t> (originalSize * m_compressionRatio);
      
      NS_LOG_DEBUG ("Compressing multimedia packet from " << originalSize 
                    << " to " << compressedSize << " bytes");
      
      // Note: This is a simulation - actual implementation would compress packet content
    }
}

double
EnergyAwareRoutingProtocol::CalculateTransmissionCost (const Ptr<const Packet> packet) const
{
  NS_LOG_FUNCTION (this << packet);
  
  // Simplified energy cost calculation based on packet size
  // In practice, this would consider transmission power, distance, etc.
  uint32_t packetSize = packet->GetSize ();
  double baseCostPerByte = 0.001; // Joules per byte (example value)
  
  return packetSize * baseCostPerByte;
}

void
EnergyAwareRoutingProtocol::SendBeacons ()
{
  NS_LOG_FUNCTION (this);
  
  // Adapt beacon interval based on energy
  AdaptBeaconInterval ();
  
  double energyRatio = GetRemainingEnergyRatio ();
  
  // Skip beacon if energy is critically low
  if (energyRatio <= m_energyThresholdCritical * 0.5)
    {
      NS_LOG_DEBUG ("Skipping beacon due to critically low energy: " << energyRatio);
      m_beaconTimer.Schedule (m_adaptiveBeaconInterval + MilliSeconds
                              (m_beaconJitter->GetValue ()));
      return;
    }
  
  // TODO: Implement epidemic beacon sending functionality
  // This method needs to send summary vectors to neighbors
  NS_LOG_DEBUG ("Sending beacon with adaptive interval: " << m_adaptiveBeaconInterval.GetSeconds ());
  
  // Schedule next beacon with adaptive interval
  m_beaconTimer.Schedule (m_adaptiveBeaconInterval + MilliSeconds
                          (m_beaconJitter->GetValue ()));
}

bool
EnergyAwareRoutingProtocol::RouteInput (Ptr<const Packet> p,
                                        const Ipv4Header &header,
                                        Ptr<const NetDevice> idev,
                                        const UnicastForwardCallback& ucb,
                                        const MulticastForwardCallback& mcb,
                                        const LocalDeliverCallback& lcb,
                                        const ErrorCallback& ecb)
{
  NS_LOG_FUNCTION (this << p << header);

  if (!m_ipv4 || !m_ipv4->IsUp (m_ipv4->GetInterfaceForDevice (idev)))
    {
      NS_LOG_DEBUG ("IPv4 not ready or interface down");
      return false;
    }

  // Handle low energy situations
  HandleLowEnergy ();

  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();

  // Check if packet is for us (local delivery)
  uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  if (m_ipv4->IsDestinationAddress (dst, iif))
    {
      if (!lcb.IsNull ())
        {
          NS_LOG_DEBUG ("Local delivery to " << dst);
          lcb (p, header, iif);
          return true;
        }
      else
        {
          NS_LOG_ERROR ("Local delivery callback is null");
          return false;
        }
    }

  // Check if this is a broadcast or multicast
  if (dst.IsBroadcast () || dst.IsMulticast ())
    {
      NS_LOG_DEBUG ("Received broadcast/multicast packet");
      // Deliver locally if we should
      if (m_ipv4->IsDestinationAddress (dst, iif))
        {
          if (!lcb.IsNull ())
            {
              lcb (p, header, iif);
            }
        }
      // Continue to forward using epidemic routing if needed
    }

  // Check energy before forwarding
  double energyRatio = GetRemainingEnergyRatio ();
  if (energyRatio <= m_energyThresholdCritical * 0.5)
    {
      NS_LOG_DEBUG ("Energy critically low (" << energyRatio << "), dropping packet");
      return false;
    }

  // For epidemic routing in ad-hoc networks, we forward packets
  // This is a simplified implementation - just forward the packet
  NS_LOG_DEBUG ("Forwarding packet from " << origin << " to " << dst);

  // Create a route for forwarding
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();

  // Find an output interface (prefer the one packet came from for now)
  uint32_t oif = iif;
  for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); ++i)
    {
      if (m_ipv4->IsUp (i) && m_ipv4->GetNAddresses (i) > 0 && i != iif)
        {
          oif = i;
          break;
        }
    }

  Ipv4InterfaceAddress ifAddr = m_ipv4->GetAddress (oif, 0);
  route->SetSource (ifAddr.GetLocal ());
  route->SetDestination (dst);
  route->SetGateway (dst); // Direct transmission in ad-hoc
  route->SetOutputDevice (m_ipv4->GetNetDevice (oif));

  // Forward the packet
  ucb (route, p, header);
  return true;
}

void
EnergyAwareRoutingProtocol::SendPacketFromQueue (Ipv4Address dst, QueueEntry queueEntry)
{
  NS_LOG_FUNCTION (this << dst);
  
  // Check energy before sending
  if (!IsEnergyAwareForwarding (queueEntry))
    {
      NS_LOG_DEBUG ("Skipping packet transmission due to energy constraints");
      return;
    }
  
  // TODO: Implement epidemic packet sending from queue
  // This method needs to send the packet using the epidemic routing protocol
  NS_LOG_DEBUG ("Sending packet from queue to " << dst);
  // Full implementation needed here
}

Ptr<Ipv4Route>
EnergyAwareRoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                                         Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << p << header);

  if (!m_ipv4)
    {
      NS_LOG_DEBUG ("IPv4 not set, cannot route");
      sockerr = Socket::ERROR_NOROUTETOHOST;
      return nullptr;
    }

  Ipv4Address dst = header.GetDestination ();
  Ipv4Address src = header.GetSource ();

  NS_LOG_DEBUG ("RouteOutput: packet from " << src << " to " << dst);

  // Check energy before creating route
  double energyRatio = GetRemainingEnergyRatio ();
  if (energyRatio <= m_energyThresholdCritical * 0.5)
    {
      NS_LOG_WARN ("Energy critically low (" << energyRatio << "), cannot route");
      sockerr = Socket::ERROR_NOROUTETOHOST;
      return nullptr;
    }

  // Find the output interface
  uint32_t interface = 0;
  bool interfaceFound = false;

  if (oif)
    {
      int32_t ifIndex = m_ipv4->GetInterfaceForDevice (oif);
      if (ifIndex >= 0)
        {
          interface = static_cast<uint32_t> (ifIndex);
          interfaceFound = true;
        }
    }

  // If no interface specified, find first available non-loopback interface
  if (!interfaceFound)
    {
      for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); ++i)
        {
          // Skip loopback interface (usually interface 0)
          if (i == 0 && m_ipv4->GetNInterfaces () > 1)
            continue;

          if (m_ipv4->IsUp (i) && m_ipv4->GetNAddresses (i) > 0)
            {
              interface = i;
              interfaceFound = true;
              NS_LOG_DEBUG ("Selected interface " << i << " for routing");
              break;
            }
        }
    }

  if (!interfaceFound)
    {
      NS_LOG_WARN ("No valid interface found");
      sockerr = Socket::ERROR_NOROUTETOHOST;
      return nullptr;
    }

  // Check if destination is local
  if (m_ipv4->IsDestinationAddress (dst, interface))
    {
      NS_LOG_DEBUG ("Destination " << dst << " is local");
      sockerr = Socket::ERROR_NOTERROR;
      return nullptr;
    }

  // Create route for epidemic routing
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();

  Ipv4InterfaceAddress ifAddr = m_ipv4->GetAddress (interface, 0);
  route->SetSource (ifAddr.GetLocal ());
  route->SetDestination (dst);
  route->SetGateway (dst); // Direct delivery in ad-hoc network
  route->SetOutputDevice (m_ipv4->GetNetDevice (interface));

  sockerr = Socket::ERROR_NOTERROR;
  NS_LOG_DEBUG ("Created route: " << route->GetSource () << " -> " << dst
                << " via interface " << interface);
  return route;
}

void
EnergyAwareRoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  // Print header
  *os << "================================================================\n";
  *os << "Node " << m_mainAddress << " - Energy-Aware Epidemic Routing\n";
  *os << "Time: " << Simulator::Now ().As (unit) << "\n";
  *os << "================================================================\n";

  // Print energy status
  double energyRatio = GetRemainingEnergyRatio ();
  *os << "Energy Status:\n";
  *os << "  Remaining Energy Ratio: " << (energyRatio * 100) << "%\n";

  if (energyRatio > 0.7)
    *os << "  Energy Level: NORMAL (Green)\n";
  else if (energyRatio > 0.4)
    *os << "  Energy Level: MODERATE (Yellow)\n";
  else if (energyRatio > 0.2)
    *os << "  Energy Level: LOW (Orange)\n";
  else if (energyRatio > 0.0)
    *os << "  Energy Level: CRITICAL (Red)\n";
  else
    *os << "  Energy Level: DEPLETED (Gray)\n";

  *os << "  Adaptive Beacon Interval: " << m_adaptiveBeaconInterval.As (unit) << "\n";
  *os << "  Max Hops (Energy-Aware): " << m_maxHopsEnergyAware << "\n";

  // Print queue status
  *os << "\nPacket Buffer Status:\n";
  *os << "  Queue Size: " << m_queue.GetSize () << "/" << m_maxQueueLen << " packets\n";
  *os << "  Queue Timeout: " << m_queueEntryExpireTime.As (unit) << "\n";

  // Print recent host contacts
  *os << "\nRecent Node Contacts:\n";
  if (m_hostContactTime.empty ())
    {
      *os << "  No recent contacts\n";
    }
  else
    {
      for (auto it = m_hostContactTime.begin (); it != m_hostContactTime.end (); ++it)
        {
          Time lastContact = it->second;
          Time timeSince = Simulator::Now () - lastContact;
          *os << "  Node " << it->first << " - Last contact: "
              << timeSince.As (unit) << " ago\n";
        }
    }

  // Print epidemic routing parameters
  *os << "\nRouting Protocol Parameters:\n";
  *os << "  Max Hop Count: " << m_hopCount << "\n";
  *os << "  Beacon Interval: " << m_beaconInterval.As (unit) << "\n";
  *os << "  Host Recent Period: " << m_hostRecentPeriod.As (unit) << "\n";

  // Print network interfaces
  *os << "\nNetwork Interfaces:\n";
  if (m_ipv4)
    {
      for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); ++i)
        {
          if (m_ipv4->GetNAddresses (i) > 0)
            {
              Ipv4InterfaceAddress addr = m_ipv4->GetAddress (i, 0);
              *os << "  Interface " << i << ": " << addr.GetLocal ()
                  << " (" << (m_ipv4->IsUp (i) ? "UP" : "DOWN") << ")\n";
            }
        }
    }

  // Epidemic routing doesn't maintain traditional routing table
  // Instead, it uses opportunistic forwarding
  *os << "\nRouting Mode: Opportunistic Store-and-Forward\n";
  *os << "  Packets are stored in buffer and forwarded when nodes meet\n";
  *os << "  No static routes - uses epidemic flooding with anti-entropy\n";
  *os << "================================================================\n\n";
}

void
EnergyAwareRoutingProtocol::NotifyInterfaceUp (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
  // TODO: Handle interface up event
}

void
EnergyAwareRoutingProtocol::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
  // TODO: Handle interface down event
}

void
EnergyAwareRoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  // TODO: Handle address addition
}

void
EnergyAwareRoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  // TODO: Handle address removal
}

void
EnergyAwareRoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION (this << ipv4);
  m_ipv4 = ipv4;
  // TODO: Initialize routing protocol with IPv4
}

} // namespace Epidemic
} // namespace ns3
