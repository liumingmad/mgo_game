#include <iostream>
#include "client.h"
#include "protocol.h"


int32_t bytesToInt(const uint8_t bytes[4]) {
    return (static_cast<int32_t>(bytes[0]) << 24) |
           (static_cast<int32_t>(bytes[1]) << 16) |
           (static_cast<int32_t>(bytes[2]) << 8) |
           static_cast<int32_t>(bytes[3]);
}

void intToBytes(int32_t value, uint8_t bytes[4]) {
    bytes[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
    bytes[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    bytes[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    bytes[3] = static_cast<uint8_t>(value & 0xFF);
}

void write_buffer(uint8_t* buf, int data_len)
{
    // MOGO
    buf[0] = 'M';
    buf[1] = 'O';
    buf[2] = 'G';
    buf[3] = 'O';

    // Version
    buf[4] = 0x01;

    // Serialization Method
    buf[5] = 0x01;

    // Command
    // HEART:   0
    // AUTH:    1
    // MOVE:    2
    // QUERY:   3
    buf[6] = 0x01;

    // Serial Number
    buf[7] = 0xcd;
    buf[8] = 0xab;

    // data length
    int32_t number = data_len; 
    intToBytes(number, &buf[9]);
    
}

void read_procotol(MgoClient client, RingBuffer* rb) {
    char buf[4*1024] = { 0 };
    int n = client.socket_read(buf);
    if (n == 0) {
        return;
    }

    rb->push(buf, n);

    ProtocolParser parser;
    while (!rb->isEmpty()) {
        // 1.在buffer中，检查是否存在一条完整的协议, 返回协议的全长
        size_t len = parser.exist_one_protocol(rb);
        if (len == 0) {
            break;
        }

        ProtocolHeader* header = parser.parse_header(rb, HEADER_SIZE);
        char tmp[len];
        rb->peek(tmp, len);
        std::string str = std::string(tmp+HEADER_SIZE, header->data_length);
        std::cout << str << std::endl;

        // 4. 删除处理过的数据
        rb->pop(nullptr, len);
    }
}

void run_client()
{
    MgoClient client;
    client.socket_init("127.0.0.1", 9001);
    client.socket_connect();
    RingBuffer* rb = new RingBuffer(128*1024);

    char message[512] = { 0 };
    
    while (true) {
        memset(message, 0, 512);

        std::cin.getline(message+13, 512, '\n');
        int data_len = strlen(message+13);
        write_buffer(reinterpret_cast<uint8_t*>(message), data_len);
        client.socket_write(message, data_len+13);

        // read
        read_procotol(client, rb);

    }
    client.socket_close();
}


int test_intToBytes() {
    int32_t number = 123456789;
    uint8_t bytes[4];
    intToBytes(number, bytes);
    std::cout << "Byte array: ";
    for (int i = 0; i < 4; ++i) {
        std::cout << static_cast<int>(bytes[i]) << " ";
    }
    std::cout << std::endl;
    return 0;
}


int test_bytesToInt() {
    uint8_t bytes[4] = {7, 91, 205, 21}; // 对应于 123456789
    int32_t number = bytesToInt(bytes);
    std::cout << "Converted number: " << number << std::endl;
    return 0;
}

int main(int argc, char const* argv[])
{
    run_client();
    // test_intToBytes();
    // test_bytesToInt();
	return 0;
}
