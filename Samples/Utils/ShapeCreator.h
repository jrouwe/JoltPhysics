// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/MeshShape.h>

namespace ShapeCreator
{
	/// Create a mesh shape in the shape of a torus
	/// @param inTorusRadius Radius of the torus ring
	/// @param inTubeRadius Radius of the torus tube
	/// @param inTorusSegments Number of segments around the torus
	/// @param inTubeSegments Number of segments around the tube of the torus
	/// @return The mesh shape
	ShapeRefC	CreateTorusMesh(float inTorusRadius, float inTubeRadius, uint inTorusSegments = 16, uint inTubeSegments = 16);
};
