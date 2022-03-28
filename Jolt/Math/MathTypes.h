// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

class Vec3;
class DVec3;
class Vec4;
class UVec4;
class Vec8;
class UVec8;
class Quat;
class Mat44;

// Types to use for passing arguments to functions
using Vec3Arg = Vec3;
using DVec3Arg = DVec3;
using Vec4Arg = Vec4;
using UVec4Arg = UVec4;
using Vec8Arg = Vec8;
using UVec8Arg = UVec8;
using QuatArg = Quat;
using Mat44Arg = const Mat44 &;

JPH_NAMESPACE_END
