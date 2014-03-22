#include "boardloader.h"

#include "TetrisCube/Geometry3D.h"

#include <QFile>
#include <QXmlStreamReader>

#include <iostream>

BoardLoader::BoardLoader()
{
}

static std::vector<int> GetSize(const QStringRef& string)
{
    std::vector<int> size;
    for (int i = 0; i < string.length(); i++)
        size.push_back(QString(string.at(i)).toInt());
    return size;
}

static PieceColor GetPieceColor(const std::string& pieceColor)
{
    return pieceColor == "B" ? PieceColor(0, 0, 255) :
           pieceColor == "R" ? PieceColor(255, 0, 0) :
                               PieceColor(255, 255, 0);
}

std::shared_ptr<Board> BoardLoader::Load(const char* filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return OnError(filename);

    QXmlStreamReader reader;
    reader.setDevice(&file);

    reader.readNext();
    reader.readNext();

    if (reader.name() != "Board")
        return OnError("Board");
    Ints boardSize = GetSize(reader.attributes().value("Size"));

    reader.readNext();
    reader.readNext();
    if (reader.name() != "Pieces")
        return OnError("Pieces");

    Pieces boardPieces;

    reader.readNext();
    reader.readNext();
    while (reader.name() == "Piece") {
        auto attr = reader.attributes();
        auto pieceColor = attr.value("Color").toString().toStdString();
        auto size = GetSize(attr.value("Size"));
        auto data = attr.value("Data");
        size.resize(2, data.length());
        Coords coords;
        for (int i = 0; i < data.length(); i++)
        {
            if (data.at(i) == '1')
            {
                int x = i % size[0];
                int yz = i / size[0];
                int y = yz % size[1];
                int z = yz / size[1];
                coords.push_back(Coord(x, y, z));
            }
        }

        boardPieces.push_back(
                    Piece(Geometry3D::GetUniqueRotations(coords),
                          GetPieceColor(pieceColor)));

        reader.readNext();
        reader.readNext();
        reader.readNext();
    }

    if (reader.name() != "Pieces")
        return OnError("/Pieces");

    reader.readNext();
    reader.readNext();
    if (reader.name() != "Board")
        return OnError("/Board");

    return std::make_shared<Board>(boardSize, boardPieces);
}

std::shared_ptr<Board> BoardLoader::OnError(const std::string &errorTag)
{
    std::cout << "XML error: Expected " << errorTag << std::endl;
    return std::shared_ptr<Board>();
}
