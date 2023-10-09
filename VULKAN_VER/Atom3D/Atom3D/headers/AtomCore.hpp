// ReSharper disable CppInconsistentNaming
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

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

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
#include <cassert>

namespace Atom {

typedef glm::vec<2, int, glm::defaultp> vec2I;

inline PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
inline PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

// Vulkan proxy functions.
VKAPI_ATTR inline VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(VkInstance instance, 
                                                                   const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
                                                                   const VkAllocationCallbacks* pAllocator, 
                                                                   VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

VKAPI_ATTR inline void VKAPI_CALL DestroyDebugUtilsMessengerEXT(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	return pfnVkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, pAllocator);
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	[[nodiscard]] bool isComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
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
	[[nodiscard]] bool checkDeviceExtensionSupport(vk::PhysicalDevice) const;
	[[nodiscard]] std::vector<const char*> getRequiredExtensions() const;
	bool isDeviceGucci(vk::PhysicalDevice) const;

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice) const;
	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice) const;

	static std::vector<char> readFile(const std::string&);

	vk::ShaderModule createShaderModule(const std::vector<char>&) const;

	void recordCommandBuffer(vk::CommandBuffer, uint32_t);

	// Swap Chain Config
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>&);
	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>&);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR&) const;

	// Debug stuff
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT&);
	void outputExtensionStatus(const std::vector<const char*>&) const;



	/****************MEMBERS*******************/
	GLFWwindow* mWindow;
	vk::Instance mInstance;
	vk::PhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	vk::Device mLogicalDevice;
	vk::Queue mGraphicsQueue;
	vk::Queue mPresentQueue;
	vk::SurfaceKHR mSurface;
	vk::SwapchainKHR mSwapchain;
	std::vector<vk::Image> mSwapchainImages;
	std::vector<vk::ImageView> mSwapchainImageViews;

	vk::CommandPool mCommandPool;
	vk::CommandBuffer mCommandBuffer;

	vk::Format mSwapchainImageFormat;
	vk::Extent2D mSwapchainExtent;
	std::vector<vk::Framebuffer> mSwapchainFramebuffers;

	vk::RenderPass mRenderPass;
	vk::PipelineLayout mPipelineLayout;
	vk::Pipeline mGraphicsPipeline;

	// Sync Objects
	vk::Semaphore mImageAvailableS;
	vk::Semaphore mRenderFinishedS;
	vk::Fence mInFlightF;

	vk::DebugUtilsMessengerEXT mDebugMessenger;

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