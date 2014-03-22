#ifndef __BOARD_H__
#define __BOARD_H__

#include "Piece.h"

// todo: solver should use bitmask to test all coord at once!
//   requires one bitmask:
//     if (coder.GetCandidates(piece, position)[0].GetMask() & grid.GetMask() == 0)
//       piece++! candidates are configurations or fixed pieces

class Board
{
public:
    Board(const Ints& size, const Pieces& pieces)
        : size(size), pieces(pieces) {
    }

    const Ints& GetSize() const {
        return size;
    }

    const Pieces& GetPieces() const {
        return pieces;
    }

private:
    Ints size;
    Pieces pieces;
};

#endif // __BOARD_H__
