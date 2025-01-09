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
	mView = static_cast<ApplicationWindowMacOS *>(inWindow)->GetMetalView();

	// Create depth only texture (no color buffer, as seen from light)
	mShadowMap = new TextureMTL(this, cShadowMapSize, cShadowMapSize);

	Renderer::Initialize(inWindow);
}

void RendererMTL::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	JPH_PROFILE_FUNCTION();

	Renderer::BeginFrame(inCamera, inWorldScale);
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
