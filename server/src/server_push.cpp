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
    auto opt = g_uidClientMap.get(uid);
    if (!opt.has_value()) {
        LOG_ERROR("server_push error: uid={}\n", uid);
        return -1;
    }

    int cid = opt.value()->id;

    nlohmann::json j = *message;
    std::string json = j.dump();

    std::shared_ptr<std::string> data = std::make_shared<std::string>(HEADER_SIZE + json.length(), 0);
    ProtocolWriter pw;
    pw.wrap_push_header_buffer(data->data(), get_push_serial_number(), json);

    std::shared_ptr<EVMessage> msg = std::make_shared<EVMessage>(cid, EVMESSAGE_TYPE_SERVER_PUSH, data);
    EventHandler::getInstance().post(msg);
    return 0;
}

int writeResponse(std::shared_ptr<Message> msg, const Response response)
{
    nlohmann::json j = response;
    std::string json = j.dump();

    std::shared_ptr<std::string> data = std::make_shared<std::string>(HEADER_SIZE + json.length(), 0);
    ProtocolWriter pw;
    pw.wrap_response_header_buffer(data->data(), msg->header->serial_number, json);

    std::shared_ptr<EVMessage> evmsg = std::make_shared<EVMessage>(msg->cid, EVMESSAGE_TYPE_SERVER_PUSH, data);
    EventHandler::getInstance().post(evmsg);
    return 0;
}

void response_heartbeat(int cid) 
{
    std::string text = "pong";
    std::shared_ptr<std::string> data = std::make_shared<std::string>(HEADER_SIZE + text.length(), 0);
    ProtocolWriter pw;
    pw.wrap_heartbeat_header_buffer(data->data(), text);

    std::shared_ptr<EVMessage> evmsg = std::make_shared<EVMessage>(cid, EVMESSAGE_TYPE_SERVER_PUSH, data);
    EventHandler::getInstance().post(evmsg);
}