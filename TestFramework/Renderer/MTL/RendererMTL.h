// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Renderer.h>
#include <Renderer/MTL/TextureMTL.h>

#include <MetalKit/MetalKit.h>

/// Metal renderer
class RendererMTL : public Renderer
{
public:
	virtual 						~RendererMTL() override;
	
	// See: Renderer
	virtual void					Initialize(ApplicationWindow *inWindow) override;
	virtual bool					BeginFrame(const CameraState &inCamera, float inWorldScale) override;
	virtual void					EndShadowPass() override;
	virtual void					EndFrame() override;
	virtual void					SetProjectionMode() override;
	virtual void					SetOrthoMode() override;
	virtual Ref<Texture>			CreateTexture(const Surface *inSurface) override;
	virtual Ref<VertexShader>		CreateVertexShader(const char *inName) override;
	virtual Ref<PixelShader>		CreatePixelShader(const char *inName) override;
	virtual unique_ptr<PipelineState> CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode) override;
	virtual RenderPrimitive *		CreateRenderPrimitive(PipelineState::ETopology inType) override;
	virtual RenderInstances *		CreateRenderInstances() override;
	virtual Texture *				GetShadowMap() const override									{ return mShadowMap; }
	virtual void					OnWindowResize() override										{ }

	MTKView *						GetView() const													{ return mView; }
	id<MTLDevice>					GetDevice() const												{ return mView.device; }
	id<MTLRenderCommandEncoder>		GetRenderEncoder() const										{ return mRenderEncoder; }

private:
	MTKView *						mView;
	MTLRenderPassDescriptor *		mShadowRenderPass;
	Ref<TextureMTL>					mShadowMap;
	id<MTLLibrary>					mShaderLibrary;
	id<MTLCommandQueue>				mCommandQueue;
	id<MTLCommandBuffer> 			mCommandBuffer;
	id<MTLRenderCommandEncoder>		mRenderEncoder;
};
