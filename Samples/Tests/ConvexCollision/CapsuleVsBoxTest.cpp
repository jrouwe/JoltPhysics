// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ConvexCollision/CapsuleVsBoxTest.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Utils/DebugRendererSP.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CapsuleVsBoxTest)
{
	JPH_ADD_BASE_CLASS(CapsuleVsBoxTest, Test)
}

void CapsuleVsBoxTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Create box
	Vec3 box_min(-1.0f, -2.0f, 0.5f);
	Vec3 box_max(2.0f, -0.5f, 3.0f);
	Ref<RotatedTranslatedShapeSettings> box_settings = new RotatedTranslatedShapeSettings(0.5f * (box_min + box_max), Quat::sIdentity(), new BoxShapeSettings(0.5f * (box_max - box_min)));
	Ref<Shape> box_shape = box_settings->Create().Get();
	Mat44 box_transform(Vec4(0.516170502f, -0.803887904f, -0.295520246f, 0.0f), Vec4(0.815010250f, 0.354940295f, 0.458012700f, 0.0f), Vec4(-0.263298869f, -0.477264702f, 0.838386655f, 0.0f), Vec4(-10.2214508f, -18.6808319f, 40.7468987f, 1.0f));

	// Create capsule
	float capsule_half_height = 75.0f;
	float capsule_radius = 1.5f;
	Quat capsule_compound_rotation(0.499999970f, -0.499999970f, -0.499999970f, 0.499999970f);
	Ref<RotatedTranslatedShapeSettings> capsule_settings = new RotatedTranslatedShapeSettings(Vec3(0, 0, 75), capsule_compound_rotation, new CapsuleShapeSettings(capsule_half_height, capsule_radius));
	Ref<Shape> capsule_shape = capsule_settings->Create().Get();
	Mat44 capsule_transform = Mat44::sTranslation(Vec3(-9.68538570f, -18.0328083f, 41.3212280f));

	// Collision settings
	CollideShapeSettings settings;
	settings.mActiveEdgeMode = EActiveEdgeMode::CollideWithAll;
	settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;
	settings.mCollectFacesMode = ECollectFacesMode::NoFaces;

	// Collide the two shapes
	AllHitCollisionCollector<CollideShapeCollector> collector;
	CollisionDispatch::sCollideShapeVsShape(capsule_shape, box_shape, Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), capsule_transform, box_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);

#ifdef JPH_DEBUG_RENDERER
	// Draw the shapes
	box_shape->Draw(mDebugRenderer, RMat44(box_transform), Vec3::sReplicate(1.0f), Color::sWhite, false, false);
	capsule_shape->Draw(mDebugRenderer, RMat44(capsule_transform), Vec3::sReplicate(1.0f), Color::sWhite, false, false);
#endif // JPH_DEBUG_RENDERER

	// Draw contact points
	const CollideShapeResult &hit = collector.mHits[0];
	DrawMarkerSP(mDebugRenderer, hit.mContactPointOn1, Color::sRed, 1.0f);
	DrawMarkerSP(mDebugRenderer, hit.mContactPointOn2, Color::sGreen, 1.0f);

	// Draw penetration axis with length of the penetration
	Vec3 pen_axis = hit.mPenetrationAxis;
	float pen_axis_len = pen_axis.Length();
	if (pen_axis_len > 0.0f)
	{
		pen_axis *= hit.mPenetrationDepth / pen_axis_len;
		DrawArrowSP(mDebugRenderer, hit.mContactPointOn2, hit.mContactPointOn2 + pen_axis, Color::sYellow, 0.01f);

#ifdef JPH_DEBUG_RENDERER
		Mat44 resolved_box = box_transform.PostTranslated(pen_axis);
		box_shape->Draw(mDebugRenderer, RMat44(resolved_box), Vec3::sReplicate(1.0f), Color::sGreen, false, false);
#endif // JPH_DEBUG_RENDERER
	}
}
