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

QMAKE_CXXFLAGS += -march=native -mtune=native

INCLUDEPATH += ..

LIBS += -lGLEW

debug {
LIBS += \
    ../build-tdogl-Desktop-Debug/libtdogl.a
}

release {
LIBS += \
    ../build-tdogl-Desktop-Debug/libtdogl.a
}

SOURCES += main.cpp\
        mainwindow.cpp \
    glw.cpp \
    Scene.cpp \
    boardloader.cpp

HEADERS  += mainwindow.h \
    glw.h \
    Scene.h \
    boardloader.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    vertex-shader.txt \
    fragment-shader.txt
