#include <cstring>
#include "BitArray2D.h"

void BitArray2D::copy(const BitArray2D &other)
{
    this->rows = other.rows;
    this->cols = other.cols;
    this->data = new char[rows * cols + 1];
    for (int i = 0; i < rows * cols + 1; i++) {
        this->data[i] = other.data[i];
    }
}

BitArray2D::BitArray2D(int rows, int cols)
{
    this->rows = rows;
    this->cols = cols;
    this->data = new char[rows * cols + 1];
    memset(this->data, '0', rows * cols);
    this->data[rows * cols] = 0; 
}

BitArray2D::BitArray2D(const BitArray2D& other)
{
    copy(other);
}

BitArray2D::~BitArray2D()
{
    delete[] this->data;
}

BitArray2D& BitArray2D::operator=(const BitArray2D& other)
{
    copy(other);
    return *this;
}

bool BitArray2D::operator==(const BitArray2D &other) const 
{
    if (this->rows != other.rows || this->cols != other.cols) {
        return false;
    }
    for (int i = 0; i < rows * cols; i++) {
        if (this->data[i] != other.data[i]) {
            return false;
        }
    }
    return true;
}

bool BitArray2D::operator!=(const BitArray2D &other) const
{
    return !(*this == other);
}

char* BitArray2D::operator[](int row) const
{
    return this->data + row * this->cols;
}

