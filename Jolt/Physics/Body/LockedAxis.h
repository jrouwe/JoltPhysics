// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Enum used in BodyCreationSettings and MotionProperties to indicate which degrees of freedom a body has
enum class ELockedAxis : uint8
{
	None				= 0b000000,								///< No axis locked
	TranslationX		= 0b000001,								///< Body cannot move in X axis
	TranslationY		= 0b000010,								///< Body cannot move in Y axis
	TranslationZ		= 0b000100,								///< Body cannot move in Z axis
	RotationX			= 0b001000,								///< Body cannot rotate around X axis
	RotationY			= 0b010000,								///< Body cannot rotate around Y axis
	RotationZ			= 0b100000,								///< Body cannot rotate around Z axis
	Plane2D				= TranslationZ | RotationX | RotationY,	///< Body can only move in X and Y axis and rotate around Z axis
};

/// Bitwise OR operator for ELockedAxis
inline ELockedAxis operator | (ELockedAxis inLHS, ELockedAxis inRHS)
{
	return ELockedAxis(uint8(inLHS) | uint8(inRHS));
}

/// Bitwise AND operator for ELockedAxis
inline ELockedAxis operator & (ELockedAxis inLHS, ELockedAxis inRHS)
{
	return ELockedAxis(uint8(inLHS) & uint8(inRHS));
}

JPH_NAMESPACE_END
