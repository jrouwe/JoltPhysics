// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/SwingTwistConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SwingTwistConstraintTest)
{
	JPH_ADD_BASE_CLASS(SwingTwistConstraintTest, Test)
}

void SwingTwistConstraintTest::Initialize()
{
	// Floor
	CreateFloor();

	float half_cylinder_height = 1.5f;

	const int cChainLength = 10;

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	Body *prev = nullptr;
	Quat rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
	RVec3 position(0, 25, 0);
	for (int i = 0; i < cChainLength; ++i)
	{
		position += Vec3(2.0f * half_cylinder_height, 0, 0);

		Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(new CapsuleShape(half_cylinder_height, 0.5f), position, Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI * i) * rotation, i == 0? EMotionType::Static : EMotionType::Dynamic, i == 0? Layers::NON_MOVING : Layers::MOVING));
		segment.SetCollisionGroup(CollisionGroup(group_filter, 0, CollisionGroup::SubGroupID(i)));
		mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);
		if (i != 0)
			segment.SetAllowSleeping(false);

		if (prev != nullptr)
		{
			Ref<SwingTwistConstraintSettings> settings = new SwingTwistConstraintSettings;
			settings->mPosition1 = settings->mPosition2 = position + Vec3(-half_cylinder_height, 0, 0);
			settings->mTwistAxis1 = settings->mTwistAxis2 = Vec3::sAxisX();
			settings->mPlaneAxis1 = settings->mPlaneAxis2 = Vec3::sAxisY();
			settings->mNormalHalfConeAngle = sNormalHalfConeAngle;
			settings->mPlaneHalfConeAngle = sPlaneHalfConeAngle;
			settings->mTwistMinAngle = sTwistMinAngle;
			settings->mTwistMaxAngle = sTwistMaxAngle;

			Ref<SwingTwistConstraint> constraint = static_cast<SwingTwistConstraint *>(settings->Create(*prev, segment));
			mPhysicsSystem->AddConstraint(constraint);
			mConstraints.push_back(constraint);
		}

		prev = &segment;
	}
}

void SwingTwistConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	for (SwingTwistConstraint *c : mConstraints)
	{
		c->SetNormalHalfConeAngle(sNormalHalfConeAngle);
		c->SetPlaneHalfConeAngle(sPlaneHalfConeAngle);
		c->SetTwistMinAngle(sTwistMinAngle);
		c->SetTwistMaxAngle(sTwistMaxAngle);
	}
}

void SwingTwistConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateSlider(inSubMenu, "Min Twist Angle (deg)", RadiansToDegrees(sTwistMinAngle), -180.0f, 0.0f, 1.0f, [=](float inValue) { sTwistMinAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Twist Angle (deg)", RadiansToDegrees(sTwistMaxAngle), 0.0f, 180.0f, 1.0f, [=](float inValue) { sTwistMaxAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Normal Half Cone Angle (deg)", RadiansToDegrees(sNormalHalfConeAngle), 0.0f, 180.0f, 1.0f, [=](float inValue) { sNormalHalfConeAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Plane Half Cone Angle (deg)", RadiansToDegrees(sPlaneHalfConeAngle), 0.0f, 180.0f, 1.0f, [=](float inValue) { sPlaneHalfConeAngle = DegreesToRadians(inValue); });

	inUI->ShowMenu(inSubMenu);
}
