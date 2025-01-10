#include <metal_stdlib>

using namespace metal;

#include "VertexConstants.h"

constexpr sampler alphaTextureSampler(mag_filter::linear, min_filter::linear);

struct FontVertex
{
	float3		vPos [[attribute(0)]];
	float2		vTex [[attribute(1)]];
	uchar4		vCol [[attribute(2)]];
};

struct FontOut
{
    float4 		oPosition [[position]];
    float2 		oTex;
    float4 		oColor;
};

vertex FontOut FontVertexShader(FontVertex vert [[stage_in]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
    FontOut out;
    out.oPosition = constants->Projection * constants->View * float4(vert.vPos, 1.0);
    out.oTex = vert.vTex;
    out.oColor = float4(vert.vCol) / 255.0;
    return out;
}

fragment float4 FontPixelShader(FontOut in [[stage_in]], texture2d<float> alphaTexture [[texture(0)]])
{
	const float4 sample = alphaTexture.sample(alphaTextureSampler, in.oTex);
	if (sample.x < 0.5)
		discard_fragment();

    return float4(in.oColor.xyz, sample.x);
}
