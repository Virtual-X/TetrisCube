#ifndef BOARDLOADER_H
#define BOARDLOADER_H

#include <memory>
#include <string>
#include <vector>

struct Coord {
    int x, y, z;
};

struct Piece {
    std::string Color;
    std::vector<Coord> Coords;
};

struct Board {
    std::vector<int> Size;

    std::vector<Piece> Pieces;
};

class BoardLoader
{
public:
    BoardLoader();

    std::shared_ptr<Board> Load(const char* filename);

private:

    std::shared_ptr<Board> OnError(const std::string& errorTag);
};

#endif // BOARDLOADER_H
