#pragma once

#include <stdexcept>

#include <GL/glew.h>

#include <ge1/span.h>

// This file should be added to ge1

template<class T>
GLuint create_mapped_uniform_buffer(T *&data) {
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

GLuint create_mapped_uniform_buffer(ge1::span<const buffer_entry> entries);

template <class T>
ge1::span<T> create_mapped_buffer(GLuint& name, unsigned size) {
    glGenBuffers(1, &name);

    glBindBuffer(GL_COPY_WRITE_BUFFER, name);
    glBufferStorage(
        GL_COPY_WRITE_BUFFER, size * sizeof(T), nullptr,
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    );
    T* pointer = reinterpret_cast<T*>(glMapBufferRange(
        GL_COPY_WRITE_BUFFER, 0, size * sizeof(T),
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    ));

    if (pointer == nullptr)
        throw std::runtime_error("Mapping of buffer failed");

    return {pointer, pointer + size};
}


