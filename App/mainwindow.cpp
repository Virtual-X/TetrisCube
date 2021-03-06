#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "glw.h"

#include <QKeyEvent>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    glw = new GLW(ui->frame);

//    std::cout << ui->frame << " " << ui->frame->layout() << std::endl;

    ui->frame->layout()->addWidget(glw);

    fpsLabel.reset(new QLabel(this));
    UpdateFps(0);
    fpsLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->statusBar->addWidget(fpsLabel.get());

    connect(glw, SIGNAL(UpdateFps(int)), this, SLOT(UpdateFps(int)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    glw->keyPress(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    glw->keyRelease(event);
}


void MainWindow::UpdateFps(int fps)
{
    fpsLabel->setText(QString::number(fps) + " fps");
}
