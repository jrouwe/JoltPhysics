// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ChangeShapeTest.h>
#include <Layers.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ChangeShapeTest)
{
	JPH_ADD_BASE_CLASS(ChangeShapeTest, Test)
}

void ChangeShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Shapes
	mShapes.push_back(new BoxShape(Vec3(0.5f, 1.5f, 0.5f)));
	mShapes.push_back(new SphereShape(0.5f));
	mShapes.push_back(new CapsuleShape(1.0f, 0.5f));
	mShapes.push_back(TaperedCapsuleShapeSettings(1.0f, 0.5f, 0.3f).Create().Get());

	// Compound with center of mass shifted (this requires a correction of the position in the body)
	StaticCompoundShapeSettings compound_settings;
	compound_settings.AddShape(Vec3(0, 1.5f, 0), Quat::sIdentity(), new CapsuleShape(1.5f, 0.5f));
	compound_settings.AddShape(Vec3(0, 3, 0), Quat::sIdentity(), new SphereShape(1));
	RefConst<Shape> compound = compound_settings.Create().Get();
	mShapes.push_back(compound);

	// Create dynamic body that changes shape
	BodyCreationSettings settings;
	settings.SetShape(mShapes[mShapeIdx]);
	settings.mPosition = RVec3(0, 10, 0);
	settings.mMotionType = EMotionType::Dynamic;
	settings.mObjectLayer = Layers::MOVING;
	mBodyID = mBodyInterface->CreateBody(settings)->GetID();
	mBodyInterface->AddBody(mBodyID, EActivation::Activate);
}

void ChangeShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	const float cSwitchTime = 3.0f;

	// Increment time
	mTime += inParams.mDeltaTime;

	// Get new shape
	int shape_idx = int(mTime / cSwitchTime) % mShapes.size();

	// Change shape
	if (mShapeIdx != shape_idx)
	{
		mShapeIdx = shape_idx;
		mBodyInterface->SetShape(mBodyID, mShapes[mShapeIdx], true, mActivateAfterSwitch? EActivation::Activate : EActivation::DontActivate);
	}
}

void ChangeShapeTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	inStream.Write(mShapeIdx);
}

void ChangeShapeTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	inStream.Read(mShapeIdx);

	// Reset the shape to what was stored
	mBodyInterface->SetShape(mBodyID, mShapes[mShapeIdx], true, EActivation::DontActivate);
}

void ChangeShapeTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateCheckBox(inSubMenu, "Activate Body After Switch", mActivateAfterSwitch, [this](UICheckBox::EState inState) { mActivateAfterSwitch = inState == UICheckBox::STATE_CHECKED; });
}

