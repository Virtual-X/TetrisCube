#include "Main.h"

#include "Timer.h"
#include "BoardLoader.h"

#define STATS_
//#define MAX_SOLUTIONS_T 3
#define TEMPLATE_T

static const int numThreads = 16;

#ifdef MAX_SOLUTIONS_T
static const int splitLevel = 1;
static const int maxSolvers = 25;
static const size_t solverPerDisplay = 1;
#else
static const int splitLevel = 3;
static const int maxSolvers = 0;
static const size_t solverPerDisplay = splitLevel == 3 ? 500 : 5;
#endif

static const std::string boardName = "Tetris4.txt";


#include "Solver.h"
#include "ParallelSolver.h"

static Solver* mainSolver = 0;

#ifdef STATS
#include <mutex>
static std::mutex mutex;
void MergeStats(const Solver& solver) {
    std::lock_guard<std::mutex> lock(mutex);
    (void)lock;
    mainSolver->MergeStats(solver);
}
#else
void MergeStats(const Solver& solver) {
    (void)solver;
}
#endif

int numSolvers = 1;
static void Solve(Solver& solver, int threadId, int solverId) {
    if (maxSolvers && solverId % (numSolvers < maxSolvers ? 1 : (numSolvers + maxSolvers - 1) / maxSolvers))
        return;

    solver.ResetStats();

#ifndef TEMPLATE_T
    while (solver.DoStep() >= splitLevel);
#else
    solver.Solve();
#endif

    if (solverId % solverPerDisplay == 0)
    std::cout << "FINISHED! [solver " << solverId << ", thread " << threadId <<
                 ", solutions " << solver.solutions.size() << "]" << std::endl;

    MergeStats(solver);
}

void SolveTetrisCube() {
    std::shared_ptr<Board> board;
    try {
        board = BoardLoader::LoadFile(boardName);
    }
    catch (std::string& error) {
        std::cout << "Error loading Tetris4.txt: " << error;
        std::cout << "\nLoading default problem set..." << std::endl;
        board = BoardLoader::LoadDefault();
    }

    Timer t;
    t.Start();
    Solver solver(board);
    mainSolver = &solver;
    t.Lap("init");

    ParallelSolver<Solver> ps(solver, splitLevel);
    numSolvers = (int)ps.GetSplitCount();
    std::cout << "Split count: " << numSolvers << std::endl;
    t.Lap("split");

    solver.solutions = ps.Solve(Solve, numThreads);
    std::cout << std::endl << "Total solutions found: " << (int)solver.solutions.size() << std::endl;
    t.Lap("completion");

    solver.PrintStats();
}

int main() {
    SolveTetrisCube();
}
