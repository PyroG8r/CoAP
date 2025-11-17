#include "coap/message.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

int main() {
    coap::Header header;
    header.version = 1;
    header.type = coap::Type::Confirmable;
    header.code = 1; 

    coap::Message message;
    message.set_header(header);
    message.set_token({0x12, 0x34});

    return 0;
}
