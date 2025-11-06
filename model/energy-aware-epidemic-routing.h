/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Energy-Aware Epidemic Routing Protocol
 * Optimized for MANET scenarios with multimedia traffic
 */

#ifndef ENERGY_AWARE_EPIDEMIC_ROUTING_H
#define ENERGY_AWARE_EPIDEMIC_ROUTING_H

#include "epidemic-packet-queue.h"
#include "epidemic-packet.h"
#include "epidemic-tag.h"
#include "ns3/energy-module.h"
#include "ns3/device-energy-model.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/random-variable-stream.h"
#include "ns3/timer.h"
#include "ns3/socket.h"

namespace ns3 {
namespace Epidemic {

/**
 * \brief Energy-Aware Epidemic Routing Protocol
 * 
 * This class extends the basic epidemic routing with energy awareness:
 * - Battery level monitoring
 * - Adaptive flooding based on energy levels
 * - Energy-efficient multimedia handling
 * - Smart beacon management
 */
class EnergyAwareRoutingProtocol : public Ipv4RoutingProtocol
{
public:
  static TypeId GetTypeId (void);
  
  EnergyAwareRoutingProtocol ();
  virtual ~EnergyAwareRoutingProtocol ();

  // Inherited from Ipv4RoutingProtocol
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                                      Ptr<NetDevice> oif, Socket::SocketErrno &sockerr) override;
  virtual bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                           Ptr<const NetDevice> idev, const UnicastForwardCallback& ucb,
                           const MulticastForwardCallback& mcb,
                           const LocalDeliverCallback& lcb, const ErrorCallback& ecb) override;
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const override;
  virtual void NotifyInterfaceUp (uint32_t interface) override;
  virtual void NotifyInterfaceDown (uint32_t interface) override;
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) override;
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) override;
  virtual void SetIpv4 (Ptr<Ipv4> ipv4) override;

  // Energy-aware routing methods
  void SetEnergySource (Ptr<energy::EnergySource> source);
  double GetRemainingEnergyRatio () const;
  bool ShouldForwardPacket (const EpidemicHeader& header) const;
  void AdaptBeaconInterval ();
  void HandleLowEnergy ();

private:
  // Core epidemic routing members
  static const uint32_t EPIDEMIC_PORT = 269; ///< Transport Port for epidemic routing
  Ipv4Address m_mainAddress;                  ///< Main IP address for the current node
  uint32_t m_hopCount;                        ///< Number of times a packet is resent
  uint32_t m_maxQueueLen;                     ///< Maximum number of packets a queue can hold
  Time m_queueEntryExpireTime;                ///< Time after which packet expires in queue
  Time m_beaconInterval;                      ///< Time for sending periodic beacon packets
  Time m_hostRecentPeriod;                    ///< Time for host recent period
  uint32_t m_beaconMaxJitterMs;               ///< Upper bound of random time added to beacon interval
  uint16_t m_dataPacketCounter;               ///< Local counter for data packets
  Ptr<Ipv4> m_ipv4;                          ///< Pointer to Ipv4 for current node
  std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses; ///< Map between sockets and IP addresses
  PacketQueue m_queue;                        ///< Queue associated with a node
  Timer m_beaconTimer;                        ///< Timer for sending beacons
  Ptr<UniformRandomVariable> m_beaconJitter;  ///< Random variable for beacon jitter
  typedef std::map<Ipv4Address, Time> HostContactMap; ///< Type to connect host address to recent contact time
  HostContactMap m_hostContactTime;           ///< Hash table to store recent contact time for nodes

  // Energy management
  Ptr<energy::EnergySource> m_energySource;
  double m_energyThresholdLow;     ///< Low energy threshold (0.2 = 20%)
  double m_energyThresholdCritical; ///< Critical energy threshold (0.1 = 10%)
  
  // Adaptive parameters
  double m_energyAwareFloodingFactor; ///< Flooding reduction factor based on energy
  Time m_adaptiveBeaconInterval;      ///< Dynamic beacon interval
  uint32_t m_maxHopsEnergyAware;      ///< Reduced hop count for low energy
  
  // Multimedia optimization
  uint32_t m_speechPacketPriority;    ///< Priority for speech packets
  double m_compressionRatio;          ///< Dynamic compression ratio
  
  // Core epidemic routing methods
  void Start ();
  void RecvEpidemic (Ptr<Socket> socket);
  void SendDisjointPackets (SummaryVectorHeader packet_SMV, Ipv4Address dest);
  void SendBeacons ();
  uint32_t FindOutputDeviceForAddress (Ipv4Address dst);
  uint32_t FindLoopbackDevice ();
  void SendPacket (Ptr<Packet> p, InetSocketAddress addr);
  bool IsMyOwnAddress (Ipv4Address src);
  void BroadcastPacket (Ptr<Packet> p);
  void SendSummaryVector (Ipv4Address dest, bool firstNode);
  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress iface) const;
  void SendPacketFromQueue (Ipv4Address dst, QueueEntry queueEntry);
  bool IsHostContactedRecently (Ipv4Address hostID);
  
  // Energy-aware packet handling
  bool IsEnergyAwareForwarding (const QueueEntry& entry) const;
  void OptimizeMultimediaPacket (Ptr<Packet> packet, const EpidemicHeader& header);
  double CalculateTransmissionCost (const Ptr<const Packet> packet) const;
};

} // namespace Epidemic
} // namespace ns3

#endif /* ENERGY_AWARE_EPIDEMIC_ROUTING_H */
