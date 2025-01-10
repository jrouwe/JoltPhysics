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
#include <Jolt/Core/Profiler.h>

RendererMTL::~RendererMTL()
{
	mShadowMap = nullptr;
}

void RendererMTL::Initialize(ApplicationWindow *inWindow)
{
	Renderer::Initialize(inWindow);

	mView = static_cast<ApplicationWindowMacOS *>(inWindow)->GetMetalView();

	id<MTLDevice> device = GetDevice();

	// Load the shader library containing all shaders for the test framework
	NSError *error = nullptr;
	NSURL *url = [NSURL URLWithString: @"Assets/Shaders/MTL/Shaders.metallib"];
	mShaderLibrary = [device newLibraryWithURL: url error: &error];
	FatalErrorIfFailed(error);

	// Create depth only texture (no color buffer, as seen from light)
	mShadowMap = new TextureMTL(this, cShadowMapSize, cShadowMapSize);

	Ref<VertexShader> vtx = CreateVertexShader("vertexShader");
	Ref<PixelShader> pix = CreatePixelShader("fragmentShader");
	mPipelineState = CreatePipelineState(vtx, nullptr, 0, pix, PipelineState::EDrawPass::Normal, PipelineState::EFillMode::Solid, PipelineState::ETopology::Triangle, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::FrontFace);

	// Create the command queue
	mCommandQueue = [device newCommandQueue];
}

void RendererMTL::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	JPH_PROFILE_FUNCTION();

	Renderer::BeginFrame(inCamera, inWorldScale);

	// Create a new command buffer
	mCommandBuffer = [mCommandQueue commandBuffer];

	// Obtain a render_pass_descriptor generated from the view's drawable textures.
	MTLRenderPassDescriptor *render_pass_descriptor = mView.currentRenderPassDescriptor;
	if (render_pass_descriptor == nullptr)
	{
		mRenderEncoder = nil;
		return;
	}

	// Create render encoder
	mRenderEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor: render_pass_descriptor];

	// Set viewport
	[mRenderEncoder setViewport: (MTLViewport){ 0.0, 0.0, double(mWindow->GetWindowWidth()), double(mWindow->GetWindowHeight()), 0.0, 1.0 }];
}

void RendererMTL::EndShadowPass()
{
}

void RendererMTL::EndFrame()
{
	JPH_PROFILE_FUNCTION();

	typedef struct
	{
		simd::float2 position;
		simd::float4 color;
	} Vertex;

	static const Vertex vertices[] =
	{
		// 2D positions,    RGBA colors
		{ {  0.5,  -0.5 }, { 1, 0, 0, 1 } },
		{ { -0.5,  -0.5 }, { 0, 1, 0, 1 } },
		{ {    0,   0.5 }, { 0, 0, 1, 1 } },
	};

	mPipelineState->Activate();

	// Pass in the parameter data.
	[mRenderEncoder setVertexBytes:vertices
						   length:sizeof(vertices)
						  atIndex:0];

	// Draw the triangle.
	[mRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
					  vertexStart:0
					  vertexCount:3];

	// Finish the encoder
	[mRenderEncoder endEncoding];

	// Schedule a present
	[mCommandBuffer presentDrawable: mView.currentDrawable];

	// Commit the command buffer
	[mCommandBuffer commit];

	Renderer::EndFrame();
}

void RendererMTL::SetProjectionMode()
{
	JPH_ASSERT(mInFrame);
}

void RendererMTL::SetOrthoMode()
{
	JPH_ASSERT(mInFrame);
}

Ref<Texture> RendererMTL::CreateTexture(const Surface *inSurface)
{
	return new TextureMTL(this, inSurface);
}

Ref<VertexShader> RendererMTL::CreateVertexShader(const char *inName)
{
	return new VertexShaderMTL([mShaderLibrary newFunctionWithName: [[NSString alloc] initWithUTF8String: inName]]);
}

Ref<PixelShader> RendererMTL::CreatePixelShader(const char *inName)
{
	return new PixelShaderMTL([mShaderLibrary newFunctionWithName: [[NSString alloc] initWithUTF8String: inName]]);
}

unique_ptr<PipelineState> RendererMTL::CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode)
{
	return make_unique<PipelineStateMTL>(this, static_cast<const VertexShaderMTL *>(inVertexShader), inInputDescription, inInputDescriptionCount, static_cast<const PixelShaderMTL *>(inPixelShader), inDrawPass, inFillMode, inTopology, inDepthTest, inBlendMode, inCullMode);
}

RenderPrimitive *RendererMTL::CreateRenderPrimitive(PipelineState::ETopology inType)
{
	return new RenderPrimitiveMTL(this);
}

RenderInstances *RendererMTL::CreateRenderInstances()
{
	return new RenderInstancesMTL(this);
}

void RendererMTL::OnWindowResize()
{
}

#ifndef JPH_USE_VULKAN
Renderer *Renderer::sCreate()
{
	return new RendererMTL;
}
#endif
