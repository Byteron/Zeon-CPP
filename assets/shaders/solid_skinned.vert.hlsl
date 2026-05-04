// Default authoring: HLSL. Cross-compile: SPIR-V (Windows SDL GPU) / MSL (macOS Metal).
// Descriptor layout matches SDL GPU SPIR-V convention (see SDL_gpu.h).

cbuffer UBO : register(b0, space1) {
    column_major float4x4 projection;
    column_major float4x4 model;
};

struct VSInput {
    float3 position : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float4 uv : TEXCOORD2;
    float4 color : TEXCOORD3;
    float3 tangent : TEXCOORD4;
    float3 bitangent : TEXCOORD5;
    float3 _pad0 : TEXCOORD6;
    float4 _pad1 : TEXCOORD7;
    uint4 joints : TEXCOORD8;
};

struct VSOutput {
    float4 pos : SV_Position;
    float4 color : TEXCOORD0;
    float4 uv : TEXCOORD1;
};

VSOutput main(VSInput input) {
    VSOutput o;
    o.pos = mul(projection, mul(model, float4(input.position, 1.0f)));
    o.color = input.color;
    o.uv = input.uv;
    return o;
}
