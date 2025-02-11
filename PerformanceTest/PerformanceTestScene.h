// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Base class for a test scene to test performance
class PerformanceTestScene
{
public:
	// Virtual destructor
	virtual					~PerformanceTestScene()								{ }

	// Get name of test for debug purposes
	virtual const char *	GetName() const = 0;

	// Get the number of MB that the temp allocator should preallocate
	virtual size_t			GetTempAllocatorSizeMB() const						{ return 32; }

	// Get the max number of bodies to support in the physics system
	virtual uint			GetMaxBodies() const								{ return 10240; }

	// Get the max number of body pairs to support in the physics system
	virtual uint			GetMaxBodyPairs() const								{ return 65536; }

	// Get the max number of contact constraints to support in the physics system
	virtual uint			GetMaxContactConstraints() const					{ return 20480; }

	// Load assets for the scene
	virtual bool			Load([[maybe_unused]] const String &inAssetPath)	{ return true; }

	// Start a new test by adding objects to inPhysicsSystem
	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) = 0;

	// Step the test
	virtual void			UpdateTest([[maybe_unused]] PhysicsSystem &inPhysicsSystem, [[maybe_unused]] TempAllocator &ioTempAllocator, [[maybe_unused]] float inDeltaTime) { }

	// Update the hash with the state of the scene
	virtual void			UpdateHash([[maybe_unused]] uint64 &ioHash) const	{ }

	// Stop a test and remove objects from inPhysicsSystem
	virtual void			StopTest(PhysicsSystem &inPhysicsSystem)			{ }
};
