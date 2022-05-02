// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifndef JPH_DEBUG_RENDERER
	#error This file should only be included when JPH_DEBUG_RENDERER is defined
#endif // !JPH_DEBUG_RENDERER

#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Core/Mutex.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <map>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

/// Implementation of DebugRenderer that records the API invocations to be played back later
class DebugRendererRecorder final : public DebugRenderer
{
public:
	/// Constructor
										DebugRendererRecorder(StreamOut &inStream) : mStream(inStream) { Initialize(); }

	/// Implementation of DebugRenderer interface
	virtual void						DrawLine(const Float3 &inFrom, const Float3 &inTo, ColorArg inColor) override;
	virtual void						DrawTriangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor) override;
	virtual Batch						CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) override;
	virtual Batch						CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) override;
	virtual void						DrawGeometry(Mat44Arg inModelMatrix, const AABox &inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
	virtual void						DrawText3D(Vec3Arg inPosition, const string_view &inString, ColorArg inColor, float inHeight) override;
	
	/// Mark the end of a frame
	void								EndFrame();

	/// Control commands written into the stream
	enum class ECommand : uint8
	{
		CreateBatch,
		CreateBatchIndexed,
		CreateGeometry,
		EndFrame
	};

	/// Holds a single line segment
	struct LineBlob
	{
		Float3							mFrom;
		Float3							mTo;
		Color							mColor;
	};
	
	/// Holds a single triangle
	struct TriangleBlob
	{
		Vec3							mV1;
		Vec3							mV2;
		Vec3							mV3;
		Color							mColor;
	};

	/// Holds a single text entry
	struct TextBlob
	{
										TextBlob() = default;
										TextBlob(Vec3Arg inPosition, const string_view &inString, const Color &inColor, float inHeight) : mPosition(inPosition), mString(inString), mColor(inColor), mHeight(inHeight) { }

		Vec3							mPosition;
		string							mString;
		Color							mColor;
		float							mHeight;
	};

	/// Holds a single geometry draw call
	struct GeometryBlob
	{
		Mat44							mModelMatrix;
		Color							mModelColor;
		uint32							mGeometryID;
		ECullMode						mCullMode;
		ECastShadow						mCastShadow;
		EDrawMode						mDrawMode;
	};

	/// All information for a single frame
	struct Frame
	{
		vector<LineBlob>				mLines;
		vector<TriangleBlob>			mTriangles;
		vector<TextBlob>				mTexts;
		vector<GeometryBlob>			mGeometries;
	};

private:
	/// Implementation specific batch object
	class BatchImpl : public RefTargetVirtual
	{
	public:
										BatchImpl(uint32 inID)		: mID(inID) {  }

		virtual void					AddRef() override			{ ++mRefCount; }
		virtual void					Release() override			{ if (--mRefCount == 0) delete this; }

		atomic<uint32>					mRefCount = 0;
		uint32							mID;
	};

	/// Lock that prevents concurrent access to the internal structures
	Mutex								mMutex;

	/// Stream that recorded data will be sent to
	StreamOut &							mStream;

	/// Next available ID
	uint32								mNextBatchID = 1;
	uint32								mNextGeometryID = 1;

	/// Cached geometries and their IDs
	map<GeometryRef, uint32>			mGeometries;

	/// Data that is being accumulated for the current frame
	Frame								mCurrentFrame;
};

JPH_NAMESPACE_END
