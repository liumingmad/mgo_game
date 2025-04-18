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

#endif // HANDLE_H