#include "glw.h"

#include "Scene.h"
#include "Timer.h"

#include "boardloader.h"

#include "TetrisCube/Solver.h"
#include "TetrisCube/ParallelSolver.h"
#include "TetrisCube/CudaSolver.h"

#include <QTimer>
#include <QElapsedTimer>
#include <QShowEvent>

#include <GL/glew.h>

#include <iostream>

static std::string GetProcessPath() {
    return "/home/igor/Development/qt-workspace/App/App";
}

static std::string ResourcePath(std::string fileName) {
    return GetProcessPath() + "/data/" + fileName;
}

GLW::GLW(QWidget *parent) :
    QGLWidget(parent)
{
}

GLW::~GLW()
{
    GLuint textures[] = { textureBackground, textureCubeFace };
    glDeleteTextures(2, textures);
}

static const int numThreads = 8;
static const int splitLevel = 1;
static const size_t solutionsPerSolver = 1;
static const size_t solutionsPerDisplay = 100000;
static const std::string boardName = "Tetris4.xml";

static void Solve(Solver& solver, int threadId, int solverId) {
    Timer t;
    t.Start();
    size_t last = 0;
    while (solver.solutions.size() < solutionsPerSolver) {
        for (int i = 0; i < 150; i++) {
            if (solver.DoStep() < splitLevel) {
                //std::cout << "." << std::flush;
                printf("FINISHED! [solver %i, thread %i, solutions %i]\n",
                         solverId, threadId, (int)solver.solutions.size());
                return;
            }
        }
        if (solver.solutions.size() / solutionsPerDisplay != last / solutionsPerDisplay) {
            char buffer[256];
            snprintf(buffer, 256, "[solver %i, thread %i, solutions %i]",
                     solverId, threadId, (int)solver.solutions.size());
            t.Lap(buffer);
            last = solver.solutions.size();
        }
    }
    while (solver.solutions.size() > solutionsPerSolver) {
        solver.solutions.pop_back();
    }
//    printf("EXHAUSTED! [solver %i, thread %i, solutions %i]\n",
//             solverId, threadId, (int)solver.solutions.size());
}

void GLW::Init()
{
    BoardLoader bl;
    auto filename = ResourcePath(boardName);
    auto b = bl.Load(filename.c_str());

    Timer cudatime;
    cudatime.Start();

    CudaSolver cs(b);
    //cs.Solve_CPU(splitLevel, solutionsPerSolver, numThreads);
    //cs.Solve_GPU(splitLevel, solutionsPerSolver);

    cudatime.Lap("cuda");

    Timer tot;
    tot.Start();

    Timer t;
    t.Start();
    Solver solver(b);
    t.Lap("init");

    t.Start();
    ParallelSolver<Solver> ps(solver, splitLevel);
    printf("Split count: %i\n", (int)ps.GetSplitCount());
    t.Lap("split");
    printf("\n\n");

    t.Start();
    solver.solutions = ps.Solve(Solve, numThreads);
    //Solve(solver, 0, 0);
    t.Lap("Solve");

    printf("\nTotal solutions found: %i\n", (int)solver.solutions.size());
    tot.Lap("completion");
    printf("\n");
    fflush(0);

    printf("\nTotal solutions found by cuda: %i\n", (int)cs.solutions.size());

    scene.reset(new Scene());
    scene->SetSolution(!cs.solutions.empty() ? cs.solutions.front() : solver.solutions.front());
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
    textureCubeFace = LoadTexture(ResourcePath("cube.bmp").c_str());

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    Init();

    elapsedTimer.reset(new QElapsedTimer());
    elapsedTimer->start();

    fps = 0;
    keyPressed = 0;

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

void GLW::keyPress(QKeyEvent* event)
{
    keyPressed = event->key();
}

void GLW::keyRelease(QKeyEvent* event)
{
    if (keyPressed == event->key())
        keyPressed = 0;
}

void GLW::updateView()
{
    qint64 millisecs = elapsedTimer->restart();
    scene->Update((float)(millisecs / 1000.0), keyPressed);
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

    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, textureBackground);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2i(0, 0); glVertex2i(-1, -1);
    glTexCoord2i(0, 1); glVertex2i(-1, 1);
    glTexCoord2i(1, 0); glVertex2i(1, -1);
    glTexCoord2i(1, 1); glVertex2i(1, 1);
    glEnd();
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
