#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include <memory>

class GLW;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:

    void keyPressEvent(QKeyEvent* event);

    void keyReleaseEvent(QKeyEvent* event);

private slots:

    void UpdateFps(int fps);

private:
    GLW* glw;

    std::unique_ptr<Ui::MainWindow> ui;

    std::unique_ptr<QLabel> fpsLabel;
};

#endif // MAINWINDOW_H
