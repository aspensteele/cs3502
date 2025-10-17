# CS 3502 – Project 2: CPU Scheduling

## Overview
This project simulates different CPU scheduling algorithms to analyze their performance under various workloads. It is based on the OwlTech Industries Performance Optimization Division scenario, where the goal is to identify which scheduling algorithms provide the best balance of efficiency, fairness, and responsiveness.

## Features
- Includes starter algorithms:
  - First-Come, First-Served (FCFS)
  - Shortest Job First (SJF)
  - Round Robin (RR)
  - Priority Scheduling
- Adds two new algorithms:
  - SRTF and HRRN
- Calculates key performance metrics:
  - Average Waiting Time
  - Average Turnaround Time
  - CPU Utilization
  - Throughput
  - Response Time (optional)


## How It Works
The simulator allows users to enter process information (arrival time, burst time, priority, etc.) and run different scheduling algorithms.  
After execution, it displays the performance metrics for each algorithm, allowing comparison between them.

## Testing
You can test the simulator with:
- Small workloads (5–10 processes) for manual verification.
- Medium and large workloads (20–100+ processes) for performance analysis.
- Edge cases such as identical burst times or all processes arriving at time 0.

## Output
The program outputs:
- Calculated averages for each performance metric.
- Visual or tabular comparisons of different scheduling algorithms.

