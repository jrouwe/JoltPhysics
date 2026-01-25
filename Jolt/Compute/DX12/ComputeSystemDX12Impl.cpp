// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_DX12

#include <Jolt/Compute/DX12/ComputeSystemDX12Impl.h>

#ifdef JPH_DEBUG
	#include <d3d12sdklayers.h>
#endif

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemDX12Impl)
{
	JPH_ADD_BASE_CLASS(ComputeSystemDX12Impl, ComputeSystemDX12)
}

ComputeSystemDX12Impl::~ComputeSystemDX12Impl()
{
	Shutdown();
	mDXGIFactory.Reset();

#ifdef JPH_DEBUG
	// Test for leaks
	ComPtr<IDXGIDebug1> dxgi_debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug))))
		dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
}

bool ComputeSystemDX12Impl::Initialize(ComputeSystemResult &outResult)
{
#if defined(JPH_DEBUG)
	// Enable the D3D12 debug layer
	ComPtr<ID3D12Debug> debug_controller;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
		debug_controller->EnableDebugLayer();
#endif

	// Create DXGI factory
	if (HRFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mDXGIFactory)), outResult))
		return false;

	// Find adapter
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<ID3D12Device> device;

	HRESULT result = E_FAIL;

	// First check if we have the Windows 1803 IDXGIFactory6 interface
	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(mDXGIFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (int search_software = 0; search_software < 2 && device == nullptr; ++search_software)
			for (UINT index = 0; factory6->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++index)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				// We don't want software renderers in the first pass
				int is_software = (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0? 1 : 0;
				if (search_software != is_software)
					continue;

				// Check to see whether the adapter supports Direct3D 12
			#if defined(JPH_PLATFORM_WINDOWS) && defined(_DEBUG)
				int prev_state = _CrtSetDbgFlag(0); // Temporarily disable leak detection as this call reports false positives
			#endif
				result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
			#if defined(JPH_PLATFORM_WINDOWS) && defined(_DEBUG)
				_CrtSetDbgFlag(prev_state);
			#endif
				if (SUCCEEDED(result))
					break;
			}
	}
	else
	{
		// Fall back to the older method that may not get the fastest GPU
		for (int search_software = 0; search_software < 2 && device == nullptr; ++search_software)
			for (UINT index = 0; mDXGIFactory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				// We don't want software renderers in the first pass
				int is_software = (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0? 1 : 0;
				if (search_software != is_software)
					continue;

				// Check to see whether the adapter supports Direct3D 12
			#if defined(JPH_PLATFORM_WINDOWS) && defined(_DEBUG)
				int prev_state = _CrtSetDbgFlag(0); // Temporarily disable leak detection as this call reports false positives
			#endif
				result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
			#if defined(JPH_PLATFORM_WINDOWS) && defined(_DEBUG)
				_CrtSetDbgFlag(prev_state);
			#endif
				if (SUCCEEDED(result))
					break;
			}
	}

	// Check if we managed to obtain a device
	if (HRFailed(result, outResult))
		return false;

	// Initialize the compute interface
	ComputeSystemDX12::Initialize(device.Get(), EDebug::DebugSymbols);

#ifdef JPH_DEBUG
	// Enable breaking on errors
	ComPtr<ID3D12InfoQueue> info_queue;
	if (SUCCEEDED(device.As(&info_queue)))
	{
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Disable an error that triggers on Windows 11 with a hybrid graphic system
		// See: https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
		};
		D3D12_INFO_QUEUE_FILTER filter = { };
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;
		info_queue->AddStorageFilterEntries(&filter);
	}
#endif // JPH_DEBUG

	return true;
}

ComputeSystemResult CreateComputeSystemDX12()
{
	ComputeSystemResult result;

	Ref<ComputeSystemDX12Impl> compute = new ComputeSystemDX12Impl();
	if (!compute->Initialize(result))
		return result;

	result.Set(compute.GetPtr());
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
