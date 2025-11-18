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
    header.type = coap::Type::NonConfirmable;
    header.code = 1; 
    header.message_id = 12345;

    coap::Message message;
    message.set_header(header);

    
    return 0;
}
