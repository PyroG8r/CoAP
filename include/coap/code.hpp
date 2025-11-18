#pragma once
#include <cstdint>
#include <string>

namespace coap {

class Code {
public:
    // Common method codes
    // Request Codes
    static constexpr Code GET() { return Code(0, 1); }
    static constexpr Code POST() { return Code(0, 2); }
    static constexpr Code PUT() { return Code(0, 3); }
    static constexpr Code DELETE() { return Code(0, 4); }

    // Response Codes
    static constexpr Code OK() { return Code(2, 0); }
    static constexpr Code Created() { return Code(2, 1); }
    static constexpr Code Deleted() { return Code(2, 2); }
    static constexpr Code Valid() { return Code(2, 3); }
    static constexpr Code Content() { return Code(2, 5); }

    // Client Error Codes
    static constexpr Code BadRequest() { return Code(4, 0); }
    static constexpr Code BadOption() { return Code(4, 2); }
    static constexpr Code NotFound() { return Code(4, 4); }

    // Server Error Codes
    static constexpr Code InternalServerError() { return Code(5, 0); }

    
    constexpr Code(std::uint8_t code_class, std::uint8_t detail)
        : messageType((code_class << 5) | detail) {}
    
    constexpr explicit Code(std::uint8_t raw) : messageType(raw) {}
    
    constexpr std::uint8_t code_class() const { return (messageType >> 5) & 0x07; }
    constexpr std::uint8_t detail() const { return messageType & 0x1F; }
    constexpr std::uint8_t raw() const { return messageType; }
    
    constexpr bool is_request() const { return code_class() == 0; }
    constexpr bool is_success() const { return code_class() == 2; }
    constexpr bool is_client_error() const { return code_class() == 4; }
    constexpr bool is_server_error() const { return code_class() == 5; }
    
    std::string format_code() const {
        return std::to_string(code_class()) + "." + 
               (detail() < 10 ? "0" : "") + std::to_string(detail());
    }

private:
    std::uint8_t messageType;
};

}  // namespace coap