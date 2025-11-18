#pragma once

#include "message.hpp"

#include <netinet/in.h>
#include <string>

namespace coap {

class Client {
public:
    Client();
    ~Client();

    void connect(const std::string& host, int port);
    bool send(const Message& message);
    Message receive();
    void close();

private:
    int client_socket {-1};
    sockaddr_in server_address {};
};

}  // namespace coap
