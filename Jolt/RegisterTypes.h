// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Register all physics types with the factory
JPH_EXPORT extern void RegisterTypes();

/// Unregisters all types with the factory and cleans up the default material
JPH_EXPORT extern void UnregisterTypes();

JPH_NAMESPACE_END
