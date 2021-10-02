#include "VertexConstants.h"

struct VS_INPUT
{
	// Per vertex data
    float3 vPos   : POSITION;
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
    float4 Position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

	// Check if the alpha = 0
	if (input.vCol.a * input.iCol.a == 0.0)
	{
		// Don't draw the triangle by moving it to an invalid location
		output.Position = float4(0, 0, 0, 0);
	}
	else
	{
		// Transform the position from world space to homogeneous projection space for the light
		float4 pos = float4(input.vPos, 1.0f);
		pos = mul(input.iModel, pos);
		pos = mul(LightView, pos);
		pos = mul(LightProjection, pos);
		output.Position = pos;
	}
	
    return output;
}