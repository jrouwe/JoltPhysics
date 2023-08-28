// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ConvexCollision/InteractivePairsTest.h>
#include <Input/Keyboard.h>
#include <Jolt/Geometry/Sphere.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Geometry/ConvexSupport.h>
#include <Jolt/Geometry/EPAPenetrationDepth.h>
#include <Utils/DebugRendererSP.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(InteractivePairsTest)
{
	JPH_ADD_BASE_CLASS(InteractivePairsTest, Test)
}

void InteractivePairsTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Keyboard controls
	if (inParams.mKeyboard->IsKeyPressed(DIK_Z))
	{
		mKeyboardMode = true;
		mDistance -= inParams.mDeltaTime;
	}
	else if (inParams.mKeyboard->IsKeyPressed(DIK_C))
	{
		mKeyboardMode = true;
		mDistance += inParams.mDeltaTime;
	}
	else if (inParams.mKeyboard->IsKeyPressed(DIK_X))
	{
		mKeyboardMode = false;
	}

	// Auto update
	if (!mKeyboardMode)
		mDistance -= inParams.mDeltaTime;

	// Clamp distance
	if (mDistance < -4.0f)
		mDistance = 4.0f;
	if (mDistance > 4.0f)
		mDistance = -4.0f;

	float z = 0.0f;

	const float r1 = 0.25f * JPH_PI;
	const float r2 = ATan(1.0f / sqrt(2.0f)); // When rotating cube by 45 degrees the one axis becomes sqrt(2) long while the other stays at length 1

	for (int i = 0; i < 2; ++i)
	{
		const float cvx_radius = i == 0? 0.0f : 0.1f; // First round without convex radius, second with
		const float edge_len = 1.0f - cvx_radius;
		AABox b(Vec3(-edge_len, -edge_len, -edge_len), Vec3(edge_len, edge_len, edge_len));

		// Face vs face
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, 0, 0), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(0, 0, 0), cvx_radius, b);
		z += 4;
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, 0, 0), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(r1, 0, 0), cvx_radius, b);
		z += 4;

		// Face vs edge
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, 0, 0), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(0, r1, 0), cvx_radius, b);
		z += 4;
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, 0, 0), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(0, 0, r1), cvx_radius, b);
		z += 4;

		// Face vs vertex
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, 0, 0), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(0, r2, r1), cvx_radius, b);
		z += 4;

		// Edge vs edge
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, r1, 0), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(0, r1, 0), cvx_radius, b);
		z += 4;
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, 0, r1), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(0, r1, 0), cvx_radius, b);
		z += 4;

		// Edge vs vertex
		TestBoxVsBox(Vec3(0, 0, z), Vec3(0, r2, r1), cvx_radius, b, Vec3(mDistance, 0, z), Vec3(0, r2, r1), cvx_radius, b);
		z += 4;

		// Sphere vs face
		TestSphereVsBox(Vec3(0, 0, z), 1.0f, Vec3(mDistance, 0, z), Vec3(0, 0, 0), cvx_radius, b);
		z += 4;
		TestSphereVsBox(Vec3(0, 0, z), 1.0f, Vec3(mDistance, 0, z), Vec3(r1, 0, 0), cvx_radius, b);
		z += 4;

		// Sphere vs edge
		TestSphereVsBox(Vec3(0, 0, z), 1.0f, Vec3(mDistance, 0, z), Vec3(0, r1, 0), cvx_radius, b);
		z += 4;
		TestSphereVsBox(Vec3(0, 0, z), 1.0f, Vec3(mDistance, 0, z), Vec3(0, 0, r1), cvx_radius, b);
		z += 4;

		// Sphere vs vertex
		TestSphereVsBox(Vec3(0, 0, z), 1.0f, Vec3(mDistance, 0, z), Vec3(0, r2, r1), cvx_radius, b);
		z += 4;

		// Sphere vs sphere
		TestSphereVsSphere(Vec3(0, 0, z), 1.0f, Vec3(mDistance, 0, z), 1.0f, i == 1);
		z += 4;
	}
}

void InteractivePairsTest::TestBoxVsBox(Vec3Arg inTranslationA, Vec3Arg inRotationA, float inConvexRadiusA, const AABox &inA, Vec3Arg inTranslationB, Vec3Arg inRotationB, float inConvexRadiusB, const AABox &inB)
{
	Mat44 mat_a = Mat44::sTranslation(inTranslationA) * Mat44::sRotationX(inRotationA.GetX()) * Mat44::sRotationY(inRotationA.GetY()) * Mat44::sRotationZ(inRotationA.GetZ());
	TransformedConvexObject<AABox> a(mat_a, inA);

	Mat44 mat_b = Mat44::sTranslation(inTranslationB) * Mat44::sRotationX(inRotationB.GetX()) * Mat44::sRotationY(inRotationB.GetY()) * Mat44::sRotationZ(inRotationB.GetZ());
	TransformedConvexObject<AABox> b(mat_b, inB);

	EPAPenetrationDepth pen_depth;
	Vec3 v = Vec3::sAxisX(), pa, pb;

	DrawBoxSP(mDebugRenderer, mat_a, inA, Color::sWhite);

	AABox widened_a = inA;
	widened_a.ExpandBy(Vec3::sReplicate(inConvexRadiusA));

	AABox widened_b = inB;
	widened_b.ExpandBy(Vec3::sReplicate(inConvexRadiusB));

	DrawBoxSP(mDebugRenderer, mat_a, inA, Color::sWhite);
	if (inConvexRadiusA > 0.0f)
		DrawWireBoxSP(mDebugRenderer, mat_a, widened_a, Color::sWhite);

	AddConvexRadius<TransformedConvexObject<AABox>> a_inc(a, inConvexRadiusA);
	AddConvexRadius<TransformedConvexObject<AABox>> b_inc(b, inConvexRadiusB);

	if (pen_depth.GetPenetrationDepth(a, a_inc, inConvexRadiusA, b, b_inc, inConvexRadiusB, 1.0e-4f, FLT_EPSILON, v, pa, pb))
	{
		DrawBoxSP(mDebugRenderer, mat_b, inB, Color::sRed);
		if (inConvexRadiusB > 0.0f)
			DrawWireBoxSP(mDebugRenderer, mat_b, widened_b, Color::sRed);
		DrawMarkerSP(mDebugRenderer, pa, Color::sYellow, 2.0f);
		DrawMarkerSP(mDebugRenderer, pb, Color::sCyan, 2.0f);
	}
	else
	{
		DrawBoxSP(mDebugRenderer, mat_b, inB, Color::sGreen);
		if (inConvexRadiusB > 0.0f)
			DrawWireBoxSP(mDebugRenderer, mat_b, widened_b, Color::sGreen);
	}
	DrawArrowSP(mDebugRenderer, inTranslationB + Vec3(0, 2, 0), inTranslationB + v + Vec3(0, 2, 0), Color::sOrange, 0.05f);
}

void InteractivePairsTest::TestSphereVsBox(Vec3Arg inTranslationA, float inRadiusA, Vec3Arg inTranslationB, Vec3Arg inRotationB, float inConvexRadiusB, const AABox &inB)
{
	Sphere s(inTranslationA, inRadiusA);
	Mat44 mat_b = Mat44::sTranslation(inTranslationB) * Mat44::sRotationX(inRotationB.GetX()) * Mat44::sRotationY(inRotationB.GetY()) * Mat44::sRotationZ(inRotationB.GetZ());
	TransformedConvexObject<AABox> b(mat_b, inB);

	AABox widened_b = inB;
	widened_b.ExpandBy(Vec3::sReplicate(inConvexRadiusB));

	EPAPenetrationDepth pen_depth;
	Vec3 v = Vec3::sAxisX(), pa, pb;

	DrawSphereSP(mDebugRenderer, inTranslationA, inRadiusA, Color::sWhite);

	AddConvexRadius<TransformedConvexObject<AABox>> b_inc(b, inConvexRadiusB);

	if (pen_depth.GetPenetrationDepth(s, s, 0.0f, b, b_inc, inConvexRadiusB, 1.0e-4f, FLT_EPSILON, v, pa, pb))
	{
		DrawBoxSP(mDebugRenderer, mat_b, inB, Color::sRed);
		if (inConvexRadiusB > 0.0f)
			DrawWireBoxSP(mDebugRenderer, mat_b, widened_b, Color::sRed);
		DrawMarkerSP(mDebugRenderer, pa, Color::sYellow, 2.0f);
		DrawMarkerSP(mDebugRenderer, pb, Color::sCyan, 2.0f);
	}
	else
	{
		DrawBoxSP(mDebugRenderer, mat_b, inB, Color::sGreen);
		if (inConvexRadiusB > 0.0f)
			DrawWireBoxSP(mDebugRenderer, mat_b, widened_b, Color::sGreen);
	}
	DrawArrowSP(mDebugRenderer, inTranslationB + Vec3(0, 2, 0), inTranslationB + v + Vec3(0, 2, 0), Color::sOrange, 0.05f);
}

void InteractivePairsTest::TestSphereVsSphere(Vec3Arg inTranslationA, float inRadiusA, Vec3Arg inTranslationB, float inRadiusB, bool inTreatSphereAsPointWithConvexRadius)
{
	Sphere s1(inTranslationA, inRadiusA);
	Sphere s2(inTranslationB, inRadiusB);

	if (inTreatSphereAsPointWithConvexRadius)
		DrawWireSphereSP(mDebugRenderer, s1.GetCenter(), s1.GetRadius(), Color::sWhite);
	else
		DrawSphereSP(mDebugRenderer, s1.GetCenter(), s1.GetRadius(), Color::sWhite);

	bool intersects;
	EPAPenetrationDepth pen_depth;
	Vec3 v = Vec3::sAxisX(), pa, pb;
	if (inTreatSphereAsPointWithConvexRadius)
		intersects = pen_depth.GetPenetrationDepth(PointConvexSupport { inTranslationA }, s1, inRadiusA, PointConvexSupport { inTranslationB }, s2, inRadiusB, 1.0e-4f, FLT_EPSILON, v, pa, pb);
	else
		intersects = pen_depth.GetPenetrationDepth(s1, s1, 0.0f, s2, s2, 0.0f, 1.0e-4f, FLT_EPSILON, v, pa, pb);

	if (intersects)
	{
		if (inTreatSphereAsPointWithConvexRadius)
			DrawWireSphereSP(mDebugRenderer, s2.GetCenter(), s2.GetRadius(), Color::sRed);
		else
			DrawSphereSP(mDebugRenderer, s2.GetCenter(), s2.GetRadius(), Color::sRed);
		DrawMarkerSP(mDebugRenderer, pa, Color::sYellow, 2.0f);
		DrawMarkerSP(mDebugRenderer, pb, Color::sCyan, 2.0f);
	}
	else
	{
		if (inTreatSphereAsPointWithConvexRadius)
			DrawWireSphereSP(mDebugRenderer, s2.GetCenter(), s2.GetRadius(), Color::sGreen);
		else
			DrawSphereSP(mDebugRenderer, s2.GetCenter(), s2.GetRadius(), Color::sGreen);
	}
	DrawArrowSP(mDebugRenderer, inTranslationB + Vec3(0, 2, 0), inTranslationB + v + Vec3(0, 2, 0), Color::sOrange, 0.05f);
}
