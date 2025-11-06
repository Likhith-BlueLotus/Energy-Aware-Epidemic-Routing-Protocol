.. include:: replace.txt
.. highlight:: cpp

Epidemic Routing
----------------

Epidemic Routing protocol is a controlled flooding routing protocol
designed specifically for use in multi-hop wireless ad hoc networks
of mobile nodes. This routing protocol is designed for intermittent 
or sparse networks. Packets are held in the source nodes' buffer until
the source node comes into communication range with another node.  
The packets are transmitted to the new node, which becomes a transit 
node for those packets. Transit nodes hold all packets received until 
they come into range with other nodes, at which point they each send 
the other any packets the other doesn't already have.  The name "epidemic" 
is drawn from the way packets spread virally by communication
contact between nodes.  At the source and all transit nodes, packets are
eventually dropped when the expire or the epidemic routing buffer 
becomes full, in which case the oldest packets are dropped. It is 
not useful for dense networks since the number of duplicate packets 
will be very large.

This model was developed by 
the `ResiliNets research group <http://www.ittc.ku.edu/resilinets>`_
at the University of Kansas.  

**Energy-Aware Extension**: This implementation includes an energy-aware 
extension that optimizes routing decisions based on battery levels, making 
it suitable for energy-constrained mobile ad hoc networks (MANETs) with 
multimedia traffic such as speech and video.  


Epidemic Routing Overview
*************************

This implementation is based on the paper titled 'Epidemic Routing
for Partially-Connected Ad Hoc Networks.'[#Vahdat]_

In the original paper, the implementation was on top of top of
the Internet MANET Encapsulation Protocol (IMEP) layer,
which is  responsible for notifying the epidemic agent
when a new node comes into radio range. However, since IMEP
is not implemented in |ns3|, a beacon mechanism is added to the implementation.

Architecture and Components
===========================

The epidemic routing module consists of several key components:

**Core Model Classes:**

* ``RoutingProtocol``: The main epidemic routing protocol implementation
* ``EnergyAwareRoutingProtocol``: Energy-aware extension with battery management
* ``PacketQueue``: Buffer management for epidemic packets
* ``QueueEntry``: Individual packet entries in the epidemic buffer

**Packet Headers:**

* ``EpidemicHeader``: Main routing header with packet ID, hop count, and timestamp
* ``TypeHeader``: Control packet type identification (BEACON, REPLY, REPLY_BACK)
* ``SummaryVectorHeader``: Summary vector for anti-entropy sessions
* ``ControlTag``: Packet tagging for control vs. data packets

**Helper Classes:**

* ``EpidemicHelper``: Basic epidemic routing setup and configuration
* ``EnergyAwareEpidemicHelper``: Energy-aware routing with battery integration

Energy-Aware Features
=====================

The energy-aware extension provides several advanced features:

**Battery Management:**
  - Real-time energy monitoring and reporting
  - Adaptive behavior based on remaining battery levels
  - Energy depletion and recharge callbacks

**Adaptive Routing:**
  - Dynamic beacon interval adjustment based on energy levels
  - Energy-aware packet forwarding decisions
  - Reduced flooding when battery is low

**Multimedia Optimization:**
  - Priority handling for speech packets
  - Dynamic compression for multimedia content
  - Energy-efficient multimedia packet processing

**Smart Resource Management:**
  - Adaptive hop count limits based on energy
  - Energy cost calculation for transmissions
  - Intelligent packet dropping policies


Configuration Parameters
========================

Basic Epidemic Routing Parameters
----------------------------------

Epidemic routing supports these core options:

  +-----------------------+-----------------------------------+---------------+
  | Parameter             | Description                       | Default       |
  +=======================+===================================+===============+
  | HopCount              | Maximum number of hops a packet   | 64            |
  |                       | can be forwarded through.         |               | 
  |                       | HopCount serves a similar         |               |
  |                       | function to TTL, but the 8-bit    |               |
  |                       | range of TTL is too small, so we  |               |
  |                       | use a 32-bit field as in          |               |
  |                       | the paper.                        |               |
  +-----------------------+-----------------------------------+---------------+
  | QueueLength           | Maximum number of packets that    | 64            |
  |                       | can be stored in Epidemic buffer  |               |
  +-----------------------+-----------------------------------+---------------+
  | QueueEntryExpireTime  | Maximum time a packet can live    |               |
  |                       | since generated at the source.    | Seconds(100)  |
  |                       | Network-wide synchronization      |               |
  |                       | is assumed.                       |               |
  +-----------------------+-----------------------------------+---------------+
  | HostRecentPeriod      | Time in seconds for host recent   |               |
  |                       | period, in which hosts can not    | Seconds(10)   |
  |                       | re-exchange summary vectors.      |               |
  +-----------------------+-----------------------------------+---------------+
  | BeaconInterval        | Mean time interval between sending| Seconds(1)    |
  |                       | beacon packets.                   |               |
  +-----------------------+-----------------------------------+---------------+
  | BeaconRandomness      | Random number of milliseconds     | 100           |
  |                       | added at the beginning            |               |
  |                       | of the BeaconInterval to avoid    |               |
  |                       | collisions.                       |               |
  +-----------------------+-----------------------------------+---------------+

Energy-Aware Parameters
-----------------------

Additional parameters for energy-aware epidemic routing:

  +---------------------------+-----------------------------------+---------------+
  | Parameter                 | Description                       | Default       |
  +===========================+===================================+===============+
  | EnergyThresholdLow        | Low energy threshold as ratio     | 0.2 (20%)     |
  |                           | (0.0-1.0). Below this threshold,  |               |
  |                           | adaptive flooding is enabled.     |               |
  +---------------------------+-----------------------------------+---------------+
  | EnergyThresholdCritical   | Critical energy threshold as      | 0.1 (10%)     |
  |                           | ratio (0.0-1.0). Below this      |               |
  |                           | threshold, only high priority     |               |
  |                           | packets are forwarded.            |               |
  +---------------------------+-----------------------------------+---------------+
  | EnergyAwareFloodingFactor | Factor to reduce flooding when    | 0.5           |
  |                           | energy is low (0.1-1.0).         |               |
  +---------------------------+-----------------------------------+---------------+
  | SpeechPacketPriority      | Priority level for speech packets| 10            |
  |                           | (higher = more priority).         |               |
  +---------------------------+-----------------------------------+---------------+
  | CompressionRatio          | Dynamic compression ratio for     | 0.8           |
  |                           | multimedia packets (0.1-1.0).    |               |
  +---------------------------+-----------------------------------+---------------+


Dropping Packets
================
Packets, stored in buffers, are dropped if they exceed HopCount, they are
older than QueueEntryExpireTime, or the holding buffer exceed QueueLength.  


Helper Classes
**************

Basic Epidemic Helper
=====================

To have a node run basic Epidemic Routing Protocol, the easiest way would be to use 
the EpidemicHelper in your simulation script. For instance (assuming ``adhocNodes`` 
is a ``NodeContainer``)::

  EpidemicHelper epidemic;
  InternetStackHelper internet;
  internet.SetRoutingHelper (epidemic);
  internet.Install (adhocNodes);

This will run the epidemic routing using the default values. To use 
different parameter values::

  EpidemicHelper epidemic;
  epidemic.Set ("HopCount", UintegerValue (20));
  epidemic.Set ("QueueLength", UintegerValue (100));
  epidemic.Set ("QueueEntryExpireTime", TimeValue (Seconds (60)));
  epidemic.Set ("BeaconInterval", TimeValue (Seconds (5)));
  
  InternetStackHelper internet;
  internet.SetRoutingHelper (epidemic);
  internet.Install (adhocNodes);

Energy-Aware Epidemic Helper
============================

For energy-constrained scenarios with battery management, use the 
EnergyAwareEpidemicHelper::

  EnergyAwareEpidemicHelper energyEpidemic;
  energyEpidemic.SetInitialEnergy (1000.0); // 1000 Joules per node
  energyEpidemic.SetEnergyThresholds (0.3, 0.15); // Low: 30%, Critical: 15%
  energyEpidemic.EnableEnergyMonitoring (true);
  
  // Set epidemic parameters
  energyEpidemic.Set ("HopCount", UintegerValue (15));
  energyEpidemic.Set ("EnergyAwareFloodingFactor", DoubleValue (0.6));
  energyEpidemic.Set ("SpeechPacketPriority", UintegerValue (8));
  
  // Install with energy management
  energyEpidemic.InstallWithEnergy (adhocNodes);

Advanced Energy Configuration
=============================

For more complex energy scenarios with harvesting::

  EnergyAwareEpidemicHelper energyEpidemic;
  energyEpidemic.SetInitialEnergy (500.0);
  energyEpidemic.SetEnergyHarvestingRate (0.1); // 0.1 Watts harvesting
  energyEpidemic.SetEnergyThresholds (0.25, 0.1);
  
  // Configure multimedia optimization
  energyEpidemic.Set ("CompressionRatio", DoubleValue (0.7));
  energyEpidemic.Set ("SpeechPacketPriority", UintegerValue (12));
  
  energyEpidemic.InstallWithEnergy (adhocNodes);


Examples
********

The epidemic routing module includes three comprehensive examples:

Basic Epidemic Example
======================

``epidemic-example.cc`` creates an N-node wireless network (default: 10 nodes). 
The mobility model can be either static Grid or RandomWaypoint (default: Grid).
The data traffic is generated using OnOff application and received by PacketSink. 
There is one source and one sink in this configuration. The example runs for 
100 seconds with data transmission from 10-15 seconds.

Key features:
  - Configurable number of nodes and mobility models
  - Single source-destination pair
  - Adjustable transmission range and node speed
  - Basic epidemic routing parameters

Usage::

  ./ns3 run "epidemic-example --nWifis=15 --txpDistance=150 --nodeSpeed=20"

Benchmark Example
=================

``epidemic-benchmark.cc`` recreates the scenarios from the original epidemic 
routing paper [#Vahdat]_. It uses 50 nodes in a 1500m x 300m area, where 
45 nodes send 1KB packets to all other 44 nodes (total: 1980 packets). 
The buffer size is set to 2000 to accommodate all packets.

Key features:
  - Large-scale scenario (50 nodes)
  - Multiple source-destination pairs (45x44)
  - Configurable transmission ranges (10m-250m, default: 50m)
  - Performance benchmarking capabilities

Usage::

  ./ns3 run "epidemic-benchmark --txpDistance=100 --hopCount=30 --queueLength=150"

Energy-Aware Example
====================

``energy-aware-epidemic-example.cc`` demonstrates energy-constrained MANET 
scenarios with multimedia traffic. It includes battery management, speech 
applications, and energy monitoring for realistic mobile scenarios.

Key features:
  - Energy-aware epidemic routing with battery models
  - Speech/multimedia traffic simulation
  - Real-time energy monitoring and reporting
  - Adaptive behavior based on battery levels
  - WiFi radio energy consumption modeling

Usage::

  ./ns3 run "energy-aware-epidemic-example --nNodes=25 --initialEnergy=1500 --speechSources=5"

The example includes:
  - ``EnergyMonitor`` class for battery tracking
  - ``SpeechApplication`` for multimedia traffic
  - Comprehensive energy consumption reporting
  - Configurable energy thresholds and harvesting


Validation and Testing
**********************

This model has been comprehensively tested as follows:

Unit Tests
==========

* **EpidemicHeaderTestCase**: Validates the EpidemicHeader serialization, 
  deserialization, and field access methods. Tests packet ID, timestamp, 
  and hop count functionality.

* **EpidemicRqueueTest**: Comprehensive testing of the PacketQueue class 
  including enqueue/dequeue operations, duplicate handling, size limits, 
  and queue management.

The complete test suite can be found in ``test/epidemic-test-suite.cc`` and 
can be run using::

  ./ns3 run test-runner --suite=epidemic

Test Coverage
=============

The test suite covers:
  - Packet header serialization/deserialization
  - Queue operations and buffer management
  - Duplicate packet detection
  - Queue size enforcement and overflow handling
  - Packet expiration and cleanup

Integration Testing
===================

The examples serve as integration tests:
  - Basic functionality validation through ``epidemic-example.cc``
  - Performance benchmarking via ``epidemic-benchmark.cc``
  - Energy-aware features testing with ``energy-aware-epidemic-example.cc``

Model Validation
================

The implementation has been validated against:
  - Original epidemic routing paper specifications [#Vahdat]_
  - Expected packet delivery ratios in sparse networks
  - Energy consumption patterns in battery-constrained scenarios
  - Multimedia traffic handling and prioritization

Limitations and Known Issues
****************************

Basic Epidemic Routing Limitations
===================================

* **Single Address per Interface**: Epidemic does not work with more than one 
  address per interface. Additional addresses are ignored.

* **Beacon Dependency**: The protocol assumes many beacons are exchanged while 
  nodes are in communication range. If beacon frequency is too low relative to 
  contact time, packets may not be delivered.

* **Dense Network Performance**: Not suitable for dense networks due to excessive 
  duplicate packet flooding, which can lead to network congestion.

* **Memory Requirements**: Buffer requirements grow with network size and traffic 
  load, potentially causing memory issues in resource-constrained scenarios.

Energy-Aware Extension Limitations
===================================

* **Energy Model Dependency**: Energy-aware features require proper energy source 
  configuration. Without energy models, the protocol falls back to basic epidemic 
  behavior.

* **Simplified Energy Calculations**: Current energy cost calculations are 
  simplified and may not reflect real-world radio energy consumption patterns.

* **Speech Packet Detection**: The current implementation uses simplified heuristics 
  for speech packet identification. More sophisticated content analysis may be needed 
  for production use.

* **Synchronization Requirements**: Energy thresholds assume some level of network-wide 
  time synchronization for consistent behavior across nodes.

Performance Considerations
==========================

* **Scalability**: Performance degrades significantly with network size due to the 
  flooding nature of epidemic routing.

* **Battery Life**: In energy-constrained scenarios, aggressive flooding can quickly 
  deplete battery reserves without proper threshold management.

* **Multimedia Traffic**: Large multimedia packets can overwhelm buffers and network 
  capacity, requiring careful traffic shaping and compression.

API Reference and Class Hierarchy
**********************************

Core Classes
============

**ns3::Epidemic::RoutingProtocol**
  Base epidemic routing protocol implementation. Inherits from Ipv4RoutingProtocol.
  
  Key Methods:
    - ``RouteInput()``: Process incoming packets
    - ``RouteOutput()``: Handle outgoing packets  
    - ``SendBeacons()``: Periodic beacon transmission
    - ``RecvEpidemic()``: Process epidemic control packets

**ns3::Epidemic::EnergyAwareRoutingProtocol**
  Energy-aware extension of the basic protocol. Inherits from RoutingProtocol.
  
  Key Methods:
    - ``SetEnergySource()``: Associate energy source with protocol
    - ``GetRemainingEnergyRatio()``: Get current energy level
    - ``ShouldForwardPacket()``: Energy-aware forwarding decision
    - ``AdaptBeaconInterval()``: Adjust beacon frequency based on energy

Packet Management
=================

**ns3::Epidemic::PacketQueue**
  Manages epidemic packet buffer with expiration and size limits.
  
**ns3::Epidemic::QueueEntry**  
  Individual packet entry with metadata (expiration, callbacks, packet ID).

Headers and Tags
================

**ns3::Epidemic::EpidemicHeader**
  Main routing header containing packet ID, hop count, and timestamp.
  
**ns3::Epidemic::TypeHeader**
  Control packet type identification (BEACON, REPLY, REPLY_BACK).
  
**ns3::Epidemic::SummaryVectorHeader**
  Contains list of packet IDs for anti-entropy sessions.
  
**ns3::Epidemic::ControlTag**
  Packet tag to distinguish control vs. data packets.

Helper Classes
==============

**ns3::EpidemicHelper**
  Basic helper for epidemic routing setup and configuration.
  
**ns3::EnergyAwareEpidemicHelper**
  Advanced helper with energy management and battery integration.

Usage Patterns
==============

1. **Basic Setup**: Use EpidemicHelper for simple scenarios
2. **Energy-Aware**: Use EnergyAwareEpidemicHelper for battery-constrained networks
3. **Multimedia**: Configure speech packet priorities and compression ratios
4. **Monitoring**: Enable energy monitoring for performance analysis

References
**********

.. rubric:: Footnotes

.. [#Vahdat] Amin Vahdat and David Becker, "Epidemic Routing for
   Partially-Connected Ad Hoc Networks," Duke University, Technical
   Report CS-200006, http://issg.cs.duke.edu/epidemic/epidemic.pdf
