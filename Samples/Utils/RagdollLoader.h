// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

namespace JPH {
	class RagdollSettings;
	enum class EMotionType : uint8;
}

#ifdef JPH_OBJECT_STREAM

enum class EConstraintOverride
{
	TypeFixed,
	TypePoint,
	TypeHinge,
	TypeSlider,
	TypeCone,
	TypeRagdoll,
};

#endif // JPH_OBJECT_STREAM

class RagdollLoader
{
public:
#ifdef JPH_OBJECT_STREAM
	/// Load a ragdoll from an ObjectStream file
	static RagdollSettings *		sLoad(const char *inFileName, EMotionType inMotionType, EConstraintOverride inConstraintOverride = EConstraintOverride::TypeRagdoll);
#endif // JPH_OBJECT_STREAM

	/// Create a ragdoll from code
	static RagdollSettings *		sCreate();
};
