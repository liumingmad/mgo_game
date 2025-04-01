#include <iostream>
#include "../include/Server.h"

int main(int argc, char const *argv[])
{
    Server server;
    server.handle_request(0);

    return 0;
}
