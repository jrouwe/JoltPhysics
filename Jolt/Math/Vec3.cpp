// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/UnorderedSet.h>
#include <Jolt/Math/Vec3.h>

JPH_NAMESPACE_BEGIN

static void sCreateVertices(std::unordered_set<Vec3> &ioVertices, Vec3Arg inDir1, Vec3Arg inDir2, Vec3Arg inDir3, int inLevel)
{
	Vec3 center1 = (inDir1 + inDir2).Normalized();
	Vec3 center2 = (inDir2 + inDir3).Normalized();
	Vec3 center3 = (inDir3 + inDir1).Normalized();

	ioVertices.insert(center1);
	ioVertices.insert(center2);
	ioVertices.insert(center3);

	if (inLevel > 0)
	{
		int new_level = inLevel - 1;
		sCreateVertices(ioVertices, inDir1, center1, center3, new_level);
		sCreateVertices(ioVertices, center1, center2, center3, new_level);
		sCreateVertices(ioVertices, center1, inDir2, center2, new_level);
		sCreateVertices(ioVertices, center3, center2, inDir3, new_level);
	}
}

const std::vector<Vec3> Vec3::sUnitSphere = []() {

	const int level = 3;

	std::unordered_set<Vec3> verts;

	// Add unit axis
	verts.insert(Vec3::sAxisX());
	verts.insert(-Vec3::sAxisX());
	verts.insert(Vec3::sAxisY());
	verts.insert(-Vec3::sAxisY());
	verts.insert(Vec3::sAxisZ());
	verts.insert(-Vec3::sAxisZ());

	// Subdivide
	sCreateVertices(verts, Vec3::sAxisX(), Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, Vec3::sAxisX(), -Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), -Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, Vec3::sAxisX(), Vec3::sAxisY(), -Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), Vec3::sAxisY(), -Vec3::sAxisZ(), level);
	sCreateVertices(verts, Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), level);

	return std::vector<Vec3>(verts.begin(), verts.end());
}();

JPH_NAMESPACE_END
