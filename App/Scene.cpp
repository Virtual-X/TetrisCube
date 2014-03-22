#include "Scene.h"



/*
 main

 Copyright 2012 Thomas Dalling - http://tomdalling.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

// third-party libraries
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// standard C++ libraries
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <list>

// tdogl classes
#include "tdogl/Program.h"
#include "tdogl/Texture.h"
#include "tdogl/Camera.h"

/*
 Represents a textured geometry asset

 Contains everything necessary to draw arbitrary geometry with a single texture:

  - shaders
  - a texture
  - a VBO
  - a VAO
  - the parameters to glDrawArrays (drawType, drawStart, drawCount)
 */
struct ModelAsset {
    tdogl::Program* shaders;
    tdogl::Texture* texture;
    GLuint vbo;
    GLuint vao;
    GLenum drawType;
    GLint drawStart;
    GLint drawCount;
    GLfloat shininess;
    glm::vec3 specularColor;

    ModelAsset() :
        shaders(NULL),
        texture(NULL),
        vbo(0),
        vao(0),
        drawType(GL_TRIANGLES),
        drawStart(0),
        drawCount(0),
        shininess(0.0f),
        specularColor(1.0f, 1.0f, 1.0f)
    {}
};

/*
 Represents an instance of an `ModelAsset`

 Contains a pointer to the asset, and a model transformation matrix to be used when drawing.
 */
struct ModelInstance {
    ModelAsset* asset;
    glm::mat4 transform;
    glm::vec4 color;

    std::vector<ModelInstance> children;
    glm::vec3 shiftDir;

    ModelInstance() :
        asset(NULL),
        transform()
    {}
};

/*
 Represents a point light
 */
struct Light {
    glm::vec3 position;
    glm::vec3 intensities; //a.k.a. the color of the light
    float attenuation;
    float ambientCoefficient;
};

// globals
tdogl::Camera gCamera;
ModelAsset gWoodenCrate;
std::vector<ModelInstance> gInstances;
GLfloat gDegreesRotated = 0.0f;
Light gLight;

static std::string GetProcessPath() {
    return "/home/igor/Development/qt-workspace/App/App";
}

// returns the full path to the file `fileName` in the resources directory of the app bundle
static std::string ResourcePath(const std::string& fileName) {
    return GetProcessPath() + "/data/" + fileName;
}

// returns a new tdogl::Program created from the given vertex and fragment shader filenames
static tdogl::Program* LoadShaders(const char* vertFilename, const char* fragFilename) {
    std::vector<tdogl::Shader> shaders;
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(vertFilename), GL_VERTEX_SHADER));
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(fragFilename), GL_FRAGMENT_SHADER));
    return new tdogl::Program(shaders);
}

#include <QImage>
#include <QGLWidget>

static tdogl::Bitmap LoadBitmap(const std::string& filename)
{
    QImage img;
    if(!img.load(filename.c_str())) {
        std::cerr << "error loading " << filename << std::endl ;
        throw 0;
    }

    QImage image;
    image = QGLWidget::convertToGLFormat(img);
    if(image.isNull()) {
        std::cerr << "error image" << std::endl ;
        throw 0;
    }

    return tdogl::Bitmap(image.width(), image.height(), tdogl::Bitmap::Format_RGBA, image.bits());
}

// returns a new tdogl::Texture created from the given filename
static tdogl::Texture* LoadTexture(const std::string& filename) {
    tdogl::Bitmap bmp(LoadBitmap(ResourcePath(filename)));
    bmp.flipVertically();
    return new tdogl::Texture(bmp);
}


// initialises the gWoodenCrate global
static void LoadWoodenCrateAsset() {
    // set all the elements of gWoodenCrate
    gWoodenCrate.shaders = LoadShaders("../vertex-shader.txt", "../fragment-shader.txt");
    gWoodenCrate.drawType = GL_TRIANGLES;
    gWoodenCrate.drawStart = 0;
    gWoodenCrate.drawCount = 6*2*3;
    //gWoodenCrate.texture = LoadTexture("wooden-crate.jpg");
    gWoodenCrate.texture = LoadTexture("cube.bmp");
    gWoodenCrate.shininess = 80.0;
    gWoodenCrate.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glGenBuffers(1, &gWoodenCrate.vbo);
    glGenVertexArrays(1, &gWoodenCrate.vao);

    // bind the VAO
    glBindVertexArray(gWoodenCrate.vao);

    // bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, gWoodenCrate.vbo);

    // Make a cube out of triangles (two triangles per side)
    GLfloat vertexData[] = {
        //  X     Y     Z       U     V          Normal
        // bottom
        -1.0f,-1.0f,-1.0f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
         1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
         1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
         1.0f,-1.0f, 1.0f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        -1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        // top
        -1.0f, 1.0f,-1.0f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
         1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
         1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
         1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,

        // front
        -1.0f,-1.0f, 1.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         1.0f,-1.0f, 1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
         1.0f,-1.0f, 1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // back
        -1.0f,-1.0f,-1.0f,   0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        -1.0f, 1.0f,-1.0f,   0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
         1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
         1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        -1.0f, 1.0f,-1.0f,   0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
         1.0f, 1.0f,-1.0f,   1.0f, 1.0f,   0.0f, 0.0f, -1.0f,

        // left
        -1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -1.0f,-1.0f,-1.0f,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

        // right
         1.0f,-1.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         1.0f, 1.0f,-1.0f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         1.0f,-1.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         1.0f, 1.0f,-1.0f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    // connect the xyz to the "vert" attribute of the vertex shader
    glEnableVertexAttribArray(gWoodenCrate.shaders->attrib("vert"));
    glVertexAttribPointer(gWoodenCrate.shaders->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), NULL);

    // connect the uv coords to the "vertTexCoord" attribute of the vertex shader
    glEnableVertexAttribArray(gWoodenCrate.shaders->attrib("vertTexCoord"));
    glVertexAttribPointer(gWoodenCrate.shaders->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,  8*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    // connect the normal to the "vertNormal" attribute of the vertex shader
    glEnableVertexAttribArray(gWoodenCrate.shaders->attrib("vertNormal"));
    glVertexAttribPointer(gWoodenCrate.shaders->attrib("vertNormal"), 3, GL_FLOAT, GL_TRUE,  8*sizeof(GLfloat), (const GLvoid*)(5 * sizeof(GLfloat)));

    // unbind the VAO
    glBindVertexArray(0);
}


// convenience function that returns a translation matrix
glm::mat4 translate(GLfloat x, GLfloat y, GLfloat z) {
    return glm::translate(glm::mat4(), glm::vec3(x,y,z));
}


// convenience function that returns a scaling matrix
glm::mat4 scale(GLfloat x, GLfloat y, GLfloat z) {
    return glm::scale(glm::mat4(), glm::vec3(x,y,z));
}

static ModelInstance GetCubeModel(const Coord& coord, const glm::vec4& color) {
    ModelInstance i;
    i.asset = &gWoodenCrate;
    float s = 2, t = -3;
    i.transform = translate(coord[0] * s + t, coord[1] * s + t, coord[2] * s + t);
    i.color = color;
    return i;
}

static ModelInstance GetPieceModel(const FixedPiece& piece) {
    const Coords& coords = piece.coords;
    const auto& cc = piece.color;
    glm::vec4 color(cc.red / 255.f, cc.green / 255.f, cc.blue / 255.f, 1);

    ModelInstance i;
    for (const auto& coord : coords) {
        i.children.push_back(GetCubeModel(coord, color));

    }
    auto min = Coord::LowerLimit(coords);
    auto max = Coord::UpperLimit(coords);
    float s = -1.5f;
    i.shiftDir = glm::vec3((min[0] + max[0]) / 2.0 + s, (min[1] + max[1]) / 2.0 + s, (min[2] + max[2]) / 2.0 + s);
    return i;
}

//create all the `instance` structs for the 3D scene, and add them to `gInstances`
static void CreateInstances(const Solution& solution) {
    gInstances.clear();
    gInstances.resize(1);

    for (const auto& p : solution.GetPieces()) {
        gInstances.back().children.push_back(GetPieceModel(p));
    }
}


//renders a single `ModelInstance`
static void DrawInstance(const ModelInstance& inst, const glm::mat4& transform) {
    ModelAsset* asset = inst.asset;
    tdogl::Program* shaders = asset->shaders;

    //bind the shaders
    shaders->use();

    //set the shader uniforms
    shaders->setUniform("camera", gCamera.matrix());
    shaders->setUniform("model", transform);
    shaders->setUniform("material.tex", 0); //set to 0 because the texture will be bound to GL_TEXTURE0
    shaders->setUniform("material.shininess", asset->shininess);
    shaders->setUniform("material.specularColor", asset->specularColor);
    shaders->setUniform("light.position", gLight.position);
    shaders->setUniform("light.intensities", gLight.intensities);
    shaders->setUniform("light.attenuation", gLight.attenuation);
    shaders->setUniform("light.ambientCoefficient", gLight.ambientCoefficient);
    shaders->setUniform("cameraPosition", gCamera.position());
    shaders->setUniform("modelColor", inst.color);

    //bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, asset->texture->object());

    //bind VAO and draw
    glBindVertexArray(asset->vao);
    glDrawArrays(asset->drawType, asset->drawStart, asset->drawCount);

    //unbind everything
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    shaders->stopUsing();
}

//renders a single `ModelInstance`
static void RenderInstance(const ModelInstance& inst, const glm::mat4& transform) {
    if (inst.asset)
        DrawInstance(inst, transform);
    for (const auto& c : inst.children)
        RenderInstance(c, transform * c.transform);
}

Scene::Scene() {
    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    // GLEW throws some errors, so discard all the errors so far
    while(glGetError() != GL_NO_ERROR) {}

    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // make sure OpenGL version 3.2 API is available
    if(!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // initialise the gWoodenCrate asset
    LoadWoodenCrateAsset();

    // setup gCamera
    gCamera.setPosition(glm::vec3(0,0,17));
    gCamera.setViewportAspectRatio(1);
    gCamera.setNearAndFarPlanes(0.5f, 100.0f);

    // setup gLight
    gLight.position = glm::vec3(5,0,25);
    gLight.intensities = glm::vec3(1,1,1); //white
    gLight.attenuation = 0.01f;
    gLight.ambientCoefficient = 0.05f;
}

void Scene::SetSolution(const Solution& solution)
{
    CreateInstances(solution);
}

void Scene::Resize(int width, int height) {
    gCamera.setViewportAspectRatio(width / (float)height);
}

static float gPieceOffset = 0;

static void UpdatePieceOffset() {
    for (ModelInstance& inst : gInstances[0].children) {
        const auto& sd = inst.shiftDir;
        inst.transform = translate(sd[0] * gPieceOffset, sd[1] * gPieceOffset, sd[2] * gPieceOffset);
    }
}

// update the scene based on the time elapsed since last update
void Scene::Update(float secondsElapsed, int key) {
    //rotate the first instance in `gInstances`
    const GLfloat degreesPerSecond = 15.0f;
    gDegreesRotated += secondsElapsed * degreesPerSecond;
    while(gDegreesRotated > 360.0f) gDegreesRotated -= 360.0f;

    float s = 0.5f;
    gInstances.front().transform = glm::rotate(scale(s, s, s), gDegreesRotated, glm::vec3(0,1,0));

    const float moveSpeed = 0.05; //units per second
    if(key == 'P'){
        gPieceOffset += moveSpeed;
        UpdatePieceOffset();
    } else if(key == 'L'){
        gPieceOffset -= moveSpeed;
        if (gPieceOffset < 0)
            gPieceOffset = 0;
        UpdatePieceOffset();
    }

//    //move position of camera based on WASD keys, and XZ keys for up and down
//    const float moveSpeed = 4.0; //units per second
//    if(glfwGetKey('S')){
//        gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.forward());
//    } else if(glfwGetKey('W')){
//        gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.forward());
//    }
//    if(glfwGetKey('A')){
//        gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.right());
//    } else if(glfwGetKey('D')){
//        gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.right());
//    }
//    if(glfwGetKey('Z')){
//        gCamera.offsetPosition(secondsElapsed * moveSpeed * -glm::vec3(0,1,0));
//    } else if(glfwGetKey('X')){
//        gCamera.offsetPosition(secondsElapsed * moveSpeed * glm::vec3(0,1,0));
//    }

//    //move light
//    if(glfwGetKey('1'))
//        gLight.position = gCamera.position();

//    // change light color
//    if(glfwGetKey('2'))
//        gLight.intensities = glm::vec3(1,0,0); //red
//    else if(glfwGetKey('3'))
//        gLight.intensities = glm::vec3(0,1,0); //green
//    else if(glfwGetKey('4'))
//        gLight.intensities = glm::vec3(1,1,1); //white


//    //rotate camera based on mouse movement
//    const float mouseSensitivity = 0.1f;
//    int mouseX, mouseY;
//    glfwGetMousePos(&mouseX, &mouseY);
//    gCamera.offsetOrientation(mouseSensitivity * mouseY, mouseSensitivity * mouseX);
//    glfwSetMousePos(0, 0); //reset the mouse, so it doesn't go out of the window

//    //increase or decrease field of view based on mouse wheel
//    const float zoomSensitivity = -0.2f;
//    float fieldOfView = gCamera.fieldOfView() + zoomSensitivity * (float)glfwGetMouseWheel();
//    if(fieldOfView < 5.0f) fieldOfView = 5.0f;
//    if(fieldOfView > 130.0f) fieldOfView = 130.0f;
//    gCamera.setFieldOfView(fieldOfView);
//    glfwSetMouseWheel(0);
}

void Scene::Render() {
    for (const auto& inst : gInstances){
        RenderInstance(inst, inst.transform);
    }
}
