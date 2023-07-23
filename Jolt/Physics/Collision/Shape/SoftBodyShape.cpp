// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/SoftBodyShape.h>
#include <Jolt/Physics/SoftBody/SoftBody.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

#ifdef JPH_DEBUG_RENDERER

void SoftBodyShape::Draw(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const
{
	Array<SoftBody::Vertex> &vertices = mSoftBody->mVertices;
	for (const SoftBody::Face &f : mSoftBody->mSettings->mFaces)
	{
		RVec3 x1 = inCenterOfMassTransform * vertices[f.mVertex[0]].mPosition;
		RVec3 x2 = inCenterOfMassTransform * vertices[f.mVertex[1]].mPosition;
		RVec3 x3 = inCenterOfMassTransform * vertices[f.mVertex[2]].mPosition;

		inRenderer->DrawTriangle(x1, x2, x3, Color::sOrange, DebugRenderer::ECastShadow::On);
	}
}

#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_END
