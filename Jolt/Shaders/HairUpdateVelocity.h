// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairApplyGlobalPose.h"

void ApplyCollisionAndUpdateVelocity(uint inVtx, JPH_IN_OUT(JPH_HairPosition) ioPos, JPH_IN(JPH_HairPosition) inPreviousPos, JPH_IN(JPH_HairMaterial) inMaterial, float inStrandFraction, JPH_OUT(JPH_HairVelocity) outVel)
{
	// Update velocities
	outVel.mVelocity = (ioPos.mPosition - inPreviousPos.mPosition) / cDeltaTime;
	outVel.mAngularVelocity = cTwoDivDeltaTime * JPH_QuatMulQuat(ioPos.mRotation, JPH_QuatConjugate(inPreviousPos.mRotation)).xyz;

	if (inMaterial.mEnableCollision)
	{
		// Calculate closest point on the collision plane
		JPH_HairCollisionPlane plane = gCollisionPlanes[inVtx];
		float distance_to_plane = JPH_PlaneSignedDistance(plane.mPlane, ioPos.mPosition);
		float3 contact_normal = JPH_PlaneGetNormal(plane.mPlane);
		float3 point_on_plane = ioPos.mPosition - distance_to_plane * contact_normal;

		// Calculate how much the plane moved in this time step
		JPH_HairCollisionShape shape = gCollisionShapes[plane.mShapeIndex];
		float3 plane_velocity = shape.mLinearVelocity + cross(shape.mAngularVelocity, point_on_plane - shape.mCenterOfMass);
		float plane_movement = dot(plane_velocity, contact_normal) * cAccumulatedDeltaTime;

		float projected_distance = -distance_to_plane + plane_movement + GradientSamplerSample(inMaterial.mHairRadius, inStrandFraction);
		if (projected_distance > 0.0f)
		{
			// Resolve penetration
			ioPos.mPosition += contact_normal * projected_distance;

			// Only update velocity when moving towards each other
			float3 v_relative = outVel.mVelocity - plane_velocity;
			float v_relative_dot_normal = dot(contact_normal, v_relative);
			if (v_relative_dot_normal < 0.0f)
			{
				// Calculate normal and tangential velocity (equation 30)
				float3 v_normal = contact_normal * v_relative_dot_normal;
				float3 v_tangential = v_relative - v_normal;
				float v_tangential_length = length(v_tangential);

				// Apply friction as described in Detailed Rigid Body Simulation with Extended Position Based Dynamics - Matthias Muller et al. (modified equation 31)
				if (v_tangential_length > 0.0f)
					outVel.mVelocity -= v_tangential * min(inMaterial.mFriction * projected_distance / (v_tangential_length * cDeltaTime), 1.0f);

				// Apply restitution of zero (equation 35)
				outVel.mVelocity -= v_normal;
			}
		}
	}
}

void LimitVelocity(JPH_IN_OUT(JPH_HairVelocity) ioVel, JPH_IN(JPH_HairMaterial) inMaterial)
{
	// Limit linear velocity
	float linear_velocity_sq = dot(ioVel.mVelocity, ioVel.mVelocity);
	if (linear_velocity_sq > inMaterial.mMaxLinearVelocitySq)
		ioVel.mVelocity *= sqrt(inMaterial.mMaxLinearVelocitySq / linear_velocity_sq);

	// Limit angular velocity
	float angular_velocity_sq = dot(ioVel.mAngularVelocity, ioVel.mAngularVelocity);
	if (angular_velocity_sq > inMaterial.mMaxAngularVelocitySq)
		ioVel.mAngularVelocity *= sqrt(inMaterial.mMaxAngularVelocitySq / angular_velocity_sq);
}
