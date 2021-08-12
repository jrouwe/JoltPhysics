// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

namespace JPH {

/// Whether or not to collect faces, used by CastShape and CollideShape
enum class ECollectFacesMode : uint8
{
	CollectFaces,										///< mShape1/2Face is desired
	NoFaces												///< mShape1/2Face is not desired
};

} // JPH