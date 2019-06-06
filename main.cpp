#include <iostream>
#include <cmath>
#include <fstream>
#include <iterator>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ge1/composition.h>
#include <ge1/vertex_array.h>
#include <ge1/program.h>

#include "utility/vertex_buffer.h"
#include "utility/io.h"

#include "physics/physic_mesh.h"

using namespace std;
using namespace glm;

struct unique_glfw {
    unique_glfw() {
        if (!glfwInit()) {
            throw runtime_error("Couldn't initialize GLFW!");
        }
    }

    ~unique_glfw() {
        glfwTerminate();
    }
};

static unique_glfw glfw;

enum : GLuint {
    position_attribute,
    normal_attribute
};
enum : GLuint {
    view_properties_binding,
    player_properties_binding,
};

struct mesh { // TODO: maybe rename to display_mesh
    mesh(GLuint vertices, GLuint faces, GLint size) :
        vertices(vertices), faces(faces),
        vertex_array(ge1::create_vertex_array(
            {
                {
                    this->vertices.get_name(), position_attribute,
                    3, GL_FLOAT, false, 8 * sizeof(float), 0
                }, {
                     this->vertices.get_name(), normal_attribute,
                     3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float)
                }
            }, this->faces.get_name()
        )),
        size(size)
    {}
    mesh(ge1::span<float> vertices, ge1::span<unsigned> faces) :
        mesh(
            create_static_buffer(vertices), create_static_buffer(faces),
            static_cast<GLint>(faces.size())
        )
    {}

    ge1::unique_buffer vertices, faces;
    ge1::unique_vertex_array vertex_array;
    GLint size;
};

mesh load_mesh(const char* vertices_filename, const char* faces_filename) {
    GLint size;
    auto faces = load_buffer_from_file(faces_filename, size);
    return {
        load_buffer_from_file(vertices_filename),
        faces,
        size / static_cast<GLint>(sizeof(unsigned))
    };
}

struct draw_elements_call : public ge1::renderable {
    draw_elements_call(
        GLuint vertex_array, GLint count, GLuint program,
        GLenum mode, GLenum type
    ) :
        vertex_array(vertex_array), count(count), program(program),
        mode(mode), type(type)
    {}

    void render() override {
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glDrawElements(mode, count, type, nullptr);
    }

    GLuint vertex_array;
    GLint count;
    GLuint program;
    GLenum mode;
    GLenum type;
};

static struct {
    mat4 view_projection;
} *view_properties;

static struct player_properties {
    mat4 model;
} *players;

static float gravity = 2.f / 60;

typedef unsigned ticks;

static struct {
    float aim_yaw = 0, aim_pitch = 0, head_yaw = 0, head_pitch = 0;
    float jump_force = 1;
    vec3 position = {0, 0, 1};
    vec3 velocity = {0, 0, 0};
    enum {
        none,
        beam
    } animation;
    ticks animation_time;
    glm::vec3 ability_start_position, ability_start_direction;
    bool on_ground;
} physic_players[2];

struct ability {
    ticks startup, active, recovery;
};

static mat4 view_matrix, projection_matrix;

void window_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);

    projection_matrix = perspective(
        radians(60.f), static_cast<float>(width) / height, 0.1f, 1000.0f
    );
}

void cursor_position_callback(GLFWwindow*, double x, double y) {
    physic_players[0].head_yaw = static_cast<float>(x) * 0.005f;
    physic_players[0].head_pitch = static_cast<float>(y) * 0.005f;
}

void mouse_button_callback(
    GLFWwindow*, int button, int action, int /*modifiers*/
) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            auto& player = physic_players[0];
            if (player.animation == player.none) {
                player.animation = player.beam;
                player.animation_time = 0;
            }
        }
    }
}

void key_callback(
    GLFWwindow*, int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/
) {
}


int main() {
    GLFWwindow* window;

    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwSwapInterval(1);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    int screen_width = mode->width, screen_height = mode->height;

    window = glfwCreateWindow(
        screen_width, screen_height, "demo", nullptr, nullptr
    );
    if (!window) {
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    view_matrix = lookAt(vec3{0, 5, 1}, {0, 0, 1}, {0, 0, 1});

    ge1::unique_buffer properties_buffer = create_mapped_buffer({
        {view_properties_binding, view_properties},
        {player_properties_binding, players, 2},
    });

    ge1::unique_program ground_program = ge1::compile_program(
        "shader/ground_vertex.glsl", nullptr, nullptr, nullptr,
        "shader/ground_fragment.glsl", {},
        {
            {"position", position_attribute},
            {"normal", normal_attribute},
        }, {},
        {{"view_properties", view_properties_binding}}
    );

    ge1::unique_program player_program = ge1::compile_program(
        "shader/player_vertex.glsl", nullptr, nullptr, nullptr,
        "shader/player_fragment.glsl", {},
        {{"position", position_attribute}}, {},
        {
            {"view_properties", view_properties_binding},
            {"player_properties", player_properties_binding}
        }
    );

    mesh ground = load_mesh(
        "models/Plane_vertices.vbo", "models/Plane_faces.vbo"
    );
    mesh player = load_mesh(
        "models/Cube_vertices.vbo", "models/Cube_faces.vbo"
    );

    draw_elements_call ground_call{
        ground.vertex_array.get_name(), ground.size,
        ground_program.get_name(), GL_TRIANGLES, GL_UNSIGNED_INT
    };
    draw_elements_call player_call{
        player.vertex_array.get_name(), player.size,
        player_program.get_name(), GL_TRIANGLES, GL_UNSIGNED_INT
    };


    ge1::pass objects_pass;
    objects_pass.renderables.push_back(ground_call);
    objects_pass.renderables.push_back(player_call);

    ge1::composition composition;
    composition.passes.push_back(objects_pass);


    // TODO: ugly
    physic_mesh physic_ground;
    auto vertices = read_file<float>("models/Plane_vertices.vbo");
    auto vertex_count = vertices.size() / 8;
    auto* buffer = new vec3[vertex_count];
    physic_ground.vertices = {buffer, buffer + vertex_count};
    for (auto i = 0u; i < vertex_count; i++) {
        buffer[i] = {
            vertices.begin()[i * 8],
            vertices.begin()[i * 8 + 1],
            vertices.begin()[i * 8 + 2]
        };
    }
    delete[] vertices.begin();
    physic_ground.faces = read_file<unsigned>(
        "models/Plane_faces.vbo"
    );



    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size_callback(window, width, height);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowSizeCallback(window, &window_size_callback);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetKeyCallback(window, &key_callback);

    glClearColor(255, 255, 255, 255);


    while (!glfwWindowShouldClose(window)) {
        int joystick_axes_count, joystick_button_count;
        const float* joystick_axes =
            glfwGetJoystickAxes(GLFW_JOYSTICK_1, &joystick_axes_count);
        const unsigned char* joystick_buttons =
            glfwGetJoystickButtons(GLFW_JOYSTICK_1, &joystick_button_count);

        auto& player = physic_players[0];

        vec2 motion{};

        if (joystick_axes_count >= 4 && joystick_axes != nullptr) {
            motion = vec2(joystick_axes[0], joystick_axes[1]);

            player.head_yaw += joystick_axes[2] * 0.05f;
            player.head_pitch += joystick_axes[3] * -0.05f;
        }
        if (joystick_button_count >= 4 && joystick_buttons != nullptr) {
            if (joystick_buttons[0] == GLFW_PRESS) {
                if (player.on_ground) {
                    player.on_ground = false;
                    player.velocity.z += player.jump_force;
                }
            }
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            motion.y += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            motion.y -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            motion.x += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            motion.x -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (player.on_ground) {
                player.on_ground = false;
                player.velocity.z += player.jump_force;
            }
        }
        motion *= 1.f / std::max(1.f, length(motion));
        motion = mat2(
            cos(player.head_yaw), -sin(player.head_yaw),
            sin(player.head_yaw), cos(player.head_yaw)
        ) * motion;
        if (player.animation == player.none) {
            player.position += vec3(motion, 0) * 0.3f;
        }
        player.velocity.z -= gravity;
        player.velocity *= 0.95f;
        player.position += player.velocity;

        if (player.animation == player.none) {
            player.aim_yaw = player.head_yaw;
            player.aim_pitch = player.head_pitch;
        } else {
            player.animation_time++;
            // TODO: read animation length form ability
            if (player.animation_time == 60) {
                player.animation = player.none;
            }
        }

        if (player.position.z < 1) {
            player.position.z = 1;
            player.velocity.z = 0;
            player.on_ground = true;
        }


        auto model = translate(
            mat4(1), player.position
        );
        players->model = rotate(
            model, -player.aim_yaw, {0, 0, 1}
        );

        view_matrix = rotate(
            mat4(1), physic_players[0].head_yaw, {0, 0, 1}
        );
        view_matrix = rotate(
            view_matrix, physic_players[0].head_pitch,
            {transpose(view_matrix) * vec4(1, 0, 0, 0)}
        );

        auto camera_hit = physic_ground.ray(
            physic_players[0].position,
            physic_players[0].position + vec3{vec4{0, 0, 10, 0} * view_matrix}
        );

        view_matrix = translate(
            view_matrix, -camera_hit.contact_point
        );

        view_properties->view_projection = projection_matrix * view_matrix;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
        composition.render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    return 0;
}
