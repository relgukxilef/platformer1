#include "physic_mesh.h"

#include <array>

using namespace glm;
using namespace ge1;
using namespace std;

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

        vec3 previous_v = v[2];
        for (auto i = 0u; i < 3; i++) {
            inside &= dot(cross(previous_v, v[i]), direction) < 0.0f;
            previous_v = v[i];
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

bool polygon_sphere_collision(
    array<vec3, 3> vertices, float radius, physic_mesh::collision& collision
) {
    float min_distance = radius * radius;

    // vertex collision
    for (auto v : vertices) {
        float distance = dot(v, v);
        if (distance < min_distance) {
            min_distance = distance;
            collision.contact_point = v;
        }
    }

    // edge collision
    vec3 previous_v = vertices[2];
    for (auto v : vertices) {
        auto edge_direction = v - previous_v;
        if (dot(v, edge_direction) > 0 && dot(previous_v, edge_direction) < 0) {
            // find closest point on edge
            auto t =
                -dot(previous_v, edge_direction) /
                (dot(v, edge_direction) - dot(previous_v, edge_direction));
            auto normal_offset = v * t + previous_v * (1 - t);
            auto distance = dot(normal_offset, normal_offset);
            if (distance < min_distance) {
                min_distance = distance;
                collision.contact_point = normal_offset;
            }
        }

        previous_v = v;
    }

    // face collision
    auto normal = cross(vertices[1] - vertices[0], vertices[2] - vertices[0]);
    bool inside = true;

    previous_v = vertices[2];
    for (auto v : vertices) {
        inside &= dot(cross(normal, v - previous_v), v) < 0.0f;
        previous_v = v;
    }
    if (inside) {
        normal = normalize(normal);
        auto distance = -dot(vertices[0], normal);
        if (distance * distance < min_distance) {
            min_distance = distance * distance;
            collision.contact_point = -normal * distance;
        }
    }

    collision.depth =
        normalize(collision.contact_point) *
        (length(collision.contact_point) - radius);

    // TODO: calculate actual contact_normal
    collision.contact_normal = normalize(collision.depth);

    return min_distance < radius * radius;
}

unsigned physic_mesh::sphere(
    vec3 center, float radius, span<collision> collision_buffer
) {
    auto face = faces.begin();
    collision* collision = collision_buffer.begin();
    unsigned collision_count = 0;

    unsigned face_index = 0;
    while (face != faces.end() && collision != collision_buffer.end()) {
        array<vec3, 3> v;
        for (auto i = 0u; i < 3; i++) {
            v[i] = vertices.begin()[face[i]] - center;
        }

        if (polygon_sphere_collision(v, radius, *collision)) {
            collision->contact_point += center;
            collision->face = face_index;
            collision++;
            collision_count++;
        }

        face_index++;
        face += 3;
    }

    return collision_count;
}
