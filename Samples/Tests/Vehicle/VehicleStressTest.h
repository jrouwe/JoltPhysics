// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

// This test simulates a large amount of vehicles
class VehicleStressTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, VehicleStressTest)

	// Destructor
	virtual							~VehicleStressTest() override;

	// See: Test
	virtual void					Initialize() override;
	virtual void					ProcessInput(const ProcessInputParams &inParams) override;
	virtual void					PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void					SaveInputState(StateRecorder &inStream) const override;
	virtual void					RestoreInputState(StateRecorder &inStream) override;

private:
	Array<Ref<VehicleConstraint>>	mVehicles;							///< The vehicle constraints

	// Player input
	float							mForward = 0.0f;
	float							mRight = 0.0f;
	float							mHandBrake = 0.0f;
};
