// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/PhysicsScene.h>

// This test tests the serialization system by creating a number of shapes, storing them, loading them and then simulating them
class LoadSaveSceneTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(LoadSaveSceneTest)

	// See: Test
	virtual void				Initialize() override;

	// Create a test scene
	static Ref<PhysicsScene>	sCreateScene();
};
