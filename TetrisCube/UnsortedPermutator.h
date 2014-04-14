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
            Reset(i);
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

    void Take(int position, int other) {
        const int i = indices[position];
        const int u = objects[i];
        objects[i] = objects[position];
        objects[position] = objects[other];
        objects[other] = u;
        indices[position] = other;
    }

    void Swap(int position, int other) {
        indices[position] = other;
        Swap(position);
    }

    void SwapBack(int position) {
        Swap(position);
    }

    void Reset(int position) {
        indices[position] = position;
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
        if (candidateNumber >= (int)indices.size()) {
            indices[position] = position;
            return false;
        }

        indices[position] = candidateNumber;
        Swap(position);
        return true;
    }

    void Swap(int position) {
        SwapObjects(position, indices[position]);
    }

    void SwapObjects(int i, int j) {
//        if (i == j)
//            return;
        int t = objects[i];
        objects[i] = objects[j];
        objects[j] = t;
    }

    Ints indices;
    Ints objects;
};

#endif // __UNSORTEDPERMUTATOR_H__
