#ifndef __PIECE_H__
#define __PIECE_H__

#include "Coord.h"
#include "PieceColor.h"

class Piece;
typedef std::vector<Piece> Pieces;

class Piece
{
public:
    Piece(const std::vector<Coords>& orientationCoords, const PieceColor& color)
        : orientationCoords(orientationCoords),
          color(color) {
    }

    int GetOrientationsCount() const {
        return orientationCoords.size();
    }

    const Coords& GetOrientationCoords(int orientation) const {
        return orientationCoords[orientation];
    }

    const PieceColor& GetColor() const {
        return color;
    }

private:
    std::vector<Coords> orientationCoords;
    PieceColor color;
};


#endif // __PIECE_H__
