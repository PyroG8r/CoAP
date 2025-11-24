#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "coap/code.hpp"
#include "coap/option.hpp"

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

    // Builder-style methods (return *this for chaining)
    Message& set_type(Type type);
    Message& set_code(Code code);
    Message& set_message_id(std::uint16_t id);
    Message& add_token(std::vector<std::uint8_t> token);
    Message& add_option(Option option);
    Message& add_uri_path(const std::string& uri_segment);
    Message& build_uri_path(const std::string& path);
    Message& set_content_format(ContentFormat format);
    Message& set_payload(std::vector<std::uint8_t> payload);
    Message& set_payload(const std::string& payload);

    // Const accessors
    const Header& get_header() const { return header; }
    const std::vector<std::uint8_t>& get_token() const { return token; }
    const std::vector<Option>& get_options() const { return options; }
    const std::vector<std::uint8_t>& get_payload() const { return payload; }

    std::vector<std::uint8_t> serialize() const;
    static Message parse(const std::vector<std::uint8_t>& buffer);

private:
    static std::uint8_t encode_first_byte(const Header& header);
    static Header decode_header(const std::vector<std::uint8_t>& buffer);

    Header header {};
    std::vector<std::uint8_t> token {};
    std::vector<Option> options {};
    std::vector<std::uint8_t> payload {};
};

}  // namespace coap
