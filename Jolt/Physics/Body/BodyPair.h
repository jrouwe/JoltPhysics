// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/HashCombine.h>

JPH_NAMESPACE_BEGIN

/// Structure that holds a body pair
struct BodyPair
{
	/// Constructor
							BodyPair() = default;
							BodyPair(BodyID inA, BodyID inB)							: mBodyA(inA), mBodyB(inB) { }

	/// Equals operator
	bool					operator == (const BodyPair &inRHS) const					{ static_assert(sizeof(*this) == sizeof(uint64), "Mismatch in class size"); return *reinterpret_cast<const uint64 *>(this) == *reinterpret_cast<const uint64 *>(&inRHS); }

	/// Smaller than operator, used for consistently ordering body pairs
	bool					operator < (const BodyPair &inRHS) const					{ static_assert(sizeof(*this) == sizeof(uint64), "Mismatch in class size"); return *reinterpret_cast<const uint64 *>(this) < *reinterpret_cast<const uint64 *>(&inRHS); }

	BodyID					mBodyA;
	BodyID					mBodyB;
};

JPH_MAKE_HASH_STRUCT(BodyPair, BodyPairHash, t.mBodyA.GetIndex(), t.mBodyB.GetIndex())

JPH_NAMESPACE_END
