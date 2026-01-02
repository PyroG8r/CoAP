# CoAP to MQTT Gateway - Industrial Motor Monitor

A lightweight IoT gateway that bridges CoAP (Constrained Application Protocol) devices to MQTT infrastructure with built-in Prometheus monitoring. Designed for Industrial IoT (IIoT) deployments to monitor electric motor health metrics including temperature, vibration, power consumption, and operational state.

## What It Does

This application acts as a **protocol translator gateway** that:

1. **Polls CoAP Motor Sensors** - Continuously queries 5 sensor endpoints (temperature, vibration, power, state, uptime) at configurable intervals
2. **Forwards Data to MQTT** - Publishes each sensor value to dedicated MQTT topics for downstream processing
3. **Exposes Metrics** - Provides real-time Prometheus metrics including live sensor gauges for monitoring
4. **Runs in Docker** - Fully containerized for easy deployment in cloud or edge environments

### Use Cases

- **Predictive Maintenance**: Monitor motor temperature and vibration to predict failures before they occur
- **IIoT Sensor Integration**: Connect CoAP-based industrial sensors to MQTT-based cloud platforms
- **Real-time Alerting**: Stream motor health data to alert systems via MQTT
- **Equipment Analytics**: Collect operational data for analysis and optimization

## Architecture

```
CoAP Motor Server → Gateway (Sequential Polling) → MQTT Topics
  ├─ /machine/temperature  →  machine/sensors/temperature
  ├─ /machine/vibration    →  machine/sensors/vibration
  ├─ /machine/power        →  machine/sensors/power
  ├─ /machine/state        →  machine/sensors/state
  └─ /machine/uptime       →  machine/sensors/uptime
                      ↓
                Prometheus Metrics (port 9090)
                  ├─ Counters (requests, errors, etc.)
                  └─ Gauges (live sensor values)
                      ↓
                Grafana Dashboard
```

## Features

- **CoAP Client**: Full RFC 7252 implementation supporting GET requests with automatic timeout handling
- **Multi-Sensor Polling**: Sequential querying of 5 industrial motor sensor endpoints
- **Persistent MQTT Connection**: Single long-lived connection using Eclipse Paho MQTT C library for efficient publishing
- **Separate MQTT Topics**: Individual topics per sensor for flexible data routing and filtering
- **Prometheus Metrics**: Counters for operations + Gauges for live sensor values
- **Production Ready**: Signal handling, graceful shutdown, error recovery, and structured logging
- **Cloud Native**: Containerized with Docker, ready for Kubernetes deployment

## Quick Start

### 1. Configure Environment

Create a `.env` file with your settings:

```bash
# CoAP Motor Server Configuration
COAP_HOST=localhost            # Target CoAP server hostname (default: localhost)
COAP_PORT=5683                 # CoAP server port (default: 5683)

# MQTT Broker Configuration
MQTT_BROKER=mqtt.example.com   # MQTT broker hostname
MQTT_PORT=1883                 # MQTT broker port
MQTT_TOPIC=machine/sensors     # Base topic (sensors publish to /temperature, /vibration, etc.)

# Gateway Settings
POLL_INTERVAL=5                # Polling interval in seconds (default: 5)
METRICS_PORT=9090              # Prometheus metrics port
```

**Note**: The gateway polls these CoAP endpoints sequentially:
- `/machine/temperature` - Motor temperature in °C
- `/machine/vibration` - Vibration level in mm/s
- `/machine/power` - Power consumption in kW
- `/machine/state` - Operational state (normal/degraded/failed)
- `/machine/uptime` - Uptime in seconds

### 2. Run with Docker

```bash
# Build and start the gateway
docker-compose up -d

# View real-time logs
docker-compose logs -f

# Check metrics
curl http://localhost:9090/metrics
```

### 3. Access Metrics

- **Metrics Endpoint**: http://localhost:9090/metrics (Prometheus format)
- **Grafana Dashboard**: Import `grafana-dashboard.json` to visualize metrics

## Prometheus Metrics

The gateway exposes the following metrics at `http://localhost:9090/metrics`:

### Gateway Counters

| Metric | Type | Description |
|--------|------|-------------|
| `coap_requests_sent_total` | Counter | Total number of CoAP requests sent |
| `coap_responses_received_total` | Counter | Total number of CoAP responses received |
| `mqtt_messages_published_total` | Counter | Total number of MQTT messages published |
| `coap_errors_total` | Counter | Total number of errors encountered |
| `coap_total_response_time_ms` | Counter | Cumulative CoAP response time in milliseconds |

### Motor Sensor Gauges

| Metric | Type | Description |
|--------|------|-------------|
| `machine_temperature_celsius` | Gauge | Current motor temperature in °C (25-95°C) |
| `machine_vibration_mm_per_second` | Gauge | Current vibration level in mm/s (2-15 mm/s) |
| `machine_power_kw` | Gauge | Current power consumption in kW (0-6.5 kW) |
| `machine_state` | Gauge | Machine state: 0=normal, 1=degraded, 2=failed |
| `machine_uptime_seconds` | Gauge | Machine uptime in seconds |

### Example Queries

```promql
# Request rate (requests/second)
rate(coap_requests_sent_total[5m])

# Success rate percentage
(coap_responses_received_total / coap_requests_sent_total) * 100

# Current motor temperature
machine_temperature_celsius

# Alert when motor is degraded or failed
machine_state >= 1

# Average vibration over 5 minutes
avg_over_time(machine_vibration_mm_per_second[5m])

# Average response time
rate(coap_total_response_time_ms[5m]) / rate(coap_requests_sent_total[5m])

# Error rate
rate(coap_errors_total[5m])
```

## Grafana Dashboard

A pre-configured Grafana dashboard is included at `grafana-dashboard.json`. It provides:

- **Real-time metrics** with 5-second auto-refresh
- **Key performance indicators**: Total requests, responses, MQTT messages, errors
- **Time-series graphs**: Request/response rates, latency, error rates
- **Success rate gauge**: Visual health indicator

**To import**: Dashboards → Import → Upload `grafana-dashboard.json`

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `COAP_HOST` | `localhost` | CoAP motor server hostname or IP |
| `COAP_PORT` | `5683` | CoAP server port |
| `MQTT_BROKER` | `localhost` | MQTT broker hostname |
| `MQTT_PORT` | `1883` | MQTT broker port |
| `MQTT_TOPIC` | `machine/sensors` | Base MQTT topic (sensors append /temperature, etc.) |
| `POLL_INTERVAL` | `5` | Polling interval in seconds |
| `METRICS_PORT` | `9090` | Port for Prometheus metrics endpoint |

### MQTT Topic Structure

The gateway publishes to separate topics for each sensor:

```
{MQTT_TOPIC}/temperature  → "67.23"     (°C)
{MQTT_TOPIC}/vibration    → "3.45"      (mm/s)
{MQTT_TOPIC}/power        → "5.12"      (kW)
{MQTT_TOPIC}/state        → "normal"    (normal|degraded|failed)
{MQTT_TOPIC}/uptime       → "1234.5"    (seconds)
```

Example with default base topic `machine/sensors`:
- `machine/sensors/temperature`
- `machine/sensors/vibration`
- `machine/sensors/power`
- `machine/sensors/state`
- `machine/sensors/uptime`

### Docker Compose

The gateway uses Docker host networking to access host services:

```yaml
extra_hosts:
  - "host.docker.internal:host-gateway"
```

This allows the container to reach services running on your host machine (e.g., local MQTT brokers).

## Remote Access with Cloudflare Tunnel

To expose metrics to a remote Prometheus instance:

```bash
# Create tunnel
cloudflared tunnel create coap-metrics

# Route DNS
cloudflared tunnel route dns coap-metrics metrics.yourdomain.com

# Configure ~/.cloudflared/config.yml
tunnel: coap-metrics
credentials-file: /home/user/.cloudflared/<TUNNEL-ID>.json
ingress:
  - hostname: metrics.yourdomain.com
    service: http://localhost:9090
  - service: http_status:404

# Run tunnel
cloudflared tunnel run coap-metrics
```

Now your Prometheus can scrape `https://metrics.yourdomain.com/metrics`

## Development

### Local Build

```bash
# Configure with CMake
cmake -S . -B build -G Ninja

# Build the project
cmake --build build

# Run tests
cd build && ctest

# Run locally (not in gateway mode)
./build/coap_client
```

### Project Structure

```
├── src/                    # Source files
│   ├── main.cpp           # Gateway application entry point
│   ├── client.cpp         # CoAP client implementation
│   ├── message.cpp        # CoAP message encoding/decoding
│   ├── mqtt_publisher.cpp # MQTT publishing with Paho MQTT C library
│   ├── metrics.cpp        # Prometheus metrics collector
│   └── metrics_server.cpp # HTTP server for metrics endpoint
├── include/coap/          # Header files
├── CMakeLists.txt         # Build configuration
├── Dockerfile             # Container image definition (includes Paho MQTT C)
├── docker-compose.yml     # Docker orchestration
└── grafana-dashboard.json # Pre-built Grafana dashboard
```

### Dependencies

- **Eclipse Paho MQTT C** (v1.3.13): Persistent MQTT client library
- **C++17**: Modern C++ features for clean, maintainable code
- **CMake + Ninja**: Fast, reliable build system
- **Docker**: Containerization and deployment

### Adding Features

The codebase is modular:
- **CoAP Protocol**: Extend [client.cpp](src/client.cpp) and [message.cpp](src/message.cpp)
- **Metrics**: Add new metrics in [metrics.cpp](src/metrics.cpp)
- **MQTT**: Modify persistent connection logic in [mqtt_publisher.cpp](src/mqtt_publisher.cpp)
- **Gateway Behavior**: Update polling logic in [main.cpp](src/main.cpp)

### MQTT Connection Details

The gateway establishes a **single persistent MQTT connection** on startup:

- **Library**: Eclipse Paho MQTT C (synchronous client)
- **QoS**: 0 (at most once delivery) for low latency
- **Keep-Alive**: 20 seconds
- **Clean Session**: Yes (no persistent state)
- **Auto-Reconnect**: Automatic reconnection on connection loss
- **Graceful Shutdown**: Proper DISCONNECT message sent on exit

This approach provides:
- ✅ Efficient network usage (one connection vs. 5+ per cycle)
- ✅ Lower latency (no connection overhead)
- ✅ Graceful broker handling (no ungraceful disconnects)
- ✅ Better reliability (connection state tracking)

## Troubleshooting

### Gateway not connecting to CoAP server (hangs after "Polling Cycle")

**Problem**: Gateway shows `=== Polling Cycle ===` but no sensor data appears

**Solutions**:

1. **Docker Networking Issue**: When running in Docker, `localhost` refers to the container itself
   ```bash
   # In your .env file, use host.docker.internal instead of localhost
   COAP_HOST=host.docker.internal
   ```

2. **Check if CoAP server is running**:
   ```bash
   # From host machine
   sudo netstat -tulpn | grep 5683
   # or
   sudo ss -tulpn | grep 5683
   ```

3. **Test CoAP server manually** (install coap-client):
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libcoap2-bin
   
   # Test endpoint
   coap-client -m get coap://localhost:5683/machine/temperature
   ```

4. **Check gateway logs** for timeout messages:
   ```bash
   docker-compose logs -f coap-gateway
   # Look for "Receive timeout - no response from server"
   ```

5. **Verify CoAP server is accessible from container**:
   ```bash
   docker-compose exec coap-gateway ping -c 3 host.docker.internal
   ```

### MQTT messages not published

```bash
# Check MQTT broker connectivity
docker-compose logs coap-gateway | grep MQTT

# Test MQTT broker manually
mosquitto_sub -h <MQTT_BROKER> -p <MQTT_PORT> -t <MQTT_TOPIC>
```

### Metrics not accessible

```bash
# Check if metrics server started
docker-compose logs coap-gateway | grep "Metrics server"

# Test metrics endpoint
curl http://localhost:9090/metrics
```

## Cleanup

```bash
# Stop the gateway
docker-compose down

# Remove containers and volumes
docker-compose down -v

# Remove images
docker-compose down --rmi all
```

## License

This project implements CoAP according to [RFC 7252](https://datatracker.ietf.org/doc/html/rfc7252).

## Contributing

Contributions welcome! Areas for improvement:
- Additional CoAP message types (NON, ACK, RST)
- CoAP DTLS security
- Block-wise transfers for large payloads
- Observing resources (CoAP Observe)
- Multi-endpoint polling
- Configurable retry logic
