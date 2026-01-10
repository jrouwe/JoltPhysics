// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#if defined(JPH_USE_DX12) || defined(JPH_USE_VK) || defined(JPH_USE_MTL) || defined(JPH_USE_CPU_COMPUTE)

#include <Jolt/Compute/ComputeSystem.h>
#include <Jolt/Compute/CPU/ComputeSystemCPU.h>
#include <Jolt/Shaders/TestComputeBindings.h>
#include <Jolt/Shaders/TestCompute2Bindings.h>
#include <Jolt/Core/IncludeWindows.h>
#include <Jolt/Core/RTTI.h>

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

#ifdef JPH_USE_CPU_COMPUTE
JPH_DECLARE_REGISTER_SHADER(TestCompute)
JPH_DECLARE_REGISTER_SHADER(TestCompute2)
#endif // JPH_USE_CPU_COMPUTE

TEST_SUITE("ComputeTests")
{
	static const char *cInvalidShaderName = "InvalidShader";
	static const char *cInvalidShaderCode = "invalid_shader_code";

	static void RunTests(ComputeSystem *inComputeSystem)
	{
		inComputeSystem->mShaderLoader = [](const char *inName, Array<uint8> &outData, String &outError) {
			// Special case to test what happens when an invalid file is returned
			if (strstr(inName, cInvalidShaderName) != nullptr)
			{
				outData.assign(cInvalidShaderCode, cInvalidShaderCode + strlen(cInvalidShaderCode));
				return true;
			}

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
				// Not implemented
				const char *application_path = "";
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
			{
				outError = String("Could not open shader file: ") + base_path + inName;
				#if defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
					outError += "\nThis can fail on macOS when dxc or spirv-cross could not be found so the shaders could not be compiled.";
				#endif
				return false;
			}

			// Read contents of file
			input.seekg(0, ios_base::end);
			ifstream::pos_type length = input.tellg();
			input.seekg(0, ios_base::beg);
			outData.resize(size_t(length));
			if (length == 0)
				return true;
			input.read((char *)&outData[0], length);
			return true;
		};

		// Create a queue
		ComputeQueueResult queue_result = inComputeSystem->CreateComputeQueue();
		CHECK(!queue_result.HasError());
		Ref<ComputeQueue> queue = queue_result.Get();
		CHECK(queue != nullptr);

		// Test failing shader creation
		{
			ComputeShaderResult shader_result = inComputeSystem->CreateComputeShader("NonExistingShader", 64);
			CHECK(shader_result.HasError());
		}

		{
			constexpr uint32 cNumElements = 1234; // Not a multiple of cTestComputeGroupSize
			constexpr uint32 cNumIterations = 10;
			constexpr JPH_float3 cFloat3Value = JPH_float3(0, 0, 0);
			constexpr JPH_float3 cFloat3Value2 = JPH_float3(0, 13, 0);
			constexpr uint32 cUIntValue = 7;
			constexpr uint32 cUploadValue = 42;

			// Can't change context buffer while commands are queued, so create multiple constant buffers
			Ref<ComputeBuffer> context[cNumIterations];
			for (uint32 iter = 0; iter < cNumIterations; ++iter)
			{
				ComputeBufferResult buffer_result = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::ConstantBuffer, 1, sizeof(TestComputeContext));
				CHECK(!buffer_result.HasError());
				context[iter] = buffer_result.Get();
			}
			CHECK(context != nullptr);

			// Create an upload buffer
			ComputeBufferResult upload_buffer_result = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, 1, sizeof(uint32));
			CHECK(!upload_buffer_result.HasError());
			Ref<ComputeBuffer> upload_buffer = upload_buffer_result.Get();
			CHECK(upload_buffer != nullptr);
			uint32 *upload_data = upload_buffer->Map<uint32>(ComputeBuffer::EMode::Write);
			upload_data[0] = cUploadValue;
			upload_buffer->Unmap();

			// Create a read buffer
			UnitTestRandom rnd;
			Array<uint32> optional_data(cNumElements);
			for (uint32 &d : optional_data)
				d = rnd();
			ComputeBufferResult optional_buffer_result = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, cNumElements, sizeof(uint32), optional_data.data());
			CHECK(!optional_buffer_result.HasError());
			Ref<ComputeBuffer> optional_buffer = optional_buffer_result.Get();
			CHECK(optional_buffer != nullptr);

			// Create a read-write buffer
			ComputeBufferResult buffer_result = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, cNumElements, sizeof(uint32));
			CHECK(!buffer_result.HasError());
			Ref<ComputeBuffer> buffer = buffer_result.Get();
			CHECK(buffer != nullptr);

			// Create a read back buffer
			ComputeBufferResult readback_buffer_result = buffer->CreateReadBackBuffer();
			CHECK(!readback_buffer_result.HasError());
			Ref<ComputeBuffer> readback_buffer = readback_buffer_result.Get();
			CHECK(readback_buffer != nullptr);

			// Create the shader
			ComputeShaderResult shader_result = inComputeSystem->CreateComputeShader("TestCompute", cTestComputeGroupSize);
			if (shader_result.HasError())
			{
				Trace("Shader could not be created: %s", shader_result.GetError().c_str());
				return;
			}
			Ref<ComputeShader> shader = shader_result.Get();
			CHECK(shader != nullptr);

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

		// Test helper functions
		{
			// Create the shader
			ComputeShaderResult shader_result = inComputeSystem->CreateComputeShader("TestCompute2", cTestCompute2GroupSize);
			if (shader_result.HasError())
			{
				Trace("Shader could not be created: %s", shader_result.GetError().c_str());
				return;
			}
			Ref<ComputeShader> shader = shader_result.Get();
			CHECK(shader != nullptr);

			const Mat44 cMat44Value(Vec4(2, 3, 5, 0), Vec4(7, 11, 13, 0), Vec4(13, 15, 17, 0), Vec4(17, 19, 23, 0));
			const Vec3 cMat44MulValue(29, 31, 37);

			const Vec3 cDecompressedVec3(Vec3(-2, 3, -5).Normalized());
			const uint32 cCompressedVec3 = cDecompressedVec3.CompressUnitVector();

			const Quat cDecompressedQuat(Vec4(2, -3, 5, -7).Normalized());
			const uint32 cCompressedQuat = cDecompressedQuat.CompressUnitQuat();

			// Generate input data
			TestCompute2Input input;
			cMat44Value.StoreFloat4x4(input.mMat44Value);
			cMat44MulValue.StoreFloat3(&input.mMat44MulValue);
			input.mCompressedVec3 = cCompressedVec3;
			input.mCompressedQuat = cCompressedQuat;

			// Create input buffer
			ComputeBufferResult buffer_result = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, 1, sizeof(TestCompute2Input), &input);
			CHECK(!buffer_result.HasError());
			Ref<ComputeBuffer> input_buffer = buffer_result.Get();

			// Create a read-write buffer for the output
			buffer_result = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, 1, sizeof(TestCompute2Output));
			CHECK(!buffer_result.HasError());
			Ref<ComputeBuffer> output_buffer = buffer_result.Get();
			CHECK(output_buffer != nullptr);

			// Create a read back buffer
			buffer_result = output_buffer->CreateReadBackBuffer();
			CHECK(!buffer_result.HasError());
			Ref<ComputeBuffer> readback_buffer = buffer_result.Get();
			CHECK(readback_buffer != nullptr);

			// Execute the shader
			queue->SetShader(shader);
			queue->SetBuffer("gInput", input_buffer);
			queue->SetRWBuffer("gOutput", output_buffer);
			queue->Dispatch(1);
			queue->ScheduleReadback(readback_buffer, output_buffer);
			queue->ExecuteAndWait();

			// Verify the output
			TestCompute2Output *output = readback_buffer->Map<TestCompute2Output>(ComputeBuffer::EMode::Read);

			const Vec3 expected_mul3x4 = cMat44Value * cMat44MulValue;
			CHECK(Vec3(output->mMul3x4Output) == expected_mul3x4);

			const Vec3 expected_mul3x3 = cMat44Value.Multiply3x3(cMat44MulValue);
			CHECK(Vec3(output->mMul3x3Output) == expected_mul3x3);

			const Vec3 expected_decompressed_vec3 = Vec3::sDecompressUnitVector(cCompressedVec3);
			CHECK(Vec3(output->mDecompressedVec3).IsClose(expected_decompressed_vec3));

			const Quat expected_decompressed_quat = Quat::sDecompressUnitQuat(cCompressedQuat);
			CHECK(Quat(output->mDecompressedQuat).IsClose(expected_decompressed_quat));

			readback_buffer->Unmap();
		}
	}

#ifdef JPH_USE_DX12
	TEST_CASE("TestComputeDX12")
	{
		ComputeSystemResult compute_system = CreateComputeSystemDX12();
		CHECK(!compute_system.HasError());
		if (!compute_system.HasError())
		{
			CHECK(compute_system.Get() != nullptr);
			RunTests(compute_system.Get());

			// Test failing shader compilation
			{
				ComputeShaderResult shader_result = compute_system.Get()->CreateComputeShader(cInvalidShaderName, 64);
				CHECK(shader_result.HasError());
				CHECK(strstr(shader_result.GetError().c_str(), cInvalidShaderCode) != nullptr); // Assume that the error message contains the invalid code
			}
		}
	}
#endif // JPH_USE_DX12

#ifdef JPH_USE_MTL
	TEST_CASE("TestComputeMTL")
	{
		ComputeSystemResult compute_system = CreateComputeSystemMTL();
		CHECK(!compute_system.HasError());
		if (!compute_system.HasError())
		{
			CHECK(compute_system.Get() != nullptr);
			RunTests(compute_system.Get());
		}
	}
#endif // JPH_USE_MTL

#ifdef JPH_USE_VK
	TEST_CASE("TestComputeVK")
	{
		ComputeSystemResult compute_system = CreateComputeSystemVK();
		CHECK(!compute_system.HasError());
		if (!compute_system.HasError())
		{
			CHECK(compute_system.Get() != nullptr);
			RunTests(compute_system.Get());
		}
	}
#endif // JPH_USE_VK

#ifdef JPH_USE_CPU_COMPUTE
	TEST_CASE("TestComputeCPU")
	{
		ComputeSystemResult compute_system = CreateComputeSystemCPU();
		CHECK(!compute_system.HasError());
		if (!compute_system.HasError())
		{
			CHECK(compute_system.Get() != nullptr);
			JPH_REGISTER_SHADER(StaticCast<ComputeSystemCPU>(compute_system.Get()), TestCompute);
			JPH_REGISTER_SHADER(StaticCast<ComputeSystemCPU>(compute_system.Get()), TestCompute2);
			RunTests(compute_system.Get());
		}
	}
#endif // JPH_USE_CPU_COMPUTE
}

#endif // defined(JPH_USE_DX12) || defined(JPH_USE_VK) || defined(JPH_USE_MTL) || defined(JPH_USE_CPU_COMPUTE)
