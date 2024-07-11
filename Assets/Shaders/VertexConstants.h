cbuffer VertexShaderConstantBuffer : register(b0)
{
	matrix View;			// view matrix
	matrix Projection;		// projection matrix
	matrix LightView;		// view matrix of the light
	matrix LightProjection;	// projection matrix of the light
};
