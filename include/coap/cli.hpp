#pragma once

#include "coap/code.hpp"

#include <string>
#include <vector>
#include <optional>

namespace coap {

struct CommandLineArgs {
    Code code {Code::GET()};
    std::string host;
    int port {5683};
    std::string path;
    std::optional<std::string> payload;
};

class CommandLineParser {
public:
    CommandLineParser(int argc, char* argv[]);
    
    bool parse();
    const CommandLineArgs& get_args() const { return args; }
    std::string get_error() const { return error_message; }
    
    static void print_usage(const char* program_name);

private:
    int argc;
    char** argv;
    CommandLineArgs args;
    std::string error_message;
    
    bool parse_method(const std::string& method_str);
    bool parse_host_and_path(const std::string& uri);
};

}  // namespace coap
