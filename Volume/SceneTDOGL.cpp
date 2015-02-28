#include "SceneTDOGL.h"

#include "loadertdogl.h"


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
    std::shared_ptr<tdogl::Program> shaders;
    std::shared_ptr<tdogl::Texture> texture;
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
std::list<ModelInstance> gInstances;
GLfloat gDegreesRotated = 0.0f;
Light gLight;


// initialises the gWoodenCrate global
static void LoadWoodenCrateAsset() {
    // set all the elements of gWoodenCrate
    gWoodenCrate.shaders = LoaderTDOGL::LoadShaders("../vertex-shader.txt", "../fragment-shader.txt");
    gWoodenCrate.drawType = GL_TRIANGLES;
    gWoodenCrate.drawStart = 0;
    gWoodenCrate.drawCount = 6*2*3;
    //gWoodenCrate.texture = LoadTexture("wooden-crate.jpg");
    gWoodenCrate.texture = LoaderTDOGL::LoadTexture("cube.bmp");
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


//create all the `instance` structs for the 3D scene, and add them to `gInstances`
static void CreateInstances() {
    ModelInstance dot;
    dot.asset = &gWoodenCrate;
    dot.transform = glm::mat4();
    dot.color = glm::vec4(1,1,0,0.5f);
    gInstances.push_back(dot);

    ModelInstance i;
    i.asset = &gWoodenCrate;
    i.transform = translate(0,-4,0) * scale(1,2,1);
    i.color = glm::vec4(0,0,1,1);
    gInstances.push_back(i);

    ModelInstance hLeft;
    hLeft.asset = &gWoodenCrate;
    hLeft.transform = translate(-8,0,0) * scale(1,6,1);
    hLeft.color = glm::vec4(1,1,1,1);
    gInstances.push_back(hLeft);

    ModelInstance hRight;
    hRight.asset = &gWoodenCrate;
    hRight.transform = translate(-4,0,0) * scale(1,6,1);
    hRight.color = glm::vec4(1,1,1,1);
    gInstances.push_back(hRight);

    ModelInstance hMid;
    hMid.asset = &gWoodenCrate;
    hMid.transform = translate(-6,0,0) * scale(2,1,0.8f);
    hMid.color = glm::vec4(1,1,1,1);
    gInstances.push_back(hMid);
}


//renders a single `ModelInstance`
static void RenderInstance(const ModelInstance& inst) {
    ModelAsset* asset = inst.asset;
    auto shaders = asset->shaders;

    //bind the shaders
    shaders->use();

    //set the shader uniforms
    shaders->setUniform("camera", gCamera.matrix());
    shaders->setUniform("model", inst.transform);
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

SceneTDOGL::SceneTDOGL()
{
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

    // create all the instances in the 3D scene based on the gWoodenCrate asset
    CreateInstances();

    // setup gCamera
    gCamera.setPosition(glm::vec3(-4,0,17));
    gCamera.setViewportAspectRatio(1);
    gCamera.setNearAndFarPlanes(0.5f, 100.0f);

    // setup gLight
    gLight.position = glm::vec3(-4,0,4);
    gLight.intensities = glm::vec3(1,1,1); //white
    gLight.attenuation = 0.1f;
    gLight.ambientCoefficient = 0.005f;
}

void SceneTDOGL::Resize(int width, int height) {
    gCamera.setViewportAspectRatio(width / (float)height);
}


// update the scene based on the time elapsed since last update
void SceneTDOGL::Update(float secondsElapsed) {
    //rotate the first instance in `gInstances`
    const GLfloat degreesPerSecond = 180.0f;
    gDegreesRotated += secondsElapsed * degreesPerSecond;
    while(gDegreesRotated > 360.0f) gDegreesRotated -= 360.0f;
    gInstances.front().transform = glm::rotate(glm::mat4(), gDegreesRotated, glm::vec3(0,1,0));

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

void SceneTDOGL::Render() {
    std::list<ModelInstance>::const_iterator it;
    for(it = gInstances.begin(); it != gInstances.end(); ++it){
        RenderInstance(*it);
    }
}
