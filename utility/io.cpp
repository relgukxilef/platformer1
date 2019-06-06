#include "io.h"

#include <stdio.h>
#include <stdexcept>

using namespace std;

ge1::span<unsigned char> read_file(const char *filename) {
    FILE* file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    auto size = static_cast<unsigned>(ftell(file));

    unsigned char* buffer = new unsigned char[size];

    fseek(file, 0, SEEK_SET);
    auto read_size = fread(buffer, sizeof(unsigned char), size, file);

    if (read_size != size) {
        throw runtime_error("Couldn't open "s + filename);
    }

    return {buffer, buffer + size};
}
