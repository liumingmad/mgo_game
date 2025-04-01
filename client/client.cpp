#include <iostream>

#include "client.h"

MgoClient::MgoClient() {
}

MgoClient::~MgoClient() {
}

int MgoClient::socket_init(const char* host, int port) {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}
    return 0;
}

int MgoClient::socket_connect() {
    int status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (status < 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
    return status;
}

int MgoClient::socket_write(const char* message, size_t len) {
	return send(client_fd, message, len, 0);
}

int MgoClient::socket_read(char* buf) {
	int n = read(client_fd, buf, 512 - 1);
    return n;
}

int MgoClient::socket_close() {
    return close(client_fd);
}
