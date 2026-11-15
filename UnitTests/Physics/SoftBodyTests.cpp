// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include "LoggingSoftBodyContactListener.h"
#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/StateRecorderImpl.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/Core/UnorderedMap.h>

TEST_SUITE("SoftBodyTests")
{
	using Vertex = SoftBodySharedSettings::Vertex;
	using VertexAttributes = SoftBodySharedSettings::VertexAttributes;
	using EBendType = SoftBodySharedSettings::EBendType;
	using ELRAType = SoftBodySharedSettings::ELRAType;

	// Create a cloth in the XZ plane with the face normals pointing up
	static Ref<SoftBodySharedSettings> sCreateCloth(uint inNumX, uint inNumZ, float inSpacing, const function<float(uint, uint)> &inInvMass, const VertexAttributes &inAttributes, EBendType inBendType = EBendType::Distance)
	{
		Ref<SoftBodySharedSettings> settings = new SoftBodySharedSettings;

		// Create vertices
		Vec3 offset(-0.5f * inSpacing * (inNumX - 1), 0, -0.5f * inSpacing * (inNumZ - 1));
		for (uint z = 0; z < inNumZ; ++z)
			for (uint x = 0; x < inNumX; ++x)
			{
				Vertex v;
				(offset + inSpacing * Vec3(float(x), 0, float(z))).StoreFloat3(&v.mPosition);
				v.mInvMass = inInvMass(x, z);
				settings->mVertices.push_back(v);
			}

		// Create faces
		auto vertex_index = [inNumX](uint inX, uint inZ) { return inX + inZ * inNumX; };
		for (uint z = 0; z < inNumZ - 1; ++z)
			for (uint x = 0; x < inNumX - 1; ++x)
			{
				settings->AddFace(SoftBodySharedSettings::Face(vertex_index(x, z), vertex_index(x, z + 1), vertex_index(x + 1, z)));
				settings->AddFace(SoftBodySharedSettings::Face(vertex_index(x + 1, z), vertex_index(x, z + 1), vertex_index(x + 1, z + 1)));
			}

		// Create constraints
		settings->CreateConstraints(&inAttributes, 1, inBendType);

		return settings;
	}

	// Create an L shaped chain of Cosserat rods with the first vertex fixed
	static Ref<SoftBodySharedSettings> sCreateRodChain()
	{
		Ref<SoftBodySharedSettings> settings = new SoftBodySharedSettings;
		for (Vec3 position : { Vec3(0, 0, 0), Vec3(0, -1, 0), Vec3(0, -2, 0), Vec3(1, -2, 0), Vec3(2, -2, 0) })
		{
			Vertex v;
			position.StoreFloat3(&v.mPosition);
			v.mInvMass = settings->mVertices.empty()? 0.0f : 1.0f;
			settings->mVertices.push_back(v);
		}

		// Note that the last rod is specified in the opposite direction
		settings->mRodStretchShearConstraints = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 4, 3 } };
		settings->mRodBendTwistConstraints = { { 0, 1 }, { 1, 2 }, { 2, 3 } };
		settings->CalculateRodProperties();
		return settings;
	}

	// Get the world space position of a vertex
	static RVec3 sGetVertexPosition(const Body &inBody, uint inIndex)
	{
		const SoftBodyMotionProperties *mp = static_cast<const SoftBodyMotionProperties *>(inBody.GetMotionProperties());
		return inBody.GetCenterOfMassTransform() * mp->GetVertex(inIndex).mPosition;
	}

	// Check that two sets of shared settings contain the same public data
	static void sCheckSettingsEqual(const SoftBodySharedSettings &inLHS, const SoftBodySharedSettings &inRHS)
	{
		CHECK(inLHS.mVertices.size() == inRHS.mVertices.size());
		for (size_t i = 0; i < min(inLHS.mVertices.size(), inRHS.mVertices.size()); ++i)
		{
			CHECK(inLHS.mVertices[i].mPosition == inRHS.mVertices[i].mPosition);
			CHECK(inLHS.mVertices[i].mVelocity == inRHS.mVertices[i].mVelocity);
			CHECK(inLHS.mVertices[i].mInvMass == inRHS.mVertices[i].mInvMass);
		}

		CHECK(inLHS.mFaces.size() == inRHS.mFaces.size());
		for (size_t i = 0; i < min(inLHS.mFaces.size(), inRHS.mFaces.size()); ++i)
		{
			for (int j = 0; j < 3; ++j)
				CHECK(inLHS.mFaces[i].mVertex[j] == inRHS.mFaces[i].mVertex[j]);
			CHECK(inLHS.mFaces[i].mMaterialIndex == inRHS.mFaces[i].mMaterialIndex);
		}

		CHECK(inLHS.mEdgeConstraints.size() == inRHS.mEdgeConstraints.size());
		for (size_t i = 0; i < min(inLHS.mEdgeConstraints.size(), inRHS.mEdgeConstraints.size()); ++i)
		{
			const SoftBodySharedSettings::Edge &l = inLHS.mEdgeConstraints[i], &r = inRHS.mEdgeConstraints[i];
			CHECK((l.mVertex[0] == r.mVertex[0] && l.mVertex[1] == r.mVertex[1] && l.mRestLength == r.mRestLength && l.mCompliance == r.mCompliance));
		}

		CHECK(inLHS.mDihedralBendConstraints.size() == inRHS.mDihedralBendConstraints.size());
		for (size_t i = 0; i < min(inLHS.mDihedralBendConstraints.size(), inRHS.mDihedralBendConstraints.size()); ++i)
		{
			const SoftBodySharedSettings::DihedralBend &l = inLHS.mDihedralBendConstraints[i], &r = inRHS.mDihedralBendConstraints[i];
			CHECK(memcmp(l.mVertex, r.mVertex, sizeof(l.mVertex)) == 0);
			CHECK((l.mCompliance == r.mCompliance && l.mInitialAngle == r.mInitialAngle));
		}

		CHECK(inLHS.mVolumeConstraints.size() == inRHS.mVolumeConstraints.size());
		for (size_t i = 0; i < min(inLHS.mVolumeConstraints.size(), inRHS.mVolumeConstraints.size()); ++i)
		{
			const SoftBodySharedSettings::Volume &l = inLHS.mVolumeConstraints[i], &r = inRHS.mVolumeConstraints[i];
			CHECK(memcmp(l.mVertex, r.mVertex, sizeof(l.mVertex)) == 0);
			CHECK((l.mCompliance == r.mCompliance && l.mSixRestVolume == r.mSixRestVolume));
		}

		CHECK(inLHS.mSkinnedConstraints.size() == inRHS.mSkinnedConstraints.size());
		for (size_t i = 0; i < min(inLHS.mSkinnedConstraints.size(), inRHS.mSkinnedConstraints.size()); ++i)
		{
			const SoftBodySharedSettings::Skinned &l = inLHS.mSkinnedConstraints[i], &r = inRHS.mSkinnedConstraints[i];
			CHECK((l.mVertex == r.mVertex && l.mMaxDistance == r.mMaxDistance && l.mBackStopDistance == r.mBackStopDistance && l.mBackStopRadius == r.mBackStopRadius && l.mNormalInfo == r.mNormalInfo));
			for (uint j = 0; j < SoftBodySharedSettings::Skinned::cMaxSkinWeights; ++j)
				CHECK((l.mWeights[j].mInvBindIndex == r.mWeights[j].mInvBindIndex && l.mWeights[j].mWeight == r.mWeights[j].mWeight));
		}

		CHECK(inLHS.mInvBindMatrices.size() == inRHS.mInvBindMatrices.size());
		for (size_t i = 0; i < min(inLHS.mInvBindMatrices.size(), inRHS.mInvBindMatrices.size()); ++i)
			CHECK((inLHS.mInvBindMatrices[i].mJointIndex == inRHS.mInvBindMatrices[i].mJointIndex && inLHS.mInvBindMatrices[i].mInvBind == inRHS.mInvBindMatrices[i].mInvBind));

		CHECK(inLHS.mLRAConstraints.size() == inRHS.mLRAConstraints.size());
		for (size_t i = 0; i < min(inLHS.mLRAConstraints.size(), inRHS.mLRAConstraints.size()); ++i)
		{
			const SoftBodySharedSettings::LRA &l = inLHS.mLRAConstraints[i], &r = inRHS.mLRAConstraints[i];
			CHECK((l.mVertex[0] == r.mVertex[0] && l.mVertex[1] == r.mVertex[1] && l.mMaxDistance == r.mMaxDistance));
		}

		CHECK(inLHS.mRodStretchShearConstraints.size() == inRHS.mRodStretchShearConstraints.size());
		for (size_t i = 0; i < min(inLHS.mRodStretchShearConstraints.size(), inRHS.mRodStretchShearConstraints.size()); ++i)
		{
			const SoftBodySharedSettings::RodStretchShear &l = inLHS.mRodStretchShearConstraints[i], &r = inRHS.mRodStretchShearConstraints[i];
			CHECK((l.mVertex[0] == r.mVertex[0] && l.mVertex[1] == r.mVertex[1] && l.mLength == r.mLength && l.mInvMass == r.mInvMass && l.mCompliance == r.mCompliance && l.mBishop == r.mBishop));
		}

		CHECK(inLHS.mRodBendTwistConstraints.size() == inRHS.mRodBendTwistConstraints.size());
		for (size_t i = 0; i < min(inLHS.mRodBendTwistConstraints.size(), inRHS.mRodBendTwistConstraints.size()); ++i)
		{
			const SoftBodySharedSettings::RodBendTwist &l = inLHS.mRodBendTwistConstraints[i], &r = inRHS.mRodBendTwistConstraints[i];
			CHECK((l.mRod[0] == r.mRod[0] && l.mRod[1] == r.mRod[1] && l.mCompliance == r.mCompliance && l.mOmega0 == r.mOmega0));
		}

		CHECK(inLHS.mMaterials.size() == inRHS.mMaterials.size());
	}

	// Create settings that use all constraint types
	static Ref<SoftBodySharedSettings> sCreateSettingsWithAllConstraintTypes()
	{
		// Create a cloth with the first row kinematic and LRA and dihedral bend constraints
		constexpr uint cNumX = 6, cNumZ = 6;
		Ref<SoftBodySharedSettings> settings = sCreateCloth(cNumX, cNumZ, 0.2f, [](uint, uint inZ) { return inZ == 0? 0.0f : 1.0f; }, VertexAttributes(1.0e-4f, 1.0e-4f, 1.0e-3f, ELRAType::EuclideanDistance, 1.1f), EBendType::Dihedral);

		// Add a volume constraint
		settings->mVolumeConstraints.push_back(SoftBodySharedSettings::Volume(1, 2, cNumX + 1, 2 * cNumX + 1, 1.0e-4f));
		settings->CalculateVolumeConstraintVolumes();

		// Add skinned constraints for the last row
		settings->mInvBindMatrices.push_back(SoftBodySharedSettings::InvBind(0, Mat44::sIdentity()));
		for (uint x = 0; x < cNumX; ++x)
		{
			SoftBodySharedSettings::Skinned skinned(x + (cNumZ - 1) * cNumX, 0.1f, 0.05f, 10.0f);
			skinned.mWeights[0] = SoftBodySharedSettings::SkinWeight(0, 1.0f);
			settings->mSkinnedConstraints.push_back(skinned);
		}
		settings->CalculateSkinnedConstraintNormals();

		// Add rods along the second row
		for (uint x = 0; x < cNumX - 1; ++x)
		{
			settings->mRodStretchShearConstraints.push_back(SoftBodySharedSettings::RodStretchShear(x + cNumX, x + 1 + cNumX, 1.0e-4f));
			if (x > 0)
				settings->mRodBendTwistConstraints.push_back(SoftBodySharedSettings::RodBendTwist(x - 1, x, 1.0e-3f));
		}
		settings->CalculateRodProperties();

		settings->Optimize();
		return settings;
	}

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

	// Test that applying a force to a soft body and rigid body of the same mass has the same effect
	TEST_CASE("TestApplyForce")
	{
		PhysicsTestContext c;
		PhysicsSystem *s = c.GetSystem();
		BodyInterface &bi = s->GetBodyInterface();

		// Soft body cube
		SoftBodyCreationSettings sb_box_settings(SoftBodySharedSettings::sCreateCube(6, 0.2f), RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		sb_box_settings.mGravityFactor = 0.0f;
		sb_box_settings.mLinearDamping = 0.0f;
		Body &sb_box = *bi.CreateSoftBody(sb_box_settings);
		BodyID sb_id = sb_box.GetID();
		bi.AddBody(sb_id, EActivation::Activate);
		constexpr float cMass = 216; // 6 * 6 * 6 * 1 kg
		CHECK_APPROX_EQUAL(sb_box.GetMotionProperties()->GetInverseMass(), 1.0f / cMass);

		// Rigid body cube of same size and mass
		const RVec3 cRBBoxPos(0, 2, 0);
		BodyCreationSettings rb_box_settings(new BoxShape(Vec3::sReplicate(0.5f)), cRBBoxPos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		rb_box_settings.mGravityFactor = 0.0f;
		rb_box_settings.mLinearDamping = 0.0f;
		rb_box_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		rb_box_settings.mMassPropertiesOverride.mMass = cMass;
		Body &rb_box = *bi.CreateBody(rb_box_settings);
		BodyID rb_id = rb_box.GetID();
		bi.AddBody(rb_id, EActivation::Activate);

		// Simulate for 3 seconds while applying the same force
		constexpr int cNumSteps = 180;
		const Vec3 cForce(10000.0f, 0, 0);
		for (int i = 0; i < cNumSteps; ++i)
		{
			bi.AddForce(sb_id, cForce, EActivation::Activate);
			bi.AddForce(rb_id, cForce, EActivation::Activate);
			c.SimulateSingleStep();
		}

		// Check that the rigid body moved as expected
		const float cTotalTime = cNumSteps * c.GetStepDeltaTime();
		const Vec3 cAcceleration = cForce / cMass;
		const RVec3 cExpectedPos = c.PredictPosition(cRBBoxPos, Vec3::sZero(), cAcceleration, cTotalTime);
		CHECK_APPROX_EQUAL(rb_box.GetPosition(), cExpectedPos);
		const Vec3 cExpectedVel = cAcceleration * cTotalTime;
		CHECK_APPROX_EQUAL(rb_box.GetLinearVelocity(), cExpectedVel, 1.0e-3f);
		CHECK_APPROX_EQUAL(rb_box.GetAngularVelocity(), Vec3::sZero());

		// Check that the soft body moved within 1% of that
		const RVec3 cExpectedPosSB = cExpectedPos - cRBBoxPos;
		CHECK_APPROX_EQUAL(sb_box.GetPosition(), cExpectedPosSB, 0.01f * cExpectedPosSB.Length());
		CHECK_APPROX_EQUAL(sb_box.GetLinearVelocity(), cExpectedVel, 2.0e-3f);
		CHECK_APPROX_EQUAL(sb_box.GetAngularVelocity(), Vec3::sZero(), 0.01f);
	}

	// Test that a rigid body sphere with high speed tunnels through a soft body cube when using discrete motion quality,
	// but collides and transfers momentum correctly when using linear cast motion quality. Test that we can affect
	// this behavior with the soft body contact listener.
	TEST_CASE("TestLinearCastSphereVsSoftBody")
	{
		constexpr float cDeltaTime = 1.0f / 60.0f;
		constexpr float cSphereMass = 200.0f;
		constexpr float cSoftBodyMass = 6.0f * 6.0f * 6.0f; // * 1 kg

		// Sphere starts at x = 2 and moves by -5 in one time step, towards the soft body cube at the origin (spans [-0.5, 0.5])
		// Velocity is high enough to tunnel through the cube in one simulation step
		const RVec3 cSphereStartPos(2.0f, 0, 0);
		const Vec3 cSphereVelocity(-5.0f / cDeltaTime, 0, 0);

		// Expensive to make, create outside of loop
		RefConst<SoftBodySharedSettings> cube = SoftBodySharedSettings::sCreateCube(6, 0.2f);

		enum class Test
		{
			NoContactListener,
			WithContactListener,
			WithRejectingContactListener
		};

		for (Test test : { Test::NoContactListener, Test::WithContactListener, Test::WithRejectingContactListener })
			for (EMotionQuality motion_quality : { EMotionQuality::Discrete, EMotionQuality::LinearCast })
			{
				PhysicsTestContext c(cDeltaTime, 1);
				c.ZeroGravity();
				BodyInterface &bi = c.GetBodyInterface();

				// Create rigid body sphere approaching the cube
				BodyCreationSettings sphere_settings(new SphereShape(1.0f), cSphereStartPos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				sphere_settings.mMotionQuality = motion_quality;
				sphere_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				sphere_settings.mMassPropertiesOverride.mMass = cSphereMass;
				sphere_settings.mLinearVelocity = cSphereVelocity;
				sphere_settings.mLinearDamping = 0.0f;
				Body &sphere = c.CreateBody(sphere_settings, EActivation::Activate);
				CHECK_APPROX_EQUAL(sphere.GetMotionProperties()->GetInverseMass(), 1.0f / cSphereMass);

				// Create soft body cube at origin (size 1 with 5 subdivisions)
				SoftBodyCreationSettings sb_settings(cube, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
				Body &soft_body = *bi.CreateSoftBody(sb_settings);
				bi.AddBody(soft_body.GetID(), EActivation::Activate);
				CHECK_APPROX_EQUAL(soft_body.GetMotionProperties()->GetInverseMass(), 1.0f / cSoftBodyMass);

				LoggingContactListener listener;
				c.GetSystem()->SetContactListener(&listener);

				// Optionally install a soft body contact listener
				LoggingSoftBodyContactListener soft_body_listener;
				if (test != Test::NoContactListener)
					c.GetSystem()->SetSoftBodyContactListener(&soft_body_listener);
				if (test == Test::WithRejectingContactListener)
					soft_body_listener.SetValidateValueToReturn(SoftBodyValidateResult::RejectContact);

				// Simulate one step
				c.SimulateSingleStep();

				// We shouldn't be receiving any contacts on the normal contact listener
				CHECK(listener.GetEntryCount() == 0);

				if (motion_quality == EMotionQuality::Discrete || test == Test::WithRejectingContactListener)
				{
					// With discrete motion  quality / rejected contacts the sphere should have tunneled through the soft body without any collision
					CHECK_APPROX_EQUAL(sphere.GetPosition(), cSphereStartPos + cDeltaTime * cSphereVelocity);
					CHECK_APPROX_EQUAL(sphere.GetLinearVelocity(), cSphereVelocity);
					CHECK(soft_body.GetLinearVelocity() == Vec3::sZero());

					// No contacts should have been detected
					if (motion_quality == EMotionQuality::Discrete)
						CHECK(soft_body_listener.GetEntryCount() == 0);
					else
					{
						CHECK(soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Validate, soft_body.GetID(), sphere.GetID())); // Should have gotten a validate that we rejected
						CHECK(!soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Add, soft_body.GetID(), sphere.GetID()));
					}
				}
				else // LinearCast
				{
					// With linear cast motion quality the sphere should have collided with the soft body and stopped there (we do time stealing)
					CHECK_APPROX_EQUAL(sphere.GetPosition(), RVec3(1.5f, 0, 0), 1.001f * c.GetSystem()->GetPhysicsSettings().mPenetrationSlop);

					// The soft body should have gained velocity in the direction the sphere was traveling
					CHECK(soft_body.GetLinearVelocity().GetX() < 0.0f);

					// Check that momentum is conserved (GetLinearVelocity returns the center of mass velocity of the soft body)
					// Note that because we apply the collision impulse to a face, some rotational velocity will be introduced and we lose some linear momentum
					float initial_momentum = cSphereMass * cSphereVelocity.GetX();
					float final_momentum = cSphereMass * sphere.GetLinearVelocity().GetX() + cSoftBodyMass * soft_body.GetLinearVelocity().GetX();
					CHECK_APPROX_EQUAL(final_momentum, initial_momentum, 0.05f * abs(initial_momentum));

					if (test == Test::NoContactListener)
						CHECK(soft_body_listener.GetEntryCount() == 0);
					else
					{
						// We should have received one contact for the soft body
						CHECK(soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Validate, soft_body.GetID(), sphere.GetID()));
						CHECK(soft_body_listener.Contains(LoggingSoftBodyContactListener::EType::Add, soft_body.GetID(), sphere.GetID()));
					}
				}
			}
	}

	// Test that long range attachment constraints are created correctly and limit the stretch of a cloth
	TEST_CASE("TestLRAConstraints")
	{
		static constexpr uint cNumX = 5, cNumZ = 10;
		constexpr float cSpacing = 0.1f;
		auto inv_mass = [](uint, uint inZ) { return inZ == 0? 0.0f : 1.0f; };
		auto vertex_index = [](uint inX, uint inZ) { return inX + inZ * cNumX; };

		// Simulate a hanging cloth and return the biggest distance between the top and bottom row
		auto simulate = [vertex_index](const SoftBodySharedSettings *inSettings) {
			PhysicsTestContext c;
			SoftBodyCreationSettings sb_settings(inSettings, RVec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), Layers::MOVING);
			sb_settings.mAllowSleeping = false;
			Body &body = *c.GetBodyInterface().CreateSoftBody(sb_settings);
			c.GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
			c.Simulate(2.0f);

			// Check that the LRA constraints are satisfied
			for (const SoftBodySharedSettings::LRA &lra : inSettings->mLRAConstraints)
			{
				float distance = float((sGetVertexPosition(body, lra.mVertex[1]) - sGetVertexPosition(body, lra.mVertex[0])).Length());
				CHECK(distance <= lra.mMaxDistance * 1.001f);
			}

			float max_distance = 0.0f;
			for (uint x = 0; x < cNumX; ++x)
				max_distance = max(max_distance, float((sGetVertexPosition(body, vertex_index(x, cNumZ - 1)) - sGetVertexPosition(body, vertex_index(x, 0))).Length()));
			return max_distance;
		};

		// Without LRA constraints, a very stretchy cloth should stretch a lot
		const float cRestLength = cSpacing * (cNumZ - 1);
		{
			Ref<SoftBodySharedSettings> settings = sCreateCloth(cNumX, cNumZ, cSpacing, inv_mass, VertexAttributes(1.0e-2f, 1.0e-2f, FLT_MAX, ELRAType::None), EBendType::None);
			CHECK(settings->mLRAConstraints.empty());
			settings->Optimize();
			CHECK(simulate(settings) > 1.5f * cRestLength);
		}

		for (ELRAType lra_type : { ELRAType::EuclideanDistance, ELRAType::GeodesicDistance })
		{
			Ref<SoftBodySharedSettings> settings = sCreateCloth(cNumX, cNumZ, cSpacing, inv_mass, VertexAttributes(1.0e-2f, 1.0e-2f, FLT_MAX, lra_type), EBendType::None);

			// Each dynamic vertex should be attached to the kinematic vertex above it
			CHECK(settings->mLRAConstraints.size() == cNumX * (cNumZ - 1));
			for (const SoftBodySharedSettings::LRA &lra : settings->mLRAConstraints)
			{
				CHECK(settings->mVertices[lra.mVertex[0]].mInvMass == 0.0f);
				CHECK(settings->mVertices[lra.mVertex[1]].mInvMass > 0.0f);
				CHECK(lra.mVertex[0] == lra.mVertex[1] % cNumX);
				CHECK_APPROX_EQUAL(lra.mMaxDistance, cSpacing * (lra.mVertex[1] / cNumX), 1.0e-5f);
			}

			// Recalculate the lengths with a multiplier
			settings->CalculateLRALengths(1.1f);
			for (const SoftBodySharedSettings::LRA &lra : settings->mLRAConstraints)
				CHECK_APPROX_EQUAL(lra.mMaxDistance, 1.1f * cSpacing * (lra.mVertex[1] / cNumX), 1.0e-5f);
			settings->CalculateLRALengths();

			// With LRA constraints the cloth should not stretch beyond its rest length
			settings->Optimize();
			CHECK(simulate(settings) <= cRestLength * 1.001f);
		}
	}

	// Test calculating the properties of Cosserat rods and simulating them
	TEST_CASE("TestCosseratRods")
	{
		Ref<SoftBodySharedSettings> settings = sCreateRodChain();

		// The last rod should have been flipped so that it points in the same direction as the previous rod
		const Array<SoftBodySharedSettings::RodStretchShear> &rods = settings->mRodStretchShearConstraints;
		CHECK(rods[3].mVertex[0] == 3);
		CHECK(rods[3].mVertex[1] == 4);

		for (const SoftBodySharedSettings::RodStretchShear &r : rods)
		{
			// Check that the length and inverse mass are correct
			CHECK_APPROX_EQUAL(r.mLength, 1.0f);
			CHECK(r.mInvMass == min(settings->mVertices[r.mVertex[0]].mInvMass, settings->mVertices[r.mVertex[1]].mInvMass));

			// The Bishop frame Z axis should point along the rod
			Vec3 tangent = (Vec3(settings->mVertices[r.mVertex[1]].mPosition) - Vec3(settings->mVertices[r.mVertex[0]].mPosition)).Normalized();
			CHECK(r.mBishop.IsNormalized());
			CHECK_APPROX_EQUAL(r.mBishop.RotateAxisZ(), tangent, 1.0e-5f);
		}

		// The first two rods are aligned so should have the same Bishop frame
		CHECK_APPROX_EQUAL(rods[0].mBishop, rods[1].mBishop, 1.0e-5f);

		// For the bent rod, the frame should be rotated around the bend axis, so the axis perpendicular to both rods should be the same in both frames
		Vec3 bend_axis = Vec3::sAxisZ();
		CHECK_APPROX_EQUAL(rods[1].mBishop.Conjugated() * bend_axis, rods[2].mBishop.Conjugated() * bend_axis, 1.0e-5f);

		// Check the rest rotation between rods
		for (const SoftBodySharedSettings::RodBendTwist &bt : settings->mRodBendTwistConstraints)
			CHECK_APPROX_EQUAL(bt.mOmega0, (rods[bt.mRod[0]].mBishop.Conjugated() * rods[bt.mRod[1]].mBishop).Normalized(), 1.0e-5f);

		settings->Optimize();

		// Simulate the chain under gravity, the first rod is static because its first vertex is static
		PhysicsTestContext c;
		SoftBodyCreationSettings sb_settings(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		sb_settings.mAllowSleeping = false;
		sb_settings.mNumIterations = 20;
		Body &body = *c.GetBodyInterface().CreateSoftBody(sb_settings);
		c.GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
		SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());

		// Initial state of the rods should match the Bishop frames
		for (uint r = 0; r < rods.size(); ++r)
		{
			CHECK_APPROX_EQUAL(mp->GetRodRotation(r), rods[r].mBishop);
			CHECK(mp->GetRodAngularVelocity(r) == Vec3::sZero());
		}

		c.Simulate(2.0f);

		// The bend twist constraints should prevent the chain from drooping
		CHECK_APPROX_EQUAL(sGetVertexPosition(body, 4), RVec3(2, -2, 0), 0.1f);

		for (uint r = 0; r < rods.size(); ++r)
		{
			// Rods should keep their length
			Vec3 delta = mp->GetVertex(rods[r].mVertex[1]).mPosition - mp->GetVertex(rods[r].mVertex[0]).mPosition;
			CHECK_APPROX_EQUAL(delta.Length(), 1.0f, 1.0e-2f);

			// The rotation of the rod should follow the vertices
			CHECK(mp->GetRodRotation(r).RotateAxisZ().Dot(delta.Normalized()) > 0.99f);
		}

		// The bend and twist constraints should keep the shape of the chain
		CHECK_APPROX_EQUAL((mp->GetVertex(3).mPosition - mp->GetVertex(1).mPosition).Length(), sqrt(2.0f), 0.1f);
		CHECK_APPROX_EQUAL((mp->GetVertex(4).mPosition - mp->GetVertex(2).mPosition).Length(), 2.0f, 0.1f);
	}

	// Test skinned constraints
	TEST_CASE("TestSkinnedConstraints")
	{
		constexpr uint cNumX = 5, cNumZ = 5;
		constexpr float cMaxDistance = 0.1f;
		constexpr float cBackStopDistance = 0.05f;

		for (bool use_back_stop : { false, true })
		{
			PhysicsTestContext c;

			// Create a cloth with the first row kinematic
			Ref<SoftBodySharedSettings> settings = sCreateCloth(cNumX, cNumZ, 0.2f, [](uint, uint inZ) { return inZ == 0? 0.0f : 1.0f; }, VertexAttributes(1.0e-3f, 1.0e-3f, FLT_MAX));

			// Skin all vertices to a single joint
			settings->mInvBindMatrices.push_back(SoftBodySharedSettings::InvBind(0, Mat44::sIdentity()));
			for (uint v = 0; v < settings->mVertices.size(); ++v)
			{
				bool kinematic = settings->mVertices[v].mInvMass == 0.0f;
				SoftBodySharedSettings::Skinned skinned(v, kinematic? 0.0f : cMaxDistance, use_back_stop? cBackStopDistance : FLT_MAX, 40.0f);
				skinned.mWeights[0] = SoftBodySharedSettings::SkinWeight(0, 1.0f);
				settings->mSkinnedConstraints.push_back(skinned);
			}
			settings->CalculateSkinnedConstraintNormals();
			settings->Optimize();

			// All vertices are surrounded by faces
			for (const SoftBodySharedSettings::Skinned &s : settings->mSkinnedConstraints)
				CHECK((s.mNormalInfo >> 24) > 0);

			SoftBodyCreationSettings sb_settings(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
			sb_settings.mAllowSleeping = false;
			sb_settings.mUpdatePosition = false;
			Body &body = *c.GetBodyInterface().CreateSoftBody(sb_settings);
			c.GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
			SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());

			auto skin = [&c, &body, mp](Vec3Arg inTranslation, bool inHardSkinAll) {
				Mat44 joint = Mat44::sTranslation(inTranslation);
				mp->SkinVertices(body.GetCenterOfMassTransform(), &joint, 1, inHardSkinAll, *c.GetTempAllocator());
			};

			// Displace the vertices and hard skin them back
			for (SoftBodyVertex &v : mp->GetVertices())
			{
				v.mPosition += Vec3(0, 1, 0);
				v.mVelocity = Vec3(1, 2, 3);
			}
			skin(Vec3::sZero(), true);
			for (uint v = 0; v < settings->mVertices.size(); ++v)
			{
				CHECK(mp->GetVertex(v).mPosition == Vec3(settings->mVertices[v].mPosition));
				CHECK(mp->GetVertex(v).mVelocity == Vec3::sZero());
			}

			// Let the cloth hang under gravity
			for (int i = 0; i < 60; ++i)
			{
				skin(Vec3::sZero(), false);
				c.SimulateSingleStep();
			}

			// Check that the vertices are limited by the max distance and the back stop
			float min_y = FLT_MAX;
			for (uint v = 0; v < settings->mVertices.size(); ++v)
			{
				Vec3 delta = mp->GetVertex(v).mPosition - Vec3(settings->mVertices[v].mPosition);
				CHECK(delta.Length() <= cMaxDistance + 1.0e-3f);
				min_y = min(min_y, delta.GetY());
			}
			if (use_back_stop)
			{
				// The back stop is a large sphere below the cloth, it should stop the vertices from moving down too much
				CHECK(min_y >= -cBackStopDistance - 1.0e-3f);
				CHECK(min_y < -0.5f * cBackStopDistance);
			}
			else
			{
				// Without back stop, the vertices should hang lower
				CHECK(min_y < -0.9f * cMaxDistance);
			}

			// Move the joint up and check that the cloth follows
			const Vec3 cTranslation(0, 1, 0);
			for (int i = 1; i <= 60; ++i)
			{
				skin(cTranslation * min(1.0f, i / 30.0f), false);
				c.SimulateSingleStep();
			}
			for (uint v = 0; v < settings->mVertices.size(); ++v)
			{
				Vec3 delta = mp->GetVertex(v).mPosition - (Vec3(settings->mVertices[v].mPosition) + cTranslation);
				if (settings->mVertices[v].mInvMass == 0.0f)
					CHECK_APPROX_EQUAL(delta, Vec3::sZero(), 1.0e-5f);
				else
					CHECK(delta.Length() <= cMaxDistance + 1.0e-3f);
			}

			// With a max distance multiplier of 0 all vertices should be hard skinned
			mp->SetSkinnedMaxDistanceMultiplier(0.0f);
			CHECK(mp->GetSkinnedMaxDistanceMultiplier() == 0.0f);
			skin(cTranslation, false);
			c.SimulateSingleStep();
			for (uint v = 0; v < settings->mVertices.size(); ++v)
				CHECK_APPROX_EQUAL(mp->GetVertex(v).mPosition, Vec3(settings->mVertices[v].mPosition) + cTranslation, 1.0e-5f);
			mp->SetSkinnedMaxDistanceMultiplier(1.0f);

			// When skin constraints are disabled, skinning only moves the kinematic vertices
			mp->SetEnableSkinConstraints(false);
			CHECK(!mp->GetEnableSkinConstraints());
			Array<Vec3> positions_before;
			for (const SoftBodyVertex &v : mp->GetVertices())
				positions_before.push_back(v.mPosition);
			const Vec3 cTranslation2(0, 2, 0);
			skin(cTranslation2, false);
			for (uint v = 0; v < settings->mVertices.size(); ++v)
				if (settings->mVertices[v].mInvMass == 0.0f)
					CHECK(mp->GetVertex(v).mPosition == Vec3(settings->mVertices[v].mPosition) + cTranslation2);
				else
					CHECK(mp->GetVertex(v).mPosition == positions_before[v]);
		}
	}

	// Test that pressure inflates a soft body
	TEST_CASE("TestPressure")
	{
		PhysicsTestContext c;
		c.ZeroGravity();

		// Create a cube without volume constraints and with soft edges
		Ref<SoftBodySharedSettings> cube = SoftBodySharedSettings::sCreateCube(5, 0.25f);
		cube->mVolumeConstraints.clear();
		for (SoftBodySharedSettings::Edge &e : cube->mEdgeConstraints)
			e.mCompliance = 1.0e-4f;
		cube->Optimize();

		Body *bodies[2];
		for (int i = 0; i < 2; ++i)
		{
			SoftBodyCreationSettings sb_settings(cube, RVec3(i * 3.0f, 0, 0), Quat::sIdentity(), Layers::MOVING);
			sb_settings.mAllowSleeping = false;
			sb_settings.mPressure = i == 0? 0.0f : 1000.0f;
			bodies[i] = c.GetBodyInterface().CreateSoftBody(sb_settings);
			c.GetBodyInterface().AddBody(bodies[i]->GetID(), EActivation::Activate);

			// Check initial volume
			const SoftBodyMotionProperties *mp = static_cast<const SoftBodyMotionProperties *>(bodies[i]->GetMotionProperties());
			CHECK_APPROX_EQUAL(mp->GetVolume(), 1.0f, 1.0e-5f);
		}

		c.Simulate(1.0f);

		// Without pressure the volume should not change, with pressure it should increase
		CHECK_APPROX_EQUAL(static_cast<const SoftBodyMotionProperties *>(bodies[0]->GetMotionProperties())->GetVolume(), 1.0f, 1.0e-3f);
		CHECK(static_cast<const SoftBodyMotionProperties *>(bodies[1]->GetMotionProperties())->GetVolume() > 1.05f);
	}

	// Test serializing shared settings
	TEST_CASE("TestSharedSettingsSaveRestore")
	{
		Ref<SoftBodySharedSettings> settings = sCreateSettingsWithAllConstraintTypes();
		CHECK(!settings->mEdgeConstraints.empty());
		CHECK(!settings->mDihedralBendConstraints.empty());
		CHECK(!settings->mVolumeConstraints.empty());
		CHECK(!settings->mSkinnedConstraints.empty());
		CHECK(!settings->mLRAConstraints.empty());
		CHECK(!settings->mRodStretchShearConstraints.empty());
		CHECK(!settings->mRodBendTwistConstraints.empty());

		// Clone should be identical
		Ref<SoftBodySharedSettings> clone = settings->Clone();
		sCheckSettingsEqual(*settings, *clone);

		// Save / restore should be identical
		Ref<SoftBodySharedSettings> restored = new SoftBodySharedSettings;
		{
			stringstream data;
			StreamOutWrapper stream_out(data);
			settings->SaveBinaryState(stream_out);
			StreamInWrapper stream_in(data);
			restored->RestoreBinaryState(stream_in);
			CHECK(!stream_in.IsFailed());
		}
		sCheckSettingsEqual(*settings, *restored);

		// Save / restore with materials, saving the same settings twice should only store it once
		{
			Ref<PhysicsMaterial> material = new PhysicsMaterial;
			settings->mMaterials = { material };

			stringstream data;
			StreamOutWrapper stream_out(data);
			SoftBodySharedSettings::SharedSettingsToIDMap settings_map;
			SoftBodySharedSettings::MaterialToIDMap material_map;
			settings->SaveWithMaterials(stream_out, settings_map, material_map);
			settings->SaveWithMaterials(stream_out, settings_map, material_map);
			CHECK(settings_map.size() == 1);
			CHECK(material_map.size() == 1);

			StreamInWrapper stream_in(data);
			SoftBodySharedSettings::IDToSharedSettingsMap id_to_settings;
			SoftBodySharedSettings::IDToMaterialMap id_to_material;
			SoftBodySharedSettings::SettingsResult result1 = SoftBodySharedSettings::sRestoreWithMaterials(stream_in, id_to_settings, id_to_material);
			CHECK(result1.IsValid());
			SoftBodySharedSettings::SettingsResult result2 = SoftBodySharedSettings::sRestoreWithMaterials(stream_in, id_to_settings, id_to_material);
			CHECK(result2.IsValid());
			CHECK(result1.Get() == result2.Get());
			sCheckSettingsEqual(*settings, *result1.Get());
			CHECK(result1.Get()->mMaterials.size() == 1);
			CHECK(result1.Get()->mMaterials[0] != nullptr);

			// Reading past the end should fail
			SoftBodySharedSettings::SettingsResult result3 = SoftBodySharedSettings::sRestoreWithMaterials(stream_in, id_to_settings, id_to_material);
			CHECK(result3.HasError());
		}

		// Simulate the original and the restored settings, they should behave identically (this also checks the private data)
		Array<Vec3> positions[2];
		const SoftBodySharedSettings *to_simulate[2] = { settings, restored };
		for (int i = 0; i < 2; ++i)
		{
			PhysicsTestContext c;
			SoftBodyCreationSettings sb_settings(to_simulate[i], RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
			sb_settings.mAllowSleeping = false;
			Body &body = *c.GetBodyInterface().CreateSoftBody(sb_settings);
			c.GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
			SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());
			Mat44 joint = Mat44::sIdentity();
			mp->SkinVertices(body.GetCenterOfMassTransform(), &joint, 1, true, *c.GetTempAllocator());
			c.Simulate(0.5f);
			for (const SoftBodyVertex &v : mp->GetVertices())
			{
				CHECK(!v.mPosition.IsNaN());
				positions[i].push_back(v.mPosition);
			}
		}
		CHECK(positions[0] == positions[1]);
	}

	// Test saving and restoring the state of soft bodies
	TEST_CASE("TestSoftBodySaveRestoreState")
	{
		PhysicsTestContext c;
		BodyInterface &bi = c.GetBodyInterface();

		// Create a soft body with all constraint types and a rod chain
		Ref<SoftBodySharedSettings> settings[] = { sCreateSettingsWithAllConstraintTypes(), sCreateRodChain() };
		settings[1]->Optimize();
		Body *bodies[2];
		for (int i = 0; i < 2; ++i)
		{
			SoftBodyCreationSettings sb_settings(settings[i], RVec3(i * 5.0f, 0, 0), Quat::sIdentity(), Layers::MOVING);
			sb_settings.mAllowSleeping = false;
			bodies[i] = bi.CreateSoftBody(sb_settings);
			bi.AddBody(bodies[i]->GetID(), EActivation::Activate);
		}

		SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(bodies[0]->GetMotionProperties());
		auto skin = [&c, mp, &bodies](int inStep) {
			Mat44 joint = Mat44::sTranslation(Vec3(0, 0.01f * inStep, 0));
			mp->SkinVertices(bodies[0]->GetCenterOfMassTransform(), &joint, 1, inStep == 0, *c.GetTempAllocator());
		};
		skin(0);

		// Simulate a bit and save the state
		for (int i = 0; i < 10; ++i)
		{
			skin(i);
			c.SimulateSingleStep();
		}
		StateRecorderImpl state;
		c.GetSystem()->SaveState(state);

		// Simulate further and record the result
		auto simulate_and_record = [&]() {
			Array<Vec3> result;
			for (int i = 10; i < 30; ++i)
			{
				skin(i);
				c.SimulateSingleStep();
			}
			for (const Body *b : bodies)
			{
				const SoftBodyMotionProperties *bmp = static_cast<const SoftBodyMotionProperties *>(b->GetMotionProperties());
				for (const SoftBodyVertex &v : bmp->GetVertices())
				{
					result.push_back(v.mPosition);
					result.push_back(v.mVelocity);
				}
				for (uint r = 0; r < (uint)bmp->GetSettings()->mRodStretchShearConstraints.size(); ++r)
				{
					result.push_back(bmp->GetRodRotation(r).GetXYZ());
					result.push_back(bmp->GetRodAngularVelocity(r));
				}
			}
			return result;
		};
		Array<Vec3> expected = simulate_and_record();

		// Restore the state and simulate again, the result should be the same
		state.Rewind();
		CHECK(c.GetSystem()->RestoreState(state));
		Array<Vec3> actual = simulate_and_record();
		CHECK(expected == actual);
	}

	// Test updating a soft body that is not part of the physics system
	TEST_CASE("TestCustomUpdate")
	{
		PhysicsTestContext c;
		BodyInterface &bi = c.GetBodyInterface();

		SoftBodyCreationSettings sb_settings(SoftBodySharedSettings::sCreateCube(4, 0.25f), RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		sb_settings.mLinearDamping = 0.0f;
		Body &body = *bi.CreateSoftBody(sb_settings);
		CHECK(!body.IsInBroadPhase());
		SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());

		// Update the body manually for 1 second
		const int cNumSteps = 60;
		for (int i = 0; i < cNumSteps; ++i)
			mp->CustomUpdate(c.GetDeltaTime(), body, *c.GetSystem());

		// The body should have fallen
		float total_time = cNumSteps * c.GetDeltaTime();
		RVec3 expected_position = c.PredictPosition(RVec3::sZero(), Vec3::sZero(), c.GetSystem()->GetGravity(), total_time);
		CHECK_APPROX_EQUAL(body.GetPosition(), expected_position, 0.05f * float(expected_position.Length()));
		CHECK_APPROX_EQUAL(body.GetLinearVelocity(), c.GetSystem()->GetGravity() * total_time, 0.05f * 9.81f * total_time);

		bi.DestroyBody(body.GetID());
	}

	// Test a soft body interacting with sensors
	TEST_CASE("TestSoftBodyVsSensor")
	{
		PhysicsTestContext c;
		c.ZeroGravity();
		BodyInterface &bi = c.GetBodyInterface();

		LoggingSoftBodyContactListener listener;
		c.GetSystem()->SetSoftBodyContactListener(&listener);

		// Soft body cube at the origin, spans [-0.5, 0.5]
		SoftBodyCreationSettings sb_settings(SoftBodySharedSettings::sCreateCube(5, 0.25f), RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		sb_settings.mAllowSleeping = false;
		Body &soft_body = *bi.CreateSoftBody(sb_settings);
		bi.AddBody(soft_body.GetID(), EActivation::Activate);

		// Sensor that overlaps with the soft body
		BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(0.3f)), RVec3(0.5f, 0, 0), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor_settings.mIsSensor = true;
		BodyID sensor_id = bi.CreateAndAddBody(sensor_settings, EActivation::DontActivate);

		// Sensor whose bounding box overlaps with the soft body but that doesn't contain any vertices
		BodyCreationSettings sensor2_settings(new SphereShape(0.05f), RVec3(0.125f, 0, 0), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
		sensor2_settings.mIsSensor = true;
		BodyID sensor2_id = bi.CreateAndAddBody(sensor2_settings, EActivation::DontActivate);

		c.SimulateSingleStep();

		// Both sensors should have been validated, but only the first one should be in contact
		CHECK(listener.Contains(LoggingSoftBodyContactListener::EType::Validate, soft_body.GetID(), sensor_id));
		CHECK(listener.Contains(LoggingSoftBodyContactListener::EType::Add, soft_body.GetID(), sensor_id));
		CHECK(listener.Contains(LoggingSoftBodyContactListener::EType::Validate, soft_body.GetID(), sensor2_id));
		CHECK(!listener.Contains(LoggingSoftBodyContactListener::EType::Add, soft_body.GetID(), sensor2_id));

		// Sensors should not affect the soft body
		for (const SoftBodyVertex &v : static_cast<const SoftBodyMotionProperties *>(soft_body.GetMotionProperties())->GetVertices())
			CHECK(v.mVelocity == Vec3::sZero());
	}

	// Test a soft body resting on a (scaled) convex hull
	TEST_CASE("TestSoftBodyVsConvexHull")
	{
		const Vec3 cHalfExtent(2.5f, 0.25f, 2.5f);
		Array<Vec3> points;
		for (int i = 0; i < 8; ++i)
			points.push_back(cHalfExtent * Vec3(i & 1? 1.0f : -1.0f, i & 2? 1.0f : -1.0f, i & 4? 1.0f : -1.0f));
		RefConst<Shape> hull = ConvexHullShapeSettings(points).Create().Get();

		// Unscaled hull with twice the size, scaled hull and inside out scaled hull should all give the same result
		Array<Vec3> big_points;
		for (Vec3 p : points)
			big_points.push_back(2.0f * p);
		RefConst<Shape> shapes[] = { ConvexHullShapeSettings(big_points).Create().Get(), new ScaledShape(hull, Vec3::sReplicate(2.0f)), new ScaledShape(hull, Vec3(-2, 2, 2)) };

		for (const Shape *shape : shapes)
		{
			PhysicsTestContext c;
			BodyInterface &bi = c.GetBodyInterface();

			// Floor with top at y = 0
			bi.CreateAndAddBody(BodyCreationSettings(shape, RVec3(0, -0.5f, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

			// Drop a soft body cube on it
			SoftBodyCreationSettings sb_settings(SoftBodySharedSettings::sCreateCube(5, 0.25f), RVec3(0, 1, 0), Quat::sIdentity(), Layers::MOVING);
			Body &soft_body = *bi.CreateSoftBody(sb_settings);
			bi.AddBody(soft_body.GetID(), EActivation::Activate);

			c.Simulate(2.0f);

			// Check that the soft body rests on the floor
			const SoftBodyMotionProperties *mp = static_cast<const SoftBodyMotionProperties *>(soft_body.GetMotionProperties());
			float min_y = FLT_MAX;
			for (uint v = 0; v < mp->GetVertices().size(); ++v)
				min_y = min(min_y, float(sGetVertexPosition(soft_body, v).GetY()));
			CHECK_APPROX_EQUAL(min_y, 0.0f, 1.0e-5f);
		}
	}
}
