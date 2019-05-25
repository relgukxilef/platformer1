#version 140

in vec3 position;
in vec3 normal;

out vec3 vertex_color;

uniform view_properties {
    mat4 view_projection;
};

void main(void) {
    gl_Position = view_projection * vec4(position, 1.0);
    vertex_color = vec3(0.5 + 0.5 * dot(
        normal, normalize(vec3(0.25, 0.5, 1))
    ));
}
