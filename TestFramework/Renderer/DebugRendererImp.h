// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#else
	// Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
	#define JPH_DEBUG_RENDERER
	// Make sure the debug renderer symbols don't get imported or exported
	#define JPH_DEBUG_RENDERER_EXPORT
	#include <Jolt/Renderer/DebugRenderer.h>
	#undef JPH_DEBUG_RENDERER
	#undef JPH_DEBUG_RENDERER_EXPORT
#endif

#include <Renderer/Renderer.h>
#include <Jolt/Core/Mutex.h>
#include <Jolt/Core/UnorderedMap.h>

class Renderer;
class Font;

/// Implementation of DebugRenderer
class DebugRendererImp final : public DebugRenderer
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
										DebugRendererImp(Renderer *inRenderer, const Font *inFont);

	/// Implementation of DebugRenderer interface
	virtual void						DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override;
	virtual void						DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) override;
	virtual Batch						CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) override;
	virtual Batch						CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) override;
	virtual void						DrawGeometry(RMat44Arg inModelMatrix, const AABox &inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
	virtual void						DrawText3D(RVec3Arg inPosition, const string_view &inString, ColorArg inColor, float inHeight) override;

	/// Draw all primitives from the light source
	void								DrawShadowPass();

	/// Draw all primitives that were added
	void								Draw();

	/// Clear all primitives (to be called after drawing)
	void								Clear();

private:
	/// Helper functions to draw sub parts
	void								DrawLines();
	void								DrawTriangles();
	void								DrawTexts();

	/// Helper functions to clear sub parts
	void								ClearLines();
	void								ClearTriangles();
	void								ClearTexts();

	/// Finalize the current locked primitive and add it to the primitives to draw
	void								FinalizePrimitive();

	/// Ensure that the current locked primitive has space for a primitive consisting inVtxSize vertices
	void								EnsurePrimitiveSpace(int inVtxSize);

	Renderer *							mRenderer;

	/// Shaders for triangles
	unique_ptr<PipelineState>			mTriangleStateBF;
	unique_ptr<PipelineState>			mTriangleStateFF;
	unique_ptr<PipelineState>			mTriangleStateWire;

	/// Shaders for shadow pass for triangles
	unique_ptr<PipelineState>			mShadowStateBF;
	unique_ptr<PipelineState>			mShadowStateFF;
	unique_ptr<PipelineState>			mShadowStateWire;

	/// Lock that protects the triangle batches from being accessed from multiple threads
	Mutex								mPrimitivesLock;

	Batch								mEmptyBatch;

	/// Properties for a single rendered instance
	struct Instance
	{
		/// Constructor
										Instance(Mat44Arg inModelMatrix, Mat44Arg inModelMatrixInvTrans, ColorArg inModelColor) : mModelMatrix(inModelMatrix), mModelMatrixInvTrans(inModelMatrixInvTrans), mModelColor(inModelColor) { }

		Mat44							mModelMatrix;
		Mat44							mModelMatrixInvTrans;
		Color							mModelColor;
	};

	/// Rendered instance with added information for lodding
	struct InstanceWithLODInfo : public Instance
	{
		/// Constructor
										InstanceWithLODInfo(Mat44Arg inModelMatrix, Mat44Arg inModelMatrixInvTrans, ColorArg inModelColor, const AABox &inWorldSpaceBounds, float inLODScaleSq) : Instance(inModelMatrix, inModelMatrixInvTrans, inModelColor), mWorldSpaceBounds(inWorldSpaceBounds), mLODScaleSq(inLODScaleSq) { }

		/// Bounding box for culling
		AABox							mWorldSpaceBounds;

		/// Square of scale factor for LODding (1 = original, > 1 = lod out further, < 1 = lod out earlier)
		float							mLODScaleSq;
	};

	/// Properties for a batch of instances that have the same primitive
	struct Instances
	{
		Array<InstanceWithLODInfo>		mInstances;

		/// Start index in mInstancesBuffer for each of the LOD in the geometry pass. Length is one longer than the number of LODs to indicate how many instances the last lod has.
		Array<int>						mGeometryStartIdx;

		/// Start index in mInstancesBuffer for each of the LOD in the light pass. Length is one longer than the number of LODs to indicate how many instances the last lod has.
		Array<int>						mLightStartIdx;
	};

	using InstanceMap = UnorderedMap<GeometryRef, Instances>;

	/// Clear map of instances and make it ready for the next frame
	void								ClearMap(InstanceMap &ioInstances);

	/// Helper function to draw instances
	inline void							DrawInstances(const Geometry *inGeometry, const Array<int> &inStartIdx);

	/// List of primitives that are finished and ready for drawing
	InstanceMap							mWireframePrimitives;
	InstanceMap							mPrimitives;
	InstanceMap							mTempPrimitives;
	InstanceMap							mPrimitivesBackFacing;
	int									mNumInstances = 0;
	Ref<RenderInstances>				mInstancesBuffer[Renderer::cFrameCount];

	/// Primitive that is being built + its properties
	Ref<RenderPrimitive>				mLockedPrimitive;
	Vertex *							mLockedVerticesStart = nullptr;
	Vertex *							mLockedVertices = nullptr;
	Vertex *							mLockedVerticesEnd = nullptr;
	AABox								mLockedPrimitiveBounds;

	/// A single text string
	struct Text
	{
										Text(Vec3Arg inPosition, const string_view &inText, ColorArg inColor, float inHeight) : mPosition(inPosition), mText(inText), mColor(inColor), mHeight(inHeight) { }

		Vec3							mPosition;
		String							mText;
		Color							mColor;
		float							mHeight;
	};

	/// All text strings that are to be drawn on screen
	Array<Text>							mTexts;
	Mutex								mTextsLock;

	/// Font with which to draw the texts
	RefConst<Font>						mFont;

	/// A single line segment
	struct Line
	{
		Float3							mFrom;
		Color							mFromColor;
		Float3							mTo;
		Color							mToColor;
	};

	/// The list of line segments
	Array<Line>							mLines;
	Mutex								mLinesLock;

	/// The shaders for the line segments
	unique_ptr<PipelineState>			mLineState;
};
