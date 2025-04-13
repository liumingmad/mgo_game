#include "server_push.h"
#include <protocol.h>
#include <wrap.h>

long get_push_serial_number() {
    return g_server_push_serial_number++;
}

int ServerPusher::server_push(int fd, PushMessage message)
{
    nlohmann::json j = message;
    std::string json = j.dump();

    ProtocolWriter pw;
    u_int8_t *buf = pw.wrap_push_header_buffer(get_push_serial_number(), json);
    Write(fd, buf, HEADER_SIZE + json.length());
}