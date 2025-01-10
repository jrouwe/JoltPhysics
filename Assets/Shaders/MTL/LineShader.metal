#include <metal_stdlib>

using namespace metal;

#include "VertexConstants.h"

struct LineVertex
{
	float3	iPosition [[attribute(0)]];
	uchar4	iColor [[attribute(1)]];
};

struct LineOut
{
    float4 position [[position]];
    float4 color;
};

vertex LineOut LineVertexShader(LineVertex vert [[stage_in]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
    LineOut out;
	out.position = constants->Projection * constants->View * float4(vert.iPosition, 1.0);
    out.color = float4(vert.iColor) / 255.0;
    return out;
}

fragment float4 LinePixelShader(LineOut in [[stage_in]])
{
    return in.color;
}
