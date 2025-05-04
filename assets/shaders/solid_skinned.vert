#version 450 core

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(set = 1, binding = 0) uniform UBO {
    mat4 u_view_projection;
    mat4 u_view;
    float u_near;
    float u_far;
    mat4 u_model;
    //mat4 u_light_view_projections[%1];
    mat4 u_joint_matrices[MAX_BONES];
};

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec4 v_uv;
layout(location = 3) in vec4 v_uv2;
layout(location = 4) in vec3 v_tangent;
layout(location = 5) in vec3 v_bitangent;
layout(location = 6) in vec4 v_color;
layout(location = 7) in ivec4 v_joints;
layout(location = 8) in vec4 v_weights;


layout(location = 0) out vec3 Position;
layout(location = 1) out vec4 Color;
layout(location = 2) out vec2 TexCoords;
layout(location = 3) out vec3 Normal;
//out vec4 LightPosition[%1];
layout(location = 4) out float Depth;

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

    vec4 position = u_model * total_position;

    gl_Position = u_view_projection * position;

    Position = position.xyz;

    vec4 view_position = u_view * position;
    Depth = (-view_position.z - u_near) / (u_far - u_near);
    
    Normal = mat3(transpose(inverse(u_model))) * v_normal;
    TexCoords = v_uv.xy;
    Color = v_color;

    //for (int i = 0; i < %1; i++)
    //{
    //    LightPosition[i] = u_light_view_projections[i] * position;
    //}
}