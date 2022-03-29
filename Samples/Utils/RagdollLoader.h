// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

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
