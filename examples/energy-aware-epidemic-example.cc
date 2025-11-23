/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Energy-Aware Epidemic Routing Example
 * Emergency Communication Network Scenario
 * 
 * This scenario simulates a disaster-affected area with rescue workers
 * communicating via mobile ad-hoc network with energy constraints.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/energy-module.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/energy-aware-epidemic-helper.h"
#include "ns3/netanim-module.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace ns3;

/**
 * \file
 * \ingroup epidemic
 * Emergency Communication Network Scenario
 * 
 * Environment: Disaster-affected area (500m × 500m)
 * Nodes: 20 mobile rescue workers with handheld devices
 * Traffic: Voice communication + GPS coordinates + emergency alerts
 * Energy: Limited battery capacity (1000J), no recharging capability
 * Mobility: Random waypoint (5-15 m/s)
 * Duration: 300 seconds (5 minutes)
 */

// Global counters for PDR calculation
uint32_t g_totalTx = 0;
uint32_t g_totalRx = 0;

void RxTrace (std::string context, Ptr<const Packet> packet, const Address &addr)
{
  g_totalRx++;
}

void TxTrace (std::string context, Ptr<const Packet> packet)
{
  g_totalTx++;
}

void PrintPdr ()
{
  double pdr = (g_totalTx > 0) ? ((double)g_totalRx / g_totalTx) : 0.0;
  std::cout << "PDR: " << pdr << " at time " << Simulator::Now ().GetSeconds () << std::endl;
  Simulator::Schedule (Seconds (10.0), &PrintPdr);
}

// Energy monitoring class for tracking battery levels
class EnergyMonitor
{
public:
  EnergyMonitor (NodeContainer nodes);
  void MonitorEnergy ();
  void PrintEnergyStats ();
  void SetAnimationInterface (ns3::AnimationInterface* anim);
  void UpdateNodeColors ();

private:
  NodeContainer m_nodes;
  std::vector<double> m_initialEnergy;
  std::vector<double> m_currentEnergy;
  Time m_monitoringInterval;
  EventId m_monitoringEvent;
  ns3::AnimationInterface* m_anim;

  void UpdateEnergyStats ();
};

EnergyMonitor::EnergyMonitor (NodeContainer nodes)
  : m_nodes (nodes),
    m_monitoringInterval (Seconds (30.0)),
    m_anim (nullptr)
{
  m_initialEnergy.resize (nodes.GetN ());
  m_currentEnergy.resize (nodes.GetN ());
  
  // Record initial energy levels
  for (uint32_t i = 0; i < nodes.GetN (); ++i)
    {
      // Energy sources are stored in an EnergySourceContainer
      Ptr<energy::EnergySourceContainer> container = nodes.Get (i)->GetObject<energy::EnergySourceContainer> ();
      if (container && container->GetN () > 0)
        {
          Ptr<energy::EnergySource> source = container->Get (0);
          m_initialEnergy[i] = source->GetInitialEnergy ();
          m_currentEnergy[i] = source->GetRemainingEnergy ();
        }
    }
}

void
EnergyMonitor::MonitorEnergy ()
{
  UpdateEnergyStats ();
  m_monitoringEvent = Simulator::Schedule (m_monitoringInterval,
                                           &EnergyMonitor::MonitorEnergy, this);
}

void
EnergyMonitor::UpdateEnergyStats ()
{
  for (uint32_t i = 0; i < m_nodes.GetN (); ++i)
    {
      // Energy sources are stored in an EnergySourceContainer
      Ptr<energy::EnergySourceContainer> container = m_nodes.Get (i)->GetObject<energy::EnergySourceContainer> ();
      if (container && container->GetN () > 0)
        {
          Ptr<energy::EnergySource> source = container->Get (0);
          m_currentEnergy[i] = source->GetRemainingEnergy ();
        }
    }
}

void
EnergyMonitor::PrintEnergyStats ()
{
  std::cout << "\n=== Emergency Network Energy Report ===" << std::endl;
  std::cout << "Time: " << Simulator::Now ().GetSeconds () << " seconds" << std::endl;
  
  double totalInitial = 0.0;
  double totalRemaining = 0.0;
  uint32_t nodesAlive = 0;
  
  for (uint32_t i = 0; i < m_nodes.GetN (); ++i)
    {
      double efficiency = (m_initialEnergy[i] > 0) ? 
                         (m_currentEnergy[i] / m_initialEnergy[i]) * 100 : 0;
      
      if (m_currentEnergy[i] > 0.0)
        {
          nodesAlive++;
        }
      
      std::cout << "Node " << i << ": "
                << "Energy=" << m_currentEnergy[i] << "J (" 
                << efficiency << "%), "
                << "Status=" << (m_currentEnergy[i] > 0 ? "OPERATIONAL" : "DEPLETED") 
                << std::endl;
      
      totalInitial += m_initialEnergy[i];
      totalRemaining += m_currentEnergy[i];
    }
  
  double networkEfficiency = (totalInitial > 0) ? (totalRemaining / totalInitial) * 100 : 0;
  
  std::cout << "\nNetwork Status:" << std::endl;
  std::cout << "  Nodes Operational: " << nodesAlive << "/" << m_nodes.GetN () << std::endl;
  std::cout << "  Total Energy Remaining: " << totalRemaining << "J" << std::endl;
  std::cout << "  Network Energy Efficiency: " << networkEfficiency << "%" << std::endl;
  std::cout << "=======================================\n" << std::endl;

  // Update node colors in NetAnim based on energy levels
  UpdateNodeColors ();
}

void
EnergyMonitor::SetAnimationInterface (ns3::AnimationInterface* anim)
{
  m_anim = anim;
}

void
EnergyMonitor::UpdateNodeColors ()
{
  if (!m_anim)
    return;

  for (uint32_t i = 0; i < m_nodes.GetN (); ++i)
    {
      double energyPercent = (m_initialEnergy[i] > 0) ?
                             (m_currentEnergy[i] / m_initialEnergy[i]) * 100 : 0;

      // Color coding based on energy level:
      // Green (100-70%): Fully operational
      // Yellow (70-40%): Medium energy
      // Orange (40-20%): Low energy
      // Red (<20%): Critical energy
      // Gray (0%): Depleted

      uint8_t r, g, b;
      if (energyPercent >= 70.0)
        {
          // Green to Yellow transition
          r = static_cast<uint8_t>((100.0 - energyPercent) * 255 / 30.0);
          g = 255;
          b = 0;
        }
      else if (energyPercent >= 40.0)
        {
          // Yellow to Orange transition
          r = 255;
          g = static_cast<uint8_t>((energyPercent - 40.0) * 255 / 30.0);
          b = 0;
        }
      else if (energyPercent >= 20.0)
        {
          // Orange to Red transition
          r = 255;
          g = static_cast<uint8_t>((energyPercent - 20.0) * 128 / 20.0);
          b = 0;
        }
      else if (energyPercent > 0.0)
        {
          // Red
          r = 255;
          g = 0;
          b = 0;
        }
      else
        {
          // Gray - depleted
          r = 128;
          g = 128;
          b = 128;
        }

      m_anim->UpdateNodeColor (i, r, g, b);

      // Update node description with current status
      std::ostringstream desc;
      desc << "Worker " << i << " [" << std::fixed << std::setprecision(1)
           << energyPercent << "%]";
      m_anim->UpdateNodeDescription (i, desc.str ());
    }
}

int main (int argc, char *argv[])
{
  // Emergency Communication Network Parameters
  uint32_t nNodes = 20;              // 20 rescue workers
  double simulationTime = 600.0;     // 10 minutes (increased to see energy depletion)
  double initialEnergy = 1000.0;     // 1000 Joules per device (reduced to see depletion faster)
  double nodeSpeedMin = 5.0;         // Minimum speed (m/s)
  double nodeSpeedMax = 15.0;        // Maximum speed (m/s)
  double transmissionRange = 100.0; // 100m transmission range
  double areaSize = 500.0;           // 500m × 500m disaster area

  // Communication parameters
  uint32_t packetSize = 512;        // Bytes (voice + GPS + alert)
  std::string dataRate = "128kbps";  // Increased data rate for faster energy consumption
  uint32_t numCommunicationPairs = 10; // Increased to 10 pairs for more traffic
  std::string audioFilePath = "contrib/epidemic/examples/sample-15s.mp3"; // MP3 file path

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of rescue workers", nNodes);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("initialEnergy", "Initial energy per node in Joules", initialEnergy);
  cmd.AddValue ("nodeSpeedMin", "Minimum node speed in m/s", nodeSpeedMin);
  cmd.AddValue ("nodeSpeedMax", "Maximum node speed in m/s", nodeSpeedMax);
  cmd.AddValue ("transmissionRange", "Transmission range in meters", transmissionRange);
  cmd.AddValue ("areaSize", "Size of disaster area in meters", areaSize);
  cmd.AddValue ("numPairs", "Number of communication pairs", numCommunicationPairs);
  cmd.AddValue ("audioFile", "Path to audio file (MP3) to transmit", audioFilePath);
  cmd.Parse (argc, argv);
  
  // Read and get MP3 file size
  uint64_t audioFileSize = 0;
  std::ifstream audioFile;
  audioFile.open (audioFilePath, std::ios::binary | std::ios::ate);
  if (audioFile.is_open ())
    {
      audioFileSize = audioFile.tellg ();
      audioFile.close ();
      std::cout << "Audio file found: " << audioFilePath << " (" 
                << (audioFileSize / 1024.0) << " KB)" << std::endl;
    }
  else
    {
      std::cerr << "Warning: Could not open audio file: " << audioFilePath << std::endl;
      std::cerr << "Using default packet size instead." << std::endl;
      audioFileSize = packetSize * 100; // Fallback: send 100 packets worth
    }

  std::cout << "\n================================================" << std::endl;
  std::cout << "  EMERGENCY COMMUNICATION NETWORK SIMULATION" << std::endl;
  std::cout << "================================================" << std::endl;
  std::cout << "Scenario: Disaster-affected area communication" << std::endl;
  std::cout << "Rescue Workers: " << nNodes << std::endl;
  std::cout << "Simulation Duration: " << simulationTime << " seconds" << std::endl;
  std::cout << "Area Coverage: " << areaSize << "m × " << areaSize << "m" << std::endl;
  std::cout << "Initial Battery: " << initialEnergy << " Joules per device" << std::endl;
  std::cout << "Transmission Range: " << transmissionRange << " meters" << std::endl;
  std::cout << "Communication Channels: " << numCommunicationPairs << std::endl;
  if (audioFileSize > 0)
    {
      std::cout << "Audio File: " << audioFilePath << " (" << (audioFileSize / 1024.0) << " KB)" << std::endl;
    }
  std::cout << "================================================\n" << std::endl;

  // Enable logging
  LogComponentEnable ("EnergyAwareEpidemicRouting", LOG_LEVEL_INFO);
  LogComponentEnable ("EnergyAwareEpidemicHelper", LOG_LEVEL_INFO);

  // Create nodes (rescue workers)
  NodeContainer nodes;
  nodes.Create (nNodes);

  // Mobility setup - Random waypoint for rescue workers
  MobilityHelper mobility;
  std::ostringstream speedBounds;
  speedBounds << "ns3::UniformRandomVariable[Min=" << nodeSpeedMin
              << "|Max=" << nodeSpeedMax << "]";

  // Create position allocator for initial positions
  Ptr<ListPositionAllocator> initialPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<UniformRandomVariable> xPos = CreateObject<UniformRandomVariable> ();
  xPos->SetAttribute ("Min", DoubleValue (0.0));
  xPos->SetAttribute ("Max", DoubleValue (areaSize));
  Ptr<UniformRandomVariable> yPos = CreateObject<UniformRandomVariable> ();
  yPos->SetAttribute ("Min", DoubleValue (0.0));
  yPos->SetAttribute ("Max", DoubleValue (areaSize));

  // Assign random initial positions to all nodes
  for (uint32_t i = 0; i < nNodes; ++i)
    {
      initialPositionAlloc->Add (Vector (xPos->GetValue (), yPos->GetValue (), 0.0));
    }

  mobility.SetPositionAllocator (initialPositionAlloc);

  // Configure RandomWaypointMobilityModel with proper bounds
  std::ostringstream xBounds, yBounds;
  xBounds << "ns3::UniformRandomVariable[Min=0.0|Max=" << areaSize << "]";
  yBounds << "ns3::UniformRandomVariable[Min=0.0|Max=" << areaSize << "]";

  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                             "Speed", StringValue (speedBounds.str ()),
                             "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=5.0]"),
                             "PositionAllocator", PointerValue (CreateObjectWithAttributes<RandomRectanglePositionAllocator> (
                               "X", StringValue (xBounds.str ()),
                               "Y", StringValue (yBounds.str ()))));
  mobility.Install (nodes);

  // WiFi setup for emergency communication
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("DsssRate2Mbps"),
                                "ControlMode", StringValue ("DsssRate1Mbps"));

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",
                                  "MaxRange", DoubleValue (transmissionRange));
  wifiPhy.SetChannel (wifiChannel.Create ());

  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // Energy model setup for battery-powered devices
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (initialEnergy));
  basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (3.3)); // Set voltage explicitly
  energy::EnergySourceContainer sources = basicSourceHelper.Install (nodes);

  // WiFi radio energy model - INCREASED values to drain battery faster for demo
  // Current consumption values (AMPLIFIED 50x for visible energy depletion):
  // - Tx: 0.85A (850mA) = 2.8W transmission power
  // - Rx: 0.65A (650mA) = 2.15W reception power
  // - Idle: 0.2A (200mA) = 0.66W idle power
  // - Sleep: 0.005A (5mA) = 0.0165W sleep mode
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.85));   // 850mA transmission (INCREASED)
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.65));   // 650mA reception (INCREASED)
  radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (0.2));  // 200mA idle (INCREASED)
  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.005)); // 5mA sleep mode (INCREASED)
  energy::DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (devices, sources);

  // Internet stack with energy-aware epidemic routing
  EnergyAwareEpidemicHelper energyAwareHelper;
  energyAwareHelper.SetInitialEnergy (initialEnergy);
  energyAwareHelper.SetEnergyThresholds (0.3, 0.15); // 30% low, 15% critical
  energyAwareHelper.EnableEnergyMonitoring (true);
  
  // Configure epidemic routing parameters
  energyAwareHelper.Set ("HopCount", UintegerValue (10));
  energyAwareHelper.Set ("QueueLength", UintegerValue (50));
  energyAwareHelper.Set ("QueueEntryExpireTime", TimeValue (Seconds (60)));
  energyAwareHelper.Set ("BeaconInterval", TimeValue (Seconds (5)));

  InternetStackHelper internet;
  internet.SetRoutingHelper (energyAwareHelper);
  internet.Install (nodes);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  // Emergency communication applications (voice + GPS + alerts)
  ApplicationContainer emergencyApps;
  uint16_t port = 9;
  
  // Create communication pairs for emergency voice channels
  for (uint32_t i = 0; i < numCommunicationPairs && i < nNodes; ++i)
    {
      uint32_t srcNode = i;
      uint32_t dstNode = (i + nNodes / 2) % nNodes;

      // Use OnOffApplication to transmit continuously for energy drain
      // Continuous transmission to drain battery and show color changes
      OnOffHelper onOff ("ns3::UdpSocketFactory",
                        InetSocketAddress (interfaces.GetAddress (dstNode), port));
      onOff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=60.0]"));
      onOff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=5.0]")); // Short off period
      onOff.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
      onOff.SetAttribute ("PacketSize", UintegerValue (packetSize));

      ApplicationContainer srcApp = onOff.Install (nodes.Get (srcNode));
      srcApp.Start (Seconds (5.0 + i * 2.0)); // Stagger start times
      srcApp.Stop (Seconds (simulationTime - 5.0));
      emergencyApps.Add (srcApp);

      std::cout << "Node " << srcNode << " will transmit continuously to Node " << dstNode
                << " starting at " << (5.0 + i * 2.0) << " seconds" << std::endl;
    }

  // Packet sinks for receiving emergency communications
  ApplicationContainer sinkApps;
  for (uint32_t i = 0; i < nNodes; ++i)
    {
      PacketSinkHelper sink ("ns3::UdpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer sinkApp = sink.Install (nodes.Get (i));
      sinkApp.Start (Seconds (0.0));
      sinkApp.Stop (Seconds (simulationTime));
      sinkApps.Add (sinkApp);
    }

  // Energy monitoring - More frequent monitoring to track energy depletion
  EnergyMonitor energyMonitor (nodes);
  energyMonitor.MonitorEnergy (); // Start monitoring immediately
  // Print stats every 30 seconds for more frequent updates
  for (double t = 30.0; t < simulationTime; t += 30.0)
    {
      Simulator::Schedule (Seconds (t), &EnergyMonitor::PrintEnergyStats, &energyMonitor);
    }
  Simulator::Schedule (Seconds (simulationTime - 5.0), &EnergyMonitor::PrintEnergyStats, &energyMonitor);

  // NetAnim setup for visualization
  AnimationInterface anim ("emergency-communication-network.xml");
  anim.SetMobilityPollInterval (Seconds (0.5));
  anim.EnablePacketMetadata (true);

  // Enable IPv4 route tracking for NetAnim
  // Note: Epidemic routing uses opportunistic forwarding, not traditional routing tables
  // This will show energy status and buffer information instead
  anim.EnableIpv4RouteTracking ("emergency-routing-table.xml", Seconds (0),
                                Seconds (simulationTime), Seconds (30.0)); // Sample every 30s

  // Set initial node colors: Green for rescue workers (full energy)
  // Node size: Larger to represent rescue workers with equipment
  for (uint32_t i = 0; i < nNodes; ++i)
    {
      anim.UpdateNodeColor (i, 0, 255, 0); // Green - full energy
      anim.UpdateNodeDescription (i, "Rescue Worker " + std::to_string(i) + " [100%]");
      anim.UpdateNodeSize (i, 5.0, 5.0); // Larger nodes for better visibility
    }

  // Connect animation interface to energy monitor for dynamic color updates
  energyMonitor.SetAnimationInterface (&anim);

  // Create additional human-readable routing information log
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> routingStream = asciiTraceHelper.CreateFileStream ("epidemic-routing-info.txt");
  *routingStream->GetStream () << "=================================================================\n";
  *routingStream->GetStream () << "   ENERGY-AWARE EPIDEMIC ROUTING - PROTOCOL STATUS LOG\n";
  *routingStream->GetStream () << "=================================================================\n";
  *routingStream->GetStream () << "Simulation Time: " << simulationTime << " seconds\n";
  *routingStream->GetStream () << "Number of Nodes: " << nNodes << "\n";
  *routingStream->GetStream () << "Initial Energy: " << initialEnergy << " Joules\n";
  *routingStream->GetStream () << "=================================================================\n\n";

  // Schedule periodic routing table dumps (every 60 seconds for readability)
  for (double t = 60.0; t <= simulationTime; t += 60.0)
    {
      Simulator::Schedule (Seconds (t), &Ipv4RoutingHelper::PrintRoutingTableAllAt,
                          Seconds (t), routingStream, Time::Unit::S);
    }

  // Connect PDR traces
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&RxTrace));
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeCallback (&TxTrace));

  // Schedule PDR printing
  Simulator::Schedule (Seconds (10.0), &PrintPdr);

  // Simulation execution
  std::cout << "\nStarting Emergency Communication Network simulation..." << std::endl;
  std::cout << "NetAnim file: emergency-communication-network.xml" << std::endl;
  std::cout << "Routing table XML: emergency-routing-table.xml" << std::endl;
  std::cout << "Routing info log: epidemic-routing-info.txt\n" << std::endl;

  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  // Final statistics
  std::cout << "\n=== Communication Statistics ===" << std::endl;
  uint64_t totalBytesReceived = 0;
  for (uint32_t i = 0; i < sinkApps.GetN (); ++i)
    {
      Ptr<PacketSink> sink = DynamicCast<PacketSink> (sinkApps.Get (i));
      if (sink)
        {
          uint64_t bytes = sink->GetTotalRx ();
          totalBytesReceived += bytes;
          if (bytes > 0)
            {
              std::cout << "Rescue Worker " << i << " received: " 
                        << bytes << " bytes (" 
                        << (bytes / 1024.0) << " KB)" << std::endl;
            }
        }
    }
  std::cout << "Total data received: " << totalBytesReceived << " bytes (" 
            << (totalBytesReceived / 1024.0) << " KB)" << std::endl;

  Simulator::Destroy ();
  
  std::cout << "\n================================================" << std::endl;
  std::cout << "  Simulation completed successfully!" << std::endl;
  std::cout << "================================================\n" << std::endl;
  std::cout << "To visualize, open: emergency-communication-network.xml in NetAnim" << std::endl;
  
  return 0;
}
