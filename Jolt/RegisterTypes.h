// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Internal helper function
JPH_EXPORT extern void RegisterTypesInternal(uint64 inVersionID);

/// Register all physics types with the factory
inline void RegisterTypes() { RegisterTypesInternal(JPH_VERSION_ID); }

/// Unregisters all types with the factory and cleans up the default material
JPH_EXPORT extern void UnregisterTypes();

JPH_NAMESPACE_END
