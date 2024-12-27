#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 iColor;
layout(location = 1) in vec2 iTex;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = iColor * texture(texSampler, iTex);
}
