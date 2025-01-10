#include <metal_stdlib>

using namespace metal;

struct RasterizerData
{
    float4 position [[position]];
    float4 color;
};

vertex RasterizerData LineVertexShader(uint vertexID [[vertex_id]])
{
    RasterizerData out;
    out.position = float4(0.0, 0.0, 0.0, 1.0);
    out.color = float4(1.0, 1.0, 1.0, 1.0);
    return out;
}

fragment float4 LinePixelShader(RasterizerData in [[stage_in]])
{
	discard_fragment();
    return in.color;
}
