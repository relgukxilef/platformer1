#pragma once

#include <GL/glew.h>

#include <ge1/span.h>

// This file should be added to ge1

template<class T>
GLuint create_mapped_buffer(T *&data) {
    GLuint name;
    glGenBuffers(1, &name);

    glBindBuffer(GL_COPY_WRITE_BUFFER, name);
    glBufferStorage(
        GL_COPY_WRITE_BUFFER, sizeof(T), nullptr,
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    );
    data = reinterpret_cast<T*>(glMapBufferRange(
        GL_COPY_WRITE_BUFFER, 0, sizeof(T),
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    ));

    return name;
}

template<class T>
GLuint create_static_buffer(ge1::span<T> data) {
    GLuint name;
    glGenBuffers(1, &name);

    glBindBuffer(GL_COPY_WRITE_BUFFER, name);
    glBufferData(
        GL_COPY_WRITE_BUFFER, data.size() * sizeof(T), data.begin(),
        GL_STATIC_DRAW
    );

    return name;
}

// TODO: rename to create_buffer_from_file
GLuint load_buffer_from_file(const char* filename, GLint& size);

inline GLuint load_buffer_from_file(const char* filename) {
    GLint size;
    return load_buffer_from_file(filename, size);
}

struct buffer_entry_file {
    GLuint& offset, & size;
    const char* filename;
};

GLuint load_buffer_from_file(ge1::span<buffer_entry_file> filenames);

struct buffer_entry {
    template<class T>
    buffer_entry(GLuint binding, T*& pointer, unsigned size = 1) :
        pointer(reinterpret_cast<void**>(&pointer)), size(size * sizeof(T)),
        binding(binding)
    {}

    void** pointer;
    unsigned size;
    GLuint binding;
};

// TODO: rename to create_mapped_uniform_buffer
GLuint create_mapped_buffer(ge1::span<buffer_entry> entries);


