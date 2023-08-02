// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>

namespace SoftBodyCreator
{
	/// Create a square cloth
	/// @param inGridSize Number of points along each axis
	/// @param inGridSpacing Distance between points
	/// @param inFixateCorners If the corners should be fixated and have infinite mass
	Ref<SoftBodySharedSettings>	CreateCloth(uint inGridSize = 30, float inGridSpacing = 0.75f, bool inFixateCorners = true);

	/// Create a cube
	/// @param inGridSize Number of points along each axis
	/// @param inGridSpacing Distance between points
	Ref<SoftBodySharedSettings>	CreateCube(uint inGridSize = 5, float inGridSpacing = 0.5f);

	/// Create a hollow sphere
	/// @param inRadius Radius of the sphere
	/// @param inNumTheta Number of segments in the theta direction
	/// @param inNumPhi Number of segments in the phi direction
	Ref<SoftBodySharedSettings>	CreateSphere(float inRadius = 1.0f, uint inNumTheta = 10, uint inNumPhi = 20);
};
