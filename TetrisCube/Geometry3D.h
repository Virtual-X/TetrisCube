#ifndef __GEOMETRY3D_H__
#define __GEOMETRY3D_H__

#include "Coord.h"

#include <algorithm>

class Geometry3D
{
public:
    static std::vector<Coords> GetUniqueRotations(const Coords& coords) {
        std::vector<Coords> r{coords};
        SubtractOriginAndSort(r.back());

        for (int i = 0; i < MaxOrientationsCount; i++) {
            auto c = RotateCoords(coords, i);
            SubtractOriginAndSort(c);
            if (std::find(r.begin(), r.end(), c) == r.end())
                r.push_back(c);
        }

        return r;
    }

private:

    // 6 [faces for main axis] x 4 [faces second axis] = 24
    static const int MaxOrientationsCount = 24;

    static Coords RotateCoords(const Coords& coords, int orientation) {
        Coords c;
        c.reserve(coords.size());
        int k = orientation;
        int sx = 1 - (k % 2) * 2;
        k /= 2;
        int sy = 1 - (k % 2) * 2;
        k /= 2;
        int ox = k % 3;
        k /= 3;
        int oy = (ox + k + 1) % 3;
        int oz = 3 - ox - oy;
        int sz = sx * sy * (oy == (ox + 1) % 3 ? 1 : -1);
        for (size_t i = 0; i < coords.size(); i++) {
            const Coord& p = coords[i];
            c.push_back(Coord(sx * p[ox], sy * p[oy], sz * p[oz]));
        }
        return c;
    }

    static void SubtractOriginAndSort(Coords& coords) {
        auto min = Coord::LowerLimit(coords);
        Coord::Subtract(coords, min);
        std::sort(coords.begin(), coords.end());
    }
};

#endif // __GEOMETRY3D_H__
