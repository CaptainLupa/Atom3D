// ReSharper disable CppCtyleCast
// ReSharper disable CppMemberFunctionMayBeStatic
#include "AtomCore.hpp"

namespace Atom {

AtomCore::AtomCore() {
	mViewSize = { 800, 600 };
}

void AtomCore::init() {
	initWindow();
	initVulkan();
}

void AtomCore::initVulkan() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwagChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
}


void AtomCore::createInstance() {
	if (mEnableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("Validation layers requested, but no available.\n");
	
	VkApplicationInfo appInfo = {};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Atom3D";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 1);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 1);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	const auto extensions = getRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// For debug messages that occur when instance is created or destroyed.
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

	if (mEnableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

#ifdef _DEBUG
	outputExtensionStatus(extensions);
#endif

	//Now create instance.
	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create mInstance in AtomCore::createInstance,\n");
	}
}


void AtomCore::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set uniqueQueues = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	constexpr float queuePriority = 1.0f;
	for (uint32_t family: uniqueQueues) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = family;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Does nothing for now.
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

	if (mEnableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device.\n");

	vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
	vkGetDeviceQueue(mLogicalDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}


void AtomCore::createSurface() {
	if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface.\n");
}


void AtomCore::createSwagChain() {
	const auto support = querySwapChainSupport(mPhysicalDevice);

	auto surfaceFormat = chooseSwapSurfaceFormat(support.formats);
	auto presentMode = chooseSwapPresentMode(support.presentModes);
	auto extent = chooseSwapExtent(support.capabilities);

	uint32_t imageCount = support.capabilities.minImageCount + 1;

	if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount)
		imageCount = support.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
	uint32_t pIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = pIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = support.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swap chain.\n");

	vkGetSwapchainImagesKHR(mLogicalDevice, mSwapchain, &imageCount, nullptr);
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mLogicalDevice, mSwapchain, &imageCount, mSwapchainImages.data());

	mSwapchainImageFormat = surfaceFormat.format;
	mSwapchainExtent = extent;
}


void AtomCore::createImageViews() {
	mSwapchainImageViews.resize(mSwapchainImages.size());

	for (size_t i = 0; i < mSwapchainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = mSwapchainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = mSwapchainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(mLogicalDevice, &createInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image views.\n");
	}
}


void AtomCore::createRenderPass() {
	VkAttachmentDescription colorAtt = {};
	colorAtt.format = mSwapchainImageFormat;
	colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference caRef = {};
	caRef.attachment = 0;
	caRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &caRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpInfo.attachmentCount = 1;
	rpInfo.pAttachments = &colorAtt;
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = 1;
	rpInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(mLogicalDevice, &rpInfo, nullptr, &mRenderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass.\n");
}


// Beefy boy
void AtomCore::createGraphicsPipeline() {
	auto vertShader = readFile("GLSL/vert.spv");
	auto fragShader = readFile("GLSL/frag.spv");

	auto vertModule = createShaderModule(vertShader);
	auto fragModule = createShaderModule(fragShader);

	VkPipelineShaderStageCreateInfo vertStageInfo = {}, fragStageInfo = {};
	vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageInfo.module = vertModule;
	vertStageInfo.pName = "main";

	fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageInfo.module = fragModule;
	fragStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo stages[] = { vertStageInfo, fragStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAss = {};
	inputAss.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAss.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAss.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0;
	rasterizer.depthBiasClamp = 0;
	rasterizer.depthBiasSlopeFactor = 0;

	VkPipelineMultisampleStateCreateInfo msaa = {};
	msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msaa.sampleShadingEnable = VK_FALSE;
	msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Unused for now
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipeLayoutInfo = {};
	pipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeLayoutInfo.setLayoutCount = 0;
	pipeLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(mLogicalDevice, &pipeLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout.\n");

	VkGraphicsPipelineCreateInfo pipeInfo = {};
	pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeInfo.stageCount = 2;
	pipeInfo.pStages = stages;
	pipeInfo.pVertexInputState = &vertexInputInfo;
	pipeInfo.pInputAssemblyState = &inputAss;
	pipeInfo.pViewportState = &viewportState;
	pipeInfo.pRasterizationState = &rasterizer;
	pipeInfo.pMultisampleState = &msaa;
	pipeInfo.pDepthStencilState = nullptr;
	pipeInfo.pColorBlendState = &colorBlending;
	pipeInfo.pDynamicState = &dynamicState;
	pipeInfo.layout = mPipelineLayout;
	pipeInfo.renderPass = mRenderPass;
	pipeInfo.subpass = 0;
	pipeInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &pipeInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline.\n");

	vkDestroyShaderModule(mLogicalDevice, fragModule, nullptr);
	vkDestroyShaderModule(mLogicalDevice, vertModule, nullptr);
}

void AtomCore::createFramebuffers() {
	mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

	for (size_t i = 0; i < mSwapchainImageViews.size(); i++) {
		VkImageView attachments[] = {
			mSwapchainImageViews[i]
		};

		VkFramebufferCreateInfo fbInfo = {};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = mRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = attachments;
		fbInfo.width = mSwapchainExtent.width;
		fbInfo.height = mSwapchainExtent.height;
		fbInfo.layers = 1;

		if (vkCreateFramebuffer(mLogicalDevice, &fbInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create frame buffer.\n");
	}
}

void AtomCore::createCommandPool() {
	QueueFamilyIndices qfi = findQueueFamilies(mPhysicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = qfi.graphicsFamily.value();

	if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool.\n");
}

void AtomCore::createCommandBuffer() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &mCommandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command buffers.\n");
}

void AtomCore::createSyncObjects() {
	VkSemaphoreCreateInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fi = {};
	fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(mLogicalDevice, &si, nullptr, &mImageAvailableS) != VK_SUCCESS ||
		vkCreateSemaphore(mLogicalDevice, &si, nullptr, &mRenderFinishedS) != VK_SUCCESS ||
		vkCreateFence(mLogicalDevice, &fi, nullptr, &mInFlightF) != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphores/fences.\n");


}

void AtomCore::drawFrame() {
	vkWaitForFences(mLogicalDevice, 1, &mInFlightF, VK_TRUE, UINT64_MAX);
	vkResetFences(mLogicalDevice, 1, &mInFlightF);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(mLogicalDevice, mSwapchain, UINT64_MAX, mImageAvailableS, VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(mCommandBuffer, 0);

	recordCommandBuffer(mCommandBuffer, imageIndex);

	VkSubmitInfo subInfo = {};
	subInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitS[] = { mImageAvailableS };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	subInfo.waitSemaphoreCount = 1;
	subInfo.pWaitSemaphores = waitS;
	subInfo.pWaitDstStageMask = waitStages;
	subInfo.commandBufferCount = 1;
	subInfo.pCommandBuffers = &mCommandBuffer;

	VkSemaphore signalS[] = { mRenderFinishedS };
	subInfo.signalSemaphoreCount = 1;
	subInfo.pSignalSemaphores = signalS;

	if (vkQueueSubmit(mGraphicsQueue, 1, &subInfo, mInFlightF) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer.\n");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalS;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(mPresentQueue, &presentInfo);
}


void AtomCore::pickPhysicalDevice() {
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("No GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	for (const auto& device: devices) {
		if (isDeviceGucci(device)) {
			mPhysicalDevice = device;
			break;
		}
	}

	if (mPhysicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find suitable GPU.");
}


bool AtomCore::isDeviceGucci(VkPhysicalDevice device) const {
	const auto indices = findQueueFamilies(device);

	const auto extensionSupported = checkDeviceExtensionSupport(device);

	bool swapChainGucci = false;
	if (extensionSupported) {
		const auto [_, formats, presentModes] = querySwapChainSupport(device);
		swapChainGucci = !formats.empty() && !presentModes.empty();
	}

	return indices.isComplete() && extensionSupported && swapChainGucci;
}


QueueFamilyIndices AtomCore::findQueueFamilies(VkPhysicalDevice device) const {
	QueueFamilyIndices indices;

	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

	std::vector<VkQueueFamilyProperties> properties(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, properties.data());

	int i = 0;
	VkBool32 presentSupport = false;
	for (const auto& property: properties) {
		if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}


SwapChainSupportDetails AtomCore::querySwapChainSupport(VkPhysicalDevice device) const {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

	uint32_t formatCount, presentModeCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
	}

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


std::vector<char> AtomCore::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

	file.close();

	return buffer;
}


VkShaderModule AtomCore::createShaderModule(const std::vector<char>& code) const {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(mLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module.\n");

	return shaderModule;
}

void AtomCore::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer.\n");

	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.renderPass = mRenderPass;
	rpInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
	rpInfo.renderArea.offset = { 0, 0 };
	rpInfo.renderArea.extent = mSwapchainExtent;

	VkClearValue clearColor = { {{0, 0, 0, 1.0f}} };
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

	VkViewport viewport = {
		0,
		0,
		static_cast<float>(mSwapchainExtent.width),
		static_cast<float>(mSwapchainExtent.height),
		0,
		1
	};

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {
		{0, 0},
		mSwapchainExtent
	};

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer.\n");
}


VkSurfaceFormatKHR AtomCore::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& format: availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return availableFormats[0]; // Could also rank and return best, but not important yet.
}


VkPresentModeKHR AtomCore::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) {
	for (const auto& mode: availableModes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}


VkExtent2D AtomCore::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	uint32_t w, h;
	glfwGetFramebufferSize(mWindow, reinterpret_cast<int*>(&w), reinterpret_cast<int*>(&h));

	w = std::clamp(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	h = std::clamp(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return { w, h };
}


bool AtomCore::checkValidationLayerSupport() const {
	uint32_t layerCount;
	bool layerFound = false;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName: mValidationLayers) {
		for (const auto& layerProps: availableLayers) {
			if (strcmp(layerName, layerProps.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}


bool AtomCore::checkDeviceExtensionSupport(VkPhysicalDevice device) const {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

	for (const auto& e: availableExtensions)
		requiredExtensions.erase(e.extensionName);

	return requiredExtensions.empty();
}


std::vector<const char*> AtomCore::getRequiredExtensions() const {
	uint32_t glfwExtCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtCount);

	if (mEnableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}


VkBool32 AtomCore::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, 
								 VkDebugUtilsMessageTypeFlagsEXT type, 
								 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
								 void* pUserData) {
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}


void AtomCore::setupDebugMessenger() {
	if (!mEnableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger.\n");
	}
}


void AtomCore::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}


void AtomCore::outputExtensionStatus(const std::vector<const char*>& extensions) const {
	std::cout << "Required glfw extensions: \n";

	for (const auto& e : extensions)
		std::cout << "\t" << e << "\n";

	uint32_t vExtCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &vExtCount, nullptr);

	std::vector<VkExtensionProperties> availableExt(vExtCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &vExtCount, availableExt.data());

	std::cout << "Available extensions: \n";

	for (const auto& e : availableExt) {
		std::cout << "\t" << e.extensionName << "\n";
	}

	std::cout << std::flush;
}


void AtomCore::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	mWindow = glfwCreateWindow(mViewSize.x, mViewSize.y, "Atom3D", nullptr, nullptr);
}

void AtomCore::run() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		drawFrame();
	}
}

void AtomCore::cleanup() const {
	if (mEnableValidationLayers)
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);

	vkResetCommandBuffer(mCommandBuffer, 0);
	vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &mCommandBuffer);

	vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);

	vkDestroySemaphore(mLogicalDevice, mImageAvailableS, nullptr);
	vkDestroySemaphore(mLogicalDevice, mRenderFinishedS, nullptr);
	vkDestroyFence(mLogicalDevice, mInFlightF, nullptr);

	for (const auto fb : mSwapchainFramebuffers)
		vkDestroyFramebuffer(mLogicalDevice, fb, nullptr);

	vkDestroyRenderPass(mLogicalDevice, mRenderPass, nullptr);
	vkDestroyPipeline(mLogicalDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);

	for (const auto iv : mSwapchainImageViews)
		vkDestroyImageView(mLogicalDevice, iv, nullptr);

	vkDestroySwapchainKHR(mLogicalDevice, mSwapchain, nullptr);
	vkDestroyDevice(mLogicalDevice, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);

	glfwDestroyWindow(mWindow);

	glfwTerminate();
}

}