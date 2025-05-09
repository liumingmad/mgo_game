#include "server_push.h"
#include <protocol.h>
#include <wrap.h>
#include "global.h"

long get_push_serial_number() {
    return g_server_push_serial_number++;
}

int ServerPusher::server_push(std::string uid, std::shared_ptr<PushMessage> message)
{
    auto it = g_uidClientMap.find(uid);
    if (it == g_uidClientMap.end()) {
        std::cout << uid << " not found" << std::endl;
        return 1;
    }

    int fd = it->second->fd;

    nlohmann::json j = *message;
    std::string json = j.dump();

    std::cout << std::endl << json << std::endl;

    ProtocolWriter pw;
    uint8_t buf[HEADER_SIZE + json.length()];
    pw.wrap_push_header_buffer(buf, get_push_serial_number(), json);
    Write(fd, buf, HEADER_SIZE + json.length());
}

int writeResponse(std::shared_ptr<Message> msg, const Response response)
{
    nlohmann::json j = response;
    std::string json = j.dump();

    std::cout << std::endl << json << std::endl;

    ProtocolWriter pw;
    uint8_t buf[HEADER_SIZE + json.length()];
    pw.wrap_response_header_buffer(buf, msg->header->serial_number, json);
    Write(msg->fd, buf, HEADER_SIZE + json.length());
    return 0;
}