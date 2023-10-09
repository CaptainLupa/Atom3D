# Generic Vulkan Pipeline

### https://vulkan-tutorial.com/en/Overview 

Uses *vulkan.hpp* style of implementation rather than plain C like the website.
So `vk::Instance` rather than `VkInstance`.
## Create `VkInstance` and `VkPhysicalDevice`

- `VkInstance` used to setup the API
- `VkPhysicalDevice` is a reference to the actual GPU

## Soft Device and Queues

- A `VkDevice` is used to represent the GPU(mLogicalDevice).
- Most render calls will be passed through a `VkQueue`
- `VkQueue` can be many different kinds, for basic rendering you need a present and graphics queue.

## Window and Swap Chain

- Use *GLFW* for windowing.
- Need a `VkSurfaceKHR` and `VkSwapChainKHR` to actually render to a window.
- Swap chain makes sure that we render to a different frame than that which is on the screen.

## Image views and Frame Buffers

- Used to draw to the swap chain.

## Render Passes

- Describes type of images that will be shown, and how.
- Tells Vulkan how to clear the frame, and what to clear it with before further rendering.

## Graphics Pipeline

- The beefiest part of the boilerplate code.
- The Vulkan pipeline is always the same order, but must be setup manually.
- Shaders and details are hardcoded into any specific pipeline, so if change is needed you have to recreate the pipeline.
    - luckily you can use a previous pipelines data to make a new one, so you don't have to type all the boilerplate again.
- The precompiled pipeline is good for performance.

## Command Pools and Buffers

- Before draw operations can occur and be committed to a queue, they must be recorded into a command buffer.
- There can be many command buffers at a time, but only one per frame.
- Command buffers are allocated from a Command Pool that is for a specific queue family.

## Draw loop

- Get image from swapchain with `vkAcquireNextImageKHR()`, select the right Command Buffer and commit it to the Graphics Queue with `vkQueueSubmit()`.
- Return the image to swapchain and present with `vkQueuePresentKHR()`.
- Also do sync work with Fences and Semaphore objects.


# Implementation Notes for AtomCore class.

## `pfnVkCreateDebugUtilMessengerEXT` and `pfnVkDestroyDebugUtilMessengerEXT`

- These 2 function pointers (of type `PFN_vkCreateDebugUtilsMessengerEXT` and `PFN_vkDestroyDebugUtilsMessengerEXT` respectively), are used to call the C functions that Vulkan provides.
- They are set up like the Demo solution in the VulkanSDK.
- 

## `createInstance` method

- Checks validation layers
- Make `vk::ApplicationInfo` object
    - Sets app name and Vulkan API version(1.3)
- Enables GPU extensions required.
- Adds a debug messenger for errors that occur either when `mInstance` is created or destroyed.

## `setupDebugMessenger`

- Creates a `VkDebugUtilsMessengerEXT` object for Vulkan API error logging.
- Severity and Type bits can be seen in `populateDebugMessengerCreateInfo` method.

## `debugCallback`

- Callback to print any Vulkan API errors.

## `createSurface`

- Instanciates `mSurface`.

## `pickPhysicalDevice`

- Chooses the best GPU device with required extensions.
- Right now this just chooses the first one that supports the required extensions.

## `createLogicalDevice`

- Sets up `mLogicalDevice`
- 
