#pragma once

#include <string>

namespace coap {

class MqttPublisher {
public:
    MqttPublisher(const std::string& broker_host = "localhost", 
                  int broker_port = 1883,
                  const std::string& topic = "coap/data");
    
    bool publish(const std::string& message);
    
    void set_broker(const std::string& host, int port);
    void set_topic(const std::string& topic);

private:
    std::string broker_host;
    int broker_port;
    std::string topic;
};

}  // namespace coap
