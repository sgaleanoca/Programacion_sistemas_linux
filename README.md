# Assignment-Sensor System Service

A systemd service that logs mock sensor data to files with fallback behavior and graceful shutdown handling.

## Documentation Checklist (README.md)

### Clone & Build

**Prerequisites:**
- GCC compiler (with C99 support)
- systemd (any recent version)
- make
- Standard C library

**Commands:**
```bash
git clone <repository-url>
cd proyecto-3-system-service-that-logs-a-mock-sensor
make
```

**Build details:**
- Compiler flags: `-Wall -g` (warnings enabled, debug info)
- Source files: `src/main.c`, `src/logger.c`, `src/sensor_mock.c`
- Object files created in `build/` directory

**Produced artifact path:**
- `./build/assignment-sensor`

### Install & Enable

**Copy binary path:**
```bash
sudo cp build/assignment-sensor /usr/local/bin/assignment-sensor
```

**Copy unit:**
```bash
sudo cp systemd/assignment-sensor.service /etc/systemd/system/assignment-sensor.service
```

**Reload systemd and enable service:**
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now assignment-sensor.service
```

### Configuration

**CLI flags:**
- `--interval <seconds>`: Interval between sensor readings (default: 5 seconds)
- `--logfile <path>`: Custom log file path (not implemented yet)
- `--device <path>`: Sensor device path (not implemented yet)

**Service configuration:**
The systemd service file (`/etc/systemd/system/assignment-sensor.service`) includes:
- Default interval: 5 seconds (configurable via CLI)
- Auto-restart on failure with 5-second delay
- Starts after multi-user.target

**Default values:**
- Interval: 5 seconds
- Primary log: `/tmp/sensor_log.txt`
- Fallback log: `/var/tmp/sensor_log.txt`

**Examples:**
```bash
assignment-sensor                    # Use default configuration
assignment-sensor --interval 10      # Read every 10 seconds
assignment-sensor --interval 1       # Read every second
```

**Manual execution:**
```bash
# Run manually (for testing)
./build/assignment-sensor
./build/assignment-sensor --interval 3
```

### Testing

**Verify running status:**
```bash
systemctl status assignment-sensor.service
```

**Check log path and example log lines:**
```bash
tail -f /tmp/sensor_log.txt
```

Example log format:
```
[2024-01-15T10:30:00Z] Sensor Value: 25.67
[2024-01-15T10:30:05Z] Sensor Value: 26.12
[2024-01-15T10:30:10Z] Sensor Value: 25.89
```

**Fallback behavior demo:**
If `/tmp` is not writable, the service automatically falls back to `/var/tmp/sensor_log.txt`:
```bash
# Simulate /tmp not being writable
sudo mkdir /tmp/sensor_log.txt
sudo systemctl restart assignment-sensor.service
# Service will use /var/tmp/sensor_log.txt instead
```

**Graceful shutdown test:**
```bash
sudo systemctl stop assignment-sensor.service
```
The service receives SIGTERM, exits the loop cleanly, and closes the log file.

**Auto-restart test:**
```bash
# Kill the process manually to test auto-restart
sudo pkill -f assignment-sensor
# Check if it restarted
systemctl status assignment-sensor.service
```

**Comprehensive testing:**
```bash
sudo ./tests/test.sh
```

**Real-time monitoring demo:**
```bash
sudo systemctl start assignment-sensor.service
sudo ./tests/demo_realtime.sh
```

### Uninstall

**Disable and stop service:**
```bash
sudo systemctl disable --now assignment-sensor.service
```

**Remove unit & binary:**
```bash
sudo rm /etc/systemd/system/assignment-sensor.service
sudo rm /usr/local/bin/assignment-sensor
sudo systemctl daemon-reload
```

**Clean up log files:**
```bash
sudo rm -f /tmp/sensor_log.txt /var/tmp/sensor_log.txt
```

**Clean build artifacts:**
```bash
make clean
```

## Features

- **Real-time logging**: Logs sensor data every 5 seconds (configurable)
- **Mock sensor simulation**: Generates random temperature values between 20.0°C and 30.0°C
- **Fallback behavior**: Automatically switches to `/var/tmp` if `/tmp` is not writable
- **Graceful shutdown**: Handles SIGTERM signal properly
- **ISO-8601 timestamps**: UTC format timestamps for all log entries
- **Systemd integration**: Full systemd service with auto-restart capabilities
- **Comprehensive testing**: Automated test suite covering all functionality
- **Signal handling**: Proper SIGTERM handling for clean shutdown

## Log Format

Each log entry follows this format:
```
[YYYY-MM-DDTHH:MM:SSZ] Sensor Value: XX.XX
```

Where:
- `YYYY-MM-DDTHH:MM:SSZ` is ISO-8601 UTC timestamp
- `XX.XX` is a random temperature value between 20.00°C and 30.00°C

## Project Structure

```
proyecto-3-system-service-that-logs-a-mock-sensor/
├── src/
│   ├── main.c              # Main program with signal handling
│   ├── logger.c/.h         # Log file management with fallback
│   └── sensor_mock.c/.h    # Mock temperature sensor simulation
├── systemd/
│   └── assignment-sensor.service  # Systemd service configuration
├── tests/
│   ├── test.sh            # Comprehensive test suite
│   └── demo_realtime.sh   # Real-time monitoring demo
├── build/                 # Compiled artifacts (created by make)
├── Makefile              # Build configuration
└── README.md             # This documentation
```
