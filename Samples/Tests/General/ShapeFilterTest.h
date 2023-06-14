#pragma once

#include <Tests/Test.h>

/// This test demonstrates how to use the ShapeFilter to filter out shapes during a collision query.
class ShapeFilterTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ShapeFilterTest)

	// See: Test
	virtual void	Initialize() override;
	virtual void	PostPhysicsUpdate(float inDeltaTime) override;

private:
	/// A value used as user data for a shape
	enum class ShapeIdentifier : uint64
	{
		Box = 42,
		Sphere = 43,
		Compound = 44
	};

	float			mElapsedTime = 0.0f;
	ShapeRefC		mCastShape;
};
