// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/MassProperties.h>
#include <Jolt/Physics/Collision/BackFaceMode.h>
#include <Jolt/Physics/Collision/CollisionCollector.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Core/Color.h>
#include <Jolt/Core/Result.h>
#include <Jolt/Core/NonCopyable.h>
#include <Jolt/ObjectStream/SerializableObject.h>

JPH_NAMESPACE_BEGIN

struct RayCast;
class RayCastSettings;
struct ShapeCast;
class ShapeCastSettings;
class RayCastResult;
class ShapeCastResult;
class CollidePointResult;
class CollideShapeResult;
class ShapeFilter;
class SubShapeIDCreator;
class SubShapeID;
class PhysicsMaterial;
class TransformedShape;
class Plane;
class Shape;
class StreamOut;
class StreamIn;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

using CastRayCollector = CollisionCollector<RayCastResult, CollisionCollectorTraitsCastRay>;
using CastShapeCollector = CollisionCollector<ShapeCastResult, CollisionCollectorTraitsCastShape>;
using CollidePointCollector = CollisionCollector<CollidePointResult, CollisionCollectorTraitsCollidePoint>;
using CollideShapeCollector = CollisionCollector<CollideShapeResult, CollisionCollectorTraitsCollideShape>;
using TransformedShapeCollector = CollisionCollector<TransformedShape, CollisionCollectorTraitsCollideShape>;

using ShapeRefC = RefConst<Shape>;
using ShapeList = vector<ShapeRefC>;
using PhysicsMaterialRefC = RefConst<PhysicsMaterial>;
using PhysicsMaterialList = vector<PhysicsMaterialRefC>;

/// Shapes are categorized in groups, each shape can return which group it belongs to through its Shape::GetType function.
enum class EShapeType : uint8
{
	Convex,							///< Used by ConvexShape, all shapes that use the generic convex vs convex collision detection system (box, sphere, capsule, tapered capsule, cylinder, triangle)
	Compound,						///< Used by CompoundShape
	Decorated,						///< Used by DecoratedShape
	Mesh,							///< Used by MeshShape
	HeightField,					///< Used by HeightFieldShape
	
	// User defined shapes
	User1,
	User2,
	User3,
	User4,
};

/// This enumerates all shape types, each shape can return its type through Shape::GetSubType
enum class EShapeSubType : uint8
{
	// Convex shapes
	Sphere,
	Box,
	Triangle,
	Capsule,
	TaperedCapsule,
	Cylinder,
	ConvexHull,
	
	// Compound shapes
	StaticCompound,
	MutableCompound,
	
	// Decorated shapes
	RotatedTranslated,
	Scaled,
	OffsetCenterOfMass,

	// Other shapes
	Mesh,
	HeightField,
	
	// User defined shapes
	User1,
	User2,
	User3,
	User4,
	User5,
	User6,
	User7,
	User8,
};

// Sets of shape sub types
static constexpr EShapeSubType sAllSubShapeTypes[] = { EShapeSubType::Sphere, EShapeSubType::Box, EShapeSubType::Triangle, EShapeSubType::Capsule, EShapeSubType::TaperedCapsule, EShapeSubType::Cylinder, EShapeSubType::ConvexHull, EShapeSubType::StaticCompound, EShapeSubType::MutableCompound, EShapeSubType::RotatedTranslated, EShapeSubType::Scaled, EShapeSubType::OffsetCenterOfMass, EShapeSubType::Mesh, EShapeSubType::HeightField, EShapeSubType::User1, EShapeSubType::User2, EShapeSubType::User3, EShapeSubType::User4, EShapeSubType::User5, EShapeSubType::User6, EShapeSubType::User7, EShapeSubType::User8 };
static constexpr EShapeSubType sConvexSubShapeTypes[] = { EShapeSubType::Sphere, EShapeSubType::Box, EShapeSubType::Triangle, EShapeSubType::Capsule, EShapeSubType::TaperedCapsule, EShapeSubType::Cylinder, EShapeSubType::ConvexHull };
static constexpr EShapeSubType sCompoundSubShapeTypes[] = { EShapeSubType::StaticCompound, EShapeSubType::MutableCompound };
static constexpr EShapeSubType sDecoratorSubShapeTypes[] = { EShapeSubType::RotatedTranslated, EShapeSubType::Scaled, EShapeSubType::OffsetCenterOfMass };

/// How many shape types we support
static constexpr uint NumSubShapeTypes = (uint)size(sAllSubShapeTypes);

/// Names of sub shape types
static constexpr const char *sSubShapeTypeNames[] = { "Sphere", "Box", "Triangle", "Capsule", "TaperedCapsule", "Cylinder", "ConvexHull", "StaticCompound", "MutableCompound", "RotatedTranslated", "Scaled", "OffsetCenterOfMass", "Mesh", "HeightField", "User1", "User2", "User3", "User4", "User5", "User6", "User7", "User8" };
static_assert(size(sSubShapeTypeNames) == NumSubShapeTypes);

/// Class that can construct shapes and that is serializable using the ObjectStream system.
/// Can be used to store shape data in 'uncooked' form (i.e. in a form that is still human readable and authorable).
/// Once the shape has been created using the Create() function, the data will be moved into the Shape class
/// in a form that is optimized for collision detection. After this, the ShapeSettings object is no longer needed
/// and can be destroyed. Each shape class has a derived class of the ShapeSettings object to store shape specific
/// data.
class ShapeSettings : public SerializableObject, public RefTarget<ShapeSettings>
{
public:
	JPH_DECLARE_SERIALIZABLE_ABSTRACT(ShapeSettings)

	using ShapeResult = Result<Ref<Shape>>;

	/// Create a shape according to the settings specified by this object. 
	virtual ShapeResult				Create() const = 0;

	/// User data (to be used freely by the application)
	uint64							mUserData = 0;

protected:
	mutable ShapeResult				mCachedResult;
};

/// Function table for functions on shapes
class ShapeFunctions
{
public:
	/// Construct a shape
	Shape *							(*mConstruct)() = nullptr;

	/// Color of the shape when drawing
	Color							mColor = Color::sBlack;

	/// Get an entry in the registry for a particular sub type
	static inline ShapeFunctions &	sGet(EShapeSubType inSubType)										{ return sRegistry[(int)inSubType]; }

private:
	static ShapeFunctions 			sRegistry[NumSubShapeTypes];
};

/// Base class for all shapes (collision volume of a body). Defines a virtual interface for collision detection.
class Shape : public RefTarget<Shape>, public NonCopyable
{
public:
	using ShapeResult = ShapeSettings::ShapeResult;

	/// Constructor
									Shape(EShapeType inType, EShapeSubType inSubType) : mShapeType(inType), mShapeSubType(inSubType) { }
									Shape(EShapeType inType, EShapeSubType inSubType, const ShapeSettings &inSettings, [[maybe_unused]] ShapeResult &outResult) : mUserData(inSettings.mUserData), mShapeType(inType), mShapeSubType(inSubType) { }

	/// Destructor
	virtual							~Shape() = default;

	/// Get type
	inline EShapeType				GetType() const														{ return mShapeType; }
	inline EShapeSubType			GetSubType() const													{ return mShapeSubType; }

	/// User data (to be used freely by the application)
	uint64							GetUserData() const													{ return mUserData; }
	void							SetUserData(uint64 inUserData)										{ mUserData = inUserData; }

	/// Check if this shape can only be used to create a static body or if it can also be dynamic/kinematic
	virtual bool					MustBeStatic() const												{ return false; }

	/// All shapes are centered around their center of mass. This function returns the center of mass position that needs to be applied to transform the shape to where it was created.
	virtual Vec3					GetCenterOfMass() const												{ return Vec3::sZero(); }

	/// Get local bounding box including convex radius, this box is centered around the center of mass rather than the world transform
	virtual AABox					GetLocalBounds() const = 0;

	/// Get the max number of sub shape ID bits that are needed to be able to address any leaf shape in this shape. Used mainly for checking that it is smaller or equal than SubShapeID::MaxBits.
	virtual uint					GetSubShapeIDBitsRecursive() const = 0;

	/// Get world space bounds including convex radius.
	/// This shape is scaled by inScale in local space first.
	/// This function can be overridden to return a closer fitting world space bounding box, by default it will just transform what GetLocalBounds() returns.
	virtual AABox					GetWorldSpaceBounds(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale) const { return GetLocalBounds().Scaled(inScale).Transformed(inCenterOfMassTransform); }

	/// Returns the radius of the biggest sphere that fits entirely in the shape. In case this shape consists of multiple sub shapes, it returns the smallest sphere of the parts. 
	/// This can be used as a measure of how far the shape can be moved without risking going through geometry.
	virtual float					GetInnerRadius() const = 0;

	/// Calculate the mass and inertia of this shape
	virtual MassProperties			GetMassProperties() const = 0;

	/// Get the material assigned to a particular sub shape ID
	virtual const PhysicsMaterial *	GetMaterial(const SubShapeID &inSubShapeID) const = 0;

	/// Get the surface normal of a particular sub shape ID and point on surface (all vectors are relative to center of mass for this shape).
	/// Note: When you have a CollideShapeResult or ShapeCastResult you should use -mPenetrationAxis.Normalized() as contact normal as GetSurfaceNormal will only return face normals (and not vertex or edge normals).
	virtual Vec3					GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const = 0;

	/// Get the user data of a particular sub shape ID
	virtual uint64					GetSubShapeUserData(const SubShapeID &inSubShapeID) const			{ return mUserData; }

	/// Get the direct child sub shape and its transform for a sub shape ID.
	/// @param inSubShapeID Sub shape ID that indicates the path to the leaf shape
	/// @param inPositionCOM The position of the center of mass of this shape
	/// @param inRotation The orientation of this shape
	/// @param inScale Scale of this shape
	/// @param outRemainder The remainder of the sub shape ID after removing the sub shape
	/// @return Direct child sub shape and its transform, note that the body ID and sub shape ID will be invalid
	virtual TransformedShape		GetSubShapeTransformedShape(const SubShapeID &inSubShapeID, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale, SubShapeID &outRemainder) const;

	/// Gets the properties needed to do buoyancy calculations for a body using this shape
	/// @param inCenterOfMassTransform Transform that takes this shape (centered around center of mass) to world space
	/// @param inScale Scale in local space of the shape
	/// @param inSurface The surface plane of the liquid in world space
	/// @param outTotalVolume On return this contains the total volume of the shape
	/// @param outSubmergedVolume On return this contains the submerged volume of the shape
	/// @param outCenterOfBuoyancy On return this contains the world space center of mass of the submerged volume
	virtual void					GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy) const = 0;
	
#ifdef JPH_DEBUG_RENDERER
	/// Draw the shape at a particular location with a particular color (debugging purposes)
	virtual void					Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const = 0;

	/// Draw the results of the GetSupportFunction with the convex radius added back on to show any errors introduced by this process (only relevant for convex shapes)
	virtual void					DrawGetSupportFunction(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inDrawSupportDirection) const { /* Only implemented for convex shapes */ }

	/// Draw the results of the GetSupportingFace function to show any errors introduced by this process (only relevant for convex shapes)
	virtual void					DrawGetSupportingFace(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale) const { /* Only implemented for convex shapes */ }
#endif // JPH_DEBUG_RENDERER

	/// Cast a ray against this shape, returns true if it finds a hit closer than ioHit.mFraction and updates that fraction. Otherwise ioHit is left untouched and the function returns false.
	/// Note that the ray should be relative to the center of mass of this shape (i.e. subtract Shape::GetCenterOfMass() from RayCast::mOrigin if you want to cast against the shape in the space it was created).
	/// Convex objects will be treated as solid (meaning if the ray starts inside, you'll get a hit fraction of 0) and back face hits against triangles are returned.
	/// If you want the surface normal of the hit use GetSurfaceNormal(ioHit.mSubShapeID2, inRay.GetPointOnRay(ioHit.mFraction)).
	virtual bool					CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const = 0;

	/// Cast a ray against this shape. Allows returning multiple hits through ioCollector. Note that this version is more flexible but also slightly slower than the CastRay function that returns only a single hit.
	/// If you want the surface normal of the hit use GetSurfaceNormal(collected sub shape ID, inRay.GetPointOnRay(collected faction)).
	virtual void					CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) const = 0;

	/// Check if inPoint is inside this shape. For this tests all shapes are treated as if they were solid. 
	/// Note that inPoint should be relative to the center of mass of this shape (i.e. subtract Shape::GetCenterOfMass() from inPoint if you want to test against the shape in the space it was created).
	/// For a mesh shape, this test will only provide sensible information if the mesh is a closed manifold.
	/// For each shape that collides, ioCollector will receive a hit.
	virtual void					CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector) const = 0;

	/// Collect the leaf transformed shapes of all leaf shapes of this shape.
	/// inBox is the world space axis aligned box which leaf shapes should collide with.
	/// inPositionCOM/inRotation/inScale describes the transform of this shape.
	/// inSubShapeIDCeator represents the current sub shape ID of this shape.
	virtual void					CollectTransformedShapes(const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale, const SubShapeIDCreator &inSubShapeIDCreator, TransformedShapeCollector &ioCollector) const;
	   
	/// Transforms this shape and all of its children with inTransform, resulting shape(s) are passed to ioCollector.
	/// Note that not all shapes support all transforms (especially true for scaling), the resulting shape will try to match the transform as accurately as possible.
	/// @param inCenterOfMassTransform The transform (rotation, translation, scale) that the center of mass of the shape should get
	/// @param ioCollector The transformed shapes will be passed to this collector
	virtual void					TransformShape(Mat44Arg inCenterOfMassTransform, TransformedShapeCollector &ioCollector) const;

	/// Scale this shape. Note that not all shapes support all scales, this will return a shape that matches the scale as accurately as possible.
	/// @param inScale The scale to use for this shape (note: this scale is applied to the entire shape in the space it was created, most function apply the scale in the space of the leaf shapes and from the center of mass!)
	ShapeResult						ScaleShape(Vec3Arg inScale) const;

	/// An opaque buffer that holds shape specific information during GetTrianglesStart/Next.
	struct alignas(16)				GetTrianglesContext { uint8 mData[4288]; };

	/// This is the minimum amount of triangles that should be requested through GetTrianglesNext.
	static constexpr int			cGetTrianglesMinTrianglesRequested = 32;

	/// To start iterating over triangles, call this function first. 
	/// ioContext is a temporary buffer and should remain untouched until the last call to GetTrianglesNext.
	/// inBox is the world space bounding in which you want to get the triangles.
	/// inPositionCOM/inRotation/inScale describes the transform of this shape.
	/// To get the actual triangles call GetTrianglesNext.
	virtual void					GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const = 0;

	/// Call this repeatedly to get all triangles in the box.
	/// outTriangleVertices should be large enough to hold 3 * inMaxTriangleRequested entries.
	/// outMaterials (if it is not null) should contain inMaxTrianglesRequested entries.
	/// The function returns the amount of triangles that it found (which will be <= inMaxTrianglesRequested), or 0 if there are no more triangles.
	/// Note that the function can return a value < inMaxTrianglesRequested and still have more triangles to process (triangles can be returned in blocks).
	/// Note that the function may return triangles outside of the requested box, only coarse culling is performed on the returned triangles.
	virtual int						GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const = 0;

	///@name Binary serialization of the shape. Note that this saves the 'cooked' shape in a format which will not be backwards compatible for newer library versions. 
	/// In this case you need to recreate the shape from the ShapeSettings object and save it again. The user is expected to call SaveBinaryState followed by SaveMaterialState and SaveSubShapeState. 
	/// The stream should be stored as is and the material and shape list should be saved using the applications own serialization system (e.g. by assigning an ID to each pointer).
	/// When restoring data, call sRestoreFromBinararyState to get the shape and then call RestoreMaterialState and RestoreSubShapeState to restore the pointers to the external objects.
	///@{

	/// Saves the contents of the shape in binary form to inStream.
	virtual void					SaveBinaryState(StreamOut &inStream) const;

	/// Creates a Shape of the correct type and restores its contents from the binary stream inStream.
	static ShapeResult				sRestoreFromBinaryState(StreamIn &inStream);

	/// Outputs the material references that this shape has to outMaterials.
	virtual void					SaveMaterialState(PhysicsMaterialList &outMaterials) const			{ /* By default do nothing */ }

	/// Restore the material references after calling sRestoreFromBinaryState. Note that the exact same materials need to be provided in the same order as returned by SaveMaterialState.
	virtual void					RestoreMaterialState(const PhysicsMaterialRefC *inMaterials, uint inNumMaterials) { JPH_ASSERT(inNumMaterials == 0); }

	/// Outputs the shape references that this shape has to outSubShapes.
	virtual void					SaveSubShapeState(ShapeList &outSubShapes) const					{ /* By default do nothing */ }

	/// Restore the shape references after calling sRestoreFromBinaryState. Note that the exact same shapes need to be provided in the same order as returned by SaveSubShapeState.
	virtual void					RestoreSubShapeState(const ShapeRefC *inSubShapes, uint inNumShapes) { JPH_ASSERT(inNumShapes == 0); }

	using ShapeToIDMap = unordered_map<const Shape *, uint32>;
	using MaterialToIDMap = unordered_map<const PhysicsMaterial *, uint32>;
	using IDToShapeMap = vector<Ref<Shape>>;
	using IDToMaterialMap = vector<Ref<PhysicsMaterial>>;

	/// Save this shape, all its children and its materials. Pass in an empty map in ioShapeMap / ioMaterialMap or reuse the same map while saving multiple shapes to the same stream in order to avoid writing duplicates.
	void							SaveWithChildren(StreamOut &inStream, ShapeToIDMap &ioShapeMap, MaterialToIDMap &ioMaterialMap) const;

	/// Restore a shape, all its children and materials. Pass in an empty map in ioShapeMap / ioMaterialMap or reuse the same map while reading multiple shapes from the same stream in order to restore duplicates.
	static ShapeResult				sRestoreWithChildren(StreamIn &inStream, IDToShapeMap &ioShapeMap, IDToMaterialMap &ioMaterialMap);

	///@}

	/// Class that holds information about the shape that can be used for logging / data collection purposes
	struct Stats
	{
									Stats(size_t inSizeBytes, uint inNumTriangles) : mSizeBytes(inSizeBytes), mNumTriangles(inNumTriangles) { }

		size_t						mSizeBytes;				///< Amount of memory used by this shape (size in bytes)
		uint						mNumTriangles;			///< Number of triangles in this shape (when applicable)
	};

	/// Get stats of this shape. Use for logging / data collection purposes only. Does not add values from child shapes, use GetStatsRecursive for this.
	virtual Stats					GetStats() const = 0;

	using VisitedShapes = unordered_set<const Shape *>;

	/// Get the combined stats of this shape and its children.
	/// @param ioVisitedShapes is used to track which shapes have already been visited, to avoid calculating the wrong memory size.
	virtual Stats					GetStatsRecursive(VisitedShapes &ioVisitedShapes) const;

	///< Volume of this shape (m^3). Note that for compound shapes the volume may be incorrect since child shapes can overlap which is not accounted for.
	virtual float					GetVolume() const = 0;

	/// Test if inScale is a valid scale for this shape. Some shapes can only be scaled uniformly, compound shapes cannot handle shapes
	/// being rotated and scaled (this would cause shearing). In this case this function will return false.
	virtual bool					IsValidScale(Vec3Arg inScale) const									{ return !inScale.IsNearZero(); }

#ifdef JPH_DEBUG_RENDERER
	/// Debug helper which draws the intersection between water and the shapes, the center of buoyancy and the submerged volume
	static bool						sDrawSubmergedVolumes;
#endif // JPH_DEBUG_RENDERER

protected:
	/// This function should not be called directly, it is used by sRestoreFromBinaryState.
	virtual void					RestoreBinaryState(StreamIn &inStream);

private:
	uint64							mUserData = 0;
	EShapeType						mShapeType;
	EShapeSubType					mShapeSubType;
};

JPH_NAMESPACE_END
