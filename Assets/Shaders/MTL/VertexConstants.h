struct VertexShaderConstantBuffer
{
	float4x4 	View;				// view matrix
	float4x4 	Projection;		// projection matrix
	float4x4 	LightView;			// view matrix of the light
	float4x4 	LightProjection;	// projection matrix of the light
};

struct PixelShaderConstantBuffer
{
	float3		CameraPos;
	float3		LightPos;
};
