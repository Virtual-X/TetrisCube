#ifndef __UNSORTEDPERMUTATOR_H__
#define __UNSORTEDPERMUTATOR_H__

#include "Vector.h"

class UnsortedPermutator
{
public:
    UnsortedPermutator(int count)
        : indices(count),
          objects(count) {
        for (int i = 0; i < count; i++) {
            objects[i] = i;
        }
    }

    int operator [] (int position) const {
        return objects[position];
    }

    int GetIndex(int position) {
        return indices[position];
    }

    bool TakeNextCandidate(int position, int nextCandidateNumber) {
        return TakeCandidate(position, indices[position] + 1 + nextCandidateNumber);
    }

    void Swap(int position, int candidateNumber) {
        indices[position] = candidateNumber;
        Swap(position);
    }

    void SwapBack(int position) {
        Swap(position);
    }

    void Reset(int position) {
        indices[position] = 0;
    }

    //IEnumerable<int> GetNextCandidates(int position) {
    //  return objects.Skip(position + indices[position] + 1);
    //}

    //IEnumerable<int> GetOldCandidates(int position) {
    //  return objects.Skip(position).Take(indices[position] + 1);
    //}

    //IEnumerable<int> GetNonCandidates(int position) {
    //  return objects.Take(position);
    //}

private:
    bool TakeCandidate(int position, int candidateNumber) {
        Swap(position);
        if (candidateNumber >= GetMaxIndex(position)) {
            indices[position] = 0;
            return false;
        }

        indices[position] = candidateNumber;
        Swap(position);
        return true;
    }

    int GetMaxIndex(int position) {
        return indices.size() - position;
    }

    void Swap(int position) {
        if (indices[position] > 0)
            SwapObjects(position, position + indices[position]);
    }

    void SwapObjects(int i, int j) {
        int t = objects[i];
        objects[i] = objects[j];
        objects[j] = t;
    }

    Ints indices;
    Ints objects;
};

#endif // __UNSORTEDPERMUTATOR_H__
