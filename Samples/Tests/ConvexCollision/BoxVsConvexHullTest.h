#pragma once

#include <Tests/Test.h>

class BoxVsConvexHullTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, BoxVsConvexHullTest)

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	float mAngle = 0.0f;
};
