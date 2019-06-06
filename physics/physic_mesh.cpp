#include "physic_mesh.h"

using namespace glm;

physic_mesh::hit physic_mesh::ray(vec3 start, vec3 end) const {
    auto face = faces.begin();
    auto direction = end - start;
    auto best_distance = 1.f;
    hit best_hit{end, static_cast<unsigned>(-1)};

    unsigned face_index = 0;
    while (face != faces.end()) {
        vec3 v[3];
        for (auto i = 0u; i < 3; i++) {
            v[i] = vertices.begin()[face[i]] - start;
        }

        bool inside = true;

        for (auto i = 0u; i < 3; i++) {
            inside &= dot(cross(v[i], v[(i + 1) % 3]), direction) < 0.0f;
        }

        if (inside) {
            auto normal = cross(v[1] - v[0], v[2] - v[0]);
            auto face_distance = dot(v[0], normal);
            auto direction_distance = dot(direction, normal);
            auto t = face_distance / direction_distance;
            if (t > 0 && best_distance > t) {
                best_distance = t;
                best_hit.contact_face = face_index;
                best_hit.contact_point = start + t * direction;
            }
        }

        face += 3;
        face_index++;
    }

    return best_hit;
}
