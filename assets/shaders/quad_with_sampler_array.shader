#vertex

#version 330 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_uv;
layout (location = 2) in vec4 v_color;

uniform mat4 u_projection;

out vec4 Color;
out vec2 TexCoords;

void main ()
{
	gl_Position = u_projection * vec4(v_position, 1.0);
	TexCoords = v_uv;
	Color = v_color;
}

#fragment

#version 330 core

uniform sampler2DArray u_diffuse_texture;
uniform float u_layer;

in vec4 Color;
in vec2 TexCoords;

out vec4 FragColor;

void main()
{
	vec4 c = texture(u_diffuse_texture, vec3(TexCoords, u_layer));
	FragColor = Color * c;
}