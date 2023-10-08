#pragma once

#ifndef ATOM_CORE_HPP
#define ATOM_CORE_HPP

// Get rid of gross Windows macros
#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <glm/vec2.hpp>

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <optional>
#include <fstream>

namespace Atom {

typedef glm::vec<2, int, glm::defaultp> vec2I;

// Vulkan proxy functions.
inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
											const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
											const VkAllocationCallbacks* pAllocator, 
											VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	[[nodiscard]] bool isComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class AtomCore {
public:
	AtomCore();

	void init();
	void run();
	void cleanup() const;

private:
	void initWindow();
	void initVulkan();
	void createInstance();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwagChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();

	void drawFrame();

	[[nodiscard]] bool checkValidationLayerSupport() const;
	[[nodiscard]] bool checkDeviceExtensionSupport(VkPhysicalDevice) const;
	[[nodiscard]] std::vector<const char*> getRequiredExtensions() const;
	bool isDeviceGucci(VkPhysicalDevice) const;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice) const;
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice) const;

	static std::vector<char> readFile(const std::string&);

	VkShaderModule createShaderModule(const std::vector<char>&) const;

	void recordCommandBuffer(VkCommandBuffer, uint32_t);

	// Swap Chain Config
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>&);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&) const;

	// Debug stuff
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);
	void outputExtensionStatus(const std::vector<const char*>&) const;

	// members
	GLFWwindow* mWindow;
	VkInstance mInstance;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mLogicalDevice;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkSurfaceKHR mSurface;
	VkSwapchainKHR mSwapchain;
	std::vector<VkImage> mSwapchainImages;
	std::vector<VkImageView> mSwapchainImageViews;

	VkCommandPool mCommandPool;
	VkCommandBuffer mCommandBuffer;

	VkFormat mSwapchainImageFormat;
	VkExtent2D mSwapchainExtent;
	std::vector<VkFramebuffer> mSwapchainFramebuffers;

	VkRenderPass mRenderPass;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;

	// Sync Objects
	VkSemaphore mImageAvailableS;
	VkSemaphore mRenderFinishedS;
	VkFence mInFlightF;

	VkDebugUtilsMessengerEXT mDebugMessenger;

	vec2I mViewSize;
	const std::vector<const char*> mValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> mDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG // Not Debug
	const bool mEnableValidationLayers = false;
#else
	const bool mEnableValidationLayers = true;
#endif
};

}


#endif