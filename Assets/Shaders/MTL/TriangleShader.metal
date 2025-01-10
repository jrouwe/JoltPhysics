#include <metal_stdlib>

using namespace metal;

#include "VertexConstants.h"

struct Vertex
{
	float3		vPos [[attribute(0)]];
	float3		vNorm [[attribute(1)]];
	float2		vTex [[attribute(2)]];
	uchar4		vCol [[attribute(3)]];
	float4		iModel0 [[attribute(4)]];
	float4		iModel1 [[attribute(5)]];
	float4		iModel2 [[attribute(6)]];
	float4		iModel3 [[attribute(7)]];
	float4		iModelInvTrans0 [[attribute(8)]];
	float4		iModelInvTrans1 [[attribute(9)]];
	float4		iModelInvTrans2 [[attribute(10)]];
	float4		iModelInvTrans3 [[attribute(11)]];
	uchar4		iCol [[attribute(12)]];
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

vertex Pix TriangleVertexShader(Vertex vert [[stage_in]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
	Pix out;

	// Convert input matrices
	float4x4 iModel(vert.iModel0, vert.iModel1, vert.iModel2, vert.iModel3);
	float4x4 iModelInvTrans(vert.iModelInvTrans0, vert.iModelInvTrans1, vert.iModelInvTrans2, vert.iModelInvTrans3);

	// Get world position
	float4 pos = float4(vert.vPos, 1.0f);
	float4 world_pos = iModel * pos;

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
	out.oNormal = normalize(iModelInvTrans * norm).xyz;

	// output world position of the vertex
	out.oWorldPos = world_pos.xyz;

	// output texture coordinates
	out.oTex = vert.vTex;

	// output color
	out.oColor = float4(vert.vCol) * float4(vert.iCol) / (255.0 * 255.0);

	return out;
}

fragment float4 TrianglePixelShader(Pix in [[stage_in]], constant PixelShaderConstantBuffer *constants)
{
	return in.oColor;
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
