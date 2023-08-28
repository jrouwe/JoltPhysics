// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/GearConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/GearConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(GearConstraintTest)
{
	JPH_ADD_BASE_CLASS(GearConstraintTest, Test)
}

void GearConstraintTest::Initialize()
{
	// Floor
	CreateFloor();

	constexpr float cGearHalfWidth = 0.05f;

	constexpr float cGear1Radius = 0.5f;
	constexpr int cGear1NumTeeth = 100;

	constexpr float cGear2Radius = 2.0f;
	constexpr int cGear2NumTeeth = int(cGear1NumTeeth * cGear2Radius / cGear1Radius);

	constexpr float cToothThicknessBottom = 0.01f;
	constexpr float cToothThicknessTop = 0.005f;
	constexpr float cToothHeight = 0.02f;

	// Create a tooth
	Array<Vec3> tooth_points = {
		Vec3(0, cGearHalfWidth, cToothThicknessBottom),
		Vec3(0, -cGearHalfWidth, cToothThicknessBottom),
		Vec3(0, cGearHalfWidth, -cToothThicknessBottom),
		Vec3(0, -cGearHalfWidth, -cToothThicknessBottom),
		Vec3(cToothHeight, -cGearHalfWidth, cToothThicknessTop),
		Vec3(cToothHeight, cGearHalfWidth, cToothThicknessTop),
		Vec3(cToothHeight, -cGearHalfWidth, -cToothThicknessTop),
		Vec3(cToothHeight, cGearHalfWidth, -cToothThicknessTop),
	};
	ConvexHullShapeSettings tooth_settings(tooth_points);
	tooth_settings.SetEmbedded();

	// Create gear 1
	CylinderShapeSettings gear1_cylinder(cGearHalfWidth, cGear1Radius);
	gear1_cylinder.SetEmbedded();

	StaticCompoundShapeSettings gear1_settings;
	gear1_settings.SetEmbedded();

	gear1_settings.AddShape(Vec3::sZero(), Quat::sIdentity(), &gear1_cylinder);
	for (int i = 0; i < cGear1NumTeeth; ++i)
	{
		Quat rotation = Quat::sRotation(Vec3::sAxisY(), 2.0f * JPH_PI * i / cGear1NumTeeth);
		gear1_settings.AddShape(rotation * Vec3(cGear1Radius, 0, 0), rotation, &tooth_settings);
	}

	RVec3 gear1_initial_p(0, 3.0f, 0);
	Quat gear1_initial_r = Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI);
	Body *gear1 = mBodyInterface->CreateBody(BodyCreationSettings(&gear1_settings, gear1_initial_p, gear1_initial_r, EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(gear1->GetID(), EActivation::Activate);

	// Create gear 2
	CylinderShapeSettings gear2_cylinder(cGearHalfWidth, cGear2Radius);
	gear2_cylinder.SetEmbedded();

	StaticCompoundShapeSettings gear2_settings;
	gear2_settings.SetEmbedded();

	gear2_settings.AddShape(Vec3::sZero(), Quat::sIdentity(), &gear2_cylinder);
	for (int i = 0; i < cGear2NumTeeth; ++i)
	{
		Quat rotation = Quat::sRotation(Vec3::sAxisY(), 2.0f * JPH_PI * (i + 0.5f) / cGear2NumTeeth);
		gear2_settings.AddShape(rotation * Vec3(cGear2Radius, 0, 0), rotation, &tooth_settings);
	}

	RVec3 gear2_initial_p = gear1_initial_p + Vec3(cGear1Radius + cGear2Radius + cToothHeight, 0, 0);
	Quat gear2_initial_r = gear1_initial_r;
	Body *gear2 = mBodyInterface->CreateBody(BodyCreationSettings(&gear2_settings, gear2_initial_p, gear2_initial_r, EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(gear2->GetID(), EActivation::Activate);

	// Create a hinge for gear 1
	HingeConstraintSettings hinge1;
	hinge1.mPoint1 = hinge1.mPoint2 = gear1_initial_p;
	hinge1.mHingeAxis1 = hinge1.mHingeAxis2 = Vec3::sAxisZ();
	hinge1.mNormalAxis1 = hinge1.mNormalAxis2 = Vec3::sAxisX();
	Constraint *hinge1_constraint = hinge1.Create(Body::sFixedToWorld, *gear1);
	mPhysicsSystem->AddConstraint(hinge1_constraint);

	// Create a hinge for gear 1
	HingeConstraintSettings hinge2;
	hinge2.mPoint1 = hinge2.mPoint2 = gear2_initial_p;
	hinge2.mHingeAxis1 = hinge2.mHingeAxis2 = Vec3::sAxisZ();
	hinge2.mNormalAxis1 = hinge2.mNormalAxis2 = Vec3::sAxisX();
	Constraint *hinge2_constraint = hinge2.Create(Body::sFixedToWorld, *gear2);
	mPhysicsSystem->AddConstraint(hinge2_constraint);

	// Disable collision between gears
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(2);
	group_filter->DisableCollision(0, 1);
	gear1->SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	gear2->SetCollisionGroup(CollisionGroup(group_filter, 0, 1));

	// Create gear constraint to constrain the two bodies
	GearConstraintSettings gear;
	gear.mHingeAxis1 = hinge1.mHingeAxis1;
	gear.mHingeAxis2 = hinge2.mHingeAxis1;
	gear.SetRatio(cGear1NumTeeth, cGear2NumTeeth);
	GearConstraint *gear_constraint = static_cast<GearConstraint *>(gear.Create(*gear1, *gear2));
	gear_constraint->SetConstraints(hinge1_constraint, hinge2_constraint);
	mPhysicsSystem->AddConstraint(gear_constraint);

	// Give the gear a spin
	gear2->SetAngularVelocity(Vec3(0, 0, 3.0f));
}
