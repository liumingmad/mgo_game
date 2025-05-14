#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <iostream>
#include <cstring>
#include "Log.h"

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

    ~RingBuffer() {
        delete[] buffer;
    }

    size_t push(char *data, int len)
    {
        if (isFull()) {
            LOG_ERROR("RingBuffer push is full\n");
            return 0;
        } 

        if (len > freespace()) {
            LOG_ERROR("RingBuffer push len>freespace\n");
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
            return 0;
        }

        if (len > size) {
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
            return 0;
        }

        if (isEmpty()) {
            return 0;
        }

        if (len > size) {
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
                LOG_DEBUG("H");
            }
            if (i == this->tail)
            {
                LOG_DEBUG("T");
            }
            if (head > tail) {
                if (i >= tail || i < head) {
                    LOG_DEBUG("{} ", buffer[i]);
                } else {
                    LOG_DEBUG("x ");
                }
            } else {
                if (i >= tail || i < head) {
                    LOG_DEBUG("{} ", buffer[i]);
                } else {
                    LOG_DEBUG("x ");
                }
            }
        }
        LOG_DEBUG("\n");
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