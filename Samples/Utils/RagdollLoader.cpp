// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Utils/RagdollLoader.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>
#include <Jolt/Physics/Constraints/ConeConstraint.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>
#include <Jolt/ObjectStream/ObjectStreamOut.h>
#include <Layers.h>
#include <Utils/Log.h>

RagdollSettings *RagdollLoader::sLoad(const char *inFileName, EMotionType inMotionType, EConstraintOverride inConstraintOverride)
{
	// Read the ragdoll
	RagdollSettings *ragdoll = nullptr;
	if (!ObjectStreamIn::sReadObject(inFileName, ragdoll))
		FatalError("Unable to read ragdoll");

	for (RagdollSettings::Part &p : ragdoll->mParts)
	{
		// Update motion type
		p.mMotionType = inMotionType;

		// Override layer
		p.mObjectLayer = Layers::MOVING;

		// Create new constraint
		Ref<SwingTwistConstraintSettings> original = DynamicCast<SwingTwistConstraintSettings>(p.mToParent);
		if (original != nullptr)
			switch (inConstraintOverride)
			{
			case EConstraintOverride::TypeFixed:
				{
					FixedConstraintSettings *settings = new FixedConstraintSettings();
					settings->mPoint1 = settings->mPoint2 = original->mPosition1;
					p.mToParent = settings;
					break;
				}

			case EConstraintOverride::TypePoint:
				{
					PointConstraintSettings *settings = new PointConstraintSettings();
					settings->mPoint1 = settings->mPoint2 = original->mPosition1;
					p.mToParent = settings;
					break;
				}

			case EConstraintOverride::TypeHinge:
				{
					HingeConstraintSettings *settings = new HingeConstraintSettings();
					settings->mPoint1 = original->mPosition1;
					settings->mHingeAxis1 = original->mPlaneAxis1;
					settings->mNormalAxis1 = original->mTwistAxis1;
					settings->mPoint2 = original->mPosition2;
					settings->mHingeAxis2 = original->mPlaneAxis2;
					settings->mNormalAxis2 = original->mTwistAxis2;
					settings->mLimitsMin = -original->mNormalHalfConeAngle;
					settings->mLimitsMax = original->mNormalHalfConeAngle;
					settings->mMaxFrictionTorque = original->mMaxFrictionTorque;
					p.mToParent = settings;
					break;
				}

			case EConstraintOverride::TypeSlider:
				{
					SliderConstraintSettings *settings = new SliderConstraintSettings();
					settings->mPoint1 = settings->mPoint2 = original->mPosition1;
					settings->mSliderAxis1 = settings->mSliderAxis2 = original->mTwistAxis1;
					settings->mNormalAxis1 = settings->mNormalAxis2 = original->mTwistAxis1.GetNormalizedPerpendicular();
					settings->mLimitsMin = -1.0f;
					settings->mLimitsMax = 1.0f;
					settings->mMaxFrictionForce = original->mMaxFrictionTorque;
					p.mToParent = settings;
					break;
				}

			case EConstraintOverride::TypeCone:
				{
					ConeConstraintSettings *settings = new ConeConstraintSettings();
					settings->mPoint1 = original->mPosition1;
					settings->mTwistAxis1 = original->mTwistAxis1;
					settings->mPoint2 = original->mPosition2;
					settings->mTwistAxis2 = original->mTwistAxis2;
					settings->mHalfConeAngle = original->mNormalHalfConeAngle;
					p.mToParent = settings;
					break;
				}

			case EConstraintOverride::TypeRagdoll:
				break;
			}
		}

	// Initialize the skeleton
	ragdoll->GetSkeleton()->CalculateParentJointIndices();

	// Stabilize the constraints of the ragdoll
	ragdoll->Stabilize();

	// Calculate body <-> constraint map
	ragdoll->CalculateBodyIndexToConstraintIndex();
	ragdoll->CalculateConstraintIndexToBodyIdxPair();

	return ragdoll;
}
