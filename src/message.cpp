#include "coap/message.hpp"

#include <cstring>
#include <stdexcept>

namespace coap {

namespace {
constexpr std::size_t kHeaderSize = 4;

std::uint8_t encode_first_byte(const Header& header) {
    std::uint8_t value = 0;
    return value;
}

Header decode_header(const std::vector<std::uint8_t>& buffer) {
    Header header;
    return header;
}

} 

void Message::set_header(Header header) {
    header = header;
}

const Header& Message::get_header() const {
    return header;
}
void Message::set_token(std::vector<std::uint8_t> token) {
    token = std::move(token);
}

const std::vector<std::uint8_t>& Message::get_token() const {
    return token;
}

void Message::set_payload(std::vector<std::uint8_t> payload) {
    payload = std::move(payload);
}

const std::vector<std::uint8_t>& Message::get_payload() const {
    return payload;
}

std::vector<std::uint8_t> Message::serialize() const {
    std::vector<std::uint8_t> buffer;
    return buffer;
}

Message Message::parse(const std::vector<std::uint8_t>& buffer) {
    Message message;
    return message;
}

}  // namespace coap
