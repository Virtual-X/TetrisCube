#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "SolverT.h"

#include "Solution.h"

#include "CandidatesCalculator.h"

#include "UnsortedPermutator.h"

#include <list>
#include <functional>
#include <bitset>
#include <iostream>
#include <atomic>
#include <sstream>

#define BIT_GRID
#define USE_AVX

#include <immintrin.h>


class Solver {
private:
    std::shared_ptr<const Board> board;
    const int piecesCount;
    const Ints gridSize;

public:
    std::list<Solution> solutions;
    double progress;

private:
#ifdef BIT_GRID
    BitGrid grid;
#else
    Grid<> grid;
#endif
    UnsortedPermutator permutator;

    int actualPiece;

    std::vector<const FixedPiece*> situation;
    Ints position;

    std::shared_ptr<std::vector<std::vector<FixedPieces>>> candidatesPerPieceData;
    const std::vector<std::vector<FixedPieces>>& candidatesPerPiece;
    const std::vector<FixedPieces>* actualCandidates;

    enum {
        PiecesCount = 12,
        MaxRotations = 24,
        MaxCandidatesPerPos = 200,
    };

    Ints currentCandidatesIndex;
    Ints validCandidatesIndex;

    struct cd {
        int piece;
        int rotation;
    };

    cd validCandidates[PiecesCount][MaxCandidatesPerPos];
    int validCandidatesRotationCount[PiecesCount];

    int minPieceSize;

    std::shared_ptr<Stats> stats;

    struct CandidatesT {
        int candidatesOffsets[PositionsCount][PiecesCount + 1];
        CandidatesMask candidatesMask;
        std::vector<const FixedPiece*> candidatesPointers;

        CandidatesT(const std::vector<std::vector<FixedPieces>>& candidatesPerPiece) {
            for (int j = 0; j < PositionsCount; j++) {
                candidatesOffsets[j][0] = (int)candidatesMaskBuffer.size();
                for (int i = 0; i < PiecesCount; i++) {
                    const FixedPieces& candidates = candidatesPerPiece[j][i];
                    for (const auto& c : candidates) {
                        candidatesMaskBuffer.push_back(c.bitset);
                        candidatesPointers.push_back(&c);
                    }
                    candidatesOffsets[j][i + 1] = (int)candidatesMaskBuffer.size();
                }
            }

            candidatesMask = &candidatesMaskBuffer[0];
        }

    private:
        std::vector<uint64_t> candidatesMaskBuffer;
    };

    std::shared_ptr<const CandidatesT> candidatesT;

public:

    void ResetStats() {
        stats.reset(new Stats());
    }

    void MergeStats(const Solver& other) {
        stats->Merge(*other.stats);
    }

    void PrintStats() {
        stats->Print();
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
#ifdef BIT_GRID
          grid(Ints{prod(gridSize)}),
#else
          grid(gridSize),
#endif
          permutator(piecesCount),
          actualPiece(0),
          situation(piecesCount),
          candidatesPerPieceData(std::make_shared<std::vector<std::vector<FixedPieces>>>(0)),
          candidatesPerPiece(*candidatesPerPieceData),
          currentCandidatesIndex(piecesCount),
          validCandidatesIndex(piecesCount),
          minPieceSize(0),
          stats(new Stats())
    {
        Coords order;
        auto d = CandidatesCalculator(board).GetSnakeCandidates(order);
        //auto d = CandidatesCalculator(board).GetOptimalCandidates(order);
        auto c = CandidatesCalculator(board).Reorder(d, order);
//        auto c = CandidatesCalculator(board).GetCandidates();

        Coord pos(Ints(gridSize.size()));
        Enumerate(pos, gridSize, [&] {
            candidatesPerPieceData->push_back((*c)[pos]);
            return true;
        });

        candidatesT = std::make_shared<CandidatesT>(candidatesPerPiece);

        for (int i = 0; i < piecesCount; i++) {
            currentCandidatesIndex[i] = -1;
            validCandidatesIndex[i] = -1;
            position.push_back(0);
        }

        for (const auto& p : board->GetPieces()) {
            int n = p.GetOrientationCoords(0).size();
            if (n < minPieceSize)
                minPieceSize = n;
        }

        CalculateValidCandidates();
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
#ifdef MAX_SOLUTIONS_T
            if (solutions.size() >= MAX_SOLUTIONS_T)
                return -1;
#endif
                DecreaseActualPiece();
            }
        }

        return actualPiece;
    }

    void Solve() {
        CandidatesOffsets candidatesOffsets = candidatesT->candidatesOffsets;

        int permutationOrder[PiecesCount];
        for (int i = 0; i < PiecesCount; i++) {
            permutationOrder[i] = permutator[i];
        }

        SituationT currentSituation;
        for (int i = 0; i < PiecesCount; i++) {
            const int off = candidatesOffsets[this->position[i]][permutator[i]];
            currentSituation.pieces[i] = off + currentCandidatesIndex[i];
        }

        std::list<SituationT> solutions;
        int minPiece = actualPiece;
        uint64_t grid = this->grid.GetBits();
        int position = this->position[actualPiece];

        try {
            SolverT::Solve(candidatesOffsets, candidatesT->candidatesMask, permutationOrder,
                           currentSituation, solutions, minPiece, grid, position, *stats);
        }
        catch (int) {
        }

        for (const auto& s : solutions) {
            FixedPieces p;
            p.reserve(PiecesCount);
            for (int i = 0; i < PiecesCount; i++) {
                p.push_back(*candidatesT->candidatesPointers[s.pieces[i]]);
            }
            this->solutions.push_back(Solution(p));
        }
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

#ifdef OLD_ALL_AT_ONCE
    void CalculateValidCandidates() {}

    bool Next() {
        stats->Next();

        auto& index = currentCandidatesIndex[actualPiece];
        index++;

        const auto& candidatesPerPosition = GetCandidatesPerPosition(actualPiece);
        permutator.SwapBack(actualPiece);
        int candidate = permutator.GetIndex(actualPiece);
        for (; candidate < piecesCount; candidate++) {
            stats->PieceCandidate(actualPiece);
            const auto& candidates = candidatesPerPosition[permutator[candidate]];
            for (; index < (int)candidates.size(); index++) {
                stats->ConfigCandidate(actualPiece);
                if (grid.IsValid(candidates[index])) { // && grid.LeavesSpace(candidates[index], minPieceSize)) {
                    permutator.Swap(actualPiece, candidate);
                    return true;
                }
            }

            index = 0;
        }

        index = -1;
        permutator.Reset(actualPiece);
        return false;
    }
#else
    void CalculateValidCandidatesNaive() {
        const auto& candidatesPerPosition = GetCandidatesPerPosition(actualPiece);
        int n = 0;
        auto& vc = validCandidates[actualPiece];
        for (int candidate = actualPiece; candidate < PiecesCount; candidate++) {
            stats->PieceCandidate(actualPiece);
            const auto& candidates = candidatesPerPosition[permutator[candidate]];
            const int size = (int)candidates.size();
            for (int index = 0; index < size; index++) {
                stats->ConfigCandidate(actualPiece);
                if (grid.IsValid(candidates[index])) {
                    cd& cd = vc[n++];
                    cd.piece = candidate;
                    cd.rotation = index;
                }
            }
        }
        validCandidatesRotationCount[actualPiece] = n;
    }

    void CalculateValidCandidatesAvx() {
        uint64_t g = grid.GetBits();
        uint64_t b[] = { g, g, g, g };
        __m256i mm_grid = _mm256_load_si256((const __m256i*)b);
        uint64_t p[4];

        const auto& candidatesPerPosition = GetCandidatesPerPosition(actualPiece);
        int n = 0;
        auto& vc = validCandidates[actualPiece];
        for (int candidate = actualPiece; candidate < PiecesCount; candidate++) {
            stats->PieceCandidate(actualPiece);
            const auto& candidates = candidatesPerPosition[permutator[candidate]];
            const int size = (int)candidates.size();
            const int sizel = ((size + 3) / 4) * 4;
            for (int index = 0; index < sizel; index++) {
                stats->ConfigCandidate(actualPiece);
                p[index % 4] = index < size ? candidates[index].bitset : (uint64_t)-1;
                if (index % 4 == 3) {
                    __m256i mm_pieces = _mm256_load_si256((__m256i*)p);
                    __m256i mm_and = _mm256_and_si256(mm_pieces, mm_grid);
                    _mm256_store_si256((__m256i*)p, mm_and);
                    // _mm256_cmpgt_epi64
                    for (int j = 0; j < 4; j++) {
                        if (!p[j]) {
                            cd& cd = vc[n++];
                            cd.piece = candidate;
                            cd.rotation = index - 3 + j;
                        }
                    }
                }
            }
        }
        validCandidatesRotationCount[actualPiece] = n;
    }

    void CalculateValidCandidates() {
        CalculateValidCandidatesNaive();
//        CalculateValidCandidatesTemplate();
//        CalculateValidCandidatesAvx();
    }


    bool Next() {
        stats->Next();

        int index = ++validCandidatesIndex[actualPiece];

        if (index >= validCandidatesRotationCount[actualPiece]) {
            validCandidatesIndex[actualPiece] = -1;
            permutator.SwapBack(actualPiece);
            permutator.Reset(actualPiece);
            return false;
        }

        const cd& cd = validCandidates[actualPiece][index];
        permutator.Take(actualPiece, cd.piece);

        currentCandidatesIndex[actualPiece] = cd.rotation;
        return true;
    }
#endif

    bool _Next() {
        stats->Next();

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
        stats->Next();

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
        stats->Decrease();

        actualPiece--;

        if (actualPiece >= 0) {
            grid.SetPiece(GetActualPiece(), false);
        }
    }

    void IncreaseActualPiece() {
        stats->Increase();

        grid.SetPiece(GetActualPiece(), true);

        actualPiece++;

        if (actualPiece < piecesCount) {
            int pos = position[actualPiece - 1] + 1;
#ifndef OPT
            while (grid[pos]) {
                pos++;
                stats->PosPP();
            }
#else
            pos = grid.GetNextValid(pos);
#endif
            position[actualPiece] = pos;
            CalculateValidCandidates();
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
        return candidatesPerPiece[position[positionIndex]];
    }

    void AddSolution() {
        solutions.push_back(GetCurrentSituation());
    }
};



#endif // __SOLVER_H__
