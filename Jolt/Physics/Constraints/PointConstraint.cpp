// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Constraints/PointConstraint.h>
#include <Physics/Body/Body.h>
#include <ObjectStream/TypeDeclarations.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(PointConstraintSettings)
{
	JPH_ADD_BASE_CLASS(PointConstraintSettings, TwoBodyConstraintSettings)

	JPH_ADD_ATTRIBUTE(PointConstraintSettings, mCommonPoint)
}

void PointConstraintSettings::SaveBinaryState(StreamOut &inStream) const
{ 
	ConstraintSettings::SaveBinaryState(inStream);

	inStream.Write(mCommonPoint);	
}

void PointConstraintSettings::RestoreBinaryState(StreamIn &inStream)
{
	ConstraintSettings::RestoreBinaryState(inStream);

	inStream.Read(mCommonPoint);
}

TwoBodyConstraint *PointConstraintSettings::Create(Body &inBody1, Body &inBody2) const
{
	return new PointConstraint(inBody1, inBody2, *this);
}

PointConstraint::PointConstraint(Body &inBody1, Body &inBody2, const PointConstraintSettings &inSettings) :
	TwoBodyConstraint(inBody1, inBody2, inSettings)
{
	// Store local positions
	mLocalSpacePosition1 = inBody1.GetInverseCenterOfMassTransform() * inSettings.mCommonPoint;
	mLocalSpacePosition2 = inBody2.GetInverseCenterOfMassTransform() * inSettings.mCommonPoint;
}

void PointConstraint::CalculateConstraintProperties()
{	
	mPointConstraintPart.CalculateConstraintProperties(*mBody1, Mat44::sRotation(mBody1->GetRotation()), mLocalSpacePosition1, *mBody2, Mat44::sRotation(mBody2->GetRotation()), mLocalSpacePosition2);
}

void PointConstraint::SetupVelocityConstraint(float inDeltaTime)
{
	CalculateConstraintProperties();
}

void PointConstraint::WarmStartVelocityConstraint(float inWarmStartImpulseRatio)
{
	// Warm starting: Apply previous frame impulse
	mPointConstraintPart.WarmStart(*mBody1, *mBody2, inWarmStartImpulseRatio);
}

bool PointConstraint::SolveVelocityConstraint(float inDeltaTime)
{
	return mPointConstraintPart.SolveVelocityConstraint(*mBody1, *mBody2);
}

bool PointConstraint::SolvePositionConstraint(float inDeltaTime, float inBaumgarte)
{
	// Update constraint properties (bodies may have moved)
	CalculateConstraintProperties();

	return mPointConstraintPart.SolvePositionConstraint(*mBody1, *mBody2, inBaumgarte);
}

#ifdef JPH_DEBUG_RENDERER
void PointConstraint::DrawConstraint(DebugRenderer *inRenderer) const
{
	// Draw constraint
	inRenderer->DrawMarker(mBody1->GetCenterOfMassTransform() * mLocalSpacePosition1, Color::sRed, 0.1f);
	inRenderer->DrawMarker(mBody2->GetCenterOfMassTransform() * mLocalSpacePosition2, Color::sGreen, 0.1f);
}
#endif // JPH_DEBUG_RENDERER

void PointConstraint::SaveState(StateRecorder &inStream) const
{
	TwoBodyConstraint::SaveState(inStream);

	mPointConstraintPart.SaveState(inStream);
}

void PointConstraint::RestoreState(StateRecorder &inStream)
{
	TwoBodyConstraint::RestoreState(inStream);

	mPointConstraintPart.RestoreState(inStream);
}

} // JPH