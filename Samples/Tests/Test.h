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
	JPH_DECLARE_RTTI_VIRTUAL_BASE(Test)

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

	class PreUpdateParams
	{
	public:
		float								mDeltaTime;
		Keyboard *							mKeyboard;
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
	virtual Mat44	GetCameraPivot(float inCameraHeading, float inCameraPitch) const { return Mat44::sIdentity(); }

	// Optional settings menu
	virtual bool	HasSettingsMenu() const										{ return false; }
	virtual void	CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)		{ }

	// Force the application to restart the test
	virtual void	RestartTest()												{ mNeedsRestart = true; }
	virtual bool	NeedsRestart() const										{ return mNeedsRestart; }

	// If this test is supposed to be deterministic
	virtual bool	IsDeterministic() const										{ return true; }

	// Saving / restoring state for replay
	virtual void	SaveState(StateRecorder &inStream) const					{ }
	virtual void	RestoreState(StateRecorder &inStream)						{ }

protected:
	// Utility function to create a static floor body
	Body &			CreateFloor();

	// Utiltity function to create a floor consisting of very large triangles
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
