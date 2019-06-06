TEMPLATE = app
CONFIG += console c++14 Wall
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += object_parallel_to_source

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast -march=native -funroll-loops -flto
QMAKE_LFLAGS_RELEASE += -flto

DEFINES += GLEW_STATIC

win32: LIBS += -lglfw3dll -lglew32s -lopengl32

SOURCES += \
    main.cpp \
    physics/physic_mesh.cpp \
    utility/io.cpp \
    utility/vertex_buffer.cpp

INCLUDEPATH += libs

include("libs/ge1/ge1.pri")

DISTFILES += \
    shader/ground_fragment.glsl \
    shader/ground_vertex.glsl \
    shader/player_fragment.glsl \
    shader/player_vertex.glsl

HEADERS += \
    physics/physic_mesh.h \
    utility/io.h \
    utility/vertex_buffer.h
