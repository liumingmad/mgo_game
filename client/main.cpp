#include <iostream>
#include "client.h"

int main(int argc, char const* argv[])
{
    MgoClient client;
    client.socket_init("127.0.0.1", 8010);
    client.socket_connect();

    char message[512] = { 0 };
    char buf[512] = { 0 };
    while (std::cin.getline(message, 512, '\n')) {
        client.socket_write(message);
        client.socket_read(buf);
        std::cout << buf << std::endl;
    }
    client.socket_close();
	return 0;
}
