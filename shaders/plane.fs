#version 330 core

uniform sampler2D plane_texture;
in vec2 tex_coords;


void main()
{    
    gl_FragColor = texture(plane_texture, tex_coords);
}
