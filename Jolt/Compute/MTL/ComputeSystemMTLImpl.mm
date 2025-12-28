// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeSystemMTLImpl.h>

JPH_NAMESPACE_BEGIN

ComputeSystemMTLImpl::~ComputeSystemMTLImpl()
{
	Shutdown();

	[GetDevice() release];
}

bool ComputeSystemMTLImpl::Initialize()
{
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();

	return ComputeSystemMTL::Initialize(device);
}

ComputeSystem *CreateComputeSystemMTL()
{
	ComputeSystemMTLImpl *compute = new ComputeSystemMTLImpl;
	if (compute->Initialize())
		return compute;

	delete compute;
	return nullptr;
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
