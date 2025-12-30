# CoAP Client

A lightweight CoAP (Constrained Application Protocol) client implementation in C++17 that supports standard CoAP methods over UDP.

## Features

- **CoAP Protocol Support**: Implements RFC 7252 CoAP protocol
- **HTTP Methods**: Supports GET, POST, PUT, DELETE operations
- **URI Options**: Automatic URI-Path option handling
- **Content Formats**: Support for text/plain and other content types
- **Hostname Resolution**: DNS resolution for CoAP server addresses
- **UDP Transport**: Native UDP socket implementation
- **Command-Line Interface**: Simple CLI for sending CoAP requests

## Building

The project uses CMake with Ninja as the build system.

```bash
# Configure
cmake -S . -B build -G Ninja

# Build
cmake --build build

# Run tests (if available)
cd build && ctest
```

## Usage

```bash
./build/coap_client <METHOD> <HOST> <PATH> [OPTIONS]
```

### Examples

```bash
# Send a GET request
./build/coap_client GET coap.me /hello

# Send a GET request with custom port
./build/coap_client GET coap.me /test -port 5683

# Send a POST request with payload
./build/coap_client POST example.com /api/data -p "Hello, CoAP!"

# Send a PUT request
./build/coap_client PUT example.com /resource -p '{"key":"value"}'

# Send a DELETE request
./build/coap_client DELETE example.com /resource/123
```

### Command-Line Options

- `<METHOD>`: CoAP method (GET, POST, PUT, DELETE)
- `<HOST>`: CoAP server hostname or IP address
- `<PATH>`: URI path (must start with /)
- `-p, --payload <DATA>`: Request payload (for POST/PUT)
- `-port <PORT>`: Server port (default: 5683)

## Project Structure

```
CoAP/
├── include/coap/       # Header files
│   ├── client.hpp      # CoAP client implementation
│   ├── message.hpp     # CoAP message format
│   ├── option.hpp      # CoAP options
│   ├── code.hpp        # CoAP response codes
│   └── cli.hpp         # Command-line parser
├── src/                # Source files
│   ├── main.cpp        # Application entry point
│   ├── client.cpp      # UDP client & DNS resolution
│   ├── message.cpp     # Message serialization
│   ├── option.cpp      # Option encoding
│   └── cli.cpp         # CLI argument parsing
└── CMakeLists.txt      # Build configuration
```

## Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.16+
- Ninja build system
- Linux (uses POSIX sockets)

## Development

Enable compiler warnings during development:

```bash
cmake -S . -B build -G Ninja -DCOAP_ENABLE_WARNINGS=ON
cmake --build build
```
