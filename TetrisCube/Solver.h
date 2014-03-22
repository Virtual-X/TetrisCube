#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "Solution.h"
#include "CandidatesCalculator.h"

#include "UnsortedPermutator.h"

#include <list>
#include <functional>
#include <bitset>
#include <iostream>

class Solver {
private:
    std::shared_ptr<const Board> board;
    const int piecesCount;
    const Ints gridSize;

public:
    std::list<Solution> solutions;
    double progress;

private:
    BitGrid grid;
    //Grid<> grid;
    UnsortedPermutator permutator;

    int actualPiece;

    std::vector<const FixedPiece*> situation;
    Ints position;

    std::shared_ptr<std::vector<std::vector<FixedPieces>>> candidatesPerPieceData;
    const std::vector<std::vector<FixedPieces>>& candidatesPerPiece;
    const std::vector<FixedPieces>* actualCandidates;
    std::shared_ptr<Ints> positionsOrderData;

    struct Dummy {
        inline int operator[] (int index) const {
            return index;
        }
    } positionsOrder;

//    const Ints& positionsOrder;

    Ints currentCandidatesIndex;

    int tries = 0;
    double avg = 0;
    int pass = 0;
    int failBorder = 0;
    int failContent = 0;
    int failHole = 0;
    int failSemiHole = 0;

public:

    std::string GetText(double elapsedSeconds) {
        std::string s = actualPiece < 0 && elapsedSeconds > 0 ? "Finished - " : "";
        return s;
        /*
        for (int i = 0; i < piecesCount; i++) {
            s += String.Format("{0,2}.{1:D2} ", permutator.GetIndex(i), currentCandidatesIndex[i] + 1);
        }
        return s + "\n[" + (int)(tries / 1e6) + " mio tries, " + elapsedSeconds.ToString("0.0") + " sec, " +
                (int)(tries / 1e3 / elapsedSeconds) + " k tries/sec]" +
                "\n[avg map: " + avg.ToString("0.00") +
                " | pass/fail: " + ((double)pass / (failBorder + failContent + failHole + failSemiHole)).ToString("0.00%") +
                " | fail border/content: " + ((double)failBorder / failContent).ToString("0.00") +
                "\n| fail hole/content: " + ((double)(failHole + failSemiHole) / failContent).ToString("0.00") +
                "\n| best/first occupied surroundings: " + ((double)(bestOccupiedSum) / firstOccupiedSum).ToString("0.00") + "]";
                */
    }

    Solver(const std::shared_ptr<const Board>& board)
        : piecesCount((int)board->GetPieces().size()),
          gridSize(board->GetSize()),
          solutions(),
          progress(0),
          grid(Ints{prod(gridSize)}),
          permutator(piecesCount),
          actualPiece(0),
          situation(piecesCount),
          candidatesPerPieceData(std::make_shared<std::vector<std::vector<FixedPieces>>>(0)),
          candidatesPerPiece(*candidatesPerPieceData),
          positionsOrderData(std::make_shared<Ints>(0)),
          //positionsOrder(*positionsOrderData),
          currentCandidatesIndex(piecesCount)
    {
        auto c = CandidatesCalculator(board).GetCandidates();
        for (int i = 0; i < prod(gridSize); i++)
            positionsOrderData->push_back(i);

//        Coords order;
//        auto c = CandidatesCalculator(board).GetSnakeCandidates(order);
//        for (const auto& o : order)
//            positionsOrderData->push_back(o.GetIndex1D(gridSize));

        Coord pos(Ints(gridSize.size()));
        Enumerate(pos, gridSize, [&] {
            candidatesPerPieceData->push_back((*c)[pos]);
            return true;
        });

        for (int i = 0; i < piecesCount; i++) {
            currentCandidatesIndex[i] = -1;
            position.push_back(0);
        }
    }

public:
    std::vector<std::shared_ptr<Solver> > Split(int level = 1) const {
        if (level < 1) {
            return std::vector<std::shared_ptr<Solver>>{std::make_shared<Solver>(*this)};
        }

        Solver temp(*this);
        std::vector<std::shared_ptr<Solver>> solvers;
        while (temp.Next()) {
            auto s = std::make_shared<Solver>(temp);
            s->IncreaseActualPiece();
            if (level <= 1) {
                solvers.push_back(s);
            }
            else {
                auto split = s->Split(level - 1);
                solvers.insert(solvers.end(), split.begin(), split.end());
            }
        }
        return solvers;
    }
    
    int DoStep() {
        if (actualPiece < 0)
            return -1;

        if (!Next()) {
            DecreaseActualPiece();
        } else {
            IncreaseActualPiece();
            if (actualPiece == piecesCount) {
                AddSolution();
                DecreaseActualPiece();
            }
        }

        return actualPiece;
    }

    Solution GetCurrentSituation() const {
        FixedPieces p;
        p.reserve(actualPiece);
        for (int i = 0; i < actualPiece; i++) {
            p.push_back(GetCurrentPiece(i));
        }
        return Solution(p);
    }

private:

    int prod(const Ints& ints) {
        int n = 1;
        for (int i : ints)
            n *= i;
        return n;
    }

    bool Next() {
        auto& index = currentCandidatesIndex[actualPiece];
        index++;

        const auto& candidatesPerPosition = GetCandidatesPerPosition(actualPiece);
        permutator.SwapBack(actualPiece);
        int candidate = actualPiece + permutator.GetIndex(actualPiece);
        for (; candidate < piecesCount; candidate++) {
            const auto& candidates = candidatesPerPosition[permutator[candidate]];
            for (; index < (int)candidates.size(); index++) {
                if (grid.IsValid(candidates[index])) {
                    permutator.Swap(actualPiece, candidate - actualPiece);
                    return true;
                }
            }

            index = 0;
        }

        index = -1;
        permutator.Reset(actualPiece);
        return false;
    }

    bool _Next() {
        do {
            auto& index = currentCandidatesIndex[actualPiece];
            index++;
            const auto& candidates = GetActualCandidates();
            while (index < (int)candidates.size()) {
                if (grid.IsValid(candidates[index]))
                    return true;
                index++;
            }

            index = -1;
        } while (permutator.TakeNextCandidate(actualPiece, 0));
        return false;
    }

    bool __Next() {
        auto& index = currentCandidatesIndex[actualPiece];
        index++;
        const auto& candidates = GetActualCandidates();
        if (index < (int)candidates.size()) {
            return grid.IsValid(candidates[index])
                    || Next();
        }

        index = -1;
        return permutator.TakeNextCandidate(actualPiece, 0)
                && Next();
    }

    void DecreaseActualPiece() {
        actualPiece--;

        if (actualPiece >= 0) {
            grid.SetPiece(GetActualPiece(), false);
        }
    }

    void IncreaseActualPiece() {
        grid.SetPiece(GetActualPiece(), true);

        actualPiece++;

        if (actualPiece < piecesCount) {
            int pos = position[actualPiece - 1] + 1;
            while (grid[positionsOrder[pos]])
                pos++;
            position[actualPiece] = pos;
        }
    }

    const FixedPiece& GetActualPiece() const {
        return GetCurrentPiece(actualPiece);
    }

    const FixedPiece& GetCurrentPiece(int piece) const {
        return GetCandidates(piece, piece)[currentCandidatesIndex[piece]];
    }

    const FixedPieces& GetActualCandidates() const {
        return GetCandidates(actualPiece, actualPiece);
    }

    const FixedPieces& GetCandidates(int positionIndex, int piece) const {
        return GetCandidatesPerPosition(positionIndex)[permutator[piece]];
    }

    const std::vector<FixedPieces>& GetCandidatesPerPosition(int positionIndex) const {
        return candidatesPerPiece[positionsOrder[position[positionIndex]]];
    }

    void AddSolution() {
        solutions.push_back(GetCurrentSituation());
    }
};



#endif // __SOLVER_H__
