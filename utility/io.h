#pragma once

#include <ge1/span.h>

// TODO: should return a unique_span
ge1::span<unsigned char> read_file(const char* filename);

template<class Type>
ge1::span<Type> read_file(const char* filename) {
    auto buffer = read_file(filename);
    return {
        reinterpret_cast<Type*>(buffer.begin()),
        reinterpret_cast<Type*>(buffer.end())
    };
}
