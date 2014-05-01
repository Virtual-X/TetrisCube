#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T22:29:04
#
#-------------------------------------------------

QT       -= core gui widgets opengl

TARGET = TetrisCube
TEMPLATE = app
#TEMPLATE = lib
#CONFIG += staticlib

#CONFIG -= x86_64 ppc64
#CONFIG += x86 ppc

CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_CXXFLAGS += -march=native -mtune=native

INCLUDEPATH +=

LIBS += -Wl,--no-as-needed

OTHER_FILES += \
    ../../../cuda-workspace/cuda_tetriscube/src/cuda_tc.h \
    ../../../cuda-workspace/cuda_tetriscube/src/cuda_tc.cu

HEADERS += \
    Vector.h \
    UnsortedPermutator.h \
    Solution.h \
    Piece.h \
    Grid.h \
    Geometry3D.h \
    Coord.h \
    Configuration.h \
    Board.h \
    FixedPiece.h \
    PieceColor.h \
    ParallelSolver.h \
    Solver.h \
    CandidatesCalculator.h \
    CudaSolver.h \
    cuda_cpu.h \
    SolverT.h \
    Stats.h \
    Main.h \
    Timer.h \
    BoardLoader.h \
    Coder.h \
    CandidatesT.h \
    Constants.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

SOURCES += \
    Main.cpp \
    BoardLoader.cpp
