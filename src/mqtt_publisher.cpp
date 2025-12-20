#include "coap/mqtt_publisher.hpp"
#include "coap/metrics.hpp"

#include <iostream>
#include <cstdlib>
#include <sstream>

namespace coap {

MqttPublisher::MqttPublisher(const std::string& broker_host, 
                             int broker_port,
                             const std::string& topic)
    : broker_host(broker_host), broker_port(broker_port), topic(topic) {}

bool MqttPublisher::publish(const std::string& message) {
    // Build mosquitto_pub command
    std::ostringstream cmd;
    cmd << "mosquitto_pub -h " << broker_host 
        << " -p " << broker_port
        << " -t " << topic
        << " -m '" << message << "' 2>/dev/null";
    
    int result = std::system(cmd.str().c_str());
    
    if (result == 0) {
        Metrics::instance().increment_mqtt_published();
        return true;
    } else {
        std::cerr << "Failed to publish MQTT message (exit code: " << result << ")\n";
        Metrics::instance().increment_errors();
        return false;
    }
}

void MqttPublisher::set_broker(const std::string& host, int port) {
    broker_host = host;
    broker_port = port;
}

void MqttPublisher::set_topic(const std::string& new_topic) {
    topic = new_topic;
}

}  // namespace coap
