// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(FixedConstraintSettings)
{
	JPH_ADD_BASE_CLASS(FixedConstraintSettings, TwoBodyConstraintSettings)

	JPH_ADD_ENUM_ATTRIBUTE(FixedConstraintSettings, mSpace)
	JPH_ADD_ATTRIBUTE(FixedConstraintSettings, mPoint1)
	JPH_ADD_ATTRIBUTE(FixedConstraintSettings, mAxisX1)
	JPH_ADD_ATTRIBUTE(FixedConstraintSettings, mAxisY1)
	JPH_ADD_ATTRIBUTE(FixedConstraintSettings, mPoint2)
	JPH_ADD_ATTRIBUTE(FixedConstraintSettings, mAxisX2)
	JPH_ADD_ATTRIBUTE(FixedConstraintSettings, mAxisY2)
}

void FixedConstraintSettings::SaveBinaryState(StreamOut &inStream) const
{ 
	ConstraintSettings::SaveBinaryState(inStream);

	inStream.Write(mSpace);
	inStream.Write(mPoint1);
	inStream.Write(mAxisX1);
	inStream.Write(mAxisY1);
	inStream.Write(mPoint2);
	inStream.Write(mAxisX2);
	inStream.Write(mAxisY2);
}

void FixedConstraintSettings::RestoreBinaryState(StreamIn &inStream)
{
	ConstraintSettings::RestoreBinaryState(inStream);

	inStream.Read(mSpace);
	inStream.Read(mPoint1);
	inStream.Read(mAxisX1);
	inStream.Read(mAxisY1);
	inStream.Read(mPoint2);
	inStream.Read(mAxisX2);
	inStream.Read(mAxisY2);
}

TwoBodyConstraint *FixedConstraintSettings::Create(Body &inBody1, Body &inBody2) const
{
	return new FixedConstraint(inBody1, inBody2, *this);
}

void FixedConstraintSettings::SetPoint(const Body &inBody1, const Body &inBody2)
{
	JPH_ASSERT(mSpace == EConstraintSpace::WorldSpace);

	// Determine anchor point: If any of the bodies can never be dynamic use the other body as anchor point
	Vec3 anchor;
	if (!inBody1.CanBeKinematicOrDynamic())
		anchor = inBody2.GetCenterOfMassPosition();
	else if (!inBody2.CanBeKinematicOrDynamic())
		anchor = inBody1.GetCenterOfMassPosition();
	else
	{
		// Otherwise use weighted anchor point towards the lightest body
		float inv_m1 = inBody1.GetMotionPropertiesUnchecked()->GetInverseMassUnchecked();
		float inv_m2 = inBody2.GetMotionPropertiesUnchecked()->GetInverseMassUnchecked();
		anchor = (inv_m1 * inBody1.GetCenterOfMassPosition() + inv_m2 * inBody2.GetCenterOfMassPosition()) / (inv_m1 + inv_m2);
	}

	mPoint1 = mPoint2 = anchor;
}

FixedConstraint::FixedConstraint(Body &inBody1, Body &inBody2, const FixedConstraintSettings &inSettings) :
	TwoBodyConstraint(inBody1, inBody2, inSettings),
	mLocalSpacePosition1(inSettings.mPoint1),
	mLocalSpacePosition2(inSettings.mPoint2)
{	
	// Store inverse of initial rotation from body 1 to body 2 in body 1 space
	mInvInitialOrientation = RotationEulerConstraintPart::sGetInvInitialOrientationXY(inSettings.mAxisX1, inSettings.mAxisY1, inSettings.mAxisX2, inSettings.mAxisY2);

	if (inSettings.mSpace == EConstraintSpace::WorldSpace)
	{
		// Store local positions
		mLocalSpacePosition1 = inBody1.GetInverseCenterOfMassTransform() * mLocalSpacePosition1;
		mLocalSpacePosition2 = inBody2.GetInverseCenterOfMassTransform() * mLocalSpacePosition2;

		// Constraints were specified in world space, so we should have replaced c1 with q10^-1 c1 and c2 with q20^-1 c2
		// => r0^-1 = (q20^-1 c2) (q10^-1 c1)^1 = q20^-1 (c2 c1^-1) q10
		mInvInitialOrientation = inBody2.GetRotation().Conjugated() * mInvInitialOrientation * inBody1.GetRotation();
	}
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

Ref<ConstraintSettings> FixedConstraint::GetConstraintSettings() const
{
	FixedConstraintSettings *settings = new FixedConstraintSettings;
	ToConstraintSettings(*settings);
	settings->mSpace = EConstraintSpace::LocalToBodyCOM;
	settings->mPoint1 = mLocalSpacePosition1;
	settings->mAxisX1 = Vec3::sAxisX();
	settings->mAxisY1 = Vec3::sAxisY();
	settings->mPoint2 = mLocalSpacePosition2;
	settings->mAxisX2 = mInvInitialOrientation.RotateAxisX();
	settings->mAxisY2 = mInvInitialOrientation.RotateAxisY();
	return settings;
}

JPH_NAMESPACE_END
