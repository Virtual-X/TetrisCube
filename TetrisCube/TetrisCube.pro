#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T22:29:04
#
#-------------------------------------------------

QT       -= core gui

TARGET = TetrisCube
TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

INCLUDEPATH +=

LIBS +=

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
    cuda_cpu.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
