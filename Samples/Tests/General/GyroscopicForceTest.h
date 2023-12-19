// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

// This shows how to enable gyrosopic forces to create the Dzhanibekov effect (see: https://en.wikipedia.org/wiki/Tennis_racket_theorem)
class GyroscopicForceTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, GyroscopicForceTest)

	// See: Test
	virtual void		Initialize() override;
};
