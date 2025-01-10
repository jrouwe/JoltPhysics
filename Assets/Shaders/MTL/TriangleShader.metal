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

vertex RasterizerData TriangleVertexShader(uint vertexID [[vertex_id]])
{
	RasterizerData out;
	out.position = float4(0.0, 0.0, 0.0, 1.0);
	out.color = float4(1.0, 1.0, 1.0, 1.0);
	return out;
}

fragment float4 TrianglePixelShader(RasterizerData in [[stage_in]])
{
	discard_fragment();
	return in.color;
}

vertex RasterizerData TriangleDepthVertexShader(uint vertexID [[vertex_id]])
{
	RasterizerData out;
	out.position = float4(0.0, 0.0, 0.0, 1.0);
	out.color = float4(1.0, 1.0, 1.0, 1.0);
	return out;
}

fragment float4 TriangleDepthPixelShader(RasterizerData in [[stage_in]])
{
	discard_fragment();
	return in.color;
}
