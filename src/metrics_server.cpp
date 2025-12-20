#include "coap/metrics_server.hpp"
#include "coap/metrics.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace coap {

MetricsServer::MetricsServer(int port) : port(port) {}

MetricsServer::~MetricsServer() {
    stop();
}

void MetricsServer::start() {
    if (running.load()) {
        return;
    }
    
    running = true;
    server_thread = std::thread(&MetricsServer::run, this);
}

void MetricsServer::stop() {
    if (!running.load()) {
        return;
    }
    
    running = false;
    
    if (server_socket >= 0) {
        ::close(server_socket);
        server_socket = -1;
    }
    
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void MetricsServer::run() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create metrics server socket\n";
        return;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind metrics server to port " << port << "\n";
        ::close(server_socket);
        return;
    }
    
    if (listen(server_socket, 5) < 0) {
        std::cerr << "Failed to listen on metrics server\n";
        ::close(server_socket);
        return;
    }
    
    std::cout << "Metrics server listening on port " << port << "\n";
    
    while (running.load()) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running.load()) {
                std::cerr << "Error accepting metrics client connection\n";
            }
            continue;
        }
        
        handle_client(client_socket);
        ::close(client_socket);
    }
}

void MetricsServer::handle_client(int client_socket) {
    // Read HTTP request (we don't really parse it, just respond)
    char buffer[1024];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        return;
    }
    
    // Get metrics in Prometheus format
    std::string metrics = Metrics::instance().export_prometheus();
    
    // Send HTTP response
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; version=0.0.4\r\n"
        "Content-Length: " + std::to_string(metrics.length()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + metrics;
    
    send(client_socket, response.c_str(), response.length(), 0);
}

}  // namespace coap
