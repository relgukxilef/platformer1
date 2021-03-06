#pragma once

#include <ge1/span.h>

#include <glm/glm.hpp>

struct physic_mesh {
    ge1::span<const glm::vec3> vertices;
    ge1::span<const unsigned> faces;
    // TODO: some operations are faster on edges
    // TODO: might need interpolated normals

    struct hit {
        glm::vec3 contact_point;
        unsigned contact_face;
    };

    hit ray(glm::vec3 start, glm::vec3 end) const;

    struct collision {
        glm::vec3 contact_point, contact_normal;
        glm::vec3 depth; // moving object by this vector resolves collision
        unsigned face;
    };

    unsigned sphere(
        glm::vec3 center, float radius, ge1::span<collision> collision_buffer
    );
};
