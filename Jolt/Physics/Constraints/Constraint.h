// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/Reference.h>
#include <Core/NonCopyable.h>
#include <Core/Result.h>
#include <ObjectStream/SerializableObject.h>

namespace JPH {

class IslandBuilder;
class BodyManager;
class StateRecorder;
class StreamIn;
class StreamOut;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

enum class EConstraintType
{
	Fixed,
	Point,
	Hinge,
	Slider,
	Distance,
	Cone,
	SwingTwist,
	SixDOF,
	Path,
	Vehicle,

	/// User defined constraint types start here
	User1,
	User2,
	User3,
	User4
};

/// Class used to store the configuration of a constraint. Allows run-time creation of constraints.
class ConstraintSettings : public SerializableObject, public RefTarget<ConstraintSettings>, public NonCopyable
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(ConstraintSettings)

	using ConstraintResult = Result<Ref<ConstraintSettings>>;

	/// Saves the contents of the constraint settings in binary form to inStream.
	virtual void				SaveBinaryState(StreamOut &inStream) const;

	/// Creates a constraint of the correct type and restores its contents from the binary stream inStream.
	static ConstraintResult		sRestoreFromBinaryState(StreamIn &inStream);

	/// Size of constraint when drawing it through the debug renderer
	float						mDrawConstraintSize = 1.0f;

protected:
	/// This function should not be called directly, it is used by sRestoreFromBinaryState.
	virtual void				RestoreBinaryState(StreamIn &inStream);
};

/// Base class for all physics constraints. A constraint removes one or more degrees of freedom for a rigid body.
class Constraint : public RefTarget<Constraint>
{
public:
	/// Constructor
	explicit					Constraint([[maybe_unused]] const ConstraintSettings &inSettings)
#ifdef JPH_DEBUG_RENDERER
									: mDrawConstraintSize(inSettings.mDrawConstraintSize)
#endif // JPH_DEBUG_RENDERER
	{
	}

	/// Virtual destructor
	virtual						~Constraint() = default;

	/// Get the type of a constraint
	virtual EConstraintType		GetType() const = 0;

	/// Enable / disable this constraint. This can e.g. be used to implement a breakable constraint by detecting that the constraint impulse went over a certain limit and then disabling the constraint. 
	/// Note that although a disabled constraint will not affect the simulation in any way anymore, it does incur some processing overhead. 
	/// Alternatively you can remove a constraint from the constraint manager (which may be more costly if you want to disable the constraint for a short while).
	void						SetEnabled(bool inEnabled)					{ mEnabled = inEnabled; }

	/// Test if a constraint is enabled.
	bool						GetEnabled() const							{ return mEnabled; }

	///@name Solver interface
	///@{
	virtual bool				IsActive() const							{ return mEnabled; }
	virtual void				SetupVelocityConstraint(float inDeltaTime) = 0;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) = 0;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) = 0;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) = 0;
	///@}

	/// Link bodies that are connected by this constraint in the island builder
	virtual void				BuildIslands(uint32 inConstraintIndex, IslandBuilder &ioBuilder, BodyManager &inBodyManager) = 0;

#ifdef JPH_DEBUG_RENDERER
	// Drawing interface
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const = 0;
	virtual void				DrawConstraintLimits(DebugRenderer *inRenderer) const { }
	virtual void				DrawConstraintReferenceFrame(DebugRenderer *inRenderer) const { }

	/// Size of constraint when drawing it through the debug renderer
	float						GetDrawConstraintSize() const				{ return mDrawConstraintSize; }
	void						SetDrawConstraintSize(float inSize)			{ mDrawConstraintSize = inSize; }
#endif // JPH_DEBUG_RENDERER

	/// Saving state for replay
	virtual void				SaveState(StateRecorder &inStream) const;

	/// Restoring state for replay
	virtual void				RestoreState(StateRecorder &inStream);

protected:
#ifdef JPH_DEBUG_RENDERER
	/// Size of constraint when drawing it through the debug renderer
	float						mDrawConstraintSize;
#endif // JPH_DEBUG_RENDERER

private:
	friend class ConstraintManager;

	/// Index that indicates this constraint is not in the constraint manager
	static constexpr uint32		cInvalidConstraintIndex = 0xffffffff;

	/// Index in the mConstraints list of the ConstraintManager for easy finding
	uint32						mConstraintIndex = cInvalidConstraintIndex;

	/// If this constraint is currently enabled
	bool						mEnabled = true;
};

} // JPH