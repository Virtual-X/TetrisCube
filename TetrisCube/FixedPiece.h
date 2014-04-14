#ifndef FIXEDPIECE_H
#define FIXEDPIECE_H

#include "Coord.h"
#include "PieceColor.h"

#include <bitset>
//typedef std::bitset<64> bitset64;
typedef uint64_t bitset64;

class FixedPiece;
typedef std::vector<FixedPiece> FixedPieces;

class FixedPiece {
public:
    FixedPiece(const Coords& coords, const PieceColor& color, const Ints& gridSize)
        : coords(coords),
          color(color),
          bitset(GetBitSet(coords, gridSize)) {
    }

    FixedPiece(const Coords& coords, const PieceColor& color, const Ints& gridSize, const Ints& maskMap)
        : coords(coords),
          color(color),
          bitset(GetBitSet(coords, gridSize, maskMap)) {
    }

    const Coords coords;
    const PieceColor color;

    const bitset64 bitset;
private:

    static bitset64 GetBitSet(const Coords& coords, const Ints& gridSize) {
        Ints maskMap(64);
        for (size_t i = 0; i < maskMap.size(); i++) {
            maskMap[i] = i;
        }
        return GetBitSet(coords, gridSize, maskMap);
    }

    static bitset64 GetBitSet(const Coords& coords, const Ints& gridSize, const Ints& maskMap) {
        bitset64 mask = 0;
        for (size_t i = 0; i < coords.size(); i++) {
            const Coord& c = coords[i];
            int bit = c.GetIndex1D(gridSize);
            int maskBit = maskMap[bit];
            mask |= (1LL << maskBit);
            //mask[maskBit] = 1;
        }
        return mask;
    }

};

#endif // FIXEDPIECE_H
