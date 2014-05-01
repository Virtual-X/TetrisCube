#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

enum {
    PositionsCount = 64,
    PiecesCount = 12,
    MaxRotations = 24,
    MaxCandidatesPerPosition = PiecesCount * MaxRotations,
    PiecesTimesPosition = PiecesCount * PositionsCount,
    CodesCount = 256,
	MaxSolutions = 10000
};

struct SituationT {
    int pieces[PiecesCount];
};

typedef const int (&CandidatesOffsets)[PositionsCount][CodesCount][PiecesCount + 1];
//typedef const int (&CandidatesOffsets)[PositionsCount][PiecesCount + 1];
typedef const uint64_t* CandidatesMask;

#ifdef FORCE_INLINE
#define _force_inline __attribute__((always_inline))
#else
#define _force_inline
#endif

#endif // CONSTANTS_H
