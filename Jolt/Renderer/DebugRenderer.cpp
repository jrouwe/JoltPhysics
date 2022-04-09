// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_DEBUG_RENDERER

#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Geometry/OrientedBox.h>

JPH_NAMESPACE_BEGIN

DebugRenderer *DebugRenderer::sInstance = nullptr;

// Number of LOD levels to create
static const int sMaxLevel = 4;

// Distance for each LOD level, these are tweaked for an object of approx. size 1. Use the lod scale to scale these distances.
static const float sLODDistanceForLevel[] = { 5.0f, 10.0f, 40.0f, FLT_MAX };

DebugRenderer::Triangle::Triangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor)
{
	// Set position
	inV1.StoreFloat3(&mV[0].mPosition);
	inV2.StoreFloat3(&mV[1].mPosition);
	inV3.StoreFloat3(&mV[2].mPosition);

	// Set color
	mV[0].mColor = mV[1].mColor = mV[2].mColor = inColor;

	// Calculate normal
	Vec3 normal = (inV2 - inV1).Cross(inV3 - inV1);
	float normal_len = normal.Length();
	if (normal_len > 0.0f)
		normal /= normal_len;
	Float3 normal3;
	normal.StoreFloat3(&normal3);
	mV[0].mNormal = mV[1].mNormal = mV[2].mNormal = normal3;

	// Reset UV's
	mV[0].mUV = mV[1].mUV = mV[2].mUV = { 0, 0 };
}

DebugRenderer::Triangle::Triangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor, Vec3Arg inUVOrigin, Vec3Arg inUVDirection)
{
	// Set position
	inV1.StoreFloat3(&mV[0].mPosition);
	inV2.StoreFloat3(&mV[1].mPosition);
	inV3.StoreFloat3(&mV[2].mPosition);

	// Set color
	mV[0].mColor = mV[1].mColor = mV[2].mColor = inColor;

	// Calculate normal
	Vec3 normal = (inV2 - inV1).Cross(inV3 - inV1).Normalized();	
	Float3 normal3;
	normal.StoreFloat3(&normal3);
	mV[0].mNormal = mV[1].mNormal = mV[2].mNormal = normal3;

	// Set UV's
	Vec3 uv1 = inV1 - inUVOrigin;
	Vec3 uv2 = inV2 - inUVOrigin;
	Vec3 uv3 = inV3 - inUVOrigin;
	Vec3 axis2 = normal.Cross(inUVDirection);
	mV[0].mUV = { inUVDirection.Dot(uv1), axis2.Dot(uv1) };
	mV[1].mUV = { inUVDirection.Dot(uv2), axis2.Dot(uv2) };
	mV[2].mUV = { inUVDirection.Dot(uv3), axis2.Dot(uv3) };
}

DebugRenderer::DebugRenderer()
{
	// Store singleton
	JPH_ASSERT(sInstance == nullptr);
	sInstance = this;
}

DebugRenderer::~DebugRenderer()
{
	JPH_ASSERT(sInstance == this);
	sInstance = nullptr;
}

void DebugRenderer::DrawWireBox(const AABox &inBox, ColorArg inColor)
{
	JPH_PROFILE_FUNCTION();

	// 8 vertices
	Float3 v1(inBox.mMin.GetX(), inBox.mMin.GetY(), inBox.mMin.GetZ());
	Float3 v2(inBox.mMin.GetX(), inBox.mMin.GetY(), inBox.mMax.GetZ());
	Float3 v3(inBox.mMin.GetX(), inBox.mMax.GetY(), inBox.mMin.GetZ());
	Float3 v4(inBox.mMin.GetX(), inBox.mMax.GetY(), inBox.mMax.GetZ());
	Float3 v5(inBox.mMax.GetX(), inBox.mMin.GetY(), inBox.mMin.GetZ());
	Float3 v6(inBox.mMax.GetX(), inBox.mMin.GetY(), inBox.mMax.GetZ());
	Float3 v7(inBox.mMax.GetX(), inBox.mMax.GetY(), inBox.mMin.GetZ());
	Float3 v8(inBox.mMax.GetX(), inBox.mMax.GetY(), inBox.mMax.GetZ());

	// 12 edges
	DrawLine(v1, v2, inColor);
	DrawLine(v1, v3, inColor);
	DrawLine(v1, v5, inColor);
	DrawLine(v2, v4, inColor);
	DrawLine(v2, v6, inColor);
	DrawLine(v3, v4, inColor);
	DrawLine(v3, v7, inColor);
	DrawLine(v4, v8, inColor);
	DrawLine(v5, v6, inColor);
	DrawLine(v5, v7, inColor);
	DrawLine(v6, v8, inColor);
	DrawLine(v7, v8, inColor);
}

void DebugRenderer::DrawWireBox(const OrientedBox &inBox, ColorArg inColor)
{
	JPH_PROFILE_FUNCTION();

	// 8 vertices
	Vec3 v1 = inBox.mOrientation * Vec3(-inBox.mHalfExtents.GetX(), -inBox.mHalfExtents.GetY(), -inBox.mHalfExtents.GetZ());
	Vec3 v2 = inBox.mOrientation * Vec3(-inBox.mHalfExtents.GetX(), -inBox.mHalfExtents.GetY(), inBox.mHalfExtents.GetZ());
	Vec3 v3 = inBox.mOrientation * Vec3(-inBox.mHalfExtents.GetX(), inBox.mHalfExtents.GetY(), -inBox.mHalfExtents.GetZ());
	Vec3 v4 = inBox.mOrientation * Vec3(-inBox.mHalfExtents.GetX(), inBox.mHalfExtents.GetY(), inBox.mHalfExtents.GetZ());
	Vec3 v5 = inBox.mOrientation * Vec3(inBox.mHalfExtents.GetX(), -inBox.mHalfExtents.GetY(), -inBox.mHalfExtents.GetZ());
	Vec3 v6 = inBox.mOrientation * Vec3(inBox.mHalfExtents.GetX(), -inBox.mHalfExtents.GetY(), inBox.mHalfExtents.GetZ());
	Vec3 v7 = inBox.mOrientation * Vec3(inBox.mHalfExtents.GetX(), inBox.mHalfExtents.GetY(), -inBox.mHalfExtents.GetZ());
	Vec3 v8 = inBox.mOrientation * Vec3(inBox.mHalfExtents.GetX(), inBox.mHalfExtents.GetY(), inBox.mHalfExtents.GetZ());

	// 12 edges
	DrawLine(v1, v2, inColor);
	DrawLine(v1, v3, inColor);
	DrawLine(v1, v5, inColor);
	DrawLine(v2, v4, inColor);
	DrawLine(v2, v6, inColor);
	DrawLine(v3, v4, inColor);
	DrawLine(v3, v7, inColor);
	DrawLine(v4, v8, inColor);
	DrawLine(v5, v6, inColor);
	DrawLine(v5, v7, inColor);
	DrawLine(v6, v8, inColor);
	DrawLine(v7, v8, inColor);
}

void DebugRenderer::DrawWireBox(Mat44Arg inMatrix, const AABox &inBox, ColorArg inColor)
{
	JPH_PROFILE_FUNCTION();

	// 8 vertices
	Vec3 v1 = inMatrix * Vec3(inBox.mMin.GetX(), inBox.mMin.GetY(), inBox.mMin.GetZ());
	Vec3 v2 = inMatrix * Vec3(inBox.mMin.GetX(), inBox.mMin.GetY(), inBox.mMax.GetZ());
	Vec3 v3 = inMatrix * Vec3(inBox.mMin.GetX(), inBox.mMax.GetY(), inBox.mMin.GetZ());
	Vec3 v4 = inMatrix * Vec3(inBox.mMin.GetX(), inBox.mMax.GetY(), inBox.mMax.GetZ());
	Vec3 v5 = inMatrix * Vec3(inBox.mMax.GetX(), inBox.mMin.GetY(), inBox.mMin.GetZ());
	Vec3 v6 = inMatrix * Vec3(inBox.mMax.GetX(), inBox.mMin.GetY(), inBox.mMax.GetZ());
	Vec3 v7 = inMatrix * Vec3(inBox.mMax.GetX(), inBox.mMax.GetY(), inBox.mMin.GetZ());
	Vec3 v8 = inMatrix * Vec3(inBox.mMax.GetX(), inBox.mMax.GetY(), inBox.mMax.GetZ());

	// 12 edges
	DrawLine(v1, v2, inColor);
	DrawLine(v1, v3, inColor);
	DrawLine(v1, v5, inColor);
	DrawLine(v2, v4, inColor);
	DrawLine(v2, v6, inColor);
	DrawLine(v3, v4, inColor);
	DrawLine(v3, v7, inColor);
	DrawLine(v4, v8, inColor);
	DrawLine(v5, v6, inColor);
	DrawLine(v5, v7, inColor);
	DrawLine(v6, v8, inColor);
	DrawLine(v7, v8, inColor);
}

void DebugRenderer::DrawMarker(Vec3Arg inPosition, ColorArg inColor, float inSize)
{
	JPH_PROFILE_FUNCTION();

	Vec3 dx(inSize, 0, 0);
	Vec3 dy(0, inSize, 0);
	Vec3 dz(0, 0, inSize);
	DrawLine(inPosition - dy, inPosition + dy, inColor);
	DrawLine(inPosition - dx, inPosition + dx, inColor);
	DrawLine(inPosition - dz, inPosition + dz, inColor);
}

void DebugRenderer::DrawArrow(Vec3Arg inFrom, Vec3Arg inTo, ColorArg inColor, float inSize)
{
	JPH_PROFILE_FUNCTION();

	// Draw base line
	DrawLine(inFrom, inTo, inColor);

	if (inSize > 0.0f)
	{
		// Draw arrow head
		Vec3 dir = inTo - inFrom;
		float len = dir.Length();
		if (len != 0.0f)
			dir = dir * (inSize / len);
		else
			dir = Vec3(inSize, 0, 0);
		Vec3 perp = inSize * dir.GetNormalizedPerpendicular();
		DrawLine(inTo - dir + perp, inTo, inColor);
		DrawLine(inTo - dir - perp, inTo, inColor);
	}
}

void DebugRenderer::DrawCoordinateSystem(Mat44Arg inTransform, float inSize)
{
	JPH_PROFILE_FUNCTION();

	DrawArrow(inTransform.GetTranslation(), inTransform * Vec3(inSize, 0, 0), Color::sRed, 0.1f * inSize);
	DrawArrow(inTransform.GetTranslation(), inTransform * Vec3(0, inSize, 0), Color::sGreen, 0.1f * inSize);
	DrawArrow(inTransform.GetTranslation(), inTransform * Vec3(0, 0, inSize), Color::sBlue, 0.1f * inSize);
}

void DebugRenderer::DrawPlane(Vec3Arg inPoint, Vec3Arg inNormal, ColorArg inColor, float inSize)
{
	// Create orthogonal basis
	Vec3 perp1 = inNormal.Cross(Vec3::sAxisY()).NormalizedOr(Vec3::sAxisX());
	Vec3 perp2 = perp1.Cross(inNormal).Normalized();
	perp1 = inNormal.Cross(perp2);

	// Calculate corners
	Vec3 corner1 = inPoint + inSize * (perp1 + perp2);
	Vec3 corner2 = inPoint + inSize * (perp1 - perp2);
	Vec3 corner3 = inPoint + inSize * (-perp1 - perp2);
	Vec3 corner4 = inPoint + inSize * (-perp1 + perp2);

	// Draw cross
	DrawLine(corner1, corner3, inColor);
	DrawLine(corner2, corner4, inColor);

	// Draw square
	DrawLine(corner1, corner2, inColor);
	DrawLine(corner2, corner3, inColor);
	DrawLine(corner3, corner4, inColor);
	DrawLine(corner4, corner1, inColor);

	// Draw normal
	DrawArrow(inPoint, inPoint + inSize * inNormal, inColor, 0.1f * inSize);
}

void DebugRenderer::DrawWireTriangle(Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, ColorArg inColor)
{
	JPH_PROFILE_FUNCTION();

	DrawLine(inV1, inV2, inColor);
	DrawLine(inV2, inV3, inColor);
	DrawLine(inV3, inV1, inColor);
}

void DebugRenderer::DrawWireSphere(Vec3Arg inCenter, float inRadius, ColorArg inColor, int inLevel)
{
	Mat44 matrix = Mat44::sTranslation(inCenter) * Mat44::sScale(inRadius);

	DrawWireUnitSphere(matrix, inColor, inLevel);
}

void DebugRenderer::DrawWireUnitSphere(Mat44Arg inMatrix, ColorArg inColor, int inLevel)
{
	JPH_PROFILE_FUNCTION();

	DrawWireUnitSphereRecursive(inMatrix, inColor, Vec3::sAxisX(), Vec3::sAxisY(), Vec3::sAxisZ(), inLevel);
	DrawWireUnitSphereRecursive(inMatrix, inColor, -Vec3::sAxisX(), Vec3::sAxisY(), Vec3::sAxisZ(), inLevel);
	DrawWireUnitSphereRecursive(inMatrix, inColor, Vec3::sAxisX(), -Vec3::sAxisY(), Vec3::sAxisZ(), inLevel);
	DrawWireUnitSphereRecursive(inMatrix, inColor, -Vec3::sAxisX(), -Vec3::sAxisY(), Vec3::sAxisZ(), inLevel);
	DrawWireUnitSphereRecursive(inMatrix, inColor, Vec3::sAxisX(), Vec3::sAxisY(), -Vec3::sAxisZ(), inLevel);
	DrawWireUnitSphereRecursive(inMatrix, inColor, -Vec3::sAxisX(), Vec3::sAxisY(), -Vec3::sAxisZ(), inLevel);
	DrawWireUnitSphereRecursive(inMatrix, inColor, Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), inLevel);
	DrawWireUnitSphereRecursive(inMatrix, inColor, -Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), inLevel);
}

void DebugRenderer::DrawWireUnitSphereRecursive(Mat44Arg inMatrix, ColorArg inColor, Vec3Arg inDir1, Vec3Arg inDir2, Vec3Arg inDir3, int inLevel)
{
	if (inLevel == 0)
	{
		Vec3 d1 = inMatrix * inDir1;
		Vec3 d2 = inMatrix * inDir2;
		Vec3 d3 = inMatrix * inDir3;

		DrawLine(d1, d2, inColor);
		DrawLine(d2, d3, inColor);
		DrawLine(d3, d1, inColor);
	}
	else
	{
		Vec3 center1 = (inDir1 + inDir2).Normalized();
		Vec3 center2 = (inDir2 + inDir3).Normalized();
		Vec3 center3 = (inDir3 + inDir1).Normalized();

		DrawWireUnitSphereRecursive(inMatrix, inColor, inDir1, center1, center3, inLevel - 1);
		DrawWireUnitSphereRecursive(inMatrix, inColor, center1, center2, center3, inLevel - 1);
		DrawWireUnitSphereRecursive(inMatrix, inColor, center1, inDir2, center2, inLevel - 1);
		DrawWireUnitSphereRecursive(inMatrix, inColor, center3, center2, inDir3, inLevel - 1);
	}
}

void DebugRenderer::Create8thSphereRecursive(vector<uint32> &ioIndices, vector<Vertex> &ioVertices, Vec3Arg inDir1, uint32 &ioIdx1, Vec3Arg inDir2, uint32 &ioIdx2, Vec3Arg inDir3, uint32 &ioIdx3, const Float2 &inUV, SupportFunction inGetSupport, int inLevel)
{
	if (inLevel == 0)
	{
		if (ioIdx1 == 0xffffffff)
		{
			ioIdx1 = (uint32)ioVertices.size();
			Float3 position, normal;
			inGetSupport(inDir1).StoreFloat3(&position);
			inDir1.StoreFloat3(&normal);
			ioVertices.push_back({ position, normal, inUV, Color::sWhite });
		}

		if (ioIdx2 == 0xffffffff)
		{
			ioIdx2 = (uint32)ioVertices.size();
			Float3 position, normal;
			inGetSupport(inDir2).StoreFloat3(&position);
			inDir2.StoreFloat3(&normal);
			ioVertices.push_back({ position, normal, inUV, Color::sWhite });
		}
		
		if (ioIdx3 == 0xffffffff)
		{
			ioIdx3 = (uint32)ioVertices.size();
			Float3 position, normal;
			inGetSupport(inDir3).StoreFloat3(&position);
			inDir3.StoreFloat3(&normal);
			ioVertices.push_back({ position, normal, inUV, Color::sWhite });
		}

		ioIndices.push_back(ioIdx1);
		ioIndices.push_back(ioIdx2);
		ioIndices.push_back(ioIdx3);
	}
	else
	{
		Vec3 center1 = (inDir1 + inDir2).Normalized();
		Vec3 center2 = (inDir2 + inDir3).Normalized();
		Vec3 center3 = (inDir3 + inDir1).Normalized();

		uint32 idx1 = 0xffffffff;
		uint32 idx2 = 0xffffffff;
		uint32 idx3 = 0xffffffff;
		
		Create8thSphereRecursive(ioIndices, ioVertices, inDir1,  ioIdx1, center1, idx1,   center3, idx3,   inUV, inGetSupport, inLevel - 1);
		Create8thSphereRecursive(ioIndices, ioVertices, center1, idx1,	  center2, idx2,   center3, idx3,   inUV, inGetSupport, inLevel - 1);
		Create8thSphereRecursive(ioIndices, ioVertices, center1, idx1,   inDir2,  ioIdx2, center2, idx2,   inUV, inGetSupport, inLevel - 1);
		Create8thSphereRecursive(ioIndices, ioVertices, center3, idx3,   center2, idx2,   inDir3,  ioIdx3, inUV, inGetSupport, inLevel - 1);
	}
}

void DebugRenderer::Create8thSphere(vector<uint32> &ioIndices, vector<Vertex> &ioVertices, Vec3Arg inDir1, Vec3Arg inDir2, Vec3Arg inDir3, const Float2 &inUV, SupportFunction inGetSupport, int inLevel)
{
	uint32 idx1 = 0xffffffff;
	uint32 idx2 = 0xffffffff;
	uint32 idx3 = 0xffffffff;

	Create8thSphereRecursive(ioIndices, ioVertices, inDir1, idx1, inDir2, idx2, inDir3, idx3, inUV, inGetSupport, inLevel);
}

void DebugRenderer::CreateQuad(vector<uint32> &ioIndices, vector<Vertex> &ioVertices, Vec3Arg inV1, Vec3Arg inV2, Vec3Arg inV3, Vec3Arg inV4)
{
	// Make room
	uint32 start_idx = uint32(ioVertices.size());
	ioVertices.resize(start_idx + 4);
	Vertex *vertices = &ioVertices[start_idx];

	// Set position
	inV1.StoreFloat3(&vertices[0].mPosition);
	inV2.StoreFloat3(&vertices[1].mPosition);
	inV3.StoreFloat3(&vertices[2].mPosition);
	inV4.StoreFloat3(&vertices[3].mPosition);

	// Set color
	vertices[0].mColor = vertices[1].mColor = vertices[2].mColor = vertices[3].mColor = Color::sWhite;

	// Calculate normal
	Vec3 normal = (inV2 - inV1).Cross(inV3 - inV1).Normalized();	
	Float3 normal3;
	normal.StoreFloat3(&normal3);
	vertices[0].mNormal = vertices[1].mNormal = vertices[2].mNormal = vertices[3].mNormal = normal3;

	// Set UV's
	vertices[0].mUV = { 0, 0 };
	vertices[1].mUV = { 2, 0 };
	vertices[2].mUV = { 2, 2 };
	vertices[3].mUV = { 0, 2 };

	// Set indices
	ioIndices.push_back(start_idx);
	ioIndices.push_back(start_idx + 1);
	ioIndices.push_back(start_idx + 2);

	ioIndices.push_back(start_idx);
	ioIndices.push_back(start_idx + 2);
	ioIndices.push_back(start_idx + 3);
}

void DebugRenderer::Initialize()
{
	// Box
	{
		vector<Vertex> box_vertices;
		vector<uint32> box_indices;

		// Get corner points
		Vec3 v0 = Vec3(-1,  1, -1);
		Vec3 v1 = Vec3( 1,  1, -1);
		Vec3 v2 = Vec3( 1,  1,  1);
		Vec3 v3 = Vec3(-1,  1,  1);
		Vec3 v4 = Vec3(-1, -1, -1);
		Vec3 v5 = Vec3( 1, -1, -1);
		Vec3 v6 = Vec3( 1, -1,  1);
		Vec3 v7 = Vec3(-1, -1,  1);

		// Top
		CreateQuad(box_indices, box_vertices, v0, v3, v2, v1);

		// Bottom
		CreateQuad(box_indices, box_vertices, v4, v5, v6, v7);

		// Left
		CreateQuad(box_indices, box_vertices, v0, v4, v7, v3);

		// Right
		CreateQuad(box_indices, box_vertices, v2, v6, v5, v1);

		// Front
		CreateQuad(box_indices, box_vertices, v3, v7, v6, v2);

		// Back
		CreateQuad(box_indices, box_vertices, v0, v1, v5, v4);

		mBox = new Geometry(CreateTriangleBatch(box_vertices, box_indices), AABox(Vec3(-1, -1, -1), Vec3(1, 1, 1)));
	}

	// Support function that returns a unit sphere
	auto sphere_support = [](Vec3Arg inDirection) { return inDirection; };

	// Construct geometries
	mSphere = new Geometry(AABox(Vec3(-1, -1, -1), Vec3(1, 1, 1)));
	mCapsuleBottom = new Geometry(AABox(Vec3(-1, -1, -1), Vec3(1, 0, 1)));
	mCapsuleTop = new Geometry(AABox(Vec3(-1, 0, -1), Vec3(1, 1, 1)));
	mCapsuleMid = new Geometry(AABox(Vec3(-1, -1, -1), Vec3(1, 1, 1)));
	mOpenCone = new Geometry(AABox(Vec3(-1, 0, -1), Vec3(1, 1, 1)));
	mCylinder = new Geometry(AABox(Vec3(-1, -1, -1), Vec3(1, 1, 1)));

	// Iterate over levels
	for (int level = sMaxLevel; level >= 1; --level)
	{
		// Determine at which distance this level should be active
		float distance = sLODDistanceForLevel[sMaxLevel - level];

		// Sphere
		mSphere->mLODs.push_back({ CreateTriangleBatchForConvex(sphere_support, level), distance });

		// Capsule bottom half sphere
		{
			vector<Vertex> capsule_bottom_vertices;
			vector<uint32> capsule_bottom_indices;
			Create8thSphere(capsule_bottom_indices, capsule_bottom_vertices, -Vec3::sAxisX(), -Vec3::sAxisY(),  Vec3::sAxisZ(), Float2(0.25f, 0.25f), sphere_support, level);
			Create8thSphere(capsule_bottom_indices, capsule_bottom_vertices, -Vec3::sAxisY(),  Vec3::sAxisX(),  Vec3::sAxisZ(), Float2(0.25f, 0.75f), sphere_support, level);
			Create8thSphere(capsule_bottom_indices, capsule_bottom_vertices,  Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), Float2(0.25f, 0.25f), sphere_support, level);
			Create8thSphere(capsule_bottom_indices, capsule_bottom_vertices, -Vec3::sAxisY(), -Vec3::sAxisX(), -Vec3::sAxisZ(), Float2(0.25f, 0.75f), sphere_support, level);
			mCapsuleBottom->mLODs.push_back({ CreateTriangleBatch(capsule_bottom_vertices, capsule_bottom_indices), distance });
		}

		// Capsule top half sphere
		{
			vector<Vertex> capsule_top_vertices;
			vector<uint32> capsule_top_indices;
			Create8thSphere(capsule_top_indices, capsule_top_vertices,  Vec3::sAxisX(),  Vec3::sAxisY(),  Vec3::sAxisZ(), Float2(0.25f, 0.75f), sphere_support, level);
			Create8thSphere(capsule_top_indices, capsule_top_vertices,  Vec3::sAxisY(), -Vec3::sAxisX(),  Vec3::sAxisZ(), Float2(0.25f, 0.25f), sphere_support, level);
			Create8thSphere(capsule_top_indices, capsule_top_vertices,  Vec3::sAxisY(),  Vec3::sAxisX(), -Vec3::sAxisZ(), Float2(0.25f, 0.25f), sphere_support, level);
			Create8thSphere(capsule_top_indices, capsule_top_vertices, -Vec3::sAxisX(),  Vec3::sAxisY(), -Vec3::sAxisZ(), Float2(0.25f, 0.75f), sphere_support, level);
			mCapsuleTop->mLODs.push_back({ CreateTriangleBatch(capsule_top_vertices, capsule_top_indices), distance });
		}

		// Capsule middle part
		{
			vector<Vertex> capsule_mid_vertices;
			vector<uint32> capsule_mid_indices;
			for (int q = 0; q < 4; ++q)
			{
				Float2 uv = (q & 1) == 0? Float2(0.25f, 0.25f) : Float2(0.25f, 0.75f);
		
				uint32 start_idx = (uint32)capsule_mid_vertices.size();
		
				int num_parts = 1 << level;
				for (int i = 0; i <= num_parts; ++i)
				{
					float angle = 0.5f * JPH_PI * (float(q) + float(i) / num_parts);
					float s = sin(angle);
					float c = cos(angle);
					Float3 vt(s, 1.0f, c);
					Float3 vb(s, -1.0f, c);
					Float3 n(s, 0, c);

					capsule_mid_vertices.push_back({ vt, n, uv, Color::sWhite });
					capsule_mid_vertices.push_back({ vb, n, uv, Color::sWhite });
				}

				for (int i = 0; i < num_parts; ++i)
				{
					uint32 start = start_idx + 2 * i;

					capsule_mid_indices.push_back(start);
					capsule_mid_indices.push_back(start + 1);
					capsule_mid_indices.push_back(start + 3);

					capsule_mid_indices.push_back(start);
					capsule_mid_indices.push_back(start + 3);
					capsule_mid_indices.push_back(start + 2);
				}
			}
			mCapsuleMid->mLODs.push_back({ CreateTriangleBatch(capsule_mid_vertices, capsule_mid_indices), distance });
		}

		// Open cone
		{
			vector<Vertex> open_cone_vertices;
			vector<uint32> open_cone_indices;
			for (int q = 0; q < 4; ++q)
			{
				Float2 uv = (q & 1) == 0? Float2(0.25f, 0.25f) : Float2(0.25f, 0.75f);
		
				uint32 start_idx = (uint32)open_cone_vertices.size();
		
				int num_parts = 2 << level;
				Float3 vt(0, 0, 0);
				for (int i = 0; i <= num_parts; ++i)
				{
					// Calculate bottom vertex
					float angle = 0.5f * JPH_PI * (float(q) + float(i) / num_parts);
					float s = sin(angle);
					float c = cos(angle);
					Float3 vb(s, 1.0f, c);

					// Calculate normal
					// perpendicular = Y cross vb (perpendicular to the plane in which 0, y and vb exists)
					// normal = perpendicular cross vb (normal to the edge 0 vb)					
					Vec3 normal = Vec3(s, -Square(s) - Square(c), c).Normalized();
					Float3 n; normal.StoreFloat3(&n);

					open_cone_vertices.push_back({ vt, n, uv, Color::sWhite });
					open_cone_vertices.push_back({ vb, n, uv, Color::sWhite });
				}

				for (int i = 0; i < num_parts; ++i)
				{
					uint32 start = start_idx + 2 * i;

					open_cone_indices.push_back(start);
					open_cone_indices.push_back(start + 1);
					open_cone_indices.push_back(start + 3);
				}
			}
			mOpenCone->mLODs.push_back({ CreateTriangleBatch(open_cone_vertices, open_cone_indices), distance });
		}

		// Cylinder
		{
			vector<Vertex> cylinder_vertices;
			vector<uint32> cylinder_indices;
			for (int q = 0; q < 4; ++q)
			{
				Float2 uv = (q & 1) == 0? Float2(0.25f, 0.75f) : Float2(0.25f, 0.25f);
		
				uint32 center_start_idx = (uint32)cylinder_vertices.size();
		
				Float3 nt(0.0f, 1.0f, 0.0f);
				Float3 nb(0.0f, -1.0f, 0.0f);
				cylinder_vertices.push_back({ Float3(0.0f, 1.0f, 0.0f), nt, uv, Color::sWhite });
				cylinder_vertices.push_back({ Float3(0.0f, -1.0f, 0.0f), nb, uv, Color::sWhite });

				uint32 vtx_start_idx = (uint32)cylinder_vertices.size();

				int num_parts = 1 << level;
				for (int i = 0; i <= num_parts; ++i)
				{
					float angle = 0.5f * JPH_PI * (float(q) + float(i) / num_parts);
					float s = sin(angle);
					float c = cos(angle);
					Float3 vt(s, 1.0f, c);
					Float3 vb(s, -1.0f, c);
					Float3 n(s, 0, c);

					cylinder_vertices.push_back({ vt, nt, uv, Color::sWhite });
					cylinder_vertices.push_back({ vb, nb, uv, Color::sWhite });
					cylinder_vertices.push_back({ vt, n, uv, Color::sWhite });
					cylinder_vertices.push_back({ vb, n, uv, Color::sWhite });
				}

				for (int i = 0; i < num_parts; ++i)
				{
					uint32 start = vtx_start_idx + 4 * i;

					// Top
					cylinder_indices.push_back(center_start_idx);
					cylinder_indices.push_back(start);
					cylinder_indices.push_back(start + 4);

					// Bottom
					cylinder_indices.push_back(center_start_idx + 1);
					cylinder_indices.push_back(start + 5);
					cylinder_indices.push_back(start + 1);

					// Side
					cylinder_indices.push_back(start + 2);
					cylinder_indices.push_back(start + 3);
					cylinder_indices.push_back(start + 7);

					cylinder_indices.push_back(start + 2);
					cylinder_indices.push_back(start + 7);
					cylinder_indices.push_back(start + 6);
				}
			}
			mCylinder->mLODs.push_back({ CreateTriangleBatch(cylinder_vertices, cylinder_indices), distance });
		}
	}
}

AABox DebugRenderer::sCalculateBounds(const Vertex *inVertices, int inVertexCount) 
{
	AABox bounds;
	for (const Vertex *v = inVertices, *v_end = inVertices + inVertexCount; v < v_end; ++v)
		bounds.Encapsulate(Vec3(v->mPosition));
	return bounds;
}

DebugRenderer::Batch DebugRenderer::CreateTriangleBatch(const VertexList &inVertices, const IndexedTriangleNoMaterialList &inTriangles)
{
	JPH_PROFILE_FUNCTION();

	vector<Vertex> vertices;

	// Create render vertices
	vertices.resize(inVertices.size());
	for (size_t v = 0; v < inVertices.size(); ++v)
	{
		vertices[v].mPosition = inVertices[v];
		vertices[v].mNormal = Float3(0, 0, 0);
		vertices[v].mUV = Float2(0, 0);
		vertices[v].mColor = Color::sWhite;
	}

	// Calculate normals
	for (size_t i = 0; i < inTriangles.size(); ++i)
	{
		const IndexedTriangleNoMaterial &tri = inTriangles[i];

		// Calculate normal of face
		Vec3 vtx[3];
		for (int j = 0; j < 3; ++j)
			vtx[j] = Vec3::sLoadFloat3Unsafe(vertices[tri.mIdx[j]].mPosition);
		Vec3 normal = ((vtx[1] - vtx[0]).Cross(vtx[2] - vtx[0])).Normalized();

		// Add normal to all vertices in face
		for (int j = 0; j < 3; ++j)
			(Vec3::sLoadFloat3Unsafe(vertices[tri.mIdx[j]].mNormal) + normal).StoreFloat3(&vertices[tri.mIdx[j]].mNormal);
	}

	// Renormalize vertex normals
	for (size_t i = 0; i < vertices.size(); ++i)
		Vec3::sLoadFloat3Unsafe(vertices[i].mNormal).Normalized().StoreFloat3(&vertices[i].mNormal);

	return CreateTriangleBatch(&vertices[0], (int)vertices.size(), &inTriangles[0].mIdx[0], (int)(3 * inTriangles.size()));
}

DebugRenderer::Batch DebugRenderer::CreateTriangleBatchForConvex(SupportFunction inGetSupport, int inLevel, AABox *outBounds)
{
	JPH_PROFILE_FUNCTION();

	vector<Vertex> vertices;
	vector<uint32> indices;
	Create8thSphere(indices, vertices,  Vec3::sAxisX(),  Vec3::sAxisY(),  Vec3::sAxisZ(), Float2(0.25f, 0.25f), inGetSupport, inLevel);
	Create8thSphere(indices, vertices,  Vec3::sAxisY(), -Vec3::sAxisX(),  Vec3::sAxisZ(), Float2(0.25f, 0.75f), inGetSupport, inLevel);
	Create8thSphere(indices, vertices, -Vec3::sAxisY(),  Vec3::sAxisX(),  Vec3::sAxisZ(), Float2(0.25f, 0.75f), inGetSupport, inLevel);
	Create8thSphere(indices, vertices, -Vec3::sAxisX(), -Vec3::sAxisY(),  Vec3::sAxisZ(), Float2(0.25f, 0.25f), inGetSupport, inLevel);
	Create8thSphere(indices, vertices,  Vec3::sAxisY(),  Vec3::sAxisX(), -Vec3::sAxisZ(), Float2(0.25f, 0.75f), inGetSupport, inLevel);
	Create8thSphere(indices, vertices, -Vec3::sAxisX(),  Vec3::sAxisY(), -Vec3::sAxisZ(), Float2(0.25f, 0.25f), inGetSupport, inLevel);
	Create8thSphere(indices, vertices,  Vec3::sAxisX(), -Vec3::sAxisY(), -Vec3::sAxisZ(), Float2(0.25f, 0.25f), inGetSupport, inLevel);
	Create8thSphere(indices, vertices, -Vec3::sAxisY(), -Vec3::sAxisX(), -Vec3::sAxisZ(), Float2(0.25f, 0.75f), inGetSupport, inLevel);

	if (outBounds != nullptr)
		*outBounds = sCalculateBounds(&vertices[0], (int)vertices.size());

	return CreateTriangleBatch(vertices, indices);
}

DebugRenderer::GeometryRef DebugRenderer::CreateTriangleGeometryForConvex(SupportFunction inGetSupport)
{
	GeometryRef geometry;

	// Iterate over levels
	for (int level = sMaxLevel; level >= 1; --level)
	{
		// Determine at which distance this level should be active
		float distance = sLODDistanceForLevel[sMaxLevel - level];

		// Create triangle batch and only calculate bounds for highest LOD level
		AABox bounds;
		Batch batch = CreateTriangleBatchForConvex(inGetSupport, level, geometry == nullptr? &bounds : nullptr);

		// Construct geometry in the first iteration
		if (geometry == nullptr)
			geometry = new Geometry(bounds);

		// Add the LOD
		geometry->mLODs.push_back({ batch, distance });
	}

	return geometry;
}

void DebugRenderer::DrawBox(const AABox &inBox, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	Mat44 m = Mat44::sScale(inBox.GetExtent());
	m.SetTranslation(inBox.GetCenter());
	DrawGeometry(m, inColor, mBox, ECullMode::CullBackFace, inCastShadow, inDrawMode);
}

void DebugRenderer::DrawBox(Mat44Arg inMatrix, const AABox &inBox, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	Mat44 m = Mat44::sScale(inBox.GetExtent());
	m.SetTranslation(inBox.GetCenter());
	DrawGeometry(inMatrix * m, inColor, mBox, ECullMode::CullBackFace, inCastShadow, inDrawMode);
}

void DebugRenderer::DrawSphere(Vec3Arg inCenter, float inRadius, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	Mat44 matrix = Mat44::sTranslation(inCenter) * Mat44::sScale(inRadius);

	DrawUnitSphere(matrix, inColor, inCastShadow, inDrawMode);
}

void DebugRenderer::DrawUnitSphere(Mat44Arg inMatrix, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	DrawGeometry(inMatrix, inColor, mSphere, ECullMode::CullBackFace, inCastShadow, inDrawMode);
}

void DebugRenderer::DrawCapsule(Mat44Arg inMatrix, float inHalfHeightOfCylinder, float inRadius, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	Mat44 scale_matrix = Mat44::sScale(inRadius);

	// Calculate world space bounding box
	AABox local_bounds(Vec3(-inRadius, -inHalfHeightOfCylinder - inRadius, -inRadius), Vec3(inRadius, inHalfHeightOfCylinder + inRadius, inRadius));
	AABox world_bounds = local_bounds.Transformed(inMatrix);
	
	float radius_sq = Square(inRadius);

	// Draw bottom half sphere
	Mat44 bottom_matrix = inMatrix * Mat44::sTranslation(Vec3(0, -inHalfHeightOfCylinder, 0)) * scale_matrix;
	DrawGeometry(bottom_matrix, world_bounds, radius_sq, inColor, mCapsuleBottom, ECullMode::CullBackFace, inCastShadow, inDrawMode);

	// Draw top half sphere
	Mat44 top_matrix = inMatrix * Mat44::sTranslation(Vec3(0, inHalfHeightOfCylinder, 0)) * scale_matrix;
	DrawGeometry(top_matrix, world_bounds, radius_sq, inColor, mCapsuleTop, ECullMode::CullBackFace, inCastShadow, inDrawMode);

	// Draw middle part
	DrawGeometry(inMatrix * Mat44::sScale(Vec3(inRadius, inHalfHeightOfCylinder, inRadius)), world_bounds, radius_sq, inColor, mCapsuleMid, ECullMode::CullBackFace, inCastShadow, inDrawMode);
}

void DebugRenderer::DrawCylinder(Mat44Arg inMatrix, float inHalfHeight, float inRadius, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	Mat44 local_transform(Vec4(inRadius, 0, 0, 0), Vec4(0, inHalfHeight, 0, 0), Vec4(0, 0, inRadius, 0), Vec4(0, 0, 0, 1));
	Mat44 transform = inMatrix * local_transform;

	DrawGeometry(transform, mCylinder->mBounds.Transformed(transform), Square(inRadius), inColor, mCylinder, ECullMode::CullBackFace, inCastShadow, inDrawMode);
}

void DebugRenderer::DrawOpenCone(Vec3Arg inTop, Vec3Arg inAxis, Vec3Arg inPerpendicular, float inHalfAngle, float inLength, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inAxis.IsNormalized(1.0e-4f));
	JPH_ASSERT(inPerpendicular.IsNormalized(1.0e-4f));
	JPH_ASSERT(abs(inPerpendicular.Dot(inAxis)) < 1.0e-4f);

	Vec3 axis = Sign(inHalfAngle) * inLength * inAxis;
	float scale = inLength * tan(abs(inHalfAngle));
	if (scale != 0.0f)
	{
		Vec3 perp1 = scale * inPerpendicular;
		Vec3 perp2 = scale * inAxis.Cross(inPerpendicular);
		Mat44 transform(Vec4(perp1, 0), Vec4(axis, 0), Vec4(perp2, 0), Vec4(inTop, 1));
		DrawGeometry(transform, inColor, mOpenCone, ECullMode::Off, inCastShadow, inDrawMode);
	}
}

void DebugRenderer::DrawSwingLimits(Mat44Arg inMatrix, float inSwingYHalfAngle, float inSwingZHalfAngle, float inEdgeLength, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	JPH_PROFILE_FUNCTION();

	// Assert sane input
	JPH_ASSERT(inSwingYHalfAngle >= 0.0f && inSwingYHalfAngle <= JPH_PI);
	JPH_ASSERT(inSwingZHalfAngle >= 0.0f && inSwingZHalfAngle <= JPH_PI);
	JPH_ASSERT(inEdgeLength > 0.0f);

	// Check cache
	SwingLimits limits { inSwingYHalfAngle, inSwingZHalfAngle };
	GeometryRef &geometry = mSwingLimits[limits];
	if (geometry == nullptr)
	{
		// Number of segments to draw the cone with
		const int num_segments = 64;
		int half_num_segments = num_segments / 2;

		// The y and z values of the quaternion are limited to an ellipse, e1 and e2 are the radii of this ellipse
		float e1 = sin(0.5f * inSwingZHalfAngle);
		float e2 = sin(0.5f * inSwingYHalfAngle);

		// Check if the limits will draw something
		if ((e1 <= 0.0f && e2 <= 0.0f) || (e2 >= 1.0f && e1 >= 1.0f))
			return;

		// Calculate squared values
		float e1_sq = Square(e1);
		float e2_sq = Square(e2);

		// Allocate space for vertices
		int num_vertices = 2 * num_segments;
		Vertex *vertices_start = (Vertex *)JPH_STACK_ALLOC(num_vertices * sizeof(Vertex));
		Vertex *vertices = vertices_start;

		// Calculate local space vertices for shape
		Vec3 ls_vertices[num_segments];
		int tgt_vertex = 0;
		for (int side_iter = 0; side_iter < 2; ++side_iter)
			for (int segment_iter = 0; segment_iter < half_num_segments; ++segment_iter)
			{
				float y, z;
				if (e2_sq > e1_sq)
				{
					// Trace the y value of the quaternion
					y = e2 - 2.0f * segment_iter * e2 / half_num_segments;

					// Calculate the corresponding z value of the quaternion
					float z_sq = e1_sq - e1_sq / e2_sq * Square(y);
					z = z_sq <= 0.0f? 0.0f : sqrt(z_sq);
				}
				else
				{
					// Trace the z value of the quaternion
					z = -e1 + 2.0f * segment_iter * e1 / half_num_segments;

					// Calculate the corresponding y value of the quaternion
					float y_sq = e2_sq - e2_sq / e1_sq * Square(z);
					y = y_sq <= 0.0f? 0.0f : sqrt(y_sq);
				}

				// If we're tracing the opposite side, flip the values
				if (side_iter == 1)
				{
					z = -z;
					y = -y;
				}

				// Create quaternion
				Vec3 q_xyz(0, y, z);
				float w = sqrt(1.0f - q_xyz.LengthSq());
				Quat q(Vec4(q_xyz, w));

				// Store vertex
				ls_vertices[tgt_vertex++] = q.RotateAxisX();
			}

		for (int i = 0; i < num_segments; ++i)
		{
			// Get output vertices
			Vertex &top = *(vertices++);
			Vertex &bottom = *(vertices++);

			// Get local position
			Vec3 &pos = ls_vertices[i];

			// Get local normal
			Vec3 &prev_pos = ls_vertices[(i + num_segments - 1) % num_segments];
			Vec3 &next_pos = ls_vertices[(i + 1) % num_segments];
			Vec3 normal = 0.5f * (next_pos.Cross(pos).Normalized() + pos.Cross(prev_pos).Normalized());
			
			// Store top vertex
			top.mPosition = { 0, 0, 0 };
			normal.StoreFloat3(&top.mNormal);
			top.mColor = Color::sWhite;
			top.mUV = { 0, 0 };

			// Store bottom vertex
			pos.StoreFloat3(&bottom.mPosition);
			normal.StoreFloat3(&bottom.mNormal);
			bottom.mColor = Color::sWhite;
			bottom.mUV = { 0, 0 };
		}

		// Allocate space for indices
		int num_indices = 3 * num_segments;
		uint32 *indices_start = (uint32 *)JPH_STACK_ALLOC(num_indices * sizeof(uint32));
		uint32 *indices = indices_start;

		// Calculate indices
		for (int i = 0; i < num_segments; ++i)
		{
			int first = 2 * i;
			int second = (first + 3) % num_vertices;
			int third = first + 1;

			// Triangle
			*indices++ = first; 
			*indices++ = second;
			*indices++ = third;
		}

		// Convert to triangle batch
		geometry = new Geometry(CreateTriangleBatch(vertices_start, num_vertices, indices_start, num_indices), sCalculateBounds(vertices_start, num_vertices));
	}

	DrawGeometry(inMatrix * Mat44::sScale(inEdgeLength), inColor, geometry, ECullMode::Off, inCastShadow, inDrawMode);
}


void DebugRenderer::DrawPie(Vec3Arg inCenter, float inRadius, Vec3Arg inNormal, Vec3Arg inAxis, float inMinAngle, float inMaxAngle, ColorArg inColor, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	if (inMinAngle >= inMaxAngle)
		return;

	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inAxis.IsNormalized(1.0e-4f));
	JPH_ASSERT(inNormal.IsNormalized(1.0e-4f));
	JPH_ASSERT(abs(inNormal.Dot(inAxis)) < 1.0e-4f);
		
	// Pies have a unique batch based on the difference between min and max angle
	float delta_angle = inMaxAngle - inMinAngle;
	GeometryRef &geometry = mPieLimits[delta_angle];
	if (geometry == nullptr)
	{	
		int num_parts = (int)ceil(64.0f * delta_angle / (2.0f * JPH_PI));

		Float3 normal = { 0, 1, 0 };
		Float3 center = { 0, 0, 0 };

		// Allocate space for vertices
		int num_vertices = num_parts + 2;
		Vertex *vertices_start = (Vertex *)JPH_STACK_ALLOC(num_vertices * sizeof(Vertex));
		Vertex *vertices = vertices_start;

		// Center of circle
		*vertices++ = { center, normal, { 0, 0 }, Color::sWhite };
	
		// Outer edge of pie
		for (int i = 0; i <= num_parts; ++i)
		{
			float angle = float(i) / float(num_parts) * delta_angle;

			Float3 pos = { cos(angle), 0, sin(angle) };
			*vertices++ = { pos, normal, { 0, 0 }, Color::sWhite };
		}

		// Allocate space for indices
		int num_indices = num_parts * 3;
		uint32 *indices_start = (uint32 *)JPH_STACK_ALLOC(num_indices * sizeof(uint32));
		uint32 *indices = indices_start;

		for (int i = 0; i < num_parts; ++i)
		{
			*indices++ = 0;
			*indices++ = i + 1;
			*indices++ = i + 2;
		}

		// Convert to triangle batch
		geometry = new Geometry(CreateTriangleBatch(vertices_start, num_vertices, indices_start, num_indices), sCalculateBounds(vertices_start, num_vertices));
	}
	
	// Construct matrix that transforms pie into world space
	Mat44 matrix = Mat44(Vec4(inRadius * inAxis, 0), Vec4(inRadius * inNormal, 0), Vec4(inRadius * inNormal.Cross(inAxis), 0), Vec4(inCenter, 1)) * Mat44::sRotationY(-inMinAngle);
		
	DrawGeometry(matrix, inColor, geometry, ECullMode::Off, inCastShadow, inDrawMode);
}

JPH_NAMESPACE_END

#endif // JPH_DEBUG_RENDERER
