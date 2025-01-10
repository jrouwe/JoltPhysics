#include <metal_stdlib>

using namespace metal;

#include "VertexConstants.h"

constexpr sampler uiTextureSampler(mag_filter::linear, min_filter::linear);

struct UIVertex
{
	float3		vPos [[attribute(0)]];
	float2		vTex [[attribute(1)]];
	uchar4		vCol [[attribute(2)]];
};

struct UIOut
{
    float4 		oPosition [[position]];
    float2 		oTex;
    float4 		oColor;
};

vertex UIOut UIVertexShader(UIVertex vert [[stage_in]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
    UIOut out;
    out.oPosition = constants->Projection * constants->View * float4(vert.vPos, 1.0);
    out.oTex = vert.vTex;
    out.oColor = float4(vert.vCol) / 255.0;
    return out;
}

fragment float4 UIPixelShader(UIOut in [[stage_in]], texture2d<float> uiTexture [[texture(0)]])
{
	const float4 sample = uiTexture.sample(uiTextureSampler, in.oTex);
    return sample * in.oColor;
}

fragment float4 UIPixelShaderUntextured(UIOut in [[stage_in]])
{
    return in.oColor;
}
