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
    float4 	oPosition [[position]];
    float4 	oColor;
};

vertex LineOut LineVertexShader(LineVertex vert [[stage_in]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
    LineOut out;
	out.oPosition = constants->Projection * constants->View * float4(vert.iPosition, 1.0);
    out.oColor = float4(vert.iColor) / 255.0;
    return out;
}

fragment float4 LinePixelShader(LineOut in [[stage_in]])
{
    return in.oColor;
}
