// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/LoadSaveBinaryTest.h>
#include <Tests/General/LoadSaveSceneTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Utils/Log.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(LoadSaveBinaryTest)
{
	JPH_ADD_BASE_CLASS(LoadSaveBinaryTest, Test)
}

void LoadSaveBinaryTest::Initialize()
{
	// Create scene
	Ref<PhysicsScene> scene = LoadSaveSceneTest::sCreateScene();

	{
		// Create a new scene by instantiating the scene in a physics system and then converting it back to a scene
		PhysicsSystem system;
		BPLayerInterfaceImpl layer_interface;
		ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
		ObjectLayerPairFilterImpl object_vs_object_layer_filter;
		system.Init(mPhysicsSystem->GetMaxBodies(), 0, 1024, 1024, layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
		scene->CreateBodies(&system);
		Ref<PhysicsScene> scene_copy = new PhysicsScene();
		scene_copy->FromPhysicsSystem(&system);

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
