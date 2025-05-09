// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Image/Surface.h>
#include <Window/ApplicationWindow.h>
#include <Renderer/Frustum.h>
#include <Renderer/PipelineState.h>
#include <Renderer/VertexShader.h>
#include <Renderer/PixelShader.h>
#include <Renderer/RenderPrimitive.h>
#include <Renderer/RenderInstances.h>
#include <memory>

// Forward declares
class Texture;

/// Camera setup
struct CameraState
{
									CameraState() : mPos(RVec3::sZero()), mForward(0, 0, -1), mUp(0, 1, 0), mFOVY(DegreesToRadians(70.0f)) { }

	RVec3							mPos;								///< Camera position
	Vec3							mForward;							///< Camera forward vector
	Vec3							mUp;								///< Camera up vector
	float							mFOVY;								///< Field of view in radians in up direction
};

/// Responsible for rendering primitives to the screen
class Renderer
{
public:
	/// Destructor
	virtual							~Renderer();

	/// Initialize renderer
	virtual void					Initialize(ApplicationWindow *inWindow);

	/// Start / end drawing a frame
	virtual bool					BeginFrame(const CameraState &inCamera, float inWorldScale);
	virtual void					EndShadowPass() = 0;
	virtual void					EndFrame();

	/// Switch between orthographic and 3D projection mode
	virtual void					SetProjectionMode() = 0;
	virtual void					SetOrthoMode() = 0;

	/// Create texture from an image surface
	virtual Ref<Texture>			CreateTexture(const Surface *inSurface) = 0;

	/// Compile a vertex shader
	virtual Ref<VertexShader>		CreateVertexShader(const char *inName) = 0;

	/// Compile a pixel shader
	virtual Ref<PixelShader>		CreatePixelShader(const char *inName) = 0;

	/// Create pipeline state object that defines the complete state of how primitives should be rendered
	virtual unique_ptr<PipelineState> CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode) = 0;

	/// Create a render primitive
	virtual RenderPrimitive *		CreateRenderPrimitive(PipelineState::ETopology inType) = 0;

	/// Create render instances object to allow drawing batches of objects
	virtual RenderInstances *		CreateRenderInstances() = 0;

	/// Get the shadow map texture
	virtual Texture *				GetShadowMap() const = 0;

	/// Get the camera state / frustum (only valid between BeginFrame() / EndFrame())
	const CameraState &				GetCameraState() const				{ JPH_ASSERT(mInFrame); return mCameraState; }
	const Frustum &					GetCameraFrustum() const			{ JPH_ASSERT(mInFrame); return mCameraFrustum; }

	/// Offset relative to which the world is rendered, helps avoiding rendering artifacts at big distances
	RVec3							GetBaseOffset() const				{ return mBaseOffset; }
	void							SetBaseOffset(RVec3 inOffset)		{ mBaseOffset = inOffset; }

	/// Get the light frustum (only valid between BeginFrame() / EndFrame())
	const Frustum &					GetLightFrustum() const				{ JPH_ASSERT(mInFrame); return mLightFrustum; }

	/// How many frames our pipeline is
	inline static const uint		cFrameCount = 2;

	/// Size of the shadow map will be cShadowMapSize x cShadowMapSize pixels
	inline static const uint		cShadowMapSize = 4096;

	/// Which frame is currently rendering (to keep track of which buffers are free to overwrite)
	uint							GetCurrentFrameIndex() const		{ JPH_ASSERT(mInFrame); return mFrameIndex; }

	/// Get the window we're rendering to
	ApplicationWindow *				GetWindow() const					{ return mWindow; }

	/// Callback when the window resizes and the back buffer needs to be adjusted
	virtual void					OnWindowResize() = 0;

	/// Create a platform specific Renderer instance
	static Renderer *				sCreate();

protected:
	struct VertexShaderConstantBuffer
	{
		Mat44						mView;
		Mat44						mProjection;
		Mat44						mLightView;
		Mat44						mLightProjection;
	};

	struct PixelShaderConstantBuffer
	{
		Vec4						mCameraPos;
		Vec4						mLightPos;
	};

	ApplicationWindow *				mWindow = nullptr;					///< The window we're rendering to
	float							mPerspectiveYSign = 1.0f;			///< Sign for the Y coordinate in the projection matrix (1 for DX, -1 for Vulkan)
	bool							mInFrame = false;					///< If we're within a BeginFrame() / EndFrame() pair
	CameraState						mCameraState;
	RVec3							mBaseOffset { RVec3::sZero() };		///< Offset to subtract from the camera position to deal with large worlds
	Frustum							mCameraFrustum;
	Frustum							mLightFrustum;
	uint							mFrameIndex = 0;					///< Current frame index (0 or 1)
	VertexShaderConstantBuffer		mVSBuffer;
	VertexShaderConstantBuffer		mVSBufferOrtho;
	PixelShaderConstantBuffer		mPSBuffer;
};
