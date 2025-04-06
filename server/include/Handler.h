#ifndef HANDLE_H
#define HANDLE_H


#include <string>

class Message {
public:
    int fd;
    ProtocolHeader* header;
    std::string text; 
};

void handle_message(Message* msg);

#endif // HANDLE_H