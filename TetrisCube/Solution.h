#ifndef __SOLUTION_H__
#define __SOLUTION_H__

#include "Configuration.h"
#include "Board.h"
#include "FixedPiece.h"

#include <memory>

//typedef Configurations Solution;

class Solution
{
public:
    Solution(const Board& board,
             const Configurations& configurations)
        : pieces(GetFixedPieces(board, configurations)) {
    }

    explicit Solution(const FixedPieces& pieces)
        : pieces(pieces) {
    }

    const FixedPieces& GetPieces() const {
        return pieces;
    }

private:

    // could implement lazy to give work to drawing thread, but copy configs is not much cheaper than this
    static std::vector<FixedPiece> GetFixedPieces(
                const Board& board,
                const Configurations& configurations) {
        std::vector<FixedPiece> f;
        f.reserve(configurations.size());
        for (const auto& c : configurations) {
            const Piece& p = board.GetPieces()[c.PieceIndex];
            auto coords = p.GetOrientationCoords(c.Orientation);
            Coord::Add(coords, c.Offset);
            f.push_back(FixedPiece(coords, p.GetColor(), board.GetSize()));
        }
        return f;
    }

    FixedPieces pieces;
};


#endif // __SOLUTION_H__
