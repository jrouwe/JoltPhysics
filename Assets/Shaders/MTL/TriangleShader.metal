#include <metal_stdlib>

using namespace metal;

typedef struct
{
	float2 position;
	float4 color;
} Vertex;

struct RasterizerData
{
    float4 position [[position]];
    float4 color;
};

vertex RasterizerData vertexShader(uint vertexID [[vertex_id]], constant Vertex *vertices [[buffer(0)]])
{
    RasterizerData out;
    out.position = float4(0.0, 0.0, 0.0, 1.0);
    out.position.xy = vertices[vertexID].position.xy;
    out.color = vertices[vertexID].color;
    return out;
}

fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    return in.color;
}
