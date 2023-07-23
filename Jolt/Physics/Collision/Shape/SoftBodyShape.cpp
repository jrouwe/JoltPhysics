// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/SoftBodyShape.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Geometry/RayTriangle.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/SoftBody/SoftBody.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

uint SoftBodyShape::GetSubShapeIDBits() const
{
	// Ensure we have enough bits to encode our shape [0, n - 1]
	uint32 n = (uint32)mSoftBody->mSettings->mFaces.size() - 1;
	return 32 - CountLeadingZeros(n);
}

bool SoftBodyShape::CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const
{
	JPH_PROFILE_FUNCTION();

	uint num_triangle_bits = GetSubShapeIDBits();
	uint triangle_idx = uint(-1);

	Array<SoftBody::Vertex> &vertices = mSoftBody->mVertices;
	for (const SoftBody::Face &f : mSoftBody->mSettings->mFaces)
	{
		Vec3 x1 = vertices[f.mVertex[0]].mPosition;
		Vec3 x2 = vertices[f.mVertex[1]].mPosition;
		Vec3 x3 = vertices[f.mVertex[2]].mPosition;

		float fraction = RayTriangle(inRay.mOrigin, inRay.mDirection, x1, x2, x3);
		if (fraction < ioHit.mFraction)
		{
			// Store fraction
			ioHit.mFraction = fraction;

			// Store triangle index
			triangle_idx = uint(&f - mSoftBody->mSettings->mFaces.data());
		}
	}

	if (triangle_idx == uint(-1))
		return false;

	ioHit.mSubShapeID2 = inSubShapeIDCreator.PushID(triangle_idx, num_triangle_bits).GetID();
	return true;
}

void SoftBodyShape::CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector, const ShapeFilter &inShapeFilter) const
{
	JPH_PROFILE_FUNCTION();

	// Test shape filter
	if (!inShapeFilter.ShouldCollide(this, inSubShapeIDCreator.GetID()))
		return;

	uint num_triangle_bits = GetSubShapeIDBits();

	Array<SoftBody::Vertex> &vertices = mSoftBody->mVertices;
	for (const SoftBody::Face &f : mSoftBody->mSettings->mFaces)
	{
		Vec3 x1 = vertices[f.mVertex[0]].mPosition;
		Vec3 x2 = vertices[f.mVertex[1]].mPosition;
		Vec3 x3 = vertices[f.mVertex[2]].mPosition;

		// Back facing check
		if (inRayCastSettings.mBackFaceMode == EBackFaceMode::IgnoreBackFaces && (x2 - x1).Cross(x3 - x1).Dot(inRay.mDirection) > 0.0f)
			return;

		// Test ray against triangle
		float fraction = RayTriangle(inRay.mOrigin, inRay.mDirection, x1, x2, x3);
		if (fraction < ioCollector.GetEarlyOutFraction())
		{
			// Better hit than the current hit
			RayCastResult hit;
			hit.mBodyID = TransformedShape::sGetBodyID(ioCollector.GetContext());
			hit.mFraction = fraction;
			hit.mSubShapeID2 = inSubShapeIDCreator.PushID(uint(&f - mSoftBody->mSettings->mFaces.data()), num_triangle_bits).GetID();
			ioCollector.AddHit(hit);
		}
	}
}

const PhysicsMaterial *SoftBodyShape::GetMaterial(const SubShapeID &inSubShapeID) const
{
	SubShapeID remainder;
	uint triangle_idx = inSubShapeID.PopID(GetSubShapeIDBits(), remainder);
	JPH_ASSERT(remainder.IsEmpty());

	const SoftBody::Face &f = mSoftBody->mSettings->mFaces[triangle_idx];
	return mSoftBody->mSettings->mMaterials[f.mMaterialIndex];
}

Vec3 SoftBodyShape::GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const
{
	SubShapeID remainder;
	uint triangle_idx = inSubShapeID.PopID(GetSubShapeIDBits(), remainder);
	JPH_ASSERT(remainder.IsEmpty());

	const SoftBody::Face &f = mSoftBody->mSettings->mFaces[triangle_idx];
	Array<SoftBody::Vertex> &vertices = mSoftBody->mVertices;

	Vec3 x1 = vertices[f.mVertex[0]].mPosition;
	Vec3 x2 = vertices[f.mVertex[1]].mPosition;
	Vec3 x3 = vertices[f.mVertex[2]].mPosition;

	return (x2 - x1).Cross(x3 - x1).NormalizedOr(Vec3::sAxisY());
}

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
