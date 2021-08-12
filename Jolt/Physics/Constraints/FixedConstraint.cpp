// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Constraints/FixedConstraint.h>
#include <Physics/Body/Body.h>
#include <ObjectStream/TypeDeclarations.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(FixedConstraintSettings)
{
	JPH_ADD_BASE_CLASS(FixedConstraintSettings, TwoBodyConstraintSettings)
}

TwoBodyConstraint *FixedConstraintSettings::Create(Body &inBody1, Body &inBody2) const
{
	return new FixedConstraint(inBody1, inBody2, *this);
}

FixedConstraint::FixedConstraint(Body &inBody1, Body &inBody2, const FixedConstraintSettings &inSettings) :
	TwoBodyConstraint(inBody1, inBody2, inSettings)
{	
	// Determine anchor point: If any of the bodies can never be dynamic use the other body as anchor point, otherwise use the mid point between the two center of masses
	Vec3 anchor;
	if (!mBody1->CanBeKinematicOrDynamic())
		anchor = mBody2->GetCenterOfMassPosition();
	else if (!mBody2->CanBeKinematicOrDynamic())
		anchor = mBody1->GetCenterOfMassPosition();
	else
		anchor = 0.5f * (mBody1->GetCenterOfMassPosition() + mBody2->GetCenterOfMassPosition());

	// Store local positions
	mLocalSpacePosition1 = inBody1.GetInverseCenterOfMassTransform() * anchor;
	mLocalSpacePosition2 = inBody2.GetInverseCenterOfMassTransform() * anchor;

	// Inverse of initial rotation from body 1 to body 2 in body 1 space
	mInvInitialOrientation = RotationEulerConstraintPart::sGetInvInitialOrientation(inBody1, inBody2);
}

void FixedConstraint::SetupVelocityConstraint(float inDeltaTime)
{
	// Calculate constraint values that don't change when the bodies don't change position
	Mat44 rotation1 = Mat44::sRotation(mBody1->GetRotation());
	Mat44 rotation2 = Mat44::sRotation(mBody2->GetRotation());
	mRotationConstraintPart.CalculateConstraintProperties(*mBody1, rotation1, *mBody2, rotation2);
	mPointConstraintPart.CalculateConstraintProperties(*mBody1, rotation1, mLocalSpacePosition1, *mBody2, rotation2, mLocalSpacePosition2);
}

void FixedConstraint::WarmStartVelocityConstraint(float inWarmStartImpulseRatio)
{
	// Warm starting: Apply previous frame impulse
	mRotationConstraintPart.WarmStart(*mBody1, *mBody2, inWarmStartImpulseRatio);
	mPointConstraintPart.WarmStart(*mBody1, *mBody2, inWarmStartImpulseRatio);
}

bool FixedConstraint::SolveVelocityConstraint(float inDeltaTime)
{
	// Solve rotation constraint
	bool rot = mRotationConstraintPart.SolveVelocityConstraint(*mBody1, *mBody2);

	// Solve position constraint
	bool pos = mPointConstraintPart.SolveVelocityConstraint(*mBody1, *mBody2);

	return rot || pos;
}

bool FixedConstraint::SolvePositionConstraint(float inDeltaTime, float inBaumgarte)
{
	// Solve rotation constraint
	mRotationConstraintPart.CalculateConstraintProperties(*mBody1, Mat44::sRotation(mBody1->GetRotation()), *mBody2, Mat44::sRotation(mBody2->GetRotation()));
	bool rot = mRotationConstraintPart.SolvePositionConstraint(*mBody1, *mBody2, mInvInitialOrientation, inBaumgarte);
	
	// Solve position constraint
	mPointConstraintPart.CalculateConstraintProperties(*mBody1, Mat44::sRotation(mBody1->GetRotation()), mLocalSpacePosition1, *mBody2, Mat44::sRotation(mBody2->GetRotation()), mLocalSpacePosition2);
	bool pos = mPointConstraintPart.SolvePositionConstraint(*mBody1, *mBody2, inBaumgarte);
	
	return rot || pos;
}

#ifdef JPH_DEBUG_RENDERER
void FixedConstraint::DrawConstraint(DebugRenderer *inRenderer) const
{
	Vec3 com1 = mBody1->GetCenterOfMassPosition();
	Vec3 com2 = mBody2->GetCenterOfMassPosition();

	// Draw constraint
	inRenderer->DrawLine(com1, com2, Color::sGreen);
}
#endif // JPH_DEBUG_RENDERER

void FixedConstraint::SaveState(StateRecorder &inStream) const
{
	TwoBodyConstraint::SaveState(inStream);

	mRotationConstraintPart.SaveState(inStream);
	mPointConstraintPart.SaveState(inStream);
}

void FixedConstraint::RestoreState(StateRecorder &inStream)
{
	TwoBodyConstraint::RestoreState(inStream);

	mRotationConstraintPart.RestoreState(inStream);
	mPointConstraintPart.RestoreState(inStream);
}

} // JPH