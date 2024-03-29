// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>

TEST_SUITE("SoftBodyTests")
{
	TEST_CASE("TestBendConstraint")
	{
		// Possible values for x3
		const Float3 x3_values[] = {
			Float3(0, 0, 1),	// forming flat plane
			Float3(0, 0, -1),	// overlapping
			Float3(0, 1, 0),	// 90 degrees concave
			Float3(0, -1, 0),	// 90 degrees convex
			Float3(0, 1, 1),	// 45 degrees concave
			Float3(0, -1, -1)	// 135 degrees convex
		};

		for (const Float3 &x3 : x3_values)
		{
			PhysicsTestContext c;
			PhysicsSystem *s = c.GetSystem();
			BodyInterface &bi = s->GetBodyInterface();

			// Create settings
			Ref<SoftBodySharedSettings> shared_settings = new SoftBodySharedSettings;

			/* Create two triangles with a shared edge, x3 = free, the rest is locked
			   x2
			e1/  \e3
			 /    \
			x0----x1
			 \ e0 /
			e2\  /e4
			   x3
			*/
			SoftBodySharedSettings::Vertex v;
			v.mPosition = Float3(-1, 0, 0);
			v.mInvMass = 0;
			shared_settings->mVertices.push_back(v);
			v.mPosition = Float3(1, 0, 0);
			shared_settings->mVertices.push_back(v);
			v.mPosition = Float3(0, 0, -1);
			shared_settings->mVertices.push_back(v);
			v.mPosition = x3;
			v.mInvMass = 1;
			shared_settings->mVertices.push_back(v);

			// Create the 2 triangles
			shared_settings->AddFace(SoftBodySharedSettings::Face(0, 1, 2));
			shared_settings->AddFace(SoftBodySharedSettings::Face(0, 3, 1));

			// Create edge and dihedral constraints
			SoftBodySharedSettings::VertexAttributes va;
			va.mShearCompliance = FLT_MAX;
			va.mBendCompliance = 0;
			shared_settings->CreateConstraints(&va, 1, SoftBodySharedSettings::EBendType::Dihedral);

			// Optimize the settings
			shared_settings->Optimize();

			// Create the soft body
			SoftBodyCreationSettings sb_settings(shared_settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
			sb_settings.mGravityFactor = 0.0f;
			sb_settings.mAllowSleeping = false;
			sb_settings.mUpdatePosition = false;
			Body &body = *bi.CreateSoftBody(sb_settings);
			bi.AddBody(body.GetID(), EActivation::Activate);
			SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());

			// Test 4 angles to see if there are singularities (the dot product between the triangles has the same value for 2 configurations)
			for (float angle : { 0.0f, 90.0f, 180.0f, 270.0f })
			{
				// Perturb x3
				Vec3 perturbed_x3(x3);
				mp->GetVertex(3).mPosition = 0.5f * (Mat44::sRotationX(DegreesToRadians(angle)) * perturbed_x3);

				// Simulate
				c.Simulate(0.25f);

				// Should return to the original position
				CHECK_APPROX_EQUAL(mp->GetVertex(3).mPosition, Vec3(x3), 1.0e-3f);
			}
		}
	}
}
