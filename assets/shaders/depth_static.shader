#vertex

#version 330 core
layout(location = 0) in vec3 v_position;

uniform mat4 u_light_view_projection;
uniform mat4 u_model;

void main()
{
    gl_Position = u_light_view_projection * u_model * vec4(v_position, 1.0f);
}

#fragment

#version 330 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}