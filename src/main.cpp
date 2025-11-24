#include "coap/client.hpp"
#include "coap/message.hpp"
#include "coap/cli.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    coap::CommandLineParser parser(argc, argv);
    
    if (!parser.parse()) {
        std::cerr << "Error: " << parser.get_error() << "\n\n";
        coap::CommandLineParser::print_usage(argv[0]);
        return 1;
    }
    
    const auto& args = parser.get_args();
    
    // Resolve hostname to IP
    std::string ip = coap::Client::resolve_hostname(args.host);
    if (ip.empty()) {
        std::cerr << "Failed to resolve hostname: " << args.host << "\n";
        return 1;
    }
    
    std::cout << "Resolved " << args.host << " to " << ip << "\n";
    
    // Connect to server
    coap::Client client;
    client.connect(ip, args.port);
    
    // Build message
    coap::Message message;
    message.set_type(coap::Type::Confirmable)
           .set_code(args.code)
           .set_message_id(1234)
           .build_uri_path(args.path);
    
    // Add payload if present (for POST/PUT)
    if (args.payload.has_value()) {
        message.set_content_format(coap::ContentFormat::TextPlain)
               .set_payload(args.payload.value());
    }
    
    // Send request
    if (!client.send(message)) {
        std::cerr << "Failed to send request\n";
        return 1;
    }
    
    std::cout << "Sent " << argv[1] << " request to coap://" << args.host 
              << ":" << args.port << args.path << "\n";
    
    // Receive response
    auto response = client.receive();
    std::cout << "\n=== Response ===\n";
    std::cout << "Code: " << response.get_header().code.format_code() << "\n";
    std::cout << "Type: " << static_cast<int>(response.get_header().type) << "\n";
    std::cout << "Message ID: " << response.get_header().message_id << "\n";
    
    // Print payload if present
    const auto& payload = response.get_payload();
    if (!payload.empty()) {
        std::cout << "\nPayload (" << payload.size() << " bytes):\n";
        std::cout << std::string(payload.begin(), payload.end()) << "\n";
    }
    
    return 0;
}
