#include "coap/client.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#include <cstring>
#include <iostream>

namespace coap {

std::string Client::resolve_hostname(const std::string& hostname) {
    struct addrinfo hints{}, *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    if (getaddrinfo(hostname.c_str(), nullptr, &hints, &result) != 0) {
        return "";
    }
    
    char ip_str[INET_ADDRSTRLEN];
    struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
    inet_ntop(AF_INET, &addr->sin_addr, ip_str, INET_ADDRSTRLEN);
    
    freeaddrinfo(result);
    return std::string(ip_str);
}

Client::Client() {
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Set receive timeout to 5 seconds
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
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
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "Receive timeout - no response from server\n";
        } else {
            std::cerr << "Receive failed: " << strerror(errno) << "\n";
        }
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
