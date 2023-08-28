// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Application/Application.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRendererPlayback.h>
#else
	// Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
	#define JPH_DEBUG_RENDERER
	// Make sure the debug renderer symbols don't get imported or exported
	#define JPH_DEBUG_RENDERER_EXPORT
	#include <Jolt/Renderer/DebugRendererPlayback.h>
	#undef JPH_DEBUG_RENDERER
	#undef JPH_DEBUG_RENDERER_EXPORT
#endif

using namespace std;

// Application that views recordings produced by DebugRendererRecorder
class JoltViewer : public Application
{
public:
	// Constructor / destructor
							JoltViewer();

	// Render the frame
	virtual bool			RenderFrame(float inDeltaTime) override;

private:
	enum class EPlaybackMode
	{
		Rewind,
		StepBack,
		Stop,
		StepForward,
		Play
	};

	DebugRendererPlayback	mRendererPlayback { *mDebugRenderer };

	EPlaybackMode			mPlaybackMode = EPlaybackMode::Play;						// Current playback state. Indicates if we're playing or scrubbing back/forward.
	uint					mCurrentFrame = 0;
};
