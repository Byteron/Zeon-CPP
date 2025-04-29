#vertex

#version 330 core
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec4 v_uv;
layout(location = 3) in vec4 v_uv2;
layout(location = 4) in vec3 v_tangent;
layout(location = 5) in vec3 v_bitangent;
layout(location = 6) in vec4 v_color;

layout(location = 9) in mat4 i_model;

uniform mat4 u_view_projection;
uniform mat4 u_view;
uniform float u_near;
uniform float u_far;

uniform mat4 u_light_view_projections[%1];

out vec3 Position;
out vec4 Color;
out vec2 TexCoords;
out vec3 Normal;
out vec4 LightPosition[%1];
out float Depth;

void main()
{
    vec4 position = vec4(v_position, 1.0) * i_model;

    gl_Position = u_view_projection * position;

    Position = position.xyz;

    vec4 view_position = u_view * position;
    Depth = (-view_position.z - u_near) / (u_far - u_near);
    
    Normal =  v_normal * mat3(inverse(i_model));
    TexCoords = v_uv.xy;
    Color = v_color;

    for (int i = 0; i < %1; i++)
    {
        LightPosition[i] = u_light_view_projections[i] * position;
    }
}

#fragment

#version 330 core
out vec4 FragColor;

in vec3 Position;
in vec4 Color;
in vec2 TexCoords;
in vec3 Normal;
in vec4 LightPosition[%1];
in float Depth;

uniform vec3 u_light_direction;
uniform vec3 u_light_color;
uniform vec3 u_camera_position;

uniform float u_cascade_splits[%2];

uniform vec4 u_albedo;
uniform sampler2D u_diffuse_texture;

uniform sampler2DArray u_shadow_map;

const float BLEND_RANGE = 0.1; // Adjust as needed

void get_cascades_and_blend_factor(float depth, out int cascade_index0, out int cascade_index1, out float blend_factor) {
    for (int i = 0; i < %1; i++)
    {
        float split1 = u_cascade_splits[i + 1];

        if (depth < split1)
        {
            cascade_index0 = i;
            cascade_index1 = i;
            blend_factor = 0.0;
            return;
        }
    }
}

float compute_shadow_for_cascade(vec4 light_position, int cascade_index, vec3 light_direction, vec3 normal) {
    vec3 projection_coords = light_position.xyz / light_position.w;
    projection_coords = projection_coords * 0.5 + 0.5;

    float current_depth = projection_coords.z;
    float closest_depth = texture(u_shadow_map, vec3(projection_coords.xy, float(cascade_index))).r;

    float bias = max(0.05 * (1.0 - dot(normal, light_direction)), 0.005);
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_shadow_map, 0).xy;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            vec2 tex_coords = projection_coords.xy + vec2(x, y) * texel_size;
            float pcf_depth = texture(u_shadow_map, vec3(tex_coords, float(cascade_index))).r;
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if(projection_coords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float shadow_calculation(vec4 light_positions[%1], vec3 light_direction, vec3 normal) {
    int cascade_index0, cascade_index1;
    float blend_factor;
    get_cascades_and_blend_factor(Depth, cascade_index0, cascade_index1, blend_factor);

    float shadow0 = compute_shadow_for_cascade(light_positions[cascade_index0], cascade_index0, light_direction, normal);
    float shadow1 = compute_shadow_for_cascade(light_positions[cascade_index1], cascade_index1, light_direction, normal);

    float shadow = mix(shadow0, shadow1, blend_factor);
    return shadow;
}

void main() {
    vec4 tex = texture(u_diffuse_texture, TexCoords);

    // Normalize the normal
    vec3 norm = normalize(Normal);

    // For directional light, direction is opposite of the light direction
    vec3 light_direction = normalize(-u_light_direction);

    // Compute NdotL for base lighting
    float NdotL = dot(norm, light_direction);
    float shadow = shadow_calculation(LightPosition, light_direction, Normal);
    
    // Quantize the diffuse shading into 3 steps:
    // For example, 
    // Step 1 (Shadow): NdotL < 0.1
    // Step 2 (Mid-tone): 0.1 <= NdotL < 0.5
    // Step 3 (Bright): NdotL >= 0.5
    float diffuseLevel;
    if (NdotL < 0.4) {
        diffuseLevel = 0.0; // shadow step
    } else if (NdotL < 0.5) {
        diffuseLevel = 0.5; // mid-tone step
    } else {
        diffuseLevel = 1.0; // bright step
    }

    // Simple ambient term (can be small since toon shading often is bold)
    float ambient_strength = 0.4;
    vec3 ambient = ambient_strength * u_light_color;

    // Toon-style diffuse (no smooth falloff, just the quantized level)
    vec3 diffuse = diffuseLevel * u_light_color;

    // Compute specular as before
    vec3 view_direction = normalize(u_camera_position - Position);
    vec3 half_vector = normalize(light_direction + view_direction);
    float NdotH = dot(norm, half_vector);
    
    // Compute a smooth spec
    float spec = pow(max(NdotH, 0.0), 32.0);

    // Now quantize the specular to a single step highlight:
    // If spec is above a threshold, show a strong highlight, else none.
    float specular_threshold = 0.5;
    float specular_strength = 0.5; 
    vec3 specular = (spec > specular_threshold) ? (specular_strength * u_light_color) : vec3(0.0);

    // Apply shadow reduction: (1.0 - shadow) to diffuse and specular
    vec3 lighting = ambient + (1.0 - shadow)*(diffuse + specular);

    // Combine with texture and albedo
    vec3 result = lighting * tex.rgb * u_albedo.rgb;

    // Toon shading often doesn't use gamma correction, but you can keep it if desired
    // Removing gamma correction gives a more "cartoonish" flat look.
    // If you want to keep gamma correction, uncomment the next lines:
    // float gamma = 2.2;
    // result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, tex.a * u_albedo.a);
}
