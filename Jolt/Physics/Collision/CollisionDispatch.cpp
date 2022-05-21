// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/CastResult.h>

JPH_NAMESPACE_BEGIN

CollisionDispatch::CollideShape CollisionDispatch::sCollideShape[NumSubShapeTypes][NumSubShapeTypes];
CollisionDispatch::CastShape CollisionDispatch::sCastShape[NumSubShapeTypes][NumSubShapeTypes];

void CollisionDispatch::sInit()
{
	for (int i = 0; i < NumSubShapeTypes; ++i)
		for (int j = 0; j < NumSubShapeTypes; ++j)
		{
			if (sCollideShape[i][j] == nullptr)
				sCollideShape[i][j] = [](const Shape *, const Shape *, Vec3Arg, Vec3Arg, Mat44Arg, Mat44Arg, const SubShapeIDCreator &, const SubShapeIDCreator &, const CollideShapeSettings &, CollideShapeCollector &, const ShapeFilter &)
				{
					JPH_ASSERT(false, "Unsupported shape pair");
				};

			if (sCastShape[i][j] == nullptr)
				sCastShape[i][j] = [](const ShapeCast &, const ShapeCastSettings &, const Shape *, Vec3Arg, const ShapeFilter &, Mat44Arg, const SubShapeIDCreator &, const SubShapeIDCreator &, CastShapeCollector &)
				{
					JPH_ASSERT(false, "Unsupported shape pair");
				};
		}
}

void CollisionDispatch::sReversedCollideShape(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector, const ShapeFilter &inShapeFilter)
{
	// A collision collector that flips the collision results
	class ReversedCollector : public CollideShapeCollector
	{
	public:
								ReversedCollector(CollideShapeCollector &ioCollector) :
			mCollector(ioCollector)
		{
			SetContext(ioCollector.GetContext());
		}

		virtual void			AddHit(const CollideShapeResult &inResult) override
		{
			// Add the reversed hit
			mCollector.AddHit(inResult.Reversed());

			// If our chained collector updated its early out fraction, we need to follow
			UpdateEarlyOutFraction(mCollector.GetEarlyOutFraction());
		}

	private:
		CollideShapeCollector &	mCollector;
	};

	ReversedCollector collector(ioCollector);
	sCollideShapeVsShape(inShape2, inShape1, inScale2, inScale1, inCenterOfMassTransform2, inCenterOfMassTransform1, inSubShapeIDCreator2, inSubShapeIDCreator1, inCollideShapeSettings, collector, inShapeFilter);
}

void CollisionDispatch::sReversedCastShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector)
{
	// A collision collector that flips the collision results
	class ReversedCollector : public CastShapeCollector
	{
	public:
								ReversedCollector(CastShapeCollector &ioCollector, Vec3Arg inCastShapeWorldDirection) :
			mCollector(ioCollector),
			mCastShapeWorldDirection(inCastShapeWorldDirection)
		{
			SetContext(ioCollector.GetContext());
		}

		virtual void			AddHit(const ShapeCastResult &inResult) override
		{
			// Add the reversed hit
			mCollector.AddHit(inResult.Reversed(mCastShapeWorldDirection));

			// If our chained collector updated its early out fraction, we need to follow
			UpdateEarlyOutFraction(mCollector.GetEarlyOutFraction());
		}

	private:
		CastShapeCollector &	mCollector;
		Vec3					mCastShapeWorldDirection;
	};

	ShapeCast shape_cast_world(inShape, inScale, inCenterOfMassTransform2, -inCenterOfMassTransform2.Multiply3x3(inShapeCast.mDirection));
	ReversedCollector collector(ioCollector, shape_cast_world.mDirection);
	sCastShapeVsShapeWorldSpace(shape_cast_world, inShapeCastSettings, inShapeCast.mShape, inShapeCast.mScale, inShapeFilter, inCenterOfMassTransform2 * inShapeCast.mCenterOfMassStart, inSubShapeIDCreator2, inSubShapeIDCreator1, collector);
}

JPH_NAMESPACE_END
