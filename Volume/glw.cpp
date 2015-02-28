#include "glw.h"

#include "SceneRaycast.h"
#include "SceneTDOGL.h"

#include "boardloader.h"

#include <QTimer>
#include <QElapsedTimer>
#include <QShowEvent>

#include <GL/glew.h>

#include <iostream>

//static std::string GetProcessPath() {
//    return "/home/igor/Development/qt-workspace/App/Volume";
//}

//static std::string ResourcePath(std::string fileName) {
//    return GetProcessPath() + "/data/" + fileName;
//}

GLW::GLW(QWidget *parent) :
    QGLWidget(parent)
{
//    BoardLoader bl;
//    auto filename = ResourcePath("Tetris3.xml");
//    auto b = bl.Load(filename.c_str());
}

GLW::~GLW()
{
    GLuint textures[] = { textureBackground, textureCubeFace };
    glDeleteTextures(2, textures);
}

GLuint GLW::LoadTexture(const char* name)
{
    QImage img;
    if(!img.load(name)) {
        std::cerr << "error loading " << name << std::endl ;
        return 0;
    }

    QImage image;
    image = QGLWidget::convertToGLFormat(img);
    if(image.isNull()) {
        std::cerr << "error image" << std::endl ;
        return 0;
    }

    std::cout << "Loaded image: " << image.width() << " x " << image.height() << std::endl;

    GLuint texture;
    //glActiveTexture(textureId);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, image.width(), image.height());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image.width(), image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
//    glTexSubImage2D(GL_TEXTURE_2D, 0,
//                    0, 0, image.width(), image.height(),
//                    GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

void GLW::initializeGL()
{
    char* version = (char*)glGetString(GL_VERSION);
    std::cout << "OpenGL version: " << version << std::endl;

    textureBackground = LoadTexture("/home/igor/Pictures/lovewallpaper.jpg");
    textureCubeFace = 0; //LoadTexture(ResourcePath("cube.bmp").c_str());

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    scene.reset(new SceneRaycast());

    elapsedTimer.reset(new QElapsedTimer());
    elapsedTimer->start();

    fps = 0;

    fpsTimer.reset(new QTimer());
    QObject::connect(fpsTimer.get(), SIGNAL(timeout()), this, SLOT(showFps()));
    fpsTimer->start(1000);

    redrawTimer.reset(new QTimer());
    QObject::connect(redrawTimer.get(), SIGNAL(timeout()), this, SLOT(updateView()));
}

void GLW::showFps()
{
    //std::cout << fps << "fps" << std::endl;
    emit UpdateFps(fps);
    fps = 0;
}

void GLW::updateView()
{
    qint64 millisecs = elapsedTimer->restart();
    scene->Update((float)(millisecs / 1000.0));
    updateGL();
}

void GLW::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
    width = w;
    height = h;

    scene->Resize(w, h);
}

void GLW::DrawBackground()
{
    //glActiveTexture(GL_TEXTURE0);

//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //return;

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBackground);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2i(0, 0); glVertex2i(-1, -1);
    glTexCoord2i(1, 0); glVertex2i(1, -1);
    glTexCoord2i(0, 1); glVertex2i(-1, 1);
    glTexCoord2i(1, 1); glVertex2i(1, 1);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}

void GLW::paintGL()
{
    DrawBackground();
    scene->Render();

    fps++;
    if (!redrawTimer->isActive())
        redrawTimer->start(0);
}
