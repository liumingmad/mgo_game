#include "server_push.h"
#include <protocol.h>
#include <wrap.h>
#include "global.h"
#include "event_handler.h"

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

    std::shared_ptr<std::string> data = std::make_shared<std::string>(HEADER_SIZE + json.length(), 0);
    ProtocolWriter pw;
    pw.wrap_push_header_buffer(data->data(), get_push_serial_number(), json);

    std::shared_ptr<EVMessage> msg = std::make_shared<EVMessage>(fd, EVMESSAGE_TYPE_SERVER_PUSH, data);
    EventHandler::getInstance().post(msg);
}

int writeResponse(std::shared_ptr<Message> msg, const Response response)
{
    nlohmann::json j = response;
    std::string json = j.dump();

    std::cout << std::endl << json << std::endl;

    std::shared_ptr<std::string> data = std::make_shared<std::string>(HEADER_SIZE + json.length(), 0);
    ProtocolWriter pw;
    pw.wrap_response_header_buffer(data->data(), msg->header->serial_number, json);

    std::shared_ptr<EVMessage> evmsg = std::make_shared<EVMessage>(msg->fd, EVMESSAGE_TYPE_SERVER_PUSH, data);
    EventHandler::getInstance().post(evmsg);
    return 0;
}

void response_heartbeat(int fd) 
{
    std::string text = "pong";
    std::shared_ptr<std::string> data = std::make_shared<std::string>(HEADER_SIZE + text.length(), 0);
    ProtocolWriter pw;
    pw.wrap_heartbeat_header_buffer(data->data(), text);

    std::shared_ptr<EVMessage> evmsg = std::make_shared<EVMessage>(fd, EVMESSAGE_TYPE_SERVER_PUSH, data);
    EventHandler::getInstance().post(evmsg);
}