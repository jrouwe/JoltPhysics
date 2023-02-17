// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/EstimateCollisionResponse.h>
#include <Jolt/Physics/Body/Body.h>

JPH_NAMESPACE_BEGIN

void EstimateCollisionResponse(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, Vec3 &outLinearVelocity1, Vec3 &outAngularVelocity1, Vec3 &outLinearVelocity2, Vec3 &outAngularVelocity2, ContactImpulses &outContactImpulses, float inCombinedRestitution, float inMinVelocityForRestitution, uint inNumIterations)
{
	// Note this code is based on AxisConstraintPart, see that class for more comments on the math

	// Start with zero impulses
	outContactImpulses.resize(inManifold.mRelativeContactPointsOn1.size());
	for (float &impulse : outContactImpulses)
		impulse = 0.0f;

	// Get body velocities
	EMotionType motion_type1 = inBody1.GetMotionType();
	const MotionProperties *motion_properties1 = inBody1.GetMotionPropertiesUnchecked();
	if (motion_type1 != EMotionType::Static)
	{
		outLinearVelocity1 = motion_properties1->GetLinearVelocity();
		outAngularVelocity1 = motion_properties1->GetAngularVelocity();
	}
	else
		outLinearVelocity1 = outAngularVelocity1 = Vec3::sZero();

	EMotionType motion_type2 = inBody2.GetMotionType();
	const MotionProperties *motion_properties2 = inBody2.GetMotionPropertiesUnchecked();
	if (motion_type2 != EMotionType::Static)
	{
		outLinearVelocity2 = motion_properties2->GetLinearVelocity();
		outAngularVelocity2 = motion_properties2->GetAngularVelocity();
	}
	else
		outLinearVelocity2 = outAngularVelocity2 = Vec3::sZero();

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
	Vec3 com1 = inBody1.GetCenterOfMassPosition() - inManifold.mBaseOffset;
	Vec3 com2 = inBody2.GetCenterOfMassPosition() - inManifold.mBaseOffset;

	struct ContactConstraint
	{
		Vec3		mR1PlusUxAxis;
		Vec3		mR2xAxis;
		Vec3		mInvI1_R1PlusUxAxis;
		Vec3		mInvI2_R2xAxis;
		float		mEffectiveMass;
		float		mBias;
	};

	// Initialize the constraint properties
	ContactConstraint constraints[ContactPoints::capacity()];
	JPH_ASSERT(inManifold.mRelativeContactPointsOn1.size() == inManifold.mRelativeContactPointsOn2.size());
	for (uint c = 0; c < inManifold.mRelativeContactPointsOn1.size(); ++c)
	{
		ContactConstraint &contact = constraints[c];

		// Calculate contact points relative to body 1 and 2
		Vec3 p = 0.5f * (inManifold.mRelativeContactPointsOn1[c] + inManifold.mRelativeContactPointsOn2[c]);
		Vec3 r1 = p - com1;
		Vec3 r2 = p - com2;

		// Calculate effective mass: K^-1 = (J M^-1 J^T)^-1
		contact.mR1PlusUxAxis = r1.Cross(inManifold.mWorldSpaceNormal);
		contact.mR2xAxis = r2.Cross(inManifold.mWorldSpaceNormal);
		contact.mInvI1_R1PlusUxAxis = inv_i1.Multiply3x3(contact.mR1PlusUxAxis);
		contact.mInvI2_R2xAxis = inv_i2.Multiply3x3(contact.mR2xAxis);
		contact.mEffectiveMass = 1.0f / (inv_m1 + contact.mInvI1_R1PlusUxAxis.Dot(contact.mR1PlusUxAxis) + inv_m2 + contact.mInvI2_R2xAxis.Dot(contact.mR2xAxis));

		// Handle elastic collisions
		contact.mBias = 0.0f;
		if (inCombinedRestitution > 0.0f)
		{
			// Calculate velocity of contact point
			Vec3 relative_velocity = outLinearVelocity2 + outAngularVelocity2.Cross(r2) - outLinearVelocity1 - outAngularVelocity1.Cross(r1);
			float normal_velocity = relative_velocity.Dot(inManifold.mWorldSpaceNormal);

			// If it is big enough, apply restitution
			if (normal_velocity < -inMinVelocityForRestitution)
				contact.mBias = inCombinedRestitution * normal_velocity;
		}
	}

	// If there's only 1 contact point, we only need 1 iteration
	int num_iterations = inManifold.mRelativeContactPointsOn1.size() == 1? 1 : inNumIterations;

	// Calculate the impulse needed to resolve the contacts
	for (int iteration = 0; iteration < num_iterations; ++iteration)
		for (uint c = 0; c < inManifold.mRelativeContactPointsOn1.size(); ++c)
		{
			ContactConstraint &contact = constraints[c];
			float &total_lambda = outContactImpulses[c];

			// Calculate jacobian multiplied by linear/angular velocity
			float jv = inManifold.mWorldSpaceNormal.Dot(outLinearVelocity1 - outLinearVelocity2) + contact.mR1PlusUxAxis.Dot(outAngularVelocity1) - contact.mR2xAxis.Dot(outAngularVelocity2);

			// Lagrange multiplier is:
			//
			// lambda = -K^-1 (J v + b)
			float lambda = contact.mEffectiveMass * (jv - contact.mBias);
			float new_lambda = max(total_lambda + lambda, 0.0f); // Clamp impulse
			lambda = new_lambda - total_lambda; // Lambda potentially got clamped, calculate the new impulse to apply
			total_lambda = new_lambda; // Store accumulated impulse

			// Apply impulse to body velocities
			outLinearVelocity1 -= (lambda * inv_m1) * inManifold.mWorldSpaceNormal;
			outAngularVelocity1 -= lambda * contact.mInvI1_R1PlusUxAxis;
			outLinearVelocity2 += (lambda * inv_m2) * inManifold.mWorldSpaceNormal;
			outAngularVelocity2 += lambda * contact.mInvI2_R2xAxis;
		}
}

JPH_NAMESPACE_END
