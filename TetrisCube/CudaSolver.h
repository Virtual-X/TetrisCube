#ifndef CUDASOLVER_H
#define CUDASOLVER_H

#include "Solution.h"
#include "CandidatesCalculator.h"
#include "ParallelSolver.h"

#define CUDA_LIB
#ifdef CUDA_LIB
#include "../../../cuda-workspace/cuda_tetriscube/src/cuda_tc.h"
#else
#include "cuda_cpu.h"
#endif

#include <iostream>

template<typename T>
static T* raw(std::vector<T>& vector) {
    return &vector[0];
}

class CudaSolver {
public:
    std::list<Solution> solutions;

    CudaSolver(const std::shared_ptr<const Board>& board)
        : gridSize(board->GetSize())
    {
        auto n = board->GetPieces().size();
        if (n != piecesCount) {
            throw 0;
        }

        auto candidatesPerPieceData = CandidatesCalculator(board).GetCandidates();
        const Grid<std::vector<FixedPieces>>& candidatesPerPiece = *candidatesPerPieceData;

        candidateOffsets.push_back(0);
        Coord pos(Ints(gridSize.size()));
        Enumerate(pos, gridSize, [&] {
            for (const auto& cs : candidatesPerPiece[pos]) {
                for (const auto& c : cs)
                    candidatesData.push_back(c);
                candidateOffsets.push_back(candidatesData.size());
            }
            return true;
        });

        for (const auto& c : candidatesData) {
            //candidates.push_back(c.bitset.to_ullong());
            candidates.push_back(c.bitset);
        }
    }

    void Solve_CPU(int splitLevel, int solutionsPerSolver, int nthreads) {
        ParallelSolver<SingleSolver> ps(PrepareSingleSolver(splitLevel, solutionsPerSolver), splitLevel);
        solutions = ps.Solve(Solve, nthreads);

        std::cout << std::endl << "CUDA CPU finished. Solutions: " << solutions.size() << std::endl;
    }

    void Solve_GPU(int splitLevel, int solutionsPerSolver) {
        SingleSolver s = PrepareSingleSolver(splitLevel, solutionsPerSolver);

        auto ss = s.Split(splitLevel); // modify solversStatus

        int n = SolveGPU(raw(candidates), raw(candidateOffsets), raw(solversStatus), 0, splitLevel, solutionsPerSolver, ss.size());
        std::vector<solution> sols(n);
        SolveGPU(raw(candidates), raw(candidateOffsets), raw(solversStatus), raw(sols), splitLevel, solutionsPerSolver, ss.size());

        s.ExtractSolutions(sols, solutions);

        std::cout << std::endl << "CUDA GPU finished. Solutions: " << solutions.size() << std::endl;
    }

private:
    class SingleSolver {
    public:
        std::list<Solution> solutions;

        SingleSolver(
                const FixedPieces& candidatesData,
                const uint64_t* candidates,
                const int* candidateOffsets,
                std::vector<solverStatus>& solversStatus,
                int minPiece,
                int maxSolutions)
            : candidatesData(candidatesData),
              candidates(candidates),
              candidateOffsets(candidateOffsets),
              solversStatus(solversStatus),
              minPiece(minPiece),
              maxSolutions(maxSolutions),
              solverIndex(0) {
        }

        std::vector<std::shared_ptr<SingleSolver> > Split(int splitLevel = 1) const {
            int n = SplitCPU(splitLevel, candidates, candidateOffsets, 0);
            solversStatus.resize(n);
            SplitCPU(splitLevel, candidates, candidateOffsets, raw(solversStatus));

            std::cout << "CUDA splits: " << n << std::endl;

            std::vector<std::shared_ptr<SingleSolver> > result;
            for (int i = 0; i < n; i++) {
                auto c = std::make_shared<SingleSolver>(*this);
                c->solverIndex = i;
                result.push_back(c);
            }
            return result;
        }

        void Solve() {
            int n = SolveCPU(candidates, candidateOffsets, raw(solversStatus) + solverIndex, 0, minPiece, maxSolutions);
            std::vector<solution> sols(n);
            SolveCPU(candidates, candidateOffsets, raw(solversStatus) + solverIndex, raw(sols), minPiece, maxSolutions);

            ExtractSolutions(sols, solutions);
        }

        void ExtractSolutions(const std::vector<solution>& sols, std::list<Solution>& solutions) const {
            for (const auto& s : sols) {
                FixedPieces fp;
                for (int j = 0; j < piecesCount; j++) {
                    fp.push_back(candidatesData[s.candidateIndex[j]]);
                }
                solutions.push_back(Solution(fp));
            }
        }

    private:
        const FixedPieces& candidatesData;
        const uint64_t* candidates;
        const int* candidateOffsets;
        std::vector<solverStatus>& solversStatus;
        int minPiece;
        int maxSolutions;
        int solverIndex;
    };

    SingleSolver PrepareSingleSolver(int splitLevel, int solutionsPerSolver) {
        solversStatus.resize(1);
        Init(solversStatus[0]);

        auto cs = raw(candidates);
        auto cos = raw(candidateOffsets);

        std::cout << std::endl << std::endl;

        return SingleSolver(candidatesData, cs, cos, solversStatus, splitLevel, solutionsPerSolver);
    }

    static void Solve(SingleSolver& solver, int threadId, int solverId) {
        (void)threadId;
        (void)solverId;

        solver.Solve();
//        std::cout << "\rEXHAUSTED! [cuda solver " << solverId << ", thread " <<
//                     threadId << ", solutions " << solver.solutions.size() << "]";
//        std::cout << "." << std::flush;
    }

    static void Init(solverStatus& status) {
        status.actualPiece = 0;
        status.grid = 0;
        status.position[0] = 0;
        for (int i = 0; i < piecesCount; i++) {
            status.currentCandidatesIndex[i] = -1;
            status.permutatorIndices[i] = 0;
            status.permutatorObjects[i] = i;
            status.position[i] = 0;
        }
    }

    const Ints gridSize;

    FixedPieces candidatesData;

    std::vector<uint64_t> candidates;
    std::vector<int> candidateOffsets;
    std::vector<solverStatus> solversStatus;
};

#endif // CUDASOLVER_H
