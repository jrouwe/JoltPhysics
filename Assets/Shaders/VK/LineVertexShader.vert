#version 450

#include "VertexConstants.h"

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec4 iColor;

layout(location = 0) out vec4 oColor;

void main() 
{
    gl_Position = c.Projection * c.View * vec4(iPosition, 1.0);
    oColor = iColor;
}
