#include "SceneRaycast.h"

#include "loadertdogl.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <sstream>


SceneRaycast::SceneRaycast()
{
    InitOpenGL();
    InitScene();
}


void SceneRaycast::Resize(int width, int height)
{
    backBuffer = std::make_shared<Buffer>(width, height);
    camera.setViewportAspectRatio(width / (float)height);
}


void SceneRaycast::Update(float secondsElapsed)
{
    world = glm::rotate(world, secondsElapsed * 10, glm::vec3(0.2, 0.5, 0.3));
}


void SceneRaycast::InitScene()
{
    //camera.setPosition(glm::vec3(3,2,10));
    camera.setPosition(glm::vec3(0,0,3));
    camera.setViewportAspectRatio(1);
    camera.setNearAndFarPlanes(0.5f, 100.0f);

    light.position = glm::vec3(-4,0,4);
    light.intensities = glm::vec3(1,1,1);
    light.attenuation = 0.1f;
    light.ambientCoefficient = 0.005f;

    backBufferProgram = LoaderTDOGL::LoadShaders("Volume/backBuffer.vert", "Volume/backBuffer.frag");
    raycastProgram = LoaderTDOGL::LoadShaders("Volume/raycast.vert", "Volume/raycast.frag");

    auto dir = "Volume/data/";
    std::vector<std::string> paths;
    for (int i = 0; i < 24; i++) {
        std::ostringstream str;
        str << dir << "flow" << i << ".txt";
        paths.push_back(str.str());
    }
    volume = LoaderTDOGL::LoadTexture(paths); //"App/data/wooden-crate.jpg");

    backCube = std::make_shared<Cube>();
    backCube->Bind(*backBufferProgram, "pos", 0, 0);

    frontCube = std::make_shared<Cube>();
    frontCube->Bind(*raycastProgram, "pos", 0, 0);
}


void SceneRaycast::Render()
{
    glm::mat4 mvp = camera.matrix() * world;

    glEnable(GL_CULL_FACE);

    backBuffer->use();
    backBufferProgram->use();
    backBufferProgram->setUniform("mvp", mvp);
    glCullFace(GL_FRONT);
    glClear(GL_COLOR_BUFFER_BIT);
    backCube->Draw();
    backBufferProgram->stopUsing();
    backBuffer->stopUsing();

    raycastProgram->use();
    backBuffer->texture->use(GL_TEXTURE0);
    volume->use(GL_TEXTURE1);
    raycastProgram->setUniform("mvp", mvp);
    raycastProgram->setUniform("backBuffer", 0);
    raycastProgram->setUniform("volume", 1);
    glCullFace(GL_BACK);
    frontCube->Draw();
    raycastProgram->stopUsing();
    volume->stopUsing();
    backBuffer->texture->stopUsing();

    glDisable(GL_CULL_FACE);
}


void SceneRaycast::InitOpenGL()
{
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");
    while(glGetError() != GL_NO_ERROR) {}
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    if (!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");

    glCullFace(GL_BACK);
    glClearColor(0, 0, 0, 0);
}

