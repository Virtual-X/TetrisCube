#ifndef GLW_H
#define GLW_H

#include "GL/glew.h"

#include <QGLWidget>

#include <memory>

class Scene;
class QElapsedTimer;

class GLW : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLW(QWidget *parent = 0);
    ~GLW();

signals:
    
    void UpdateFps(int fps);

public slots:

protected:
    void initializeGL();

    void resizeGL(int w, int h);

    void paintGL();

private slots:

    void showFps();
    void updateView();

private:
    GLuint LoadTexture(const char* name);

    void DrawBackground();

    int width, height;

    GLuint textureBackground;
    GLuint textureCubeFace;

    int fps;

    std::unique_ptr<Scene> scene;
    std::unique_ptr<QTimer> redrawTimer;
    std::unique_ptr<QElapsedTimer> elapsedTimer;
    std::unique_ptr<QTimer> fpsTimer;
};

#endif // GLW_H
