// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <vulkan/vulkan.h>

/// Convert Vulkan error to readable text and alert
void FatalErrorIfFailed(VkResult inVkResult);

