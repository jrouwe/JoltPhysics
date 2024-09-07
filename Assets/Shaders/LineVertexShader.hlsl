#include "VertexConstants.h"

struct VS_INPUT
{
	float3 vPos	  : POSITION;
	float3 vColor : COLOR;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color	: COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT Output;

	float4 pos = float4(input.vPos, 1.0f);

	// Transform the position from object space to homogeneous projection space
	pos = mul(View, pos);
	pos = mul(Projection, pos);
	Output.Position = pos;

	// Output color
	Output.Color = float4(input.vColor, 1.0f);

	return Output;
}