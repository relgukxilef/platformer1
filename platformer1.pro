TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += GLEW_STATIC

win32: LIBS += -lglfw3dll -lglew32s -lopengl32

SOURCES += \
    main.cpp

INCLUDEPATH += libs

include("libs/ge1/ge1.pri")

DISTFILES += \
    shader/ground_fragment.glsl \
    shader/ground_vertex.glsl \
    shader/player_fragment.glsl \
    shader/player_vertex.glsl
