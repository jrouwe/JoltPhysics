// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/EstimateCollisionResponse.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Constraints/ConstraintPart/ContactConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AngularFrictionConstraintPart.h>

JPH_NAMESPACE_BEGIN

void EstimateCollisionResponse(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, CollisionEstimationResult &outResult, float inCombinedFriction, float inCombinedRestitution, float inMinVelocityForRestitution, uint inNumIterations)
{
	ContactPoints::size_type num_points = inManifold.mRelativeContactPointsOn1.size();
	JPH_ASSERT(num_points == inManifold.mRelativeContactPointsOn2.size());

	// Calculate friction directions
	Vec3 tangent1 = inManifold.mWorldSpaceNormal.GetNormalizedPerpendicular();
	Vec3 tangent2 = inManifold.mWorldSpaceNormal.Cross(tangent1);

	// Get body velocities
	EMotionType motion_type1 = inBody1.GetMotionType();
	const MotionProperties *motion_properties1 = inBody1.GetMotionPropertiesUnchecked();
	if (motion_type1 != EMotionType::Static)
	{
		outResult.mLinearVelocity1 = motion_properties1->GetLinearVelocity();
		outResult.mAngularVelocity1 = motion_properties1->GetAngularVelocity();
	}
	else
		outResult.mLinearVelocity1 = outResult.mAngularVelocity1 = Vec3::sZero();

	EMotionType motion_type2 = inBody2.GetMotionType();
	const MotionProperties *motion_properties2 = inBody2.GetMotionPropertiesUnchecked();
	if (motion_type2 != EMotionType::Static)
	{
		outResult.mLinearVelocity2 = motion_properties2->GetLinearVelocity();
		outResult.mAngularVelocity2 = motion_properties2->GetAngularVelocity();
	}
	else
		outResult.mLinearVelocity2 = outResult.mAngularVelocity2 = Vec3::sZero();

	// Get inverse mass and inertia
	float inv_m1, inv_m2;
	Mat44 inv_i1, inv_i2;
	if (motion_type1 == EMotionType::Dynamic)
	{
		inv_m1 = motion_properties1->GetInverseMass();
		inv_i1 = inBody1.GetInverseInertia();
	}
	else
	{
		inv_m1 = 0.0f;
		inv_i1 = Mat44::sZero();
	}

	if (motion_type2 == EMotionType::Dynamic)
	{
		inv_m2 = motion_properties2->GetInverseMass();
		inv_i2 = inBody2.GetInverseInertia();
	}
	else
	{
		inv_m2 = 0.0f;
		inv_i2 = Mat44::sZero();
	}

	// Get center of masses relative to the base offset
	Vec3 com1 = Vec3(inBody1.GetCenterOfMassPosition() - inManifold.mBaseOffset);
	Vec3 com2 = Vec3(inBody2.GetCenterOfMassPosition() - inManifold.mBaseOffset);

	// Initialize the constraint properties
	struct Contact
	{
		ContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic> mPart;
		Vec3		mR1;
		Vec3		mR2;
		float		mEffectiveMass;
		float		mBias;
		float		mDistanceToFrictionCenter;
		float		mNonPenetrationLambda;
	};
	Contact contact_constraints[ContactPoints::Capacity];
	Vec3 friction_r1 = Vec3::sZero();
	for (uint c = 0; c < num_points; ++c)
	{
		Contact &contact = contact_constraints[c];
		contact.mNonPenetrationLambda = 0.0f;

		// Calculate contact points relative to body 1 and 2
		Vec3 p = 0.5f * (inManifold.mRelativeContactPointsOn1[c] + inManifold.mRelativeContactPointsOn2[c]);

		// Calculate contact point relative to com
		Vec3 r1 = p - com1;
		Vec3 r2 = p - com2;
		contact.mR1 = r1;
		contact.mR2 = r2;

		// Calculate friction point
		friction_r1 += r1;

		// Handle elastic collisions
		contact.mBias = 0.0f;
		if (inCombinedRestitution > 0.0f)
		{
			// Calculate velocity of contact point
			Vec3 relative_velocity = outResult.mLinearVelocity2 + outResult.mAngularVelocity2.Cross(r2) - outResult.mLinearVelocity1 - outResult.mAngularVelocity1.Cross(r1);
			float normal_velocity = relative_velocity.Dot(inManifold.mWorldSpaceNormal);

			// If it is big enough, apply restitution
			if (normal_velocity < -inMinVelocityForRestitution)
				contact.mBias = inCombinedRestitution * normal_velocity;
		}

		// Initialize contact constraint
		contact.mEffectiveMass = 1.0f / contact.mPart.GetInvEffectiveMass(r1, r2, inv_m1, inv_i1, inv_m2, inv_i2, inManifold.mWorldSpaceNormal);
	}

	// Calculate distance to friction center for each point
	float num_points_f = float(num_points);
	friction_r1 /= num_points_f;
	for (uint c = 0; c < num_points; ++c)
	{
		Contact &contact = contact_constraints[c];
		Vec3 delta = contact.mR1 - friction_r1;
		contact.mDistanceToFrictionCenter = (delta - delta.Dot(inManifold.mWorldSpaceNormal) * inManifold.mWorldSpaceNormal).Length();
	}
	outResult.mFrictionPoint = com1 + friction_r1;
	Vec3 friction_r2 = friction_r1 + (com2 - com1);

	// Initialize friction constraints
	ContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic> friction1, friction2;
	AngularFrictionConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic> angular_friction;
	float friction1_effective_mass = 0.0f;
	float friction2_effective_mass = 0.0f;
	float angular_friction_effective_mass = 0.0f;
	if (inCombinedFriction > 0.0f)
	{
		friction1_effective_mass = 1.0f / friction1.GetInvEffectiveMass(friction_r1, friction_r2, inv_m1, inv_i1, inv_m2, inv_i2, tangent1);
		friction2_effective_mass = 1.0f / friction2.GetInvEffectiveMass(friction_r1, friction_r2, inv_m1, inv_i1, inv_m2, inv_i2, tangent2);

		if (num_points > 1)
			angular_friction_effective_mass = 1.0f / angular_friction.GetInvEffectiveMass(inv_i1, inv_i2, inManifold.mWorldSpaceNormal);
	}

	// If there's only 1 contact point, we only need 1 iteration
	int num_iterations = inCombinedFriction <= 0.0f && num_points == 1? 1 : inNumIterations;

	// Solve iteratively
	float angular_friction_lambda = 0.0f;
	float friction1_lambda = 0.0f;
	float friction2_lambda = 0.0f;
	for (int iteration = 0; iteration < num_iterations; ++iteration)
	{
		// Solve friction constraints first
		if (inCombinedFriction > 0.0f)
		{
			// Calculate max impulse that can be applied
			float max_linear_lambda = 0.0f, max_angular_lambda = 0.0f;
			for (uint c = 0; c < num_points; ++c)
			{
				const Contact &contact = contact_constraints[c];
				float lambda = contact.mNonPenetrationLambda;
				max_linear_lambda += lambda;
				max_angular_lambda += contact.mDistanceToFrictionCenter * lambda;
			}
			max_linear_lambda *= inCombinedFriction;
			max_angular_lambda *= inCombinedFriction;

			// Calculate impulse to stop motion in tangential direction
			float lambda1 = friction1_lambda + friction1_effective_mass * friction1.GetMinusJV(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, tangent1);
			float lambda2 = friction2_lambda + friction2_effective_mass * friction2.GetMinusJV(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, tangent2);

			// If the total lambda that we will apply is too large, scale it back
			float total_lambda_sq = Square(lambda1) + Square(lambda2);
			if (total_lambda_sq > Square(max_linear_lambda) + FLT_MIN) // ensure total_lambda_sq > FLT_MIN to avoid division by zero in MulRSqrtApproximate
			{
				float scale = MulRSqrtApproximate(max_linear_lambda, total_lambda_sq);
				lambda1 *= scale;
				lambda2 *= scale;
			}

			// Apply the friction impulse
			friction1.ApplyImpulse(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, inv_m1, inv_m2, lambda1 - friction1_lambda, tangent1);
			friction2.ApplyImpulse(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, inv_m1, inv_m2, lambda2 - friction2_lambda, tangent2);
			friction1_lambda = lambda1;
			friction2_lambda = lambda2;

			// Apply angular friction
			if (num_points > 1)
			{
				float lambda = angular_friction_lambda + angular_friction_effective_mass * angular_friction.sGetMinusJV(outResult.mAngularVelocity1, outResult.mAngularVelocity2, inManifold.mWorldSpaceNormal);
				lambda = Clamp(lambda, -max_angular_lambda, max_angular_lambda);
				angular_friction.ApplyImpulse(outResult.mAngularVelocity1, outResult.mAngularVelocity2, lambda - angular_friction_lambda);
				angular_friction_lambda = lambda;
			}
		}

		// Solve contact constraints last
		for (uint c = 0; c < num_points; ++c)
		{
			Contact &contact = contact_constraints[c];

			float lambda = contact.mNonPenetrationLambda + contact.mEffectiveMass * (contact.mPart.GetMinusJV(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, inManifold.mWorldSpaceNormal) - contact.mBias);
			contact.mPart.ApplyImpulse(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, inv_m1, inv_m2, lambda - contact.mNonPenetrationLambda, inManifold.mWorldSpaceNormal);
			contact.mNonPenetrationLambda = lambda;
		}
	}

	// Store impulses
	outResult.mContactImpulse.resize(num_points);
	for (uint c = 0; c < num_points; ++c)
		outResult.mContactImpulse[c] = contact_constraints[c].mNonPenetrationLambda;
	outResult.mFrictionImpulse = friction1_lambda * tangent1 + friction2_lambda * tangent2;
	outResult.mAngularFrictionImpulse = angular_friction_lambda;
}

JPH_NAMESPACE_END
