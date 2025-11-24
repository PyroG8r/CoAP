#include "coap/option.hpp"
#include <stdexcept>
#include <cstring>

namespace coap {

Option::Option(OptionType number, std::vector<std::uint8_t> value)
    : option_type(number), value(std::move(value)) {}

Option::Option(OptionType type, const std::string &value)
    : option_type(type), value(value.begin(), value.end()) {}

void Option::encode_delta_length(std::uint16_t delta, std::uint16_t length, std::vector<std::uint8_t>& buffer) {
    // Temporarily store extended bytes
    std::vector<std::uint8_t> delta_ext;
    std::vector<std::uint8_t> length_ext;
    
    // Encode delta
    std::uint8_t delta_nibble;
    if (delta < 13) {
        delta_nibble = static_cast<std::uint8_t>(delta);
    } else if (delta < 269) {
        delta_nibble = 13;
        delta_ext.push_back(static_cast<std::uint8_t>(delta - 13));
    } else {
        delta_nibble = 14;
        std::uint16_t extended = delta - 269;
        delta_ext.push_back(static_cast<std::uint8_t>(extended >> 8)); // High byte
        delta_ext.push_back(static_cast<std::uint8_t>(extended & 0xFF)); // Low byte
    }
    
    // Encode length
    std::uint8_t length_nibble;
    if (length < 13) {
        length_nibble = static_cast<std::uint8_t>(length);
    } else if (length < 269) {
        length_nibble = 13;
        length_ext.push_back(static_cast<std::uint8_t>(length - 13));
    } else {
        length_nibble = 14;
        std::uint16_t extended = length - 269;
        length_ext.push_back(static_cast<std::uint8_t>(extended >> 8)); // High byte
        length_ext.push_back(static_cast<std::uint8_t>(extended & 0xFF)); // Low byte
    }
    
    // Write header byte
    buffer.push_back((delta_nibble << 4) | length_nibble); // Combine nibbles
    
    // Write extended bytes
    buffer.insert(buffer.end(), delta_ext.begin(), delta_ext.end()); // Append delta extended bytes
    buffer.insert(buffer.end(), length_ext.begin(), length_ext.end()); // Append length extended bytes
}

std::vector<std::uint8_t> Option::encode(const std::vector<Option>& options, std::uint16_t previous_number) {
    std::vector<std::uint8_t> buffer;
    
    for (const auto& option : options) {
        std::uint16_t option_num = static_cast<std::uint16_t>(option.option_type);
        std::uint16_t delta = option_num - previous_number;
        std::uint16_t length = option.value.size();
        
        encode_delta_length(delta, length, buffer);
        buffer.insert(buffer.end(), option.value.begin(), option.value.end());
        
        previous_number = option_num;
    }
    
    return buffer;
}

std::uint16_t Option::decode_nibble_value(std::uint8_t nibble, 
                                           const std::vector<std::uint8_t>& buffer, 
                                           std::size_t& offset) {
    if (nibble < 13) {
        return nibble;
    } else if (nibble == 13) {
        return buffer[offset++] + 13;   // Read one byte
    } else if (nibble == 14) {
        std::uint16_t val = (static_cast<std::uint16_t>(buffer[offset]) << 8) | buffer[offset + 1]; // Read two bytes
        offset += 2;
        return val + 269;
    } 
}

std::pair<std::uint16_t, std::uint16_t> Option::decode_delta_length(
    const std::vector<std::uint8_t>& buffer, std::size_t& offset) {
    
    std::uint8_t header = buffer[offset++];             // Read header byte
    std::uint8_t delta_nibble = (header >> 4) & 0x0F;   // High nibble
    std::uint8_t length_nibble = header & 0x0F;         // Low nibble
    
    return {decode_nibble_value(delta_nibble, buffer, offset), 
            decode_nibble_value(length_nibble, buffer, offset)};
}

std::vector<Option> Option::decode(
    const std::vector<std::uint8_t>& buffer, std::size_t start, std::size_t end) {
    
    std::vector<Option> options;
    std::size_t offset = start;
    std::uint16_t current_number = 0;
    
    while (offset < end && buffer[offset] != 0xFF) { // 0xFF is payload marker
        auto [delta, length] = decode_delta_length(buffer, offset);
        
        current_number += delta;
        
        std::vector<std::uint8_t> value(
            buffer.begin() + offset,
            buffer.begin() + offset + length
        );
        offset += length;
        
        options.emplace_back(static_cast<OptionType>(current_number), std::move(value));
    }
    
    return options;
}

}
