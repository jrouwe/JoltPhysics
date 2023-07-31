// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/SoftBody/SoftBodyShape.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Geometry/RayTriangle.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

uint SoftBodyShape::GetSubShapeIDBits() const
{
	// Ensure we have enough bits to encode our shape [0, n - 1]
	uint32 n = (uint32)mSoftBodyMotionProperties->mSettings->mFaces.size() - 1;
	return 32 - CountLeadingZeros(n);
}

AABox SoftBodyShape::GetLocalBounds() const
{
	return mSoftBodyMotionProperties->mLocalBounds;
}

bool SoftBodyShape::CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const
{
	JPH_PROFILE_FUNCTION();

	uint num_triangle_bits = GetSubShapeIDBits();
	uint triangle_idx = uint(-1);

	const Array<SoftBodyMotionProperties::Vertex> &vertices = mSoftBodyMotionProperties->mVertices;
	for (const SoftBodyMotionProperties::Face &f : mSoftBodyMotionProperties->mSettings->mFaces)
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
			triangle_idx = uint(&f - mSoftBodyMotionProperties->mSettings->mFaces.data());
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

	const Array<SoftBodyMotionProperties::Vertex> &vertices = mSoftBodyMotionProperties->mVertices;
	for (const SoftBodyMotionProperties::Face &f : mSoftBodyMotionProperties->mSettings->mFaces)
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
			hit.mSubShapeID2 = inSubShapeIDCreator.PushID(uint(&f - mSoftBodyMotionProperties->mSettings->mFaces.data()), num_triangle_bits).GetID();
			ioCollector.AddHit(hit);
		}
	}
}

const PhysicsMaterial *SoftBodyShape::GetMaterial(const SubShapeID &inSubShapeID) const
{
	SubShapeID remainder;
	uint triangle_idx = inSubShapeID.PopID(GetSubShapeIDBits(), remainder);
	JPH_ASSERT(remainder.IsEmpty());

	const SoftBodyMotionProperties::Face &f = mSoftBodyMotionProperties->mSettings->mFaces[triangle_idx];
	return mSoftBodyMotionProperties->mSettings->mMaterials[f.mMaterialIndex];
}

Vec3 SoftBodyShape::GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const
{
	SubShapeID remainder;
	uint triangle_idx = inSubShapeID.PopID(GetSubShapeIDBits(), remainder);
	JPH_ASSERT(remainder.IsEmpty());

	const SoftBodyMotionProperties::Face &f = mSoftBodyMotionProperties->mSettings->mFaces[triangle_idx];
	const Array<SoftBodyMotionProperties::Vertex> &vertices = mSoftBodyMotionProperties->mVertices;

	Vec3 x1 = vertices[f.mVertex[0]].mPosition;
	Vec3 x2 = vertices[f.mVertex[1]].mPosition;
	Vec3 x3 = vertices[f.mVertex[2]].mPosition;

	return (x2 - x1).Cross(x3 - x1).NormalizedOr(Vec3::sAxisY());
}

#ifdef JPH_DEBUG_RENDERER

void SoftBodyShape::Draw(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const
{
	const Array<SoftBodyMotionProperties::Vertex> &vertices = mSoftBodyMotionProperties->mVertices;
	for (const SoftBodyMotionProperties::Face &f : mSoftBodyMotionProperties->mSettings->mFaces)
	{
		RVec3 x1 = inCenterOfMassTransform * vertices[f.mVertex[0]].mPosition;
		RVec3 x2 = inCenterOfMassTransform * vertices[f.mVertex[1]].mPosition;
		RVec3 x3 = inCenterOfMassTransform * vertices[f.mVertex[2]].mPosition;

		inRenderer->DrawTriangle(x1, x2, x3, Color::sOrange, DebugRenderer::ECastShadow::On);
	}
}

#endif // JPH_DEBUG_RENDERER

struct SoftBodyShape::SBSGetTrianglesContext
{
	Mat44		mCenterOfMassTransform;
	int			mTriangleIndex;
};

void SoftBodyShape::GetTrianglesStart(GetTrianglesContext &ioContext, [[maybe_unused]] const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const
{
	SBSGetTrianglesContext &context = reinterpret_cast<SBSGetTrianglesContext &>(ioContext);
	context.mCenterOfMassTransform = Mat44::sRotationTranslation(inRotation, inPositionCOM) * Mat44::sScale(inScale);
	context.mTriangleIndex = 0;
}

int SoftBodyShape::GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials) const
{
	SBSGetTrianglesContext &context = reinterpret_cast<SBSGetTrianglesContext &>(ioContext);

	const Array<SoftBodyMotionProperties::Face> &faces = mSoftBodyMotionProperties->mSettings->mFaces;
	const Array<SoftBodyMotionProperties::Vertex> &vertices = mSoftBodyMotionProperties->mVertices;
	const PhysicsMaterialList &materials = mSoftBodyMotionProperties->mSettings->mMaterials;

	int num_triangles = min(inMaxTrianglesRequested, (int)faces.size() - context.mTriangleIndex);
	for (int i = 0; i < num_triangles; ++i)
	{
		const SoftBodyMotionProperties::Face &f = faces[context.mTriangleIndex + i];

		Vec3 x1 = context.mCenterOfMassTransform * vertices[f.mVertex[0]].mPosition;
		Vec3 x2 = context.mCenterOfMassTransform * vertices[f.mVertex[1]].mPosition;
		Vec3 x3 = context.mCenterOfMassTransform * vertices[f.mVertex[2]].mPosition;

		x1.StoreFloat3(outTriangleVertices++);
		x2.StoreFloat3(outTriangleVertices++);
		x3.StoreFloat3(outTriangleVertices++);

		if (outMaterials != nullptr)
			*outMaterials++ = materials[f.mMaterialIndex];
	}

	context.mTriangleIndex += num_triangles;
	return num_triangles;
}

JPH_NAMESPACE_END
