// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/RenderPrimitive.h>
#include <Renderer/VK/RendererVK.h>

/// Vulkan implementation of a render primitive
class RenderPrimitiveVK : public RenderPrimitive
{
public:
	/// Constructor
							RenderPrimitiveVK(RendererVK *inRenderer)										: mRenderer(inRenderer) { }
	virtual					~RenderPrimitiveVK() override													{ Clear(); }

	/// Vertex buffer management functions
	virtual void			CreateVertexBuffer(int inNumVtx, int inVtxSize, const void *inData = nullptr) override;
	virtual void			ReleaseVertexBuffer() override;
	virtual void *			LockVertexBuffer() override;
	virtual void			UnlockVertexBuffer() override;

	/// Index buffer management functions
	virtual void			CreateIndexBuffer(int inNumIdx, const uint32 *inData = nullptr) override;
	virtual void			ReleaseIndexBuffer() override;
	virtual uint32 *		LockIndexBuffer() override;
	virtual void			UnlockIndexBuffer() override;

	/// Draw the primitive
	virtual void			Draw() const override;

private:
	friend class RenderInstancesVK;

	RendererVK *			mRenderer;

	VkBuffer				mVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory			mVertexBufferMemory = VK_NULL_HANDLE;
	bool					mVertexBufferDeviceLocal = false;

	VkBuffer				mIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory			mIndexBufferMemory = VK_NULL_HANDLE;
	bool					mIndexBufferDeviceLocal = false;
};