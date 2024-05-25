// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>

namespace SoftBodyCreator
{
	/// Create a square cloth
	/// @param inGridSizeX Number of points along the X axis
	/// @param inGridSizeZ Number of points along the Z axis
	/// @param inGridSpacing Distance between points
	/// @param inVertexGetInvMass Function that determines the inverse mass of each vertex
	Ref<SoftBodySharedSettings>	CreateCloth(uint inGridSizeX = 30, uint inGridSizeZ = 30, float inGridSpacing = 0.75f, const function<float(uint, uint)> &inVertexGetInvMass = [](uint, uint) { return 1.0f; }, const function<Vec3(uint, uint)> &inVertexPerturbation = [](uint, uint) { return Vec3::sZero(); }, SoftBodySharedSettings::EBendType inBendType = SoftBodySharedSettings::EBendType::None, const SoftBodySharedSettings::VertexAttributes &inVertexAttributes = { 1.0e-5f, 1.0e-5f, 1.0e-5f });

	/// Same as above but fixates the corners of the cloth
	Ref<SoftBodySharedSettings>	CreateClothWithFixatedCorners(uint inGridSizeX = 30, uint inGridSizeZ = 30, float inGridSpacing = 0.75f);

	/// Create a cube
	/// @param inGridSize Number of points along each axis
	/// @param inGridSpacing Distance between points
	Ref<SoftBodySharedSettings>	CreateCube(uint inGridSize = 5, float inGridSpacing = 0.5f);

	/// Create a hollow sphere
	/// @param inRadius Radius of the sphere
	/// @param inNumTheta Number of segments in the theta direction
	/// @param inNumPhi Number of segments in the phi direction
	Ref<SoftBodySharedSettings>	CreateSphere(float inRadius = 1.0f, uint inNumTheta = 10, uint inNumPhi = 20, SoftBodySharedSettings::EBendType inBendType = SoftBodySharedSettings::EBendType::None, const SoftBodySharedSettings::VertexAttributes &inVertexAttributes = { 1.0e-4f, 1.0e-4f, 1.0e-3f });
};
