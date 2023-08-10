// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

void SoftBodyMotionProperties::CalculateMassAndInertia()
{
	MassProperties mp;

	for (const Vertex &v : mVertices)
		if (v.mInvMass > 0.0f)
		{
			Vec3 pos = v.mPosition;

			// Accumulate mass
			float mass = 1.0f / v.mInvMass;
			mp.mMass += mass;

			// Inertia tensor, diagonal
			// See equations https://en.wikipedia.org/wiki/Moment_of_inertia section 'Inertia Tensor'
			for (int i = 0; i < 3; ++i)
				mp.mInertia(i, i) += mass * (Square(pos[(i + 1) % 3]) + Square(pos[(i + 2) % 3]));

			// Inertia tensor off diagonal
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					if (i != j)
						mp.mInertia(i, j) -= mass * pos[i] * pos[j];
		}
		else
		{
			// If one vertex is kinematic, the entire body will have infinite mass and inertia
			SetInverseMass(0.0f);
			SetInverseInertia(Vec3::sZero(), Quat::sIdentity());
			return;
		}

	SetMassProperties(EAllowedDOFs::All, mp);
}

void SoftBodyMotionProperties::Initialize(const SoftBodyCreationSettings &inSettings)
{
	// Store settings
	mSettings = inSettings.mSettings;
	mNumIterations = inSettings.mNumIterations;
	mPressure = inSettings.mPressure;
	mUpdatePosition = inSettings.mUpdatePosition;

	// Initialize vertices
	mVertices.resize(inSettings.mSettings->mVertices.size());
	Mat44 rotation = inSettings.mMakeRotationIdentity? Mat44::sRotation(inSettings.mRotation) : Mat44::sIdentity();
	for (Array<Vertex>::size_type v = 0, s = mVertices.size(); v < s; ++v)
	{
		const SoftBodySharedSettings::Vertex &in_vertex = inSettings.mSettings->mVertices[v];
		Vertex &out_vertex = mVertices[v];
		out_vertex.mPreviousPosition = out_vertex.mPosition = rotation * Vec3(in_vertex.mPosition);
		out_vertex.mVelocity = rotation.Multiply3x3(Vec3(in_vertex.mVelocity));
		out_vertex.mCollidingShapeIndex = -1;
		out_vertex.mLargestPenetration = -FLT_MAX;
		out_vertex.mInvMass = in_vertex.mInvMass;
		out_vertex.mProjectedDistance = 0.0f;
		mLocalBounds.Encapsulate(out_vertex.mPosition);
	}

	// We don't know delta time yet, so we can't predict the bounds and use the local bounds as the predicted bounds
	mLocalPredictedBounds = mLocalBounds;

	CalculateMassAndInertia();
}

float SoftBodyMotionProperties::GetVolumeTimesSix() const
{
	float six_volume = 0.0f;
	for (const Face &f : mSettings->mFaces)
	{
		Vec3 x1 = mVertices[f.mVertex[0]].mPosition;
		Vec3 x2 = mVertices[f.mVertex[1]].mPosition;
		Vec3 x3 = mVertices[f.mVertex[2]].mPosition;
		six_volume += x1.Cross(x2).Dot(x3); // We pick zero as the origin as this is the center of the bounding box so should give good accuracy
	}
	return six_volume;
}

ECanSleep SoftBodyMotionProperties::Update(float inDeltaTime, Body &inSoftBody, Vec3 &outDeltaPosition, PhysicsSystem &inSystem)
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

		Mat44			mCenterOfMassTransform;				///< Transform of the body relative to the soft body
		RefConst<Shape>	mShape;
		BodyID			mBodyID;							///< Body ID of the body we hit
		EMotionType		mMotionType;						///< Motion type of the body we hit
		float			mInvMass;							///< Inverse mass of the body we hit
		float			mFriction;							///< Combined friction of the two bodies
		float			mRestitution;						///< Combined restitution of the two bodies
		bool 			mUpdateVelocities;					///< If the linear/angular velocity changed and the body needs to be updated
		Mat44			mInvInertia;						///< Inverse inertia in local space to the soft body
		Vec3			mLinearVelocity;					///< Linear velocity of the body in local space to the soft body
		Vec3			mAngularVelocity;					///< Angular velocity of the body in local space to the soft body
	};
	struct Collector : public CollideShapeBodyCollector
	{
									Collector(Body &inSoftBody, RMat44Arg inTransform, const PhysicsSystem &inSystem) :
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
					cs.mCenterOfMassTransform = (mInverseTransform * body.GetCenterOfMassTransform()).ToMat44();
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
						cs.mInvInertia = mp->GetInverseInertiaForRotation(cs.mCenterOfMassTransform.GetRotation());
						cs.mLinearVelocity = mInverseTransform.Multiply3x3(mp->GetLinearVelocity());
						cs.mAngularVelocity = mInverseTransform.Multiply3x3(mp->GetAngularVelocity());
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
	for (const CollidingShape &cs : collector.mHits)
		cs.mShape->CollideSoftBodyVertices(cs.mCenterOfMassTransform, Vec3::sReplicate(1.0f), mVertices, inDeltaTime, displacement_due_to_gravity, int(&cs - collector.mHits.data()));

	float inv_dt_sq = 1.0f / dt_sq;
	float linear_damping = max(0.0f, 1.0f - GetLinearDamping() * dt); // See: MotionProperties::ApplyForceTorqueAndDragInternal

	for (uint iteration = 0; iteration < mNumIterations; ++iteration)
	{
		float pressure_coefficient = mPressure;
		if (pressure_coefficient > 0.0f)
		{
			// Calculate total volume
			float six_volume = GetVolumeTimesSix();
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
					for (uint32 i : f.mVertex)
					{
						Vertex &v = mVertices[i];
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
						Vec3 r2 = v.mPosition - cs.mCenterOfMassTransform.GetTranslation();
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

	// Loop through vertices once more to update the global state
	float max_linear_velocity_sq = Square(GetMaxLinearVelocity());
	float max_v_sq = 0.0f;
	Vec3 linear_velocity = Vec3::sZero(), angular_velocity = Vec3::sZero();
	mLocalPredictedBounds = mLocalBounds = { };
	for (Vertex &v : mVertices)
	{
		// Calculate max square velocity
		float v_sq = v.mVelocity.LengthSq();
		max_v_sq = max(max_v_sq, v_sq);

		// Clamp if velocity is too high
		if (v_sq > max_linear_velocity_sq)
			v.mVelocity *= sqrt(max_linear_velocity_sq / v_sq);

		// Calculate local linear/angular velocity
		linear_velocity += v.mVelocity;
		angular_velocity += v.mPosition.Cross(v.mVelocity);

		// Update local bounding box
		mLocalBounds.Encapsulate(v.mPosition);

		// Create predicted position for the next frame in order to detect collisions before they happen
		mLocalPredictedBounds.Encapsulate(v.mPosition + v.mVelocity * inDeltaTime + displacement_due_to_gravity);

		// Reset collision data for the next iteration
		v.mCollidingShapeIndex = -1;
		v.mLargestPenetration = -FLT_MAX;
	}

	// Calculate linear/angular velocity of the body by averaging all vertices and bringing the value to world space
	float num_vertices_divider = float(max(int(mVertices.size()), 1));
	SetLinearVelocity(body_transform.Multiply3x3(linear_velocity / num_vertices_divider));
	SetAngularVelocity(body_transform.Multiply3x3(angular_velocity / num_vertices_divider));

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
	BodyInterface &body_interface = inSystem.GetBodyInterfaceNoLock();
	for (const CollidingShape &cs : collector.mHits)
		if (cs.mUpdateVelocities)
			body_interface.SetLinearAndAngularVelocity(cs.mBodyID, body_transform.Multiply3x3(cs.mLinearVelocity), body_transform.Multiply3x3(cs.mAngularVelocity));

	// Test if we should go to sleep
	if (!GetAllowSleeping())
		return ECanSleep::CannotSleep;

	const PhysicsSettings &physics_settings = inSystem.GetPhysicsSettings();
	if (max_v_sq > physics_settings.mPointVelocitySleepThreshold)
	{
		ResetSleepTestTimer();
		return ECanSleep::CannotSleep;
	}

	return AccumulateSleepTime(inDeltaTime, physics_settings.mTimeBeforeSleep);
}

#ifdef JPH_DEBUG_RENDERER

void SoftBodyMotionProperties::DrawVertices(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const
{
	for (const Vertex &v : mVertices)
		inRenderer->DrawMarker(inCenterOfMassTransform * v.mPosition, Color::sRed, 0.05f);
}

void SoftBodyMotionProperties::DrawEdgeConstraints(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const
{
	for (const Edge &e : mSettings->mEdgeConstraints)
		inRenderer->DrawLine(inCenterOfMassTransform * mVertices[e.mVertex[0]].mPosition, inCenterOfMassTransform * mVertices[e.mVertex[1]].mPosition, Color::sWhite);
}

void SoftBodyMotionProperties::DrawVolumeConstraints(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const
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

void SoftBodyMotionProperties::DrawPredictedBounds(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const
{
	inRenderer->DrawWireBox(inCenterOfMassTransform, mLocalPredictedBounds, Color::sRed);
}

#endif // JPH_DEBUG_RENDERER

void SoftBodyMotionProperties::SaveState(StateRecorder &inStream) const
{
	MotionProperties::SaveState(inStream);

	for (const Vertex &v : mVertices)
	{
		inStream.Write(v.mPreviousPosition);
		inStream.Write(v.mPosition);
		inStream.Write(v.mVelocity);
	}

	inStream.Write(mLocalBounds.mMin);
	inStream.Write(mLocalBounds.mMax);
	inStream.Write(mLocalPredictedBounds.mMin);
	inStream.Write(mLocalPredictedBounds.mMax);
}

void SoftBodyMotionProperties::RestoreState(StateRecorder &inStream)
{
	MotionProperties::RestoreState(inStream);

	for (Vertex &v : mVertices)
	{
		inStream.Read(v.mPreviousPosition);
		inStream.Read(v.mPosition);
		inStream.Read(v.mVelocity);
	}

	inStream.Read(mLocalBounds.mMin);
	inStream.Read(mLocalBounds.mMax);
	inStream.Read(mLocalPredictedBounds.mMin);
	inStream.Read(mLocalPredictedBounds.mMax);
}

JPH_NAMESPACE_END
