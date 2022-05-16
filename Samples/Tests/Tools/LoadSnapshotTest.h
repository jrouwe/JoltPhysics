// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test loads a physics scene from 'snapshot.bin' and runs it
class LoadSnapshotTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(LoadSnapshotTest)

	// See: Test
	virtual void		Initialize() override;
};
