// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Collision/BackFaceMode.h>
#include <Jolt/Physics/Body/BodyID.h>

JPH_NAMESPACE_BEGIN

struct RayCast;
class CollideShapeSettings;
class RayCastResult;

/// Temporary data structure that contains a shape and a transform.
/// This structure can be obtained from a body (e.g. after a broad phase query) under lock protection.
/// The lock can then be released and collision detection operations can be safely performed since
/// the class takes a reference on the shape and does not use anything from the body anymore.
class TransformedShape
{
public:
	/// Constructor
								TransformedShape() = default;
								TransformedShape(Vec3Arg inPositionCOM, QuatArg inRotation, const Shape *inShape, const BodyID &inBodyID, const SubShapeIDCreator &inSubShapeIDCreator = SubShapeIDCreator()) : mShapePositionCOM(inPositionCOM), mShapeRotation(inRotation), mShape(inShape), mBodyID(inBodyID), mSubShapeIDCreator(inSubShapeIDCreator) { }

	/// Cast a ray and find the closest hit. Returns true if it finds a hit. Hits further than ioHit.mFraction will not be considered and in this case ioHit will remain unmodified (and the function will return false).
	/// Convex objects will be treated as solid (meaning if the ray starts inside, you'll get a hit fraction of 0) and back face hits are returned.
	/// If you want the surface normal of the hit use GetWorldSpaceSurfaceNormal(ioHit.mSubShapeID2, inRay.GetPointOnRay(ioHit.mFraction)) on this object.
	bool						CastRay(const RayCast &inRay, RayCastResult &ioHit) const;

	/// Cast a ray, allows collecting multiple hits. Note that this version is more flexible but also slightly slower than the CastRay function that returns only a single hit.
	/// If you want the surface normal of the hit use GetWorldSpaceSurfaceNormal(collected sub shape ID, inRay.GetPointOnRay(collected fraction)) on this object.
	void						CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, CastRayCollector &ioCollector) const;

	/// Check if inPoint is inside any shapes. For this tests all shapes are treated as if they were solid. 
	/// For a mesh shape, this test will only provide sensible information if the mesh is a closed manifold.
	/// For each shape that collides, ioCollector will receive a hit
	void						CollidePoint(Vec3Arg inPoint, CollidePointCollector &ioCollector) const;

	/// Collide a shape and report any hits to ioCollector
	void						CollideShape(const Shape *inShape, Vec3Arg inShapeScale, Mat44Arg inCenterOfMassTransform, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector) const;

	/// Cast a shape and report any hits to ioCollector
	void						CastShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, CastShapeCollector &ioCollector, const ShapeFilter &inShapeFilter = { }) const;

	/// Collect the leaf transformed shapes of all leaf shapes of this shape
	/// inBox is the world space axis aligned box which leaf shapes should collide with
	void						CollectTransformedShapes(const AABox &inBox, TransformedShapeCollector &ioCollector) const;

	/// Use the context from Shape
	using GetTrianglesContext = Shape::GetTrianglesContext;

	/// To start iterating over triangles, call this function first. 
	/// ioContext is a temporary buffer and should remain untouched until the last call to GetTrianglesNext.
	/// inBox is the world space bounding in which you want to get the triangles.
	/// To get the actual triangles call GetTrianglesNext.
	void						GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox) const;

	/// Call this repeatedly to get all triangles in the box.
	/// outTriangleVertices should be large enough to hold 3 * inMaxTriangleRequested entries
	/// outMaterials (if it is not null) should contain inMaxTrianglesRequested entries
	/// The function returns the amount of triangles that it found (which will be <= inMaxTrianglesRequested), or 0 if there are no more triangles.
	/// Note that the function can return a value < inMaxTrianglesRequested and still have more triangles to process (triangles can be returned in blocks)
	/// Note that the function may return triangles outside of the requested box, only coarse culling is performed on the returned triangles
	int							GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const;

	/// Get/set the scale of the shape as a Vec3
	inline Vec3					GetShapeScale() const						{ return Vec3::sLoadFloat3Unsafe(mShapeScale); }
	inline void					SetShapeScale(Vec3Arg inScale)				{ inScale.StoreFloat3(&mShapeScale); }

	/// Calculates the transform for this shapes's center of mass (excluding scale)
	inline Mat44				GetCenterOfMassTransform() const			{ return Mat44::sRotationTranslation(mShapeRotation, mShapePositionCOM); }

	/// Calculates the inverse of the transform for this shape's center of mass (excluding scale)
	inline Mat44				GetInverseCenterOfMassTransform() const		{ return Mat44::sInverseRotationTranslation(mShapeRotation, mShapePositionCOM); }

	/// Sets the world transform (including scale) of this transformed shape (not from the center of mass but in the space the shape was created)
	inline void					SetWorldTransform(Vec3Arg inPosition, QuatArg inRotation, Vec3Arg inScale)
	{
		mShapePositionCOM = inPosition + inRotation * (inScale * mShape->GetCenterOfMass());
		mShapeRotation = inRotation;
		SetShapeScale(inScale);
	}

	/// Sets the world transform (including scale) of this transformed shape (not from the center of mass but in the space the shape was created)
	inline void					SetWorldTransform(Mat44Arg inTransform)
	{
		Vec3 scale;
		Mat44 rot_trans = inTransform.Decompose(scale);
		SetWorldTransform(rot_trans.GetTranslation(), rot_trans.GetRotation().GetQuaternion(), scale);
	}

	/// Calculates the world transform including scale of this shape (not from the center of mass but in the space the shape was created)
	inline Mat44				GetWorldTransform() const					
	{	
		Mat44 transform = Mat44::sRotation(mShapeRotation) * Mat44::sScale(GetShapeScale());
		transform.SetTranslation(mShapePositionCOM - transform.Multiply3x3(mShape->GetCenterOfMass()));
		return transform;
	}

	/// Get the world space bounding box for this transformed shape
	AABox						GetWorldSpaceBounds() const					{ return mShape != nullptr? mShape->GetWorldSpaceBounds(GetCenterOfMassTransform(), GetShapeScale()) : AABox(); }

	/// Make inSubShapeID relative to mShape. When mSubShapeIDCreator is not empty, this is needed in order to get the correct path to the sub shape.
	inline SubShapeID			MakeSubShapeIDRelativeToShape(const SubShapeID &inSubShapeID) const
	{
		// Take off the sub shape ID part that comes from mSubShapeIDCreator and validate that it is the same
		SubShapeID sub_shape_id;
		uint num_bits_written = mSubShapeIDCreator.GetNumBitsWritten();
		JPH_IF_ENABLE_ASSERTS(uint32 root_id =) inSubShapeID.PopID(num_bits_written, sub_shape_id);
		JPH_ASSERT(root_id == (mSubShapeIDCreator.GetID().GetValue() & ((1 << num_bits_written) - 1)));
		return sub_shape_id;
	}

	/// Get surface normal of a particular sub shape and its world space surface position on this body.
	/// Note: When you have a CollideShapeResult or ShapeCastResult you should use -mPenetrationAxis.Normalized() as contact normal as GetWorldSpaceSurfaceNormal will only return face normals (and not vertex or edge normals).
	inline Vec3					GetWorldSpaceSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inPosition) const
	{
		Mat44 inv_com = GetInverseCenterOfMassTransform();
		Vec3 scale = GetShapeScale(); // See comment at ScaledShape::GetSurfaceNormal for the math behind the scaling of the normal
		return inv_com.Multiply3x3Transposed(mShape->GetSurfaceNormal(MakeSubShapeIDRelativeToShape(inSubShapeID), (inv_com * inPosition) / scale) / scale).Normalized();
	}

	/// Get material of a particular sub shape
	inline const PhysicsMaterial *GetMaterial(const SubShapeID &inSubShapeID) const
	{
		return mShape->GetMaterial(MakeSubShapeIDRelativeToShape(inSubShapeID));
	}

	/// Get the user data of a particular sub shape
	inline uint64				GetSubShapeUserData(const SubShapeID &inSubShapeID) const
	{
		return mShape->GetSubShapeUserData(MakeSubShapeIDRelativeToShape(inSubShapeID));
	}

	/// Get the direct child sub shape and its transform for a sub shape ID.
	/// @param inSubShapeID Sub shape ID that indicates the path to the leaf shape
	/// @param outRemainder The remainder of the sub shape ID after removing the sub shape
	/// @return Direct child sub shape and its transform, note that the body ID and sub shape ID will be invalid
	TransformedShape			GetSubShapeTransformedShape(const SubShapeID &inSubShapeID, SubShapeID &outRemainder) const
	{
		return mShape->GetSubShapeTransformedShape(inSubShapeID, mShapePositionCOM, mShapeRotation, GetShapeScale(), outRemainder);
	}

	/// Helper function to return the body id from a transformed shape. If the transformed shape is null an invalid body ID will be returned.
	inline static BodyID		sGetBodyID(const TransformedShape *inTS)	{ return inTS != nullptr? inTS->mBodyID : BodyID(); }

	Vec3						mShapePositionCOM;							///< Center of mass world position of the shape
	Quat						mShapeRotation;								///< Rotation of the shape
	RefConst<Shape>				mShape;										///< The shape itself
	Float3						mShapeScale { 1, 1, 1 };					///< Not stored as Vec3 to get a nicely packed structure
	BodyID						mBodyID;									///< Optional body ID from which this shape comes
	SubShapeIDCreator			mSubShapeIDCreator;							///< Optional sub shape ID creator for the shape (can be used when expanding compound shapes into multiple transformed shapes)
};

static_assert(sizeof(TransformedShape) == 64, "Not properly packed");
static_assert(alignof(TransformedShape) == 16, "Not properly aligned");

JPH_NAMESPACE_END
