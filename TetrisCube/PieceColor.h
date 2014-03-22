#ifndef PIECECOLOR_H
#define PIECECOLOR_H

struct PieceColor {
    PieceColor(int red, int green, int blue)
        : red(red),
          green(green),
          blue(blue) {
    }

    int red;
    int green;
    int blue;
};

#endif // PIECECOLOR_H
