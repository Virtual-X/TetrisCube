#ifndef SCENE_H
#define SCENE_H


class Scene
{
public:
    virtual ~Scene() {}

    virtual void Resize(int width, int height) = 0;
    virtual void Update(float secondsElapsed) = 0;
    virtual void Render() = 0;
};


#endif // SCENE_H
