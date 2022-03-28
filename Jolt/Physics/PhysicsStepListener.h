// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

class PhysicsSystem;

/// A listener class that receives a callback before every physics simulation step
class PhysicsStepListener
{
public:
	/// Ensure virtual destructor
	virtual					~PhysicsStepListener() = default;

	/// Called before every simulation step (received inCollisionSteps times for every PhysicsSystem::Update(...) call)
	/// This is called while all bodies and constraints are locked for modifications. Multiple listeners can be executed in parallel and it is the responsibility of the listener
	/// to avoid race conditions.
	/// Note that this function is not called if there aren't any active bodies or when the physics system is updated with 0 delta time.
	virtual void			OnStep(float inDeltaTime, PhysicsSystem &inPhysicsSystem) = 0;
};

JPH_NAMESPACE_END
