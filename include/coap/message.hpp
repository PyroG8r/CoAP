#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "coap/code.hpp"

namespace coap {

enum class Type : std::uint8_t {
    Confirmable = 0,
    NonConfirmable = 1,
    Acknowledgement = 2,
    Reset = 3
};

// CoAP message header structure, 4 bytes
struct Header {
    std::uint8_t version {1};
    Type type {Type::Confirmable};
    std::uint8_t token_length {0};
    Code code {Code::GET()};
    std::uint16_t message_id {0};
};

class Message {
public:
    Message() = default;

    void set_header(Header header);
    const Header& get_header() const;

    void set_token(std::vector<std::uint8_t> token);
    const std::vector<std::uint8_t>& get_token() const;
    void set_payload(std::vector<std::uint8_t> payload);
    const std::vector<std::uint8_t>& get_payload() const;

    std::vector<std::uint8_t> serialize() const;
    static Message parse(const std::vector<std::uint8_t>& buffer);

private:
    static std::uint8_t encode_first_byte(const Header& header);
    static Header decode_header(const std::vector<std::uint8_t>& buffer);

    Header header {};
    std::vector<std::uint8_t> token {};
    std::vector<std::uint8_t> payload {};
};

}  // namespace coap
