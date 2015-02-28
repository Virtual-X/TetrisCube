#ifndef SCENERAYCAST_H
#define SCENERAYCAST_H

#include "Scene.h"
#include "Cube.h"
#include "Buffer.h"

#include "tdogl/Program.h"
#include "tdogl/Texture.h"
#include "tdogl/Camera.h"

#include <vector>
#include <memory>


class SceneRaycast : public Scene
{
public:
    SceneRaycast();

    virtual void Resize(int width, int height);
    virtual void Update(float secondsElapsed);
    virtual void Render();

private:
    void InitOpenGL();
    void InitScene();

    struct Light {
        glm::vec3 position;
        glm::vec3 intensities; //a.k.a. the color of the light
        float attenuation;
        float ambientCoefficient;
    };

    struct Model {
        std::shared_ptr<tdogl::Texture> texture;
        glm::vec3 vertices[4];
    };

    tdogl::Camera camera;
    glm::mat4 world;
    Light light;

    std::shared_ptr<Buffer> backBuffer;
    std::shared_ptr<tdogl::Program> backBufferProgram;
    std::shared_ptr<Cube> backCube;

    std::shared_ptr<tdogl::Texture> volume;
    std::shared_ptr<tdogl::Program> raycastProgram;
    std::shared_ptr<Cube> frontCube;
};


#endif // SCENERAYCAST_H
