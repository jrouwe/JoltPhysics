// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/TwoDFunnelTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(TwoDFunnelTest) 
{ 
	JPH_ADD_BASE_CLASS(TwoDFunnelTest, Test) 
}

void TwoDFunnelTest::Initialize()
{
	// Floor
	CreateFloor();

	// 2D funnel
	RefConst<Shape> wall = new BoxShape(Vec3(0.1f, 10, 1));
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, RVec3(-12, 8, 0), Quat::sRotation(Vec3::sAxisZ(), 0.2f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, RVec3(12, 8, 0), Quat::sRotation(Vec3::sAxisZ(), -0.2f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Shapes falling in 2D funnel
	Ref<Shape> shapes[] = {
		new SphereShape(0.5f),
		new BoxShape(Vec3::sReplicate(0.5f)),
		new CapsuleShape(0.2f, 0.3f)
	};

	// Constraint to limit motion in XY plane
	SixDOFConstraintSettings constraint_settings;
	constraint_settings.mSpace = EConstraintSpace::LocalToBodyCOM;
	constraint_settings.MakeFreeAxis(SixDOFConstraintSettings::EAxis::RotationX); // Don't limit, we use inertia tensor to limit rotation
	constraint_settings.MakeFreeAxis(SixDOFConstraintSettings::EAxis::RotationY); // Don't limit, we use inertia tensor to limit rotation
	constraint_settings.MakeFreeAxis(SixDOFConstraintSettings::EAxis::RotationZ);
	constraint_settings.MakeFreeAxis(SixDOFConstraintSettings::EAxis::TranslationX);
	constraint_settings.MakeFreeAxis(SixDOFConstraintSettings::EAxis::TranslationY);
	constraint_settings.MakeFixedAxis(SixDOFConstraintSettings::EAxis::TranslationZ); // Don't allow movement in Z direction

	BodyCreationSettings bcs(shapes[0], RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	for (int x = 0; x < 20; ++x)
		for (int y = 0; y < 10; ++y)
		{
			// Create body
			bcs.SetShape(shapes[(x * y) % size(shapes)]);
			bcs.mPosition = RVec3(-10.0_r + x, 10.0_r + y, 0);
			Body &body = *mBodyInterface->CreateBody(bcs);			
			mBodyInterface->AddBody(body.GetID(), EActivation::Activate);

			// Constraint the body to the XY plane
			constraint_settings.mPosition1 = body.GetCenterOfMassPosition(); // Attach it at the initial position of the body for best precision
			TwoBodyConstraint *constraint = constraint_settings.Create(Body::sFixedToWorld, body);
			mPhysicsSystem->AddConstraint(constraint);

			// Update the mass properties for this body to make it rotate only around Z
			MassProperties mass_properties = bcs.GetShape()->GetMassProperties();
			MotionProperties *mp = body.GetMotionProperties();
			mp->SetInverseMass(1.0f / mass_properties.mMass);
			mp->SetInverseInertia(Vec3(0, 0, 1.0f / mass_properties.mInertia.GetAxisZ().Length()), Quat::sIdentity());
		}
}
