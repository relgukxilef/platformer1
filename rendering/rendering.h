#pragma once

#include <memory>

#include <glm/glm.hpp>

#include <ge1/draw_call.h>
#include <ge1/program.h>
#include <ge1/vertex_array.h>
#include <ge1/framebuffer.h>

#include "gameplay/gameplay.h"

namespace rendering {
    struct vr_window;

    struct mesh {
        unsigned count, first_index, base_vertex;
    };

    struct game {
        game();
        ~game();
        void update(const gameplay::game& g);
        void render();

        void set_desktop_size(unsigned width, unsigned height);

        struct {
            ge1::span<glm::mat3x4> model_transposed;
            // TODO: skeleton information
            ge1::span<ge1::draw_elements_indirect_command> command;
            ge1::span<mesh> mesh;
            ge1::span<glm::vec4> flash_color;
        } agents;

        struct view {
            glm::mat4 view_projection;
        };
        ge1::span<view> views[2];

        GLuint view_buffer[2];

        ge1::unique_buffer mesh_vertex_buffer;
        ge1::unique_buffer mesh_face_buffer;
        unsigned agent_command_buffer;
        unsigned agent_model_buffer;
        unsigned agent_flash_color_buffer;

        ge1::unique_program agent_program;
        ge1::unique_vertex_array agent_vertex_array;

        // TODO: might be able to share buffers
        unsigned arena_count;
        ge1::unique_buffer arena_vertex_buffer;
        ge1::unique_buffer arena_face_buffer;
        ge1::unique_program arena_program;
        ge1::unique_vertex_array arena_vertex_array;

        GLuint eye_framebuffers[2];
        GLuint eye_resolve_framebuffers[2];
        GLuint eye_multisample_renderbuffers[2];
        GLuint eye_depth_renderbuffers[2];
        GLuint eye_textures[2];

        // TODO: vr shouldn't be handled by rendering,
        // because it also concerns controller inputs
        vr_window* vr;

        GLint desktop_width, desktop_height;
    };
}
