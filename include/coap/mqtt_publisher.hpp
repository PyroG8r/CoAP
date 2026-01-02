#pragma once

#include <string>
#include <MQTTClient.h>

namespace coap {

class MqttPublisher {
public:
    MqttPublisher(const std::string& broker_host = "localhost", 
                  int broker_port = 1883,
                  const std::string& topic = "coap/data");
    ~MqttPublisher();
    
    bool connect();
    bool publish(const std::string& message);
    bool publish_to_topic(const std::string& topic, const std::string& message);
    void disconnect();
    bool is_connected() const;
    
    void set_broker(const std::string& host, int port);
    void set_topic(const std::string& topic);

private:
    std::string broker_host;
    int broker_port;
    std::string topic;
    MQTTClient client;
    bool connected;
    std::string client_id;
};

}  // namespace coap
