// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

JPH_NAMESPACE_BEGIN

// Table that shifts vector components by 4 - X floats to the left
const UVec4 UVec4::sFourMinusXShuffle[5] = 
{
	UVec4(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff),
	UVec4(0x0f0e0d0c, 0xffffffff, 0xffffffff, 0xffffffff),
	UVec4(0x0b0a0908, 0x0f0e0d0c, 0xffffffff, 0xffffffff),
	UVec4(0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0xffffffff),
	UVec4(0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c)
};

JPH_NAMESPACE_END
