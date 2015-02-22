#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:

    void UpdateFps(int fps);

private:
    std::unique_ptr<Ui::MainWindow> ui;

    std::unique_ptr<QLabel> fpsLabel;
};

#endif // MAINWINDOW_H
