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

	virtual size_t			GetTempAllocatorSizeMB() const override
	{
		return 8192;
	}

	virtual uint			GetMaxBodies() const override
	{
		return PhysicsSystem::cMaxBodiesLimit;
	}

	virtual uint			GetMaxBodyPairs() const override
	{
		return PhysicsSystem::cMaxBodyPairsLimit;
	}

	virtual uint			GetMaxContactConstraints() const override
	{
		return PhysicsSystem::cMaxContactConstraintsLimit;
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
		uint num_bodies = inPhysicsSystem.GetMaxBodies();
		uint num_constraints = 0;
		BodyIDVector body_ids;
		body_ids.reserve(num_bodies);
		uint num_per_axis = uint(pow(float(num_bodies), 1.0f / 3.0f)) + 1;
		Vec3 half_extent = Vec3::sReplicate(0.5f);
		BodyCreationSettings bcs(new BoxShape(half_extent), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		bcs.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
		bcs.mMassPropertiesOverride.SetMassAndInertiaOfSolidBox(2.0f * half_extent, 1000.0f);
		for (uint z = 0; z < num_per_axis && body_ids.size() < num_bodies; ++z)
			for (uint y = 0; y < num_per_axis && body_ids.size() < num_bodies; ++y)
				for (uint x = 0; x < num_per_axis && body_ids.size() < num_bodies; ++x)
				{
					// When we reach the limit of contact constraints, start placing the boxes further apart so they don't collide
					bcs.mPosition = RVec3(num_constraints < PhysicsSystem::cMaxContactConstraintsLimit? Real(x) : 2.0_r * x, 2.0_r * y, 2.0_r * z);
					body_ids.push_back(bi.CreateBody(bcs)->GetID());

					// From the 2nd box onwards in a row, we will get a contact constraint
					if (x > 0)
						++num_constraints;
				}

		// Add the bodies to the simulation
		BodyInterface::AddState state = bi.AddBodiesPrepare(body_ids.data(), num_bodies);
		bi.AddBodiesFinalize(body_ids.data(), num_bodies, state, EActivation::Activate);
	}
};
