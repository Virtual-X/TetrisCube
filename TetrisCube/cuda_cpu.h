#ifndef CUDA_CPU_H
#define CUDA_CPU_H

#define API
#define __solver_access__

enum {
    piecesCount = 12,
    gridSize = 64,
    solutionBufferSize = 5
};

struct solverStatus {
    uint64_t grid;
    int actualPiece;
    int position[piecesCount];
    int currentCandidatesIndex[piecesCount];
    int permutatorIndices[piecesCount];
    int permutatorObjects[piecesCount];
};

struct solution {
    int candidateIndex[piecesCount];
    //solution* next;
};

struct solutionBuffer {
    int solutionsCount;
    solution solutions[solutionBufferSize];
};

#define __host_solver__

#ifdef __host_solver__
void API SolveCPU(
        const uint64_t* candidates,
        const int* candidateOffsets,
        solverStatus* solversStatus,
        solutionBuffer* solutions,
        int minPiece,
        int maxSolutions,
        int solverIndex);
#endif

void API SolveGPU(
        const uint64_t* candidates,
        const int* candidateOffsets,
        solverStatus* solversStatus,
        solutionBuffer* solutions,
        int minPiece,
        int maxSolutions,
        int solversCount);

int API SplitCPU(
        int splitLevel,
        const uint64_t* candidates,
        const int* candidateOffsets,
        solverStatus* status);

class solver
{
public:
    __solver_access__
    solver(
            const uint64_t* candidates,
            const int* candidateOffsets,
            solverStatus* status,
            solutionBuffer* solutions)
: candidates(candidates),
  candidateOffsets(candidateOffsets),
  status(status),
  solutions(solutions) {
}

    __solver_access__
    void DoStep();

#ifdef __host_solver__
    void Split(int level,
            const uint64_t* candidates,
            const int* candidateOffsets,
            solverStatus* status,
            int& n) const {
        solverStatus tempStatus = *this->status;
        solver temp(candidates, candidateOffsets, &tempStatus, 0);
        while (temp.Next()) {
            solverStatus ss = tempStatus;
            solver s(candidates, candidateOffsets, &ss, 0);
            s.IncreaseActualPiece();
            if (level <= 1) {
                if (status)
                    status[n] = ss;
                n++;
            }
            else {
                s.Split(level - 1, candidates, candidateOffsets, status, n);
            }
        }
    }
#endif

private:

    __solver_access__
    static bool IsValid(uint64_t candidate, uint64_t grid);

    __solver_access__
    static void Swap(int* objects, int i, int j);

    __solver_access__
    static bool TakeCandidate(int* permutatorIndices, int* permutatorObjects, int position, int candidateNumber);

    __solver_access__
    static bool TakeNextCandidate(int* permutatorIndices, int* permutatorObjects, int actualPiece);

    __solver_access__
    bool Next();

    __solver_access__
    void DecreaseActualPiece();

    __solver_access__
    void IncreaseActualPiece();

    __solver_access__
    void AddSolution();

    const uint64_t* candidates;
    const int* candidateOffsets;
    solverStatus* status;
    solutionBuffer* solutions;
};

__solver_access__
bool solver::IsValid(uint64_t candidate, uint64_t grid)
{
    return (candidate & grid) == 0;
}

__solver_access__
void solver::Swap(int* objects, int i, int j)
{
    int t = objects[i];
    objects[i] = objects[j];
    objects[j] = t;
}

__solver_access__
bool solver::TakeCandidate(int* permutatorIndices, int* permutatorObjects, int position, int candidateNumber)
{
    Swap(permutatorObjects, position, position + permutatorIndices[position]);
    if (candidateNumber >= piecesCount - position) {
        permutatorIndices[position] = 0;
        return false;
    }

    permutatorIndices[position] = candidateNumber;
    Swap(permutatorObjects, position, position + candidateNumber);
    return true;
}

__solver_access__
bool solver::TakeNextCandidate(int* permutatorIndices, int* permutatorObjects, int actualPiece)
{
    return TakeCandidate(permutatorIndices, permutatorObjects, actualPiece, permutatorIndices[actualPiece] + 1);
}

__solver_access__
bool solver::Next()
{
    int actualPiece = status->actualPiece;
    int candidatesOffset = status->position[actualPiece] * piecesCount;

    do {
        int& index = status->currentCandidatesIndex[actualPiece];
        index++;
        int candidatesIndex = candidatesOffset + status->permutatorObjects[actualPiece];
        int min = candidateOffsets[candidatesIndex];
        int max = candidateOffsets[candidatesIndex + 1];
        if (index < min)
            index = min;
        while (index < max) {
            if (IsValid(candidates[index], status->grid))
                return true;
            index++;
        }

        index = -1;
    } while (TakeNextCandidate(status->permutatorIndices, status->permutatorObjects, actualPiece));
    return false;
}

__solver_access__
void solver::DecreaseActualPiece() {
    int& actualPiece = status->actualPiece;

    actualPiece--;

    if (actualPiece >= 0) {
        int pieceIndex = status->currentCandidatesIndex[actualPiece];
        status->grid &= ~candidates[pieceIndex];
    }
}

__solver_access__
void solver::IncreaseActualPiece() {
    int& actualPiece = status->actualPiece;

    int pieceIndex = status->currentCandidatesIndex[actualPiece];
    status->grid |= candidates[pieceIndex];

    actualPiece++;

    if (actualPiece < piecesCount) {
        int pos = status->position[actualPiece - 1] + 1;
        while ((status->grid & (1ULL << pos)) > 0)
            pos++;
        status->position[actualPiece] = pos;
#ifndef __CUDA_ARCH__
//        static int x = 0;
//        if (x++ < 15)
//        	std::cout << "pos " << pos << std::endl;
#endif
    }
}

__solver_access__
void solver::AddSolution() {
    int* current = status->currentCandidatesIndex;
    int* solution = solutions->solutions[solutions->solutionsCount].candidateIndex;
    for (int i = 0; i < piecesCount; i++) {
        solution[i] = current[i];
    }
    solutions->solutionsCount++;
}

__solver_access__
void solver::DoStep() {
    if (!Next()) {
        DecreaseActualPiece();
    } else {
        IncreaseActualPiece();
        if (status->actualPiece == piecesCount) {
            AddSolution();
            DecreaseActualPiece();
        }
    }
}

// todo: try kernel steps
// todo: try local data instead of shared (copy all)

__solver_access__
void SolveSingle(
        const uint64_t* candidates,
        const int* candidateOffsets,
        solverStatus* solversStatus,
        solutionBuffer* solutions,
        int minPiece,
        int maxSolutions,
        int solverIndex)
{
    solverStatus* status = solversStatus + solverIndex;
    solutionBuffer* solution = solutions + solverIndex;
    solver solver(candidates, candidateOffsets, status, solution);

    while (status->actualPiece >= minPiece && solution->solutionsCount < maxSolutions)
        solver.DoStep();
}

#ifdef __host_solver__
void SolveCPU(
        const uint64_t* candidates,
        const int* candidateOffsets,
        solverStatus* solversStatus,
        solutionBuffer* solutions,
        int minPiece,
        int maxSolutions,
        int solverIndex)
{
    SolveSingle(candidates, candidateOffsets, solversStatus, solutions, minPiece, maxSolutions, solverIndex);
}

#endif

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

int SplitCPU(int splitLevel,
             const uint64_t* candidates,
             const int* candidateOffsets,
             solverStatus* status) {
    if (splitLevel < 1) {
        if (status)
            Init(*status);
        return 1;
    }

    int n = 0;
    solverStatus ss;
    Init(ss);
    solver s(candidates, candidateOffsets, &ss, 0);
    s.Split(splitLevel, candidates, candidateOffsets, status, n);
    return n;
}

#endif // CUDA_CPU_H
