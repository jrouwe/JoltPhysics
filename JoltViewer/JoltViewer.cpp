// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <JoltViewer.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Application/EntryPoint.h>
#include <Renderer/DebugRendererImp.h>
#include <UI/UIManager.h>
#include <Application/DebugUI.h>
#include <fstream>

#ifndef JPH_DEBUG_RENDERER	
	// Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
	#define JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRendererRecorder.cpp>
	#include <Jolt/Renderer/DebugRendererPlayback.cpp>
	#undef JPH_DEBUG_RENDERER
#endif

JoltViewer::JoltViewer()
{
	// Get file name from commandline
	string cmd_line = GetCommandLineA();
	vector<string> args;
	StringToVector(cmd_line, args, " ");
	
	// Check arguments
	if (args.size() != 2 || args[1].empty())
	{
		MessageBoxA(nullptr, "Usage: JoltViewer <recording filename>", "Error", MB_OK);
		return;
	}

	// Open file
	ifstream stream(args[1], ifstream::in | ifstream::binary);
	if (!stream.is_open())
	{
		MessageBoxA(nullptr, "Could not open recording file", "Error", MB_OK);
		return;
	}

	// Parse the stream
	StreamInWrapper wrapper(stream);
	mRendererPlayback.Parse(wrapper);
	if (mRendererPlayback.GetNumFrames() == 0)
	{
		MessageBoxA(nullptr, "Recording file did not contain any frames", "Error", MB_OK);
		return;
	}

	// Draw the first frame
	mRendererPlayback.DrawFrame(0);

	// Start paused
	Pause(true);

	// Create UI
	UIElement *main_menu = mDebugUI->CreateMenu();
	mDebugUI->CreateTextButton(main_menu, "Help", [this](){
		UIElement *help = mDebugUI->CreateMenu();
		mDebugUI->CreateStaticText(help,
			"ESC: Back to previous menu.\n"
			"WASD + Mouse: Fly around. Hold Shift to speed up, Ctrl to slow down.\n"
			"P: Pause / unpause simulation.\n"
			"O: Single step simulation.\n"
			",: Step back.\n"
			".: Step forward.\n"
			"Shift + ,: Play reverse.\n"
			"Shift + .: Replay forward."
		);
		mDebugUI->ShowMenu(help);
	});
	mDebugUI->ShowMenu(main_menu);
}

bool JoltViewer::RenderFrame(float inDeltaTime)
{
	// If no frames were read, abort
	if (mRendererPlayback.GetNumFrames() == 0)
		return false;

	// Handle keyboard input
	bool shift = mKeyboard->IsKeyPressed(DIK_LSHIFT) || mKeyboard->IsKeyPressed(DIK_RSHIFT);
	for (int key = mKeyboard->GetFirstKey(); key != 0; key = mKeyboard->GetNextKey())
		switch (key)
		{
		case DIK_R:
			// Restart
			mCurrentFrame = 0;
			mPlaybackMode = EPlaybackMode::Play;
			Pause(true);
			break;

		case DIK_O:
			// Step
			mPlaybackMode = EPlaybackMode::Play;
			SingleStep();
			break;

		case DIK_COMMA:
			// Back
			mPlaybackMode = shift? EPlaybackMode::Rewind : EPlaybackMode::StepBack;
			Pause(false);
			break;

		case DIK_PERIOD:
			// Forward
			mPlaybackMode = shift? EPlaybackMode::Play : EPlaybackMode::StepForward;
			Pause(false);
			break;
		}

	// If paused, do nothing
	if (inDeltaTime > 0.0f)
	{
		// Determine new frame number
		switch (mPlaybackMode)
		{
		case EPlaybackMode::StepForward:
			mPlaybackMode = EPlaybackMode::Stop;
			[[fallthrough]];

		case EPlaybackMode::Play:
			if (mCurrentFrame + 1 < mRendererPlayback.GetNumFrames())
				++mCurrentFrame;
			break;

		case EPlaybackMode::StepBack:
			mPlaybackMode = EPlaybackMode::Stop;
			[[fallthrough]];

		case EPlaybackMode::Rewind:
			if (mCurrentFrame > 0)
				--mCurrentFrame;
			break;

		case EPlaybackMode::Stop:
			break;
		}

		// Render the frame
		mRendererPlayback.DrawFrame(mCurrentFrame);
	}

	return true;
}

ENTRY_POINT(JoltViewer)
