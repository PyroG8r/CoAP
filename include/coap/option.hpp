#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace coap {

// CoAP option numbers (RFC 7252)
enum class OptionType : std::uint16_t {
    IfMatch = 1,
    UriHost = 3,
    ETag = 4,
    IfNoneMatch = 5,
    UriPort = 7,
    LocationPath = 8,
    UriPath = 11,
    ContentFormat = 12,
    MaxAge = 14,
    UriQuery = 15,
    Accept = 17,
    LocationQuery = 20,
    ProxyUri = 35,
    ProxyScheme = 39,
    Size1 = 60
};
// Content format codes
enum class ContentFormat : std::uint16_t {
    TextPlain = 0,
    ApplicationLinkFormat = 40,
    ApplicationXml = 41,
    ApplicationOctetStream = 42,
    ApplicationExi = 47,
    ApplicationJson = 50
};

class Option {
public:
    Option(OptionType type, std::vector<std::uint8_t> value);

    OptionType get_type() const { return option_type; }
    const std::vector<std::uint8_t>& get_value() const { return value; }

    static std::vector<std::uint8_t> encode(const std::vector<Option>& options, std::uint16_t previous_number = 0);

    static std::vector<Option> decode(const std::vector<std::uint8_t>& buffer, std::size_t start, std::size_t end);

private:
    OptionType option_type;
    std::vector<std::uint8_t> value;

    static void encode_delta_length(std::uint16_t delta, std::uint16_t length, std::vector<std::uint8_t>& buffer);
    static std::pair<std::uint16_t, std::uint16_t> decode_delta_length(const std::vector<std::uint8_t>& buffer, std::size_t& offset);
};

}