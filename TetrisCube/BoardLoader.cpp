#include "BoardLoader.h"

#include "Geometry3D.h"

#include <fstream>
#include <sstream>
#include <functional>

std::shared_ptr<Board> BoardLoader::LoadFile(const std::string &filename)
{
    std::ifstream in(filename);
    return Load(in);
}

std::shared_ptr<Board> BoardLoader::LoadDefault()
{
    std::ostringstream out;
    GetDefault(out);
    std::istringstream in(out.str());
    return Load(in);
}

static PieceColor GetPieceColor(const std::string& pieceColor)
{
    return pieceColor == "b" ? PieceColor(0, 0, 255) :
           pieceColor == "r" ? PieceColor(255, 0, 0) :
                               PieceColor(255, 255, 0);
}

std::shared_ptr<Board> BoardLoader::Load(std::istream &in)
{
    if (!in)
        throw std::string("Could not read the file. Please check the file path!");

    std::function<int()> readDimSize = [&] {
        int i;
        in >> i;
        if (i <= 0)
            throw std::string("Invalid board dimension!");
        return i;
    };

    std::function<bool()> readX = [&] {
        char i;
        in >> i;
        return i == 'x';
    };

    std::function<std::string()> readName = [&] {
//        char i;
//        bool comment = false;
//        std::ostringstream name;
//        while (in >> i && i != '\n') {
//            if (i == '#')
//                comment = true;
//            if (!comment)
//                name << i;
//        }
        std::string name;
        std::getline(in, name);
        return name;
    };

    Ints boardSize;
    do {
        boardSize.push_back(readDimSize());
    } while (readX());

    auto x = readName();

    Pieces boardPieces;
    Coords coords;
    Ints coord(3);

    char c;
    while (in >> c) {
        if (c == '0') {
            coord[0]++;
        }
        else if (c == '1') {
            coords.push_back(Coord(coord));
            coord[0]++;
        }
        else if (c == ',') {
            coord[0] = 0;
            coord[1]++;
        }
        else if (c == ';') {
            coord[0] = 0;
            coord[1] = 0;
            coord[2]++;
        }
        else if (c == '.') {
            coord[0] = 0;
            coord[1] = 0;
            coord[2] = 0;
            std::string name = readName();
            boardPieces.push_back(
                        Piece(Geometry3D::GetUniqueRotations(coords),
                              GetPieceColor(name)));
            coords.clear();
        }
    }

    return std::make_shared<Board>(boardSize, boardPieces);
}

void BoardLoader::GetDefault(std::ostream& out)
{
    out << "4x4x4.Tetris4\n";
    out << "100,111,010;100,000,000.r\n";
    out << "010,111,000;000,010,010.y\n";
    out << "111,100,100;000,000,100.b\n";
    out << "111,100,100;000,100,000.b\n";
    out << "111,010;000,010.t\n";
    out << "110,011;100,000.b\n";
    out << "110,010;000,011.y\n";
    out << "111,100;000,100.y\n";
    out << "111;100;100.r\n";
    out << "1111,0100.r\n";
    out << "1110,0011.b\n";
    out << "11,11;10,00.r\n";
}
