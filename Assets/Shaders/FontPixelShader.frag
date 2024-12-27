#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 iTex;
layout(location = 1) in vec4 iColor;

layout(location = 0) out vec4 oColor;

void main()
{
	float t = texture(texSampler, iTex).x;
	if (t < 0.5)
		discard;
	
	oColor = vec4(iColor.xyz, t);
}
