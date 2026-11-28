// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Math/Vec3.h>

JPH_NAMESPACE_BEGIN

static void sAddVertex(StaticArray<Vec3, 1026> &ioVertices, Vec3Arg inVertex)
{
	bool found = false;
	for (const Vec3 &v : ioVertices)
		if (v.IsClose(inVertex, 1.0e-6f))
		{
			found = true;
			break;
		}
	if (!found)
		ioVertices.push_back(inVertex);
}

/// Subdivides the triangle inDir1, inDir2, inDir3 inLevel + 1 times and adds all new vertices.
static void sCreateVertices(StaticArray<Vec3, 1026> &ioVertices, Vec3Arg inDir1, Vec3Arg inDir2, Vec3Arg inDir3, int inLevel)
{
	struct Triangle
	{
		Vec3	mDir1;
		Vec3	mDir2;
		Vec3	mDir3;
		int		mLevel;
	};

	// Every subdivision replaces 1 triangle with 4, so the stack grows by 3 per level
	constexpr int cMaxLevel = 3;
	JPH_ASSERT(inLevel >= 0 && inLevel <= cMaxLevel);
	Triangle stack[3 * cMaxLevel + 1];
	int stack_size = 0;
	stack[stack_size++] = { inDir1, inDir2, inDir3, inLevel };

	while (stack_size > 0)
	{
		Triangle t = stack[--stack_size];

		Vec3 center1 = (t.mDir1 + t.mDir2).Normalized();
		Vec3 center2 = (t.mDir2 + t.mDir3).Normalized();
		Vec3 center3 = (t.mDir3 + t.mDir1).Normalized();

		sAddVertex(ioVertices, center1);
		sAddVertex(ioVertices, center2);
		sAddVertex(ioVertices, center3);

		if (t.mLevel > 0)
		{
			int new_level = t.mLevel - 1;
			stack[stack_size++] = { center3, center2, t.mDir3, new_level };
			stack[stack_size++] = { center1, t.mDir2, center2, new_level };
			stack[stack_size++] = { center1, center2, center3, new_level };
			stack[stack_size++] = { t.mDir1, center1, center3, new_level };
		}
	}
}

const StaticArray<Vec3, 1026> Vec3::sUnitSphere = []() {

	const int level = 3;

	StaticArray<Vec3, 1026> verts;

	// Add unit axis
	verts.push_back(Vec3::sAxisX());
	verts.push_back(-Vec3::sAxisX());
	verts.push_back(Vec3::sAxisY());
	verts.push_back(-Vec3::sAxisY());
	verts.push_back(Vec3::sAxisZ());
	verts.push_back(-Vec3::sAxisZ());

	// Subdivide
	sCreateVertices(verts, Vec3::sAxisX(), Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, Vec3::sAxisX(), -Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), -Vec3::sAxisY(), Vec3::sAxisZ(), level);
	sCreateVertices(verts, Vec3::sAxisX(), Vec3::sAxisY(), -Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), Vec3::sAxisY(), -Vec3::sAxisZ(), level);
	sCreateVertices(verts, Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), level);
	sCreateVertices(verts, -Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), level);

	JPH_ASSERT(verts.size() == verts.capacity());
	return verts;
}();

JPH_NAMESPACE_END
