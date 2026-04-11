// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeSystemVKWithAllocator.h>

JPH_NAMESPACE_BEGIN

/// Implementation of ComputeSystemVK that fully initializes Vulkan
class JPH_EXPORT ComputeSystemVKImpl : public ComputeSystemVKWithAllocator
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_EXPORT, ComputeSystemVKImpl)

	/// Destructor
	virtual							~ComputeSystemVKImpl() override;

	/// Initialize the compute system
	bool							Initialize(ComputeSystemResult &outResult);

protected:
	/// Override to perform actions once the instance has been created
	virtual void					OnInstanceCreated()																{ /* Do nothing */ }

	/// Override to add platform specific instance extensions
	virtual void					GetInstanceExtensions(Array<const char *> &outExtensions)						{ /* Add nothing */ }

	/// Override to add platform specific device extensions
	virtual void					GetDeviceExtensions(Array<const char *> &outExtensions)							{ /* Add nothing */ }

	/// Override  to enable specific features
	virtual void					GetEnabledFeatures(VkPhysicalDeviceFeatures2 &ioFeatures)						{ /* Add nothing */ }

	/// Override to check for present support on a given device and queue family
	virtual bool					HasPresentSupport(VkPhysicalDevice inDevice, uint32 inQueueFamilyIndex)			{ return true; }

	/// Override to select the surface format
	virtual VkSurfaceFormatKHR		SelectFormat(VkPhysicalDevice inDevice)											{ return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }; }

	VkInstance						mInstance = VK_NULL_HANDLE;
#ifdef JPH_DEBUG
	VkDebugUtilsMessengerEXT		mDebugMessenger = VK_NULL_HANDLE;
	PFN_vkDestroyDebugUtilsMessengerEXT mVkDestroyDebugUtilsMessengerEXT = nullptr;
#endif
	uint32							mGraphicsQueueIndex = 0;
	uint32							mPresentQueueIndex = 0;
	VkQueue							mGraphicsQueue = VK_NULL_HANDLE;
	VkQueue							mPresentQueue = VK_NULL_HANDLE;
	VkSurfaceFormatKHR				mSelectedFormat;

	PFN_vkGetInstanceProcAddr		mVkGetInstanceProcAddr = nullptr;
	PFN_vkGetDeviceProcAddr			mVkGetDeviceProcAddr = nullptr;

	// Pre instance functions
	PFN_vkCreateInstance			mVkCreateInstance = nullptr;
	PFN_vkEnumerateInstanceExtensionProperties mVkEnumerateInstanceExtensionProperties = nullptr;
	PFN_vkEnumerateInstanceLayerProperties mVkEnumerateInstanceLayerProperties = nullptr;

	// Post instance functions
	PFN_vkCreateDevice				mVkCreateDevice = nullptr;
	PFN_vkDestroyInstance			mVkDestroyInstance = nullptr;
	PFN_vkEnumerateDeviceExtensionProperties mVkEnumerateDeviceExtensionProperties = nullptr;
	PFN_vkEnumeratePhysicalDevices	mVkEnumeratePhysicalDevices = nullptr;
	PFN_vkGetPhysicalDeviceProperties mVkGetPhysicalDeviceProperties = nullptr;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties mVkGetPhysicalDeviceQueueFamilyProperties = nullptr;
};

JPH_NAMESPACE_END

#endif // JPH_USE_VK
