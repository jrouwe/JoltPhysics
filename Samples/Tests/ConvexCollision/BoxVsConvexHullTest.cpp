#include <TestFramework.h>

#include <Tests/ConvexCollision/BoxVsConvexHullTest.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Utils/DebugRendererSP.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(BoxVsConvexHullTest)
{
	JPH_ADD_BASE_CLASS(BoxVsConvexHullTest, Test)
}

static const JPH::Vec3 hull_points[] = {
	{-2.5, -0.25, -1.5},
	{-2.5, 0.25, -1.5},
	{2.5, -0.25, -1.5},
	{-2.5, -0.25, 1.5},
	{-2.5, 0.25, 1.5},
	{2.5, 0.25, -1.5},
	{2.5, -0.25, 1.5},
	{2.5, 0.25, 1.5}
};

void BoxVsConvexHullTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	mAngle += DegreesToRadians(45.0f) * inParams.mDeltaTime;

	Ref<BoxShapeSettings> box_settings = new BoxShapeSettings(Vec3(0.25f, 0.75f, 0.375f), 0.0f);
	Ref<Shape> box_shape = box_settings->Create().Get();

	Ref<ConvexHullShapeSettings> hull_settings = new ConvexHullShapeSettings(hull_points, 8, 0.0f);
	Ref<Shape> hull_shape = hull_settings->Create().Get();

	const Mat44 hull_transform = Mat44::sRotationY(mAngle);

	// Setting this to <= 0.0004 or >= 0.0006 will fail to reproduce the issue with this particular setup
	constexpr float box_separation_from_hull = 0.0005f;

	const Mat44 box_local_translation = Mat44::sTranslation({0.1f, 1.0f + box_separation_from_hull, -0.5f});
	const Mat44 box_local_rotation = Mat44::sRotationY(DegreesToRadians(-45));
	const Mat44 box_local_transform = box_local_translation * box_local_rotation;
	const Mat44 box_transform = hull_transform * box_local_transform;

	CollideShapeSettings settings;
	settings.mMaxSeparationDistance = 0.001f;

	ClosestHitCollisionCollector<CollideShapeCollector> collector;
	CollisionDispatch::sCollideShapeVsShape(box_shape, hull_shape, Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), box_transform, hull_transform, SubShapeIDCreator(), SubShapeIDCreator(), settings, collector);

	const CollideShapeResult &hit = collector.mHit;

#ifdef JPH_DEBUG_RENDERER
	box_shape->Draw(mDebugRenderer, RMat44(box_transform), Vec3::sReplicate(1.0f), Color::sWhite, false, false);
	hull_shape->Draw(mDebugRenderer, RMat44(hull_transform), Vec3::sReplicate(1.0f), Color::sWhite, false, false);
#endif // JPH_DEBUG_RENDERER

	const Vec3 normal = -hit.mPenetrationAxis.NormalizedOr(Vec3::sZero());
	if (normal.LengthSq() > 0.0f)
	{
		const bool correct_normal = normal.GetY() > 0.0;
		if (!correct_normal)
			Trace("Detected flipped normal");

		DrawArrowSP(mDebugRenderer, hit.mContactPointOn2, hit.mContactPointOn2 + normal * 4.0f, correct_normal ? Color::sGreen : Color::sRed, 0.1f);
	}
}
