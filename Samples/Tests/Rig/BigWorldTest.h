// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

// This test tests the performance of a pile of ragdolls on a terrain at various distances from the origin.
class BigWorldTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, BigWorldTest)

	// Destructor
	virtual					~BigWorldTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	virtual void			Initialize() override;

	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

	virtual RMat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;
	virtual RVec3			GetDrawOffset() const override;

#ifndef JPH_DOUBLE_PRECISION
	virtual String			GetStatusString() const override							{ return "Define JPH_DOUBLE_PRECISION for an accurate simulation!"; }
#endif // JPH_DOUBLE_PRECISION

private:
	// If we want to draw the further scenes in wireframe
	inline static bool		sDrawWireframe = true;

	// A bitfield that determines which piles to draw
	inline static uint32	sDrawPileMask = ~uint32(0);

	// Pivot for the camera
	inline static RVec3		sPivot { RVec3::sZero() };

	// Bookkeeping for a pile
	struct Pile
	{
		// Distance label for this test
		String				GetLabel() const											{ return StringFormat("%.0f km", 1.0e-3 * double(mOrigin.Length())); }

		RVec3				mOrigin;													///< Origin for this pile
		Array<Ref<Ragdoll>>	mRagdolls;													///< Ragdolls in the pile
	};
	Array<Pile>				mPiles;
};
