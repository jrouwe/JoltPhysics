// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyTest.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Core/Reference.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyTest) 
{ 
	JPH_ADD_BASE_CLASS(SoftBodyTest, Test) 
}

class SoftBodySettings : public RefTarget<SoftBodySettings>
{
public:
	void				CalculateRestLengths()
	{
		for (Edge &e : mEdgeConstraints)
		{
			e.mRestLength = (Vec3(mVertices[e.mVertex[1]].mPosition) - Vec3(mVertices[e.mVertex[0]].mPosition)).Length();
			JPH_ASSERT(e.mRestLength > 0.0f);
		}
	}

	void				CalculateRestVolumes()
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
		Float3			mPosition;
		float			mInvMass = 1.0f;
	};

	struct Edge
	{
		uint32			mVertex[2];
		float			mRestLength = 1.0f;
		float			mCompliance = 0.0f;
	};

	struct Face
	{
		uint32			mVertex[3];
	};

	struct Volume
	{
		uint32			mVertex[4];
		float			mSixRestVolume = 1.0f;
		float			mCompliance = 0.0f;
	};

	Array<Vertex>		mVertices;
	Array<Edge>			mEdgeConstraints;
	Array<Face>			mFaceConstraints;
	Array<Volume>		mVolumeConstraints;

	float				mLinearDamping = 0.05f;
	float				mRestitution = 0.0f;
	float				mFriction = 0.2f;
	float				mPressure = 0.0f;
};

class SoftBody
{
public:
						SoftBody(const SoftBodySettings *inSettings, Mat44Arg inWorldTransform);

	void				Update(float inDeltaTime, Vec3Arg inGravity, uint inNumIterations);

	struct DrawSettings
	{
		bool			mDrawVertices = true;
		bool			mDrawEdges = true;
		bool			mDrawFaces = true;
		bool			mDrawVolumes = true;
	};

	void				Draw(DebugRenderer *inRenderer, const DrawSettings &inDrawSettings) const;

	struct Vertex
	{
		Vec3			mPreviousPosition;
		Vec3 			mPosition;
		Vec3 			mVelocity;
		float			mInvMass;
		float			mProjectedDistance;
	};

	using Edge = SoftBodySettings::Edge;
	using Face = SoftBodySettings::Face;
	using Volume = SoftBodySettings::Volume;

	RefConst<SoftBodySettings> mSettings;
	Array<Vertex>		mVertices;
};

SoftBody::SoftBody(const SoftBodySettings *inSettings, Mat44Arg inWorldTransform)
{
	mSettings = inSettings;

	mVertices.resize(inSettings->mVertices.size());
	for (Array<Vertex>::size_type v = 0; v < mVertices.size(); ++v)
	{
		const SoftBodySettings::Vertex &in_vertex = inSettings->mVertices[v];
		Vertex &out_vertex = mVertices[v];
		out_vertex.mPreviousPosition = out_vertex.mPosition = inWorldTransform * Vec3(in_vertex.mPosition);
		out_vertex.mVelocity = Vec3::sZero();
		out_vertex.mInvMass = in_vertex.mInvMass;
	}
}

void SoftBody::Update(float inDeltaTime, Vec3Arg inGravity, uint inNumIterations)
{
	// Based on: XPBD, Extended Position Based Dynamics, Matthias Muller, Ten Minute Physics
	// See: https://matthias-research.github.io/pages/tenMinutePhysics/09-xpbd.pdf

	float dt = inDeltaTime / inNumIterations;
	float inv_dt_sq = 1.0f / Square(dt);
	float linear_damping = max(0.0f, 1.0f - mSettings->mLinearDamping * dt); // See: MotionProperties::ApplyForceTorqueAndDragInternal

	for (uint iteration = 0; iteration < inNumIterations; ++iteration)
	{
		// Integrate
		for (Vertex &v : mVertices)
			if (v.mInvMass > 0.0f)
			{
				// Gravity
				v.mVelocity += inGravity * dt;

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

		// Satisfy collision (for now a single static plane)
		Plane plane(Vec3::sAxisY(), 0.0f);
		for (Vertex &v : mVertices)
			if (v.mInvMass > 0.0f)
			{
				float distance = plane.SignedDistance(v.mPosition);
				if (distance < 0.0f)
				{
					v.mPosition -= plane.GetNormal() * distance;
					v.mProjectedDistance -= distance; // For friction calculation
				}
			}

		// Update velocity
		float friction = mSettings->mFriction;
		float restitution = mSettings->mRestitution;
		float restitution_treshold = -2.0f * inGravity.Length() * dt;
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
					Vec3 contact_normal = plane.GetNormal();
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
}

void SoftBody::Draw(DebugRenderer *inRenderer, const DrawSettings &inDrawSettings) const
{
	if (inDrawSettings.mDrawVertices)
		for (const Vertex &v : mVertices)
			inRenderer->DrawMarker(v.mPosition, Color::sRed, 0.05f);

	if (inDrawSettings.mDrawEdges)
		for (const Edge &e : mSettings->mEdgeConstraints)
			inRenderer->DrawLine(mVertices[e.mVertex[0]].mPosition, mVertices[e.mVertex[1]].mPosition, Color::sWhite);

	if (inDrawSettings.mDrawFaces)
		for (const Face &f : mSettings->mFaceConstraints)
		{
			Vec3 x1 = mVertices[f.mVertex[0]].mPosition;
			Vec3 x2 = mVertices[f.mVertex[1]].mPosition;
			Vec3 x3 = mVertices[f.mVertex[2]].mPosition;

			inRenderer->DrawTriangle(x1, x2, x3, Color::sOrange);
		}

	if (inDrawSettings.mDrawVolumes)
		for (const Volume &v : mSettings->mVolumeConstraints)
		{
			Vec3 x1 = mVertices[v.mVertex[0]].mPosition;
			Vec3 x2 = mVertices[v.mVertex[1]].mPosition;
			Vec3 x3 = mVertices[v.mVertex[2]].mPosition;
			Vec3 x4 = mVertices[v.mVertex[3]].mPosition;

			inRenderer->DrawTriangle(x1, x3, x2, Color::sYellow);
			inRenderer->DrawTriangle(x2, x3, x4, Color::sYellow);
			inRenderer->DrawTriangle(x1, x4, x3, Color::sYellow);
			inRenderer->DrawTriangle(x1, x2, x4, Color::sYellow);
		}
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

	// Create settings
	SoftBodySettings *settings = new SoftBodySettings;
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodySettings::Vertex v;
			v.mPosition = Float3(x * cGridSpacing, 10.0f, y * cGridSpacing);
			settings->mVertices.push_back(v);
		}

	// Fixate corners
	settings->mVertices[0].mInvMass = 0.0f;
	settings->mVertices[cGridSize - 1].mInvMass = 0.0f;
	settings->mVertices[(cGridSize - 1) * cGridSize].mInvMass = 0.0f;
	settings->mVertices[(cGridSize - 1) * cGridSize + cGridSize - 1].mInvMass = 0.0f;

	// Create edges
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodySettings::Edge e;
			e.mVertex[0] = x + y * cGridSize;
			if (x < cGridSize - 1)
			{
				e.mVertex[1] = x + 1 + y * cGridSize;
				settings->mEdgeConstraints.push_back(e);
			}
			if (y < cGridSize - 1)
			{
				e.mVertex[1] = x + (y + 1) * cGridSize;
				settings->mEdgeConstraints.push_back(e);
			}
			if (x < cGridSize - 1 && y < cGridSize - 1)
			{
				e.mVertex[1] = x + 1 + (y + 1) * cGridSize;
				settings->mEdgeConstraints.push_back(e);

				e.mVertex[0] = x + 1 + y * cGridSize;
				e.mVertex[1] = x + (y + 1) * cGridSize;
				settings->mEdgeConstraints.push_back(e);
			}
		}
	settings->CalculateRestLengths();

	return settings;
}

static SoftBodySettings *sCreateCube()
{
	const uint cGridSize = 5;
	const float cGridSpacing = 0.5f;

	// Create settings
	SoftBodySettings *settings = new SoftBodySettings;
	for (uint z = 0; z < cGridSize; ++z)
		for (uint y = 0; y < cGridSize; ++y)
			for (uint x = 0; x < cGridSize; ++x)
			{
				SoftBodySettings::Vertex v;
				Vec3(x * cGridSpacing, y * cGridSpacing, z * cGridSpacing).StoreFloat3(&v.mPosition);
				settings->mVertices.push_back(v);
			}

	// Create edges
	for (uint z = 0; z < cGridSize; ++z)
		for (uint y = 0; y < cGridSize; ++y)
			for (uint x = 0; x < cGridSize; ++x)
			{
				SoftBodySettings::Edge e;
				e.mVertex[0] = x + y * cGridSize + z * cGridSize * cGridSize;
				if (x < cGridSize - 1)
				{
					e.mVertex[1] = x + 1 + y * cGridSize + z * cGridSize * cGridSize;
					settings->mEdgeConstraints.push_back(e);
				}
				if (y < cGridSize - 1)
				{
					e.mVertex[1] = x + (y + 1) * cGridSize + z * cGridSize * cGridSize;
					settings->mEdgeConstraints.push_back(e);
				}
				if (z < cGridSize - 1)
				{
					e.mVertex[1] = x + y * cGridSize + (z + 1) * cGridSize * cGridSize;
					settings->mEdgeConstraints.push_back(e);
				}
			}
	settings->CalculateRestLengths();

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
						v.mVertex[i] = x + tetra_indices[t][i][0] + (y + tetra_indices[t][i][1]) * cGridSize + (z + tetra_indices[t][i][2]) * cGridSize * cGridSize;
					settings->mVolumeConstraints.push_back(v);
				}

	settings->CalculateRestVolumes();

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

	// Create edges
	for (uint phi = 0; phi < cNumPhi; ++phi)
	{
		for (uint theta = 0; theta < cNumTheta - 1; ++theta)
		{
			SoftBodySettings::Edge e;
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
	settings->CalculateRestLengths();

	// Create faces
	SoftBodySettings::Face f;
	for (uint phi = 0; phi < cNumPhi; ++phi)
	{
		for (uint theta = 0; theta < cNumTheta - 1; ++theta)
		{
			f.mVertex[0] = vertex_index(theta, phi);
			f.mVertex[1] = vertex_index(theta + 1, phi);
			f.mVertex[2] = vertex_index(theta + 1, phi + 1);
			settings->mFaceConstraints.push_back(f);

			if (theta > 0 && theta < cNumTheta - 2)
			{
				f.mVertex[1] = vertex_index(theta + 1, phi + 1);
				f.mVertex[2] = vertex_index(theta, phi + 1);
				settings->mFaceConstraints.push_back(f);
			}
		}

		f.mVertex[0] = vertex_index(cNumTheta - 2, phi + 1);
		f.mVertex[1] = vertex_index(cNumTheta - 2, phi);
		f.mVertex[2] = vertex_index(cNumTheta - 1, 0);
		settings->mFaceConstraints.push_back(f);
	}

	return settings;
}

void SoftBodyTest::Initialize()
{
	const Quat cCubeOrientation = Quat::sRotation(Vec3::sReplicate(sqrt(1.0f / 3.0f)), DegreesToRadians(45.0f));

	// Floor
	CreateFloor();

	sSoftBodies[0] = new SoftBody(sCreateCloth(), Mat44::sIdentity());

	SoftBodySettings *cube1 = sCreateCube();
	cube1->mRestitution = 0.0f;
	sSoftBodies[1] = new SoftBody(cube1, Mat44::sRotationTranslation(cCubeOrientation, Vec3(30.0f, 10.0f, 0.0f)));

	SoftBodySettings *cube2 = sCreateCube();
	cube2->mRestitution = 1.0f;
	sSoftBodies[2] = new SoftBody(cube2, Mat44::sRotationTranslation(cCubeOrientation, Vec3(40.0f, 10.0f, 0.0f)));

	SoftBodySettings *sphere = sCreatePressurizedSphere();
	sSoftBodies[3] = new SoftBody(sphere, Mat44::sTranslation(Vec3(30.0f, 10.0f, 10.0f)));
}

void SoftBodyTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	for (SoftBody *s : sSoftBodies)
	{
		s->Update(1.0f / 60.0f, Vec3(0.0f, -9.8f, 0.0f), 5);

		SoftBody::DrawSettings settings;
		s->Draw(DebugRenderer::sInstance, settings);
	}
}
