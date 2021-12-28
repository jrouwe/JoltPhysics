// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Application/Application.h>
#include <Renderer/DebugRendererPlayback.h>

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
