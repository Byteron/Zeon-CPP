#vertex

#version 330 core
layout(location = 0) in vec3 v_position;

layout(location = 9) in mat4 i_model;

uniform mat4 u_light_view_projection;

void main()
{
    gl_Position = u_light_view_projection * (vec4(v_position, 1.0f) * i_model);
}

#fragment

#version 330 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}