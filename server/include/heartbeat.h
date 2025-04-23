#include <protocol.h>
#include "wrap.h"
#ifndef HEARTBEAT_H
#define HEARTBEAT_H

long g_heartbeat_serial_number = 0;

void response_heartbeat(int fd) {
    std::string text = "pong";
    ProtocolWriter pw;
    u_int8_t buf[HEADER_SIZE + text.length()];
    pw.wrap_heartbeat_header_buffer(buf, g_heartbeat_serial_number++, text);
    Write(fd, buf, HEADER_SIZE + text.length());
}

#endif //HEARTBEAT_H