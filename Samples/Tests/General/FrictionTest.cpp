// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/General/FrictionTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(FrictionTest)
{
	JPH_ADD_BASE_CLASS(FrictionTest, Test)
}

const char *FrictionTest::sScenes[] =
{
	"Ramp",
	"Continuous Impulses"
};

int FrictionTest::sSelectedScene = 0;

void FrictionTest::Initialize()
{
	if (sSelectedScene == 0)
	{
		// Test a ramp with different friction values

		// Floor
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(100.0f, 1.0f, 100.0f), 0.0f), RVec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

		RefConst<Shape> box = new BoxShape(Vec3(2.0f, 2.0f, 2.0f));
		RefConst<Shape> sphere = new SphereShape(2.0f);

		// Bodies with increasing friction
		for (int i = 0; i <= 10; ++i)
		{
			Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(box, RVec3(-50.0f + i * 10.0f, 55.0f, -50.0f), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
			body.SetFriction(0.1f * i);
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
			SetBodyLabel(body.GetID(), StringFormat("Friction: %.1f", double(body.GetFriction())));
		}

		for (int i = 0; i <= 10; ++i)
		{
			Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(sphere, RVec3(-50.0f + i * 10.0f, 47.0f, -40.0f), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
			body.SetFriction(0.1f * i);
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
			SetBodyLabel(body.GetID(), StringFormat("Friction: %.1f", double(body.GetFriction())));
		}
	}
	else
	{
		// Test that continous impulses in a direction tied to the body will not cause the body to rotate

		// Floor
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(100.0f, 1.0f, 100.0f), 0.0f), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

		RefConst<Shape> box = new BoxShape(Vec3(3.0f, 2.0f, 2.0f));

		// Bodies with increasing friction
		for (int i = 0; i <= 10; ++i)
		{
			Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(box, RVec3(-50.0f + i * 10.0f, 2.999f, -50.0f), Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
			body.SetFriction(0.1f * i);
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
			mBodies.push_back(&body);
		}
	}
}

void FrictionTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	if (sSelectedScene == 1)
		for (Body *body : mBodies)
		{
			mBodyInterface->AddImpulse(body->GetID(), body->GetRotation() * Vec3(0, 0, 0.15f) / body->GetMotionProperties()->GetInverseMass());
			SetBodyLabel(body->GetID(), StringFormat("Friction: %.1f, x %.1f, orientation %.6f", double(body->GetFriction()), double(body->GetPosition().GetX()), double(RadiansToDegrees(body->GetRotation().GetRotationAngle(Vec3::sAxisY())))));
		}
}

void FrictionTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() {
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSelectedScene = i; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}
