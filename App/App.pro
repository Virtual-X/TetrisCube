#-------------------------------------------------
#
# Project created by QtCreator 2014-02-05T18:40:56
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = App
TEMPLATE = app

CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

INCLUDEPATH += ..

LIBS += -lGLEW

debug {
LIBS += \
    ../build-tdogl-Desktop-Debug/libtdogl.a \
    ../../../cuda-workspace/cuda_tetriscube/Debug/libcuda_tetriscube.so
}

release {
LIBS += \
    ../build-tdogl-Desktop-Debug/libtdogl.a \
    ../../../cuda-workspace/cuda_tetriscube/Release/libcuda_tetriscube.so
}

SOURCES += main.cpp\
        mainwindow.cpp \
    glw.cpp \
    Scene.cpp \
    boardloader.cpp

HEADERS  += mainwindow.h \
    glw.h \
    Scene.h \
    boardloader.h \
    Timer.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    vertex-shader.txt \
    fragment-shader.txt
