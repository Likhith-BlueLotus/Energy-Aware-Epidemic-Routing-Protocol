/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Energy-Aware Epidemic Helper Implementation
 */

#include "energy-aware-epidemic-helper.h"
#include "ns3/energy-aware-epidemic-routing.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/energy-source-container.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EnergyAwareEpidemicHelper");

EnergyAwareEpidemicHelper::EnergyAwareEpidemicHelper ()
  : Ipv4RoutingHelper (),
    m_initialEnergy (1000.0),
    m_harvestingRate (0.0),
    m_energyMonitoring (false),
    m_lowThreshold (0.2),
    m_criticalThreshold (0.1)
{
  NS_LOG_FUNCTION (this);
  // Set the factory to create EnergyAwareRoutingProtocol
  m_agentFactory.SetTypeId ("ns3::Epidemic::EnergyAwareRoutingProtocol");
}

EnergyAwareEpidemicHelper::~EnergyAwareEpidemicHelper ()
{
  NS_LOG_FUNCTION (this);
}

EnergyAwareEpidemicHelper*
EnergyAwareEpidemicHelper::Copy (void) const
{
  NS_LOG_FUNCTION (this);
  return new EnergyAwareEpidemicHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
EnergyAwareEpidemicHelper::Create (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  // Create the energy-aware routing protocol
  Ptr<Epidemic::EnergyAwareRoutingProtocol> agent =
    m_agentFactory.Create<Epidemic::EnergyAwareRoutingProtocol> ();

  // Get existing energy source from node (should be installed by example code)
  // Energy sources are stored in an EnergySourceContainer aggregated to the node
  Ptr<energy::EnergySourceContainer> energySourceContainer = node->GetObject<energy::EnergySourceContainer> ();
  if (energySourceContainer && energySourceContainer->GetN () > 0)
    {
      // Get the first energy source from the container
      Ptr<energy::EnergySource> energySource = energySourceContainer->Get (0);
      NS_LOG_DEBUG ("Found existing energy source on node " << node->GetId ());
      // Set energy source in the routing protocol
      agent->SetEnergySource (energySource);
    }
  else
    {
      NS_LOG_WARN ("No energy source found on node " << node->GetId ());
    }

  // Set energy thresholds
  agent->SetAttribute ("EnergyThresholdLow", DoubleValue (m_lowThreshold));
  agent->SetAttribute ("EnergyThresholdCritical", DoubleValue (m_criticalThreshold));

  node->AggregateObject (agent);
  return agent;
}

void
EnergyAwareEpidemicHelper::SetInitialEnergy (double energy)
{
  NS_LOG_FUNCTION (this << energy);
  m_initialEnergy = energy;
}

void
EnergyAwareEpidemicHelper::SetEnergyHarvestingRate (double rate)
{
  NS_LOG_FUNCTION (this << rate);
  m_harvestingRate = rate;
}

void
EnergyAwareEpidemicHelper::InstallWithEnergy (NodeContainer nodes)
{
  NS_LOG_FUNCTION (this);
  
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i)
    {
      Ptr<Node> node = *i;
      
      // Install energy source
      Ptr<energy::EnergySource> energySource = InstallEnergySource (node);
      
      // Create and install routing protocol
      Ptr<Ipv4RoutingProtocol> protocol = Create (node);
      
      // Note: Energy monitoring is handled via periodic checks in the example code
      // EnergySource doesn't provide direct callback interfaces - monitoring is done
      // by querying energy levels periodically
    }
}

void
EnergyAwareEpidemicHelper::EnableEnergyMonitoring (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_energyMonitoring = enable;
}

void
EnergyAwareEpidemicHelper::SetEnergyThresholds (double lowThreshold, double criticalThreshold)
{
  NS_LOG_FUNCTION (this << lowThreshold << criticalThreshold);
  m_lowThreshold = lowThreshold;
  m_criticalThreshold = criticalThreshold;
}

void
EnergyAwareEpidemicHelper::Set (std::string name, const AttributeValue &value)
{
  NS_LOG_FUNCTION (this << name);
  m_agentFactory.Set (name, value);
}

Ptr<energy::EnergySource>
EnergyAwareEpidemicHelper::InstallEnergySource (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  
  // Create basic energy source
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (m_initialEnergy));
  
  if (m_harvestingRate > 0.0)
    {
      basicSourceHelper.Set ("BasicEnergyHarvestingCurrentA", DoubleValue (m_harvestingRate / 3.3));
    }
  
  energy::EnergySourceContainer sources = basicSourceHelper.Install (node);
  return sources.Get (0);
}

void
EnergyAwareEpidemicHelper::EnergyDepletionCallback (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  NS_LOG_WARN ("Node " << node->GetId () << " energy depleted at time " 
               << Simulator::Now ().GetSeconds () << " seconds");
}

void
EnergyAwareEpidemicHelper::EnergyRechargeCallback (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  NS_LOG_INFO ("Node " << node->GetId () << " energy recharged at time " 
               << Simulator::Now ().GetSeconds () << " seconds");
}

} // namespace ns3
