#include "coap/client.hpp"
#include "coap/message.hpp"

#include <iostream>

int main() {
    coap::Client client;
    
    if (!client.connect("134.102.218.18", 5683)) {
        return 1;
    }

    // Build a simple GET request
    coap::Header header;
    header.version = 1;
    header.type = coap::Type::Confirmable;
    header.token_length = 0;
    header.code = 1;  // GET
    header.message_id = 1234;

    coap::Message message;
    message.set_header(header);

    if (!client.send(message)) {
        std::cerr << "Send failed\n";
        return 1;
    }

    std::cout << "Sent GET request to coap.me\n";

    auto response = client.receive();
    std::cout << "Received response, code: " 
              << static_cast<int>(response.get_header().code) << "\n";

    return 0;
}
