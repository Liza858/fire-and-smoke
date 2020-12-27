#version 330 core

layout (location = 0) in vec2 tex_coords;

out vec2 texture_coords;
out vec3 col;


uniform float particle_scale;
uniform float particle_size;
uniform vec3 position;
uniform vec3 color;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 posView = view * model * vec4(position, 1);
    posView.xy += tex_coords * particle_size * particle_scale;
    texture_coords = vec2((tex_coords.x + 1.0) / 2.0, (tex_coords.y + 1.0) / 2.0);
    col = color;

    gl_Position =  projection * posView;
}