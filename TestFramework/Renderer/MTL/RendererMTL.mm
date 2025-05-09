// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/RendererMTL.h>
#include <Renderer/MTL/RenderPrimitiveMTL.h>
#include <Renderer/MTL/RenderInstancesMTL.h>
#include <Renderer/MTL/PipelineStateMTL.h>
#include <Renderer/MTL/VertexShaderMTL.h>
#include <Renderer/MTL/PixelShaderMTL.h>
#include <Renderer/MTL/TextureMTL.h>
#include <Renderer/MTL/FatalErrorIfFailedMTL.h>
#include <Window/ApplicationWindowMacOS.h>
#include <Utils/Log.h>
#include <Utils/AssetStream.h>
#include <Jolt/Core/Profiler.h>

RendererMTL::~RendererMTL()
{
	[mCommandQueue release];
	[mShadowRenderPass release];
	[mShaderLibrary release];
}

void RendererMTL::Initialize(ApplicationWindow *inWindow)
{
	Renderer::Initialize(inWindow);

	mView = static_cast<ApplicationWindowMacOS *>(inWindow)->GetMetalView();

	id<MTLDevice> device = GetDevice();

	// Load the shader library containing all shaders for the test framework
	NSError *error = nullptr;
	NSURL *url = [NSURL URLWithString: [NSString stringWithCString: (AssetStream::sGetAssetsBasePath() + "Shaders/MTL/Shaders.metallib").c_str() encoding: NSUTF8StringEncoding]];
	mShaderLibrary = [device newLibraryWithURL: url error: &error];
	FatalErrorIfFailed(error);

	// Create depth only texture (no color buffer, as seen from light)
	mShadowMap = new TextureMTL(this, cShadowMapSize, cShadowMapSize);

	// Create render pass descriptor for shadow pass
	mShadowRenderPass = [[MTLRenderPassDescriptor alloc] init];
	mShadowRenderPass.depthAttachment.texture = mShadowMap->GetTexture();
	mShadowRenderPass.depthAttachment.loadAction = MTLLoadActionClear;
	mShadowRenderPass.depthAttachment.storeAction = MTLStoreActionStore;
	mShadowRenderPass.depthAttachment.clearDepth = 0.0f;

	// Create the command queue
	mCommandQueue = [device newCommandQueue];
}

bool RendererMTL::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	JPH_PROFILE_FUNCTION();

	Renderer::BeginFrame(inCamera, inWorldScale);

	// Update frame index
	mFrameIndex = (mFrameIndex + 1) % cFrameCount;

	// Create a new command buffer
	mCommandBuffer = [mCommandQueue commandBuffer];

	// Create shadow render encoder
	mRenderEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor: mShadowRenderPass];

	// Set viewport to size of shadow map
	[mRenderEncoder setViewport: (MTLViewport){ 0.0, 0.0, double(cShadowMapSize), double(cShadowMapSize), 0.0, 1.0 }];

	// Set pixel shader constants
	[mRenderEncoder setFragmentBytes: &mPSBuffer length: sizeof(mPSBuffer) atIndex: 0];

	// Counter clockwise is default winding order
	[mRenderEncoder setFrontFacingWinding: MTLWindingCounterClockwise];

	// Start with projection mode
	SetProjectionMode();

	return true;
}

void RendererMTL::EndShadowPass()
{
	// Finish the shadow encoder
	[mRenderEncoder endEncoding];
	mRenderEncoder = nil;

	// Get the descriptor for the main window
	MTLRenderPassDescriptor *render_pass_descriptor = mView.currentRenderPassDescriptor;
	if (render_pass_descriptor == nullptr)
		return;

	// Create render encoder
	mRenderEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor: render_pass_descriptor];

	// Set viewport
	[mRenderEncoder setViewport: (MTLViewport){ 0.0, 0.0, double(mWindow->GetWindowWidth()), double(mWindow->GetWindowHeight()), 0.0, 1.0 }];

	// Set pixel shader constants
	[mRenderEncoder setFragmentBytes: &mPSBuffer length: sizeof(mPSBuffer) atIndex: 0];

	// Counter clockwise is default winding order
	[mRenderEncoder setFrontFacingWinding: MTLWindingCounterClockwise];

	// Start with projection mode
	SetProjectionMode();
}

void RendererMTL::EndFrame()
{
	JPH_PROFILE_FUNCTION();

	// Finish the encoder
	[mRenderEncoder endEncoding];
	mRenderEncoder = nil;

	// Schedule a present
	[mCommandBuffer presentDrawable: mView.currentDrawable];

	// Commit the command buffer
	[mCommandBuffer commit];

	Renderer::EndFrame();
}

void RendererMTL::SetProjectionMode()
{
	JPH_ASSERT(mInFrame);

	[mRenderEncoder setVertexBytes: &mVSBuffer length: sizeof(mVSBuffer) atIndex: 2];
}

void RendererMTL::SetOrthoMode()
{
	JPH_ASSERT(mInFrame);

	[mRenderEncoder setVertexBytes: &mVSBufferOrtho length: sizeof(mVSBufferOrtho) atIndex: 2];
}

Ref<Texture> RendererMTL::CreateTexture(const Surface *inSurface)
{
	return new TextureMTL(this, inSurface);
}

Ref<VertexShader> RendererMTL::CreateVertexShader(const char *inName)
{
	id<MTLFunction> function = [mShaderLibrary newFunctionWithName: [NSString stringWithCString: inName encoding: NSUTF8StringEncoding]];
	if (function == nil)
		FatalError("Vertex shader %s not found", inName);
	return new VertexShaderMTL(function);
}

Ref<PixelShader> RendererMTL::CreatePixelShader(const char *inName)
{
	id<MTLFunction> function = [mShaderLibrary newFunctionWithName: [NSString stringWithCString: inName encoding: NSUTF8StringEncoding]];
	if (function == nil)
		FatalError("Pixel shader %s not found", inName);
	return new PixelShaderMTL(function);
}

unique_ptr<PipelineState> RendererMTL::CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode)
{
	return make_unique<PipelineStateMTL>(this, static_cast<const VertexShaderMTL *>(inVertexShader), inInputDescription, inInputDescriptionCount, static_cast<const PixelShaderMTL *>(inPixelShader), inDrawPass, inFillMode, inTopology, inDepthTest, inBlendMode, inCullMode);
}

RenderPrimitive *RendererMTL::CreateRenderPrimitive(PipelineState::ETopology inType)
{
	return new RenderPrimitiveMTL(this, inType == PipelineState::ETopology::Line? MTLPrimitiveTypeLine : MTLPrimitiveTypeTriangle);
}

RenderInstances *RendererMTL::CreateRenderInstances()
{
	return new RenderInstancesMTL(this);
}

#ifndef JPH_ENABLE_VULKAN
Renderer *Renderer::sCreate()
{
	return new RendererMTL;
}
#endif
