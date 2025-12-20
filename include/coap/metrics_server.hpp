#pragma once

#include <string>
#include <thread>
#include <atomic>

namespace coap {

class MetricsServer {
public:
    MetricsServer(int port = 9090);
    ~MetricsServer();
    
    void start();
    void stop();

private:
    void run();
    void handle_client(int client_socket);
    
    int port;
    int server_socket{-1};
    std::atomic<bool> running{false};
    std::thread server_thread;
};

}  // namespace coap
