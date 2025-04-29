#vertex

#version 330 core
layout (location = 0) in vec3 v_position;

out vec3 TexCoords;

uniform mat4 u_view;
uniform mat4 u_camera_projection;

void main()
{
    TexCoords = v_position;
    mat4 view = mat4(mat3(u_view));
    vec4 pos = u_camera_projection * view * vec4(v_position, 1.0);
    gl_Position = pos.xyww;
}  

#fragment

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_skybox;

void main()
{    
    FragColor = texture(u_skybox, TexCoords);

    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
}