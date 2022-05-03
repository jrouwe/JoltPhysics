// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifndef JPH_DEBUG_RENDERER
	#error This file should only be included when JPH_DEBUG_RENDERER is defined
#endif // !JPH_DEBUG_RENDERER

#include <Jolt/Core/Color.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Core/HashCombine.h>
#include <Jolt/Math/Float2.h>
#include <Jolt/Geometry/IndexedTriangle.h>
#include <Jolt/Geometry/AABox.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <unordered_map>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

class OrientedBox;

/// Simple triangle renderer for debugging purposes.
class DebugRenderer
{
public:
	/// Constructor
										DebugRenderer();
	virtual								~DebugRenderer();

	/// Draw line
	void								DrawLine(Vec3Arg inFrom, Vec3Arg inTo, ColorArg inColor) { Float3 from, to; inFrom.StoreFloat3(&from); inTo.StoreFloat3(&to); DrawLine(from, to, inColor); }
	virtual void						DrawLine(const Float3 &inFrom, const Float3 &inTo, ColorArg inColor) = 0;

	/// Draw wireframe box
	void								DrawWireBox(const AABox &inBox, ColorArg inColor);
	void								DrawWireBox(const OrientedBox &inBox, ColorArg inColor);
	void								DrawWireBox(Mat44Arg inMatrix, const AABox &inBox, ColorArg inColor);

	/// Draw a marker on a position
	void								DrawMarker(Vec3Arg inPosition, ColorArg inColor, float inSize);

	/// Draw an arrow
	void								DrawArrow(Vec3Arg inFrom, Vec3Arg inTo, ColorArg inColor, float inSize);

	/// Draw coordinate system (3 arrows, x = red, y = green, z = blue)
	void								DrawCoordinateSystem(Mat44Arg inTransform, float inSize = 1.0f);

	/// Draw a plane through inPoint with normal inNormal
	void								DrawPlane(Vec3Arg inPoint, Vec3Arg inNormal, ColorArg inColor, float inSize);

	/// Draw wireframe triangle
	void								DrawWireTriangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor);

	/// Draw a wireframe polygon
	template <class VERTEX_ARRAY>
	void								DrawWirePolygon(const VERTEX_ARRAY &inVertices, ColorArg inColor, float inArrowSize = 0.0f) { for (typename VERTEX_ARRAY::size_type i = 0; i < inVertices.size(); ++i) DrawArrow(inVertices[i], inVertices[(i + 1) % inVertices.size()], inColor, inArrowSize); }
	template <class VERTEX_ARRAY>
	void								DrawWirePolygon(Mat44Arg inTransform, const VERTEX_ARRAY &inVertices, ColorArg inColor, float inArrowSize = 0.0f) { for (typename VERTEX_ARRAY::size_type i = 0; i < inVertices.size(); ++i) DrawArrow(inTransform * inVertices[i], inTransform * inVertices[(i + 1) % inVertices.size()], inColor, inArrowSize); }

	/// Draw wireframe sphere
	void								DrawWireSphere(Vec3Arg inCenter, float inRadius, ColorArg inColor, int inLevel = 3);
	void								DrawWireUnitSphere(Mat44Arg inMatrix, ColorArg inColor, int inLevel = 3);

	/// Enum that determines if a shadow should be cast or not
	enum class ECastShadow
	{
		On,								// This shape should cast a shadow
		Off								// This shape should not cast a shadow
	};

	/// Determines how triangles are drawn
	enum class EDrawMode
	{
		Solid,							///< Draw as a solid shape
		Wireframe,						///< Draw as wireframe
	};

	/// Draw a single back face culled triangle without any shadows
	virtual void						DrawTriangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor) = 0;

	/// Draw a box
	void								DrawBox(const AABox &inBox, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);
	void								DrawBox(Mat44Arg inMatrix, const AABox &inBox, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);

	/// Draw a sphere
	void								DrawSphere(Vec3Arg inCenter, float inRadius, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);
	void								DrawUnitSphere(Mat44Arg inMatrix, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);

	/// Draw a capsule with one half sphere at (0, -inHalfHeightOfCylinder, 0) and the other half sphere at (0, inHalfHeightOfCylinder, 0) and radius inRadius. 
	/// The capsule will be transformed by inMatrix.
	void								DrawCapsule(Mat44Arg inMatrix, float inHalfHeightOfCylinder, float inRadius, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);

	/// Draw a cylinder with top (0, inHalfHeight, 0) and bottom (0, -inHalfHeight, 0) and radius inRadius.
	/// The cylinder will be transformed by inMatrix
	void								DrawCylinder(Mat44Arg inMatrix, float inHalfHeight, float inRadius, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);

	/// Draw a bottomless cone. 
	/// @param inTop Top of cone, center of base is at inTop + inAxis. 
	/// @param inAxis Height and direction of cone
	/// @param inPerpendicular Perpendicular vector to inAxis.
	/// @param inHalfAngle Specifies the cone angle in radians (angle measured between inAxis and cone surface).
	/// @param inLength The length of the cone.
	/// @param inColor Color to use for drawing the cone.
	/// @param inCastShadow determins if this geometry should cast a shadow or not.
	/// @param inDrawMode determines if we draw the geometry solid or in wireframe.
	void								DrawOpenCone(Vec3Arg inTop, Vec3Arg inAxis, Vec3Arg inPerpendicular, float inHalfAngle, float inLength, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);

	/// Draws rotation limits as used by the SwingTwistConstraintPart.
	/// @param inMatrix Matrix that transforms from constraint space to world space
	/// @param inSwingYHalfAngle See SwingTwistConstraintPart
	/// @param inSwingZHalfAngle See SwingTwistConstraintPart
	/// @param inEdgeLength Size of the edge of the cone shape
	/// @param inColor Color to use for drawing the cone.
	/// @param inCastShadow determins if this geometry should cast a shadow or not.
	/// @param inDrawMode determines if we draw the geometry solid or in wireframe.
	void								DrawSwingLimits(Mat44Arg inMatrix, float inSwingYHalfAngle, float inSwingZHalfAngle, float inEdgeLength, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);

	/// Draw a pie (part of a circle). 
	/// @param inCenter The center of the circle.
	/// @param inRadius Radius of the circle.
	/// @param inNormal The plane normal in which the pie resides.
	/// @param inAxis The axis that defines an angle of 0 radians. 
	/// @param inMinAngle The pie will be drawn between [inMinAngle, inMaxAngle] (in radians).
	/// @param inMaxAngle The pie will be drawn between [inMinAngle, inMaxAngle] (in radians).
	/// @param inColor Color to use for drawing the pie.
	/// @param inCastShadow determins if this geometry should cast a shadow or not.
	/// @param inDrawMode determines if we draw the geometry solid or in wireframe.
	void								DrawPie(Vec3Arg inCenter, float inRadius, Vec3Arg inNormal, Vec3Arg inAxis, float inMinAngle, float inMaxAngle, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid);

	/// Singleton instance
	static DebugRenderer *				sInstance;

	/// Vertex format used by the triangle renderer
	class Vertex
	{
	public:
		Float3							mPosition;
		Float3							mNormal;
		Float2							mUV;
		Color							mColor;
	};

	/// A single triangle
	class Triangle
	{
	public:
										Triangle() = default;
										Triangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor);
										Triangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor, Vec3Arg inUVOrigin, Vec3Arg inUVDirection);

		Vertex							mV[3];
	};

	/// Handle for a batch of triangles
	using Batch = Ref<RefTargetVirtual>;

	/// A single level of detail
	class LOD
	{
	public:
		Batch							mTriangleBatch;
		float							mDistance;
	};

	/// A geometry primitive containing triangle batches for various lods
	class Geometry : public RefTarget<Geometry>
	{
	public:
		/// Constructor
										Geometry(const AABox &inBounds) : mBounds(inBounds) { }
										Geometry(const Batch &inBatch, const AABox &inBounds) : mBounds(inBounds) { mLODs.push_back({ inBatch, FLT_MAX }); }

		/// All level of details for this mesh
		vector<LOD>						mLODs;

		/// Bounding box that encapsulates all LODs
		AABox							mBounds;
	};

	/// Handle for a lodded triangle batch
	using GeometryRef = Ref<Geometry>;

	/// Calculate bounding box for a batch of triangles
	static AABox						sCalculateBounds(const Vertex *inVertices, int inVertexCount);

	/// Create a batch of triangles that can be drawn efficiently
	virtual Batch						CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) = 0;
	virtual Batch						CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) = 0;
	Batch								CreateTriangleBatch(const vector<Triangle> &inTriangles) { return CreateTriangleBatch(inTriangles.empty()? nullptr : &inTriangles[0], (int)inTriangles.size()); }
	Batch								CreateTriangleBatch(const vector<Vertex> &inVertices, const vector<uint32> &inIndices) { return CreateTriangleBatch(inVertices.empty()? nullptr : &inVertices[0], (int)inVertices.size(), inIndices.empty()? nullptr : &inIndices[0], (int)inIndices.size()); }
	Batch								CreateTriangleBatch(const VertexList &inVertices, const IndexedTriangleNoMaterialList &inTriangles);

	/// Create a primitive for a convex shape using its support function
	using SupportFunction = function<Vec3 (Vec3Arg inDirection)>;
	Batch								CreateTriangleBatchForConvex(SupportFunction inGetSupport, int inLevel, AABox *outBounds = nullptr); 
	GeometryRef							CreateTriangleGeometryForConvex(SupportFunction inGetSupport);

	/// Determines which polygons are culled
	enum class ECullMode
	{
		CullBackFace,					///< Don't draw backfacing polygons
		CullFrontFace,					///< Don't draw front facing polygons
		Off								///< Don't do culling and draw both sides
	};

	/// Draw some geometry
	/// @param inModelMatrix is the matrix that transforms the geometry to world space.
	/// @param inWorldSpaceBounds is the bounding box of the geometry after transforming it into world space.
	/// @param inLODScaleSq is the squared scale of the model matrix, it is multiplied with the LOD distances in inGeometry to calculate the real LOD distance (so a number > 1 will force a higher LOD).
	/// @param inModelColor is the color with which to multiply the vertex colors in inGeometry.
	/// @param inGeometry The geometry to draw.
	/// @param inCullMode determines which polygons are culled.
	/// @param inCastShadow determines if this geometry should cast a shadow or not.
	/// @param inDrawMode determines if we draw the geometry solid or in wireframe.
	virtual void						DrawGeometry(Mat44Arg inModelMatrix, const AABox &inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) = 0;
	void								DrawGeometry(Mat44Arg inModelMatrix, ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) { DrawGeometry(inModelMatrix, inGeometry->mBounds.Transformed(inModelMatrix), max(max(inModelMatrix.GetAxisX().LengthSq(), inModelMatrix.GetAxisY().LengthSq()), inModelMatrix.GetAxisZ().LengthSq()), inModelColor, inGeometry, inCullMode, inCastShadow, inDrawMode); }

	/// Draw text
	virtual void						DrawText3D(Vec3Arg inPosition, const string_view &inString, ColorArg inColor = Color::sWhite, float inHeight = 0.5f)	= 0;

protected:
	/// Initialize the system, must be called from the constructor of the DebugRenderer implementation
	void								Initialize();

private:
	/// Recursive helper function for DrawWireUnitSphere
	void								DrawWireUnitSphereRecursive(Mat44Arg inMatrix, ColorArg inColor, Vec3Arg inDir1, Vec3Arg inDir2, Vec3Arg inDir3, int inLevel);

	/// Helper functions to create a box
	void								CreateQuad(vector<uint32> &ioIndices, vector<Vertex> &ioVertices, Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, Vec3Arg inV4);

	/// Helper functions to create a vertex and index buffer for a sphere
	void								Create8thSphereRecursive(vector<uint32> &ioIndices, vector<Vertex> &ioVertices, Vec3Arg inDir1, uint32 &ioIdx1, Vec3Arg inDir2, uint32 &ioIdx2, Vec3Arg inDir3, uint32 &ioIdx3, const Float2 &inUV, SupportFunction inGetSupport, int inLevel);
	void								Create8thSphere(vector<uint32> &ioIndices, vector<Vertex> &ioVertices, Vec3Arg inDir1, Vec3Arg inDir2, Vec3Arg inDir3, const Float2 &inUV, SupportFunction inGetSupport, int inLevel);

	// Predefined shapes
	GeometryRef							mBox;
	GeometryRef							mSphere;
	GeometryRef							mCapsuleTop;
	GeometryRef							mCapsuleMid;
	GeometryRef							mCapsuleBottom;
	GeometryRef							mOpenCone;
	GeometryRef							mCylinder;

	struct SwingLimits
	{
		bool							operator == (const SwingLimits &inRHS) const	{ return mSwingYHalfAngle == inRHS.mSwingYHalfAngle && mSwingZHalfAngle == inRHS.mSwingZHalfAngle; }

		float							mSwingYHalfAngle;
		float							mSwingZHalfAngle;
	};

	JPH_MAKE_HASH_STRUCT(SwingLimits, SwingLimitsHasher, t.mSwingYHalfAngle, t.mSwingZHalfAngle)

	using SwingBatches = unordered_map<SwingLimits, GeometryRef, SwingLimitsHasher>;
	SwingBatches						mSwingLimits;

	using PieBatces = unordered_map<float, GeometryRef>;
	PieBatces							mPieLimits;
};

JPH_NAMESPACE_END
