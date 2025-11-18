#include "coap/client.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace coap {

Client::Client() {
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
}

Client::~Client() {
    close();
}

void Client::connect(const std::string& host, int port) {
    server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server_address.sin_addr);
}

bool Client::send(const Message& message) {
    auto bytes = message.serialize();
    ssize_t sent = sendto(client_socket, bytes.data(), bytes.size(), 0,
                          (struct sockaddr*)&server_address, sizeof(server_address));
    return sent > 0;
}

Message Client::receive() {
    std::vector<std::uint8_t> buffer(1024);
    sockaddr_in from_addr{};
    socklen_t from_len = sizeof(from_addr);
    
    ssize_t recv_len = recvfrom(client_socket, buffer.data(), buffer.size(), 0,
                                (struct sockaddr*)&from_addr, &from_len);
    if (recv_len < 0) {
        std::cerr << "Receive failed\n";
        return Message{};
    }

    buffer.resize(recv_len);
    return Message::parse(buffer);
}

void Client::close() {
    if (client_socket >= 0) {
        ::close(client_socket);
        client_socket = -1;
    }
}

}  // namespace coap
