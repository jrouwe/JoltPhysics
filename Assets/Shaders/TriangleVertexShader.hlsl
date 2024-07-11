#include "VertexConstants.h"

struct VS_INPUT
{
	// Per vertex data
	float3 vPos	  : POSITION;
	float3 vNorm  : NORMAL;
	float2 vTex	  : TEXCOORD0;
	float4 vCol	  : COLOR;

	// Per instance data
	matrix iModel : INSTANCE_TRANSFORM;				// model matrix
	matrix iModelInvTrans : INSTANCE_INV_TRANSFORM;	// (model matrix^-1)^T
	float4 iCol : INSTANCE_COLOR;					// color of the model
};

struct VS_OUTPUT
{
	float4 Position	 : SV_POSITION;
	float3 Normal	 : TEXCOORD0;
	float3 WorldPos	 : TEXCOORD1;
	float2 Tex		 : TEXCOORD2;
	float4 PositionL : TEXCOORD3;
	float4 Color	 : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	// Get world position
	float4 pos = float4(input.vPos, 1.0f);
	float4 world_pos = mul(input.iModel, pos);

	// Transform the position from world space to homogeneous projection space
	float4 proj_pos = mul(View, world_pos);
	proj_pos = mul(Projection, proj_pos);
	output.Position = proj_pos;

	// Transform the position from world space to projection space of the light
	float4 proj_lpos = mul(LightView, world_pos);
	proj_lpos = mul(LightProjection, proj_lpos);
	output.PositionL = proj_lpos;

	// output normal
	float4 norm = float4(input.vNorm, 0.0f);
	output.Normal = normalize(mul(input.iModelInvTrans, norm).xyz);

	// output world position of the vertex
	output.WorldPos = world_pos.xyz;

	// output texture coordinates
	output.Tex = input.vTex;

	// output color
	output.Color = input.vCol * input.iCol;

	return output;
}