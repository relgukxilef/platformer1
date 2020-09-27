#version 140

//in vec3 vertex_color;
in vec3 world_normal;

in vec4 vertex_flash_color;

void main(void) {
    vec3 color = vec3(0.5 + 0.5 * dot(
        world_normal,
        normalize(vec3(0.25, 0.5, 1))
    ));

    color = mix(color, vertex_flash_color.xyz, vertex_flash_color.a);

    gl_FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
