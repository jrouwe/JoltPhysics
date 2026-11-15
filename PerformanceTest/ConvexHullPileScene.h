// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that drops a number of complex convex hulls on a flat floor
class ConvexHullPileScene : public PerformanceTestScene
{
public:
	virtual const char *	GetName() const override
	{
		return "ConvexHullPile";
	}

	virtual bool			Load(const String &inAssetPath) override
	{
		mt19937 random;
		auto random_float = [&random]() { return float(random()) / float(random.max() - random.min()) * 2.0f - 1.0f; };

		Array<Vec3> points;
		constexpr int cNumPoints = 40;
		points.reserve(cNumPoints);
		for (int i = 0; i < 100; ++i)
		{
			points.clear();
			for (int j = 0; j < cNumPoints; ++j)
				points.push_back(Vec3(1.5f * random_float(), 0.5f * random_float(), 0.75f * random_float()));
			mShapes.push_back(ConvexHullShapeSettings(points).Create().Get());
		}

		return true;
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		BodyInterface &bi = inPhysicsSystem.GetBodyInterface();

		// Create floor
		BodyCreationSettings floor(new BoxShape(Vec3(50.0f, 1.0f, 50.0f)), RVec3(0, -0.5_r, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		floor.mFriction = 1.0f;
		bi.CreateAndAddBody(floor, EActivation::DontActivate);

		// Construct bodies
		mt19937 random;
		int next_shape = 0;
		for (int x = 0; x < 10; ++x)
			for (int y = 0; y < 40; ++y)
				for (int z = 0; z < 10; ++z)
				{
					BodyCreationSettings bcs(mShapes[next_shape], RVec3(x * 2.0_r - 10.0_r, y * 2.0_r + 1.0_r, z * 2.0_r - 10.0_r), Quat::sRandom(random), EMotionType::Dynamic, Layers::MOVING);
					bcs.mFriction = 1.0f;
					bi.CreateAndAddBody(bcs, EActivation::Activate);
					next_shape = (next_shape + 1) % (int)mShapes.size();
				}
	}

private:
	Array<RefConst<Shape>>	mShapes;
};
