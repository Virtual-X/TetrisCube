#ifndef SCENETDOGL_H
#define SCENETDOGL_H

#include "Scene.h"


class SceneTDOGL : public Scene
{
public:
    SceneTDOGL();

    virtual void Resize(int width, int height);
    virtual void Update(float secondsElapsed);
    virtual void Render();
};


#endif // SCENETDOGL_H
