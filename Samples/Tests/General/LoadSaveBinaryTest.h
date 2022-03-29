// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests the binary serialization system by creating a number of shapes, storing them, loading them and then simulating them
class LoadSaveBinaryTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(LoadSaveBinaryTest)

	// See: Test
	virtual void		Initialize() override;
};
