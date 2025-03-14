// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/Body.h>

class SoftBodySkinnedConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodySkinnedConstraintTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Shows how to attach a soft body to a skinned mesh and control the animation.";
	}

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void			GetInitialCamera(CameraState &ioState) const override		{ ioState.mPos = RVec3(15, 30, 15); }
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	// Size and spacing of the cloth
	static constexpr int	cNumVerticesX = 10;
	static constexpr int	cNumVerticesZ = 50;
	static constexpr float	cVertexSpacing = 0.5f;

	// Number of joints that drive the cloth
	static constexpr int	cNumJoints = 11;

	// Position of the body
	static constexpr float	cBodyPosY = 20.0f;

	// Get a procedurally generated pose
	Array<Mat44>			GetWorldSpacePose(float inTime) const;

	// Skin the vertices of the soft body to the pose
	void					SkinVertices(bool inHardSkinAll);

	// The soft body
	Body *					mBody;

	// Current time
	float					mTime = 0.0f;

	// Settings
	static inline float		sTimeScale = 1.0f;
	static inline bool		sUpdateSkinning = true;
	static inline bool		sEnableSkinConstraints = true;
	static inline float		sMaxDistanceMultiplier = 1.0f;
};
