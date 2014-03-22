#ifndef __COORD_H__
#define __COORD_H__

#include "Vector.h"

class Coord;
typedef std::vector<Coord> Coords;

class Coord
{
public:
    Coord(int x, int y, int z)
        : values{x, y, z} {
    }

    explicit Coord(const Ints& values)
        : values(values) {
    }

//    Coord(std::initializer_list<int> coords)
//        : values(coords) {
//    }

    static Coord LowerLimit(const Coords& coords) {
        Coord min(coords[0]);
        for (size_t i = 1; i < coords.size(); i++) {
            const Coord& c = coords[i];
            for (size_t j = 0; j < c.GetSize(); j++) {
                if (c[j] < min[j])
                    min[j] = c[j];
            }
        }
        return min;
    }

    static Coord UpperLimit(const Coords& coords) {
        Coord max(coords[0]);
        for (size_t i = 1; i < coords.size(); i++) {
            const Coord& c = coords[i];
            for (size_t j = 0; j < c.GetSize(); j++) {
                if (c[j] > max[j])
                    max[j] = c[j];
            }
        }
        return max;
    }

    static bool AreEqual(const Coords& coords1, const Coords& coords2) {
        if (coords1.size() != coords2.size())
            return false;
        for (size_t i = 0; i < coords2.size(); i++) {
            if (coords1[i] != coords2[i])
                return false;
        }
        return true;
    }

    static bool Add(Coords& coords, const Coord& coord) {
        for (size_t i = 0; i < coords.size(); i++) {
            Coord& c = coords[i];
            for (size_t j = 0; j < c.GetSize(); j++) {
                c[j] += coord[j];
            }
        }
        return true;
    }

    static bool Subtract(Coords& coords, const Coord& coord) {
        for (size_t i = 0; i < coords.size(); i++) {
            Coord& c = coords[i];
            for (size_t j = 0; j < c.GetSize(); j++) {
                c[j] -= coord[j];
            }
        }
        return true;
    }

    size_t GetSize() const {
        return values.size();
    }

    const int& operator [] (int index) const {
        return values[index];
    }

    int& operator [] (int index) {
        return values[index];
    }

    bool operator == (const Coord& other) const {
        return values == other.values;
    }

    bool operator != (const Coord& other) const {
        return !(*this == other);
    }

    bool operator < (const Coord& other) const {
        for (size_t j = 0; j < values.size(); j++) {
            const size_t i = values.size() - 1 - j;
            if (values[i] < other[i])
                return true;
            if (values[i] > other[i])
                return false;
        }
        return false;
    }

    int GetIndex1D(const Ints& gridSize) const {
        int n = 1;
        int v = 0;
        for (size_t i = 0; i < values.size(); i++) {
            v += values[i] * n;
            n *= gridSize[i];
        }
        return v;
    }

private:
    Ints values;
};

#endif // __COORD_H__
