#include <metal_stdlib>

using namespace metal;

#include "VertexConstants.h"

struct Vertex
{
	float3		vPos;
	float3		vNorm;
	float2		vTex;
	uint32_t	vCol;
};

struct Instance
{
	float4x4	iModel;
	float4x4	iModelInvTrans;
	uint32_t	iCol;
};

struct Pix
{
	float4		oPosition [[position]];
	float3		oNormal;
	float3		oWorldPos;
	float2		oTex;
	float4		oPositionL;
	float4		oColor;
};

vertex Pix TriangleVertexShader(uint vertexID [[vertex_id]], uint instanceID [[instance_id]], constant Vertex *vertices [[buffer(0)]], constant Instance *instances [[buffer(1)]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
	Pix out;

	constant Vertex &vert = vertices[vertexID];
	constant Instance &inst = instances[instanceID];

	// Get world position
	float4 pos = float4(vert.vPos, 1.0f);
	float4 world_pos = inst.iModel * pos;

	// Transform the position from world space to homogeneous projection space
	float4 proj_pos = constants->View * world_pos;
	proj_pos = constants->Projection * proj_pos;
	out.oPosition = proj_pos;

	// Transform the position from world space to projection space of the light
	float4 proj_lpos = constants->LightView * world_pos;
	proj_lpos = constants->LightProjection * proj_lpos;
	out.oPositionL = proj_lpos;

	// output normal
	float4 norm = float4(vert.vNorm, 0.0f);
	out.oNormal = normalize(inst.iModelInvTrans * norm).xyz;

	// output world position of the vertex
	out.oWorldPos = world_pos.xyz;

	// output texture coordinates
	out.oTex = vert.vTex;

	// output color
	out.oColor = vert.vCol * inst.iCol;

	return out;
}

fragment float4 TrianglePixelShader(Pix in [[stage_in]], constant PixelShaderConstantBuffer *constants)
{
	return float4(1.0, 1.0, 1.0, 1.0);
}

struct Depth
{
	float4		oPosition [[position]];
};

vertex Depth TriangleDepthVertexShader(uint vertexID [[vertex_id]])
{
	Depth out;
	out.oPosition = float4(0.0, 0.0, 0.0, 1.0);
	return out;
}

fragment float4 TriangleDepthPixelShader(Depth in [[stage_in]], constant PixelShaderConstantBuffer *constants [[buffer(0)]])
{
	discard_fragment();
	return float4(1.0, 1.0, 1.0, 1.0);
}
