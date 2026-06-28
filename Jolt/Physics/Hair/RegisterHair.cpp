// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/RTTI.h>

JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH_EXPORT, JPH, HairSettings)

JPH_NAMESPACE_BEGIN

void RegisterHair()
{
	Factory::sInstance->Register(JPH_RTTI(HairSettings));
}

JPH_NAMESPACE_END
