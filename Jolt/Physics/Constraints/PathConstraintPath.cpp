// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Constraints/PathConstraintPath.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Core/Factory.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT(PathConstraintPath)
{
	JPH_ADD_BASE_CLASS(PathConstraintPath, SerializableObject)
}

#ifdef JPH_DEBUG_RENDERER
// Helper function to transform the results of GetPointOnPath to world space
static inline void sTransformPathPoint(RMat44Arg inTransform, Vec3Arg inPosition, RVec3 &outPosition, Vec3 &ioNormal, Vec3 &ioBinormal)
{
	outPosition = inTransform * inPosition;
	ioNormal = inTransform.Multiply3x3(ioNormal);
	ioBinormal = inTransform.Multiply3x3(ioBinormal);
}

// Helper function to draw a path segment
static inline void sDrawPathSegment(DebugRenderer *inRenderer, RVec3Arg inPrevPosition, RVec3Arg inPosition, Vec3Arg inNormal, Vec3Arg inBinormal)
{
	inRenderer->DrawLine(inPrevPosition, inPosition, Color::sWhite);
	inRenderer->DrawArrow(inPosition, inPosition + 0.1f * inNormal, Color::sRed, 0.02f);
	inRenderer->DrawArrow(inPosition, inPosition + 0.1f * inBinormal, Color::sGreen, 0.02f);
}

void PathConstraintPath::DrawPath(DebugRenderer *inRenderer, RMat44Arg inBaseTransform) const
{
	// Calculate first point
	Vec3 lfirst_pos, first_tangent, first_normal, first_binormal;
	GetPointOnPath(0.0f, lfirst_pos, first_tangent, first_normal, first_binormal);
	RVec3 first_pos;
	sTransformPathPoint(inBaseTransform, lfirst_pos, first_pos, first_normal, first_binormal);

	float t_max = GetPathMaxFraction();

	// Draw the segments
	RVec3 prev_pos = first_pos;
	for (float t = 0.1f; t < t_max; t += 0.1f)
	{
		Vec3 lpos, tangent, normal, binormal;
		GetPointOnPath(t, lpos, tangent, normal, binormal);
		RVec3 pos;
		sTransformPathPoint(inBaseTransform, lpos, pos, normal, binormal);
		sDrawPathSegment(inRenderer, prev_pos, pos, normal, binormal);
		prev_pos = pos;
	}

	// Draw last point
	Vec3 lpos, tangent, normal, binormal;
	GetPointOnPath(t_max, lpos, tangent, normal, binormal);
	RVec3 pos;
	sTransformPathPoint(inBaseTransform, lpos, pos, normal, binormal);
	sDrawPathSegment(inRenderer, prev_pos, pos, normal, binormal);
}
#endif // JPH_DEBUG_RENDERER

void PathConstraintPath::SaveBinaryState(StreamOut &inStream) const
{ 
	inStream.Write(GetRTTI()->GetHash());
	inStream.Write(mIsLooping);
}

void PathConstraintPath::RestoreBinaryState(StreamIn &inStream)
{
	// Type hash read by sRestoreFromBinaryState
	inStream.Read(mIsLooping);
}

PathConstraintPath::PathResult PathConstraintPath::sRestoreFromBinaryState(StreamIn &inStream)
{
	PathResult result;

	// Read the type of the constraint
	uint32 hash;
	inStream.Read(hash);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read type id");
		return result;
	}

	// Get the RTTI for the shape
	const RTTI *rtti = Factory::sInstance->Find(hash);
	if (rtti == nullptr)
	{
		result.SetError("Failed to resolve type. Type not registered in factory?");
		return result;
	}

	// Construct and read the data of the shape
	Ref<PathConstraintPath> path = reinterpret_cast<PathConstraintPath *>(rtti->CreateObject());
	path->RestoreBinaryState(inStream);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to restore constraint");
		return result;
	}

	result.Set(path);
	return result;
}

JPH_NAMESPACE_END
