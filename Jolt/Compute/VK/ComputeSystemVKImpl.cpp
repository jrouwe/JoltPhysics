// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeSystemVKImpl.h>
#include <Jolt/Core/QuickSort.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemVKImpl)
{
	JPH_ADD_BASE_CLASS(ComputeSystemVKImpl, ComputeSystemVKWithAllocator)
}

#ifdef JPH_DEBUG

static VKAPI_ATTR VkBool32 VKAPI_CALL sVulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT inSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT inType, const VkDebugUtilsMessengerCallbackDataEXT *inCallbackData, [[maybe_unused]] void *inUserData)
{
	if (inSeverity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
		Trace("VK: %s", inCallbackData->pMessage);
	JPH_ASSERT((inSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) == 0);
	return VK_FALSE;
}

#endif // JPH_DEBUG

ComputeSystemVKImpl::~ComputeSystemVKImpl()
{
	ComputeSystemVK::Shutdown();

	if (mDevice != VK_NULL_HANDLE)
		vkDestroyDevice(mDevice, nullptr);

#ifdef JPH_DEBUG
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)(void *)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (mInstance != VK_NULL_HANDLE && mDebugMessenger != VK_NULL_HANDLE && vkDestroyDebugUtilsMessengerEXT != nullptr)
		vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif

	if (mInstance != VK_NULL_HANDLE)
		vkDestroyInstance(mInstance, nullptr);
}

bool ComputeSystemVKImpl::Initialize(ComputeSystemResult &outResult)
{
	// Required instance extensions
	Array<const char *> required_instance_extensions;
	required_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#ifdef JPH_PLATFORM_MACOS
	required_instance_extensions.push_back("VK_KHR_portability_enumeration");
	required_instance_extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif
	GetInstanceExtensions(required_instance_extensions);

	// Required device extensions
	Array<const char *> required_device_extensions;
	required_device_extensions.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
#ifdef JPH_PLATFORM_MACOS
	required_device_extensions.push_back("VK_KHR_portability_subset"); // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#endif
	GetDeviceExtensions(required_device_extensions);

	// Query supported instance extensions
	uint32 instance_extension_count = 0;
	if (VKFailed(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr), outResult))
		return false;
	Array<VkExtensionProperties> instance_extensions;
	instance_extensions.resize(instance_extension_count);
	if (VKFailed(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()), outResult))
		return false;

	// Query supported validation layers
	uint32 validation_layer_count;
	vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
	Array<VkLayerProperties> validation_layers(validation_layer_count);
	vkEnumerateInstanceLayerProperties(&validation_layer_count, validation_layers.data());

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_API_VERSION_1_1;

	// Create Vulkan instance
	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef JPH_PLATFORM_MACOS
	instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
	instance_create_info.pApplicationInfo = &app_info;

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
	if (VKFailed(vkCreateInstance(&instance_create_info, nullptr, &mInstance), outResult))
		return false;

#ifdef JPH_DEBUG
	// Finalize debug messenger callback
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)(std::uintptr_t)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
	if (vkCreateDebugUtilsMessengerEXT != nullptr)
		if (VKFailed(vkCreateDebugUtilsMessengerEXT(mInstance, &messenger_create_info, nullptr, &mDebugMessenger), outResult))
			return false;
#endif

	// Notify that instance has been created
	OnInstanceCreated();

	// Select device
	uint32 device_count = 0;
	if (VKFailed(vkEnumeratePhysicalDevices(mInstance, &device_count, nullptr), outResult))
		return false;
	Array<VkPhysicalDevice> devices;
	devices.resize(device_count);
	if (VKFailed(vkEnumeratePhysicalDevices(mInstance, &device_count, devices.data()), outResult))
		return false;
	struct Device
	{
		VkPhysicalDevice		mPhysicalDevice;
		String					mName;
		VkSurfaceFormatKHR		mFormat;
		uint32					mGraphicsQueueIndex;
		uint32					mPresentQueueIndex;
		uint32					mComputeQueueIndex;
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
		uint32 compute_queue = ~uint32(0);
		for (uint32 i = 0; i < uint32(queue_families.size()); ++i)
		{
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphics_queue = i;

				if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
					compute_queue = i;
			}

			if (HasPresentSupport(device, i))
				present_queue = i;

			if (graphics_queue != ~uint32(0) && present_queue != ~uint32(0) && compute_queue != ~uint32(0))
				break;
		}
		if (graphics_queue == ~uint32(0) || present_queue == ~uint32(0) || compute_queue == ~uint32(0))
			continue;

		// Select surface format
		VkSurfaceFormatKHR selected_format = SelectFormat(device);
		if (selected_format.format == VK_FORMAT_UNDEFINED)
			continue;

		// Add the device
		available_devices.push_back({ device, properties.deviceName, selected_format, graphics_queue, present_queue, compute_queue, score });
	}
	if (available_devices.empty())
	{
		outResult.SetError("No suitable Vulkan device found");
		return false;
	}

	// Sort the devices by score
	QuickSort(available_devices.begin(), available_devices.end(), [](const Device &inLHS, const Device &inRHS) {
		return inLHS.mScore > inRHS.mScore;
	});
	const Device &selected_device = available_devices[0];

	// Create device
	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo queue_create_info[3] = {};
	for (VkDeviceQueueCreateInfo &q : queue_create_info)
	{
		q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		q.queueCount = 1;
		q.pQueuePriorities = &queue_priority;
	}
	uint32 num_queues = 0;
	queue_create_info[num_queues++].queueFamilyIndex = selected_device.mGraphicsQueueIndex;
	bool found = false;
	for (uint32 i = 0; i < num_queues; ++i)
		if (queue_create_info[i].queueFamilyIndex == selected_device.mPresentQueueIndex)
		{
			found = true;
			break;
		}
	if (!found)
		queue_create_info[num_queues++].queueFamilyIndex = selected_device.mPresentQueueIndex;
	found = false;
	for (uint32 i = 0; i < num_queues; ++i)
		if (queue_create_info[i].queueFamilyIndex == selected_device.mComputeQueueIndex)
		{
			found = true;
			break;
		}
	if (!found)
		queue_create_info[num_queues++].queueFamilyIndex = selected_device.mComputeQueueIndex;

	VkPhysicalDeviceScalarBlockLayoutFeatures enable_scalar_block = {};
	enable_scalar_block.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
	enable_scalar_block.scalarBlockLayout = VK_TRUE;

	VkPhysicalDeviceFeatures2 enabled_features2 = {};
	enabled_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	GetEnabledFeatures(enabled_features2);
	enable_scalar_block.pNext = enabled_features2.pNext;
	enabled_features2.pNext = &enable_scalar_block;

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = num_queues;
	device_create_info.pQueueCreateInfos = queue_create_info;
	device_create_info.enabledLayerCount = instance_create_info.enabledLayerCount;
	device_create_info.ppEnabledLayerNames = instance_create_info.ppEnabledLayerNames;
	device_create_info.enabledExtensionCount = uint32(required_device_extensions.size());
	device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
	device_create_info.pNext = &enabled_features2;
	device_create_info.pEnabledFeatures = nullptr;

	VkDevice device = VK_NULL_HANDLE;
	if (VKFailed(vkCreateDevice(selected_device.mPhysicalDevice, &device_create_info, nullptr, &device), outResult))
		return false;

	// Get the queues
	mGraphicsQueueIndex = selected_device.mGraphicsQueueIndex;
	mPresentQueueIndex = selected_device.mPresentQueueIndex;
	vkGetDeviceQueue(device, mGraphicsQueueIndex, 0, &mGraphicsQueue);
	vkGetDeviceQueue(device, mPresentQueueIndex, 0, &mPresentQueue);

	// Store selected format
	mSelectedFormat = selected_device.mFormat;

	// Initialize the compute system
	return ComputeSystemVK::Initialize(selected_device.mPhysicalDevice, device, selected_device.mComputeQueueIndex, outResult);
}

ComputeSystemResult CreateComputeSystemVK()
{
	ComputeSystemResult result;

	Ref<ComputeSystemVKImpl> compute = new ComputeSystemVKImpl;
	if (!compute->Initialize(result))
		return result;

	result.Set(compute.GetPtr());
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK
