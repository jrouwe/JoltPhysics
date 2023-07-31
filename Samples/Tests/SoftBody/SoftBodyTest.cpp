// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyTest) 
{ 
	JPH_ADD_BASE_CLASS(SoftBodyTest, Test) 
}

const SoftBodyParticleSettings *sCreateCloth(bool inFixateCorners = true)
{
	const uint cGridSize = 30;
	const float cGridSpacing = 0.75f;
	const float cOffset = -0.5f * cGridSpacing * (cGridSize - 1);

	// Create settings
	SoftBodyParticleSettings *settings = new SoftBodyParticleSettings;
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodyParticleSettings::Vertex v;
			v.mPosition = Float3(cOffset + x * cGridSpacing, 0.0f, cOffset + y * cGridSpacing);
			settings->mVertices.push_back(v);
		}

	// Function to get the vertex index of a point on the cloth
	auto vertex_index = [cGridSize](uint inX, uint inY) -> uint
	{
		return inX + inY * cGridSize;
	};

	if (inFixateCorners)
	{
		// Fixate corners
		settings->mVertices[vertex_index(0, 0)].mInvMass = 0.0f;
		settings->mVertices[vertex_index(cGridSize - 1, 0)].mInvMass = 0.0f;
		settings->mVertices[vertex_index(0, cGridSize - 1)].mInvMass = 0.0f;
		settings->mVertices[vertex_index(cGridSize - 1, cGridSize - 1)].mInvMass = 0.0f;
	}

	// Create edges
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodyParticleSettings::Edge e;
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
			SoftBodyParticleSettings::Face f;
			f.mVertex[0] = vertex_index(x, y);
			f.mVertex[1] = vertex_index(x, y + 1);
			f.mVertex[2] = vertex_index(x + 1, y + 1);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(x + 1, y + 1);
			f.mVertex[2] = vertex_index(x + 1, y);
			settings->mFaces.push_back(f);
		}

	return settings;
}

static SoftBodyParticleSettings *sCreateCube()
{
	const uint cGridSize = 5;
	const float cGridSpacing = 0.5f;
	const Vec3 cOffset = Vec3::sReplicate(-0.5f * cGridSpacing * (cGridSize - 1));

	// Create settings
	SoftBodyParticleSettings *settings = new SoftBodyParticleSettings;
	for (uint z = 0; z < cGridSize; ++z)
		for (uint y = 0; y < cGridSize; ++y)
			for (uint x = 0; x < cGridSize; ++x)
			{
				SoftBodyParticleSettings::Vertex v;
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
				SoftBodyParticleSettings::Edge e;
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
					SoftBodyParticleSettings::Volume v;
					for (uint i = 0; i < 4; ++i)
						v.mVertex[i] = vertex_index(x + tetra_indices[t][i][0], y + tetra_indices[t][i][1], z + tetra_indices[t][i][2]);
					settings->mVolumeConstraints.push_back(v);
				}

	settings->CalculateVolumeConstraintVolumes();

	// Create faces
	for (uint y = 0; y < cGridSize - 1; ++y)
		for (uint x = 0; x < cGridSize - 1; ++x)
		{
			SoftBodyParticleSettings::Face f;

			// Face 1
			f.mVertex[0] = vertex_index(x, y, 0);
			f.mVertex[1] = vertex_index(x, y + 1, 0);
			f.mVertex[2] = vertex_index(x + 1, y + 1, 0);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(x + 1, y + 1, 0);
			f.mVertex[2] = vertex_index(x + 1, y, 0);
			settings->mFaces.push_back(f);

			// Face 2
			f.mVertex[0] = vertex_index(x, y, cGridSize - 1);
			f.mVertex[1] = vertex_index(x + 1, y + 1, cGridSize - 1);
			f.mVertex[2] = vertex_index(x, y + 1, cGridSize - 1);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(x + 1, y, cGridSize - 1);
			f.mVertex[2] = vertex_index(x + 1, y + 1, cGridSize - 1);
			settings->mFaces.push_back(f);

			// Face 3
			f.mVertex[0] = vertex_index(x, 0, y);
			f.mVertex[1] = vertex_index(x + 1, 0, y + 1);
			f.mVertex[2] = vertex_index(x, 0, y + 1);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(x + 1, 0, y);
			f.mVertex[2] = vertex_index(x + 1, 0, y + 1);
			settings->mFaces.push_back(f);

			// Face 4
			f.mVertex[0] = vertex_index(x, cGridSize - 1, y);
			f.mVertex[1] = vertex_index(x, cGridSize - 1, y + 1);
			f.mVertex[2] = vertex_index(x + 1, cGridSize - 1, y + 1);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(x + 1, cGridSize - 1, y + 1);
			f.mVertex[2] = vertex_index(x + 1, cGridSize - 1, y);
			settings->mFaces.push_back(f);

			// Face 5
			f.mVertex[0] = vertex_index(0, x, y);
			f.mVertex[1] = vertex_index(0, x, y + 1);
			f.mVertex[2] = vertex_index(0, x + 1, y + 1);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(0, x + 1, y + 1);
			f.mVertex[2] = vertex_index(0, x + 1, y);
			settings->mFaces.push_back(f);

			// Face 6
			f.mVertex[0] = vertex_index(cGridSize - 1, x, y);
			f.mVertex[1] = vertex_index(cGridSize - 1, x + 1, y + 1);
			f.mVertex[2] = vertex_index(cGridSize - 1, x, y + 1);
			settings->mFaces.push_back(f);

			f.mVertex[1] = vertex_index(cGridSize - 1, x + 1, y);
			f.mVertex[2] = vertex_index(cGridSize - 1, x + 1, y + 1);
			settings->mFaces.push_back(f);
		}

	return settings;
}

static SoftBodyParticleSettings *sCreateSphere()
{
	const uint cNumTheta = 10;
	const uint cNumPhi = 20;

	// Create settings
	SoftBodyParticleSettings *settings = new SoftBodyParticleSettings;

	// Create vertices
	SoftBodyParticleSettings::Vertex v;
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
			SoftBodyParticleSettings::Edge e;
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
	SoftBodyParticleSettings::Face f;
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

	return settings;
}

void SoftBodyTest::Initialize()
{
	const Quat cCubeOrientation = Quat::sRotation(Vec3::sReplicate(sqrt(1.0f / 3.0f)), DegreesToRadians(45.0f));

	// Floor
	CreateMeshTerrain();

	// Create cloth that's fixated at the corners
	SoftBodyCreationSettings cloth(sCreateCloth(), RVec3(0, 10.0f, 0));
	cloth.mObjectLayer = Layers::MOVING;
	cloth.mUpdatePosition = false; // Don't update the position of the cloth as it is fixed to the world
	mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// Create cube
	SoftBodyCreationSettings cube(sCreateCube(), RVec3(15.0f, 10.0f, 0.0f), cCubeOrientation);
	cube.mObjectLayer = Layers::MOVING;
	cube.mRestitution = 0.0f;
	mBodyInterface->CreateAndAddSoftBody(cube, EActivation::Activate);

	// Create another cube that shares information with the first cube
	cube.mPosition = RVec3(25.0f, 10.0f, 0.0f);
	cube.mRestitution = 1.0f;
	cube.mGravityFactor = 0.5f;
	mBodyInterface->CreateAndAddSoftBody(cube, EActivation::Activate);

	// Create pressurized sphere
	SoftBodyCreationSettings sphere(sCreateSphere(), RVec3(15.0f, 10.0f, 15.0f));
	sphere.mObjectLayer = Layers::MOVING;
	sphere.mPressure = 2000.0f;
	mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);

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
