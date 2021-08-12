// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Collision/Shape/Shape.h>
#include <Physics/Collision/PhysicsMaterial.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

namespace JPH {

class ConvexShape;
class CollideShapeSettings;

/// Constants for HeightFieldShape, this was moved out of the HeightFieldShape because of a linker bug
namespace HeightFieldShapeConstants
{
	/// Value used to create gaps in the height field
	constexpr float			cNoCollisionValue = FLT_MAX;

	/// Hierarchical grids stop when cBlockSize x cBlockSize height samples remain
	constexpr uint			cBlockSize = 2;

	/// Stack size to use during WalkHeightField
	constexpr int			cStackSize = 128;

	/// A position in the hierarchical grid is defined by a level (which grid), x and y position. We encode this in a single uint32 as: level << 28 | y << 14 | x
	constexpr uint			cNumBitsXY = 14;
	constexpr uint			cMaskBitsXY = (1 << cNumBitsXY) - 1;
	constexpr uint			cLevelShift = 2 * cNumBitsXY;

	/// When height samples are converted to 16 bit:
	constexpr uint16		cNoCollisionValue16 = 0xffff;		///< This is the magic value for 'no collision'
	constexpr uint16		cMaxHeightValue16 = 0xfffe;			///< This is the maximum allowed height value

	/// When height samples are converted to 8 bit:
	constexpr uint8			cNoCollisionValue8 = 0xff;			///< This is the magic value for 'no collision'
	constexpr uint8			cMaxHeightValue8 = 0xfe;			///< This is the maximum allowed height value
};

/// Class that constructs a HeightFieldShape
class HeightFieldShapeSettings final : public ShapeSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(HeightFieldShapeSettings)

	/// Default constructor for deserialization
									HeightFieldShapeSettings() = default;

	/// Create a height field shape of inSampleCount * inSampleCount vertices.
	/// The height field is a surface defined by: inOffset + inScale * (x, inSamples[y * inSampleCount + x], y).
	/// where x and y are integers in the range x and y e [0, inSampleCount - 1].
	/// inSampleCount: Must be a power of 2 and minimally 8.
	/// inSamples: inSampleCount^2 vertices.
	/// inMaterialIndices: (inSampleCount - 1)^2 indices that index into inMaterialList.
									HeightFieldShapeSettings(const float *inSamples, Vec3Arg inOffset, Vec3Arg inScale, uint32 inSampleCount, const uint8 *inMaterialIndices = nullptr, const PhysicsMaterialList &inMaterialList = PhysicsMaterialList());

	// See: ShapeSettings
	virtual ShapeResult				Create() const override;

	/// The height field is a surface defined by: mOffset + mScale * (x, mHeightSamples[y * mSampleCount + x], y).
	/// where x and y are integers in the range x and y e [0, mSampleCount - 1].
	Vec3							mOffset = Vec3::sZero();
	Vec3							mScale = Vec3::sReplicate(1.0f);
	uint32							mSampleCount = 0;

	vector<float>					mHeightSamples;
	vector<uint8>					mMaterialIndices;

	/// The materials of square at (x, y) is: mMaterials[mMaterialIndices[x + y * (mSampleCount - 1)]]
	PhysicsMaterialList				mMaterials;
};

/// A height field shape. Cannot be used as a dynamic object.
class HeightFieldShape final : public Shape
{
public:
	/// Constructor
									HeightFieldShape() : Shape(EShapeType::HeightField, EShapeSubType::HeightField) { }
									HeightFieldShape(const HeightFieldShapeSettings &inSettings, ShapeResult &outResult);

	// See Shape::MustBeStatic
	virtual bool					MustBeStatic() const override										{ return true; }

	// See Shape::GetLocalBounds
	virtual AABox					GetLocalBounds() const override;

	// See Shape::GetSubShapeIDBitsRecursive
	virtual uint					GetSubShapeIDBitsRecursive() const override							{ return GetSubShapeIDBits(); }

	// See Shape::GetInnerRadius
	virtual float					GetInnerRadius() const override										{ return 0.0f; }

	// See Shape::GetMassProperties
	virtual MassProperties			GetMassProperties() const override;
	
	// See Shape::GetMaterial
	virtual const PhysicsMaterial *	GetMaterial(const SubShapeID &inSubShapeID) const override;

	/// Overload to get the material at a particular location
	const PhysicsMaterial *			GetMaterial(uint inX, uint inY) const;

	// See Shape::GetSurfaceNormal
	virtual Vec3					GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const override;

	// See Shape::GetSubmergedVolume
	virtual void					GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy) const override { JPH_ASSERT(false, "Not supported"); }

#ifdef JPH_DEBUG_RENDERER
	// See Shape::Draw
	virtual void					Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override;
#endif // JPH_DEBUG_RENDERER

	// See Shape::CastRay
	virtual bool					CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;
	virtual void					CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) const override;

	// See: Shape::CollidePoint
	virtual void					CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector) const override;

	// See Shape::CastShape
	virtual void					CastShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector) const override;

	// See Shape::GetTrianglesStart
	virtual void					GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const override;

	// See Shape::GetTrianglesNext
	virtual int						GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const override;

	/// Collide 2 shapes and pass any collisions on to ioCollector
	static void						sCollideConvexVsHeightField(const ConvexShape *inShape1, const HeightFieldShape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);

	/// Get height field position at sampled location (inX, inY).
	/// where inX and inY are integers in the range inX e [0, mSampleCount - 1] and inY e [0, mSampleCount - 1].
	const Vec3						GetPosition(uint inX, uint inY) const;

	/// Check if height field at sampled location (inX, inY) has collision (has a hole or not)
	bool							IsNoCollision(uint inX, uint inY) const;

	/// Projects inLocalPosition (a point in the space of the shape) along the Y axis onto the surface and returns it in outSurfacePosition.
	/// When there is no surface position (because of a hole or because the point is outside the heightfield) the function will return false.
	bool							ProjectOntoSurface(Vec3Arg inLocalPosition, Vec3 &outSurfacePosition, SubShapeID &outSubShapeID) const;

	// See Shape
	virtual void					SaveBinaryState(StreamOut &inStream) const override;
	virtual void					SaveMaterialState(PhysicsMaterialList &outMaterials) const override;
	virtual void					RestoreMaterialState(const PhysicsMaterialList &inMaterials) override;

	// See Shape::GetStats
	virtual Stats					GetStats() const override;

	// See Shape::GetVolume
	virtual float					GetVolume() const override											{ return 0; }

#ifdef JPH_DEBUG_RENDERER
	// Settings
	static bool						sDrawTriangleOutlines;
#endif // JPH_DEBUG_RENDERER

	// Register shape functions with the registry
	static void						sRegister();

protected:
	// See: Shape::RestoreBinaryState
	virtual void					RestoreBinaryState(StreamIn &inStream) override;

private:	
	class							DecodingContext;						///< Context class for walking through all nodes of a heightfield
	struct							HSGetTrianglesContext;					///< Context class for GetTrianglesStart/Next
	
	/// For location (inX, inY) get the block that contains this position and get the offset and scale needed to decode a uint8 height sample to a uint16
	void							GetBlockOffsetAndScale(uint inX, uint inY, float &outBlockOffset, float &outBlockScale) const;

	/// Faster version of GetPosition when block offset and scale are already known
	const Vec3						GetPosition(uint inX, uint inY, float inBlockOffset, float inBlockScale) const;
	
	/// Determine amount of bits needed to encode sub shape id
	uint							GetSubShapeIDBits() const;

	/// En/decode a sub shape ID. inX and inY specify the coordinate of the triangle. inTriangle == 0 is the lower triangle, inTriangle == 1 is the upper triangle.
	inline SubShapeID				EncodeSubShapeID(const SubShapeIDCreator &inCreator, uint inX, uint inY, uint inTriangle) const;
	inline void						DecodeSubShapeID(const SubShapeID &inSubShapeID, uint &outX, uint &outY, uint &outTriangle) const;

	/// Get the edge flags for a triangle
	inline uint8					GetEdgeFlags(uint inX, uint inY, uint inTriangle) const;

	/// Visit the entire height field using a visitor pattern
	template <class Visitor>
	void							WalkHeightField(Visitor &ioVisitor) const;

	/// A block of 2x2 ranges used to form a hierarchical grid, ordered left top, right top, left bottom, right bottom
	struct alignas(16) RangeBlock
	{
		uint16						mMin[4];
		uint16						mMax[4];
	};

	/// Offset of first RangedBlock in grid per level
	static const uint				sGridOffsets[];

	/// The height field is a surface defined by: mOffset + mScale * (x, mHeightSamples[y * mSampleCount + x], y).
	/// where x and y are integers in the range x and y e [0, mSampleCount - 1].
	Vec3							mOffset = Vec3::sZero();
	Vec3							mScale = Vec3::sReplicate(1.0f);

	/// The materials of square at (x, y) is: mMaterials[mMaterialIndices[x + y * (mSampleCount - 1)]]
	PhysicsMaterialList				mMaterials;

	// Calculated data
	uint32							mSampleCount = 0;
	uint16							mMinSample = HeightFieldShapeConstants::cNoCollisionValue16;	///< Min and max value in mHeightSamples quantized to 16 bit, for calculating bounding box
	uint16							mMaxSample = HeightFieldShapeConstants::cNoCollisionValue16;
	vector<RangeBlock>				mRangeBlocks;						///< Hierarchical grid of range data describing the height variations within 1 block. The grid for level <level> starts at offset sGridOffsets[<level>]
	vector<uint8>					mHeightSamples;						///< 8-bit height samples. Value [0, cMaxHeightValue8] maps to highest detail grid in mRangeBlocks [mMin, mMax].
	vector<uint8>					mActiveEdges;						///< (mSampleCount - 1)^2 * 3-bit active edge flags. 
	vector<uint8>					mMaterialIndices;					///< Compressed to the minimum amount of bits per material index (mSampleCount - 1) * (mSampleCount - 1) * mNumBitsPerMaterialIndex bits of data
	uint							mNumBitsPerMaterialIndex = 0;		///< Number of bits per material index

#ifdef JPH_DEBUG_RENDERER
	/// Temporary rendering data
	mutable vector<DebugRenderer::GeometryRef>	mGeometry;
	mutable bool					mCachedUseMaterialColors = false;	///< This is used to regenerate the triangle batch if the drawing settings change
#endif // JPH_DEBUG_RENDERER
};

} // JPH