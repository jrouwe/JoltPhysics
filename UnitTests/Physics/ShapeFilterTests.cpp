// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/SimShapeFilter.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

TEST_SUITE("ShapeFilterTests")
{
	// Tests two spheres in one simulated body, one collides with a static platform, the other doesn't
	TEST_CASE("TestSimShapeFilter")
	{
		// Test once per motion quality type
		for (int q = 0; q < 2; ++q)
		{
			PhysicsTestContext c;

			// Log contacts
			LoggingContactListener contact_listener;
			c.GetSystem()->SetContactListener(&contact_listener);

			// Install simulation shape filter
			class Filter : public SimShapeFilter
			{
			public:
				virtual bool	ShouldCollide(const Body &inBody1, const Shape *inShape1, const SubShapeID &inSubShapeIDOfShape1, const Body &inBody2, const Shape *inShape2, const SubShapeID &inSubShapeIDOfShape2) const override
				{
					// If the platform is colliding with the compound, filter out collisions where the shape has user data 1
					if (inBody1.GetID() == mPlatformID && inBody2.GetID() == mCompoundID)
						return inShape2->GetUserData() != 1;
					else if (inBody1.GetID() == mCompoundID && inBody2.GetID() == mPlatformID)
						return inShape1->GetUserData() != 1;
					return true;
				}

				BodyID			mPlatformID;
				BodyID			mCompoundID;
			};
			Filter shape_filter;
			c.GetSystem()->SetSimShapeFilter(&shape_filter);

			// Floor
			BodyID floor_id = c.CreateFloor().GetID();

			// Platform
			BodyInterface &bi = c.GetBodyInterface();
			shape_filter.mPlatformID = bi.CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(10, 0.5f, 10)), RVec3(0, 3.5f, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

			// Compound shape that starts above platform
			Ref<Shape> sphere = new SphereShape(0.5f);
			sphere->SetUserData(1); // Don't want sphere to collide with the platform
			Ref<Shape> sphere2 = new SphereShape(0.5f);
			Ref<StaticCompoundShapeSettings> compound_settings = new StaticCompoundShapeSettings;
			compound_settings->AddShape(Vec3(0, -2, 0), Quat::sIdentity(), sphere);
			compound_settings->AddShape(Vec3(0, 2, 0), Quat::sIdentity(), sphere2);
			Ref<StaticCompoundShape> compound = StaticCast<StaticCompoundShape>(compound_settings->Create().Get());
			BodyCreationSettings bcs(compound, RVec3(0, 7, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			if (q == 1)
			{
				// For the 2nd iteration activate CCD
				bcs.mMotionQuality = EMotionQuality::LinearCast;
				bcs.mLinearVelocity = Vec3(0, -50, 0);
			}
			shape_filter.mCompoundID = bi.CreateAndAddBody(bcs, EActivation::Activate);

			// Get sub shape IDs
			SubShapeID sphere_id = compound->GetSubShapeIDFromIndex(0, SubShapeIDCreator()).GetID();
			SubShapeID sphere2_id = compound->GetSubShapeIDFromIndex(1, SubShapeIDCreator()).GetID();

			// Simulate for 2 seconds
			c.Simulate(2.0f);

			// The compound should now be resting with sphere on the platform and the sphere2 on the floor
			CHECK_APPROX_EQUAL(bi.GetPosition(shape_filter.mCompoundID), RVec3(0, 2.5f, 0), 1.01f * c.GetSystem()->GetPhysicsSettings().mPenetrationSlop);
			CHECK_APPROX_EQUAL(bi.GetRotation(shape_filter.mCompoundID), Quat::sIdentity());

			// Check that sphere2 collided with the platform but sphere did not
			CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, shape_filter.mPlatformID, SubShapeID(), shape_filter.mCompoundID, sphere2_id));
			CHECK(!contact_listener.Contains(LoggingContactListener::EType::Add, shape_filter.mPlatformID, SubShapeID(), shape_filter.mCompoundID, sphere_id));

			// Check that sphere2 didn't collide with the floor but that the sphere did
			CHECK(contact_listener.Contains(LoggingContactListener::EType::Add, floor_id, SubShapeID(), shape_filter.mCompoundID, sphere_id));
			CHECK(!contact_listener.Contains(LoggingContactListener::EType::Add, floor_id, SubShapeID(), shape_filter.mCompoundID, sphere2_id));
		}
	}
}
