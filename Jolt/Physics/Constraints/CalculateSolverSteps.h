// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/PhysicsSettings.h>

JPH_NAMESPACE_BEGIN

/// @cond INTERNAL
/// Internal class used to calculate the total number of solver sub steps
class CalculateSolverSteps
{
public:
	/// Constructor
	JPH_INLINE explicit			CalculateSolverSteps(const PhysicsSettings &inSettings) : mSettings(inSettings) { }

	/// Combine the number of sub steps for this body/constraint with the current values
	template <class Type>
	JPH_INLINE void				operator () (const Type *inObject)
	{
		uint num_sub_steps = inObject->GetNumSolverSubStepsOverride();
		mNumSolverSubSteps = max(mNumSolverSubSteps, num_sub_steps);
		mApplyDefaultVelocity |= num_sub_steps == 0;
	}

	/// Must be called after all bodies/constraints have been processed
	JPH_INLINE void				Finalize()
	{
		// If we have a default sub step count, take the max of the default and the overrides
		if (mApplyDefaultVelocity)
			mNumSolverSubSteps = max(mNumSolverSubSteps, mSettings.mNumSolverSubSteps);
		else if (mNumSolverSubSteps == 0)
			mNumSolverSubSteps = 1; // No bodies or constraints were added, this means there are only kinematic bodies so we don't need sub steps
	}

	/// Get the results of the calculation
	JPH_INLINE uint				GetNumSolverSubSteps() const					{ return mNumSolverSubSteps; }

private:
	const PhysicsSettings &		mSettings;

	uint						mNumSolverSubSteps = 0;

	bool						mApplyDefaultVelocity = false;
};
/// @endcond

JPH_NAMESPACE_END
