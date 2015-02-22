#include "boardloader.h"

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

std::shared_ptr<Board> BoardLoader::Load(const char* filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return OnError(filename);

    auto board = std::make_shared<Board>();

    QXmlStreamReader reader;
    reader.setDevice(&file);

    reader.readNext();
    reader.readNext();

    if (reader.name() != "Board")
        return OnError("Board");
    board->Size = GetSize(reader.attributes().value("Size"));

    reader.readNext();
    reader.readNext();
    if (reader.name() != "Pieces")
        return OnError("Pieces");

    reader.readNext();
    reader.readNext();
    while (reader.name() == "Piece") {
        board->Pieces.resize(board->Pieces.size() + 1);
        Piece& p = board->Pieces.back();
        auto attr = reader.attributes();
        p.Color = attr.value("Color").string()->toStdString();
        auto size = GetSize(attr.value("Size"));
        auto data = attr.value("Data");
        size.resize(2, data.length());
        for (int i = 0; i < data.length(); i++)
        {
            if (data.at(i) == '1')
            {
                Coord c;
                c.x = i % size[0];
                int yz = i / size[0];
                c.y = yz % size[1];
                c.z = yz / size[1];
                p.Coords.push_back(c);
            }
        }

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

    std::cout << board->Pieces.size() << board->Pieces.back().Coords.size() << std::endl;
    return board;
}

std::shared_ptr<Board> BoardLoader::OnError(const std::string &errorTag)
{
    std::cout << "XML error: Expected " << errorTag << std::endl;
    return std::shared_ptr<Board>();
}
