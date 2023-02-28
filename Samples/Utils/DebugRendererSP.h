// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/DebugRendererImp.h>

// This file contains debug renderer functions that take single precision arguments for tests that do not need to deal with large worlds.
// They're split off so that we don't accidentally call single precision versions.

inline void DrawLineSP(DebugRenderer *inRenderer, Vec3Arg inFrom, Vec3Arg inTo, Color inColor)
{
	inRenderer->DrawLine(RVec3(inFrom), RVec3(inTo), inColor);
}

inline void DrawMarkerSP(DebugRenderer *inRenderer, Vec3Arg inPosition, ColorArg inColor, float inSize)
{
	inRenderer->DrawMarker(RVec3(inPosition), inColor, inSize);
}

inline void DrawArrowSP(DebugRenderer *inRenderer, Vec3Arg inFrom, Vec3Arg inTo, ColorArg inColor, float inSize)
{
	inRenderer->DrawArrow(RVec3(inFrom), RVec3(inTo), inColor, inSize);
}

inline void DrawTriangleSP(DebugRenderer *inRenderer, Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor)
{
	inRenderer->DrawTriangle(RVec3(inV1), RVec3(inV2), RVec3(inV3), inColor);
}

inline void DrawWireBoxSP(DebugRenderer *inRenderer, Mat44Arg inMatrix, const AABox &inBox, ColorArg inColor)
{
	inRenderer->DrawWireBox(RMat44(inMatrix), inBox, inColor);
}

inline void DrawBoxSP(DebugRenderer *inRenderer, Mat44Arg inMatrix, const AABox &inBox, ColorArg inColor, DebugRenderer::ECastShadow inCastShadow = DebugRenderer::ECastShadow::On, DebugRenderer::EDrawMode inDrawMode = DebugRenderer::EDrawMode::Solid)
{
	inRenderer->DrawBox(RMat44(inMatrix), inBox, inColor, inCastShadow, inDrawMode);
}

inline void DrawWireSphereSP(DebugRenderer *inRenderer, Vec3Arg inCenter, float inRadius, ColorArg inColor, int inLevel = 3)
{
	inRenderer->DrawWireSphere(RVec3(inCenter), inRadius, inColor, inLevel);
}

inline void DrawSphereSP(DebugRenderer *inRenderer, Vec3Arg inCenter, float inRadius, ColorArg inColor, DebugRenderer::ECastShadow inCastShadow = DebugRenderer::ECastShadow::On, DebugRenderer::EDrawMode inDrawMode = DebugRenderer::EDrawMode::Solid)
{
	inRenderer->DrawSphere(RVec3(inCenter), inRadius, inColor, inCastShadow, inDrawMode);
}

inline void DrawText3DSP(DebugRenderer *inRenderer, Vec3Arg inPosition, const string_view &inString, ColorArg inColor = Color::sWhite, float inHeight = 0.5f)
{
	inRenderer->DrawText3D(RVec3(inPosition), inString, inColor, inHeight);
}
