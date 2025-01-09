layout(binding = 0) uniform VertexShaderConstantBuffer
{
	mat4	View;
	mat4	Projection;
	mat4	LightView;
	mat4	LightProjection;
} c;
