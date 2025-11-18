#include "coap/option.hpp"
#include <stdexcept>
#include <cstring>

namespace coap {

Option::Option(OptionType number, std::vector<std::uint8_t> value)
    : option_type(number), value(std::move(value)) {}



}

