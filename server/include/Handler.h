#ifndef HANDLE_H
#define HANDLE_H


#include <vector>
#include <string>
#include <memory>
#include "protocol.h"

class Message {
public:
    int fd;
    ProtocolHeader* header;
    std::string text; 
};

class Client;
void handle_message(std::shared_ptr<Client> client);

#endif // HANDLE_H