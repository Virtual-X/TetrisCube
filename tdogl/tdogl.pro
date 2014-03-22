#-------------------------------------------------
#
# Project created by QtCreator 2014-02-16T11:27:12
#
#-------------------------------------------------

QT       += opengl

QT       -= core gui

TARGET = tdogl
TEMPLATE = lib
CONFIG += staticlib

LIBS += -lGLEW

SOURCES += \
    Shader.cpp \
    Program.cpp \
    Texture.cpp \
    Camera.cpp \
    Bitmap.cpp

HEADERS += \
    Shader.h \
    Program.h \
    Texture.h \
    Camera.h \
    Bitmap.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES +=
