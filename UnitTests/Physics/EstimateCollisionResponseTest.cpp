// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/EstimateCollisionResponse.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include "PhysicsTestContext.h"
#include "Layers.h"

TEST_SUITE("EstimateCollisionResponseTests")
{
	// Test CastShape ordering according to penetration depth
	TEST_CASE("TestEstimateCollisionResponse")
	{
		const Vec3 cBox1HalfExtents(0.1f, 1, 2);
		const Vec3 cBox2HalfExtents(0.2f, 3, 4);

		// Test different motion types, restitution, positions and angular velocities
		for (EMotionType mt : { EMotionType::Static, EMotionType::Kinematic, EMotionType::Dynamic })
			for (float restitution : { 0.0f, 0.3f, 1.0f })
				for (float y : { 0.0f, 0.5f, cBox2HalfExtents.GetY() })
					for (float z : { 0.0f, 0.5f, cBox2HalfExtents.GetZ() })
						for (float w : { 0.0f, -1.0f, 1.0f })
						{
							PhysicsTestContext c;
							c.ZeroGravity();

							// Install a listener that predicts the collision response
							class MyListener : public ContactListener
							{
							public:
								virtual void	OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
								{
									EstimateCollisionResponse(inBody1, inBody2, inManifold, mPredictedV1, mPredictedW1, mPredictedV2, mPredictedW2, mImpulses, ioSettings.mCombinedRestitution);
								}

								ContactImpulses	mImpulses;
								Vec3			mPredictedV1 = Vec3::sNaN();
								Vec3			mPredictedW1 = Vec3::sNaN();
								Vec3			mPredictedV2 = Vec3::sNaN();
								Vec3			mPredictedW2 = Vec3::sNaN();
							};

							MyListener listener;
							c.GetSystem()->SetContactListener(&listener);

							const RVec3 cBaseOffset(1, 2, 3);
							const Real cEpsilon = 0.0001_r;

							Body &box1 = c.CreateBox(cBaseOffset, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, cBox1HalfExtents);
							box1.SetRestitution(restitution);
							box1.SetFriction(0.0f); // No friction, the estimation doesn't handle that
							box1.SetLinearVelocity(Vec3(1, 1, 0));
							box1.SetAngularVelocity(Vec3(0, w, 0));

							Body &box2 = c.CreateBox(cBaseOffset + RVec3(cBox1HalfExtents.GetX() + cBox2HalfExtents.GetX() - cEpsilon, y, z), Quat::sIdentity(), mt, EMotionQuality::Discrete, mt == EMotionType::Static? Layers::NON_MOVING : Layers::MOVING, cBox2HalfExtents);
							box2.SetRestitution(restitution);
							box2.SetFriction(0.0f);
							if (mt != EMotionType::Static)
								box2.SetLinearVelocity(Vec3(-1, 0, 0));

							c.SimulateSingleStep();

							// Check that the predicted velocities are correct
							CHECK_APPROX_EQUAL(listener.mPredictedV1, box1.GetLinearVelocity());
							CHECK_APPROX_EQUAL(listener.mPredictedW1, box1.GetAngularVelocity());
							CHECK_APPROX_EQUAL(listener.mPredictedV2, box2.GetLinearVelocity());
							CHECK_APPROX_EQUAL(listener.mPredictedW2, box2.GetAngularVelocity());
						}
	}
}
