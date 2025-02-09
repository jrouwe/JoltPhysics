// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that creates the max number of bodies that Jolt supports and simulates them
class MaxBodiesScene : public PerformanceTestScene
{
public:
	virtual const char *	GetName() const override
	{
		return "MaxBodies";
	}

	virtual uint			GetTempAllocatorSizeMB() const
	{
		return 256;
	}

	virtual uint			GetMaxBodies() const
	{
		return BodyID::cMaxBodyIndex + 1;
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		BodyInterface &bi = inPhysicsSystem.GetBodyInterface();

		// Reduce the solver iteration count in the interest of performance
		PhysicsSettings settings = inPhysicsSystem.GetPhysicsSettings();
		settings.mNumVelocitySteps = 4;
		settings.mNumPositionSteps = 1;
		inPhysicsSystem.SetPhysicsSettings(settings);

		// Create the bodies
		uint num_bodies = GetMaxBodies();
		BodyIDVector body_ids;
		body_ids.reserve(num_bodies);
		uint num_per_axis = uint(pow(float(num_bodies), 1.0f / 3.0f)) + 1;
		BodyCreationSettings bcs(new BoxShape(Vec3::sReplicate(0.5f)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		for (uint x = 0; x < num_per_axis && body_ids.size() < num_bodies; ++x)
			for (uint y = 0; y < num_per_axis && body_ids.size() < num_bodies; ++y)
				for (uint z = 0; z < num_per_axis && body_ids.size() < num_bodies; ++z)
				{
					bcs.mPosition = RVec3(x, y, z);
					body_ids.push_back(bi.CreateBody(bcs)->GetID());
				}

		// Add the bodies to the simulation
		BodyInterface::AddState state = bi.AddBodiesPrepare(body_ids.data(), num_bodies);
		bi.AddBodiesFinalize(body_ids.data(), num_bodies, state, EActivation::Activate);
	}
};
