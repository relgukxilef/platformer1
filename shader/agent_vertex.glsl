#version 140

in vec3 position;
in vec3 normal;

in vec4 model_transposed_0;
in vec4 model_transposed_1;
in vec4 model_transposed_2;

//out vec3 vertex_color;
out vec3 world_normal;

uniform view_properties {
    mat4 view_projection;
};

void main(void) {
    mat3x4 model_transposed = mat3x4(
        model_transposed_0, model_transposed_1, model_transposed_2
    );

    gl_Position =
        view_projection * vec4(vec4(position, 1.0) * model_transposed, 1.0);
    world_normal = normalize(normal) * mat3(model_transposed);
}
