#ifndef FIXEDPIECE_H
#define FIXEDPIECE_H

#include "Coord.h"
#include "PieceColor.h"

#include <bitset>
typedef std::bitset<64> bitset64;
//typedef uint64_t bitset64;

class FixedPiece;
typedef std::vector<FixedPiece> FixedPieces;

struct FixedPiece {
    FixedPiece(const Coords& coords, const PieceColor& color, const Ints& gridSize)
        : coords(coords),
          color(color),
          bitset(GetBitSet(coords, gridSize)) {
    }

    const Coords coords;
    const PieceColor color;

    const bitset64 bitset;
private:

    static bitset64 GetBitSet(const Coords& coords, const Ints& gridSize) {
        bitset64 mask = 0;
        for (size_t i = 0; i < coords.size(); i++) {
            const Coord& c = coords[i];
            int bit = c.GetIndex1D(gridSize);
            //mask |= (1LL << bit);
            mask[bit] = 1;
        }
        return mask;
    }


};

#endif // FIXEDPIECE_H
