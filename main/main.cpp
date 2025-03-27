#include <iostream>
#include "Server.h"
#include "wrap.h"

int main(int argc, char const *argv[])
{
    Server server;
    server.init();
    server.run(8010);
    return 0;
}

