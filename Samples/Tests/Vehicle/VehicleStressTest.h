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
	virtual void					PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	Array<Ref<VehicleConstraint>>	mVehicles;							///< The vehicle constraints
};
