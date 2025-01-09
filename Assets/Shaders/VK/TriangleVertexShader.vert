#version 450

#include "VertexConstants.h"

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;
layout(location = 3) in vec4 vCol;

layout(location = 4) in mat4 iModel;
layout(location = 8) in mat4 iModelInvTrans;
layout(location = 12) in vec4 iCol;

layout(location = 0) out vec3 oNormal;
layout(location = 1) out vec3 oWorldPos;
layout(location = 2) out vec2 oTex;
layout(location = 3) out vec4 oPositionL;
layout(location = 4) out vec4 oColor;

void main() 
{
	// Get world position
	vec4 pos = vec4(vPos, 1.0f);
	vec4 world_pos = iModel * pos;

	// Transform the position from world space to homogeneous projection space
	vec4 proj_pos = c.View * world_pos;
	proj_pos = c.Projection * proj_pos;
	gl_Position = proj_pos;

	// Transform the position from world space to projection space of the light
	vec4 proj_lpos = c.LightView * world_pos;
	proj_lpos = c.LightProjection * proj_lpos;
	oPositionL = proj_lpos;

	// output normal
	vec4 norm = vec4(vNorm, 0.0f);
	oNormal = normalize(iModelInvTrans * norm).xyz;

	// output world position of the vertex
	oWorldPos = world_pos.xyz;

	// output texture coordinates
	oTex = vTex;

	// output color
	oColor = vCol * iCol;
}
