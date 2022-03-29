// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/StaticArray.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <unordered_map>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

class CollideShapeSettings;

/// Class that constructs a ConvexShape (abstract)
class ConvexShapeSettings : public ShapeSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_ABSTRACT(ConvexShapeSettings)

	/// Constructor
									ConvexShapeSettings() = default;
	explicit						ConvexShapeSettings(const PhysicsMaterial *inMaterial)		: mMaterial(inMaterial) { }

	/// Set the density of the object in kg / m^3
	void							SetDensity(float inDensity)									{ mDensity = inDensity; }

	// Properties
	RefConst<PhysicsMaterial>		mMaterial;													///< Material assigned to this shape
	float							mDensity = 1000.0f;											///< Uniform density of the interior of the convex object (kg / m^3)
};

/// Base class for all convex shapes. Defines a virtual interface.
class ConvexShape : public Shape
{
public:
	/// Constructor
	explicit						ConvexShape(EShapeSubType inSubType) : Shape(EShapeType::Convex, inSubType) { }
									ConvexShape(EShapeSubType inSubType, const ConvexShapeSettings &inSettings, ShapeResult &outResult) : Shape(EShapeType::Convex, inSubType, inSettings, outResult), mMaterial(inSettings.mMaterial), mDensity(inSettings.mDensity) { }
									ConvexShape(EShapeSubType inSubType, const PhysicsMaterial *inMaterial) : Shape(EShapeType::Convex, inSubType), mMaterial(inMaterial) { }

	// See Shape::GetSubShapeIDBitsRecursive
	virtual uint					GetSubShapeIDBitsRecursive() const override					{ return 0; } // Convex shapes don't have sub shapes

	// See Shape::GetMaterial
	virtual const PhysicsMaterial *	GetMaterial(const SubShapeID &inSubShapeID) const override	{ JPH_ASSERT(inSubShapeID.IsEmpty(), "Invalid subshape ID"); return GetMaterial(); }

	// See Shape::CastRay
	virtual bool					CastRay(const RayCast &inRay, const SubShapeIDCreator &inSubShapeIDCreator, RayCastResult &ioHit) const override;
	virtual void					CastRay(const RayCast &inRay, const RayCastSettings &inRayCastSettings, const SubShapeIDCreator &inSubShapeIDCreator, CastRayCollector &ioCollector) const override;

	// See: Shape::CollidePoint
	virtual void					CollidePoint(Vec3Arg inPoint, const SubShapeIDCreator &inSubShapeIDCreator, CollidePointCollector &ioCollector) const override;

	// See Shape::GetTrianglesStart
	virtual void					GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const override;

	// See Shape::GetTrianglesNext
	virtual int						GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials = nullptr) const override;

	// See Shape::GetSubmergedVolume
	virtual void					GetSubmergedVolume(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, const Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, Vec3 &outCenterOfBuoyancy) const override;

	/// Function that provides an interface for GJK
	class Support
	{
	public:
		/// Warning: Virtual destructor will not be called on this object!
		virtual						~Support() = default;

		/// Calculate the support vector for this convex shape (includes / excludes the convex radius depending on how this was obtained). 
		/// Support vector is relative to the center of mass of the shape.
		virtual Vec3				GetSupport(Vec3Arg inDirection) const = 0;

		/// Convex radius of shape. Collision detection on penetrating shapes is much more expensive, 
		/// so you can add a radius around objects to increase the shape. This makes it far less likely that they will actually penetrate.
		virtual float				GetConvexRadius() const = 0;
	};

	/// Buffer to hold a Support object, used to avoid dynamic memory allocations
	class alignas(16) SupportBuffer
	{
	public:
		uint8						mData[4160];
	};

	/// How the GetSupport function should behave
	enum class ESupportMode	
	{
		ExcludeConvexRadius,		///< Return the shape excluding the convex radius
		IncludeConvexRadius,		///< Return the shape including the convex radius
	};

	/// Returns an object that provides the GetSupport function for this shape.
	/// inMode determines if this support function includes or excludes the convex radius.
	/// of the values returned by the GetSupport function. This improves numerical accuracy of the results.
	/// inScale scales this shape in local space.
	virtual const Support *			GetSupportFunction(ESupportMode inMode, SupportBuffer &inBuffer, Vec3Arg inScale) const = 0;

	/// Type definition for a supporting face
	using SupportingFace = StaticArray<Vec3, 32>;

	/// Get the vertices of the face that faces inDirection the most (includes convex radius).
	/// Face is relative to the center of mass of the shape.
	virtual void					GetSupportingFace(Vec3Arg inDirection, Vec3Arg inScale, SupportingFace &outVertices) const = 0;

	/// Material of the shape
	void							SetMaterial(const PhysicsMaterial *inMaterial)				{ mMaterial = inMaterial; }
	const PhysicsMaterial *			GetMaterial() const											{ return mMaterial != nullptr? mMaterial : PhysicsMaterial::sDefault; }

	/// Set density of the shape (kg / m^3)
	void							SetDensity(float inDensity)									{ mDensity = inDensity; }

	/// Get density of the shape (kg / m^3)
	float							GetDensity() const											{ return mDensity; }

#ifdef JPH_DEBUG_RENDERER
	// See Shape::DrawGetSupportFunction
	virtual void					DrawGetSupportFunction(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inDrawSupportDirection) const override;

	// See Shape::DrawGetSupportingFace
	virtual void					DrawGetSupportingFace(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform, Vec3Arg inScale) const override;
#endif // JPH_DEBUG_RENDERER

	// See Shape
	virtual void					SaveBinaryState(StreamOut &inStream) const override;
	virtual void					SaveMaterialState(PhysicsMaterialList &outMaterials) const override;
	virtual void					RestoreMaterialState(const PhysicsMaterialRefC *inMaterials, uint inNumMaterials) override;

	// Register shape functions with the registry
	static void						sRegister();

protected:
	// See: Shape::RestoreBinaryState
	virtual void					RestoreBinaryState(StreamIn &inStream) override;

	/// Vertex list that forms a unit sphere
	static const vector<Vec3>		sUnitSphereTriangles;

private:
	// Class for GetTrianglesStart/Next
	class							CSGetTrianglesContext;

	// Helper functions called by CollisionDispatch
	static void						sCollideConvexVsConvex(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);
	static void						sCastConvexVsConvex(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);

	// Properties
	RefConst<PhysicsMaterial>		mMaterial;													///< Material assigned to this shape
	float							mDensity = 1000.0f;											///< Uniform density of the interior of the convex object (kg / m^3)

#ifdef JPH_DEBUG_RENDERER
	mutable unordered_map<Vec3, DebugRenderer::GeometryRef> mGetSupportFunctionGeometry;
#endif // JPH_DEBUG_RENDERER
};

JPH_NAMESPACE_END
