// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class MultithreadedTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, MultithreadedTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "This test spawns boxes and ragdolls and performs ray cast tests from threads / jobs to see if the simulation is thread safe.";
	}

	// Destructor
	virtual					~MultithreadedTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	// Initialization
	virtual void			Initialize() override;

	// Test will never be deterministic since various threads are trying to concurrently add / remove bodies
	virtual bool			IsDeterministic() const override							{ return false; }

private:
	// Execute a lambda either on this thread or in a separate job
	void					Execute(default_random_engine &ioRandom, const char *inName, function<void()> inFunction);

	// Thread main function that spawns boxes
	void					BoxSpawner();

	// Thread main function that spawns ragdolls
	void					RagdollSpawner();

	// Thread main function that casts rays
	void					CasterMain();

	thread					mBoxSpawnerThread;
	thread					mRagdollSpawnerThread;
	thread					mCasterThread;
	atomic<bool>			mIsQuitting = false;
};
