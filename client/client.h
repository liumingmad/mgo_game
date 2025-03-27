#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <string>

class MgoClient {
private:
    int client_fd;
    struct sockaddr_in serv_addr;

public:
    MgoClient();
    ~MgoClient();
    int socket_init(const char* host, int port);
    int socket_connect();
    int socket_write(const char* message);
    int socket_read(char* buf);
    int socket_close();
};