// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Renderer/Renderer.h>
#include <Renderer/Font.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>
#include <Jolt/Core/Reference.h>

class UIManager;
class DebugUI;
namespace JPH {
	class DebugRenderer;
}

class Application
{
private:
	/// Camera state
	CameraState					mLocalCamera;
	CameraState					mWorldCamera;

protected:
	/// Debug renderer module
	DebugRenderer *				mDebugRenderer;

	/// Render module
	Renderer *					mRenderer;

	/// Default font
	RefConst<Font>				mFont;

	/// Input
	Keyboard *					mKeyboard;
	Mouse *						mMouse;

	/// Menu
	UIManager *					mUI;
	DebugUI *					mDebugUI;

	/// A string that is shown on screen to indicate the status of the application
	string						mStatusString;

public:
	/// Constructor
								Application();
	virtual						~Application();

	/// Enter the main loop
	void						Run();

protected:
	/// Callback to render a frame
	virtual bool				RenderFrame(float inDeltaTime)					{ return false; }

	/// Pause / unpause the simulation
	void						Pause(bool inPaused)							{ mIsPaused = inPaused; }

	/// Programmatically single step the simulation
	void						SingleStep()									{ mIsPaused = true; mSingleStep = true; }

	/// Will restore camera position to that returned by GetInitialCamera
	void						ResetCamera();

	/// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void				GetInitialCamera(CameraState &ioState) const	{ }

	/// Override to specify a camera pivot point and orientation (world space)
	virtual Mat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const { return Mat44::sIdentity(); }

	/// Get scale factor for this world, used to boost camera speed and to scale detail of the shadows
	virtual float				GetWorldScale() const							{ return 1.0f; }

	/// Get current state of the camera (world space)
	const CameraState &			GetCamera() const								{ return mWorldCamera; }

	/// Clear debug lines / triangles / texts that have been accumulated
	void						ClearDebugRenderer();

private:
	/// Extract heading and pitch from the local space (relative to the camera pivot) camera forward
	void						GetCameraLocalHeadingAndPitch(float &outHeading, float &outPitch);

	/// Convert local space camera to world space camera
	void						ConvertCameraLocalToWorld(float inCameraHeading, float inCameraPitch);

	/// Update the local and world space camera transform
	void						UpdateCamera(float inDeltaTime);

	/// Draw the frame rate counter
	void						DrawFPS(float inDeltaTime);

	uint64						mLastUpdateTicks;
	bool						mIsPaused = false;
	bool						mSingleStep = false;
	bool						mDebugRendererCleared = true;
	bool						mLeftMousePressed = false;
	float						mFPS = 0.0f;
	float						mTotalDeltaTime = 0.0f;
	int							mNumFrames = 0;
};
