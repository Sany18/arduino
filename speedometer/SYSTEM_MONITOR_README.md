# System Monitor with Servo Display

Display PC system stats on 5 servo motors showing:
- **Servo 1**: CPU Usage (%)
- **Servo 2**: RAM Usage (%)
- **Servo 3**: Battery Level (%)
- **Servo 4**: Network Load (%)
- **Servo 5**: Disk Usage (%)

## Setup Instructions

### 1. Install Python Dependencies

```bash
pip3 install psutil requests
```

### 2. Configure ESP32 IP Address

Edit `system_monitor.py` and change the ESP32 IP:

```python
ESP32_IP = "192.168.4.1"  # Change to your ESP32's IP address
```

You can find your ESP32's IP address in the Serial Monitor after it connects to WiFi.

### 3. Upload Firmware to ESP32

```bash
platformio run --target upload
```

### 4. Run the System Monitor

```bash
python3 system_monitor.py
```

Or make it executable and run directly:

```bash
chmod +x system_monitor.py
./system_monitor.py
```

## Output Example

The script will print stats every 2 seconds:

```
System Monitor started - Sending to 192.168.4.1
Stats: CPU | RAM | Battery | Network | Disk
--------------------------------------------------
✓ Sent - CPU:45% RAM:62% BAT:87% NET:124KB/s DISK:58%
✓ Sent - CPU:48% RAM:62% BAT:87% NET:89KB/s DISK:58%
```

## Network Load Calculation

- Network speed is measured in KB/s
- Normalized to 0-100% (capped at 10MB/s = 100%)
- Shows combined upload + download speed

## Troubleshooting

### Connection Failed
- Verify ESP32 IP address is correct
- Check ESP32 is on the same network
- Check ESP32 is powered on and WiFi connected

### No Battery Info (Desktop PC)
- Battery will show 0% on desktop computers
- Only laptops/devices with batteries report this stat

### Temperature Not Available (macOS)
- macOS requires additional tools for temperature sensors
- Works on Linux with appropriate sensor drivers
- Windows requires additional libraries

## Customization

To change update frequency, edit in `system_monitor.py`:

```python
UPDATE_INTERVAL = 2  # seconds
```

To change network speed cap (default 10MB/s):

```python
stats['network'] = min(int((net_speed / 10000) * 100), 100)
#                                        ^^^^^ change this value
```
