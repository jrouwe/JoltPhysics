// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ConvexCollision/ClosestPointTest.h>
#include <Jolt/Geometry/ClosestPoint.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ClosestPointTest) 
{ 
	JPH_ADD_BASE_CLASS(ClosestPointTest, Test) 
}

void ClosestPointTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	//Vec3 pos = Vec3::sZero();
	Vec3 pos = inParams.mCameraState.mPos;

	{
		// Normal tetrahedron
		Vec3 a(2, 0, 0);
		Vec3 b(1, 0, 1);
		Vec3 c(2, 0, 1);
		Vec3 d(1, 1, 0);
						
		TestTetra(pos, a, b, c, d);
	}

	{
		// Inside out tetrahedron
		Vec3 a(2, -2, 0);
		Vec3 b(1, -2, 1);
		Vec3 c(2, -2, 1);
		Vec3 d(1, -3, 0);

		TestTetra(pos, a, b, c, d);
	}

	{
		// Degenerate tetrahedron
		Vec3 a(2, 3, 0);
		Vec3 b = a;
		Vec3 c(2, 3, 1);
		Vec3 d(1, 4, 0);

		TestTetra(pos, a, b, c, d);
	}

	{
		// Degenerate tetrahedron
		Vec3 a(2, 6, 0);
		Vec3 b(1, 6, 1);
		Vec3 c = a;
		Vec3 d(1, 7, 0);

		TestTetra(pos, a, b, c, d);
	}

	{
		// Degenerate tetrahedron
		Vec3 a(2, 9, 0);
		Vec3 b(1, 9, 1);
		Vec3 c(2, 9, 1);
		Vec3 d = a;

		TestTetra(pos, a, b, c, d);
	}

	{
		// Degenerate tetrahedron
		Vec3 a(2, 12, 0);
		Vec3 b(1, 12, 1);
		Vec3 c = b;
		Vec3 d(1, 13, 0);

		TestTetra(pos, a, b, c, d);
	}

	{
		// Degenerate tetrahedron
		Vec3 a(2, 15, 0);
		Vec3 b(1, 15, 1);
		Vec3 c(2, 15, 1);
		Vec3 d = b;

		TestTetra(pos, a, b, c, d);
	}

	{
		// Degenerate tetrahedron
		Vec3 a(2, 18, 0);
		Vec3 b(1, 18, 1);
		Vec3 c(2, 18, 1);
		Vec3 d = c;

		TestTetra(pos, a, b, c, d);
	}

	{
		// Normal tri
		Vec3 a(5, 0, 0);
		Vec3 b(4, 0, 1);
		Vec3 c(5, 0, 1);

		TestTri(pos, a, b, c);
	}

	{
		// Degenerate tri
		Vec3 a(5, 3, 0);
		Vec3 b = a;
		Vec3 c(5, 3, 1);

		TestTri(pos, a, b, c);
	}

	{
		// Degenerate tri
		Vec3 a(5, 6, 0);
		Vec3 b(4, 6, 1);
		Vec3 c = a;

		TestTri(pos, a, b, c);
	}

	{
		// Degenerate tri
		Vec3 a(5, 9, 0);
		Vec3 b(4, 9, 1);
		Vec3 c = b;

		TestTri(pos, a, b, c);
	}

	{
		// Normal line
		Vec3 a(10, 0, 0);
		Vec3 b(9, 0, 1);
		TestLine(pos, a, b);
	}
			   
	{
		// Degenerate line
		Vec3 a(10, 3, 0);
		Vec3 b = a;
		TestLine(pos, a, b);
	}
}

void ClosestPointTest::TestLine(Vec3Arg inPosition, Vec3Arg inA, Vec3Arg inB)
{
	Vec3 a = inA - inPosition;
	Vec3 b = inB - inPosition;
	
	uint32 set;
	Vec3 closest = ClosestPoint::GetClosestPointOnLine(a, b, set) + inPosition;

	mDebugRenderer->DrawLine(inA, inB, Color::sWhite);

	mDebugRenderer->DrawMarker(closest, Color::sRed, 0.1f);

	if (set & 0b0001)
		mDebugRenderer->DrawMarker(inA, Color::sYellow, 0.5f);
	if (set & 0b0010)
		mDebugRenderer->DrawMarker(inB, Color::sYellow, 0.5f);

	Vec3 a2 = inA - closest;
	Vec3 b2 = inB - closest;

	float u, v;
	ClosestPoint::GetBaryCentricCoordinates(a2, b2, u, v);
	mDebugRenderer->DrawWireSphere(u * inA + v * inB, 0.05f, Color::sGreen);

	mDebugRenderer->DrawText3D(inA, "a");
	mDebugRenderer->DrawText3D(inB, "b");
}

void ClosestPointTest::TestTri(Vec3Arg inPosition, Vec3Arg inA, Vec3Arg inB, Vec3Arg inC)
{
	Vec3 a = inA - inPosition;
	Vec3 b = inB - inPosition;
	Vec3 c = inC - inPosition;
	
	uint32 set;
	Vec3 closest = ClosestPoint::GetClosestPointOnTriangle(a, b, c, set) + inPosition;

	mDebugRenderer->DrawLine(inA, inB, Color::sWhite);
	mDebugRenderer->DrawLine(inA, inC, Color::sWhite);
	mDebugRenderer->DrawLine(inB, inC, Color::sWhite);

	mDebugRenderer->DrawTriangle(inA, inB, inC, Color::sGrey);

	mDebugRenderer->DrawMarker(closest, Color::sRed, 0.1f);

	if (set & 0b0001)
		mDebugRenderer->DrawMarker(inA, Color::sYellow, 0.5f);
	if (set & 0b0010)
		mDebugRenderer->DrawMarker(inB, Color::sYellow, 0.5f);
	if (set & 0b0100)
		mDebugRenderer->DrawMarker(inC, Color::sYellow, 0.5f);

	Vec3 a2 = inA - closest;
	Vec3 b2 = inB - closest;
	Vec3 c2 = inC - closest;

	float u, v, w;
	ClosestPoint::GetBaryCentricCoordinates(a2, b2, c2, u, v, w);
	mDebugRenderer->DrawWireSphere(u * inA + v * inB + w * inC, 0.05f, Color::sGreen);

	mDebugRenderer->DrawText3D(inA, "a");
	mDebugRenderer->DrawText3D(inB, "b");
	mDebugRenderer->DrawText3D(inC, "c");
}

void ClosestPointTest::TestTetra(Vec3Arg inPosition, Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, Vec3Arg inD)
{
	Vec3 a = inA - inPosition;
	Vec3 b = inB - inPosition;
	Vec3 c = inC - inPosition;
	Vec3 d = inD - inPosition;
	
	uint32 set;
	Vec3 closest = ClosestPoint::GetClosestPointOnTetrahedron(a, b, c, d, set) + inPosition;

	mDebugRenderer->DrawLine(inA, inB, Color::sWhite);
	mDebugRenderer->DrawLine(inA, inC, Color::sWhite);
	mDebugRenderer->DrawLine(inA, inD, Color::sWhite);
	mDebugRenderer->DrawLine(inB, inC, Color::sWhite);
	mDebugRenderer->DrawLine(inB, inD, Color::sWhite);
	mDebugRenderer->DrawLine(inC, inD, Color::sWhite);

	mDebugRenderer->DrawTriangle(inA, inC, inB, Color::sGrey);
	mDebugRenderer->DrawTriangle(inA, inD, inC, Color::sGrey);
	mDebugRenderer->DrawTriangle(inA, inB, inD, Color::sGrey);
	mDebugRenderer->DrawTriangle(inB, inC, inD, Color::sGrey);

	mDebugRenderer->DrawMarker(closest, Color::sRed, 0.1f);

	if (set & 0b0001)
		mDebugRenderer->DrawMarker(inA, Color::sYellow, 0.5f);
	if (set & 0b0010)
		mDebugRenderer->DrawMarker(inB, Color::sYellow, 0.5f);
	if (set & 0b0100)
		mDebugRenderer->DrawMarker(inC, Color::sYellow, 0.5f);
	if (set & 0b1000)
		mDebugRenderer->DrawMarker(inD, Color::sYellow, 0.5f);

	mDebugRenderer->DrawText3D(inA, "a");
	mDebugRenderer->DrawText3D(inB, "b");
	mDebugRenderer->DrawText3D(inC, "c");
	mDebugRenderer->DrawText3D(inD, "d");
}
