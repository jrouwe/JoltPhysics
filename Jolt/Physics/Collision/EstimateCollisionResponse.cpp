// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/EstimateCollisionResponse.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Constraints/ConstraintPart/ContactConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AngularFrictionConstraintPart.h>

JPH_NAMESPACE_BEGIN

void EstimateCollisionResponse(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, CollisionEstimationResult &outResult, const ContactSettings &inContactSettings, float inMinVelocityForRestitution, uint inNumIterations)
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

	if (inContactSettings.mIsSensor)
	{
		// Sensors don't have a collision response
		outResult.mFrictionPoint = Vec3::sZero();
		outResult.mFrictionImpulse = Vec3::sZero();
		outResult.mAngularFrictionImpulse = 0.0f;
		outResult.mContactImpulse.resize(num_points);
		for (uint c = 0; c < num_points; ++c)
			outResult.mContactImpulse[c] = 0.0f;
		return;
	}

	// Get inverse mass and inertia
	float inv_m1, inv_m2;
	Mat44 inv_i1, inv_i2;
	if (motion_type1 == EMotionType::Dynamic)
	{
		inv_m1 = inContactSettings.mInvMassScale1 * motion_properties1->GetInverseMass();
		inv_i1 = inContactSettings.mInvInertiaScale1 * inBody1.GetInverseInertia();
	}
	else
	{
		inv_m1 = 0.0f;
		inv_i1 = Mat44::sZero();
	}

	if (motion_type2 == EMotionType::Dynamic)
	{
		inv_m2 = inContactSettings.mInvMassScale2 * motion_properties2->GetInverseMass();
		inv_i2 = inContactSettings.mInvInertiaScale2 * inBody2.GetInverseInertia();
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
	ContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic> contact_constraints[ContactPoints::Capacity];
	Vec3 friction_point1 = Vec3::sZero(), friction_point2 = Vec3::sZero();
	for (uint c = 0; c < num_points; ++c)
	{
		Vec3 p1 = inManifold.mRelativeContactPointsOn1[c];
		Vec3 p2 = inManifold.mRelativeContactPointsOn2[c];

		// Calculate friction lever arm points per body so that the friction lever arm is not perturbed
		// by the normal-direction separation of a speculative contact
		friction_point1 += p1;
		friction_point2 += p2;

		// Calculate contact point relative to com
		Vec3 r1 = p1 - com1;
		Vec3 r2 = p2 - com2;

		// Handle elastic collisions
		float bias = 0.0f;
		if (inContactSettings.mCombinedRestitution > 0.0f)
		{
			// Calculate velocity of contact point
			Vec3 relative_velocity = outResult.mLinearVelocity2 + outResult.mAngularVelocity2.Cross(r2) - outResult.mLinearVelocity1 - outResult.mAngularVelocity1.Cross(r1);
			float normal_velocity = relative_velocity.Dot(inManifold.mWorldSpaceNormal);

			// If it is big enough, apply restitution
			if (normal_velocity < -inMinVelocityForRestitution)
				bias = inContactSettings.mCombinedRestitution * normal_velocity;
		}

		// Initialize contact constraint
		ContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic> &constraint = contact_constraints[c];
		constraint.SetTotalLambda(0.0f);
		constraint.CalculateConstraintProperties(inv_m1, inv_i1, r1, inv_m2, inv_i2, r2, inManifold.mWorldSpaceNormal, bias);
	}

	// Calculate distance to friction center for each point
	float num_points_f = float(num_points);
	friction_point1 /= num_points_f;
	friction_point2 /= num_points_f;
	float distance_to_friction_center[ContactPoints::Capacity];
	for (uint c = 0; c < num_points; ++c)
	{
		Vec3 delta = inManifold.mRelativeContactPointsOn1[c] - friction_point1;
		distance_to_friction_center[c] = (delta - delta.Dot(inManifold.mWorldSpaceNormal) * inManifold.mWorldSpaceNormal).Length();
	}
	outResult.mFrictionPoint = 0.5f * (friction_point1 + friction_point2);

	// Initialize friction constraints
	ContactConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic> friction1, friction2;
	AngularFrictionConstraintPart<EMotionType::Dynamic, EMotionType::Dynamic> angular_friction;
	angular_friction.SetTotalLambda(0.0f);
	friction1.SetTotalLambda(0.0f);
	friction2.SetTotalLambda(0.0f);
	if (inContactSettings.mCombinedFriction > 0.0f)
	{
		Vec3 r1 = friction_point1 - com1;
		Vec3 r2 = friction_point2 - com2;

		// Get surface velocity relative to tangents
		Vec3 ws_surface_velocity = inContactSettings.mRelativeLinearSurfaceVelocity + inContactSettings.mRelativeAngularSurfaceVelocity.Cross(friction_point1);
		float surface_velocity1 = tangent1.Dot(ws_surface_velocity);
		float surface_velocity2 = tangent2.Dot(ws_surface_velocity);

		friction1.CalculateConstraintProperties(inv_m1, inv_i1, r1, inv_m2, inv_i2, r2, tangent1, surface_velocity1);
		friction2.CalculateConstraintProperties(inv_m1, inv_i1, r1, inv_m2, inv_i2, r2, tangent2, surface_velocity2);

		if (num_points > 1)
			angular_friction.CalculateConstraintProperties(inv_i1, inv_i2, inManifold.mWorldSpaceNormal, inContactSettings.mRelativeAngularSurfaceVelocity.Dot(inManifold.mWorldSpaceNormal));
	}

	// If there's only 1 contact point, we only need 1 iteration
	int num_iterations = inContactSettings.mCombinedFriction <= 0.0f && num_points == 1? 1 : inNumIterations;

	// Solve iteratively
	for (int iteration = 0; iteration < num_iterations; ++iteration)
	{
		// Solve friction constraints first
		if (inContactSettings.mCombinedFriction > 0.0f)
		{
			// Calculate max impulse that can be applied
			float max_linear_lambda = 0.0f, max_angular_lambda = 0.0f;
			for (uint c = 0; c < num_points; ++c)
			{
				float lambda = contact_constraints[c].GetTotalLambda();
				max_linear_lambda += lambda;
				max_angular_lambda += distance_to_friction_center[c] * lambda;
			}
			max_linear_lambda *= inContactSettings.mCombinedFriction;
			max_angular_lambda *= inContactSettings.mCombinedFriction;

			// Calculate impulse to stop motion in tangential direction
			float lambda1 = friction1.SolveVelocityConstraintGetTotalLambda(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, tangent1);
			float lambda2 = friction2.SolveVelocityConstraintGetTotalLambda(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, tangent2);

			// If the total lambda that we will apply is too large, scale it back
			float total_lambda_sq = Square(lambda1) + Square(lambda2);
			if (total_lambda_sq > Square(max_linear_lambda) + FLT_MIN) // ensure total_lambda_sq > FLT_MIN to avoid division by zero in MulRSqrtApproximate
			{
				float scale = MulRSqrtApproximate(max_linear_lambda, total_lambda_sq);
				lambda1 *= scale;
				lambda2 *= scale;
			}

			// Apply the friction impulse
			friction1.SolveVelocityConstraintApplyLambda(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, inv_m1, inv_m2, tangent1, lambda1);
			friction2.SolveVelocityConstraintApplyLambda(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, inv_m1, inv_m2, tangent2, lambda2);

			// Apply angular friction
			if (num_points > 1)
				angular_friction.SolveVelocityConstraint(outResult.mAngularVelocity1, outResult.mAngularVelocity2, inManifold.mWorldSpaceNormal, -max_angular_lambda, max_angular_lambda);
		}

		// Solve contact constraints last
		for (uint c = 0; c < num_points; ++c)
			contact_constraints[c].SolveVelocityConstraint(outResult.mLinearVelocity1, outResult.mAngularVelocity1, outResult.mLinearVelocity2, outResult.mAngularVelocity2, inv_m1, inv_m2, inManifold.mWorldSpaceNormal, 0.0f, FLT_MAX);
	}

	// Store impulses
	outResult.mContactImpulse.resize(num_points);
	for (uint c = 0; c < num_points; ++c)
		outResult.mContactImpulse[c] = contact_constraints[c].GetTotalLambda();
	outResult.mFrictionImpulse = friction1.GetTotalLambda() * tangent1 + friction2.GetTotalLambda() * tangent2;
	outResult.mAngularFrictionImpulse = angular_friction.GetTotalLambda();
}

JPH_NAMESPACE_END
