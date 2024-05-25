// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Utils/SoftBodyCreator.h>

namespace SoftBodyCreator {

Ref<SoftBodySharedSettings> CreateCloth(uint inGridSizeX, uint inGridSizeZ, float inGridSpacing, const function<float(uint, uint)> &inVertexGetInvMass, const function<Vec3(uint, uint)> &inVertexPerturbation, SoftBodySharedSettings::EBendType inBendType, const SoftBodySharedSettings::VertexAttributes &inVertexAttributes)
{
	const float cOffsetX = -0.5f * inGridSpacing * (inGridSizeX - 1);
	const float cOffsetZ = -0.5f * inGridSpacing * (inGridSizeZ - 1);

	// Create settings
	SoftBodySharedSettings *settings = new SoftBodySharedSettings;
	for (uint z = 0; z < inGridSizeZ; ++z)
		for (uint x = 0; x < inGridSizeX; ++x)
		{
			SoftBodySharedSettings::Vertex v;
			Vec3 position = inVertexPerturbation(x, z) + Vec3(cOffsetX + x * inGridSpacing, 0.0f, cOffsetZ + z * inGridSpacing);
			position.StoreFloat3(&v.mPosition);
			v.mInvMass = inVertexGetInvMass(x, z);
			settings->mVertices.push_back(v);
		}

	// Function to get the vertex index of a point on the cloth
	auto vertex_index = [inGridSizeX](uint inX, uint inY) -> uint
	{
		return inX + inY * inGridSizeX;
	};

	// Create faces
	for (uint z = 0; z < inGridSizeZ - 1; ++z)
		for (uint x = 0; x < inGridSizeX - 1; ++x)
		{
			SoftBodySharedSettings::Face f;
			f.mVertex[0] = vertex_index(x, z);
			f.mVertex[1] = vertex_index(x, z + 1);
			f.mVertex[2] = vertex_index(x + 1, z + 1);
			settings->AddFace(f);

			f.mVertex[1] = vertex_index(x + 1, z + 1);
			f.mVertex[2] = vertex_index(x + 1, z);
			settings->AddFace(f);
		}

	// Create constraints
	settings->CreateConstraints(&inVertexAttributes, 1, inBendType);

	// Optimize the settings
	settings->Optimize();

	return settings;
}

Ref<SoftBodySharedSettings> CreateClothWithFixatedCorners(uint inGridSizeX, uint inGridSizeZ, float inGridSpacing)
{
	auto inv_mass = [inGridSizeX, inGridSizeZ](uint inX, uint inZ) {
		return (inX == 0 && inZ == 0)
			|| (inX == inGridSizeX - 1 && inZ == 0)
			|| (inX == 0 && inZ == inGridSizeZ - 1)
			|| (inX == inGridSizeX - 1 && inZ == inGridSizeZ - 1)? 0.0f : 1.0f;
	};

	return CreateCloth(inGridSizeX, inGridSizeZ, inGridSpacing, inv_mass);
}

Ref<SoftBodySharedSettings> CreateCube(uint inGridSize, float inGridSpacing)
{
	const Vec3 cOffset = Vec3::sReplicate(-0.5f * inGridSpacing * (inGridSize - 1));

	// Create settings
	SoftBodySharedSettings *settings = new SoftBodySharedSettings;
	for (uint z = 0; z < inGridSize; ++z)
		for (uint y = 0; y < inGridSize; ++y)
			for (uint x = 0; x < inGridSize; ++x)
			{
				SoftBodySharedSettings::Vertex v;
				(cOffset + Vec3::sReplicate(inGridSpacing) * Vec3(float(x), float(y), float(z))).StoreFloat3(&v.mPosition);
				settings->mVertices.push_back(v);
			}

	// Function to get the vertex index of a point on the cloth
	auto vertex_index = [inGridSize](uint inX, uint inY, uint inZ) -> uint
	{
		return inX + inY * inGridSize + inZ * inGridSize * inGridSize;
	};

	// Create edges
	for (uint z = 0; z < inGridSize; ++z)
		for (uint y = 0; y < inGridSize; ++y)
			for (uint x = 0; x < inGridSize; ++x)
			{
				SoftBodySharedSettings::Edge e;
				e.mVertex[0] = vertex_index(x, y, z);
				if (x < inGridSize - 1)
				{
					e.mVertex[1] = vertex_index(x + 1, y, z);
					settings->mEdgeConstraints.push_back(e);
				}
				if (y < inGridSize - 1)
				{
					e.mVertex[1] = vertex_index(x, y + 1, z);
					settings->mEdgeConstraints.push_back(e);
				}
				if (z < inGridSize - 1)
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
	for (uint z = 0; z < inGridSize - 1; ++z)
		for (uint y = 0; y < inGridSize - 1; ++y)
			for (uint x = 0; x < inGridSize - 1; ++x)
				for (uint t = 0; t < 6; ++t)
				{
					SoftBodySharedSettings::Volume v;
					for (uint i = 0; i < 4; ++i)
						v.mVertex[i] = vertex_index(x + tetra_indices[t][i][0], y + tetra_indices[t][i][1], z + tetra_indices[t][i][2]);
					settings->mVolumeConstraints.push_back(v);
				}

	settings->CalculateVolumeConstraintVolumes();

	// Create faces
	for (uint y = 0; y < inGridSize - 1; ++y)
		for (uint x = 0; x < inGridSize - 1; ++x)
		{
			SoftBodySharedSettings::Face f;

			// Face 1
			f.mVertex[0] = vertex_index(x, y, 0);
			f.mVertex[1] = vertex_index(x, y + 1, 0);
			f.mVertex[2] = vertex_index(x + 1, y + 1, 0);
			settings->AddFace(f);

			f.mVertex[1] = vertex_index(x + 1, y + 1, 0);
			f.mVertex[2] = vertex_index(x + 1, y, 0);
			settings->AddFace(f);

			// Face 2
			f.mVertex[0] = vertex_index(x, y, inGridSize - 1);
			f.mVertex[1] = vertex_index(x + 1, y + 1, inGridSize - 1);
			f.mVertex[2] = vertex_index(x, y + 1, inGridSize - 1);
			settings->AddFace(f);

			f.mVertex[1] = vertex_index(x + 1, y, inGridSize - 1);
			f.mVertex[2] = vertex_index(x + 1, y + 1, inGridSize - 1);
			settings->AddFace(f);

			// Face 3
			f.mVertex[0] = vertex_index(x, 0, y);
			f.mVertex[1] = vertex_index(x + 1, 0, y + 1);
			f.mVertex[2] = vertex_index(x, 0, y + 1);
			settings->AddFace(f);

			f.mVertex[1] = vertex_index(x + 1, 0, y);
			f.mVertex[2] = vertex_index(x + 1, 0, y + 1);
			settings->AddFace(f);

			// Face 4
			f.mVertex[0] = vertex_index(x, inGridSize - 1, y);
			f.mVertex[1] = vertex_index(x, inGridSize - 1, y + 1);
			f.mVertex[2] = vertex_index(x + 1, inGridSize - 1, y + 1);
			settings->AddFace(f);

			f.mVertex[1] = vertex_index(x + 1, inGridSize - 1, y + 1);
			f.mVertex[2] = vertex_index(x + 1, inGridSize - 1, y);
			settings->AddFace(f);

			// Face 5
			f.mVertex[0] = vertex_index(0, x, y);
			f.mVertex[1] = vertex_index(0, x, y + 1);
			f.mVertex[2] = vertex_index(0, x + 1, y + 1);
			settings->AddFace(f);

			f.mVertex[1] = vertex_index(0, x + 1, y + 1);
			f.mVertex[2] = vertex_index(0, x + 1, y);
			settings->AddFace(f);

			// Face 6
			f.mVertex[0] = vertex_index(inGridSize - 1, x, y);
			f.mVertex[1] = vertex_index(inGridSize - 1, x + 1, y + 1);
			f.mVertex[2] = vertex_index(inGridSize - 1, x, y + 1);
			settings->AddFace(f);

			f.mVertex[1] = vertex_index(inGridSize - 1, x + 1, y);
			f.mVertex[2] = vertex_index(inGridSize - 1, x + 1, y + 1);
			settings->AddFace(f);
		}

	// Optimize the settings
	settings->Optimize();

	return settings;
}

Ref<SoftBodySharedSettings> CreateSphere(float inRadius, uint inNumTheta, uint inNumPhi, SoftBodySharedSettings::EBendType inBendType, const SoftBodySharedSettings::VertexAttributes &inVertexAttributes)
{
	// Create settings
	SoftBodySharedSettings *settings = new SoftBodySharedSettings;

	// Create vertices
	// NOTE: This is not how you should create a soft body sphere, we explicitly use polar coordinates to make the vertices unevenly distributed.
	// Doing it this way tests the pressure algorithm as it receives non-uniform triangles. Better is to use uniform triangles,
	// see the use of DebugRenderer::Create8thSphere for an example.
	SoftBodySharedSettings::Vertex v;
	(inRadius * Vec3::sUnitSpherical(0, 0)).StoreFloat3(&v.mPosition);
	settings->mVertices.push_back(v);
	(inRadius * Vec3::sUnitSpherical(JPH_PI, 0)).StoreFloat3(&v.mPosition);
	settings->mVertices.push_back(v);
	for (uint theta = 1; theta < inNumTheta - 1; ++theta)
		for (uint phi = 0; phi < inNumPhi; ++phi)
		{
			(inRadius * Vec3::sUnitSpherical(JPH_PI * theta / (inNumTheta - 1), 2.0f * JPH_PI * phi / inNumPhi)).StoreFloat3(&v.mPosition);
			settings->mVertices.push_back(v);
		}

	// Function to get the vertex index of a point on the sphere
	auto vertex_index = [inNumTheta, inNumPhi](uint inTheta, uint inPhi) -> uint
	{
		if (inTheta == 0)
			return 0;
		else if (inTheta == inNumTheta - 1)
			return 1;
		else
			return 2 + (inTheta - 1) * inNumPhi + inPhi % inNumPhi;
	};

	// Create faces
	SoftBodySharedSettings::Face f;
	for (uint phi = 0; phi < inNumPhi; ++phi)
	{
		for (uint theta = 0; theta < inNumTheta - 2; ++theta)
		{
			f.mVertex[0] = vertex_index(theta, phi);
			f.mVertex[1] = vertex_index(theta + 1, phi);
			f.mVertex[2] = vertex_index(theta + 1, phi + 1);
			settings->AddFace(f);

			if (theta > 0)
			{
				f.mVertex[1] = vertex_index(theta + 1, phi + 1);
				f.mVertex[2] = vertex_index(theta, phi + 1);
				settings->AddFace(f);
			}
		}

		f.mVertex[0] = vertex_index(inNumTheta - 2, phi + 1);
		f.mVertex[1] = vertex_index(inNumTheta - 2, phi);
		f.mVertex[2] = vertex_index(inNumTheta - 1, 0);
		settings->AddFace(f);
	}

	// Create constraints
	settings->CreateConstraints(&inVertexAttributes, 1, inBendType);

	// Optimize the settings
	settings->Optimize();

	return settings;
}

};
