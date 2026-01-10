// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Hair/Hair.h>
#include <Jolt/Physics/Hair/HairShaders.h>

class HairTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, HairTest)

	// Destructor
	virtual					~HairTest() override										{ delete mHair; }

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Hair demo.";
	}

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.01f; }

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	using Gradient = HairSettings::Gradient;

	void					GradientSetting(DebugUI *inUI, UIElement *inSubMenu, const String &inName, float inMax, float inStep, Gradient &inStaticStorage, Gradient &inDynamicStorage);

	struct Groom
	{
		const char *		mName;
		std::function<Vec3(Mat44Arg, Vec3Arg)> mVertexTransform;
		bool				mAttachToHull;
	};

	static const Groom		sGrooms[];
	static const Groom *	sSelectedGroom;
	inline static bool		sLimitMaxStrands = true;
	inline static uint		sMaxStrands = JPH_IF_DEBUG(500u) JPH_IF_NOT_DEBUG(25000u);
	inline static float		sSimulationStrandsPercentage = 10.0f;
	inline static uint		sOverrideVerticesPerStrand = 32;
	inline static uint		sNumSolverIterationsPerSecond = HairSettings::cDefaultIterationsPerSecond;
	inline static bool		sEnableCollision = true;
	inline static bool		sEnableLRA = true;
	inline static float		sLinearDamping = 2.0f;
	inline static float		sAngularDamping = 2.0f;
	inline static float		sFriction = 0.2f;
	inline static float		sMaxLinearVelocity = 10.0f;
	inline static float		sMaxAngularVelocity = 50.0f;
	inline static float		sBendComplianceExponent = -7.0f; // 1.0e-7f
	inline static float		sStretchComplianceExponent = -8.0f; // 1.0e-8f
	inline static float		sInertiaMultiplier = 10.0f;
	inline static Gradient	sHairRadius { 0.001f, 0.001f };
	inline static Gradient	sWorldTransformInfluence { 0.0f, 1.0f };
	inline static Gradient	sGravityFactor { 0.1f, 1.0f, 0.2f, 0.8f };
	inline static float		sGravityPreloadFactor = 1.0f;
	inline static Gradient	sGridVelocityFactor { 0.05f, 0.01f };
	inline static Gradient	sGlobalPose { 0.01f, 0, 0.0f, 0.3f };
	inline static Gradient	sSkinGlobalPose { 1.0f, 0.0f, 0.0f, 0.1f };
	inline static float		sGridDensityForceFactor = 0.0f;
#ifdef JPH_DEBUG_RENDERER
	inline static uint		sDrawSimulationStrandBegin = 0;
	inline static uint		sDrawSimulationStrandCount = UINT_MAX;
	inline static bool		sDrawRods = false;
	inline static bool		sDrawUnloadedRods = false;
	inline static bool		sDrawVertexVelocity = false;
	inline static bool		sDrawAngularVelocity = false;
	inline static bool		sDrawOrientations = false;
	inline static bool		sDrawNeutralDensity = false;
	inline static bool		sDrawGridDensity = false;
	inline static bool		sDrawGridVelocity = false;
	inline static bool		sDrawSkinPoints = false;
	inline static Hair::ERenderStrandColor sRenderStrandColor = Hair::ERenderStrandColor::PerSimulatedStrand;
	inline static bool		sDrawInitialGravity = false;
#endif // JPH_DEBUG_RENDERER
	inline static bool		sDrawRenderStrands = true;
	inline static bool		sDrawHeadMesh = true;

	uint32					mHeadJointIdx = 0;
	Array<Array<Mat44>>		mFaceAnimation;
	struct AttachedBody
	{
		uint32				mJointIdx;
		BodyID				mBodyID;
	};
	Array<AttachedBody>		mAttachedBodies;
	Ref<HairSettings>		mHairSettings = nullptr;
	HairShaders				mHairShaders;
	Hair *					mHair = nullptr;
	uint					mFrame = 0;
};
