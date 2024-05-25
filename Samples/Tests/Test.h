// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/PhysicsSystem.h>
#include <Renderer/Renderer.h>
#include <Input/Keyboard.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Jolt/Core/RTTI.h>

class DebugUI;
class UIElement;
namespace JPH {
	class StateRecorder;
	class JobSystem;
	class ContactListener;
	class DebugRenderer;
}

class Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL_BASE(JPH_NO_EXPORT, Test)

	// Destructor
	virtual			~Test() = default;

	// Set the physics system
	virtual void	SetPhysicsSystem(PhysicsSystem *inPhysicsSystem)			{ mPhysicsSystem = inPhysicsSystem; mBodyInterface = &inPhysicsSystem->GetBodyInterface(); }

	// Set the job system
	void			SetJobSystem(JobSystem *inJobSystem)						{ mJobSystem = inJobSystem; }

	// Set the debug renderer
	void			SetDebugRenderer(DebugRenderer *inDebugRenderer)			{ mDebugRenderer = inDebugRenderer; }

	// Set the temp allocator
	void			SetTempAllocator(TempAllocator *inTempAllocator)			{ mTempAllocator = inTempAllocator; }

	// Initialize the test
	virtual void	Initialize()												{ }

	// Number used to scale the terrain and camera movement to the scene
	virtual float	GetWorldScale() const										{ return 1.0f; }

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener()								{ return nullptr; }

	class ProcessInputParams
	{
	public:
		float								mDeltaTime;
		Keyboard *							mKeyboard;
		CameraState							mCameraState;
	};

	// Process input, this is called before SaveInputState is called. This allows you to determine the player input and adjust internal state accordingly.
	// This state should not be applied until PrePhysicsUpdate because on replay you will receive a call to RestoreInputState to restore the stored player input state before receiving another PrePhysicsUpdate.
	virtual void	ProcessInput(const ProcessInputParams &inParams)			{ }

	class PreUpdateParams
	{
	public:
		float								mDeltaTime;
		CameraState							mCameraState;
#ifdef JPH_DEBUG_RENDERER
		const SkeletonPose::DrawSettings *	mPoseDrawSettings;
#endif // JPH_DEBUG_RENDERER
	};

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams)			{ }

	// Update the test, called after the physics update
	virtual void	PostPhysicsUpdate(float inDeltaTime)						{ }

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void	GetInitialCamera(CameraState &ioState) const				{ }

	// Override to specify a camera pivot point and orientation (world space)
	virtual RMat44	GetCameraPivot(float inCameraHeading, float inCameraPitch) const { return RMat44::sIdentity(); }

	// Offset around which to center drawing. This floating point accuracy issues when the camera is far from the origin.
	virtual RVec3	GetDrawOffset() const										{ return RVec3::sZero(); }

	// Optional settings menu
	virtual bool	HasSettingsMenu() const										{ return false; }
	virtual void	CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)		{ }

	// Force the application to restart the test
	void			RestartTest()												{ mNeedsRestart = true; }
	bool			NeedsRestart() const										{ return mNeedsRestart; }

	// If this test is supposed to be deterministic
	virtual bool	IsDeterministic() const										{ return true; }

	// Saving / restoring state for replay
	virtual void	SaveState(StateRecorder &inStream) const					{ }
	virtual void	RestoreState(StateRecorder &inStream)						{ }

	// Saving / restoring controller input state for replay
	virtual void	SaveInputState(StateRecorder &inStream) const				{ }
	virtual void	RestoreInputState(StateRecorder &inStream)					{ }

	// Return a string that is displayed in the top left corner of the screen
	virtual String	GetStatusString() const										{ return String(); }

protected:
	// Utility function to create a static floor body
	Body &			CreateFloor(float inSize = 200.0f);

	// Utility function to create a floor consisting of very large triangles
	Body &			CreateLargeTriangleFloor();

	// Create an uneven terrain floor body
	Body &			CreateMeshTerrain();
	Body &			CreateHeightFieldTerrain();

	JobSystem *		mJobSystem = nullptr;
	PhysicsSystem *	mPhysicsSystem = nullptr;
	BodyInterface *	mBodyInterface = nullptr;
	DebugRenderer *	mDebugRenderer = nullptr;
	TempAllocator *	mTempAllocator = nullptr;

private:
	bool			mNeedsRestart = false;
};
