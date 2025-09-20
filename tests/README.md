# Testing Documentation for Assignment-Sensor

This directory contains comprehensive testing scripts for the assignment-sensor systemd service.

## Scripts Available

### 1. `test.sh` - Comprehensive Test Suite

The main testing script that provides complete validation of the assignment-sensor service.

**Features:**
- ✅ Real-time log monitoring (10 seconds)
- ✅ Complete installation/uninstallation testing
- ✅ Fallback behavior verification
- ✅ Graceful shutdown testing
- ✅ Configuration validation
- ✅ Automatic cleanup

**Usage:**
```bash
sudo ./tests/test.sh
```

**What it tests:**
1. **Clone & Build**
   - Prerequisites verification (GCC, systemd, make)
   - Project compilation
   - Artifact generation

2. **Install & Enable**
   - Binary installation to `/usr/local/bin/`
   - Service file installation to `/etc/systemd/system/`
   - Service enabling and starting
   - Systemd daemon reload

3. **Configuration**
   - CLI flags documentation
   - Default values verification
   - Usage examples

4. **Testing Comprehensive**
   - Log file creation in `/tmp`
   - Real-time log monitoring (10 seconds)
   - Log format validation (ISO-8601)
   - Graceful shutdown with SIGTERM
   - Fallback behavior to `/var/tmp`
   - Real-time monitoring of fallback logs
   - Automatic service restart verification

5. **Uninstall Demo**
   - Service disabling and stopping
   - File removal verification
   - Cleanup demonstration

### 2. `demo_realtime.sh` - Real-time Monitoring Demo

A lightweight script that demonstrates real-time log monitoring without running the full test suite.

**Usage:**
```bash
./tests/demo_realtime.sh
```

**Requirements:**
- The assignment-sensor service must be running
- Run with: `sudo systemctl start assignment-sensor.service`

## Command Line Arguments

The assignment-sensor service supports the following arguments:

```bash
assignment-sensor [--interval <seconds>] [--logfile <path>] [--device <path>]
```

### Supported Arguments

| Argument | Description | Default | Status |
|----------|-------------|---------|---------|
| `--interval <seconds>` | Interval between sensor readings | 5 | ✅ Implemented |
| `--logfile <path>` | Custom log file path | `/tmp/sensor_log.txt` | ⚠️ Not implemented |
| `--device <path>` | Sensor device path | N/A | ⚠️ Not implemented |

### Examples

```bash
# Default configuration (5-second interval)
assignment-sensor

# Custom interval (10 seconds)
assignment-sensor --interval 10

# Fast monitoring (1 second)
assignment-sensor --interval 1
```

## Log Files

### Primary Log Location
- **Path:** `/tmp/sensor_log.txt`
- **Format:** ISO-8601 UTC timestamp + sensor value
- **Example:** `[2024-01-15T14:30:25Z] Sensor Value: 23.45`

### Fallback Log Location
- **Path:** `/var/tmp/sensor_log.txt`
- **Trigger:** When `/tmp` is not writable
- **Format:** Same as primary log

## Service Management

### Installation
```bash
# Build the project
make

# Install binary
sudo cp ./build/assignment-sensor /usr/local/bin/

# Install service
sudo cp systemd/assignment-sensor.service /etc/systemd/system/

# Reload systemd and enable service
sudo systemctl daemon-reload
sudo systemctl enable --now assignment-sensor.service
```

### Service Commands
```bash
# Check status
sudo systemctl status assignment-sensor.service

# Start service
sudo systemctl start assignment-sensor.service

# Stop service
sudo systemctl stop assignment-sensor.service

# Restart service
sudo systemctl restart assignment-sensor.service

# Disable service
sudo systemctl disable assignment-sensor.service
```

### Uninstallation
```bash
# Stop and disable service
sudo systemctl disable --now assignment-sensor.service

# Remove files
sudo rm /usr/local/bin/assignment-sensor
sudo rm /etc/systemd/system/assignment-sensor.service

# Reload systemd
sudo systemctl daemon-reload
```

## Real-time Monitoring

### Using the Test Script
The main test script includes automatic real-time monitoring:

```bash
sudo ./tests/test.sh
```

This will show logs in real-time for 10 seconds during the testing process.

### Manual Real-time Monitoring
You can manually monitor logs using standard Unix tools:

```bash
# Monitor primary log
tail -f /tmp/sensor_log.txt

# Monitor fallback log
tail -f /var/tmp/sensor_log.txt

# Monitor with timeout (10 seconds)
timeout 10s tail -f /tmp/sensor_log.txt
```

### Using the Demo Script
For a quick demonstration without full testing:

```bash
# Ensure service is running
sudo systemctl start assignment-sensor.service

# Run demo
./tests/demo_realtime.sh
```

## Troubleshooting

### Service Not Starting
1. Check service status: `sudo systemctl status assignment-sensor.service`
2. Check logs: `sudo journalctl -u assignment-sensor.service`
3. Verify binary exists: `ls -la /usr/local/bin/assignment-sensor`
4. Verify service file: `cat /etc/systemd/system/assignment-sensor.service`

### No Log Files Created
1. Check if service is running: `sudo systemctl is-active assignment-sensor.service`
2. Check permissions on `/tmp` and `/var/tmp`
3. Verify service can write to log locations

### Real-time Monitoring Not Working
1. Ensure log file exists: `ls -la /tmp/sensor_log.txt /var/tmp/sensor_log.txt`
2. Check if service is actively writing: `tail -n 5 /tmp/sensor_log.txt`
3. Verify `tail` command is available: `which tail`

## Test Results Interpretation

### Success Indicators
- ✅ All tests pass without errors
- ✅ Service starts and stops cleanly
- ✅ Log files are created with correct format
- ✅ Real-time monitoring shows new entries
- ✅ Fallback behavior works when `/tmp` is blocked
- ✅ Service restarts automatically after failures

### Failure Indicators
- ❌ Compilation errors
- ❌ Service fails to start
- ❌ No log files created
- ❌ Incorrect log format
- ❌ Service doesn't stop gracefully
- ❌ Fallback doesn't work
- ❌ No automatic restart after failure

## Development Notes

### Current Limitations
- `--logfile` and `--device` arguments are not yet implemented
- Service only supports mock sensor data
- No configuration file support

### Future Enhancements
- Implement custom log file path support
- Add device path configuration
- Add configuration file support
- Add more comprehensive error handling
- Add metrics and monitoring capabilities
