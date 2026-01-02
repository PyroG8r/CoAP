#include "coap/mqtt_publisher.hpp"
#include "coap/metrics.hpp"

#include <iostream>
#include <sstream>
#include <chrono>
#include <cstring>

namespace coap {

MqttPublisher::MqttPublisher(const std::string& broker_host, 
                             int broker_port,
                             const std::string& topic)
    : broker_host(broker_host), broker_port(broker_port), topic(topic), 
      client(nullptr), connected(false) {
    
    // Generate unique client ID with timestamp
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    client_id = "coap_gateway_" + std::to_string(timestamp);
}

MqttPublisher::~MqttPublisher() {
    disconnect();
}

bool MqttPublisher::connect() {
    if (connected) {
        return true;
    }
    
    // Build broker URI
    std::ostringstream uri;
    uri << "tcp://" << broker_host << ":" << broker_port;
    
    // Create MQTT client
    int rc = MQTTClient_create(&client, uri.str().c_str(), client_id.c_str(),
                               MQTTCLIENT_PERSISTENCE_NONE, nullptr);
    if (rc != MQTTCLIENT_SUCCESS) {
        std::cerr << "Failed to create MQTT client: " << rc << "\n";
        return false;
    }
    
    // Set connection options
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.connectTimeout = 5;
    
    // Connect to broker
    rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        static bool error_logged = false;
        if (!error_logged) {
            std::cerr << "Failed to connect to MQTT broker " << uri.str() 
                      << " (error: " << rc << ")\n";
            error_logged = true;
        }
        MQTTClient_destroy(&client);
        client = nullptr;
        return false;
    }
    
    connected = true;
    std::cout << "Connected to MQTT broker: " << uri.str() << "\n";
    return true;
}

void MqttPublisher::disconnect() {
    if (client && connected) {
        MQTTClient_disconnect(client, 1000);  // 1 second timeout
        MQTTClient_destroy(&client);
        client = nullptr;
        connected = false;
        std::cout << "Disconnected from MQTT broker\n";
    }
}

bool MqttPublisher::is_connected() const {
    return connected && client != nullptr && MQTTClient_isConnected(client);
}

bool MqttPublisher::publish(const std::string& message) {
    return publish_to_topic(topic, message);
}

bool MqttPublisher::publish_to_topic(const std::string& pub_topic, const std::string& message) {
    // Ensure we're connected
    if (!is_connected()) {
        if (!connect()) {
            Metrics::instance().increment_mqtt_errors();
            return false;
        }
    }
    
    // Create message
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = const_cast<char*>(message.c_str());
    pubmsg.payloadlen = message.length();
    pubmsg.qos = 0;  // QoS 0 - at most once delivery
    pubmsg.retained = 0;
    
    // Publish message
    MQTTClient_deliveryToken token;
    int rc = MQTTClient_publishMessage(client, pub_topic.c_str(), &pubmsg, &token);
    
    if (rc != MQTTCLIENT_SUCCESS) {
        static int error_count = 0;
        if (error_count < 3) {
            std::cerr << "Failed to publish to " << pub_topic 
                      << " (error: " << rc << ")\n";
            error_count++;
            if (error_count == 3) {
                std::cerr << "Suppressing further MQTT publish errors...\n";
            }
        }
        connected = false;  // Mark as disconnected to retry connection
        Metrics::instance().increment_mqtt_errors();
        return false;
    }
    
    // Wait for message to be delivered (with short timeout for QoS 0)
    rc = MQTTClient_waitForCompletion(client, token, 100);  // 100ms timeout
    
    if (rc == MQTTCLIENT_SUCCESS) {
        Metrics::instance().increment_mqtt_published();
        return true;
    } else {
        Metrics::instance().increment_mqtt_errors();
        return false;
    }
}

void MqttPublisher::set_broker(const std::string& host, int port) {
    bool was_connected = connected;
    if (was_connected) {
        disconnect();
    }
    
    broker_host = host;
    broker_port = port;
    
    if (was_connected) {
        connect();
    }
}

void MqttPublisher::set_topic(const std::string& new_topic) {
    topic = new_topic;
}

}  // namespace coap
