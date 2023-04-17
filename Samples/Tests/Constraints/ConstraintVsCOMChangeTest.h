// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test demonstrates how to notify a constraint that the center of mass of a body changed (constraints store their attachment points in center of mass space).
class ConstraintVsCOMChangeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ConstraintVsCOMChangeTest)

	// See: Test
	virtual void				Initialize() override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams& inParams) override;
	virtual void				SaveState(StateRecorder& inStream) const override;
	virtual void				RestoreState(StateRecorder& inStream) override;

private:
	void						UpdateShapes();

	RefConst<Shape>				mBox;
	Array<Body *>				mBodies;
	Array<Ref<Constraint>>		mConstraints;

	static constexpr float		cBoxSize = 2.0f;

	float						mTime = 0.0f;
	int							mNumShapes = -1;
};
