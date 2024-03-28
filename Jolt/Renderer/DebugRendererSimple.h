// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifndef JPH_DEBUG_RENDERER
	#error This file should only be included when JPH_DEBUG_RENDERER is defined
#endif // !JPH_DEBUG_RENDERER

#include <Jolt/Renderer/DebugRenderer.h>

JPH_NAMESPACE_BEGIN

/// Inherit from this class to simplify implementing a debug renderer, start with this implementation:
///
///		class MyDebugRenderer : public JPH::DebugRendererSimple
///		{
///		public:
///			virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
///			{
///				// Implement
///			}
///		
///			virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::ECastShadow inCastShadow) override
///			{
///				// Implement
///			}
///		
///			virtual void DrawText3D(JPH::RVec3Arg inPosition, const string_view &inString, JPH::ColorArg inColor, float inHeight) override
///			{
///				// Implement
///			}
///		};
///
/// Note that this class is meant to be a quick start for implementing a debug renderer, it is not the most efficient way to implement a debug renderer.
class JPH_DEBUG_RENDERER_EXPORT DebugRendererSimple : public DebugRenderer
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
								DebugRendererSimple();

protected:
	/// Implementation of DebugRenderer interface
	virtual Batch				CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) override;
	virtual Batch				CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) override;
	virtual void				DrawGeometry(RMat44Arg inModelMatrix, const AABox &inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;

private:
	/// Implementation specific batch object
	class BatchImpl : public RefTargetVirtual
	{
	public:
		JPH_OVERRIDE_NEW_DELETE

		virtual void			AddRef() override			{ ++mRefCount; }
		virtual void			Release() override			{ if (--mRefCount == 0) delete this; }

		Array<Triangle>			mTriangles;

	private:
		atomic<uint32>			mRefCount = 0;
	};
};

JPH_NAMESPACE_END
