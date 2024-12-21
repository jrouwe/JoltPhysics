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
