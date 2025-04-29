
#vertex

#version 330 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;

uniform mat4 u_view_projection;

out vec4 Color;

void main()
{
	gl_Position = u_view_projection * vec4(v_position, 1);
	Color = v_color;
}

#fragment

#version 330 core

in vec4 Color;

out vec4 FragColor;

void main()
{
	FragColor = Color;
}