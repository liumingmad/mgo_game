#ifndef BITARRAY2D_H
#define BITARRAY2D_H

class BitArray2D
{
private:
    char* data;
    int rows;
    int cols;

    void copy(const BitArray2D& other);

public:
    BitArray2D(int rows, int cols);
    ~BitArray2D();
    BitArray2D(const BitArray2D& other);
    BitArray2D& operator=(const BitArray2D& other);
    bool operator==(const BitArray2D &other) const;
    bool operator!=(const BitArray2D &other) const;
    char* operator[](int row) const;
    int getRows() const { return this->rows; }
    int getCols() const { return this->cols; }
    char* getData() const { return this->data; }
};

#endif // BITARRAY2D_H