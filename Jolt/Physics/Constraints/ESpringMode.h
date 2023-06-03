// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Enum used by constraints to specify how the spring is defined
enum class ESpringMode : uint8
{
	FrequencyAndDamping,	///< Frequency and damping are specified
	StiffnessAndDamping,	///< Stiffness and damping are specified
};

JPH_NAMESPACE_END
