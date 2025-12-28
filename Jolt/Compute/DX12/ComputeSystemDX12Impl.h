// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_DX12

#include <Jolt/Compute/DX12/ComputeSystemDX12.h>

JPH_NAMESPACE_BEGIN

/// Implementation of ComputeSystemDX12 that fully initializes DirectX 12
class JPH_EXPORT ComputeSystemDX12Impl : public ComputeSystemDX12
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Destructor
	virtual 						~ComputeSystemDX12Impl() override;

	/// Initialize the compute system
	bool							Initialize();

	IDXGIFactory4 *					GetDXGIFactory() const						{ return mDXGIFactory.Get(); }

private:
	ComPtr<IDXGIFactory4>			mDXGIFactory;
};

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
