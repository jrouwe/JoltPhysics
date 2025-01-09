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

	// Create depth only texture (no color buffer, as seen from light)
	mShadowMap = new TextureMTL(this, cShadowMapSize, cShadowMapSize);

	NSError *error = nullptr;

	NSURL *url = [NSURL URLWithString: @"Assets/Shaders/MTL/Shaders.metallib"];
	id<MTLLibrary> defaultLibrary = [device newLibraryWithURL: url error: &error];
	FatalErrorIfFailed(error);

	id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
	id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];

	// Configure a pipeline descriptor that is used to create a pipeline state.
	MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	pipelineStateDescriptor.vertexFunction = vertexFunction;
	pipelineStateDescriptor.fragmentFunction = fragmentFunction;
	pipelineStateDescriptor.colorAttachments[0].pixelFormat = mView.colorPixelFormat;

	mPipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
	FatalErrorIfFailed(error);

	// Create the command queue
	mCommandQueue = [device newCommandQueue];
}

void RendererMTL::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	JPH_PROFILE_FUNCTION();

	Renderer::BeginFrame(inCamera, inWorldScale);

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

	// Create a new command buffer for each render pass to the current drawable.
	id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];

	// Obtain a renderPassDescriptor generated from the view's drawable textures.
	MTLRenderPassDescriptor *renderPassDescriptor = mView.currentRenderPassDescriptor;

	if(renderPassDescriptor != nil)
	{
		// Create a render command encoder.
		id<MTLRenderCommandEncoder> renderEncoder =
		[commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

		// Set the region of the drawable to draw into.
		[renderEncoder setViewport:(MTLViewport){0.0, 0.0, double(mWindow->GetWindowWidth()), double(mWindow->GetWindowHeight()), 0.0, 1.0 }];

		[renderEncoder setRenderPipelineState:mPipelineState];

		// Pass in the parameter data.
		[renderEncoder setVertexBytes:vertices
							   length:sizeof(vertices)
							  atIndex:0];

		// Draw the triangle.
		[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
						  vertexStart:0
						  vertexCount:3];

		[renderEncoder endEncoding];

		// Schedule a present once the framebuffer is complete using the current drawable.
		[commandBuffer presentDrawable:mView.currentDrawable];
	}

	// Finalize rendering here & push the command buffer to the GPU.
	[commandBuffer commit];
}

void RendererMTL::EndShadowPass()
{
}

void RendererMTL::EndFrame()
{
	JPH_PROFILE_FUNCTION();

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
	return new VertexShaderMTL();
}

Ref<PixelShader> RendererMTL::CreatePixelShader(const char *inName)
{
	return new PixelShaderMTL();
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
