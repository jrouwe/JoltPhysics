// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Tools/LoadSnapshotTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Application/DebugUI.h>
#include <Utils/Log.h>
#include <Layers.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <commdlg.h>
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_IMPLEMENT_RTTI_VIRTUAL(LoadSnapshotTest)
{
	JPH_ADD_BASE_CLASS(LoadSnapshotTest, Test)
}

void LoadSnapshotTest::Initialize()
{
	// Let user browse for a file
	char file_name[MAX_PATH] = "";
	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "Snapshots\0*.bin\0";
	ofn.lpstrFile = file_name;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select a Jolt Binary Snapshot";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
	if (!GetOpenFileNameA(&ofn))
		return;

	ifstream stream(file_name, ifstream::in | ifstream::binary);
	if (!stream.is_open())
		FatalError("Unable to open file");

	StreamInWrapper wrapper(stream);
	PhysicsScene::PhysicsSceneResult result = PhysicsScene::sRestoreFromBinaryState(wrapper);
	if (result.HasError())
		FatalError(result.GetError().c_str());
	Ref<PhysicsScene> scene = result.Get();

	// Determine quaternion that rotates the world so that up is Y
	Quat up_rotation;
	switch (sUpAxis)
	{
	case 0:		up_rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);	break;
	case 2:		up_rotation = Quat::sRotation(Vec3::sAxisX(), -0.5f * JPH_PI);	break;
	default:	up_rotation = Quat::sIdentity();								break;
	}

	// Determine if we are forced to override the object layers because one of the bodies has a layer number that is invalid in the context of this application
	bool override_layers = sOverrideLayers;
	if (!override_layers)
		for (BodyCreationSettings &settings : scene->GetBodies())
			if (settings.mObjectLayer >= Layers::NUM_LAYERS)
			{
				override_layers = true;
				break;
			}

	for (BodyCreationSettings &settings : scene->GetBodies())
	{
		if (override_layers)
		{
			// Override the layer so that all static objects are in the non-moving layer and everything else is in the moving layer
			if (settings.mMotionType == EMotionType::Static)
				settings.mObjectLayer = Layers::NON_MOVING;
			else
				settings.mObjectLayer = Layers::MOVING;
		}

		// Rotate the body so that it matches Y is up
		settings.mPosition = RMat44::sRotation(up_rotation) * settings.mPosition;
		settings.mRotation = up_rotation * settings.mRotation;
	}

	scene->CreateBodies(mPhysicsSystem);
}

void LoadSnapshotTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateComboBox(inSubMenu, "Up Axis", { "X", "Y", "Z" }, sUpAxis, [](int inItem) { sUpAxis = inItem; });
	inUI->CreateCheckBox(inSubMenu, "Override Object Layers", sOverrideLayers, [](UICheckBox::EState inState) { sOverrideLayers = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateTextButton(inSubMenu, "Accept Changes", [this]() { RestartTest(); });
}
