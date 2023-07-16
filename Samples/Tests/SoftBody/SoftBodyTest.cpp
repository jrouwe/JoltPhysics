// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyTest.h>
#include <Jolt/Core/Reference.h>
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

	void				Update(float inDeltaTime, const PhysicsSystem &inSystem);

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
		BodyID			mCollisionBodyID;			///< ID of the body we may collide with
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
		out_vertex.mVelocity = Vec3::sZero();
		out_vertex.mInvMass = in_vertex.mInvMass;

		mLocalBounds.Encapsulate(out_vertex.mPosition);
	}
}

void SoftBody::Update(float inDeltaTime, const PhysicsSystem &inSystem)
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

	// Collect all colliding bodies
	AllHitCollisionCollector<CollideShapeBodyCollector> collector;
	AABox bounds = mLocalBounds;
	bounds.Translate(mPosition);
	inSystem.GetBroadPhaseQuery().CollideAABox(bounds, collector);

	// Collect information about the colliding bodies
	const BodyInterface &body_interface = inSystem.GetBodyInterfaceNoLock();
	struct CollidingShape
	{
		Mat44			mInverseShapeTransform;
		RefConst<Shape>	mShape;
		BodyID			mBodyID;
	};
	Array<CollidingShape> colliding_shapes;
	colliding_shapes.reserve(collector.mHits.size());
	for (const BodyID &id : collector.mHits)
	{
		TransformedShape ts = body_interface.GetTransformedShape(id);
		colliding_shapes.push_back({ Mat44::sInverseRotationTranslation(ts.mShapeRotation, Vec3(ts.mShapePositionCOM - mPosition)), ts.mShape, ts.mBodyID });
	}

	// Generate collision planes
	Vec3 step_gravity = inSystem.GetGravity() * inDeltaTime;
	for (Vertex &v : mVertices)
		if (v.mInvMass > 0.0f)
		{
			// Create a ray in the direction the particle is expected to move, start before the particle to avoid falling through a thin floor
			Vec3 direction = (v.mVelocity + step_gravity) * inDeltaTime;
			RayCast ray(v.mPosition - 0.5f * direction, direction);

			// Find the closest collision
			RayCastResult hit;
			hit.mFraction = 2.0f; // Add a little extra distance in case the particle speeds up
			SubShapeID hit_sub_shape_id;
			const CollidingShape *hit_colliding_shape = nullptr;
			for (const CollidingShape &shape : colliding_shapes)
			{
				RayCast local_ray(ray.Transformed(shape.mInverseShapeTransform));
				if (shape.mShape->CastRay(local_ray, SubShapeIDCreator(), hit))
				{
					hit.mBodyID = shape.mBodyID;
					hit_sub_shape_id = hit.mSubShapeID2;
					hit_colliding_shape = &shape;
				}
			}

			if (hit_colliding_shape != nullptr)
			{
				// Store collision
				Vec3 point = ray.GetPointOnRay(hit.mFraction);
				Vec3 normal = hit_colliding_shape->mShape->GetSurfaceNormal(hit_sub_shape_id, hit_colliding_shape->mInverseShapeTransform * point);
				v.mCollisionPlane = Plane::sFromPointAndNormal(point, normal);
				v.mCollisionBodyID = hit.mBodyID;
			}
			else
			{
				// No collision
				v.mCollisionBodyID = BodyID();
			}
		}

	uint32 num_iterations = mSettings->mNumIterations;
	float dt = inDeltaTime / num_iterations;
	float inv_dt_sq = 1.0f / Square(dt);
	float linear_damping = max(0.0f, 1.0f - mSettings->mLinearDamping * dt); // See: MotionProperties::ApplyForceTorqueAndDragInternal

	for (uint iteration = 0; iteration < num_iterations; ++iteration)
	{
		float pressure_coefficient = mSettings->mPressure;
		if (pressure_coefficient > 0.0f)
		{
			// Calculate total volume
			float six_volume = 0.0f;
			Vec3 origin = mVertices[0].mPosition;
			for (const Face &f : mSettings->mFaces)
			{
				Vec3 x1 = mVertices[f.mVertex[0]].mPosition;
				Vec3 x2 = mVertices[f.mVertex[1]].mPosition;
				Vec3 x3 = mVertices[f.mVertex[2]].mPosition;
				six_volume += (x1 - origin).Cross(x2 - origin).Dot(x3 - origin);
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
		Plane plane(Vec3::sAxisY(), float(mPosition.GetY()));
		for (Vertex &v : mVertices)
			if (!v.mCollisionBodyID.IsInvalid())
			{
				float distance = v.mCollisionPlane.SignedDistance(v.mPosition);
				if (distance < 0.0f)
				{
					v.mPosition -= v.mCollisionPlane.GetNormal() * distance;
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
					// Apply friction as described in Detailed Rigid Body Simulation with Extended Position Based Dynamics - Matthias Muller et al.
					// See section 3.6:
					// Inverse mass: w1 = 1 / m1 (particle has no inertia), w2 = 0 (plane is static)
					// Lagrange multiplier for contact: lambda = -c / (w1 + w2)
					// Where c is the constraint equation (the distance to the plane, negative because penetrating)
					// Contact normal force: fn = lambda / dt^2
					// Delta velocity due to friction dv = -vt / |vt| * min(dt * friction * fn * (w1 + w2), |vt|) = -vt * min(-friction * c / (|vt| * dt), 1)
					// Note that I think there is an error in the paper, I added a mass term, see: https://github.com/matthias-research/pages/issues/29
					// Normal velocity: vn = (v1 - v2) . contact_normal (but v2 is zero because the plane is static)
					// Tangential velocity: vt = v1 - v2 - contact_normal * vn
					// Impulse: p = dv / (w1 + w2)
					// Changes in particle velocities:
					// v1 = v1 + p / m1
					// v2 = v2 - p / m2 (but the plane is static so this is zero)
					Vec3 contact_normal = v.mCollisionPlane.GetNormal();
					Vec3 v_normal = contact_normal * contact_normal.Dot(v.mVelocity);
					Vec3 v_tangential = v.mVelocity - v_normal;
					float v_tangential_length = v_tangential.Length();
					if (v_tangential_length > 0.0f)
						v.mVelocity -= v_tangential * min(friction * v.mProjectedDistance / (v_tangential_length * dt), 1.0f);

					// Apply restitution (equation 35)
					// First cancel out the normal velocity
					v.mVelocity -= v_normal;

					// Then apply restitution if the velocity is above the treshold
					float prev_v_normal = prev_v.Dot(contact_normal);
					if (prev_v_normal < restitution_treshold)
						v.mVelocity -= restitution * prev_v_normal * contact_normal;
				}
			}
		}

	// Update local bounding box
	mLocalBounds = AABox();
	for (Vertex &v : mVertices)
		mLocalBounds.Encapsulate(v.mPosition);
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

			inRenderer->DrawTriangle(x1, x2, x3, Color::sOrange);
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

			inRenderer->DrawTriangle(x1, x3, x2, Color::sYellow);
			inRenderer->DrawTriangle(x2, x3, x4, Color::sYellow);
			inRenderer->DrawTriangle(x1, x4, x3, Color::sYellow);
			inRenderer->DrawTriangle(x1, x2, x4, Color::sYellow);
		}

	if (inDrawSettings.mDrawBounds)
		inRenderer->DrawWireBox(RMat44::sTranslation(mPosition), mLocalBounds, Color::sGreen);
}

static SoftBody *sSoftBodies[4];

SoftBodyTest::~SoftBodyTest()
{
	for (SoftBody *s : sSoftBodies)
		delete s;
}

const SoftBodySettings *sCreateCloth()
{
	const uint cGridSize = 20;
	const float cGridSpacing = 1.0f;
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

	sSoftBodies[0] = new SoftBody(sCreateCloth(), RVec3(0, 10.0f, 0), Quat::sIdentity());

	SoftBodySettings *cube1 = sCreateCube();
	cube1->mRestitution = 0.0f;
	sSoftBodies[1] = new SoftBody(cube1, RVec3(15.0f, 10.0f, 0.0f), cCubeOrientation);

	SoftBodySettings *cube2 = sCreateCube();
	cube2->mRestitution = 1.0f;
	sSoftBodies[2] = new SoftBody(cube2, RVec3(25.0f, 10.0f, 0.0f), cCubeOrientation);

	SoftBodySettings *sphere = sCreatePressurizedSphere();
	sSoftBodies[3] = new SoftBody(sphere, RVec3(15.0f, 10.0f, 15.0f), Quat::sIdentity());
}

void SoftBodyTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	for (SoftBody *s : sSoftBodies)
	{
		s->Update(1.0f / 60.0f, *mPhysicsSystem);

		SoftBody::DrawSettings settings;
		s->Draw(DebugRenderer::sInstance, settings);
	}
}
