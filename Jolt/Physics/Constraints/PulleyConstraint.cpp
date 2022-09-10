// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Constraints/PulleyConstraint.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(PulleyConstraintSettings)
{
	JPH_ADD_BASE_CLASS(PulleyConstraintSettings, TwoBodyConstraintSettings)

	JPH_ADD_ENUM_ATTRIBUTE(PulleyConstraintSettings, mSpace)
	JPH_ADD_ATTRIBUTE(PulleyConstraintSettings, mBodyPoint1)
	JPH_ADD_ATTRIBUTE(PulleyConstraintSettings, mFixedPoint1)
	JPH_ADD_ATTRIBUTE(PulleyConstraintSettings, mBodyPoint2)
	JPH_ADD_ATTRIBUTE(PulleyConstraintSettings, mFixedPoint2)
	JPH_ADD_ATTRIBUTE(PulleyConstraintSettings, mRatio)
	JPH_ADD_ATTRIBUTE(PulleyConstraintSettings, mMinLength)
	JPH_ADD_ATTRIBUTE(PulleyConstraintSettings, mMaxLength)
}

void PulleyConstraintSettings::SaveBinaryState(StreamOut &inStream) const
{ 
	ConstraintSettings::SaveBinaryState(inStream);

	inStream.Write(mSpace);
	inStream.Write(mBodyPoint1);
	inStream.Write(mFixedPoint1);
	inStream.Write(mBodyPoint2);
	inStream.Write(mFixedPoint2);
	inStream.Write(mRatio);
	inStream.Write(mMinLength);
	inStream.Write(mMaxLength);
}

void PulleyConstraintSettings::RestoreBinaryState(StreamIn &inStream)
{
	ConstraintSettings::RestoreBinaryState(inStream);

	inStream.Read(mSpace);
	inStream.Read(mBodyPoint1);
	inStream.Read(mFixedPoint1);
	inStream.Read(mBodyPoint2);
	inStream.Read(mFixedPoint2);
	inStream.Read(mRatio);
	inStream.Read(mMinLength);
	inStream.Read(mMaxLength);
}

TwoBodyConstraint *PulleyConstraintSettings::Create(Body &inBody1, Body &inBody2) const
{
	return new PulleyConstraint(inBody1, inBody2, *this);
}

PulleyConstraint::PulleyConstraint(Body &inBody1, Body &inBody2, const PulleyConstraintSettings &inSettings) :
	TwoBodyConstraint(inBody1, inBody2, inSettings),
	mLocalSpacePosition1(inSettings.mBodyPoint1),
	mLocalSpacePosition2(inSettings.mBodyPoint2),
	mFixedPosition1(inSettings.mFixedPoint1),
	mFixedPosition2(inSettings.mFixedPoint2),
	mRatio(inSettings.mRatio),
	mMinLength(inSettings.mMinLength),
	mMaxLength(inSettings.mMaxLength),
	mWorldSpacePosition1(inSettings.mBodyPoint1),
	mWorldSpacePosition2(inSettings.mBodyPoint2)
{
	if (inSettings.mSpace == EConstraintSpace::WorldSpace)
	{
		// If all properties were specified in world space, take them to local space now
		mLocalSpacePosition1 = inBody1.GetInverseCenterOfMassTransform() * mLocalSpacePosition1;
		mLocalSpacePosition2 = inBody2.GetInverseCenterOfMassTransform() * mLocalSpacePosition2;
	}
	else
	{
		// If properties were specified in local space, we need to calculate world space positions
		mWorldSpacePosition1 = inBody1.GetCenterOfMassTransform() * mWorldSpacePosition1;
		mWorldSpacePosition2 = inBody2.GetCenterOfMassTransform() * mWorldSpacePosition2;
	}

	// Calculate max length if it was not provided
	if (mMaxLength < 0.0f)
		mMaxLength = GetCurrentLength();

	// Most likely gravity is going to tear us apart (this is only used when the distance between the points = 0)
	mWorldSpaceNormal1 = mWorldSpaceNormal2 = Vec3::sAxisY(); 
}

void PulleyConstraint::CalculateConstraintProperties(float inDeltaTime)
{
	// Update world space positions (the bodies may have moved)
	mWorldSpacePosition1 = mBody1->GetCenterOfMassTransform() * mLocalSpacePosition1;
	mWorldSpacePosition2 = mBody2->GetCenterOfMassTransform() * mLocalSpacePosition2;

	/*
	// Calculate current length
	float length = (mWorldSpacePosition2 - mFixedPosition2).Length() + mRatio * (mWorldSpacePosition2 - mFixedPosition2).Length();

	
	// Calculate points relative to body
	// r1 + u = (p1 - x1) + (p2 - p1) = p2 - x1
	Vec3 r1_plus_u = mWorldSpacePosition2 - mBody1->GetCenterOfMassPosition();
	Vec3 r2 = mWorldSpacePosition2 - mBody2->GetCenterOfMassPosition();

	if (mMinDistance == mMaxDistance)
	{
		mAxisConstraint.CalculateConstraintProperties(inDeltaTime, *mBody1, r1_plus_u, *mBody2, r2, mWorldSpaceNormal, 0.0f, delta_len - mMinDistance, mFrequency, mDamping);

		// Single distance, allow constraint forces in both directions
		mMinLambda = -FLT_MAX;
		mMaxLambda = FLT_MAX;
	}
	if (delta_len <= mMinDistance)
	{
		mAxisConstraint.CalculateConstraintProperties(inDeltaTime, *mBody1, r1_plus_u, *mBody2, r2, mWorldSpaceNormal, 0.0f, delta_len - mMinDistance, mFrequency, mDamping);

		// Allow constraint forces to make distance bigger only
		mMinLambda = 0;
		mMaxLambda = FLT_MAX;
	}
	else if (delta_len >= mMaxDistance)
	{
		mAxisConstraint.CalculateConstraintProperties(inDeltaTime, *mBody1, r1_plus_u, *mBody2, r2, mWorldSpaceNormal, 0.0f, delta_len - mMaxDistance, mFrequency, mDamping);

		// Allow constraint forces to make distance smaller only
		mMinLambda = -FLT_MAX;
		mMaxLambda = 0;
	}
	else
		mAxisConstraint.Deactivate();
	*/
}

void PulleyConstraint::SetupVelocityConstraint(float inDeltaTime)
{
	CalculateConstraintProperties(inDeltaTime);
}

void PulleyConstraint::WarmStartVelocityConstraint(float inWarmStartImpulseRatio)
{
	//mAxisConstraint.WarmStart(*mBody1, *mBody2, mWorldSpaceNormal, inWarmStartImpulseRatio);
}

bool PulleyConstraint::SolveVelocityConstraint(float inDeltaTime)
{
	/*
	if (mAxisConstraint.IsActive())
		return mAxisConstraint.SolveVelocityConstraint(*mBody1, *mBody2, mWorldSpaceNormal, mMinLambda, mMaxLambda);
	else*/
		return false;
}

bool PulleyConstraint::SolvePositionConstraint(float inDeltaTime, float inBaumgarte)
{
	/*
	float distance = (mWorldSpacePosition2 - mWorldSpacePosition1).Dot(mWorldSpaceNormal);

	// Calculate position error
	float position_error = 0.0f;
	if (distance < mMinDistance)
		position_error = distance - mMinDistance;
	else if (distance > mMaxDistance)
		position_error = distance - mMaxDistance;

	if (position_error != 0.0f)
	{
		// Update constraint properties (bodies may have moved)
		CalculateConstraintProperties(inDeltaTime);

		return mAxisConstraint.SolvePositionConstraint(*mBody1, *mBody2, mWorldSpaceNormal, position_error, inBaumgarte);
	}
	*/

	return false;
}

#ifdef JPH_DEBUG_RENDERER
void PulleyConstraint::DrawConstraint(DebugRenderer *inRenderer) const
{
	// Draw constraint
	inRenderer->DrawLine(mWorldSpacePosition1, mFixedPosition1, Color::sGreen);
	inRenderer->DrawLine(mFixedPosition1, mFixedPosition2, Color::sGreen);
	inRenderer->DrawLine(mFixedPosition2, mWorldSpacePosition2, Color::sGreen);

	// Draw current length
	inRenderer->DrawText3D(0.5f * (mFixedPosition1 + mFixedPosition2), StringFormat("%.2f", (double)GetCurrentLength()));
}
#endif // JPH_DEBUG_RENDERER

void PulleyConstraint::SaveState(StateRecorder &inStream) const
{
	TwoBodyConstraint::SaveState(inStream);

	mAxisConstraint.SaveState(inStream);
	//inStream.Write(mWorldSpaceNormal); // When distance = 0, the normal is used from last frame so we need to store it
}

void PulleyConstraint::RestoreState(StateRecorder &inStream)
{
	TwoBodyConstraint::RestoreState(inStream);

	mAxisConstraint.RestoreState(inStream);
	//inStream.Read(mWorldSpaceNormal);
}

Ref<ConstraintSettings> PulleyConstraint::GetConstraintSettings() const
{
	PulleyConstraintSettings *settings = new PulleyConstraintSettings;
	ToConstraintSettings(*settings);
	settings->mSpace = EConstraintSpace::LocalToBodyCOM;
	settings->mBodyPoint1 = mLocalSpacePosition1;
	settings->mFixedPoint1 = mFixedPosition1;
	settings->mBodyPoint2 = mLocalSpacePosition2;
	settings->mFixedPoint2 = mFixedPosition2;
	settings->mRatio = mRatio;
	settings->mMinLength = mMinLength;
	settings->mMaxLength = mMaxLength;
	return settings;
}

JPH_NAMESPACE_END
