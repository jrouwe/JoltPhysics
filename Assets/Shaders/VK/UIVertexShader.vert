#version 450

#include "VertexConstants.h"

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTex;
layout(location = 2) in vec4 vCol;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec2 oTex;

void main() 
{
	gl_Position = c.Projection * c.View * vec4(vPos, 1.0f);
	oTex = vTex;
	oColor = vCol;
}
