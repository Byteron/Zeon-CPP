#vertex

#version 330 core
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 7) in ivec4 v_joints;
layout(location = 8) in vec4 v_weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 u_light_view_projection;
uniform mat4 u_model;

uniform mat4 u_joint_matrices[MAX_BONES];

void main()
{
    vec4 total_position = vec4(0.0f);

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(v_joints[i] == -1) 
            continue;
        
        if(v_joints[i] >= MAX_BONES) 
        {
            total_position = vec4(v_position, 1.0f);
            break;
        }
        vec4 local_position = u_joint_matrices[v_joints[i]] * vec4(v_position, 1.0f);
        total_position += local_position * v_weights[i];
        vec3 local_normal = mat3(u_joint_matrices[v_joints[i]]) * v_normal;
   }

    gl_Position = u_light_view_projection * u_model * total_position;
}

#fragment

#version 330 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}