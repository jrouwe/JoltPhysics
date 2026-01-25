// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeSystemMTL.h>

JPH_NAMESPACE_BEGIN

/// Interface to run a workload on the GPU that fully initializes Metal.
class JPH_EXPORT ComputeSystemMTLImpl : public ComputeSystemMTL
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_EXPORT, ComputeSystemMTLImpl)

	/// Destructor
	virtual							~ComputeSystemMTLImpl() override;

	/// Initialize / shutdown the compute system
	bool							Initialize();
};

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
