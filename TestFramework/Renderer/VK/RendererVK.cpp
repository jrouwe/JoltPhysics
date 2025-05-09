// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/RendererVK.h>
#include <Renderer/VK/RenderPrimitiveVK.h>
#include <Renderer/VK/RenderInstancesVK.h>
#include <Renderer/VK/PipelineStateVK.h>
#include <Renderer/VK/VertexShaderVK.h>
#include <Renderer/VK/PixelShaderVK.h>
#include <Renderer/VK/TextureVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>
#include <Utils/Log.h>
#include <Utils/ReadData.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/QuickSort.h>
#include <Jolt/Core/RTTI.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#ifdef JPH_PLATFORM_WINDOWS
	#include <vulkan/vulkan_win32.h>
	#include <Window/ApplicationWindowWin.h>
#elif defined(JPH_PLATFORM_LINUX)
	#include <vulkan/vulkan_xlib.h>
	#include <Window/ApplicationWindowLinux.h>
#elif defined(JPH_PLATFORM_MACOS)
	#include <vulkan/vulkan_metal.h>
	#include <Window/ApplicationWindowMacOS.h>
#endif
JPH_SUPPRESS_WARNINGS_STD_END

#ifdef JPH_DEBUG

static VKAPI_ATTR VkBool32 VKAPI_CALL sVulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT inSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT inType, const VkDebugUtilsMessengerCallbackDataEXT *inCallbackData, [[maybe_unused]] void *inUserData)
{
	Trace("VK: %s", inCallbackData->pMessage);
	JPH_ASSERT((inSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) == 0);
	return VK_FALSE;
}

#endif // JPH_DEBUG

RendererVK::~RendererVK()
{
	vkDeviceWaitIdle(mDevice);

	// Trace allocation stats
	Trace("VK: Max allocations: %u, max size: %u MB", mMaxNumAllocations, uint32(mMaxTotalAllocated >> 20));

	// Destroy the shadow map
	mShadowMap = nullptr;
	vkDestroyFramebuffer(mDevice, mShadowFrameBuffer, nullptr);

	// Release constant buffers
	for (unique_ptr<ConstantBufferVK> &cb : mVertexShaderConstantBufferProjection)
		cb = nullptr;
	for (unique_ptr<ConstantBufferVK> &cb : mVertexShaderConstantBufferOrtho)
		cb = nullptr;
	for (unique_ptr<ConstantBufferVK> &cb : mPixelShaderConstantBuffer)
		cb = nullptr;
	
	// Free all buffers
	for (BufferCache &bc : mFreedBuffers)
		for (BufferCache::value_type &vt : bc)
			for (BufferVK &bvk : vt.second)
				FreeBufferInternal(bvk);
	for (BufferCache::value_type &vt : mBufferCache)
		for (BufferVK &bvk : vt.second)
			FreeBufferInternal(bvk);

	// Free all blocks in the memory cache
	for (MemoryCache::value_type &mc : mMemoryCache)
		for (Memory &m : mc.second)
			if (m.mOffset == 0)
				vkFreeMemory(mDevice, m.mMemory, nullptr); // Don't care about memory tracking anymore
	
	for (VkFence fence : mInFlightFences)
		vkDestroyFence(mDevice, fence, nullptr);

	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);

	vkDestroyRenderPass(mDevice, mRenderPassShadow, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

	vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

	vkDestroySampler(mDevice, mTextureSamplerShadow, nullptr);
	vkDestroySampler(mDevice, mTextureSamplerRepeat, nullptr);

	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayoutUBO, nullptr);
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayoutTexture, nullptr);

	DestroySwapChain();

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

	vkDestroyDevice(mDevice, nullptr);

#ifdef JPH_DEBUG
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)(void *)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (vkDestroyDebugUtilsMessengerEXT != nullptr)
		vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif

	 vkDestroyInstance(mInstance, nullptr);
}

void RendererVK::Initialize(ApplicationWindow *inWindow)
{
	Renderer::Initialize(inWindow);

	// Flip the sign of the projection matrix
	mPerspectiveYSign = -1.0f;

	// Required instance extensions
	Array<const char *> required_instance_extensions;
	required_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef JPH_PLATFORM_WINDOWS
	required_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(JPH_PLATFORM_LINUX)
	required_instance_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(JPH_PLATFORM_MACOS)
	required_instance_extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
	required_instance_extensions.push_back("VK_KHR_portability_enumeration");
	required_instance_extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

	// Required device extensions
	Array<const char *> required_device_extensions;
	required_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef JPH_PLATFORM_MACOS
	required_device_extensions.push_back("VK_KHR_portability_subset"); // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#endif

	// Query supported instance extensions
	uint32 instance_extension_count = 0;
	FatalErrorIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
	Array<VkExtensionProperties> instance_extensions;
	instance_extensions.resize(instance_extension_count);
	FatalErrorIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

	// Query supported validation layers
	uint32 validation_layer_count;
	vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
	Array<VkLayerProperties> validation_layers(validation_layer_count);
	vkEnumerateInstanceLayerProperties(&validation_layer_count, validation_layers.data());

	// Create Vulkan instance
	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef JPH_PLATFORM_MACOS
	instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

#ifdef JPH_DEBUG
	// Enable validation layer if supported
	const char *desired_validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
	for (const VkLayerProperties &p : validation_layers)
		if (strcmp(desired_validation_layers[0], p.layerName) == 0)
		{
			instance_create_info.enabledLayerCount = 1;
			instance_create_info.ppEnabledLayerNames = desired_validation_layers;
			break;
		}

	// Setup debug messenger callback if the extension is supported
	VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = {};
	for (const VkExtensionProperties &ext : instance_extensions)
		if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ext.extensionName) == 0)
		{
			messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			messenger_create_info.pfnUserCallback = sVulkanDebugCallback;
			instance_create_info.pNext = &messenger_create_info;
			required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			break;
		}
#endif

	instance_create_info.enabledExtensionCount = (uint32)required_instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames = required_instance_extensions.data();
	FatalErrorIfFailed(vkCreateInstance(&instance_create_info, nullptr, &mInstance));

#ifdef JPH_DEBUG
	// Finalize debug messenger callback
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)(std::uintptr_t)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
	if (vkCreateDebugUtilsMessengerEXT != nullptr)
		FatalErrorIfFailed(vkCreateDebugUtilsMessengerEXT(mInstance, &messenger_create_info, nullptr, &mDebugMessenger));
#endif

	// Create surface
#ifdef JPH_PLATFORM_WINDOWS
	VkWin32SurfaceCreateInfoKHR surface_create_info = {};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.hwnd = static_cast<ApplicationWindowWin *>(mWindow)->GetWindowHandle();
	surface_create_info.hinstance = GetModuleHandle(nullptr);
	FatalErrorIfFailed(vkCreateWin32SurfaceKHR(mInstance, &surface_create_info, nullptr, &mSurface));
#elif defined(JPH_PLATFORM_LINUX)
	VkXlibSurfaceCreateInfoKHR surface_create_info = {};
	surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surface_create_info.dpy = static_cast<ApplicationWindowLinux *>(mWindow)->GetDisplay();
	surface_create_info.window = static_cast<ApplicationWindowLinux *>(mWindow)->GetWindow();
	FatalErrorIfFailed(vkCreateXlibSurfaceKHR(mInstance, &surface_create_info, nullptr, &mSurface));
#elif defined(JPH_PLATFORM_MACOS)
	VkMetalSurfaceCreateInfoEXT surface_create_info = {};
	surface_create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
	surface_create_info.pNext = nullptr;
	surface_create_info.pLayer = static_cast<ApplicationWindowMacOS *>(mWindow)->GetMetalLayer();
	FatalErrorIfFailed(vkCreateMetalSurfaceEXT(mInstance, &surface_create_info, nullptr, &mSurface));
#endif

	// Select device
	uint32 device_count = 0;
	FatalErrorIfFailed(vkEnumeratePhysicalDevices(mInstance, &device_count, nullptr));
	Array<VkPhysicalDevice> devices;
	devices.resize(device_count);
	FatalErrorIfFailed(vkEnumeratePhysicalDevices(mInstance, &device_count, devices.data()));
	struct Device
	{
		VkPhysicalDevice		mPhysicalDevice;
		String					mName;
		VkSurfaceFormatKHR		mFormat;
		uint32					mGraphicsQueueIndex;
		uint32					mPresentQueueIndex;
		int						mScore;
	};
	Array<Device> available_devices;
	for (VkPhysicalDevice device : devices)
	{
		// Get device properties
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		// Test if it is an appropriate type
		int score = 0;
		switch (properties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			score = 30;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			score = 20;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			score = 10;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			score = 5;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
			continue;
		}

		// Check if the device supports all our required extensions
		uint32 device_extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &device_extension_count, nullptr);
		Array<VkExtensionProperties> available_extensions;
		available_extensions.resize(device_extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &device_extension_count, available_extensions.data());
		int found_extensions = 0;
		for (const char *required_device_extension : required_device_extensions)
			for (const VkExtensionProperties &ext : available_extensions)
				if (strcmp(required_device_extension, ext.extensionName) == 0)
				{
					found_extensions++;
					break;
				}
		if (found_extensions != int(required_device_extensions.size()))
			continue;

		// Find the right queues
		uint32 queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		Array<VkQueueFamilyProperties> queue_families;
		queue_families.resize(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
		uint32 graphics_queue = ~uint32(0);
		uint32 present_queue = ~uint32(0);
		for (uint32 i = 0; i < uint32(queue_families.size()); ++i)
		{
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphics_queue = i;

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &present_support);
			if (present_support)
				present_queue = i;

			if (graphics_queue != ~uint32(0) && present_queue != ~uint32(0))
				break;
		}
		if (graphics_queue == ~uint32(0) || present_queue == ~uint32(0))
			continue;

		// Select surface format
		VkSurfaceFormatKHR selected_format = SelectFormat(device);
		if (selected_format.format == VK_FORMAT_UNDEFINED)
			continue;

		// Add the device
		available_devices.push_back({ device, properties.deviceName, selected_format, graphics_queue, present_queue, score });
	}
	if (available_devices.empty())
		FatalError("No Vulkan device found!");
	QuickSort(available_devices.begin(), available_devices.end(), [](const Device &inLHS, const Device &inRHS) {
		return inLHS.mScore > inRHS.mScore;
	});
	const Device &selected_device = available_devices[0];
	Trace("Selected device: %s", selected_device.mName.c_str());
	mPhysicalDevice = selected_device.mPhysicalDevice;

	// Get memory properties
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

	// Get features
	VkPhysicalDeviceFeatures physical_device_features = {};
	vkGetPhysicalDeviceFeatures(mPhysicalDevice, &physical_device_features);

	// Create device
	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo queue_create_info[2] = {};
	for (size_t i = 0; i < std::size(queue_create_info); ++i)
	{
		queue_create_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[i].queueCount = 1;
		queue_create_info[i].pQueuePriorities = &queue_priority;
	}
	queue_create_info[0].queueFamilyIndex = selected_device.mGraphicsQueueIndex;
	queue_create_info[1].queueFamilyIndex = selected_device.mPresentQueueIndex;

	VkPhysicalDeviceFeatures device_features = {};

	if (!physical_device_features.fillModeNonSolid)
		FatalError("fillModeNonSolid not supported!");
	device_features.fillModeNonSolid = VK_TRUE;

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = selected_device.mGraphicsQueueIndex != selected_device.mPresentQueueIndex? 2 : 1;
	device_create_info.pQueueCreateInfos = queue_create_info;
	device_create_info.enabledLayerCount = instance_create_info.enabledLayerCount;
	device_create_info.ppEnabledLayerNames = instance_create_info.ppEnabledLayerNames;
	device_create_info.enabledExtensionCount = uint32(required_device_extensions.size());
	device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
	device_create_info.pEnabledFeatures = &device_features;
	FatalErrorIfFailed(vkCreateDevice(selected_device.mPhysicalDevice, &device_create_info, nullptr, &mDevice));

	// Get the queues
	mGraphicsQueueIndex = selected_device.mGraphicsQueueIndex;
	mPresentQueueIndex = selected_device.mPresentQueueIndex;
	vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, mPresentQueueIndex, 0, &mPresentQueue);

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = selected_device.mGraphicsQueueIndex;
	FatalErrorIfFailed(vkCreateCommandPool(mDevice, &pool_info, nullptr, &mCommandPool));

	VkCommandBufferAllocateInfo command_buffer_info = {};
	command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_info.commandPool = mCommandPool;
	command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_info.commandBufferCount = 1;
	for (uint32 i = 0; i < cFrameCount; ++i)
		FatalErrorIfFailed(vkAllocateCommandBuffers(mDevice, &command_buffer_info, &mCommandBuffers[i]));

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (uint32 i = 0; i < cFrameCount; ++i)
		FatalErrorIfFailed(vkCreateFence(mDevice, &fence_info, nullptr, &mInFlightFences[i]));

	// Create constant buffer. One per frame to avoid overwriting the constant buffer while the GPU is still using it.
	for (uint n = 0; n < cFrameCount; ++n)
	{
		mVertexShaderConstantBufferProjection[n] = CreateConstantBuffer(sizeof(VertexShaderConstantBuffer));
		mVertexShaderConstantBufferOrtho[n] = CreateConstantBuffer(sizeof(VertexShaderConstantBuffer));
		mPixelShaderConstantBuffer[n] = CreateConstantBuffer(sizeof(PixelShaderConstantBuffer));
	}

	// Create descriptor set layout for the uniform buffers
	VkDescriptorSetLayoutBinding ubo_layout_binding[2] = {};
	ubo_layout_binding[0].binding = 0;
	ubo_layout_binding[0].descriptorCount = 1;
	ubo_layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding[1].binding = 1;
	ubo_layout_binding[1].descriptorCount = 1;
	ubo_layout_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo ubo_dsl = {};
	ubo_dsl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ubo_dsl.bindingCount = std::size(ubo_layout_binding);
	ubo_dsl.pBindings = ubo_layout_binding;
	FatalErrorIfFailed(vkCreateDescriptorSetLayout(mDevice, &ubo_dsl, nullptr, &mDescriptorSetLayoutUBO));

	// Create descriptor set layout for the texture binding
	VkDescriptorSetLayoutBinding texture_layout_binding = {};
	texture_layout_binding.binding = 0;
	texture_layout_binding.descriptorCount = 1;
	texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo texture_dsl = {};
	texture_dsl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	texture_dsl.bindingCount = 1;
	texture_dsl.pBindings = &texture_layout_binding;
	FatalErrorIfFailed(vkCreateDescriptorSetLayout(mDevice, &texture_dsl, nullptr, &mDescriptorSetLayoutTexture));

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout = {};
	VkDescriptorSetLayout layout_handles[] = { mDescriptorSetLayoutUBO, mDescriptorSetLayoutTexture };
	pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout.setLayoutCount = std::size(layout_handles);
	pipeline_layout.pSetLayouts = layout_handles;
	pipeline_layout.pushConstantRangeCount = 0;
	FatalErrorIfFailed(vkCreatePipelineLayout(mDevice, &pipeline_layout, nullptr, &mPipelineLayout));

	// Create descriptor pool
	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128 },
	};
	VkDescriptorPoolCreateInfo descriptor_info = {};
	descriptor_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_info.poolSizeCount = std::size(descriptor_pool_sizes);
	descriptor_info.pPoolSizes = descriptor_pool_sizes;
	descriptor_info.maxSets = 256;
	FatalErrorIfFailed(vkCreateDescriptorPool(mDevice, &descriptor_info, nullptr, &mDescriptorPool));

	// Allocate descriptor sets for 3d rendering
	Array<VkDescriptorSetLayout> layouts(cFrameCount, mDescriptorSetLayoutUBO);
	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
	descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_alloc_info.descriptorPool = mDescriptorPool;
	descriptor_set_alloc_info.descriptorSetCount = cFrameCount;
	descriptor_set_alloc_info.pSetLayouts = layouts.data();
	FatalErrorIfFailed(vkAllocateDescriptorSets(mDevice, &descriptor_set_alloc_info, mDescriptorSets));
	for (uint i = 0; i < cFrameCount; i++)
	{
		VkDescriptorBufferInfo vs_buffer_info = {};
		vs_buffer_info.buffer = mVertexShaderConstantBufferProjection[i]->GetBuffer();
		vs_buffer_info.range = sizeof(VertexShaderConstantBuffer);

		VkDescriptorBufferInfo ps_buffer_info = {};
		ps_buffer_info.buffer = mPixelShaderConstantBuffer[i]->GetBuffer();
		ps_buffer_info.range = sizeof(PixelShaderConstantBuffer);

		VkWriteDescriptorSet descriptor_write[2] = {};
		descriptor_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write[0].dstSet = mDescriptorSets[i];
		descriptor_write[0].dstBinding = 0;
		descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write[0].descriptorCount = 1;
		descriptor_write[0].pBufferInfo = &vs_buffer_info;
		descriptor_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write[1].dstSet = mDescriptorSets[i];
		descriptor_write[1].dstBinding = 1;
		descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write[1].descriptorCount = 1;
		descriptor_write[1].pBufferInfo = &ps_buffer_info;
		vkUpdateDescriptorSets(mDevice, 2, descriptor_write, 0, nullptr);
	}

	// Allocate descriptor sets for 2d rendering
	FatalErrorIfFailed(vkAllocateDescriptorSets(mDevice, &descriptor_set_alloc_info, mDescriptorSetsOrtho));
	for (uint i = 0; i < cFrameCount; i++)
	{
		VkDescriptorBufferInfo vs_buffer_info = {};
		vs_buffer_info.buffer = mVertexShaderConstantBufferOrtho[i]->GetBuffer();
		vs_buffer_info.range = sizeof(VertexShaderConstantBuffer);

		VkWriteDescriptorSet descriptor_write = {};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = mDescriptorSetsOrtho[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &vs_buffer_info;
		vkUpdateDescriptorSets(mDevice, 1, &descriptor_write, 0, nullptr);
	}

	// Create regular texture sampler
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = VK_LOD_CLAMP_NONE;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	FatalErrorIfFailed(vkCreateSampler(mDevice, &sampler_info, nullptr, &mTextureSamplerRepeat));

	// Create sampler for shadow maps
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	FatalErrorIfFailed(vkCreateSampler(mDevice, &sampler_info, nullptr, &mTextureSamplerShadow));

	{
		// Create shadow render pass
		VkAttachmentDescription shadowmap_attachment = {};
		shadowmap_attachment.format = FindDepthFormat();
		shadowmap_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		shadowmap_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		shadowmap_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		shadowmap_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		shadowmap_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		shadowmap_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		shadowmap_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkAttachmentReference shadowmap_attachment_ref = {};
		shadowmap_attachment_ref.attachment = 0;
		shadowmap_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass_shadow = {};
		subpass_shadow.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_shadow.pDepthStencilAttachment = &shadowmap_attachment_ref;
		VkSubpassDependency dependencies_shadow = {};
		dependencies_shadow.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies_shadow.dstSubpass = 0;
		dependencies_shadow.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies_shadow.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies_shadow.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies_shadow.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo render_pass_shadow = {};
		render_pass_shadow.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_shadow.attachmentCount = 1;
		render_pass_shadow.pAttachments = &shadowmap_attachment;
		render_pass_shadow.subpassCount = 1;
		render_pass_shadow.pSubpasses = &subpass_shadow;
		render_pass_shadow.dependencyCount = 1;
		render_pass_shadow.pDependencies = &dependencies_shadow;
		FatalErrorIfFailed(vkCreateRenderPass(mDevice, &render_pass_shadow, nullptr, &mRenderPassShadow));
	}

	// Create depth only texture (no color buffer, as seen from light)
	mShadowMap = new TextureVK(this, cShadowMapSize, cShadowMapSize);

	// Create frame buffer for the shadow pass
	VkImageView attachments[] = { mShadowMap->GetImageView() };
	VkFramebufferCreateInfo frame_buffer_info = {};
	frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frame_buffer_info.renderPass = mRenderPassShadow;
	frame_buffer_info.attachmentCount = std::size(attachments);
	frame_buffer_info.pAttachments = attachments;
	frame_buffer_info.width = cShadowMapSize;
	frame_buffer_info.height = cShadowMapSize;
	frame_buffer_info.layers = 1;
	FatalErrorIfFailed(vkCreateFramebuffer(mDevice, &frame_buffer_info, nullptr, &mShadowFrameBuffer));

	{
		// Create normal render pass
		VkAttachmentDescription attachments_normal[2] = {};
		VkAttachmentDescription &color_attachment = attachments_normal[0];
		color_attachment.format = selected_device.mFormat.format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentDescription &depth_attachment = attachments_normal[1];
		depth_attachment.format = FindDepthFormat();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depth_attachment_ref = {};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass_normal = {};
		subpass_normal.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_normal.colorAttachmentCount = 1;
		subpass_normal.pColorAttachments = &color_attachment_ref;
		subpass_normal.pDepthStencilAttachment = &depth_attachment_ref;
		VkSubpassDependency dependencies_normal = {};
		dependencies_normal.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies_normal.dstSubpass = 0;
		dependencies_normal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies_normal.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies_normal.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies_normal.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		VkRenderPassCreateInfo render_pass_normal = {};
		render_pass_normal.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_normal.attachmentCount = std::size(attachments_normal);
		render_pass_normal.pAttachments = attachments_normal;
		render_pass_normal.subpassCount = 1;
		render_pass_normal.pSubpasses = &subpass_normal;
		render_pass_normal.dependencyCount = 1;
		render_pass_normal.pDependencies = &dependencies_normal;
		FatalErrorIfFailed(vkCreateRenderPass(mDevice, &render_pass_normal, nullptr, &mRenderPass));
	}

	// Create the swap chain
	CreateSwapChain(mPhysicalDevice);
}

VkSurfaceFormatKHR RendererVK::SelectFormat(VkPhysicalDevice inDevice)
{
	uint32 format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(inDevice, mSurface, &format_count, nullptr);
	if (format_count == 0)
		return { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	Array<VkSurfaceFormatKHR> formats;
	formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(inDevice, mSurface, &format_count, formats.data());

	// Select BGRA8 UNORM format if available, otherwise the 1st format
	for (const VkSurfaceFormatKHR &format : formats)
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	return formats[0];
}

VkFormat RendererVK::FindDepthFormat()
{
	VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

		if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			return format;
	}

	FatalError("Failed to find format!");
}

void RendererVK::CreateSwapChain(VkPhysicalDevice inDevice)
{
	// Select the format
	VkSurfaceFormatKHR format = SelectFormat(inDevice);
	mSwapChainImageFormat = format.format;

	// Determine swap chain extent
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(inDevice, mSurface, &capabilities);
	mSwapChainExtent = capabilities.currentExtent;
	if (mSwapChainExtent.width == UINT32_MAX || mSwapChainExtent.height == UINT32_MAX)
		mSwapChainExtent = { uint32(mWindow->GetWindowWidth()), uint32(mWindow->GetWindowHeight()) };
	mSwapChainExtent.width = Clamp(mSwapChainExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	mSwapChainExtent.height = Clamp(mSwapChainExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	Trace("VK: Create swap chain %ux%u", mSwapChainExtent.width, mSwapChainExtent.height);

	// Early out if our window has been minimized
	if (mSwapChainExtent.width == 0 || mSwapChainExtent.height == 0)
		return;

	// Create the swap chain
	uint32 desired_image_count = max(min(cFrameCount, capabilities.maxImageCount), capabilities.minImageCount);
	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = mSurface;
	swapchain_create_info.minImageCount = desired_image_count;
	swapchain_create_info.imageFormat = format.format;
	swapchain_create_info.imageColorSpace = format.colorSpace;
	swapchain_create_info.imageExtent = mSwapChainExtent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32 queue_family_indices[] = { mGraphicsQueueIndex, mPresentQueueIndex };
	if (mGraphicsQueueIndex != mPresentQueueIndex)
	{
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else
	{
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchain_create_info.clipped = VK_TRUE;
	FatalErrorIfFailed(vkCreateSwapchainKHR(mDevice, &swapchain_create_info, nullptr, &mSwapChain));

	// Get the actual swap chain image count
	uint32 image_count;
	FatalErrorIfFailed(vkGetSwapchainImagesKHR(mDevice, mSwapChain, &image_count, nullptr));

	// Get the swap chain images
	mSwapChainImages.resize(image_count);
	FatalErrorIfFailed(vkGetSwapchainImagesKHR(mDevice, mSwapChain, &image_count, mSwapChainImages.data()));

	// Create image views
	mSwapChainImageViews.resize(image_count);
	for (uint32 i = 0; i < image_count; ++i)
		mSwapChainImageViews[i] = CreateImageView(mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	// Create depth buffer
	VkFormat depth_format = FindDepthFormat();
	VkImageUsageFlags depth_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	VkMemoryPropertyFlags depth_memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// Test and utilize support for transient memory for the depth buffer
	VkImageFormatProperties depth_transient_properties = {};
	VkResult depth_transient_support = vkGetPhysicalDeviceImageFormatProperties(mPhysicalDevice, depth_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, depth_usage | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, 0, &depth_transient_properties);
	if (depth_transient_support == VK_SUCCESS)
	{
		depth_usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

		// Test and utilize lazily allocated memory for the depth buffer
		for (size_t i = 0; i < mMemoryProperties.memoryTypeCount; i++)
			if (mMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			{
				depth_memory_properties = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
				break;
			}
	}

	CreateImage(mSwapChainExtent.width, mSwapChainExtent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, depth_usage, depth_memory_properties, mDepthImage, mDepthImageMemory);
	mDepthImageView = CreateImageView(mDepthImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

	// Create frame buffers for the normal pass
	mSwapChainFramebuffers.resize(image_count);
	for (size_t i = 0; i < mSwapChainFramebuffers.size(); i++)
	{
		VkImageView attachments[] = { mSwapChainImageViews[i], mDepthImageView };
		VkFramebufferCreateInfo frame_buffer_info = {};
		frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_info.renderPass = mRenderPass;
		frame_buffer_info.attachmentCount = std::size(attachments);
		frame_buffer_info.pAttachments = attachments;
		frame_buffer_info.width = mSwapChainExtent.width;
		frame_buffer_info.height = mSwapChainExtent.height;
		frame_buffer_info.layers = 1;
		FatalErrorIfFailed(vkCreateFramebuffer(mDevice, &frame_buffer_info, nullptr, &mSwapChainFramebuffers[i]));
	}

	// Allocate space to remember the image available semaphores
	mImageAvailableSemaphores.resize(image_count, VK_NULL_HANDLE);

	// Allocate the render finished semaphores
	mRenderFinishedSemaphores.resize(image_count, VK_NULL_HANDLE);
	for (uint32 i = 0; i < image_count; ++i)
		mRenderFinishedSemaphores[i] = AllocateSemaphore();
}

void RendererVK::DestroySwapChain()
{
	// Destroy semaphores
	for (VkSemaphore semaphore : mImageAvailableSemaphores) 
		vkDestroySemaphore(mDevice, semaphore, nullptr);
	mImageAvailableSemaphores.clear();

	for (VkSemaphore semaphore : mRenderFinishedSemaphores) 
		vkDestroySemaphore(mDevice, semaphore, nullptr);
	mRenderFinishedSemaphores.clear();

	for (VkSemaphore semaphore : mAvailableSemaphores) 
		vkDestroySemaphore(mDevice, semaphore, nullptr);
	mAvailableSemaphores.clear();

	// Destroy depth buffer
	if (mDepthImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(mDevice, mDepthImageView, nullptr);
		mDepthImageView = VK_NULL_HANDLE;

		DestroyImage(mDepthImage, mDepthImageMemory);
		mDepthImage = VK_NULL_HANDLE;
		mDepthImageMemory = VK_NULL_HANDLE;
	}

	for (VkFramebuffer frame_buffer : mSwapChainFramebuffers)
		vkDestroyFramebuffer(mDevice, frame_buffer, nullptr);
	mSwapChainFramebuffers.clear();

	for (VkImageView view : mSwapChainImageViews)
		vkDestroyImageView(mDevice, view, nullptr);
	mSwapChainImageViews.clear();

	mSwapChainImages.clear();

	if (mSwapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
		mSwapChain = VK_NULL_HANDLE;
	}
}

void RendererVK::OnWindowResize()
{
	vkDeviceWaitIdle(mDevice);
	DestroySwapChain();
	CreateSwapChain(mPhysicalDevice);
}

VkSemaphore RendererVK::AllocateSemaphore()
{
	VkSemaphore semaphore;

	if (mAvailableSemaphores.empty())
	{
		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		FatalErrorIfFailed(vkCreateSemaphore(mDevice, &semaphore_info, nullptr, &semaphore));
	}
	else
	{
		semaphore = mAvailableSemaphores.back();
		mAvailableSemaphores.pop_back();
	}

	return semaphore;
}

void RendererVK::FreeSemaphore(VkSemaphore inSemaphore)
{
	if (inSemaphore != VK_NULL_HANDLE)
		mAvailableSemaphores.push_back(inSemaphore);
}

bool RendererVK::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	JPH_PROFILE_FUNCTION();

	Renderer::BeginFrame(inCamera, inWorldScale);

	// If we have no swap chain, bail out
	if (mSwapChain == VK_NULL_HANDLE)
	{
		Renderer::EndFrame();
		return false;
	}

	// Update frame index
	mFrameIndex = (mFrameIndex + 1) % cFrameCount;

	// Wait for this frame to complete
	vkWaitForFences(mDevice, 1, &mInFlightFences[mFrameIndex], VK_TRUE, UINT64_MAX);

	VkSemaphore semaphore = AllocateSemaphore();
	VkResult result = mSubOptimalSwapChain? VK_ERROR_OUT_OF_DATE_KHR : vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &mImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkDeviceWaitIdle(mDevice);
		DestroySwapChain();
		CreateSwapChain(mPhysicalDevice);
		if (mSwapChain == VK_NULL_HANDLE)
		{
			FreeSemaphore(semaphore);
			Renderer::EndFrame();
			return false;
		}
		result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &mImageIndex);
		mSubOptimalSwapChain = false;
	}
	else if (result == VK_SUBOPTIMAL_KHR)
	{
		// Render this frame with the suboptimal swap chain as we've already acquired an image
		mSubOptimalSwapChain = true;
		result = VK_SUCCESS;
	}
	FatalErrorIfFailed(result);

	// The previous semaphore is now no longer in use, associate the new semaphore with the image
	FreeSemaphore(mImageAvailableSemaphores[mImageIndex]);
	mImageAvailableSemaphores[mImageIndex] = semaphore;

	// Free buffers that weren't used this frame
	for (BufferCache::value_type &vt : mBufferCache)
		for (BufferVK &bvk : vt.second)
			FreeBufferInternal(bvk);
	mBufferCache.clear();

	// Recycle the buffers that were freed
	mBufferCache.swap(mFreedBuffers[mFrameIndex]);
	
	vkResetFences(mDevice, 1, &mInFlightFences[mFrameIndex]);

	VkCommandBuffer command_buffer = GetCommandBuffer();
	FatalErrorIfFailed(vkResetCommandBuffer(command_buffer, 0));

	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	FatalErrorIfFailed(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

	// Begin the shadow pass
	VkClearValue clear_value;
	clear_value.depthStencil = { 0.0f, 0 };
	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = mRenderPassShadow;
	render_pass_begin_info.framebuffer = mShadowFrameBuffer;
	render_pass_begin_info.renderArea.extent = { cShadowMapSize, cShadowMapSize };
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = &clear_value;
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	// Set constants for vertex shader in projection mode
	VertexShaderConstantBuffer *vs = mVertexShaderConstantBufferProjection[mFrameIndex]->Map<VertexShaderConstantBuffer>();
	*vs = mVSBuffer;
	mVertexShaderConstantBufferProjection[mFrameIndex]->Unmap();

	// Set constants for vertex shader in ortho mode
	vs = mVertexShaderConstantBufferOrtho[mFrameIndex]->Map<VertexShaderConstantBuffer>();
	*vs = mVSBufferOrtho;
	mVertexShaderConstantBufferOrtho[mFrameIndex]->Unmap();

	// Set constants for pixel shader
	PixelShaderConstantBuffer *ps = mPixelShaderConstantBuffer[mFrameIndex]->Map<PixelShaderConstantBuffer>();
	*ps = mPSBuffer;
	mPixelShaderConstantBuffer[mFrameIndex]->Unmap();

	// Set the view port and scissor rect to the shadow map size
	UpdateViewPortAndScissorRect(cShadowMapSize, cShadowMapSize);

	// Switch to 3d projection mode
	SetProjectionMode();

	return true;
}

void RendererVK::EndShadowPass()
{
	VkCommandBuffer command_buffer = GetCommandBuffer();

	// End the shadow pass
	vkCmdEndRenderPass(command_buffer);

	// Begin the normal render pass
	VkClearValue clear_values[2];
	clear_values[0].color = {{ 0.098f, 0.098f, 0.439f, 1.000f }};
	clear_values[1].depthStencil = { 0.0f, 0 }; // Reverse-Z clears to 0
	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = mRenderPass;
	JPH_ASSERT(mImageIndex < mSwapChainFramebuffers.size());
	render_pass_begin_info.framebuffer = mSwapChainFramebuffers[mImageIndex];
	render_pass_begin_info.renderArea.extent = mSwapChainExtent;
	render_pass_begin_info.clearValueCount = std::size(clear_values);
	render_pass_begin_info.pClearValues = clear_values;
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	// Set the view port and scissor rect to the screen size
	UpdateViewPortAndScissorRect(mSwapChainExtent.width, mSwapChainExtent.height);
}

void RendererVK::EndFrame()
{
	JPH_PROFILE_FUNCTION();

	VkCommandBuffer command_buffer = GetCommandBuffer();
	vkCmdEndRenderPass(command_buffer);

	FatalErrorIfFailed(vkEndCommandBuffer(command_buffer));

	VkSemaphore wait_semaphores[] = { mImageAvailableSemaphores[mImageIndex] };
	VkSemaphore signal_semaphores[] = { mRenderFinishedSemaphores[mImageIndex] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;
	FatalErrorIfFailed(vkQueueSubmit(mGraphicsQueue, 1, &submit_info, mInFlightFences[mFrameIndex]));

	VkSwapchainKHR swap_chains[] = { mSwapChain };
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &mImageIndex;
	vkQueuePresentKHR(mPresentQueue, &present_info);

	Renderer::EndFrame();
}

void RendererVK::SetProjectionMode()
{
	JPH_ASSERT(mInFrame);

	// Bind descriptor set for 3d rendering
	vkCmdBindDescriptorSets(GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[mFrameIndex], 0, nullptr);
}

void RendererVK::SetOrthoMode()
{
	JPH_ASSERT(mInFrame);

	// Bind descriptor set for 2d rendering
	vkCmdBindDescriptorSets(GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSetsOrtho[mFrameIndex], 0, nullptr);
}

Ref<Texture> RendererVK::CreateTexture(const Surface *inSurface)
{
	return new TextureVK(this, inSurface);
}

Ref<VertexShader> RendererVK::CreateVertexShader(const char *inName)
{
	Array<uint8> data = ReadData((String("Shaders/VK/") + inName + ".vert.spv").c_str());

	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = data.size();
	create_info.pCode = reinterpret_cast<const uint32 *>(data.data());
	VkShaderModule shader_module;
	FatalErrorIfFailed(vkCreateShaderModule(mDevice, &create_info, nullptr, &shader_module));

	return new VertexShaderVK(mDevice, shader_module);
}

Ref<PixelShader> RendererVK::CreatePixelShader(const char *inName)
{
	Array<uint8> data = ReadData((String("Shaders/VK/") + inName + ".frag.spv").c_str());

	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = data.size();
	create_info.pCode = reinterpret_cast<const uint32 *>(data.data());
	VkShaderModule shader_module;
	FatalErrorIfFailed(vkCreateShaderModule(mDevice, &create_info, nullptr, &shader_module));

	return new PixelShaderVK(mDevice, shader_module);
}

unique_ptr<PipelineState> RendererVK::CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode)
{
	return make_unique<PipelineStateVK>(this, static_cast<const VertexShaderVK *>(inVertexShader), inInputDescription, inInputDescriptionCount, static_cast<const PixelShaderVK *>(inPixelShader), inDrawPass, inFillMode, inTopology, inDepthTest, inBlendMode, inCullMode);
}

RenderPrimitive *RendererVK::CreateRenderPrimitive(PipelineState::ETopology inType)
{
	return new RenderPrimitiveVK(this);
}

RenderInstances *RendererVK::CreateRenderInstances()
{
	return new RenderInstancesVK(this);
}

uint32 RendererVK::FindMemoryType(uint32 inTypeFilter, VkMemoryPropertyFlags inProperties)
{
	for (uint32 i = 0; i < mMemoryProperties.memoryTypeCount; i++)
		if ((inTypeFilter & (1 << i))
			&& (mMemoryProperties.memoryTypes[i].propertyFlags & inProperties) == inProperties)
			return i;

	FatalError("Failed to find memory type!");
}

void RendererVK::AllocateMemory(VkDeviceSize inSize, uint32 inMemoryTypeBits, VkMemoryPropertyFlags inProperties, VkDeviceMemory &outMemory)
{
	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = inSize;
	alloc_info.memoryTypeIndex = FindMemoryType(inMemoryTypeBits, inProperties);
	FatalErrorIfFailed(vkAllocateMemory(mDevice, &alloc_info, nullptr, &outMemory));

	// Track allocation
	++mNumAllocations;
	mTotalAllocated += inSize;

	// Track max usage
	mMaxTotalAllocated = max(mMaxTotalAllocated, mTotalAllocated);
	mMaxNumAllocations = max(mMaxNumAllocations, mNumAllocations);
}

void RendererVK::FreeMemory(VkDeviceMemory inMemory, VkDeviceSize inSize)
{
	vkFreeMemory(mDevice, inMemory, nullptr);

	// Track free
	--mNumAllocations;
	mTotalAllocated -= inSize;
}

void RendererVK::CreateBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties, BufferVK &outBuffer)
{
	// Check the cache
	BufferCache::iterator i = mBufferCache.find({ inSize, inUsage, inProperties });
	if (i != mBufferCache.end() && !i->second.empty())
	{
		outBuffer = i->second.back();
		i->second.pop_back();
		return;
	}

	// Create a new buffer
	outBuffer.mSize = inSize;
	outBuffer.mUsage = inUsage;
	outBuffer.mProperties = inProperties;

	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = inSize;
	create_info.usage = inUsage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	FatalErrorIfFailed(vkCreateBuffer(mDevice, &create_info, nullptr, &outBuffer.mBuffer));

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(mDevice, outBuffer.mBuffer, &mem_requirements);

	if (mem_requirements.size > cMaxAllocSize)
	{
		// Allocate block directly
		AllocateMemory(mem_requirements.size, mem_requirements.memoryTypeBits, inProperties, outBuffer.mMemory);
		outBuffer.mAllocatedSize = mem_requirements.size;
		outBuffer.mOffset = 0;
	}
	else
	{
		// Round allocation to the next power of 2 so that we can use a simple block based allocator
		outBuffer.mAllocatedSize = max(VkDeviceSize(GetNextPowerOf2(uint32(mem_requirements.size))), cMinAllocSize);

		// Ensure that we have memory available from the right pool
		Array<Memory> &mem_array = mMemoryCache[{ outBuffer.mAllocatedSize, outBuffer.mUsage, outBuffer.mProperties }];
		if (mem_array.empty())
		{
			// Allocate a bigger block
			VkDeviceMemory device_memory;
			AllocateMemory(cBlockSize, mem_requirements.memoryTypeBits, inProperties, device_memory);

			// Divide into sub blocks
			for (VkDeviceSize offset = 0; offset < cBlockSize; offset += outBuffer.mAllocatedSize)
				mem_array.push_back({ device_memory, offset });
		}

		// Claim memory from the pool
		Memory &memory = mem_array.back();
		outBuffer.mMemory = memory.mMemory;
		outBuffer.mOffset = memory.mOffset;
		mem_array.pop_back();
	}

	// Bind the memory to the buffer
	vkBindBufferMemory(mDevice, outBuffer.mBuffer, outBuffer.mMemory, outBuffer.mOffset);
}

VkCommandBuffer RendererVK::StartTempCommandBuffer()
{
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = mCommandPool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(mDevice, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;
}

void RendererVK::EndTempCommandBuffer(VkCommandBuffer inCommandBuffer)
{
	vkEndCommandBuffer(inCommandBuffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &inCommandBuffer;

	vkQueueSubmit(mGraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(mGraphicsQueue); // Inefficient, but we only use this during initialization

	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &inCommandBuffer);
}

void RendererVK::CopyBuffer(VkBuffer inSrc, VkBuffer inDst, VkDeviceSize inSize)
{
	VkCommandBuffer command_buffer = StartTempCommandBuffer();

	VkBufferCopy region = {};
	region.size = inSize;
	vkCmdCopyBuffer(command_buffer, inSrc, inDst, 1, &region);

	EndTempCommandBuffer(command_buffer);
}

void RendererVK::CreateDeviceLocalBuffer(const void *inData, VkDeviceSize inSize, VkBufferUsageFlags inUsage, BufferVK &outBuffer)
{
	BufferVK staging_buffer;
	CreateBuffer(inSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer);

	void *data;
	vkMapMemory(mDevice, staging_buffer.mMemory, staging_buffer.mOffset, inSize, 0, &data);
	memcpy(data, inData, (size_t)inSize);
	vkUnmapMemory(mDevice, staging_buffer.mMemory);

	CreateBuffer(inSize, inUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outBuffer);

	CopyBuffer(staging_buffer.mBuffer, outBuffer.mBuffer, inSize);

	FreeBuffer(staging_buffer);
}

void RendererVK::FreeBuffer(BufferVK &ioBuffer)
{
	if (ioBuffer.mBuffer != VK_NULL_HANDLE)
	{
		JPH_ASSERT(mFrameIndex < cFrameCount);
		mFreedBuffers[mFrameIndex][{ ioBuffer.mSize, ioBuffer.mUsage, ioBuffer.mProperties }].push_back(ioBuffer);
	}
}

void RendererVK::FreeBufferInternal(BufferVK &ioBuffer)
{
	// Destroy the buffer
	vkDestroyBuffer(mDevice, ioBuffer.mBuffer, nullptr);
	ioBuffer.mBuffer = VK_NULL_HANDLE;

	if (ioBuffer.mAllocatedSize > cMaxAllocSize)
		FreeMemory(ioBuffer.mMemory, ioBuffer.mAllocatedSize);
	else
		mMemoryCache[{ ioBuffer.mAllocatedSize, ioBuffer.mUsage, ioBuffer.mProperties }].push_back({ ioBuffer.mMemory, ioBuffer.mOffset });
	ioBuffer.mMemory = VK_NULL_HANDLE;
}

unique_ptr<ConstantBufferVK> RendererVK::CreateConstantBuffer(VkDeviceSize inBufferSize)
{
	return make_unique<ConstantBufferVK>(this, inBufferSize);
}

VkImageView RendererVK::CreateImageView(VkImage inImage, VkFormat inFormat, VkImageAspectFlags inAspectFlags)
{
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = inImage;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = inFormat;
	view_info.subresourceRange.aspectMask = inAspectFlags;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.layerCount = 1;

	VkImageView image_view;
	FatalErrorIfFailed(vkCreateImageView(mDevice, &view_info, nullptr, &image_view));

	return image_view;
}

void RendererVK::CreateImage(uint32 inWidth, uint32 inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkImage &outImage, VkDeviceMemory &outMemory)
{
	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = inWidth;
	image_info.extent.height = inHeight;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = inFormat;
	image_info.tiling = inTiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = inUsage;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	FatalErrorIfFailed(vkCreateImage(mDevice, &image_info, nullptr, &outImage));

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(mDevice, outImage, &mem_requirements);

	AllocateMemory(mem_requirements.size, mem_requirements.memoryTypeBits, inProperties, outMemory);

	vkBindImageMemory(mDevice, outImage, outMemory, 0);
}

void RendererVK::DestroyImage(VkImage inImage, VkDeviceMemory inMemory)
{
	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(mDevice, inImage, &mem_requirements);

	vkDestroyImage(mDevice, inImage, nullptr);

	FreeMemory(inMemory, mem_requirements.size);
}

void RendererVK::UpdateViewPortAndScissorRect(uint32 inWidth, uint32 inHeight)
{
	VkCommandBuffer command_buffer = GetCommandBuffer();

	// Update the view port rect
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)inWidth;
	viewport.height = (float)inHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	// Update the scissor rect
	VkRect2D scissor = {};
	scissor.extent = { inWidth, inHeight };
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

#ifdef JPH_ENABLE_VULKAN
Renderer *Renderer::sCreate()
{
	return new RendererVK;
}
#endif
