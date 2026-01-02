# Multi-stage build for CoAP-MQTT Gateway

# Build stage
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Build Paho MQTT C library
WORKDIR /tmp
RUN git clone https://github.com/eclipse/paho.mqtt.c.git && \
    cd paho.mqtt.c && \
    git checkout v1.3.13 && \
    cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF -DPAHO_BUILD_STATIC=ON && \
    cmake --build build/ --target install

WORKDIR /build

COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Runtime stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy Paho MQTT C library from builder
COPY --from=builder /usr/local/lib/libpaho-mqtt3* /usr/local/lib/
RUN ldconfig

WORKDIR /app

COPY --from=builder /build/build/coap_client .

# Environment variables with defaults
ENV COAP_HOST=localhost \
    COAP_PORT=5683 \
    COAP_PATH=/ \
    MQTT_BROKER=localhost \
    MQTT_PORT=1883 \
    MQTT_TOPIC=coap/data \
    POLL_INTERVAL=10 \
    METRICS_PORT=9090

EXPOSE 9090

CMD ["./coap_client"]
