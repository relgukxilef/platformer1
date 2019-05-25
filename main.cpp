#include <iostream>
#include <cmath>
#include <fstream>
#include <iterator>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ge1/composition.h>
#include <ge1/vertex_array.h>
#include <ge1/program.h>

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

template<class T>
GLuint create_static_buffer(ge1::span<T> data) {
    GLuint name;
    glGenBuffers(1, &name);

    glBindBuffer(GL_COPY_WRITE_BUFFER, name);
    glBufferData(
        GL_COPY_WRITE_BUFFER, data.size() * sizeof(T), data.begin(),
        GL_STATIC_DRAW
    );

    return name;
}

template<class T>
GLuint create_mapped_buffer(T *&data) {
    GLuint name;
    glGenBuffers(1, &name);

    glBindBuffer(GL_COPY_WRITE_BUFFER, name);
    glBufferStorage(
        GL_COPY_WRITE_BUFFER, sizeof(T), nullptr,
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    );
    data = reinterpret_cast<T*>(glMapBufferRange(
        GL_COPY_WRITE_BUFFER, 0, sizeof(T),
        GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT
    ));

    return name;
}

GLuint load_buffer_from_file(const char* filename, GLint& size) {
    ifstream file(filename, ios::in | ios::binary);
    file.unsetf(ios::skipws);

    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open "s + filename);
    }

    istream_iterator<unsigned char> start(file), end;
    vector<unsigned char> data(start, end);
    file.close();

    size = static_cast<GLint>(data.size());

    return create_static_buffer<unsigned char>(
        {data.data(), data.data() + data.size()}
    );
}

GLuint load_buffer_from_file(const char* filename) {
    GLint size;
    return load_buffer_from_file(filename, size);
}

struct mesh {
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

static struct {
    float yaw = 0, pitch = 0;
    vec3 position = {0, 0, 1};
} physic_players[2];

static mat4 view_matrix, projection_matrix;

void window_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);

    projection_matrix = perspective(
        radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.0f
    );

    view_properties->view_projection = projection_matrix * view_matrix;
}

void cursor_position_callback(GLFWwindow*, double x, double y) {
    physic_players[0].yaw = static_cast<float>(x * 0.005);
    physic_players[0].pitch = static_cast<float>(y * 0.005);
}

void mouse_button_callback(
    GLFWwindow* window, int button, int action, int modifiers
) {

}

void key_callback(
    GLFWwindow*, int key, int scancode, int action, int mods
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

    ge1::unique_buffer view_properties_buffer = create_mapped_buffer(
        view_properties
    );

    // TODO: support arrays
    ge1::unique_buffer player_properties_buffer = create_mapped_buffer(
        players
    );

    glBindBufferRange(
        GL_UNIFORM_BUFFER, view_properties_binding,
        view_properties_buffer.get_name(),
        0, sizeof(*view_properties)
    );

    glBindBufferRange(
        GL_UNIFORM_BUFFER, player_properties_binding,
        player_properties_buffer.get_name(),
        0, sizeof(*players)
    );

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

    GLint size;
    auto faces = load_buffer_from_file("models/Plane_faces.vbo", size);
    mesh ground{
        load_buffer_from_file("models/Plane_vertices.vbo"),
        faces,
        static_cast<GLint>(size / sizeof(unsigned))
    };
    faces = load_buffer_from_file("models/Cube_faces.vbo", size);
    mesh player{
        load_buffer_from_file("models/Cube_vertices.vbo"),
        faces,
        static_cast<GLint>(size / sizeof(unsigned))
    };

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


    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size_callback(window, width, height);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowSizeCallback(window, &window_size_callback);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetKeyCallback(window, &key_callback);


    while (!glfwWindowShouldClose(window)) {
        vec2 motion{};
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
        motion *= 1.f / std::max(1.f, length(motion));
        motion = mat2(
            cos(physic_players[0].yaw), -sin(physic_players[0].yaw),
            sin(physic_players[0].yaw), cos(physic_players[0].yaw)
        ) * motion;
        physic_players->position += vec3(motion, 0) * 0.3f;
        auto model = translate(
            mat4(1), physic_players->position
        );
        players->model = rotate(
            model, -physic_players->yaw, {0, 0, 1}
        );

        view_matrix = translate(
            mat4(1), {0, 0, -5}
        );

        view_matrix = rotate(
            view_matrix, physic_players[0].yaw, {0, 0, 1}
        );
        view_matrix = rotate(
            view_matrix, physic_players[0].pitch,
            {transpose(view_matrix) * vec4(1, 0, 0, 0)}
        );

        view_matrix = translate(
            view_matrix, -physic_players[0].position
        );

        view_properties->view_projection = projection_matrix * view_matrix;

        glClearColor(255, 255, 255, 255);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        composition.render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    return 0;
}
