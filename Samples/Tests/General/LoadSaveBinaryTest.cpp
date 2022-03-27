// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/LoadSaveBinaryTest.h>
#include <Tests/General/LoadSaveSceneTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Utils/Log.h>
#include <Jolt/Core/StreamWrapper.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(LoadSaveBinaryTest) 
{ 
	JPH_ADD_BASE_CLASS(LoadSaveBinaryTest, Test) 
}

void LoadSaveBinaryTest::Initialize()
{
	// Create scene
	Ref<PhysicsScene> scene = LoadSaveSceneTest::sCreateScene();

	{
		// Create a new scene by creating the body first and then converting it back to body creation settings
		Ref<PhysicsScene> scene_copy = new PhysicsScene();
		BodyInterface &bi = mPhysicsSystem->GetBodyInterface();
		for (const BodyCreationSettings &b : scene->GetBodies())
		{
			Body &body = *bi.CreateBody(b);
			scene_copy->AddBody(body.GetBodyCreationSettings());
			bi.DestroyBody(body.GetID());
		}

		// Replace the original scene
		scene = scene_copy;
	}

	stringstream data;

	// Write scene
	{
		StreamOutWrapper stream_out(data);
		scene->SaveBinaryState(stream_out, true, true);
	}

	// Clear scene
	scene = nullptr;

	// Read scene back in
	{
		StreamInWrapper stream_in(data);
		PhysicsScene::PhysicsSceneResult result = PhysicsScene::sRestoreFromBinaryState(stream_in);
		if (result.HasError())
			FatalError(result.GetError().c_str());
		scene = result.Get();
	}

	// Instantiate scene
	scene->CreateBodies(mPhysicsSystem);
}