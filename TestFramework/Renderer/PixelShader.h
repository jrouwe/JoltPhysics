// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>

/// Pixel shader handle
class PixelShader : public RefTarget<PixelShader>
{
public:
	/// Destructor
	virtual					~PixelShader() = default;
};
