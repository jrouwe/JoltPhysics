// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Core/ByteBuffer.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Geometry/IndexedTriangle.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

class ConvexShape;
class CollideShapeSettings;

/// Class that constructs a MeshShape
class MeshShapeSettings final : public ShapeSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(MeshShapeSettings)

	/// Default constructor for deserialization
									MeshShapeSettings() = default;

	/// Create a mesh shape.
									MeshShapeSettings(const TriangleList &inTriangles, const PhysicsMaterialList &inMaterials = PhysicsMaterialList());
									MeshShapeSettings(const VertexList &inVertices, const IndexedTriangleList &inTriangles, const PhysicsMaterialList &inMaterials = PhysicsMaterialList());

	/// Sanitize the mesh data. Remove duplicate and degenerate triangles.
	void							Sanitize();

	// See: ShapeSettings
	virtual ShapeResult				Create() const override;

	/// Mesh data.
	VertexList						mTriangleVertices;											///< Vertices belonging to mIndexedTriangles
	IndexedTriangleList				mIndexedTriangles;											///< Original list of indexed triangles

	/// Materials assigned to the triangles. Each triangle specifies which material it uses through its mMaterialIndex
	PhysicsMaterialList				mMaterials;

	/// Maximum number of triangles in each leaf of the axis aligned box tree. This is a balance between memory and performance. Can be in the range [1, MeshShape::MaxTrianglesPerLeaf].
	/// Sensible values are between 4 (for better performance) and 8 (for less memory usage).
	uint							mMaxTrianglesPerLeaf = 8;
};

/// A mesh shape, consisting of triangles. Cannot be used as a dynamic object.
class MeshShape final : public Shape
{
public:
	/// Constructor
									MeshShape() : Shape(EShapeType::Mesh, EShapeSubType::Mesh) { }
									MeshShape(const MeshShapeSettings &inSettings, ShapeResult &outResult);

	// See Shape::MustBeStatic
	virtual bool					MustBeStatic() const override								{ return true; }

	// See Shape::GetLocalBounds
	virtual AABox					GetLocalBounds() const override;

	// See Shape::GetSubShapeIDBitsRecursive
	virtual uint					GetSubShapeIDBitsRecursive() const override;

	// See Shape::GetInnerRadius
	virtual float					GetInnerRadius() const override								{ return 0.0f; }

	// See Shape::GetMassProperties
	virtual MassProperties			GetMassProperties() const override;
	
	// See Shape::GetMaterial
	virtual const PhysicsMaterial *	GetMaterial(const SubShapeID &inSubShapeID) const override;

	/// Get the list of all materials
	const PhysicsMaterialList &		GetMaterialList() const										{ return mMaterials; }

	/// Determine which material index a particular sub shape uses (note that if there are no materials this function will return 0 so check the array size)
	/// Note: This could for example be used to create a decorator shape around a mesh shape that overrides the GetMaterial call to replace a material with another material.
	uint							GetMaterialIndex(const SubShapeID &inSubShapeID) const;

	// See Shape::GetSurfaceNormal
	virtual Vec3					GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const override;

#ifdef JPH_DEBUG_RENDERER
	// See Shape::Draw
	virtual void					Draw(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override;
#endif // JPH_DEBUG_RENDERER

	// See Shape::CastRay
	virtual bool					CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;
	virtual void					CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) const override;

	/// See: Shape::CollidePoint
	/// Note that for CollidePoint to work for a mesh shape, the mesh needs to be closed (a manifold) or multiple non-intersecting manifolds. Triangles may be facing the interior of the manifold. 
	/// Insideness is tested by counting the amount of triangles encountered when casting an infinite ray from inPoint. If the number of hits is odd we're inside, if it's even we're outside.
	virtual void					CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector) const override;

	// See Shape::GetTrianglesStart
	virtual void					GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const override;

	// See Shape::GetTrianglesNext
	virtual int						GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const override;

	// See Shape::GetSubmergedVolume
	virtual void					GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy) const override { JPH_ASSERT(false, "Not supported"); }

	// See Shape
	virtual void					SaveBinaryState(StreamOut &inStream) const override;
	virtual void					SaveMaterialState(PhysicsMaterialList &outMaterials) const override;
	virtual void					RestoreMaterialState(const PhysicsMaterialRefC *inMaterials, uint inNumMaterials) override;

	// See Shape::GetStats
	virtual Stats					GetStats() const override;

	// See Shape::GetVolume
	virtual float					GetVolume() const override									{ return 0; }

#ifdef JPH_DEBUG_RENDERER
	// Settings
	static bool						sDrawTriangleGroups;
	static bool						sDrawTriangleOutlines;
#endif // JPH_DEBUG_RENDERER

	// Register shape functions with the registry
	static void						sRegister();

protected:
	// See: Shape::RestoreBinaryState
	virtual void					RestoreBinaryState(StreamIn &inStream) override;

private:
	struct							MSGetTrianglesContext;										///< Context class for GetTrianglesStart/Next

	static constexpr int			NumTriangleBits = 3;										///< How many bits to reserve to encode the triangle index
	static constexpr int			MaxTrianglesPerLeaf = 1 << NumTriangleBits;					///< Number of triangles that are stored max per leaf aabb node 

	/// Find and flag active edges
	static void						sFindActiveEdges(const VertexList &inVertices, IndexedTriangleList &ioIndices);

	/// Visit the entire tree using a visitor pattern
	template <class Visitor>
	void							WalkTree(Visitor &ioVisitor) const;

	/// Same as above but with a callback per triangle instead of per block of triangles
	template <class Visitor>
	void							WalkTreePerTriangle(const SubShapeIDCreator &inSubShapeIDCreator2, Visitor &ioVisitor) const;

	/// Decode a sub shape ID
	inline void						DecodeSubShapeID(const SubShapeID &inSubShapeID, const void *&outTriangleBlock, uint32 &outTriangleIndex) const;

	// Helper functions called by CollisionDispatch
	static void						sCollideConvexVsMesh(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);
	static void						sCollideSphereVsMesh(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);
	static void						sCastConvexVsMesh(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);
	static void						sCastSphereVsMesh(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);

	/// Materials assigned to the triangles. Each triangle specifies which material it uses through its mMaterialIndex
	PhysicsMaterialList				mMaterials;

	ByteBuffer						mTree;														///< Resulting packed data structure

	/// 8 bit flags stored per triangle
	enum ETriangleFlags
	{
		/// Material index
		FLAGS_MATERIAL_BITS			= 5,
		FLAGS_MATERIAL_MASK			= (1 << FLAGS_MATERIAL_BITS) - 1,

		/// Active edge bits
		FLAGS_ACTIVE_EGDE_SHIFT		= FLAGS_MATERIAL_BITS,
		FLAGS_ACTIVE_EDGE_BITS		= 3,
		FLAGS_ACTIVE_EDGE_MASK		= (1 << FLAGS_ACTIVE_EDGE_BITS) - 1
	};

#ifdef JPH_DEBUG_RENDERER
	mutable DebugRenderer::GeometryRef	mGeometry;												///< Debug rendering data
	mutable bool					mCachedTrianglesColoredPerGroup = false;					///< This is used to regenerate the triangle batch if the drawing settings change
	mutable bool					mCachedUseMaterialColors = false;							///< This is used to regenerate the triangle batch if the drawing settings change
#endif // JPH_DEBUG_RENDERER
};

JPH_NAMESPACE_END
