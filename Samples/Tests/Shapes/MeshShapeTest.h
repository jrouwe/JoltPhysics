// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class MeshShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(MeshShapeTest)

	// See: Test
	virtual void	Initialize() override;
};
