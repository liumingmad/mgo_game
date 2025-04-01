#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <iostream>
#include <cstring>

class RingBuffer
{
private:
    char* buffer;
    int head;
    int tail;
    int size;
    int capacity;

public:
    RingBuffer(int capacity)
    {
        this->capacity = capacity;
        this->buffer = new char[capacity];
        this->head = 0;
        this->tail = 0;
        this->size = 0;
    }

    size_t push(char *data, int len)
    {
        if (isFull()) {
            std::cout << "RingBuffer push is full" << std::endl;
            return 0;
        } 

        if (len > freespace()) {
            std::cout << "RingBuffer push len>freespace" << std::endl;
            return 0;
        } 

        if (head + len < capacity) {
            memcpy(buffer+head, data, len);
        } else {
            // 分两段拷贝
            const size_t first_chunk = capacity - head;
            memcpy(buffer+head, data, first_chunk);
            memcpy(buffer, data+first_chunk, len-first_chunk);
        }
        head = (head + len) % capacity;
        size += len;
        return len;
    }

    size_t pop(char *data, int len)
    {
        if (isEmpty()) {
            std::cout << "RingBuffer pop is empty" << std::endl;
            return 0;
        }

        if (len > size) {
            std::cout << "RingBuffer pop len > size" << std::endl;
            return 0;
        }

        if (data == nullptr) {
            tail = (tail + len) % capacity;
            size -= len;
            return len;
        }

        if (tail + len < capacity) {
            memcpy(data, buffer+tail, len);
        } else {
            // 分两段拷贝
            const size_t first_chunk = capacity - tail;
            memcpy(data, buffer+tail, first_chunk);
            memcpy(data+first_chunk, buffer, len-first_chunk);
        }

        tail = (tail + len) % capacity;
        size -= len;
        return len;
    }

    size_t peek(char* data, size_t len) {
        if (len <= 0) {
            std::cout << "RingBuffer peek len <= 0" << std::endl;
            return 0;
        }

        if (isEmpty()) {
            std::cout << "RingBuffer peek is empty" << std::endl;
            return 0;
        }

        if (len > size) {
            std::cout << "RingBuffer peek len > size" << std::endl;
            return 0;
        }

        if (tail + len < capacity) {
            memcpy(data, buffer+tail, len);
        } else {
            // 分两段拷贝
            const size_t first_chunk = capacity - tail;
            memcpy(data, buffer+tail, first_chunk);
            memcpy(data+first_chunk, buffer, len-first_chunk);
        }
        return len;
    }

    bool isFull()
    {
        return this->size == this->capacity;
    }

    bool isEmpty()
    {
        return this->size == 0;
    }

    void print()
    {
        for (int i = 0; i < this->capacity; i++)
        {
            if (i == this->head)
            {
                std::cout << "H";
            }
            if (i == this->tail)
            {
                std::cout << "T";
            }
            if (head > tail) {
                if (i >= tail || i < head) {
                    std::cout << this->buffer[i] << " ";
                } else {
                    std::cout << "x" << " ";
                }
            } else {
                if (i >= tail || i < head) {
                    std::cout << this->buffer[i] << " ";
                } else {
                    std::cout << "x" << " ";
                }
            }
        }
        std::cout << std::endl;
    }

    void clear()
    {
        this->head = 0;
        this->tail = 0;
        this->size = 0;
    }

    size_t freespace()
    {
        return capacity - size;
    }

    size_t get_size() const {
        return this->size;
    }


};

#endif