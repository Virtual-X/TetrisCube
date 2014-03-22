#ifndef SCENE_H
#define SCENE_H


#include "TetrisCube/Solution.h"

class Scene
{
public:
    Scene();

    void SetSolution(const Solution& solution);

    void Resize(int width, int height);
    void Update(float secondsElapsed, int key);
    void Render();
};

#endif // SCENE_H
