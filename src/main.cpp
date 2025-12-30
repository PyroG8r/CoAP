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
    
    // Get configuration from environment variables
    const char* coap_host = std::getenv("COAP_HOST");
    const char* coap_port_str = std::getenv("COAP_PORT");
    const char* coap_path = std::getenv("COAP_PATH");
    const char* mqtt_broker = std::getenv("MQTT_BROKER");
    const char* mqtt_port_str = std::getenv("MQTT_PORT");
    const char* mqtt_topic = std::getenv("MQTT_TOPIC");
    const char* poll_interval_str = std::getenv("POLL_INTERVAL");
    const char* metrics_port_str = std::getenv("METRICS_PORT");
    
    // Set defaults
    std::string host = coap_host ? coap_host : "localhost";
    int port = coap_port_str ? std::atoi(coap_port_str) : 5683;
    std::string path = coap_path ? coap_path : "/";
    std::string mqtt_host = mqtt_broker ? mqtt_broker : "localhost";
    int mqtt_port = mqtt_port_str ? std::atoi(mqtt_port_str) : 1883;
    std::string topic = mqtt_topic ? mqtt_topic : "coap/data";
    int poll_interval = poll_interval_str ? std::atoi(poll_interval_str) : 10;
    int metrics_port = metrics_port_str ? std::atoi(metrics_port_str) : 9090;
    
    std::cout << "=== CoAP-MQTT Gateway Starting ===\n";
    std::cout << "CoAP Server: " << host << ":" << port << path << "\n";
    std::cout << "MQTT Broker: " << mqtt_host << ":" << mqtt_port << "\n";
    std::cout << "MQTT Topic: " << topic << "\n";
    std::cout << "Poll Interval: " << poll_interval << " seconds\n";
    std::cout << "Metrics Port: " << metrics_port << "\n";
    std::cout << "===================================\n\n";
    
    // Start metrics server
    coap::MetricsServer metrics_server(metrics_port);
    metrics_server.start();
    
    // Initialize MQTT publisher
    coap::MqttPublisher mqtt_publisher(mqtt_host, mqtt_port, topic);
    
    // Resolve CoAP server hostname
    std::string ip = coap::Client::resolve_hostname(host);
    std::cout << "Resolved " << host << " to " << ip << "\n";
    
    uint16_t message_id = 1;
    
    // Main polling loop
    while (running) {
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
                   .build_uri_path(path);
            
            // Send request
            if (!client.send(message)) {
                std::cerr << "Failed to send CoAP request\n";
                coap::Metrics::instance().increment_errors();
                std::this_thread::sleep_for(std::chrono::seconds(poll_interval));
                continue;
            }
            
            coap::Metrics::instance().increment_requests_sent();
            
            // Receive response
            auto response = client.receive();
            coap::Metrics::instance().increment_responses_received();
            
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            coap::Metrics::instance().record_response_time(duration.count());
            
            // Extract payload
            const auto& payload = response.get_payload();
            std::string payload_str(payload.begin(), payload.end());
            
            std::cout << "[" << std::time(nullptr) << "] Received: " 
                      << response.get_header().code.format_code() 
                      << " (" << payload.size() << " bytes)\n";
            
            // Forward to MQTT if we have data
            if (!payload_str.empty()) {
                if (mqtt_publisher.publish(payload_str)) {
                    std::cout << "Published to MQTT topic: " << topic << "\n";
                }
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error in polling loop: " << e.what() << "\n";
            coap::Metrics::instance().increment_errors();
        }
        
        // Sleep until next poll
        std::this_thread::sleep_for(std::chrono::seconds(poll_interval));
    }
    
    std::cout << "\nShutting down gracefully...\n";
    metrics_server.stop();
    
    return 0;
}
