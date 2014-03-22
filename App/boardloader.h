#ifndef BOARDLOADER_H
#define BOARDLOADER_H

#include <memory>
#include <string>

#include "TetrisCube/Board.h"

class BoardLoader
{
public:
    BoardLoader();

    std::shared_ptr<Board> Load(const char* filename);

private:

    std::shared_ptr<Board> OnError(const std::string& errorTag);
};

#endif // BOARDLOADER_H
