#include "coap/client.hpp"
#include "coap/message.hpp"

#include <iostream>

int main() {
    coap::Client client;
    client.connect("134.102.218.18", 5683);

    // Build GET request for /.well-known/core
    coap::Message message;
    message.set_type(coap::Type::Confirmable)
           .set_code(coap::Code::GET())
           .set_message_id(1234)
           .add_uri_path(".well-known")
           .add_uri_path("core");

    if (!client.send(message)) {
        std::cerr << "Send failed\n";
        return 1;
    }

    std::cout << "Sent GET request to coap://coap.me/.well-known/core\n";

    auto response = client.receive();
    std::cout << "Received response, code: " 
              << response.get_header().code.format_code() << "\n";
    
    // Print payload if present
    const auto& payload = response.get_payload();
    if (!payload.empty()) {
        std::cout << "Payload:\n" << std::string(payload.begin(), payload.end()) << "\n";
    }

    return 0;
}
