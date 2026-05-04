// Default authoring: HLSL. Cross-compile: SPIR-V (Windows) / MSL (macOS).

Texture2D tex_sampler : register(t0, space2);
SamplerState tex_sampler_s : register(s0, space2);

struct PSInput {
    float4 color : TEXCOORD0;
    float4 uv : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target {
    return tex_sampler.Sample(tex_sampler_s, input.uv.xy) * input.color;
}
