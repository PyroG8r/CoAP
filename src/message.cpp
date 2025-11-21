#include "coap/message.hpp"

#include <cstring>
#include <stdexcept>

namespace coap {

std::uint8_t Message::encode_first_byte(const Header& header) {
    std::uint8_t value = 0;
    value |= (header.version & 0x03) << 6;
    value |= (static_cast<std::uint8_t>(header.type) & 0x03) << 4;
    value |= (header.token_length & 0x0F);
    return value;
}

/* 
    * Decodes the header from the given buffer.
*/ 
Header Message::decode_header(const std::vector<std::uint8_t>& buffer) {
    Header header;
    std::uint8_t first_byte = buffer[0];
    header.version = (first_byte >> 6) & 0x03;
    header.type = static_cast<Type>((first_byte >> 4) & 0x03);
    header.token_length = first_byte & 0x0F;
    header.code = Code(buffer[1]);
    header.message_id = (static_cast<std::uint16_t>(buffer[2]) << 8) | buffer[3];
    return header;
} 

std::vector<std::uint8_t> Message::serialize() const {
    std::vector<std::uint8_t> buffer;

    buffer.push_back(encode_first_byte(header));
    buffer.push_back(header.code.raw());

    buffer.push_back(static_cast<std::uint8_t>((header.message_id >> 8) & 0xFF));
    buffer.push_back(static_cast<std::uint8_t>(header.message_id & 0xFF));

    buffer.insert(buffer.end(), token.begin(), token.end());

    auto options_encoded = Option::encode(options);
    buffer.insert(buffer.end(), options_encoded.begin(), options_encoded.end());

    if (!payload.empty()) {
        buffer.push_back(0xFF); // Payload marker
        buffer.insert(buffer.end(), payload.begin(), payload.end());
    }
    return buffer;
}

Message Message::parse(const std::vector<std::uint8_t>& buffer) {
    Message message;
    message.header = decode_header(buffer);
    if(message.header.token_length > 0) {
        message.token = std::vector<std::uint8_t>(
            buffer.begin() + 4,
            buffer.begin() + 4 + message.header.token_length);
    }
    const auto options_start = 4 + message.header.token_length;

    std::size_t payload_marker = buffer.size();
    for (std::size_t i = options_start; i < buffer.size(); ++i) {
        if (buffer[i] == 0xFF) {
            payload_marker = i;
            break;
        }
    }
    const auto options_end = payload_marker;
    
    // Decode options if present
    if (options_start < options_end) {
        message.options = Option::decode(buffer, options_start, options_end);
    }
    
    if (payload_marker < buffer.size()) {
        const auto payload_start = payload_marker + 1;
        if (payload_start < buffer.size()) {
            message.payload = std::vector<std::uint8_t>(
                buffer.begin() + payload_start,
                buffer.end());
        }
    }
    
    return message;

}

Message& Message::set_type(Type type) {
    header.type = type;
    return *this;
}

Message& Message::set_code(Code code) {
    header.code = code;
    return *this;
}

Message& Message::set_message_id(std::uint16_t id) {
    header.message_id = id;
    return *this;
}

Message& Message::add_token(std::vector<std::uint8_t> token) {
    this->token = std::move(token);
    header.token_length = this->token.size();
    return *this;
}

Message& Message::add_option(Option option) {
    options.push_back(std::move(option));
    return *this;
}

Message& Message::add_uri_path(const std::string& uri_segment) {
    options.emplace_back(OptionType::UriPath, uri_segment);
    return *this;
}

Message& Message::set_payload(std::vector<std::uint8_t> payload) {
    this->payload = std::move(payload);
    return *this;
}

Message& Message::set_payload(const std::string& payload) {
    this->payload.assign(payload.begin(), payload.end());
    return *this;
}

}  // namespace coap
