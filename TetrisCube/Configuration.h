#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "Coord.h"

class Configuration;
typedef std::vector<Configuration> Configurations;

class Configuration {
public:
    int PieceIndex;
    int Orientation;
    Coord Offset;
    bool IsTemporary;
};


#endif // __CONFIGURATION_H__
