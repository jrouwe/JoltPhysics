// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyTest.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyTest) 
{ 
	JPH_ADD_BASE_CLASS(SoftBodyTest, Test) 
}

class SoftBodySettings : public RefTarget<SoftBodySettings>
{
public:
	void				CalculateEdgeLengths()
	{
		for (Edge &e : mEdgeConstraints)
		{
			e.mRestLength = (Vec3(mVertices[e.mVertex[1]].mPosition) - Vec3(mVertices[e.mVertex[0]].mPosition)).Length();
			JPH_ASSERT(e.mRestLength > 0.0f);
		}
	}

	void				CalculateVolumeConstraintVolumes()
	{
		for (Volume &v : mVolumeConstraints)
		{
			Vec3 x1(mVertices[v.mVertex[0]].mPosition);
			Vec3 x2(mVertices[v.mVertex[1]].mPosition);
			Vec3 x3(mVertices[v.mVertex[2]].mPosition);
			Vec3 x4(mVertices[v.mVertex[3]].mPosition);

			Vec3 x1x2 = x2 - x1;
			Vec3 x1x3 = x3 - x1;
			Vec3 x1x4 = x4 - x1;

			v.mSixRestVolume = abs(x1x2.Cross(x1x3).Dot(x1x4));
		}
	}

	struct Vertex
	{
		Float3			mPosition;					///< Initial position of the vertex
		Float3			mVelocity { 0, 0, 0 };		///< Initial velocity of the vertex	
		float			mInvMass = 1.0f;			///< Inverse of the mass of the vertex
	};

	struct Face
	{
		uint32			mVertex[3];					///< Indices of the vertices that form the face
	};

	/// An edge is kept at a constant length: |x1 - x2| = rest length
	struct Edge
	{
		uint32			mVertex[2];					///< Indices of the vertices that form the edge
		float			mRestLength = 1.0f;			///< Rest length of the spring
		float			mCompliance = 0.0f;			///< Inverse of the stiffness of the spring
	};

	/// Volume constraint, keeps the volume of a tetrahedron constant
	struct Volume
	{
		uint32			mVertex[4];					///< Indices of the vertices that form the tetrhedron
		float			mSixRestVolume = 1.0f;		///< 6 times the rest volume of the tetrahedron
		float			mCompliance = 0.0f;			///< Inverse of the stiffness of the constraint
	};

	Array<Vertex>		mVertices;
	Array<Face>			mFaces;
	Array<Edge>			mEdgeConstraints;
	Array<Volume>		mVolumeConstraints;

	uint32				mNumIterations = 5;			///< Number of solver iterations
	float				mLinearDamping = 0.05f;		///< Linear damping: dv/dt = -mLinearDamping * v
	float				mRestitution = 0.0f;		///< Restitution when colliding
	float				mFriction = 0.2f;			///< Friction coefficient when colliding
	float				mPressure = 0.0f;			///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	bool				mUpdatePosition = true;		///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

class SoftBody
{
public:
						SoftBody(const SoftBodySettings *inSettings, RVec3 inPosition, Quat inOrientation);

	void				Update(float inDeltaTime, PhysicsSystem &inSystem);

	struct DrawSettings
	{
		bool			mDrawPosition = false;
		bool			mDrawBounds = false;
		bool			mDrawVertices = true;
		bool			mDrawFaces = true;
		bool			mDrawEdges = true;
		bool			mDrawVolumeConstraints = true;
	};

	void				Draw(DebugRenderer *inRenderer, const DrawSettings &inDrawSettings) const;

	struct Vertex
	{
		Vec3			mPreviousPosition;
		Vec3 			mPosition;
		Vec3 			mVelocity;
		Plane			mCollisionPlane;			///< Nearest collision plane
		int				mCollidingShapeIndex;		///< Index in the colliding shapes list of the body we may collide with
		float			mInvMass;
		float			mProjectedDistance;
	};

	using Edge = SoftBodySettings::Edge;
	using Face = SoftBodySettings::Face;
	using Volume = SoftBodySettings::Volume;

	RefConst<SoftBodySettings> mSettings;
	Array<Vertex>		mVertices;
	RVec3				mPosition;
	AABox				mLocalBounds;
	AABox				mLocalPredictedBounds;
};

SoftBody::SoftBody(const SoftBodySettings *inSettings, RVec3 inPosition, Quat inOrientation)
{
	mSettings = inSettings;

	mPosition = inPosition;
	Mat44 orientation = Mat44::sRotation(inOrientation);

	mVertices.resize(inSettings->mVertices.size());
	for (Array<Vertex>::size_type v = 0; v < mVertices.size(); ++v)
	{
		const SoftBodySettings::Vertex &in_vertex = inSettings->mVertices[v];
		Vertex &out_vertex = mVertices[v];
		out_vertex.mPreviousPosition = out_vertex.mPosition = orientation * Vec3(in_vertex.mPosition);
		out_vertex.mVelocity = orientation.Multiply3x3(Vec3(in_vertex.mVelocity));
		out_vertex.mInvMass = in_vertex.mInvMass;

		mLocalBounds.Encapsulate(out_vertex.mPosition);
	}

	// We don't know delta time yet, so we can't predict the bounds and use the local bounds as the predicted bounds
	mLocalPredictedBounds = mLocalBounds;
}

void SoftBody::Update(float inDeltaTime, PhysicsSystem &inSystem)
{
	// Based on: XPBD, Extended Position Based Dynamics, Matthias Muller, Ten Minute Physics
	// See: https://matthias-research.github.io/pages/tenMinutePhysics/09-xpbd.pdf

	if (mSettings->mUpdatePosition)
	{
		// Shift the body so that the position is the center of the local bounds
		Vec3 delta = mLocalBounds.GetCenter();
		mPosition += delta;
		for (Vertex &v : mVertices)
			v.mPosition -= delta;
	}

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
		bool 			mUpdateVelocities;
		Mat44			mInvInertia;
		Vec3			mLinearVelocity;
		Vec3			mAngularVelocity;
	};
	struct Collector : public CollideShapeBodyCollector
	{
									Collector(RVec3Arg inPosition, const BodyLockInterface &inBodyLockInterface) : mPosition(inPosition), mBodyLockInterface(inBodyLockInterface) { }

		virtual void				AddHit(const BodyID &inResult) override
		{
			BodyLockRead lock(mBodyLockInterface, inResult);
			if (lock.Succeeded())
			{
				const Body &body = lock.GetBody();

				CollidingShape cs;
				cs.mCenterOfMassPosition = Vec3(body.GetCenterOfMassPosition() - mPosition);
				cs.mInverseShapeTransform = Mat44::sInverseRotationTranslation(body.GetRotation(), cs.mCenterOfMassPosition);
				cs.mShape = body.GetShape();
				cs.mBodyID = inResult;
				cs.mMotionType = body.GetMotionType();
				cs.mUpdateVelocities = false;
				if (cs.mMotionType == EMotionType::Dynamic)
				{
					const MotionProperties *mp = body.GetMotionProperties();
					cs.mInvMass = mp->GetInverseMass();
					cs.mInvInertia = mp->GetInverseInertiaForRotation(Mat44::sRotation(body.GetRotation()));
					cs.mLinearVelocity = mp->GetLinearVelocity();
					cs.mAngularVelocity = mp->GetAngularVelocity();
				}
				mHits.push_back(cs);
			}
		}

		RVec3						mPosition;
		const BodyLockInterface &	mBodyLockInterface;
		Array<CollidingShape>		mHits;
	};
	Collector collector(mPosition, inSystem.GetBodyLockInterfaceNoLock());
	AABox bounds = mLocalBounds;
	bounds.Encapsulate(mLocalPredictedBounds);
	bounds.Translate(mPosition);
	inSystem.GetBroadPhaseQuery().CollideAABox(bounds, collector);

	// Calculate delta time for sub step
	uint32 num_iterations = mSettings->mNumIterations;
	float dt = inDeltaTime / num_iterations;
	float dt_sq = Square(dt);

	// Calculate total displacement we'll have due to gravity over all sub steps
	// The total displacement as produced by our integrator can be written as: Sum(i * g * dt^2, i = 0..num_iterations).
	// This is bigger than 0.5 * g * dt^2 because we first increment the velocity and then update the position
	// Using Sum(i, i = 0..n) = n * (n + 1) / 2 we can write this as:
	Vec3 displacement_due_to_gravity = (0.5f * num_iterations * (num_iterations + 1) * dt_sq) * inSystem.GetGravity();

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
				for (const CollidingShape &shape : collector.mHits)
				{
					// TODO: Needs to be implemented on the shape itself
					if (shape.mShape->GetSubType() == EShapeSubType::Sphere)
					{
						// Special case for spheres
						const SphereShape *sphere = static_cast<const SphereShape *>(shape.mShape.GetPtr());
						float radius = sphere->GetRadius();
						Vec3 delta = v.mPosition - shape.mCenterOfMassPosition;
						float distance = delta.Length();
						float penetration = radius - distance;
						if (penetration > largest_penetration)
						{
							largest_penetration = penetration;
							Vec3 point, normal;
							if (distance > 0.0f)
							{
								point = shape.mCenterOfMassPosition + delta * (radius / distance);
								normal = delta / distance;
							}
							else
							{
								point = shape.mCenterOfMassPosition + Vec3(0, radius, 0);
								normal = Vec3::sAxisY();
							}
							v.mCollisionPlane = Plane::sFromPointAndNormal(point, normal);
							v.mCollidingShapeIndex = int(&shape - collector.mHits.data());
						}
					}
					else
					{
						// Fallback
						RayCastResult hit;
						hit.mFraction = 2.0f; // Add a little extra distance in case the particle speeds up
						RayCast ray(v.mPosition - 0.5f * movement, movement);
						RayCast local_ray(ray.Transformed(shape.mInverseShapeTransform));
						if (shape.mShape->CastRay(local_ray, SubShapeIDCreator(), hit))
						{
							float penetration = (hit.mFraction - 0.5f) * movement.Length();
							if (penetration > largest_penetration)
							{
								// Store collision
								largest_penetration = penetration;
								Vec3 point = ray.GetPointOnRay(hit.mFraction);
								Vec3 normal = shape.mShape->GetSurfaceNormal(hit.mSubShapeID2, shape.mInverseShapeTransform * point);
								v.mCollisionPlane = Plane::sFromPointAndNormal(point, normal);
								v.mCollidingShapeIndex = int(&shape - collector.mHits.data());
							}
						}
					}
				}
			}
	}

	float inv_dt_sq = 1.0f / dt_sq;
	float linear_damping = max(0.0f, 1.0f - mSettings->mLinearDamping * dt); // See: MotionProperties::ApplyForceTorqueAndDragInternal

	for (uint iteration = 0; iteration < num_iterations; ++iteration)
	{
		float pressure_coefficient = mSettings->mPressure;
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
		Vec3 sub_step_gravity = inSystem.GetGravity() * dt;
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
		float friction = mSettings->mFriction;
		float restitution = mSettings->mRestitution;
		float restitution_treshold = -2.0f * inSystem.GetGravity().Length() * dt;
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
							dv = v_tangential * min(friction * v.mProjectedDistance / (v_tangential_length * dt), 1.0f);
						else
							dv = Vec3::sZero();

						// Calculate delta relative velocity due to restitution (equation 35)
						dv += v_normal;
						float prev_v_normal = (prev_v - v2).Dot(contact_normal);
						if (prev_v_normal < restitution_treshold)
							dv += restitution * prev_v_normal * contact_normal;

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
							v.mVelocity -= v_tangential * min(friction * v.mProjectedDistance / (v_tangential_length * dt), 1.0f);

						// Apply restitution (equation 35)
						v.mVelocity -= v_normal;
						float prev_v_normal = prev_v.Dot(contact_normal);
						if (prev_v_normal < restitution_treshold)
							v.mVelocity -= restitution * prev_v_normal * contact_normal;
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

	// Write back velocities
	BodyInterface &body_interface = inSystem.GetBodyInterfaceNoLock();
	for (const CollidingShape &cs : collector.mHits)
		if (cs.mUpdateVelocities)
			body_interface.SetLinearAndAngularVelocity(cs.mBodyID, cs.mLinearVelocity, cs.mAngularVelocity);
}

void SoftBody::Draw(DebugRenderer *inRenderer, const DrawSettings &inDrawSettings) const
{
	if (inDrawSettings.mDrawPosition)
		inRenderer->DrawMarker(mPosition, Color::sYellow, 0.5f);

	if (inDrawSettings.mDrawVertices)
		for (const Vertex &v : mVertices)
			inRenderer->DrawMarker(mPosition + v.mPosition, Color::sRed, 0.05f);

	if (inDrawSettings.mDrawFaces)
		for (const Face &f : mSettings->mFaces)
		{
			RVec3 x1 = mPosition + mVertices[f.mVertex[0]].mPosition;
			RVec3 x2 = mPosition + mVertices[f.mVertex[1]].mPosition;
			RVec3 x3 = mPosition + mVertices[f.mVertex[2]].mPosition;

			inRenderer->DrawTriangle(x1, x2, x3, Color::sOrange, DebugRenderer::ECastShadow::On);
		}

	if (inDrawSettings.mDrawEdges)
		for (const Edge &e : mSettings->mEdgeConstraints)
			inRenderer->DrawLine(mPosition + mVertices[e.mVertex[0]].mPosition, mPosition + mVertices[e.mVertex[1]].mPosition, Color::sWhite);

	if (inDrawSettings.mDrawVolumeConstraints)
		for (const Volume &v : mSettings->mVolumeConstraints)
		{
			RVec3 x1 = mPosition + mVertices[v.mVertex[0]].mPosition;
			RVec3 x2 = mPosition + mVertices[v.mVertex[1]].mPosition;
			RVec3 x3 = mPosition + mVertices[v.mVertex[2]].mPosition;
			RVec3 x4 = mPosition + mVertices[v.mVertex[3]].mPosition;

			inRenderer->DrawTriangle(x1, x3, x2, Color::sYellow, DebugRenderer::ECastShadow::On);
			inRenderer->DrawTriangle(x2, x3, x4, Color::sYellow, DebugRenderer::ECastShadow::On);
			inRenderer->DrawTriangle(x1, x4, x3, Color::sYellow, DebugRenderer::ECastShadow::On);
			inRenderer->DrawTriangle(x1, x2, x4, Color::sYellow, DebugRenderer::ECastShadow::On);
		}

	if (inDrawSettings.mDrawBounds)
	{
		inRenderer->DrawWireBox(RMat44::sTranslation(mPosition), mLocalBounds, Color::sGreen);
		inRenderer->DrawWireBox(RMat44::sTranslation(mPosition), mLocalPredictedBounds, Color::sRed);
	}
}

SoftBodyTest::~SoftBodyTest()
{
	for (SoftBody *s : mSoftBodies)
		delete s;
}

const SoftBodySettings *sCreateCloth()
{
	const uint cGridSize = 30;
	const float cGridSpacing = 0.75f;
	const float cOffset = -0.5f * cGridSpacing * (cGridSize - 1);

	// Create settings
	SoftBodySettings *settings = new SoftBodySettings;
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodySettings::Vertex v;
			v.mPosition = Float3(cOffset + x * cGridSpacing, 0.0f, cOffset + y * cGridSpacing);
			settings->mVertices.push_back(v);
		}

	// Function to get the vertex index of a point on the cloth
	auto vertex_index = [cGridSize](uint inX, uint inY) -> uint
	{
		return inX + inY * cGridSize;
	};

	// Fixate corners
	settings->mVertices[vertex_index(0, 0)].mInvMass = 0.0f;
	settings->mVertices[vertex_index(cGridSize - 1, 0)].mInvMass = 0.0f;
	settings->mVertices[vertex_index(0, cGridSize - 1)].mInvMass = 0.0f;
	settings->mVertices[vertex_index(cGridSize - 1, cGridSize - 1)].mInvMass = 0.0f;

	// Create edges
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodySettings::Edge e;
			e.mCompliance = 0.00001f;
			e.mVertex[0] = vertex_index(x, y);
			if (x < cGridSize - 1)
			{
				e.mVertex[1] = vertex_index(x + 1, y);
				settings->mEdgeConstraints.push_back(e);
			}
			if (y < cGridSize - 1)
			{
				e.mVertex[1] = vertex_index(x, y + 1);
				settings->mEdgeConstraints.push_back(e);
			}
			if (x < cGridSize - 1 && y < cGridSize - 1)
			{
				e.mVertex[1] = vertex_index(x + 1, y + 1);
				settings->mEdgeConstraints.push_back(e);

				e.mVertex[0] = vertex_index(x + 1, y);
				e.mVertex[1] = vertex_index(x, y + 1);
				settings->mEdgeConstraints.push_back(e);
			}
		}
	settings->CalculateEdgeLengths();

	// Create faces
	for (uint y = 0; y < cGridSize - 1; ++y)
		for (uint x = 0; x < cGridSize - 1; ++x)
		{
			SoftBodySettings::Face f;
			f.mVertex[0] = vertex_index(x, y);
			f.mVertex[1] = vertex_index(x, y + 1);
			f.mVertex[2] = vertex_index(x + 1, y + 1);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(x + 1, y + 1);
			f.mVertex[2] = vertex_index(x + 1, y);
			settings->mFaces.push_back(f);
		}

	// Don't update the position of the cloth as it is fixed to the world
	settings->mUpdatePosition = false;

	return settings;
}

static SoftBodySettings *sCreateCube()
{
	const uint cGridSize = 5;
	const float cGridSpacing = 0.5f;
	const Vec3 cOffset = Vec3::sReplicate(-0.5f * cGridSpacing * (cGridSize - 1));

	// Create settings
	SoftBodySettings *settings = new SoftBodySettings;
	for (uint z = 0; z < cGridSize; ++z)
		for (uint y = 0; y < cGridSize; ++y)
			for (uint x = 0; x < cGridSize; ++x)
			{
				SoftBodySettings::Vertex v;
				(cOffset + Vec3::sReplicate(cGridSpacing) * Vec3(float(x), float(y), float(z))).StoreFloat3(&v.mPosition);
				settings->mVertices.push_back(v);
			}

	// Function to get the vertex index of a point on the cloth
	auto vertex_index = [cGridSize](uint inX, uint inY, uint inZ) -> uint
	{
		return inX + inY * cGridSize + inZ * cGridSize * cGridSize;
	};

	// Create edges
	for (uint z = 0; z < cGridSize; ++z)
		for (uint y = 0; y < cGridSize; ++y)
			for (uint x = 0; x < cGridSize; ++x)
			{
				SoftBodySettings::Edge e;
				e.mVertex[0] = vertex_index(x, y, z);
				if (x < cGridSize - 1)
				{
					e.mVertex[1] = vertex_index(x + 1, y, z);
					settings->mEdgeConstraints.push_back(e);
				}
				if (y < cGridSize - 1)
				{
					e.mVertex[1] = vertex_index(x, y + 1, z);
					settings->mEdgeConstraints.push_back(e);
				}
				if (z < cGridSize - 1)
				{
					e.mVertex[1] = vertex_index(x, y, z + 1);
					settings->mEdgeConstraints.push_back(e);
				}
			}
	settings->CalculateEdgeLengths();

	// Tetrahedrons to fill a cube
	const int tetra_indices[6][4][3] = {
		{ {0, 0, 0}, {0, 1, 1}, {0, 0, 1}, {1, 1, 1} },
		{ {0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1} },
		{ {0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1} },
		{ {0, 0, 0}, {1, 0, 1}, {1, 0, 0}, {1, 1, 1} },
		{ {0, 0, 0}, {1, 1, 0}, {0, 1, 0}, {1, 1, 1} },
		{ {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1} }
	};

	// Create volume constraints
	for (uint z = 0; z < cGridSize - 1; ++z)
		for (uint y = 0; y < cGridSize - 1; ++y)
			for (uint x = 0; x < cGridSize - 1; ++x)
				for (uint t = 0; t < 6; ++t)
				{
					SoftBodySettings::Volume v;
					for (uint i = 0; i < 4; ++i)
						v.mVertex[i] = vertex_index(x + tetra_indices[t][i][0], y + tetra_indices[t][i][1], z + tetra_indices[t][i][2]);
					settings->mVolumeConstraints.push_back(v);
				}

	settings->CalculateVolumeConstraintVolumes();

	return settings;
}

static SoftBodySettings *sCreatePressurizedSphere()
{
	const uint cNumTheta = 10;
	const uint cNumPhi = 20;

	// Create settings
	SoftBodySettings *settings = new SoftBodySettings;

	// Create vertices
	SoftBodySettings::Vertex v;
	Vec3::sUnitSpherical(0, 0).StoreFloat3(&v.mPosition);
	settings->mVertices.push_back(v);
	Vec3::sUnitSpherical(JPH_PI, 0).StoreFloat3(&v.mPosition);
	settings->mVertices.push_back(v);
	for (uint theta = 1; theta < cNumTheta - 1; ++theta)
		for (uint phi = 0; phi < cNumPhi; ++phi)
		{
			Vec3::sUnitSpherical(JPH_PI * theta / (cNumTheta - 1), 2.0f * JPH_PI * phi / cNumPhi).StoreFloat3(&v.mPosition);
			settings->mVertices.push_back(v);
		}

	// Function to get the vertex index of a point on the sphere
	auto vertex_index = [cNumTheta, cNumPhi](uint inTheta, uint inPhi) -> uint
	{
		if (inTheta == 0)
			return 0;
		else if (inTheta == cNumTheta - 1)
			return 1;
		else
			return 2 + (inTheta - 1) * cNumPhi + inPhi % cNumPhi;
	};

	// Create edge constraints
	for (uint phi = 0; phi < cNumPhi; ++phi)
	{
		for (uint theta = 0; theta < cNumTheta - 1; ++theta)
		{
			SoftBodySettings::Edge e;
			e.mCompliance = 0.0001f;
			e.mVertex[0] = vertex_index(theta, phi);

			e.mVertex[1] = vertex_index(theta + 1, phi);
			settings->mEdgeConstraints.push_back(e);

			e.mVertex[1] = vertex_index(theta + 1, phi + 1);
			settings->mEdgeConstraints.push_back(e);
			
			if (theta > 0)
			{
				e.mVertex[1] =  vertex_index(theta, phi + 1);
				settings->mEdgeConstraints.push_back(e);
			}
		}
	}

	settings->CalculateEdgeLengths();

	// Create faces
	SoftBodySettings::Face f;
	for (uint phi = 0; phi < cNumPhi; ++phi)
	{
		for (uint theta = 0; theta < cNumTheta - 1; ++theta)
		{
			f.mVertex[0] = vertex_index(theta, phi);
			f.mVertex[1] = vertex_index(theta + 1, phi);
			f.mVertex[2] = vertex_index(theta + 1, phi + 1);
			settings->mFaces.push_back(f);

			if (theta > 0 && theta < cNumTheta - 2)
			{
				f.mVertex[1] = vertex_index(theta + 1, phi + 1);
				f.mVertex[2] = vertex_index(theta, phi + 1);
				settings->mFaces.push_back(f);
			}
		}

		f.mVertex[0] = vertex_index(cNumTheta - 2, phi + 1);
		f.mVertex[1] = vertex_index(cNumTheta - 2, phi);
		f.mVertex[2] = vertex_index(cNumTheta - 1, 0);
		settings->mFaces.push_back(f);
	}

	settings->mPressure = 2000.0f;

	return settings;
}

void SoftBodyTest::Initialize()
{
	const Quat cCubeOrientation = Quat::sRotation(Vec3::sReplicate(sqrt(1.0f / 3.0f)), DegreesToRadians(45.0f));

	// Floor
	CreateMeshTerrain();

	mSoftBodies.push_back(new SoftBody(sCreateCloth(), RVec3(0, 10.0f, 0), Quat::sIdentity()));

	SoftBodySettings *cube1 = sCreateCube();
	cube1->mRestitution = 0.0f;
	mSoftBodies.push_back(new SoftBody(cube1, RVec3(15.0f, 10.0f, 0.0f), cCubeOrientation));

	SoftBodySettings *cube2 = sCreateCube();
	cube2->mRestitution = 1.0f;
	mSoftBodies.push_back(new SoftBody(cube2, RVec3(25.0f, 10.0f, 0.0f), cCubeOrientation));

	SoftBodySettings *sphere = sCreatePressurizedSphere();
	mSoftBodies.push_back(new SoftBody(sphere, RVec3(15.0f, 10.0f, 15.0f), Quat::sIdentity()));

	// Sphere below pressurized sphere
	BodyCreationSettings bcs(new SphereShape(1.0f), RVec3(15.5f, 7.0f, 15.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = 100.0f;
	mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	
	// Spheres above cloth
	for (int i = 0; i < 5; ++i)
	{
		bcs.mPosition = RVec3(0, 15.0f + 3.0f * i, 0);
		mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	}
}

void SoftBodyTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	for (SoftBody *s : mSoftBodies)
	{
		s->Update(1.0f / 60.0f, *mPhysicsSystem);

		SoftBody::DrawSettings settings;
		s->Draw(DebugRenderer::sInstance, settings);
	}
}
