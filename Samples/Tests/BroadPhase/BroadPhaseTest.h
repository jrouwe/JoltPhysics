// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>
#include <Layers.h>

// Base class for a test involving only the broad phase
class BroadPhaseTest : public Test
{
public:
	JPH_DECLARE_RTTI_ABSTRACT(BroadPhaseTest)

	// Destructor
	virtual					~BroadPhaseTest() override;

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called after the physics update
	virtual void			PostPhysicsUpdate(float inDeltaTime) override;

protected:
	// Create bodies according to method outlined in "FAST SOFTWARE FOR BOX INTERSECTIONS by AFRA ZOMORODIAN" section "The balanced distribution"
	// http://pub.ist.ac.at/~edels/Papers/2002-J-01-FastBoxIntersection.pdf
	void					CreateBalancedDistribution(BodyManager *inBodyManager, int inNumBodies, float inEnvironmentSize = 512.0f);

	BPLayerInterfaceImpl	mBroadPhaseLayerInterface;
	BroadPhase *			mBroadPhase = nullptr;
	BodyManager *			mBodyManager = nullptr;
};
