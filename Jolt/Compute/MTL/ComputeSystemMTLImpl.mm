// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_MTL

#include <Jolt/Compute/MTL/ComputeSystemMTLImpl.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemMTLImpl)
{
	JPH_ADD_BASE_CLASS(ComputeSystemMTLImpl, ComputeSystemMTL)
}

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

ComputeSystemResult CreateComputeSystemMTL()
{
	ComputeSystemResult result;

	Ref<ComputeSystemMTLImpl> compute = new ComputeSystemMTLImpl;
	if (!compute->Initialize())
	{
		result.SetError("Failed to initialize compute system");
		return result;
	}

	result.Set(compute.GetPtr());
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_MTL
