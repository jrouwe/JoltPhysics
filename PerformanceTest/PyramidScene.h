// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that creates a pyramid of boxes to create a very large island
class PyramidScene : public PerformanceTestScene
{
public:
	virtual const char *	GetName() const override
	{
		return "Pyramid";
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		BodyInterface &bi = inPhysicsSystem.GetBodyInterface();

		// Floor
		bi.CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(50.0f, 1.0f, 50.0f), 0.0f), RVec3(Vec3(0.0f, -1.0f, 0.0f)), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		const float cBoxSize = 2.0f;
		const float cBoxSeparation = 0.5f;
		const float cHalfBoxSize = 0.5f * cBoxSize;
		const int cPyramidHeight = 15;

		RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(cHalfBoxSize), 0.0f); // No convex radius to force more collisions

		// Pyramid
		for (int i = 0; i < cPyramidHeight; ++i)
			for (int j = i / 2; j < cPyramidHeight - (i + 1) / 2; ++j)
				for (int k = i / 2; k < cPyramidHeight - (i + 1) / 2; ++k)
				{
					RVec3 position(Real(-cPyramidHeight + cBoxSize * j + (i & 1? cHalfBoxSize : 0.0f)), Real(1.0f + (cBoxSize + cBoxSeparation) * i), Real(-cPyramidHeight + cBoxSize * k + (i & 1? cHalfBoxSize : 0.0f)));
					BodyCreationSettings settings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
					settings.mAllowSleeping = false; // No sleeping to force the large island to stay awake
					bi.CreateAndAddBody(settings, EActivation::Activate);
				}
	}
};
