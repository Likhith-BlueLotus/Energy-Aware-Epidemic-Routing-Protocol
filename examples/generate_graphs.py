#!/usr/bin/env python3
"""
Graph Generation Script for Energy-Aware Epidemic Routing
Generates graphs from NS-3 simulation results
"""

import matplotlib.pyplot as plt
import numpy as np
import re
import sys
import os
from collections import defaultdict

def parse_energy_log(filename):
    """Parse energy consumption from simulation log"""
    node_energy = defaultdict(list)
    times = []
    
    if not os.path.exists(filename):
        print(f"Error: File {filename} not found")
        return None, None
    
    with open(filename, 'r') as f:
        for line in f:
            # Parse: "Node X: Energy=1000J, ..." or "Node X: ... Remaining=1000J"
            match = re.search(r'Node (\d+):.*(?:Energy|Remaining)=([\d.]+)J', line)
            if match:
                node_id = int(match.group(1))
                energy = float(match.group(2))
                node_energy[node_id].append(energy)
            
            # Parse time
            time_match = re.search(r'Time: ([\d.]+)', line)
            if time_match:
                times.append(float(time_match.group(1)))
    
    return times, node_energy

def parse_pdr_log(filename):
    """Parse packet delivery ratio from log"""
    times = []
    pdr_values = []
    
    with open(filename, 'r') as f:
        for line in f:
            # Parse: "PDR: 0.85 at time 100.0"
            match = re.search(r'PDR: ([\d.]+).*time ([\d.]+)', line)
            if match:
                pdr_values.append(float(match.group(1)))
                times.append(float(match.group(2)))
    
    return times, pdr_values

def plot_energy_consumption(times, node_energy, output_file='energy_consumption.png'):
    """Generate energy consumption graph for all nodes"""
    plt.figure(figsize=(12, 8))
    
    # Plot first 10 nodes to avoid clutter
    for node_id in sorted(node_energy.keys())[:10]:
        energies = node_energy[node_id]
        if len(energies) == len(times):
            plt.plot(times, energies, label=f'Node {node_id}', linewidth=1.5, alpha=0.7)
    
    plt.xlabel('Time (seconds)', fontsize=14, fontweight='bold')
    plt.ylabel('Remaining Energy (Joules)', fontsize=14, fontweight='bold')
    plt.title('Energy Consumption - Multiple Nodes', fontsize=16, fontweight='bold')
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.legend(loc='upper right', ncol=2, fontsize=10)
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"[OK] Graph saved: {output_file}")
    plt.close()

def plot_average_energy(times, node_energy, output_file='average_energy.png'):
    """Generate average energy consumption graph"""
    plt.figure(figsize=(12, 8))
    
    # Calculate average energy across all nodes
    if node_energy:
        num_nodes = len(node_energy)
        avg_energy = []
        for i in range(len(times)):
            total = sum(node_energy[nid][i] for nid in node_energy.keys() 
                       if i < len(node_energy[nid]))
            avg_energy.append(total / num_nodes if num_nodes > 0 else 0)
        
        plt.plot(times, avg_energy, label='Average Energy', linewidth=2.5, color='red')
    
    plt.xlabel('Time (seconds)', fontsize=14, fontweight='bold')
    plt.ylabel('Average Remaining Energy (Joules)', fontsize=14, fontweight='bold')
    plt.title('Average Network Energy Consumption', fontsize=16, fontweight='bold')
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.legend(fontsize=12)
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"[OK] Graph saved: {output_file}")
    plt.close()

def plot_packet_delivery_ratio(times, pdr_values, output_file='pdr_graph.png'):
    """Generate packet delivery ratio graph"""
    plt.figure(figsize=(12, 8))
    
    if times and pdr_values:
        plt.plot(times, pdr_values, label='Packet Delivery Ratio', 
                linewidth=2.5, color='blue', marker='o', markersize=4)
    
    plt.xlabel('Time (seconds)', fontsize=14, fontweight='bold')
    plt.ylabel('Packet Delivery Ratio', fontsize=14, fontweight='bold')
    plt.title('Packet Delivery Ratio Over Time', fontsize=16, fontweight='bold')
    plt.ylim([0, 1.1])
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.legend(fontsize=12)
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"[OK] Graph saved: {output_file}")
    plt.close()

def plot_network_lifetime(node_energy, output_file='network_lifetime.png'):
    """Generate network lifetime comparison graph"""
    plt.figure(figsize=(12, 8))
    
    lifetimes = []
    node_ids = []
    
    for node_id in sorted(node_energy.keys()):
        energies = node_energy[node_id]
        # Find when energy drops to 0
        lifetime = len(energies)  # Simplified: use simulation duration
        lifetimes.append(lifetime)
        node_ids.append(node_id)
    
    plt.bar(node_ids[:20], lifetimes[:20], alpha=0.7, color='green')
    plt.xlabel('Node ID', fontsize=14, fontweight='bold')
    plt.ylabel('Network Lifetime (time units)', fontsize=14, fontweight='bold')
    plt.title('Network Lifetime per Node', fontsize=16, fontweight='bold')
    plt.grid(True, alpha=0.3, linestyle='--', axis='y')
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"[OK] Graph saved: {output_file}")
    plt.close()

def plot_energy_efficiency_comparison(output_file='energy_efficiency.png'):
    """Generate energy efficiency comparison graph"""
    plt.figure(figsize=(12, 8))
    
    # Example data - replace with actual simulation results
    node_counts = [10, 15, 20, 25, 30]
    energy_aware = [0.85, 0.82, 0.78, 0.75, 0.72]  # Energy efficiency
    standard = [0.65, 0.60, 0.55, 0.50, 0.45]     # Standard epidemic
    
    x = np.arange(len(node_counts))
    width = 0.35
    
    plt.bar(x - width/2, energy_aware, width, label='Energy-Aware', 
           alpha=0.8, color='green')
    plt.bar(x + width/2, standard, width, label='Standard Epidemic', 
           alpha=0.8, color='red')
    
    plt.xlabel('Number of Nodes', fontsize=14, fontweight='bold')
    plt.ylabel('Energy Efficiency', fontsize=14, fontweight='bold')
    plt.title('Energy Efficiency Comparison', fontsize=16, fontweight='bold')
    plt.xticks(x, node_counts)
    plt.legend(fontsize=12)
    plt.grid(True, alpha=0.3, linestyle='--', axis='y')
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"[OK] Graph saved: {output_file}")
    plt.close()

def main():
    """Main function to generate all graphs"""
    log_file = 'simulation.log'
    
    if len(sys.argv) > 1:
        log_file = sys.argv[1]
    
    print("=" * 60)
    print("Energy-Aware Epidemic Routing - Graph Generation")
    print("=" * 60)
    
    # Parse log file
    print(f"\nParsing log file: {log_file}")
    times, node_energy = parse_energy_log(log_file)
    
    if times is None:
        print("Error: Could not parse log file")
        return
    
    # Generate graphs
    print("\nGenerating graphs...")
    
    if times and node_energy:
        plot_energy_consumption(times, node_energy)
        plot_average_energy(times, node_energy)
        plot_network_lifetime(node_energy)
    
    # Parse PDR if available
    times_pdr, pdr_values = parse_pdr_log(log_file)
    if times_pdr and pdr_values:
        plot_packet_delivery_ratio(times_pdr, pdr_values)
    
    # Comparison graph
    plot_energy_efficiency_comparison()
    
    print("\n" + "=" * 60)
    print("All graphs generated successfully!")
    print("=" * 60)
    print("\nGenerated files:")
    print("  - energy_consumption.png")
    print("  - average_energy.png")
    print("  - pdr_graph.png")
    print("  - network_lifetime.png")
    print("  - energy_efficiency.png")

if __name__ == '__main__':
    main()

