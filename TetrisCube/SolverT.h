#ifndef SOLVERT_H
#define SOLVERT_H

#include "Stats.h"
#include "Coder.h"

#include <list>

enum {
    PositionsCount = 64,
    PiecesCount = 12,
    MaxRotations = 24,
    MaxCandidatesPerPosition = PiecesCount * MaxRotations,
    PiecesTimesPosition = PiecesCount * PositionsCount
};

struct SituationT {
    int pieces[PiecesCount];
};

struct MaskBit {

    static int GetNextValid(uint64_t grid, int position) {
//        while (grid & (1LL << position))
//            position++;
        static MaskBit x;
        return x.GetNextValid_(grid, position);
    }

private:

    int GetNextValid_(uint64_t grid, int position) const {
        int pos = nextValid[(grid >> position) & 0xff];
        if (pos >= 0)
            return position + pos;
        int byte = position / 8;
        for (int i = byte + 1;; i++) {
            pos = nextValid[(grid >> (i * 8)) & 0xff];
            if (pos >= 0)
                return (i * 8) + pos;
        }
        return -1;
    }

    MaskBit() {
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

    int nextValid[256];
};

typedef const int (&CandidatesOffsets)[PositionsCount][CodesCount][PiecesCount + 1];
//typedef const int (&CandidatesOffsets)[PositionsCount][PiecesCount + 1];
typedef const uint64_t* CandidatesMask;

typedef void (*AddValidCandidatesFunc)(uint64_t , int, int, int, int*, int*, int&, CandidatesMask);
typedef AddValidCandidatesFunc (&AddValidCandidates)[PiecesTimesPosition];

template<int ActualPiece>
class SolverT_T {

    CandidatesOffsets candidatesOffsets;
    CandidatesMask candidatesMask;
    AddValidCandidates addValidCandidates;

    int (&permutationOrder)[PiecesCount];
    int& currentSituation;

    Stats& stats;

    SolverT_T<ActualPiece + 1> nextSolver;

    enum {
        PiecesLeft = PiecesCount - ActualPiece
    };

public:

    static void SolveX(CandidatesOffsets candidatesOffsets,
                       CandidatesMask candidatesMask,
                       AddValidCandidates addValidCandidates,
                       int (&permutationOrder)[PiecesCount],
                       SituationT& currentSituation,
                       std::list<SituationT>& solutions,
                       int minPiece,
                       uint64_t grid,
                       int position,
                       Stats& stats) {
        if (ActualPiece == minPiece) {
            SolverT_T<ActualPiece> solver(candidatesOffsets, candidatesMask, addValidCandidates,
                                          permutationOrder, currentSituation, solutions, stats);
            solver.Solve(grid, position);
        }
        else {
            SolverT_T<ActualPiece + 1>::SolveX(candidatesOffsets, candidatesMask, addValidCandidates, permutationOrder,
                                               currentSituation, solutions, minPiece, grid, position, stats);
        }
    }

    SolverT_T(CandidatesOffsets candidatesOffsets,
              CandidatesMask candidatesMask,
              AddValidCandidates addValidCandidates,
              int (&permutationOrder)[PiecesCount],
              SituationT& currentSituation,
              std::list<SituationT>& solutions,
              Stats& stats)
        : candidatesOffsets(candidatesOffsets),
          candidatesMask(candidatesMask),
          addValidCandidates(addValidCandidates),
          permutationOrder(permutationOrder),
          currentSituation(currentSituation.pieces[ActualPiece]),
          stats(stats),
          nextSolver(candidatesOffsets, candidatesMask, addValidCandidates,
                     permutationOrder, currentSituation, solutions, stats) {
    }

    static void AddNoCandies(uint64_t , int, int, int, int*, int*, int&, CandidatesMask) {
    }

    template<int Offset>
    void GetBeginEnd(int* begins, int* ends, const int (&candidatesOffsets_)[PiecesCount + 1]) const {
        stats.PieceCandidate(Offset);
        const int piece = permutationOrder[ActualPiece];
        begins[ActualPiece - Offset] = candidatesOffsets_[piece];
        ends[ActualPiece - Offset] = candidatesOffsets_[piece + 1];
        nextSolver.template GetBeginEnd<Offset>(begins, ends, candidatesOffsets_);
    }

    template<int Offset>
    void AddValidCands(uint64_t grid, const int* begins, const int* ends, int* validCandidates, int* validIndices, int& validsCount) const {
        const int begin = begins[ActualPiece - Offset];
        const int end = ends[ActualPiece - Offset];
        for (int index = begin; index < end; index++) {
            stats.ConfigCandidate(Offset);
            if (!(candidatesMask[index] & grid)) {
                validCandidates[validsCount] = ActualPiece;
                validIndices[validsCount] = index;
                ++validsCount;
            }
        }

        nextSolver.template AddValidCands<Offset>(grid, begins, ends, validCandidates, validIndices, validsCount);
    }

    void Solve(const uint64_t grid, const int position) {
        stats.Next();

        const int current = permutationOrder[ActualPiece];
        const int pos = MaskBit::GetNextValid(grid, position);
        const int code = Coder::GetCode(grid, pos);

        int begins[PiecesLeft];
        int ends[PiecesLeft];

        const int (&candidatesOffsets_)[PiecesCount + 1] = candidatesOffsets[pos][code];

        GetBeginEnd<ActualPiece>(begins, ends, candidatesOffsets_);

        int validsCount = 0;
        int validCandidates[MaxCandidatesPerPosition];
        int validIndices[MaxCandidatesPerPosition];

        AddValidCands<ActualPiece>(grid, begins, ends, validCandidates, validIndices, validsCount);

        if (!validsCount)
            return;

        int other = ActualPiece;
        for (int index = 0; index < validsCount; index++) {
            const int c = validCandidates[index];
            const int i = validIndices[index];
            currentSituation = i;
            Take(c, other, current);
            nextSolver.Solve(candidatesMask[i] | grid, pos + 1);
            other = c;
        }
        SwapBack(other, current);
    }

private:

    void Take(const int index, const int other, const int current) {
        permutationOrder[other] = permutationOrder[ActualPiece];
        permutationOrder[ActualPiece] = permutationOrder[index];
        permutationOrder[index] = current;
    }

    void Swap(const int index, const int current) {
        permutationOrder[ActualPiece] = permutationOrder[index];
        permutationOrder[index] = current;
    }

    void SwapBack(const int index, const int current) {
        permutationOrder[index] = permutationOrder[ActualPiece];
        permutationOrder[ActualPiece] = current;
    }
};

template<>
class SolverT_T<PiecesCount> {

    SituationT& currentSituation;
    std::list<SituationT>& solutions;

public:

    static void SolveX(CandidatesOffsets candidatesOffsets,
                       CandidatesMask candidatesMask,
                       AddValidCandidates addValidCandidates,
                       int (&permutationOrder)[PiecesCount],
                       SituationT& currentSituation,
                       std::list<SituationT>& solutions,
                       int minPiece,
                       uint64_t grid,
                       int position,
                       Stats& stats) {
        (void)candidatesOffsets;
        (void)candidatesMask;
        (void)addValidCandidates;
        (void)permutationOrder;
        (void)currentSituation;
        (void)solutions;
        (void)minPiece;
        (void)grid;
        (void)position;
        (void)stats;
    }

    SolverT_T(CandidatesOffsets candidatesOffsets,
              CandidatesMask candidatesMask,
              AddValidCandidates addValidCandidates,
              int (&permutationOrder)[PiecesCount],
              SituationT& currentSituation,
              std::list<SituationT>& solutions,
              Stats& stats)
        : currentSituation(currentSituation),
          solutions(solutions) {
        (void)candidatesOffsets;
        (void)candidatesMask;
        (void)addValidCandidates;
        (void)permutationOrder;
        (void)stats;
    }

    template<int Offset>
    void GetBeginEnd(int*, int*, const int*) const {
    }

    template<int Offset>
    void AddValids(uint64_t, const int*, int*, int*, int&) const {
    }

    template<int Offset>
    void AddValidCands(uint64_t, const int*, const int*, int*, int*, int&) const {
    }

    void Solve(uint64_t grid, int position) {
        (void)grid;
        (void)position;
        solutions.push_back(currentSituation);
#ifdef MAX_SOLUTIONS_T
        if (solutions.size() >= MAX_SOLUTIONS_T)
            throw 0;
#endif
    }

private:
};

class SolverT {
public:
    static void Solve(CandidatesOffsets candidatesOffsets,
                      CandidatesMask candidatesMask,
                      int (&permutationOrder)[PiecesCount],
                      SituationT& currentSituation,
                      std::list<SituationT>& solutions,
                      int minPiece,
                      uint64_t grid,
                      int position,
                      Stats& stats)
    {
        AddValidCandidatesFunc addValidCandidates[PiecesTimesPosition];
//        for (int i = 0; i < PiecesTimesPosition; i++) {
//            const int pos = i / PiecesCount;
//            const int piece = i % PiecesCount;
//            addValidCandidates[i] = candidatesOffsets[pos][piece] < candidatesOffsets[pos][piece + 1] ?
//                        &SolverT_T<0>::AddValidCandies : &SolverT_T<0>::AddNoCandies;
//        };
        SolverT_T<0>::SolveX(candidatesOffsets, candidatesMask, addValidCandidates, permutationOrder,
                             currentSituation, solutions, minPiece, grid, position, stats);
    }

private:
};

#endif // SOLVERT_H
