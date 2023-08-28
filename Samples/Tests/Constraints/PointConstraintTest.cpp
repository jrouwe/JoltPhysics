// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/PointConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PointConstraintTest)
{
	JPH_ADD_BASE_CLASS(PointConstraintTest, Test)
}

void PointConstraintTest::Initialize()
{
	// Floor
	CreateFloor();

	float half_cylinder_height = 2.5f;

	const int cChainLength = 15;

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	// Bodies attached through point constraints
	Quat rotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
	RVec3 position(0, 50, 0);
	Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(new CapsuleShape(half_cylinder_height, 1), position, rotation, EMotionType::Static, Layers::NON_MOVING));
	top.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

	Body *prev = &top;
	for (int i = 1; i < cChainLength; ++i)
	{
		position += Vec3(2.0f * half_cylinder_height, 0, 0);

		Body &segment = *mBodyInterface->CreateBody(BodyCreationSettings(new CapsuleShape(half_cylinder_height, 1), position, rotation, EMotionType::Dynamic, Layers::MOVING));
		segment.SetCollisionGroup(CollisionGroup(group_filter, 0, CollisionGroup::SubGroupID(i)));
		mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

		PointConstraintSettings settings;
		settings.mPoint1 = settings.mPoint2 = position + Vec3(-half_cylinder_height, 0, 0);
		mPhysicsSystem->AddConstraint(settings.Create(*prev, segment));

		prev = &segment;
	}
}
