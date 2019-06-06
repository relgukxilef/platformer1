#pragma once

#include <ge1/span.h>

#include <glm/glm.hpp>

struct physic_mesh {
    ge1::span<glm::vec3> vertices;
    ge1::span<unsigned> faces;
    // TODO: some operations are faster on edges

    struct hit {
        glm::vec3 contact_point;
        unsigned contact_face;
    };

    hit ray(glm::vec3 start, glm::vec3 end) const;
};
