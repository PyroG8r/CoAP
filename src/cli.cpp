#include "coap/cli.hpp"

#include <iostream>
#include <algorithm>

namespace coap {

CommandLineParser::CommandLineParser(int argc, char* argv[])
    : argc(argc), argv(argv) {}

bool CommandLineParser::parse_method(const std::string& method_str) {
    std::string upper_method = method_str;
    std::transform(upper_method.begin(), upper_method.end(), 
                   upper_method.begin(), ::toupper);
    
    if (upper_method == "GET") {
        args.code = Code::GET();
    } else if (upper_method == "POST") {
        args.code = Code::POST();
    } else if (upper_method == "PUT") {
        args.code = Code::PUT();
    } else if (upper_method == "DELETE") {
        args.code = Code::DELETE();
    } else {
        error_message = "Unknown method: " + method_str;
        return false;
    }
    
    return true;
}

bool CommandLineParser::parse_host_and_path(const std::string& uri) {
    std::string processed_uri = uri;
    
    // Remove coap:// prefix if present
    if (processed_uri.find("coap://") == 0) {
        processed_uri = processed_uri.substr(7);
    }
    
    // Find first slash to separate host from path
    size_t slash_pos = processed_uri.find('/');
    
    if (slash_pos == std::string::npos) {
        // No path specified, just host
        args.host = processed_uri;
        args.path = "/";
    } else {
        args.host = processed_uri.substr(0, slash_pos);
        args.path = processed_uri.substr(slash_pos);
    }
    
    // Check for port in host (host:port)
    size_t colon_pos = args.host.find(':');
    if (colon_pos != std::string::npos) {
        std::string port_str = args.host.substr(colon_pos + 1);
        args.host = args.host.substr(0, colon_pos);
        
        try {
            args.port = std::stoi(port_str);
        } catch (...) {
            error_message = "Invalid port: " + port_str;
            return false;
        }
    }
    
    return true;
}

bool CommandLineParser::parse() {
    // Minimum: program_name METHOD HOST [PATH] [PAYLOAD]
    if (argc < 3) {
        error_message = "Too few arguments";
        return false;
    }
    
    // Parse method
    if (!parse_method(argv[1])) {
        return false;
    }
    
    // Parse host/path
    if (!parse_host_and_path(argv[2])) {
        return false;
    }
    
    // Parse optional path (if provided as separate argument)
    if (argc >= 4 && argv[3][0] == '/') {
        args.path = argv[3];
        
        // Parse optional payload
        if (argc >= 5) {
            args.payload = argv[4];
        }
    } else if (argc >= 4) {
        // Third argument is payload (path was in URI)
        args.payload = argv[3];
    }
    
    return true;
}

void CommandLineParser::print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " METHOD HOST[:PORT][/PATH] [PAYLOAD]\n\n"
              << "Methods:\n"
              << "  GET       Retrieve resource\n"
              << "  POST      Create/update resource with payload\n"
              << "  PUT       Update resource with payload\n"
              << "  DELETE    Remove resource\n\n"
              << "Examples:\n"
              << "  " << program_name << " GET coap.me /test\n"
              << "  " << program_name << " GET coap.me:5683/test\n"
              << "  " << program_name << " GET coap.me /.well-known/core\n"
              << "  " << program_name << " POST coap.me /sink \"temperature=22\"\n"
              << "  " << program_name << " PUT coap.me:5683 /hello \"new data\"\n"
              << "  " << program_name << " DELETE coap.me /resource\n";
}

}  // namespace coap
