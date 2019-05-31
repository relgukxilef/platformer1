TEMPLATE = app
CONFIG += console c++14 Wall
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += object_parallel_to_source

DEFINES += GLEW_STATIC

win32: LIBS += -lglfw3dll -lglew32s -lopengl32

SOURCES += \
    main.cpp \
    utility/vertex_buffer.cpp

INCLUDEPATH += libs

include("libs/ge1/ge1.pri")

DISTFILES += \
    shader/ground_fragment.glsl \
    shader/ground_vertex.glsl \
    shader/player_fragment.glsl \
    shader/player_vertex.glsl

HEADERS += \
    utility/vertex_buffer.h
