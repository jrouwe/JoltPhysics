// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#else
	// Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
	#define JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
	#undef JPH_DEBUG_RENDERER
#endif

#include <Renderer/RenderPrimitive.h>
#include <Renderer/RenderInstances.h>
#include <Jolt/Core/Mutex.h>
#include <unordered_map>

class Renderer;
class Font;

/// Implementation of DebugRenderer
class DebugRendererImp final : public DebugRenderer
{
public:
	/// Constructor
										DebugRendererImp(Renderer *inRenderer, const Font *inFont);

	/// Implementation of DebugRenderer interface
	virtual void						DrawLine(const Float3 &inFrom, const Float3 &inTo, ColorArg inColor) override;
	virtual void						DrawTriangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor) override;
	virtual Batch						CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) override;
	virtual Batch						CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) override;
	virtual void						DrawGeometry(Mat44Arg inModelMatrix, const AABox &inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
	virtual void						DrawText3D(Vec3Arg inPosition, const string_view &inString, ColorArg inColor, float inHeight) override;
	
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

	/// Implementation specific batch object
	class BatchImpl : public RefTargetVirtual, public RenderPrimitive
	{
	public:
										BatchImpl(Renderer *inRenderer, D3D_PRIMITIVE_TOPOLOGY inType) : RenderPrimitive(inRenderer, inType) { }

		virtual void					AddRef() override			{ RenderPrimitive::AddRef(); }
		virtual void					Release() override			{ if (--mRefCount == 0) delete this; }
	};

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

	/// The shadow buffer (depth buffer rendered from the light)
	Ref<Texture>						mDepthTexture;

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
		vector<InstanceWithLODInfo>		mInstances;

		/// Start index in mInstancesBuffer for each of the LOD in the geometry pass. Length is one longer than the number of LODs to indicate how many instances the last lod has.
		vector<int>						mGeometryStartIdx;

		/// Start index in mInstancesBuffer for each of the LOD in the light pass. Length is one longer than the number of LODs to indicate how many instances the last lod has.
		vector<int>						mLightStartIdx;
	};

	using InstanceMap = unordered_map<GeometryRef, Instances>;

	/// Clear map of instances and make it ready for the next frame
	void								ClearMap(InstanceMap &ioInstances);

	/// Helper function to draw instances
	inline void							DrawInstances(const Geometry *inGeometry, const vector<int> &inStartIdx);

	/// List of primitives that are finished and ready for drawing
	InstanceMap							mWireframePrimitives;
	InstanceMap							mPrimitives;
	InstanceMap							mTempPrimitives;
	InstanceMap							mPrimitivesBackFacing;
	int									mNumInstances = 0;
	Ref<RenderInstances>				mInstancesBuffer[Renderer::cFrameCount];

	/// Primitive that is being built + its properties
	Batch								mLockedPrimitive;
	Vertex *							mLockedVerticesStart = nullptr;
	Vertex *							mLockedVertices = nullptr;
	Vertex *							mLockedVerticesEnd = nullptr;
	AABox								mLockedPrimitiveBounds;

	/// A single text string
	struct Text
	{
										Text(Vec3Arg inPosition, const string_view &inText, ColorArg inColor, float inHeight) : mPosition(inPosition), mText(inText), mColor(inColor), mHeight(inHeight) { }

		Vec3							mPosition;
		string							mText;
		Color							mColor;
		float							mHeight;
	};

	/// All text strings that are to be drawn on screen
	vector<Text>						mTexts;
	Mutex								mTextsLock;

	/// Font with which to draw the texts
	RefConst<Font>						mFont;

	/// A single line segment
	struct Line
	{
										Line(const Float3 &inFrom, const Float3 &inTo, ColorArg inColor) : mFrom(inFrom), mFromColor(inColor), mTo(inTo), mToColor(inColor) { }

		Float3							mFrom;
		Color							mFromColor;
		Float3							mTo;
		Color							mToColor;
	};

	/// The list of line segments
	vector<Line>						mLines;
	Mutex								mLinesLock;

	/// The shaders for the line segments
	unique_ptr<PipelineState>			mLineState;
};
