// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

namespace JPH {

/// How to treat active/inactive edges. 
/// An active edge is an edge that either has no neighbouring edge or if the angle between the two connecting faces is too large, see: ActiveEdges
enum class EActiveEdgeMode : uint8
{
	CollideOnlyWithActive,								///< Do not collide with inactive edges. For physics simulation, this gives less ghost collisions.
	CollideWithAll,										///< Collide with all edges. Use this when you're interested in all collisions.
};

} // JPH