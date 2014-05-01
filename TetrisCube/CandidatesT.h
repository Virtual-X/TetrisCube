#ifndef CANDIDATEST_H
#define CANDIDATEST_H

#include "Coder.h"

#include <vector>
#include <list>

class CandidatesT {
public:
	typedef std::vector<std::vector<FixedPieces>> Type;

    int candidatesOffsets[PositionsCount][CodesCount][PiecesCount + 1];
    CandidatesMask candidatesMask;

    CandidatesT(const std::shared_ptr<Type>& candidatesPerPiece)
    : candidatesPerPiece(candidatesPerPiece) {
        for (int j = 0; j < PositionsCount; j++) {
            for (int k = 0; k < CodesCount; k++) {
                candidatesOffsets[j][k][0] = (int)candidatesMaskBuffer.size();
                for (int i = 0; i < PiecesCount; i++) {
                    const FixedPieces& candidates = (*candidatesPerPiece)[j][i];
                    for (const auto& c : candidates) {
                        if (Coder::IsCompatible(c.bitset, j, k)) {
                            candidatesMaskBuffer.push_back(c.bitset);
                            candidatesPointers.push_back(&c);
                        }
                    }
                    candidatesOffsets[j][k][i + 1] = (int)candidatesMaskBuffer.size();
                }
            }
        }

        candidatesMask = &candidatesMaskBuffer[0];
    }

    void Convert(const std::list<SituationT>& in, std::list<Solution>& out) const {
        for (const auto& s : in) {
            out.push_back(Convert(s));
        }
    }

    Solution Convert(const SituationT& s) const {
		FixedPieces p;
		p.reserve(PiecesCount);
		for (int i = 0; i < PiecesCount; i++) {
			if (s.pieces[i] >= 0 && s.pieces[i] < (int)candidatesPointers.size()) {
				p.push_back(*candidatesPointers[s.pieces[i]]);
			}
			else throw 0;
		}
		return Solution(p);
    }

private:
    std::vector<const FixedPiece*> candidatesPointers;
    std::vector<uint64_t> candidatesMaskBuffer;
    std::shared_ptr<Type> candidatesPerPiece;
};

#endif // CANDIDATEST_H
