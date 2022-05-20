// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test drops a mesh shape on a box
class DynamicMeshTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(DynamicMeshTest)

	// See: Test
	virtual void		Initialize() override;
};
