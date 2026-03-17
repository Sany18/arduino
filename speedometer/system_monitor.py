#!/usr/bin/env python3
"""
System Monitor - Sends PC stats to ESP32
Displays: CPU, RAM, Battery, Network, Disk on 5 servos
"""

import psutil
import requests
import time
import json

# Configuration
ESP32_IP = "192.170.60.103"  # Change to your ESP32 IP
UPDATE_INTERVAL = 2  # seconds

# Network speed tracking
last_net_io = None
last_time = None

def get_network_speed():
    """Calculate network speed in KB/s"""
    global last_net_io, last_time
    
    current_net_io = psutil.net_io_counters()
    current_time = time.time()
    
    if last_net_io is None:
        last_net_io = current_net_io
        last_time = current_time
        return 0
    
    # Calculate speed (bytes/sec -> KB/sec)
    time_delta = current_time - last_time
    bytes_sent = (current_net_io.bytes_sent - last_net_io.bytes_sent) / time_delta
    bytes_recv = (current_net_io.bytes_recv - last_net_io.bytes_recv) / time_delta
    total_speed = (bytes_sent + bytes_recv) / 1024  # KB/s
    
    last_net_io = current_net_io
    last_time = current_time
    
    return int(total_speed)

def get_system_stats():
    """Collect all system statistics"""
    stats = {}
    
    # CPU Usage (0-100%)
    stats['cpu'] = int(psutil.cpu_percent(interval=0.5))
    
    # RAM Usage (0-100%)
    stats['ram'] = int(psutil.virtual_memory().percent)
    
    # Battery Level (0-100%)
    battery = psutil.sensors_battery()
    if battery:
        stats['battery'] = int(battery.percent)
    else:
        stats['battery'] = 0  # Desktop PC without battery
    
    # Network Speed (KB/s) - normalize to 0-100 (cap at 10MB/s = 10000 KB/s)
    net_speed = get_network_speed()
    stats['network'] = min(int((net_speed / 10000) * 100), 100)
    stats['network_raw'] = net_speed  # Actual KB/s value
    
    # Disk Usage (0-100%)
    stats['disk'] = int(psutil.disk_usage('/').percent)
    
    # Optional: Temperature (if available on macOS/Linux)
    try:
        temps = psutil.sensors_temperatures()
        if temps:
            # Get first available temperature
            first_sensor = list(temps.values())[0][0]
            stats['temp'] = int(first_sensor.current)
        else:
            stats['temp'] = 0
    except (AttributeError, IndexError):
        stats['temp'] = 0  # Not available on macOS without additional tools
    
    return stats

def send_to_esp32(stats):
    """Send statistics to ESP32"""
    try:
        url = f"http://{ESP32_IP}/api/stats"
        response = requests.post(
            url, 
            json=stats, 
            timeout=2,
            headers={'Content-Type': 'application/json'}
        )
        if response.status_code == 200:
            print(f"✓ Sent - CPU:{stats['cpu']}% RAM:{stats['ram']}% "
                  f"BAT:{stats['battery']}% NET:{stats['network_raw']}KB/s "
                  f"DISK:{stats['disk']}%")
        else:
            print(f"✗ Error: HTTP {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"✗ Connection failed: {e}")

def main():
    print(f"System Monitor started - Sending to {ESP32_IP}")
    print("Stats: CPU | RAM | Battery | Network | Disk")
    print("-" * 50)
    
    while True:
        try:
            stats = get_system_stats()
            send_to_esp32(stats)
            time.sleep(UPDATE_INTERVAL)
        except KeyboardInterrupt:
            print("\nStopped by user")
            break
        except Exception as e:
            print(f"Error: {e}")
            time.sleep(UPDATE_INTERVAL)

if __name__ == "__main__":
    main()
