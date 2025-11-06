/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Energy-Aware Epidemic Helper
 */

#ifndef ENERGY_AWARE_EPIDEMIC_HELPER_H
#define ENERGY_AWARE_EPIDEMIC_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/energy-module.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3 {

/**
 * \brief Helper class for Energy-Aware Epidemic Routing
 * 
 * This helper extends the basic EpidemicHelper with energy management:
 * - Automatic energy source setup
 * - Battery model integration
 * - Energy monitoring and reporting
 */
class EnergyAwareEpidemicHelper : public Ipv4RoutingHelper
{
public:
  EnergyAwareEpidemicHelper ();
  ~EnergyAwareEpidemicHelper ();

  // Inherited
  EnergyAwareEpidemicHelper* Copy (void) const override;
  Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const override;

  /**
   * \brief Set initial energy for all nodes
   * \param energy Initial energy in Joules
   */
  void SetInitialEnergy (double energy);

  /**
   * \brief Set energy harvesting rate
   * \param rate Energy harvesting rate in Watts
   */
  void SetEnergyHarvestingRate (double rate);

  /**
   * \brief Install energy sources and routing protocol
   * \param nodes Container of nodes to install on
   */
  void InstallWithEnergy (NodeContainer nodes);

  /**
   * \brief Enable energy monitoring and logging
   * \param enable True to enable energy monitoring
   */
  void EnableEnergyMonitoring (bool enable);

  /**
   * \brief Set energy thresholds
   * \param lowThreshold Low energy threshold (0.0-1.0)
   * \param criticalThreshold Critical energy threshold (0.0-1.0)
   */
  void SetEnergyThresholds (double lowThreshold, double criticalThreshold);

  /**
   * \brief Set attributes by name
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void Set (std::string name, const AttributeValue &value);

private:
  ObjectFactory m_agentFactory;  ///< The factory to create routing objects
  double m_initialEnergy;        ///< Initial energy per node (Joules)
  double m_harvestingRate;       ///< Energy harvesting rate (Watts)
  bool m_energyMonitoring;       ///< Enable energy monitoring
  double m_lowThreshold;         ///< Low energy threshold
  double m_criticalThreshold;    ///< Critical energy threshold

  /**
   * \brief Install energy source on a node
   * \param node Node to install energy source on
   * \return Pointer to installed energy source
   */
  Ptr<energy::EnergySource> InstallEnergySource (Ptr<Node> node) const;

  /**
   * \brief Energy depletion callback
   * \param node Node that depleted energy
   */
  void EnergyDepletionCallback (Ptr<Node> node);

  /**
   * \brief Energy recharge callback
   * \param node Node that recharged energy
   */
  void EnergyRechargeCallback (Ptr<Node> node);
};

} // namespace ns3

#endif /* ENERGY_AWARE_EPIDEMIC_HELPER_H */
