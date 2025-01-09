#version 450

#include "VertexConstants.h"

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;
layout(location = 3) in vec4 vCol;

layout(location = 4) in mat4 iModel;
layout(location = 8) in mat4 iModelInvTrans;
layout(location = 12) in vec4 iCol;

void main() 
{
	// Check if the alpha = 0
	if (vCol.a * iCol.a == 0.0)
	{
		// Don't draw the triangle by moving it to an invalid location
		gl_Position = vec4(0, 0, 0, 0);
	}
	else
	{
		// Transform the position from world space to homogeneous projection space for the light
		vec4 pos = vec4(vPos, 1.0f);
		pos = iModel * pos;
		pos = c.LightView * pos;
		pos = c.LightProjection * pos;
		gl_Position = pos;
	}
}
