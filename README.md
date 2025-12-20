# CoAP Client with Prometheus Metrics and MQTT Integration

A CoAP (Constrained Application Protocol) client implementation with integrated Prometheus metrics and MQTT publishing capabilities, all containerized with Docker.

## Features

- **CoAP Client**: Full-featured CoAP client supporting GET, POST, PUT, DELETE methods
- **Prometheus Metrics**: Built-in metrics server exposing CoAP operation statistics
- **MQTT Integration**: Automatic publishing of CoAP responses to MQTT topics
- **Containerized**: Complete Docker setup with Mosquitto broker, Prometheus, and Grafana

## Quick Start

### Build and Run with Docker Compose

```bash
# Build and start all services
docker-compose up -d

# Check service status
docker-compose ps

# View logs
docker-compose logs -f coap-client
```

### Use the CoAP Client

```bash
# Send a GET request
docker-compose exec coap-client /app/coap_client GET coap.me /hello

# Send a POST request with payload
docker-compose exec coap-client /app/coap_client POST your-server.com /api/data -p "Hello, CoAP!"
```

### Access Services

- **Prometheus**: http://localhost:9090
- **Grafana**: http://localhost:3000 (admin/admin)
- **CoAP Metrics**: http://localhost:8080/metrics
- **Mosquitto MQTT**: localhost:1883

## Metrics

The application exposes Prometheus metrics at `http://localhost:8080/metrics`:

- `coap_requests_sent_total` - Total CoAP requests sent
- `coap_responses_received_total` - Total CoAP responses received
- `coap_request_errors_total` - Total CoAP request errors
- `mqtt_messages_published_total` - Total MQTT messages published
- `coap_last_response_time_ms` - Last response time in milliseconds
- `coap_request_duration_seconds` - Request duration histogram

## Environment Variables

- `METRICS_PORT` - Metrics endpoint port (default: 8080)
- `MQTT_BROKER` - MQTT broker hostname (default: mosquitto)
- `MQTT_PORT` - MQTT broker port (default: 1883)
- `MQTT_TOPIC` - MQTT topic for responses (default: coap/responses)

## Development

```bash
# Local build
cmake -S . -B build -G Ninja
cmake --build build
./build/coap_client GET coap.me /hello

# Docker build
docker build -t coap-client .
```

## Cleanup

```bash
docker-compose down -v
```
