#ifndef __GRID_H__
#define __GRID_H__

#include "Vector.h"
#include "FixedPiece.h"

void Enumerate(Coord& indices, const Ints& max, std::function<bool()> callback) {
    size_t index = 0;
    if (!callback())
        return;
    while (index < max.size()) {
        indices[index]++;
        if (indices[index] == max[index]) {
            indices[index] = 0;
            index++;
        } else {
            if (!callback())
                return;
            index = 0;
        }
    }
}

template<typename T = char>
class Grid
{
public:
    Grid(const Ints& size) {
        size_t n = size[0];
        for (size_t i = 1; i < size.size(); i++) {
            offsets.push_back(n);
            n *= size[i];
        }
        values.resize(n);
    }

    static Grid Make3D(int x, int y, int z) {
        Ints c(3);
        c[0] = x;
        c[1] = y;
        c[2] = z;
        return Grid(c);
    }

    template<typename Indices> // Ints || Coord
    const T& operator [] (const Indices& index) const {
        return values[GetIndex1D(index)];
    }

    template<typename Indices>
    T& operator [] (const Indices& index) {
        return values[GetIndex1D(index)];
    }

    T& operator [] (const int& index) {
        return values[index];
    }

    void SetPiece(const FixedPiece& piece, bool value) {
        for (size_t j = 0; j < piece.coords.size(); j++) {
            (*this)[piece.coords[j]] = value;
        }
    }

    bool IsValid(const FixedPiece& piece) const {
        for (size_t j = 0; j < piece.coords.size(); j++) {
            if ((*this)[piece.coords[j]])
                return false;
        }
        return true;
    }

private:
    template<typename Indices>
    int GetIndex1D(const Indices& index) const {
        int n = index[0];
        for (size_t i = 0; i < offsets.size(); i++) {
            n += index[i + 1] * offsets[i];
        }
        if (n >= (int)values.size()) {
            throw 0;
        }
        return n;
    }

    std::vector<T> values;
    Ints offsets;
};

class BitGrid
{
public:
    BitGrid(const Ints& size)
        : bitset(0) {
        size_t n = size[0];
        for (size_t i = 1; i < size.size(); i++) {
            offsets.push_back(n);
            n *= size[i];
        }
        for (int i = 0; i < 256; i++) {
            nextValid[i] = -1;
            for (int k = 0; k < 8; k++) {
                if ((i & (1 << k)) == 0) {
                    nextValid[i] = k;
                    break;
                }
            }
        }
    }

    template<typename Indices>
    bool operator [] (const Indices& index) const {
        int bit = GetIndex1D(index);
        //return bitset[bit];
        return (bitset & (1LL << bit)) != 0;
    }

    bool operator [] (int bit) const {
        //return bitset[bit];
        return (bitset & (1LL << bit)) != 0;
    }

    void SetPiece(const FixedPiece& piece, bool value) {
        if (value) {
            Or(piece.bitset);
        } else {
            And(~piece.bitset);
        }
    }

    bool IsValid(const FixedPiece& piece) const {
        return (piece.bitset & bitset) == 0;
    }

    int GetNextValid(int index) const {
        int pos = nextValid[(bitset >> index) & 0xff];
        if (pos >= 0)
            return index + pos;
        int byte = index / 8;
        for (int i = byte + 1; i < 8; i++) {
            pos = nextValid[(bitset >> (i * 8)) & 0xff];
            if (pos >= 0)
                return (i * 8) + pos;
        }
        return -1;
    }

    uint64_t GetBits() const {
        return bitset;
    }

private:

    int nextValid[256];

    template<typename Indices>
    int GetIndex1D(const Indices& index) const {
        int n = index[0];
        for (size_t i = 0; i < offsets.size(); i++) {
            n += index[i + 1] * offsets[i];
        }
        return n;
    }

    void And(const bitset64& bitmask) {
        bitset &= bitmask;
    }

    void Or(const bitset64& bitmask) {
        bitset |= bitmask;
    }

    bitset64 bitset;
    Ints offsets;
};

#endif // __PIECE_H__
