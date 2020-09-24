#version 140

//in vec3 vertex_color;
in vec3 world_normal;

void main(void) {
    vec3 vertex_color = vec3(0.5 + 0.5 * dot(
         world_normal,
         normalize(vec3(0.25, 0.5, 1))
     ));
    gl_FragColor = vec4(pow(vertex_color, vec3(1.0 / 2.2)), 1.0);
}
