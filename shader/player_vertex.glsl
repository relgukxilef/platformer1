#version 140

in vec3 position;
in vec3 normal;

//out vec3 vertex_color;
out vec3 world_normal;

uniform view_properties {
    mat4 view_projection;
};

uniform player_properties {
    mat4 model;
};

void main(void) {
    gl_Position = view_projection * model * vec4(position, 1.0);
    /*vertex_color = vec3(0.5 + 0.5 * dot(
        (model * vec4(normalize(normal), 0)).xyz,
        normalize(vec3(0.25, 0.5, 1))
    ));*/
    world_normal = (model * vec4(normalize(normal), 0)).xyz;
}
