#version 330 core

in vec2 texture_coords;
in vec3 col;

uniform sampler2D particle_texture;
uniform float alpha;


void main()
{    
    vec4 tex_color = texture(particle_texture, texture_coords);
    float intensivity = 2.0;
    gl_FragColor = vec4(tex_color.x * col * intensivity, tex_color.w * alpha);
}