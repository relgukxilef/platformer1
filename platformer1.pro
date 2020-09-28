TEMPLATE = app
CONFIG += console c++17 Wall
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += object_parallel_to_source

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast -march=native -funroll-loops -flto
QMAKE_LFLAGS_RELEASE += -flto

DEFINES += GLEW_STATIC

win32: LIBS += -lglfw3dll -lglew32s -lopengl32 -lopenvr_api

SOURCES += \
    gameplay/gameplay.cpp \
    main.cpp \
    physics/physic_mesh.cpp \
    rendering/rendering.cpp \
    rendering/vr.cpp \
    utility/io.cpp \
    utility/vertex_buffer.cpp

INCLUDEPATH += libs

include("libs/game_engine1/ge1.pri")

DISTFILES += \
    shader/agent_fragment.glsl \
    shader/agent_vertex.glsl \
    shader/ground_fragment.glsl \
    shader/ground_vertex.glsl

HEADERS += \
    gameplay/gameplay.h \
    physics/physic_mesh.h \
    rendering/rendering.h \
    rendering/vr.h \
    utility/io.h \
    utility/vertex_buffer.h
