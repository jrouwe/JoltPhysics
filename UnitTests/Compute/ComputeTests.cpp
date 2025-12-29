// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#if defined(JPH_USE_DX12) || defined(JPH_USE_MTL) || defined(JPH_USE_VK)

#include <Jolt/Compute/ComputeSystem.h>
#include <Jolt/Shaders/TestCompute.h>
#include <Jolt/Core/IncludeWindows.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
#include <filesystem>
#ifdef JPH_PLATFORM_LINUX
#include <unistd.h>
#endif
JPH_SUPPRESS_WARNINGS_STD_END

#if defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#endif

TEST_SUITE("ComputeTests")
{
	static void RunTests(ComputeSystem *inComputeSystem)
	{
		inComputeSystem->mShaderLoader = [](const char *inName, Array<uint8> &outData) {
		#if defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
			// In macOS the shaders are copied to the bundle
			CFBundleRef bundle = CFBundleGetMainBundle();
			CFURLRef resources = CFBundleCopyResourcesDirectoryURL(bundle);
			CFURLRef absolute = CFURLCopyAbsoluteURL(resources);
			CFRelease(resources);
			CFStringRef path_string = CFURLCopyFileSystemPath(absolute, kCFURLPOSIXPathStyle);
			CFRelease(absolute);
			char path[PATH_MAX];
			CFStringGetCString(path_string, path, PATH_MAX, kCFStringEncodingUTF8);
			CFRelease(path_string);
    		String base_path = String(path) + "/Jolt/Shaders/";
		#else
			// On other platforms, start searching up from the application path
			#ifdef JPH_PLATFORM_WINDOWS
				char application_path[MAX_PATH] = { 0 };
				GetModuleFileName(nullptr, application_path, MAX_PATH);
			#elif defined(JPH_PLATFORM_LINUX)
				char application_path[PATH_MAX] = { 0 };
				int count = readlink("/proc/self/exe", application_path, PATH_MAX);
				if (count > 0)
					application_path[count] = 0;
			#else
				#error Unsupported platform
			#endif
			String base_path;
			filesystem::path shader_path(application_path);
			while (!shader_path.empty())
			{
				filesystem::path parent_path = shader_path.parent_path();
				if (parent_path == shader_path)
					break;
				shader_path = parent_path;
				filesystem::path full_path = shader_path / "Jolt" / "Shaders" / "";
				if (filesystem::exists(full_path))
				{
					base_path = String(full_path.string());
					break;
				}
			}
		#endif

			// Open file
			std::ifstream input((base_path + inName).c_str(), std::ios::in | std::ios::binary);
			if (!input.is_open())
				return false;

			// Read contents of file
			input.seekg(0, ios_base::end);
			ifstream::pos_type length = input.tellg();
			input.seekg(0, ios_base::beg);
			outData.resize(size_t(length));
			input.read((char *)&outData[0], length);
			return true;
		};

		constexpr uint32 cNumElements = 1234; // Not a multiple of cTestComputeGroupSize
		constexpr uint32 cNumIterations = 10;
		constexpr JPH_float3 cFloat3Value = JPH_float3(0, 0, 0);
		constexpr JPH_float3 cFloat3Value2 = JPH_float3(0, 13, 0);
		constexpr uint32 cUIntValue = 7;
		constexpr uint32 cUploadValue = 42;

		// Can't change context buffer while commands are queued, so create multiple constant buffers
		Ref<ComputeBuffer> context[cNumIterations];
		for (uint32 iter = 0; iter < cNumIterations; ++iter)
			context[iter] = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::ConstantBuffer, 1, sizeof(TestComputeContext));
		CHECK(context != nullptr);

		// Create an upload buffer
		Ref<ComputeBuffer> upload_buffer = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, 1, sizeof(uint32));
		CHECK(upload_buffer != nullptr);
		uint32 *upload_data = upload_buffer->Map<uint32>(ComputeBuffer::EMode::Write);
		upload_data[0] = cUploadValue;
		upload_buffer->Unmap();

		// Create a read buffer
		UnitTestRandom rnd;
		Array<uint32> optional_data(cNumElements);
		for (uint32 &d : optional_data)
			d = rnd();
		Ref<ComputeBuffer> optional_buffer = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, cNumElements, sizeof(uint32), optional_data.data());
		CHECK(optional_buffer != nullptr);

		// Create a read-write buffer
		Ref<ComputeBuffer> buffer = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, cNumElements, sizeof(uint32));
		CHECK(buffer != nullptr);

		// Create a read back buffer
		Ref<ComputeBuffer> readback_buffer = buffer->CreateReadBackBuffer();
		CHECK(readback_buffer != nullptr);

		// Create the shader
		Ref<ComputeShader> shader = inComputeSystem->CreateComputeShader("TestCompute", cTestComputeGroupSize);
		CHECK(shader != nullptr);
		if (shader == nullptr) // Shader was not found. This can e.g. fail on macOS when dxc or spirv-cross could not be found so the shaders could not be compiled.
			return;

		// Create the queue
		Ref<ComputeQueue> queue = inComputeSystem->CreateComputeQueue();

		// Schedule work
		for (uint32 iter = 0; iter < cNumIterations; ++iter)
		{
			// Fill in the context
			TestComputeContext *value = context[iter]->Map<TestComputeContext>(ComputeBuffer::EMode::Write);
			value->cFloat3Value = cFloat3Value;
			value->cUIntValue = cUIntValue;
			value->cFloat3Value2 = cFloat3Value2;
			value->cUIntValue2 = iter;
			value->cNumElements = cNumElements;
			context[iter]->Unmap();

			queue->SetShader(shader);
			queue->SetConstantBuffer("gContext", context[iter]);
			context[iter] = nullptr; // Release the reference to ensure the queue keeps ownership
			queue->SetBuffer("gOptionalData", optional_buffer);
			optional_buffer = nullptr; // Release the reference so we test that the queue keeps ownership and that in the 2nd iteration we can set a null buffer
			queue->SetBuffer("gUploadData", upload_buffer);
			queue->SetRWBuffer("gData", buffer);
			queue->Dispatch((cNumElements + cTestComputeGroupSize - 1) / cTestComputeGroupSize);
		}

		// Run all queued commands
		queue->ScheduleReadback(readback_buffer, buffer);
		queue->ExecuteAndWait();

		// Calculate the expected result
		Array<uint32> expected_data(cNumElements);
		for (uint32 iter = 0; iter < cNumIterations; ++iter)
		{
			// Copy of the shader logic
			uint cUIntValue2 = iter;
			if (cUIntValue2 == 0)
			{
				// First write, uses optional data and tests that the packing of float3/uint3's works
				for (uint32 i = 0; i < cNumElements; ++i)
					expected_data[i] = optional_data[i] + int(cFloat3Value2.y) + cUploadValue;
			}
			else
			{
				// Read-modify-write gData
				for (uint32 i = 0; i < cNumElements; ++i)
					expected_data[i] = (expected_data[i] + cUIntValue) * cUIntValue2;
			}
		}

		// Compare computed data with expected data
		uint32 *data = readback_buffer->Map<uint32>(ComputeBuffer::EMode::Read);
		for (uint32 i = 0; i < cNumElements; ++i)
			CHECK(data[i] == expected_data[i]);
		readback_buffer->Unmap();
	}

#ifdef JPH_USE_DX12
	TEST_CASE("TestComputeDX12")
	{
		Ref<ComputeSystem> compute_system = CreateComputeSystemDX12();
		CHECK(compute_system != nullptr);
		if (compute_system != nullptr)
			RunTests(compute_system);
	}
#endif // JPH_USE_DX12

#ifdef JPH_USE_MTL
	TEST_CASE("TestComputeMTL")
	{
		Ref<ComputeSystem> compute_system = CreateComputeSystemMTL();
		CHECK(compute_system != nullptr);
		if (compute_system != nullptr)
			RunTests(compute_system);
	}
#endif // JPH_USE_MTL

#ifdef JPH_USE_VK
	TEST_CASE("TestComputeVK")
	{
		Ref<ComputeSystem> compute_system = CreateComputeSystemVK();
		CHECK(compute_system != nullptr);
		if (compute_system != nullptr)
			RunTests(compute_system);
	}
#endif // JPH_USE_VK
}

#endif // defined(JPH_USE_DX12) || defined(JPH_USE_MTL) || defined(JPH_USE_VK)
