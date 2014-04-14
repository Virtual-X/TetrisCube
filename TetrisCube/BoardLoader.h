#ifndef BOARDLOADER_H
#define BOARDLOADER_H

#include "Board.h"

#include <memory>

class BoardLoader
{
public:

    static std::shared_ptr<Board> LoadFile(const std::string& filename);
    static std::shared_ptr<Board> LoadDefault();

private:

    static std::shared_ptr<Board> Load(std::istream& in);
    static void GetDefault(std::ostream& out);

};

#endif // BOARDLOADER_H
