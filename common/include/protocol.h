#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <cstring>
#include "protocol.h"
#include "ring_buffer.h"

/*
自定义网络协议的协议头设计需要兼顾高效性、可扩展性和安全性，通常包含以下核心字段：

一、基础标识字段
​魔数（Magic Number）​
通常为4字节固定值，用于快速识别协议有效性，防止非协议数据干扰（如网页7示例中魔数用于端口校验）。
​版本号（Version）​
1字节标识协议版本，支持向后兼容和升级迭代（如网页7通过版本号区分新旧协议格式）。
二、数据解析控制字段
​序列化类型（Serialization Method）​
1字节指定数据编码方式（如JSON、Protobuf、Hessian），确保接收方能正确解析数据体（参考网页2的设计）。
​指令/命令类型（Command）​
1字节定义操作类型（如登录、心跳、业务操作），用于路由请求到对应处理逻辑（网页7通过指令字段实现业务分发）。
三、传输控制字段
​流水号/序列号（Serial Number）​
2字节唯一标识请求，用于实现请求响应匹配和异步通信追踪（如网页7通过流水号追踪任务状态）。
​数据长度（Data Length）​
4字节明确数据体大小，解决TCP粘包/拆包问题（网页2和网页7均采用此字段界定数据边界）。

* 协议头:魔数(4B) + 版本(1B) + 序列化方式(1B) + 指令(1B) + 流水号(2B) + 数据长度(4B)
* 数据:数据

示例：
MOGO + 1 + JSON + MOVE/HEART + 10 + 129
{
    "x": 1,
    "y": 2
}
*/

#define HEADER_SIZE 13

#define MAGIC_NUMBER "MOGO"

class ProtocolHeader
{
public:
    // MOGO
    char magic_number[4];

    // 1
    int version;

    // JSON: 0
    char serialization_method;

    // HEART:   0
    // AUTH:    1
    // MOVE:    2
    // QUERY:   3
    char command;

    // no
    u_int16_t serial_number;

    // 129
    int data_length;
};

class Protocol
{
public:
    ProtocolHeader header;
    char *data;
};

class ProtocolParser
{
public:
    ProtocolParser() {}
    ~ProtocolParser() {}

    // 检查并删除ringbuffer中无效的data
    void clear_invalid_data(RingBuffer *buf)
    {
        size_t size = buf->get_size();
        char tmp[size];
        buf->peek(tmp, size);
        std::string str(tmp, size);
        size_t index = str.find_first_of(MAGIC_NUMBER);
        buf->pop(nullptr, index);
    }

    // 检测ringbuffer里是否包含一个完整的协议, 如果存在，返回长度，不存在返回0
    int exist_one_protocol(RingBuffer *rb)
    {
        if (rb->get_size() < HEADER_SIZE)
        {
            return 0;
        }

        char tmp[HEADER_SIZE];
        rb->peek(tmp, HEADER_SIZE);

        // magic_number
        if (memcmp(tmp, MAGIC_NUMBER, strlen(MAGIC_NUMBER)) != 0)
        {
            return false;
        }

        // data_length
        size_t data_len = (tmp[9] << 24) | (tmp[10] << 16) | (tmp[11] << 8) | tmp[12];
        int proc_len = HEADER_SIZE + data_len;
        if (proc_len > rb->get_size())
        {
            return false;
        }

        return proc_len;
    }

    ProtocolHeader *parse_header(RingBuffer *rb, int len)
    {
        if (len < HEADER_SIZE)
        {
            return nullptr;
        }

        char buf[HEADER_SIZE];
        rb->peek(buf, HEADER_SIZE);

        // magic_number
        if (memcmp(buf, MAGIC_NUMBER, 4) != 0)
        {
            return nullptr;
        }

        ProtocolHeader *header = new ProtocolHeader();

        // version
        header->version = buf[4];

        // serialization_method
        header->serialization_method = buf[5];

        // command
        header->command = buf[6];

        // serial_number
        header->serial_number = (buf[7] << 8) | buf[8];

        // data_length
        header->data_length = (buf[9] << 24) | (buf[10] << 16) | (buf[11] << 8) | buf[12];

        return header;
    }
};

class ProtocolWriter
{
public:
    uint8_t* wrap_push_header_buffer(u_int16_t serial_number, std::string data)
    {
        uint8_t* buf = new uint8_t[HEADER_SIZE + data.length()]();
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
        // COMMAND_BUSSNESS:   4
        // COMMAND_SERVER_PUSH 5
        buf[6] = 0x05;

        // Serial Number
        buf[7] = static_cast<uint8_t>((serial_number >> 8) & 0xFF);
        buf[8] = static_cast<uint8_t>(serial_number & 0xFF);

        // data length
        uint32_t number = data.length();
        intToBytes(number, &buf[9]);

        memcpy(buf+HEADER_SIZE, data.c_str(), data.length());

        return buf;
    }

    uint8_t* wrap_response_header_buffer(u_int16_t serial_number, std::string data)
    {
        uint8_t* buf = new uint8_t[HEADER_SIZE + data.length()]();
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
        // COMMAND_BUSSNESS:   4
        // COMMAND_SERVER_PUSH 5
        buf[6] = 0x04;

        // Serial Number
        buf[7] = static_cast<uint8_t>((serial_number >> 8) & 0xFF);
        buf[8] = static_cast<uint8_t>(serial_number & 0xFF);

        // data length
        uint32_t number = data.length();
        intToBytes(number, &buf[9]);

        memcpy(buf+HEADER_SIZE, data.c_str(), data.length());

        return buf;
    }

    void intToBytes(uint32_t value, uint8_t bytes[4])
    {
        bytes[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
        bytes[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        bytes[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        bytes[3] = static_cast<uint8_t>(value & 0xFF);
    }
};

#endif // PROTOCOL_H