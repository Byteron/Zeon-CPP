#vertex

#version 330 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_uv;
layout (location = 2) in vec4 v_color;

out vec4 COLOR;
out vec2 UV;

uniform mat4 u_projection;

void main()
{
    COLOR = v_color;
    UV = v_uv;

    gl_Position = u_projection * vec4(v_position, 1.0);
}

#fragment

#version 330 core
out vec4 FragColor;

in vec4 COLOR;
in vec2 UV;

uniform sampler2D u_glyph_texture;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_glyph_texture, UV).r);
    FragColor = COLOR * sampled;
}  