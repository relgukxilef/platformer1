#include "rendering.h"

#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include "vr.h"

#include "utility/vertex_buffer.h"

#include <ge1/framebuffer.h>

using namespace rendering;

const unsigned MAX_PLAYER_COUNT = 1;
const unsigned MAX_AGENT_COUNT = gameplay::MAX_ENEMY_COUNT + MAX_PLAYER_COUNT;

struct transform {
    glm::vec3 position;
    float yaw;
};

glm::mat3x4 get_model_matrix_transposed(const transform& t) {
    auto model = translate(
        glm::mat4(1), t.position
    );
    model = rotate(
        model, -t.yaw, {0, 0, 1}
    );
    return glm::mat3x4(glm::transpose(model));
};

enum : GLuint {
    position_attribute,
    normal_attribute,
    model_0_attribute,
    model_1_attribute,
    model_2_attribute,
    flash_color_attribute,
};
enum : GLuint {
    view_properties_binding,
    player_properties_binding,
    enemy_properties_binding,
};

rendering::game::game() {
    vr = new vr_window();

    unsigned samples = 8;

    for (auto i = 0u; i < 2; i++) {
        // render buffers
        eye_framebuffers[i] = ge1::create_framebuffer_multisample(
            vr->width, vr->height, samples, {
                {
                    GL_COLOR_ATTACHMENT0, &eye_multisample_renderbuffers[i],
                    GL_RGBA8
                }
            }, {
                {
                    GL_DEPTH_ATTACHMENT, &eye_depth_renderbuffers[i],
                    GL_DEPTH_COMPONENT
                }
            }
        );

        // resolve
        eye_resolve_framebuffers[i] = ge1::create_framebuffer(
            vr->width, vr->height, GL_TEXTURE_2D, {
                {
                    GL_COLOR_ATTACHMENT0, &eye_textures[i],
                    GL_RGBA8
                },
            }
        );

        views[i] = create_mapped_buffer<view>(
            view_buffer[i], sizeof(view)
        );
    }

    vr->set_textures(eye_textures[0], eye_textures[1]);


    agent_program = ge1::compile_program(
        "shader/agent_vertex.glsl", nullptr, nullptr, nullptr,
        "shader/agent_fragment.glsl", {},
        {{"position", position_attribute}}
    );
    ge1::bind_uniform_blocks(agent_program.get_name(), {
       {"view_properties", view_properties_binding}
    });

    agents.command = create_mapped_buffer<ge1::draw_elements_indirect_command>(
        agent_command_buffer, MAX_AGENT_COUNT
    );
    agents.model_transposed = create_mapped_buffer<glm::mat3x4>(
        agent_model_buffer, MAX_AGENT_COUNT
    );

    agents.flash_color = create_mapped_buffer<glm::vec4>(
        agent_flash_color_buffer, MAX_AGENT_COUNT
    );

    std::vector<unsigned char> vertices;
    {
        std::ifstream file("models/miku_vertices.vbo", std::ios::binary);
        file.unsetf(std::ios::skipws);
        std::istream_iterator<unsigned char> start(file), end;
        std::move(start, end, std::back_inserter(vertices));
    }

    mesh_vertex_buffer = create_static_buffer<unsigned char>(
        {vertices.data(), vertices.data() + vertices.size()}
    );

    std::vector<unsigned char> faces;
    {
        std::ifstream file("models/miku_faces.vbo", std::ios::binary);
        file.unsetf(std::ios::skipws);
        std::istream_iterator<unsigned char> start(file), end;
        std::move(start, end, std::back_inserter(faces));
    }

    mesh_face_buffer = create_static_buffer<unsigned char>(
        {faces.data(), faces.data() + faces.size()}
    );

    agents.mesh = ge1::span<mesh>(1);
    agents.mesh[0] = {
        static_cast<unsigned int>(faces.size() / sizeof(unsigned)), 0, 0
    };

    agent_vertex_array = ge1::create_vertex_array(
        {
            {
                mesh_vertex_buffer.get_name(), position_attribute,
                3, GL_FLOAT, false, 8 * sizeof(float), 0
            }, {
                mesh_vertex_buffer.get_name(), normal_attribute,
                3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float)
            }, {
                agent_model_buffer, model_0_attribute,
                4, GL_FLOAT, false, 3 * 4 * sizeof(float), 0 * sizeof(float)
            }, {
                agent_model_buffer, model_1_attribute,
                4, GL_FLOAT, false, 3 * 4 * sizeof(float), 4 * sizeof(float)
            }, {
                agent_model_buffer, model_2_attribute,
                4, GL_FLOAT, false, 3 * 4 * sizeof(float), 8 * sizeof(float)
            }, {
                agent_flash_color_buffer, flash_color_attribute,
                4, GL_FLOAT, false, 4 * sizeof(float), 0
            }
        }, mesh_face_buffer.get_name(), agent_command_buffer
    );
    glVertexAttribDivisor(model_0_attribute, 1);
    glVertexAttribDivisor(model_1_attribute, 1);
    glVertexAttribDivisor(model_2_attribute, 1);
    glVertexAttribDivisor(flash_color_attribute, 1);

    for (auto i = 0u; i < agents.command.size(); i++) {
        agents.command[i] = {0, 0, 0, 0, i};
    }

    // set player mesh
    agents.command[0].instance_count = 1;
    agents.command[0].count = agents.mesh[0].count;
    agents.command[0].base_vertex = agents.mesh[0].base_vertex;
    agents.command[0].first_index = agents.mesh[0].first_index;


    // load arena
    arena_program = ge1::compile_program(
        "shader/ground_vertex.glsl", nullptr, nullptr, nullptr,
        "shader/ground_fragment.glsl", {},
        {
            {"position", position_attribute},
            {"normal", normal_attribute},
        }
    );
    ge1::bind_uniform_blocks(agent_program.get_name(), {
       {"view_properties", view_properties_binding}
    });

    arena_vertex_buffer = load_buffer_from_file("models/arena_vertices.vbo");
    GLint arena_size;
    arena_face_buffer =
        load_buffer_from_file("models/arena_faces.vbo", arena_size);
    arena_count = arena_size;

    arena_vertex_array = ge1::create_vertex_array(
        {
            {
                arena_vertex_buffer.get_name(), position_attribute,
                3, GL_FLOAT, false, 8 * sizeof(float), 0
            }, {
                 arena_vertex_buffer.get_name(), normal_attribute,
                 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float)
            }
        }, arena_face_buffer.get_name()
    );
}

game::~game() {
    delete vr;
}

void update(game &game, unsigned index, const gameplay::agent& agent) {
    if (agent.active_state == &gameplay::hit) {
        game.agents.flash_color[index] = glm::vec4(
            1, 0, 0, std::max(1 - agent.state_time / 0.1f, 0.f)
        );
    } else if (agent.active_state == &gameplay::charging) {
        game.agents.flash_color[index] = glm::vec4(
            0, 0, 1, std::max(1 - agent.state_time / 0.1f, 0.f)
        );
    } else if (agent.active_state == &gameplay::evading) {
        game.agents.flash_color[index] = glm::vec4(
            0, 1, 0, 0.5
        );
    } else {
        game.agents.flash_color[index] = glm::vec4(0);
    }
}

void game::update(const gameplay::game& g) {
    agents.model_transposed[0] =
        get_model_matrix_transposed({g.player.position, g.player.yaw});
    ::update(*this, 0, g.player);

    for (auto i = 0u; i < gameplay::MAX_ENEMY_COUNT; i++) {
        const auto& enemy = g.enemies[i];
        if (enemy.active_state != &gameplay::dead) {
            agents.command[i + MAX_PLAYER_COUNT].instance_count = 1;
            agents.command[i + MAX_PLAYER_COUNT].count = agents.mesh[0].count;
            agents.command[i + MAX_PLAYER_COUNT].base_vertex =
                agents.mesh[0].base_vertex;
            agents.command[i + MAX_PLAYER_COUNT].first_index =
                agents.mesh[0].first_index;
            agents.model_transposed[i + MAX_PLAYER_COUNT] =
                get_model_matrix_transposed({enemy.position, enemy.yaw});
        } else {
            agents.command[i + MAX_PLAYER_COUNT].instance_count = 0;
        }
        ::update(*this, i + MAX_PLAYER_COUNT, enemy);
    }
}

void rendering::game::render() {
    vr->wait_and_poll();

    glFinish();
    for (auto i = 0u; i < 2; i++) {
        views[i][0].view_projection = vr->view_projection_matrices[i];
    }

    for (auto i = 0u; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, eye_framebuffers[i]);

        glViewport(0, 0, vr->width, vr->height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBufferBase(
            GL_UNIFORM_BUFFER, view_properties_binding, view_buffer[i]
        );

        // render environment
        glUseProgram(arena_program.get_name());
        glBindVertexArray(arena_vertex_array.get_name());
        glDrawElements(GL_TRIANGLES, arena_count, GL_UNSIGNED_INT, 0);

        // render players and enemies
        glUseProgram(agent_program.get_name());
        glBindVertexArray(agent_vertex_array.get_name());
        glMultiDrawElementsIndirect(
            GL_TRIANGLES, GL_UNSIGNED_INT, nullptr,
            MAX_AGENT_COUNT,
            sizeof(ge1::draw_elements_indirect_command)
        );

        // resolve samples
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eye_resolve_framebuffers[i]);
        glBlitFramebuffer(
            0, 0, vr->width, vr->height, 0, 0, vr->width, vr->height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
    }

    vr->submit();

    // render desktop view
    glBindFramebuffer(GL_READ_FRAMEBUFFER, eye_resolve_framebuffers[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    float scale = std::min(
        static_cast<float>(vr->width) / desktop_width,
        static_cast<float>(vr->height) / desktop_height
    );

    GLint
        left = 0.5 * (vr->width - desktop_width * scale),
        right = vr->width - left,
        bottom = 0.5 * (vr->height - desktop_height * scale),
        top = vr->height - bottom;

    glBlitFramebuffer(
        left, bottom, right, top,
        0, 0, desktop_width, desktop_height,
        GL_COLOR_BUFFER_BIT, GL_LINEAR
    );

    // TODO: desktop only rendering?
}

void game::set_desktop_size(unsigned width, unsigned height) {
    desktop_width = width;
    desktop_height = height;
}
