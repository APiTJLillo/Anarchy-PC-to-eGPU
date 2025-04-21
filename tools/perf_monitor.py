#!/usr/bin/env python3

import sys
import time
import argparse
import curses
import re
from pathlib import Path
from datetime import datetime
from collections import deque

class PerformanceMetrics:
    def __init__(self, window_size=60):
        self.window_size = window_size
        self.tx_throughput = deque(maxlen=window_size)
        self.rx_throughput = deque(maxlen=window_size)
        self.dma_latency = deque(maxlen=window_size)
        self.tb_connects = 0
        self.tb_errors = 0
        self.timestamps = deque(maxlen=window_size)

    def update(self, stats):
        now = datetime.now()
        self.timestamps.append(now)
        
        # Extract throughput
        tx_match = re.search(r'TX:.*\(([\d.]+) MB/s\)', stats)
        rx_match = re.search(r'RX:.*\(([\d.]+) MB/s\)', stats)
        self.tx_throughput.append(float(tx_match.group(1)) if tx_match else 0)
        self.rx_throughput.append(float(rx_match.group(1)) if rx_match else 0)

        # Extract latency
        latency_match = re.search(r'Average:\s+(\d+)\s+ns', stats)
        self.dma_latency.append(int(latency_match.group(1)) if latency_match else 0)

        # Extract Thunderbolt stats
        tb_connect_match = re.search(r'Connections:\s+(\d+)', stats)
        tb_error_match = re.search(r'Errors:\s+(\d+)', stats)
        self.tb_connects = int(tb_connect_match.group(1)) if tb_connect_match else 0
        self.tb_errors = int(tb_error_match.group(1)) if tb_error_match else 0

def read_stats(path):
    try:
        with open(path, 'r') as f:
            return f.read()
    except Exception as e:
        print(f"Error reading statistics: {e}", file=sys.stderr)
        sys.exit(1)

def draw_graph(win, data, max_val, title, y_offset, height=10):
    if not data:
        return

    width = min(len(data), win.getmaxyx()[1] - 2)
    win.addstr(y_offset, 0, f"{title} (max: {max(data):.2f})")
    
    for x in range(width):
        idx = x * len(data) // width
        val = data[idx]
        y = int((val / max_val) * (height - 1))
        for h in range(y + 1):
            win.addch(y_offset + height - h, x + 1, '|')

def main(stdscr):
    parser = argparse.ArgumentParser(description='Anarchy eGPU Performance Monitor')
    parser.add_argument('--interval', type=float, default=1.0,
                       help='Update interval in seconds')
    parser.add_argument('--stats-file', type=str,
                       default='/sys/kernel/debug/anarchy-egpu/performance/statistics',
                       help='Path to statistics file')
    args = parser.parse_args()

    # Initialize curses
    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(1, curses.COLOR_GREEN, -1)
    curses.init_pair(2, curses.COLOR_RED, -1)
    stdscr.nodelay(1)
    stdscr.timeout(100)

    metrics = PerformanceMetrics()
    max_throughput = 100  # Initial max throughput (MB/s)
    max_latency = 1000    # Initial max latency (ns)

    while True:
        try:
            stats = read_stats(args.stats_file)
            metrics.update(stats)

            stdscr.clear()
            height, width = stdscr.getmaxyx()

            # Update max values if needed
            max_throughput = max(max_throughput,
                               max(max(metrics.tx_throughput or [0]),
                                   max(metrics.rx_throughput or [0])) * 1.1)
            max_latency = max(max_latency,
                            max(metrics.dma_latency or [0]) * 1.1)

            # Draw throughput graphs
            draw_graph(stdscr, metrics.tx_throughput, max_throughput,
                      "TX Throughput (MB/s)", 1, 8)
            draw_graph(stdscr, metrics.rx_throughput, max_throughput,
                      "RX Throughput (MB/s)", 10, 8)
            draw_graph(stdscr, metrics.dma_latency, max_latency,
                      "DMA Latency (ns)", 19, 8)

            # Draw Thunderbolt statistics
            stdscr.addstr(28, 0, f"Thunderbolt Status:", curses.A_BOLD)
            stdscr.addstr(29, 2, f"Connections: {metrics.tb_connects}")
            stdscr.addstr(30, 2, f"Errors: {metrics.tb_errors}",
                         curses.COLOR_PAIR(2) if metrics.tb_errors > 0 else 0)

            # Draw current values
            stdscr.addstr(32, 0, "Current Values:", curses.A_BOLD)
            stdscr.addstr(33, 2,
                f"TX: {metrics.tx_throughput[-1]:.2f} MB/s  "
                f"RX: {metrics.rx_throughput[-1]:.2f} MB/s  "
                f"Latency: {metrics.dma_latency[-1]:.0f} ns")

            stdscr.addstr(height-1, 0,
                         "Press 'q' to quit, 'r' to reset scaling",
                         curses.A_REVERSE)
            stdscr.refresh()

            # Handle input
            c = stdscr.getch()
            if c == ord('q'):
                break
            elif c == ord('r'):
                max_throughput = 100
                max_latency = 1000

            time.sleep(args.interval)

        except KeyboardInterrupt:
            break
        except Exception as e:
            stdscr.addstr(0, 0, f"Error: {e}")
            stdscr.refresh()
            time.sleep(1)

if __name__ == '__main__':
    curses.wrapper(main) 