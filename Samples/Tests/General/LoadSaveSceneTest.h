// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/PhysicsScene.h>

class LoadSaveSceneTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, LoadSaveSceneTest)

	// Description of the test
	virtual const char *		GetDescription() const override
	{
		return "Tests the object stream serialization system by creating a number of shapes, storing them, loading them and then simulating them.";
	}

	// See: Test
	virtual void				Initialize() override;

	// Create a test scene
	static Ref<PhysicsScene>	sCreateScene();
};
