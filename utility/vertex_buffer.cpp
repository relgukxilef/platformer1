#include "vertex_buffer.h"

#include <fstream>
#include <iterator>
#include <vector>
#include <memory>

using namespace std;

GLuint load_buffer_from_file(const char *filename, GLint &size) {
    ifstream file(filename, ios::in | ios::binary);
    file.unsetf(ios::skipws);

    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open "s + filename);
    }

    istream_iterator<unsigned char> start(file), end;
    vector<unsigned char> data(start, end);
    file.close();

    size = static_cast<GLint>(data.size());

    return create_static_buffer<unsigned char>(
        {data.data(), data.data() + data.size()}
    );
}

GLuint load_buffer_from_file(ge1::span<buffer_entry_file> filenames) {
    auto size = 0u;
    for (auto entry : filenames) {
        ifstream file(entry.filename, ios::ate | ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Couldn't open "s + entry.filename);
        }
        entry.offset = size;
        size += file.tellg();
    }

    unique_ptr<char[]> data(new char[size]);
    char* offset = data.get();

    for (auto entry : filenames) {
        ifstream file(entry.filename, ios::binary);
        file.unsetf(ios::skipws);
        istream_iterator<unsigned char> start(file), end;
        // TODO: assumes file length has not changed
        offset = move(start, end, offset);
    }

    GLuint name;
    glGenBuffers(1, &name);

    glBindBuffer(GL_COPY_WRITE_BUFFER, name);
    glBufferData(
        GL_COPY_WRITE_BUFFER, size, data.get(),
        GL_STATIC_DRAW
    );

    return name;
}

unsigned ceiling(unsigned x, unsigned y) {
    return (1 + ((x - 1) / y)) * y;
}

GLuint create_mapped_uniform_buffer(ge1::span<const buffer_entry> entries) {
    GLint signed_alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &signed_alignment);
    auto alignment = static_cast<unsigned>(signed_alignment);

    auto size = 0u;
    for (auto entry : entries) {
        size += ceiling(entry.size, alignment);
    }

    char* pointer;
    GLuint name;
    glGenBuffers(1, &name);

    glBindBuffer(GL_COPY_WRITE_BUFFER, name);
    glBufferStorage(
        GL_COPY_WRITE_BUFFER, size, nullptr,
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    );
    pointer = reinterpret_cast<char*>(glMapBufferRange(
        GL_COPY_WRITE_BUFFER, 0, size,
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    ));

    unsigned offset = 0;

    for (auto entry : entries) {
        *entry.pointer = pointer;
        auto aligned_size = ceiling(entry.size, alignment);
        glBindBufferRange(
            GL_UNIFORM_BUFFER, entry.binding, name, offset, entry.size
        );
        pointer += aligned_size;
        offset += aligned_size;
    }

    return name;
}
