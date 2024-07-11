#include <TestFramework.h>

#include <Tests/General/ShapeFilterTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ShapeFilterTest)
{
	JPH_ADD_BASE_CLASS(ShapeFilterTest, Test)
}

void ShapeFilterTest::Initialize()
{
	// Create geometry to cast against
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(20, 1, 3)), RVec3(0, -1, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::Activate);
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(3)), RVec3(0, 3, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::Activate);

	// Create shape to cast
	Ref box_shape = new BoxShapeSettings(Vec3::sReplicate(1));
	box_shape->mUserData = (uint64)ShapeIdentifier::Box;

	Ref sphere_shape = new SphereShapeSettings(1);
	sphere_shape->mUserData = (uint64)ShapeIdentifier::Sphere;

	StaticCompoundShapeSettings cast_shape;
	cast_shape.AddShape(Vec3(3, 2, 0), Quat::sIdentity(), box_shape);
	cast_shape.AddShape(Vec3(0, 0, 0), Quat::sIdentity(), sphere_shape);
	cast_shape.mUserData = (uint64)ShapeIdentifier::Compound;

	mCastShape = cast_shape.Create().Get();
}

void ShapeFilterTest::PostPhysicsUpdate(float inDeltaTime)
{
	mElapsedTime += inDeltaTime;
	float phase = mElapsedTime;

	const RVec3 cast_origin = RVec3(Cos(phase) * 10, 10, 0);
	const Vec3 cast_motion = Vec3(0, -15, 0);

	ClosestHitCollisionCollector<CastShapeCollector> cast_shape_collector;

	class MyShapeFilter : public ShapeFilter
	{
	public:
		// Not used in this example
		virtual bool	ShouldCollide(const Shape *inShape2, const SubShapeID &inSubShapeIDOfShape2) const override
		{
			return true;
		}

		virtual bool	ShouldCollide(const Shape *inShape1, const SubShapeID &inSubShapeID1, const Shape *inShape2, const SubShapeID &inSubShapeID2) const override
		{
			return inShape1->GetUserData() != mUserDataOfShapeToIgnore;
		}

		uint64			mUserDataOfShapeToIgnore = (uint64)ShapeIdentifier::Sphere;
	};

	MyShapeFilter shape_filter;

	// Select which shape to ignore
	float shape_select = fmod(phase, 6.0f * JPH_PI);
	const char *text;
	if (shape_select < 2.0f * JPH_PI)
	{
		shape_filter.mUserDataOfShapeToIgnore = (uint64)ShapeIdentifier::Box;
		text = "Box";
	}
	else if (shape_select < 4.0f * JPH_PI)
	{
		shape_filter.mUserDataOfShapeToIgnore = (uint64)ShapeIdentifier::Sphere;
		text = "Sphere";
	}
	else
	{
		shape_filter.mUserDataOfShapeToIgnore = (uint64)ShapeIdentifier::Compound;
		text = "Compound";
	}
	mDebugRenderer->DrawText3D(cast_origin, StringFormat("Ignoring shape: %s", text), Color::sWhite);

	// Do the cast
	mPhysicsSystem->GetNarrowPhaseQuery().CastShape(
		RShapeCast(mCastShape, Vec3::sReplicate(1), RMat44::sTranslation(cast_origin), cast_motion),
		ShapeCastSettings(),
		RVec3::sZero(),
		cast_shape_collector,
		{ },
		{ },
		{ },
		shape_filter
	);

	// Show the result
	RVec3 cast_point;
	Color color;
	if (cast_shape_collector.HadHit())
	{
		cast_point = cast_origin + cast_motion * cast_shape_collector.mHit.mFraction;
		color = Color::sGreen;
	}
	else
	{
		cast_point = cast_origin + cast_motion;
		color = Color::sRed;
	}
	mDebugRenderer->DrawArrow(cast_origin, cast_point, Color::sOrange, 0.1f);
	JPH_IF_DEBUG_RENDERER(mCastShape->Draw(mDebugRenderer, RMat44::sTranslation(RVec3(cast_point)), Vec3::sReplicate(1.0f), color, false, true);)
}
