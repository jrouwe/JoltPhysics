// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/PhysicsSystem.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

void SoftBodyMotionProperties::Initialize(const SoftBodyCreationSettings &inSettings)
{
	// TODO: Set a sensible mass and inertia
	SetInverseMass(0.0f);
	SetInverseInertia(Vec3::sZero(), Quat::sIdentity());

	// Store settings
	mSettings = inSettings.mSettings;
	mNumIterations = inSettings.mNumIterations;
	mPressure = inSettings.mPressure;
	mUpdatePosition = inSettings.mUpdatePosition;

	// Initialize vertices
	mVertices.resize(inSettings.mSettings->mVertices.size());
	for (Array<SoftBodyMotionProperties::Vertex>::size_type v = 0, s = mVertices.size(); v < s; ++v)
	{
		const SoftBodyParticleSettings::Vertex &in_vertex = inSettings.mSettings->mVertices[v];
		SoftBodyMotionProperties::Vertex &out_vertex = mVertices[v];
		out_vertex.mPreviousPosition = out_vertex.mPosition = Vec3(in_vertex.mPosition);
		out_vertex.mVelocity = Vec3(in_vertex.mVelocity);
		out_vertex.mInvMass = in_vertex.mInvMass;
		mLocalBounds.Encapsulate(out_vertex.mPosition);
	}

	// We don't know delta time yet, so we can't predict the bounds and use the local bounds as the predicted bounds
	mLocalPredictedBounds = mLocalBounds;
}

void SoftBodyMotionProperties::Update(float inDeltaTime, Body &inSoftBody, Vec3 &outDeltaPosition, PhysicsSystem &inSystem)
{
	// Based on: XPBD, Extended Position Based Dynamics, Matthias Muller, Ten Minute Physics
	// See: https://matthias-research.github.io/pages/tenMinutePhysics/09-xpbd.pdf

	// Convert gravity to local space
	RMat44 body_transform = inSoftBody.GetCenterOfMassTransform();
	Vec3 gravity = body_transform.Multiply3x3Transposed(GetGravityFactor() * inSystem.GetGravity());

	// Collect information about the colliding bodies
	struct CollidingShape
	{
		/// Get the velocity of a point on this body
		Vec3			GetPointVelocity(Vec3Arg inPointRelativeToCOM) const
		{
			return mLinearVelocity + mAngularVelocity.Cross(inPointRelativeToCOM);
		}

		Vec3			mCenterOfMassPosition;
		Mat44			mInverseShapeTransform;
		RefConst<Shape>	mShape;
		BodyID			mBodyID;
		EMotionType		mMotionType;
		float			mInvMass;
		float			mFriction;
		float			mRestitution;
		bool 			mUpdateVelocities;
		Mat44			mInvInertia;
		Vec3			mLinearVelocity;
		Vec3			mAngularVelocity;
	};
	struct Collector : public CollideShapeBodyCollector
	{
									Collector(Body &inSoftBody, RMat44Arg inTransform, PhysicsSystem &inSystem) :
										mSoftBody(inSoftBody),
										mInverseTransform(inTransform.InversedRotationTranslation()),
										mBodyLockInterface(inSystem.GetBodyLockInterfaceNoLock()),
										mCombineFriction(inSystem.GetCombineFriction()),
										mCombineRestitution(inSystem.GetCombineRestitution())
		{
		}

		virtual void				AddHit(const BodyID &inResult) override
		{
			BodyLockRead lock(mBodyLockInterface, inResult);
			if (lock.Succeeded())
			{
				const Body &body = lock.GetBody();
				if (body.IsRigidBody() // TODO: We should support soft body vs soft body
					&& mSoftBody.GetCollisionGroup().CanCollide(body.GetCollisionGroup()))
				{
					CollidingShape cs;
					Mat44 shape_transform = (mInverseTransform * body.GetCenterOfMassTransform()).ToMat44();
					cs.mCenterOfMassPosition = Vec3(shape_transform.GetTranslation());
					cs.mInverseShapeTransform = shape_transform.InversedRotationTranslation();
					cs.mShape = body.GetShape();
					cs.mBodyID = inResult;
					cs.mMotionType = body.GetMotionType();
					cs.mUpdateVelocities = false;
					cs.mFriction = mCombineFriction(mSoftBody, SubShapeID(), body, SubShapeID());
					cs.mRestitution = mCombineRestitution(mSoftBody, SubShapeID(), body, SubShapeID());
					if (cs.mMotionType == EMotionType::Dynamic)
					{
						const MotionProperties *mp = body.GetMotionProperties();
						cs.mInvMass = mp->GetInverseMass();
						cs.mInvInertia = mp->GetInverseInertiaForRotation(shape_transform.GetRotation());
						cs.mLinearVelocity = mp->GetLinearVelocity();
						cs.mAngularVelocity = mp->GetAngularVelocity();
					}
					mHits.push_back(cs);
				}
			}
		}

		Body &						mSoftBody;
		RMat44						mInverseTransform;
		const BodyLockInterface &	mBodyLockInterface;
		ContactConstraintManager::CombineFunction mCombineFriction;
		ContactConstraintManager::CombineFunction mCombineRestitution;
		Array<CollidingShape>		mHits;
	};
	Collector collector(inSoftBody, body_transform, inSystem);
	AABox bounds = mLocalBounds;
	bounds.Encapsulate(mLocalPredictedBounds);
	bounds = bounds.Transformed(body_transform);
	DefaultBroadPhaseLayerFilter broadphase_layer_filter = inSystem.GetDefaultBroadPhaseLayerFilter(inSoftBody.GetObjectLayer());
	DefaultObjectLayerFilter object_layer_filter = inSystem.GetDefaultLayerFilter(inSoftBody.GetObjectLayer());
	inSystem.GetBroadPhaseQuery().CollideAABox(bounds, collector, broadphase_layer_filter, object_layer_filter);

	// Calculate delta time for sub step
	float dt = inDeltaTime / mNumIterations;
	float dt_sq = Square(dt);

	// Calculate total displacement we'll have due to gravity over all sub steps
	// The total displacement as produced by our integrator can be written as: Sum(i * g * dt^2, i = 0..mNumIterations).
	// This is bigger than 0.5 * g * dt^2 because we first increment the velocity and then update the position
	// Using Sum(i, i = 0..n) = n * (n + 1) / 2 we can write this as:
	Vec3 displacement_due_to_gravity = (0.5f * mNumIterations * (mNumIterations + 1) * dt_sq) * gravity;

	// Generate collision planes
	if (collector.mHits.empty())
	{
		// No collisions
		for (Vertex &v : mVertices)
			v.mCollidingShapeIndex = -1;
	}
	else
	{
		// Process collisions
		for (Vertex &v : mVertices)
			if (v.mInvMass > 0.0f)
			{
				// Start with no collision
				v.mCollidingShapeIndex = -1;

				// Calculate the distance we will move this frame
				Vec3 movement = v.mVelocity * inDeltaTime + displacement_due_to_gravity;

				// Create a collision plane for each vertex
				float largest_penetration = -FLT_MAX;
				for (const CollidingShape &cs : collector.mHits)
				{
					// TODO: Needs to be implemented on the shape itself
					if (cs.mShape->GetSubType() == EShapeSubType::Sphere)
					{
						// Special case for spheres
						const SphereShape *sphere = static_cast<const SphereShape *>(cs.mShape.GetPtr());
						float radius = sphere->GetRadius();
						Vec3 delta = v.mPosition - cs.mCenterOfMassPosition;
						float distance = delta.Length();
						float penetration = radius - distance;
						if (penetration > largest_penetration)
						{
							largest_penetration = penetration;
							Vec3 point, normal;
							if (distance > 0.0f)
							{
								point = cs.mCenterOfMassPosition + delta * (radius / distance);
								normal = delta / distance;
							}
							else
							{
								point = cs.mCenterOfMassPosition + Vec3(0, radius, 0);
								normal = Vec3::sAxisY();
							}
							v.mCollisionPlane = Plane::sFromPointAndNormal(point, normal);
							v.mCollidingShapeIndex = int(&cs - collector.mHits.data());
						}
					}
					else
					{
						// Fallback
						RayCastResult hit;
						hit.mFraction = 2.0f; // Add a little extra distance in case the particle speeds up
						RayCast ray(v.mPosition - 0.5f * movement, movement);
						RayCast local_ray(ray.Transformed(cs.mInverseShapeTransform));
						if (cs.mShape->CastRay(local_ray, SubShapeIDCreator(), hit))
						{
							float penetration = (hit.mFraction - 0.5f) * movement.Length();
							if (penetration > largest_penetration)
							{
								// Store collision
								largest_penetration = penetration;
								Vec3 point = ray.GetPointOnRay(hit.mFraction);
								Vec3 normal = cs.mInverseShapeTransform.Multiply3x3Transposed(cs.mShape->GetSurfaceNormal(hit.mSubShapeID2, cs.mInverseShapeTransform * point));
								v.mCollisionPlane = Plane::sFromPointAndNormal(point, normal);
								v.mCollidingShapeIndex = int(&cs - collector.mHits.data());
							}
						}
					}
				}
			}
	}

	float inv_dt_sq = 1.0f / dt_sq;
	float linear_damping = max(0.0f, 1.0f - GetLinearDamping() * dt); // See: MotionProperties::ApplyForceTorqueAndDragInternal

	for (uint iteration = 0; iteration < mNumIterations; ++iteration)
	{
		float pressure_coefficient = mPressure;
		if (pressure_coefficient > 0.0f)
		{
			// Calculate total volume
			float six_volume = 0.0f;
			for (const Face &f : mSettings->mFaces)
			{
				Vec3 x1 = mVertices[f.mVertex[0]].mPosition;
				Vec3 x2 = mVertices[f.mVertex[1]].mPosition;
				Vec3 x3 = mVertices[f.mVertex[2]].mPosition;
				six_volume += x1.Cross(x2).Dot(x3); // We pick zero as the origin as this is the center of the bounding box so should give good accuracy
			}
			if (six_volume > 0.0f)
			{
				// Apply pressure
				// p = F / A = n R T / V (see https://en.wikipedia.org/wiki/Pressure)
				// Our pressure coefficient is n R T so the impulse is:
				// P = F dt = pressure_coefficient / V * A * dt
				float coefficient = pressure_coefficient * dt / six_volume; // Need to still multiply by 6 for the volume
				for (const Face &f : mSettings->mFaces)
				{
					Vec3 x1 = mVertices[f.mVertex[0]].mPosition;
					Vec3 x2 = mVertices[f.mVertex[1]].mPosition;
					Vec3 x3 = mVertices[f.mVertex[2]].mPosition;

					Vec3 impulse = coefficient * (x2 - x1).Cross(x3 - x1); // Area is half the cross product so need to still divide by 2
					for (int i = 0; i < 3; ++i)
					{
						Vertex &v = mVertices[f.mVertex[i]];
						v.mVelocity += v.mInvMass * impulse; // Want to divide by 3 because we spread over 3 vertices
					}
				}
			}
		}

		// Integrate
		Vec3 sub_step_gravity = gravity * dt;
		for (Vertex &v : mVertices)
			if (v.mInvMass > 0.0f)
			{
				// Gravity
				v.mVelocity += sub_step_gravity;

				// Damping
				v.mVelocity *= linear_damping;

				// Integrate
				v.mPreviousPosition = v.mPosition;
				v.mPosition += v.mVelocity * dt;

				// Reset projected distance
				v.mProjectedDistance = 0.0f;
			}
			else
			{
				// Integrate
				v.mPreviousPosition = v.mPosition;
				v.mPosition += v.mVelocity * dt;
			}

		// Satisfy volume constraints
		for (const Volume &v : mSettings->mVolumeConstraints)
		{
			Vertex &v1 = mVertices[v.mVertex[0]];
			Vertex &v2 = mVertices[v.mVertex[1]];
			Vertex &v3 = mVertices[v.mVertex[2]];
			Vertex &v4 = mVertices[v.mVertex[3]];

			Vec3 x1 = v1.mPosition;
			Vec3 x2 = v2.mPosition;
			Vec3 x3 = v3.mPosition;
			Vec3 x4 = v4.mPosition;

			// Calculate constraint equation
			Vec3 x1x2 = x2 - x1;
			Vec3 x1x3 = x3 - x1;
			Vec3 x1x4 = x4 - x1;
			float c = abs(x1x2.Cross(x1x3).Dot(x1x4)) - v.mSixRestVolume;

			// Calculate gradient of constraint equation
			Vec3 d1c = (x4 - x2).Cross(x3 - x2);
			Vec3 d2c = x1x3.Cross(x1x4);
			Vec3 d3c = x1x4.Cross(x1x2);
			Vec3 d4c = x1x2.Cross(x1x3);

			float w1 = v1.mInvMass;
			float w2 = v2.mInvMass;
			float w3 = v3.mInvMass;
			float w4 = v4.mInvMass;
			JPH_ASSERT(w1 > 0.0f || w2 > 0.0f || w3 > 0.0f || w4 > 0.0f);

			// Apply correction
			float lambda = -c / (w1 * d1c.LengthSq() + w2 * d2c.LengthSq() + w3 * d3c.LengthSq() + w4 * d4c.LengthSq() + v.mCompliance * inv_dt_sq);
			v1.mPosition += lambda * w1 * d1c;
			v2.mPosition += lambda * w2 * d2c;
			v3.mPosition += lambda * w3 * d3c;
			v4.mPosition += lambda * w4 * d4c;
		}

		// Satisfy edge constraints
		for (const Edge &e : mSettings->mEdgeConstraints)
		{
			Vertex &v0 = mVertices[e.mVertex[0]];
			Vertex &v1 = mVertices[e.mVertex[1]];

			// Calculate current length
			Vec3 delta = v1.mPosition - v0.mPosition;
			float length = delta.Length();
			if (length > 0.0f)
			{
				// Apply correction
				Vec3 correction = delta * (length - e.mRestLength) / (length * (v0.mInvMass + v1.mInvMass + e.mCompliance * inv_dt_sq));
				v0.mPosition += v0.mInvMass * correction;
				v1.mPosition -= v1.mInvMass * correction;
			}
		}

		// Satisfy collision
		for (Vertex &v : mVertices)
			if (v.mCollidingShapeIndex >= 0)
			{
				float distance = v.mCollisionPlane.SignedDistance(v.mPosition);
				if (distance < 0.0f)
				{
					Vec3 delta = v.mCollisionPlane.GetNormal() * distance;
					v.mPosition -= delta;
					v.mPreviousPosition -= delta; // Apply delta to previous position so that we will not accumulate velocity by being pushed out of collision
					v.mProjectedDistance -= distance; // For friction calculation
				}
			}

		// Update velocity
		float restitution_treshold = -2.0f * gravity.Length() * dt;
		for (Vertex &v : mVertices)
			if (v.mInvMass > 0.0f)
			{
				Vec3 prev_v = v.mVelocity;

				// XPBD velocity update
				v.mVelocity = (v.mPosition - v.mPreviousPosition) / dt;

				// If there was a collision
				if (v.mProjectedDistance > 0.0f)
				{
					JPH_ASSERT(v.mCollidingShapeIndex >= 0);
					CollidingShape &cs = collector.mHits[v.mCollidingShapeIndex];

					// Apply friction as described in Detailed Rigid Body Simulation with Extended Position Based Dynamics - Matthias Muller et al.
					// See section 3.6:
					// Inverse mass: w1 = 1 / m1, w2 = 1 / m2 + (r2 x n)^T I^-1 (r2 x n) = 0 for a static object
					// r2 are the contact point relative to the center of mass of body 2
					// Lagrange multiplier for contact: lambda = -c / (w1 + w2)
					// Where c is the constraint equation (the distance to the plane, negative because penetrating)
					// Contact normal force: fn = lambda / dt^2
					// Delta velocity due to friction dv = -vt / |vt| * min(dt * friction * fn * (w1 + w2), |vt|) = -vt * min(-friction * c / (|vt| * dt), 1)
					// Note that I think there is an error in the paper, I added a mass term, see: https://github.com/matthias-research/pages/issues/29
					// Relative velocity: vr = v1 - v2 - omega2 x r2
					// Normal velocity: vn = vr . contact_normal
					// Tangential velocity: vt = vr - contact_normal * vn
					// Impulse: p = dv / (w1 + w2)
					// Changes in particle velocities:
					// v1 = v1 + p / m1
					// v2 = v2 - p / m2 (no change when colliding with a static body)
					// w2 = w2 - I^-1 (r2 x p) (no change when colliding with a static body)
					Vec3 contact_normal = v.mCollisionPlane.GetNormal();
					if (cs.mMotionType == EMotionType::Dynamic)
					{
						// Calculate normal and tangential velocity (equation 30)
						Vec3 r2 = v.mPosition - cs.mCenterOfMassPosition;
						Vec3 v2 = cs.GetPointVelocity(r2);
						Vec3 relative_velocity = v.mVelocity - v2;
						Vec3 v_normal = contact_normal * contact_normal.Dot(relative_velocity);
						Vec3 v_tangential = relative_velocity - v_normal;
						float v_tangential_length = v_tangential.Length();

						// Calculate inverse effective mass
						Vec3 r2_cross_n = r2.Cross(contact_normal);
						float w2 = cs.mInvMass + r2_cross_n.Dot(cs.mInvInertia * r2_cross_n);
						float w1_plus_w2 = v.mInvMass + w2;

						// Calculate delta relative velocity due to friction (modified equation 31)
						Vec3 dv;
						if (v_tangential_length > 0.0f)
							dv = v_tangential * min(cs.mFriction * v.mProjectedDistance / (v_tangential_length * dt), 1.0f);
						else
							dv = Vec3::sZero();

						// Calculate delta relative velocity due to restitution (equation 35)
						dv += v_normal;
						float prev_v_normal = (prev_v - v2).Dot(contact_normal);
						if (prev_v_normal < restitution_treshold)
							dv += cs.mRestitution * prev_v_normal * contact_normal;

						// Calculate impulse
						Vec3 p = dv / w1_plus_w2;

						// Apply impulse to particle
						v.mVelocity -= p * v.mInvMass;

						// Apply impulse to rigid body
						cs.mLinearVelocity += p * cs.mInvMass;
						cs.mAngularVelocity += cs.mInvInertia * r2.Cross(p);

						// Mark that the velocities of the body we hit need to be updated
						cs.mUpdateVelocities = true;
					}
					else
					{
						// Body is not moveable, equations are simpler

						// Calculate normal and tangential velocity (equation 30)
						Vec3 v_normal = contact_normal * contact_normal.Dot(v.mVelocity);
						Vec3 v_tangential = v.mVelocity - v_normal;
						float v_tangential_length = v_tangential.Length();

						// Apply friction (modified equation 31)
						if (v_tangential_length > 0.0f)
							v.mVelocity -= v_tangential * min(cs.mFriction * v.mProjectedDistance / (v_tangential_length * dt), 1.0f);

						// Apply restitution (equation 35)
						v.mVelocity -= v_normal;
						float prev_v_normal = prev_v.Dot(contact_normal);
						if (prev_v_normal < restitution_treshold)
							v.mVelocity -= cs.mRestitution * prev_v_normal * contact_normal;
					}
				}
			}
		}

	// Update local bounding box
	mLocalPredictedBounds = mLocalBounds = { };
	for (Vertex &v : mVertices)
	{
		mLocalBounds.Encapsulate(v.mPosition);

		// Create predicted position for the next frame in order to detect collisions before they happen
		mLocalPredictedBounds.Encapsulate(v.mPosition + v.mVelocity * inDeltaTime + displacement_due_to_gravity);
	}

	if (mUpdatePosition)
	{
		// Shift the body so that the position is the center of the local bounds
		Vec3 delta = mLocalBounds.GetCenter();
		outDeltaPosition = body_transform.Multiply3x3(delta);
		for (Vertex &v : mVertices)
			v.mPosition -= delta;

		// Offset bounds to match new position
		mLocalBounds.Translate(-delta);
		mLocalPredictedBounds.Translate(-delta);
	}
	else
		outDeltaPosition = Vec3::sZero();

	// Write back velocities
	const BodyLockInterface &body_lock_interface = inSystem.GetBodyLockInterfaceNoLock();
	for (const CollidingShape &cs : collector.mHits)
		if (cs.mUpdateVelocities)
		{
			BodyLockWrite lock(body_lock_interface, cs.mBodyID);
			if (lock.Succeeded())
			{
				Body &body = lock.GetBody();
				body.SetLinearVelocityClamped(cs.mLinearVelocity);
				body.SetAngularVelocityClamped(cs.mAngularVelocity);
			}
		}
}

#ifdef JPH_DEBUG_RENDERER

void SoftBodyMotionProperties::DrawVertices(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const
{
	for (const Vertex &v : mVertices)
		inRenderer->DrawMarker(inCenterOfMassTransform * v.mPosition, Color::sRed, 0.05f);
}

void SoftBodyMotionProperties::DrawEdgeConstraints(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const
{
	for (const Edge &e : mSettings->mEdgeConstraints)
		inRenderer->DrawLine(inCenterOfMassTransform * mVertices[e.mVertex[0]].mPosition, inCenterOfMassTransform * mVertices[e.mVertex[1]].mPosition, Color::sWhite);
}

void SoftBodyMotionProperties::DrawVolumeConstraints(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const
{
	for (const Volume &v : mSettings->mVolumeConstraints)
	{
		RVec3 x1 = inCenterOfMassTransform * mVertices[v.mVertex[0]].mPosition;
		RVec3 x2 = inCenterOfMassTransform * mVertices[v.mVertex[1]].mPosition;
		RVec3 x3 = inCenterOfMassTransform * mVertices[v.mVertex[2]].mPosition;
		RVec3 x4 = inCenterOfMassTransform * mVertices[v.mVertex[3]].mPosition;

		inRenderer->DrawTriangle(x1, x3, x2, Color::sYellow, DebugRenderer::ECastShadow::On);
		inRenderer->DrawTriangle(x2, x3, x4, Color::sYellow, DebugRenderer::ECastShadow::On);
		inRenderer->DrawTriangle(x1, x4, x3, Color::sYellow, DebugRenderer::ECastShadow::On);
		inRenderer->DrawTriangle(x1, x2, x4, Color::sYellow, DebugRenderer::ECastShadow::On);
	}
}

void SoftBodyMotionProperties::DrawPredictedBounds(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const
{
	inRenderer->DrawWireBox(inCenterOfMassTransform, mLocalPredictedBounds, Color::sRed);
}

#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_END
