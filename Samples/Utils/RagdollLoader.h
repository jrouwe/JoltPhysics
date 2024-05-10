// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifndef JPH_NO_OBJECT_STREAM

namespace JPH {
	class RagdollSettings;
	enum class EMotionType : uint8;
}

enum class EConstraintOverride
{
	TypeFixed,
	TypePoint,
	TypeHinge,
	TypeSlider,
	TypeCone,
	TypeRagdoll,
};

class RagdollLoader
{
public:
	static RagdollSettings *		sLoad(const char *inFileName, EMotionType inMotionType, EConstraintOverride inConstraintOverride = EConstraintOverride::TypeRagdoll);
};

#endif // !JPH_NO_OBJECT_STREAM
