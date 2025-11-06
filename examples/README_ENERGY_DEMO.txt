================================================================================
ENERGY-AWARE EPIDEMIC ROUTING - DEMO CONFIGURATION
How to See All Energy Phases (Green → Yellow → Orange → Red → Gray)
================================================================================

CHANGES MADE TO SEE VISIBLE ENERGY DEPLETION:
============================================

1. SIMULATION TIME: Increased from 300s to 600s (10 minutes)
   - Reason: More time to observe energy depletion through all phases

2. INITIAL ENERGY: Reduced from 5000J to 1000J
   - Reason: Faster battery depletion to see color changes

3. ENERGY CONSUMPTION: Amplified radio power by 50x
   - TX Current: 0.017A → 0.85A (850mA)
   - RX Current: 0.013A → 0.65A (650mA)
   - Idle Current: 0.004A → 0.2A (200mA)
   - Reason: Realistic values deplete too slowly for demo purposes

4. DATA RATE: Increased from 64kbps to 128kbps
   - Reason: More packets per second = more energy consumption

5. COMMUNICATION PAIRS: Increased from 5 to 10 pairs
   - Reason: More active transmitters = faster network-wide depletion

6. TRANSMISSION PATTERN: Continuous with short breaks
   - OnTime: 60 seconds
   - OffTime: 5 seconds
   - Reason: Sustained traffic to drain batteries consistently

7. ENERGY REPORTS: Every 30 seconds instead of 60 seconds
   - Reason: More frequent updates to track energy phases


EXPECTED ENERGY DEPLETION TIMELINE:
==================================

Time 0-120s (0-2 min):
  - Color: GREEN (100% → 70%)
  - Status: All nodes operational, full performance
  - Beacon interval: 5 seconds (normal)
  - Forwarding: 100% of packets

Time 120-240s (2-4 min):
  - Color: GREEN → YELLOW (70% → 40%)
  - Status: Nodes transitioning to low energy
  - Beacon interval: Starting to increase on some nodes
  - Forwarding: Beginning to reduce on low-energy nodes

Time 240-360s (4-6 min):
  - Color: YELLOW → ORANGE (40% → 20%)
  - Status: Low energy mode active
  - Beacon interval: 10 seconds (doubled)
  - Forwarding: Probabilistic (50% chance)

Time 360-480s (6-8 min):
  - Color: ORANGE → RED (20% → 5%)
  - Status: Critical energy mode
  - Beacon interval: 20 seconds (quadrupled)
  - Forwarding: Priority packets only (~10%)

Time 480-600s (8-10 min):
  - Color: RED → GRAY (5% → 0%)
  - Status: Some nodes depleting completely
  - Beacon interval: Very sparse or stopped
  - Forwarding: Minimal to none
  - First nodes going GRAY (depleted)


ENERGY PHASE COLOR CODING:
==========================

Energy Level    Color    RGB Values       Beacon Interval    Forwarding
-----------------------------------------------------------------------------
100% - 70%     GREEN    (0-85, 255, 0)   5s (normal)       100%
70%  - 40%     YELLOW   (255, 0-255, 0)  5-10s (adaptive)  100-50%
40%  - 20%     ORANGE   (255, 0-128, 0)  10-20s (reduced)  50-10%
20%  - 1%      RED      (255, 0, 0)      20s (critical)    <10%
0%             GRAY     (128, 128, 128)  No beacons        No forwarding


HOW TO RUN THE ENHANCED SIMULATION:
===================================

1. BUILD THE PROJECT:
   cd /d/BMSCE/Seventh_SEM_CSE/NP/NS3/ns-allinone-3.45/ns-3.45
   ./ns3 build

2. RUN WITH DEFAULT ENHANCED PARAMETERS:
   ./ns3 run energy-aware-epidemic-example

3. RUN WITH LOGGING (to see protocol behavior):
   NS_LOG="EnergyAwareEpidemicRouting=level_info" ./ns3 run energy-aware-epidemic-example

4. RUN WITH DETAILED LOGGING (all components):
   NS_LOG="EnergyAwareEpidemicRouting=level_all:EnergyAwareEpidemicHelper=level_info" \
   ./ns3 run energy-aware-epidemic-example

5. RUN WITH CUSTOM PARAMETERS:
   ./ns3 run "energy-aware-epidemic-example --nNodes=20 --simulationTime=600 --initialEnergy=1000"

6. VISUALIZE IN NETANIM:
   - Open NetAnim application
   - File → Open → Select: emergency-communication-network.xml
   - Press Play button
   - Watch nodes change colors as energy depletes


WHAT YOU SHOULD SEE IN CONSOLE OUTPUT:
======================================

Starting output shows configuration:
-----------------------------------
================================================
  EMERGENCY COMMUNICATION NETWORK SIMULATION
================================================
Scenario: Disaster-affected area communication
Rescue Workers: 20
Simulation Duration: 600 seconds  <-- INCREASED
Area Coverage: 500m × 500m
Initial Battery: 1000 Joules per device  <-- REDUCED
Transmission Range: 100 meters
Communication Channels: 10  <-- INCREASED
================================================

Energy reports every 30 seconds showing depletion:
-------------------------------------------------

Time: 30 seconds
Rescue Worker 0: Energy=950.2J (95.0%), Status=OPERATIONAL  <-- GREEN
...
Network Energy Efficiency: 95.0%

Time: 60 seconds
Rescue Worker 0: Energy=898.5J (89.9%), Status=OPERATIONAL  <-- GREEN
...
Network Energy Efficiency: 89.8%

Time: 120 seconds
Rescue Worker 0: Energy=750.3J (75.0%), Status=OPERATIONAL  <-- GREEN
Rescue Worker 2: Energy=685.7J (68.6%), Status=OPERATIONAL  <-- YELLOW (transitioning)
...
Network Energy Efficiency: 72.3%

Time: 180 seconds
Rescue Worker 0: Energy=587.2J (58.7%), Status=OPERATIONAL  <-- YELLOW
Rescue Worker 2: Energy=512.8J (51.3%), Status=OPERATIONAL  <-- YELLOW
...
Network Energy Efficiency: 55.4%

Time: 240 seconds
Rescue Worker 0: Energy=387.5J (38.8%), Status=OPERATIONAL  <-- ORANGE (transitioning)
Rescue Worker 2: Energy=298.3J (29.8%), Status=OPERATIONAL  <-- ORANGE
...
Network Energy Efficiency: 35.2%

Time: 300 seconds
Rescue Worker 0: Energy=234.6J (23.5%), Status=OPERATIONAL  <-- ORANGE
Rescue Worker 2: Energy=167.8J (16.8%), Status=OPERATIONAL  <-- RED (transitioning)
...
Network Energy Efficiency: 20.8%

Time: 360 seconds
Rescue Worker 0: Energy=112.3J (11.2%), Status=OPERATIONAL  <-- RED
Rescue Worker 2: Energy=45.7J (4.6%), Status=OPERATIONAL   <-- RED (critical)
...
Network Energy Efficiency: 9.7%

Time: 420 seconds
Rescue Worker 0: Energy=34.8J (3.5%), Status=OPERATIONAL   <-- RED (critical)
Rescue Worker 2: Energy=0.0J (0.0%), Status=DEPLETED      <-- GRAY (first depleted!)
Rescue Worker 5: Energy=12.3J (1.2%), Status=OPERATIONAL  <-- RED (critical)
...
Network Status:
  Nodes Operational: 18/20  <-- Some nodes depleted!
  Network Energy Efficiency: 3.2%

Time: 480 seconds
Rescue Worker 0: Energy=0.0J (0.0%), Status=DEPLETED      <-- GRAY
Rescue Worker 2: Energy=0.0J (0.0%), Status=DEPLETED      <-- GRAY
Rescue Worker 4: Energy=0.0J (0.0%), Status=DEPLETED      <-- GRAY
Rescue Worker 5: Energy=0.0J (0.0%), Status=DEPLETED      <-- GRAY
...
Network Status:
  Nodes Operational: 12/20  <-- Multiple depleted!


NETANIM VISUALIZATION FEATURES TO OBSERVE:
==========================================

1. NODE COLOR CHANGES:
   - Watch nodes gradually change from green → yellow → orange → red → gray
   - High-traffic nodes (transmitters) deplete faster
   - Nodes with more neighbors (relay nodes) deplete faster
   - Isolated nodes retain energy longer

2. MOBILITY PATTERNS:
   - Nodes move randomly within 500m × 500m area
   - Speed varies between 5-15 m/s
   - Nodes pause for 5 seconds at each waypoint
   - No overlap at origin (fixed positioning issue)

3. PACKET TRANSMISSION:
   - See packets as animated lines between nodes
   - Packet flow reduces as energy depletes
   - RED/GRAY nodes transmit very few or no packets
   - GREEN nodes transmit actively

4. NETWORK PARTITIONING:
   - Some nodes become isolated as others deplete
   - Communication patterns change as network fragments
   - Critical nodes (high connectivity) deplete first

5. NODE DESCRIPTIONS:
   - Hover over nodes to see current energy percentage
   - Format: "Rescue Worker X [Y%]"
   - Updates in real-time during playback


INTERPRETING THE RESULTS:
=========================

GOOD INDICATORS (Working Correctly):
- Energy depletes gradually over time (not constant 99.99%)
- Different nodes deplete at different rates
- Sender nodes deplete faster than receivers
- All color phases appear: Green → Yellow → Orange → Red → Gray
- Network efficiency drops from 100% to <10%
- Some nodes reach 0.0J (depleted) before simulation ends
- Packet delivery decreases as energy depletes

BAD INDICATORS (Something Wrong):
- Energy stays at 99.99% throughout simulation
- All nodes have identical energy levels
- No color changes in NetAnim
- Only 1-2 nodes receiving packets
- Network efficiency stays >95%
- No nodes reach low energy thresholds


ADJUSTING PARAMETERS FOR DIFFERENT SCENARIOS:
============================================

To see FASTER energy depletion:
- Reduce initialEnergy (e.g., 500J)
- Increase TxCurrentA and RxCurrentA (e.g., 1.5A)
- Increase dataRate (e.g., 256kbps)
- Increase numCommunicationPairs (e.g., 15)
- Reduce simulationTime if depleting too fast

To see SLOWER energy depletion (more realistic):
- Increase initialEnergy (e.g., 5000J)
- Reduce TxCurrentA to realistic values (0.017A)
- Decrease dataRate (e.g., 64kbps)
- Decrease numCommunicationPairs (e.g., 5)
- Increase simulationTime to see full depletion

To emphasize ADAPTIVE BEHAVIOR:
- Set thresholds higher (0.5 for low, 0.3 for critical)
- Increase beacon interval (10s base instead of 5s)
- Enable logging to see adaptation messages
- Watch beacon intervals increase as energy drops


TROUBLESHOOTING:
===============

PROBLEM: Energy not depleting
SOLUTION:
- Check that applications are actually running (look for "will transmit" messages)
- Verify WiFi devices are installed correctly
- Ensure energy models are attached to devices
- Check that simulation time is long enough

PROBLEM: All nodes depleting at same rate
SOLUTION:
- Verify different nodes have different traffic loads
- Check that some nodes are senders, others receivers
- Ensure mobility creates varying connectivity patterns

PROBLEM: No packets being delivered
SOLUTION:
- This is expected with epidemic routing if nodes never meet
- Check transmission range (100m) vs. area size (500m)
- Verify mobility is working (nodes moving in NetAnim)
- Epidemic routing requires opportunistic contacts

PROBLEM: Can't see color changes in NetAnim
SOLUTION:
- Verify energyMonitor.SetAnimationInterface(&anim) is called
- Check that UpdateNodeColors() is being scheduled
- Make sure simulation ran completely before opening XML
- Try playing simulation slowly to see gradual changes


LOGGING OPTIONS TO DEBUG:
=========================

Enable specific component logging:

1. See routing protocol decisions:
   NS_LOG="EnergyAwareEpidemicRouting=level_debug"

2. See energy monitoring:
   NS_LOG="BasicEnergySource=level_info:WifiRadioEnergyModel=level_info"

3. See packet transmission:
   NS_LOG="OnOffApplication=level_info:PacketSink=level_info"

4. See mobility:
   NS_LOG="RandomWaypointMobilityModel=level_debug"

5. See everything (very verbose):
   NS_LOG="*=level_all"

Combine multiple components:
   NS_LOG="EnergyAwareEpidemicRouting=level_info:EnergyAwareEpidemicHelper=level_debug"


EXPECTED FILES GENERATED:
=========================

1. emergency-communication-network.xml
   - NetAnim visualization file
   - Contains node positions, movements, packets
   - Size: ~1-5 MB depending on simulation time
   - Open with NetAnim application

2. emergency-routing-table.xml
   - Routing table changes over time
   - Shows how routes evolve during simulation
   - Size: ~500KB - 2MB
   - Can be analyzed with NetAnim

3. Console output
   - Energy reports every 30 seconds
   - Final communication statistics
   - Total packets sent/received
   - Can redirect to file: ./ns3 run energy-aware-epidemic-example > output.txt


UNDERSTANDING EPIDEMIC ROUTING BEHAVIOR:
=======================================

Key characteristics you'll observe:

1. NO IMMEDIATE PACKET DELIVERY:
   - Epidemic routing is store-and-forward
   - Packets delivered when nodes meet (within 100m)
   - High mobility helps but delivery is not guaranteed
   - This is NORMAL and expected for DTN protocols

2. PACKET FLOODING:
   - Packets replicated to all encountered nodes
   - Creates redundancy for reliability
   - Increases network traffic and energy consumption
   - Controlled by hop count (max 10 hops)

3. ENERGY-AWARE ADAPTATIONS:
   - Low energy: Probabilistic forwarding (50% chance)
   - Critical energy: Priority packets only
   - Beacon intervals increase to conserve energy
   - Queue management drops low-priority packets

4. NETWORK FRAGMENTATION:
   - As nodes deplete, network becomes partitioned
   - Remaining nodes may be isolated
   - Message delivery drops significantly
   - This demonstrates importance of energy management


COMPARING WITH/WITHOUT ENERGY AWARENESS:
========================================

To compare traditional epidemic vs. energy-aware:

1. Run with energy-aware (current configuration)
2. Note network lifetime until first node depletes
3. Disable energy-aware features by setting very low thresholds:
   energyAwareHelper.SetEnergyThresholds(0.01, 0.005);
4. Run again and compare:
   - Network lifetime
   - Energy distribution
   - Packet delivery ratio


SUMMARY OF KEY OBSERVATIONS:
============================

✓ Simulation runs for 10 minutes (600 seconds)
✓ Initial energy: 1000J per node
✓ First nodes deplete around 420-480 seconds
✓ All color phases visible: Green → Yellow → Orange → Red → Gray
✓ Energy depletion varies by node role (sender/receiver/relay)
✓ Adaptive behavior: beacon intervals increase, forwarding reduces
✓ Network fragments as nodes deplete
✓ Demonstrates successful energy-aware epidemic routing

================================================================================
For questions or issues, refer to the main documentation:
ENERGY_AWARE_EPIDEMIC_ROUTING_DOCUMENTATION.txt
================================================================================
