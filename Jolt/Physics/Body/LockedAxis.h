// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Enum used in BodyCreationSettings and MotionProperties to indicate which degrees of freedom a body has
enum class ELockedAxis : uint8
{
	None				= 0b000000,		///< No axis locked
	LockTranslationX	= 0b000001,		///< Body cannot move in X axis
	LockTranslationY	= 0b000010,		///< Body cannot move in Y axis
	LockTranslationZ	= 0b000100,		///< Body cannot move in Z axis
	LockRotationX		= 0b001000,		///< Body cannot rotate around X axis
	LockRotationY		= 0b010000,		///< Body cannot rotate around Y axis
	LockRotationZ		= 0b100000,		///< Body cannot rotate around Z axis
	LockToXYPlane		= LockTranslationZ | LockRotationX | LockRotationY, ///< Body is constrained to the XY plane and can only rotate around Z
};

JPH_NAMESPACE_END
