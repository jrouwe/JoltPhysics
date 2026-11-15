// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowApple.h>

#import <MetalKit/MetalKit.h>

ApplicationWindowApple::~ApplicationWindowApple()
{
	[mMetalView release];
}

void ApplicationWindowApple::SetMetalView(MTKView *inView)
{
	inView.clearColor = MTLClearColorMake(0.098f, 0.098f, 0.439f, 1.000f);
	inView.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
	inView.clearDepth = 0.0f;
	mMetalView = inView;
}

CAMetalLayer *ApplicationWindowApple::GetMetalLayer() const
{
	return (CAMetalLayer *)mMetalView.layer;
}
