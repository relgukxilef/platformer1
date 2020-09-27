#pragma once

#include <glm/glm.hpp>

#include <ge1/draw_call.h>
#include <ge1/program.h>
#include <ge1/vertex_array.h>

#include "gameplay/gameplay.h"

namespace rendering {
    struct mesh {
        unsigned count, first_index, base_vertex;
    };

    struct game {
        game();
        void update(const gameplay::game& g);
        void render();

        struct {
            ge1::span<glm::mat3x4> model_transposed;
            // TODO: skeleton information
            ge1::span<ge1::draw_elements_indirect_command> command;
            ge1::span<mesh> mesh;
        } agents;

        ge1::unique_buffer mesh_vertex_buffer;
        ge1::unique_buffer mesh_face_buffer;
        unsigned agent_command_buffer;
        unsigned agent_model_buffer;

        ge1::unique_program agent_program;
        ge1::unique_vertex_array agent_vertex_array;
    };
}
