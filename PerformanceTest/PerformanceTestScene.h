// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Base class for a test scene to test performance
class PerformanceTestScene
{
public:
	// Virtual destructor
	virtual					~PerformanceTestScene()							{ }

	// Get name of test for debug purposes
	virtual const char *	GetName() const = 0;

	// Load assets for the scene
	virtual bool			Load()											{ return true; }

	// Start a new test by adding objects to inPhysicsSystem
	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) = 0;

	// Stop a test and remove objects from inPhysicsSystem
	virtual void			StopTest(PhysicsSystem &inPhysicsSystem)		{ }
};
