#include <metal_stdlib>

using namespace metal;

#include "VertexConstants.h"

struct LineVertex
{
	float  iPosition[3];
	uchar4 iColor;
};

struct LineOut
{
    float4 position [[position]];
    float4 color;
};

vertex LineOut LineVertexShader(uint vertexID [[vertex_id]], constant LineVertex *vertices [[buffer(0)]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
	constant LineVertex &vert = vertices[vertexID];

    LineOut out;
	out.position = constants->Projection * constants->View * float4(vert.iPosition[0], vert.iPosition[1], vert.iPosition[2], 1.0);
    out.color = float4(vert.iColor) / 255.0f;
    return out;
}

fragment float4 LinePixelShader(LineOut in [[stage_in]])
{
    return in.color;
}
