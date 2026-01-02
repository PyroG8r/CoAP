#include "coap/client.hpp"
#include "coap/message.hpp"
#include "coap/cli.hpp"
#include "coap/metrics.hpp"
#include "coap/metrics_server.hpp"
#include "coap/mqtt_publisher.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <cstdlib>
#include <iomanip>

namespace {
    volatile std::sig_atomic_t running = 1;
    
    void signal_handler(int signal) {
        if (signal == SIGINT || signal == SIGTERM) {
            running = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    // Disable stdout buffering for immediate Docker logging
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Get configuration from environment variables (required)
    auto get_env_or_exit = [](const char* name) -> std::string {
        const char* value = std::getenv(name);
        if (!value) {
            std::cerr << "ERROR: Required environment variable " << name << " not set\n";
            std::exit(1);
        }
        return std::string(value);
    };
    
    std::string host = get_env_or_exit("COAP_HOST");
    int port = std::atoi(get_env_or_exit("COAP_PORT").c_str());
    std::string mqtt_host = get_env_or_exit("MQTT_BROKER");
    int mqtt_port = std::atoi(get_env_or_exit("MQTT_PORT").c_str());
    std::string topic = get_env_or_exit("MQTT_TOPIC");
    int poll_interval = std::atoi(get_env_or_exit("POLL_INTERVAL").c_str());
    int metrics_port = std::atoi(get_env_or_exit("METRICS_PORT").c_str());
    
    std::cout << "=== CoAP-MQTT Gateway Starting ===\n";
    std::cout << "CoAP Server: " << host << ":" << port << "\n";
    std::cout << "MQTT Broker: " << mqtt_host << ":" << mqtt_port << "\n";
    std::cout << "MQTT Topics: " << topic << "/[temperature|vibration|power|state|uptime]\n";
    std::cout << "Poll Interval: " << poll_interval << " seconds\n";
    std::cout << "Metrics Port: " << metrics_port << "\n";
    std::cout << "===================================\n\n";
    
    // Start metrics server
    coap::MetricsServer metrics_server(metrics_port);
    metrics_server.start();
    
    // Initialize MQTT publisher (single instance for all sensors)
    coap::MqttPublisher mqtt_publisher(mqtt_host, mqtt_port, topic);
    
    // Connect to MQTT broker
    if (!mqtt_publisher.connect()) {
        std::cerr << "Warning: Failed to connect to MQTT broker initially. Will retry on publish.\n";
    }
    
    // Resolve CoAP server hostname
    std::string ip = coap::Client::resolve_hostname(host);
    std::cout << "Resolved " << host << " to " << ip << "\n";
    
    uint16_t message_id = 1;
    
    // Define sensor endpoints
    struct SensorEndpoint {
        std::string path;
        std::string mqtt_topic_suffix;
        std::string name;
    };
    
    std::vector<SensorEndpoint> sensors = {
        {"/machine/temperature", "/temperature", "Temperature"},
        {"/machine/vibration", "/vibration", "Vibration"},
        {"/machine/power", "/power", "Power"},
        {"/machine/state", "/state", "State"},
        {"/machine/uptime", "/uptime", "Uptime"}
    };
    
    // Main polling loop
    while (running) {
        auto now = std::time(nullptr);
        std::cout << "\n=== Polling Cycle [" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "] ===\n";
        
        // Poll each sensor sequentially
        for (const auto& sensor : sensors) {
            try {
                auto start_time = std::chrono::steady_clock::now();
                
                // Connect to CoAP server
                coap::Client client;
                client.connect(ip, port);
                
                // Build GET request
                coap::Message message;
                message.set_type(coap::Type::Confirmable)
                       .set_code(coap::Code::GET())
                       .set_message_id(message_id++)
                       .build_uri_path(sensor.path);
                
                // Send request
                if (!client.send(message)) {
                    std::cerr << "Failed to send CoAP request for " << sensor.name << "\n";
                    coap::Metrics::instance().increment_errors();
                    continue;
                }
                
                coap::Metrics::instance().increment_requests_sent();
                
                // Receive response
                auto response = client.receive();
                coap::Metrics::instance().increment_responses_received();
                
                auto end_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                coap::Metrics::instance().record_response_time(sensor.name, duration.count());
                
                // Extract payload
                const auto& payload = response.get_payload();
                std::string payload_str(payload.begin(), payload.end());
                
                std::cout << sensor.name << ": " << payload_str 
                          << " [" << response.get_header().code.format_code() << ", "
                          << duration.count() << "ms]\n";
                
                // Update metrics gauges
                if (!payload_str.empty()) {
                    try {
                        if (sensor.path == "/machine/temperature") {
                            double temp = std::stod(payload_str);
                            coap::Metrics::instance().set_temperature(temp);
                        } else if (sensor.path == "/machine/vibration") {
                            double vib = std::stod(payload_str);
                            coap::Metrics::instance().set_vibration(vib);
                        } else if (sensor.path == "/machine/power") {
                            double pwr = std::stod(payload_str);
                            coap::Metrics::instance().set_power(pwr);
                        } else if (sensor.path == "/machine/state") {
                            coap::Metrics::instance().set_state(payload_str);
                        } else if (sensor.path == "/machine/uptime") {
                            double uptime = std::stod(payload_str);
                            coap::Metrics::instance().set_uptime(uptime);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing " << sensor.name << " value: " << e.what() << "\n";
                    }
                    
                    // Forward to MQTT with sensor-specific topic
                    std::string sensor_topic = topic + sensor.mqtt_topic_suffix;
                    if (mqtt_publisher.publish_to_topic(sensor_topic, payload_str)) {
                        // Only log first sensor to reduce noise
                        if (sensor.path == "/machine/temperature") {
                            std::cout << "Published to MQTT topics: " << topic << "/[temperature|vibration|power|state|uptime]\n";
                        }
                    }
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error polling " << sensor.name << ": " << e.what() << "\n";
                coap::Metrics::instance().increment_errors();
            }
        }
        
        // Sleep until next poll
        std::this_thread::sleep_for(std::chrono::seconds(poll_interval));
    }
    
    std::cout << "\nShutting down gracefully...\n";
    mqtt_publisher.disconnect();
    metrics_server.stop();
    
    return 0;
}
