#ifndef CANDIDATESCALCULATOR_H
#define CANDIDATESCALCULATOR_H

#include "Grid.h"
#include "Board.h"

#include <iostream>

class CandidatesCalculator {
public:

    typedef std::shared_ptr<Grid<std::vector<FixedPieces>>> Candidates;

    CandidatesCalculator(const std::shared_ptr<const Board>& board)
        : board(board),
          piecesCount((int)board->GetPieces().size()),
          gridSize(board->GetSize()) {
    }

    Candidates GetCandidates() const {
        auto candidatesPerPiece = std::make_shared<Grid<std::vector<FixedPieces>>>(gridSize);
        InitCandidates(*candidatesPerPiece);
        Evaluate(*candidatesPerPiece);
        return candidatesPerPiece;
    }

    Candidates GetSnakeCandidates(Coords& order) const {
        auto candidatesPerPiece = std::make_shared<Grid<std::vector<FixedPieces>>>(gridSize);
        InitSnakeCandidates(*candidatesPerPiece, order);
        Evaluate(*candidatesPerPiece);
        return candidatesPerPiece;
    }

    Candidates GetOptimalCandidates(Coords& order) const {
        auto candidatesPerPiece = std::make_shared<Grid<std::vector<FixedPieces>>>(gridSize);
        InitOptimalCandidates(*candidatesPerPiece, order);
        Evaluate(*candidatesPerPiece);
        return candidatesPerPiece;
    }

private:

    static int GetFixedPiece(const Pieces& p) {
        int max = -1;
        int maxOrientation = 0;
        for (int i = 0; i < (int)p.size(); i++) {
            if (p[i].GetOrientationsCount() > maxOrientation) {
                max = i;
                maxOrientation = p[i].GetOrientationsCount();
            }
        }

        return max;
    }

    static bool IsInside(const Coords& coords, const Ints& max) {
        for (size_t i = 0; i < coords.size(); i++) {
            const Coord& c = coords[i];
            for (size_t j = 0; j < c.GetSize(); j++) {
                if (c[j] < 0 || c[j] >= max[j])
                    return false;
            }
        }
        return true;
    }

    void TryAddCandidate(FixedPieces& candidates, const Grid<>& tempGrid, const Coord& pos, int pieceIndex, int orientation, int coordIndex) const {
        const Pieces& p = board->GetPieces();
        const Coords& coords = p[pieceIndex].GetOrientationCoords(orientation);
        Coords fixedCoords = coords;
        Coord::Add(fixedCoords, pos);
        Coord::Subtract(fixedCoords, coords[coordIndex]);
        if (IsInside(fixedCoords, gridSize)) {
            FixedPiece fixedPiece(fixedCoords, p[pieceIndex].GetColor(), gridSize);
            if (tempGrid.IsValid(fixedPiece)) {
                candidates.push_back(fixedPiece);
            }
        }
    }

    void AddCandidatesConstGrid(std::vector<FixedPieces>& candidatesPerPiece, const Grid<>& tempGrid, const Coord& pos, int fixedPiece) const {
        candidatesPerPiece.resize(piecesCount);
        const Pieces& p = board->GetPieces();
        for (int i = 0; i < piecesCount; i++) {
            int orientationsCount = fixedPiece == i ? 1 : p[i].GetOrientationsCount();
            for (int o = 0; o < orientationsCount; o++) {
                const Coords& coords = p[i].GetOrientationCoords(o);
                for (int j = 0; j < (int)coords.size(); j++) {
                    TryAddCandidate(candidatesPerPiece[i], tempGrid, pos, i, o, j);
                }
            }
        }
    }

    void AddCandidates(Grid<std::vector<FixedPieces>>& candidatesPerPiece, Grid<>& tempGrid, const Coord& pos, int fixedPiece) const {
        candidatesPerPiece[pos].resize(piecesCount);
        const Pieces& p = board->GetPieces();
        for (int i = 0; i < piecesCount; i++) {
            int orientationsCount = fixedPiece == i ? 1 : p[i].GetOrientationsCount();
            for (int o = 0; o < orientationsCount; o++) {
                const Coords& coords = p[i].GetOrientationCoords(o);
                for (int j = 0; j < (int)coords.size(); j++) {
                    TryAddCandidate(candidatesPerPiece[pos][i], tempGrid, pos, i, o, j);
                }
            }
        }

        tempGrid[pos] = true;
    }

    Coord GetOptimalCoord(const Grid<>& tempGrid, int fixedPiece) {
        Coord pos(Ints(gridSize.size()));
        Coord bestPos(pos);
        size_t bestCount = -1;
        Enumerate(pos, gridSize, [&] {
            if (tempGrid[pos])
                return true;
            std::vector<FixedPieces> candidatesPerPiece;
            AddCandidatesConstGrid(candidatesPerPiece, tempGrid, pos, fixedPiece);
            size_t n = 0;
            for (size_t i = 0; i < candidatesPerPiece.size(); i++)
                n += candidatesPerPiece[i].size();
            if (n < bestCount) {
                bestPos = pos;
                bestCount = n;
            }
            return true;
        });
        return bestPos;
    }

    void InitCandidates(Grid<std::vector<FixedPieces>>& candidatesPerPiece) const {
        const Pieces& p = board->GetPieces();
        int fixedPiece = GetFixedPiece(p);

        Grid<> tempGrid(gridSize);
        Coord pos(Ints(gridSize.size()));
        Enumerate(pos, gridSize, [&] {
            AddCandidates(candidatesPerPiece, tempGrid, pos, fixedPiece);
            return true;
        });
    }

    void InitSnakeCandidates(Grid<std::vector<FixedPieces>>& candidatesPerPiece, Coords& order) const {
        const Pieces& p = board->GetPieces();
        int fixedPiece = GetFixedPiece(p);

        int gridDim = (int)gridSize.size();
        int dim = gridDim - 1;
        Ints ints(gridDim);
        Coord px(ints);

        Grid<> tempGrid(gridSize);
        Coord pos(ints);
        //int counter = 0;
        Enumerate(pos, gridSize, [&] {
            //std::cout << counter++ << ": " << px[0] << " " << px[1] << " " << px[2] << std::endl;
            order.push_back(px);
            AddCandidates(candidatesPerPiece, tempGrid, px, fixedPiece);
            for (int i = 0; i < gridDim; i++) {
                if (i != dim) {
                    px[i]++;
                    if (px[i] < gridSize[i]) {
                        break;
                    }
                    else {
                        px[i] = i < dim ? px[dim] : px[dim] + 1;
                        if (i == gridDim - 1) {
                            px[dim]++;
                            dim = (dim + gridDim - 1) % gridDim;
                        } else if (i == gridDim - 2 && dim == gridDim - 1) {
                            px[dim]++;
                            dim = (dim + gridDim - 1) % gridDim;
                            break;
                        }
                    }
                }
            }
            return true;
        });
    }

    void InitOptimalCandidates(Grid<std::vector<FixedPieces>>& candidatesPerPiece, Coords& order) const {
        const Pieces& p = board->GetPieces();
        int fixedPiece = GetFixedPiece(p);

        Grid<> tempGrid(gridSize);
        Coord pos(Ints(gridSize.size()));
        int counter = 0;
        Enumerate(pos, gridSize, [&] {
            Coord px = GetOptimalCoord(tempGrid, fixedPiece);
            order.push_back(px);
            std::cout << counter++ << ": " << px[0] << " " << px[1] << " " << px[2] << std::endl;
            AddCandidates(candidatesPerPiece, tempGrid, px, fixedPiece);
            return true;
        });
    }

    void Evaluate(Grid<std::vector<FixedPieces>>& candidatesPerPiece) const {
        Coord pos = Coord(Ints(gridSize.size()));
        int max = 0;
        int count = 0;
        int sum = 0;
        double squareSum = 0;
        Enumerate(pos, gridSize, [&] {
            int n = 0;
            for (size_t i = 0; i < candidatesPerPiece[pos].size(); i++)
                n += candidatesPerPiece[pos][i].size();
            if (n > max)
                max = n;
            count++;
            sum += n;
            squareSum += n * n;

            std::cout << n << " ";
            if (count % 16 == 0)
                std::cout << std::endl;
            return true;
        });
        double mean = sum / (double)count;
        std::cout << "Candidates count " << sum << ", mean " << (int)mean
                  << ", squareSum " << squareSum << std::endl;
    }

    std::shared_ptr<const Board> board;
    const int piecesCount;
    const Ints gridSize;
};

#endif // CANDIDATESCALCULATOR_H
