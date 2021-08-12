// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Vehicle/VehicleController.h>
#include <ObjectStream/TypeDeclarations.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT(VehicleControllerSettings)
{
	JPH_ADD_BASE_CLASS(VehicleControllerSettings, SerializableObject)
}

} // JPH