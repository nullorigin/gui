// gui: Renderer Backend for Vulkan
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32,
// custom..)

// Implemented features:
//  [x] Renderer: User texture binding. Use 'VkDescriptorSet' as TextureID.
//  [X] Renderer:
//  Large meshes support (64k+ vertices) with 16-bit indices. [x] Renderer:
//  Multi-viewport / platform windows. With issues (flickering when creating a
//  new viewport).

// Important: on 32-bit systems, user texture binding is only supported if your
// imconfig file has '#define TextureID unsigned long long'. See vulkan.cpp file
// for details.
//
// Important note to the reader who wish to integrate vulkan.cpp/.h
// in their own engine/app.
// - Common Vulkan_XXX functions and structures are used to interface
// with vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your
//   engine/app.
// - Helper Vulkan_XXX functions and structures are only used by this
// example (main.cpp) and by
//   the backend itself (vulkan.cpp), but should PROBABLY NOT be used
//   by your own engine/app code.
// Read comments in vulkan.hpp.

#pragma once
#ifndef DISABLE
#include "gui.hpp" // API

// [Configuration] in order to use a custom Vulkan function loader:
// (1) You'll need to disable default Vulkan function prototypes.
//     We provide a '#define VULKAN_NO_PROTOTYPES' convenience
//     configuration flag. In order to make sure this is visible from the
//     vulkan.cpp compilation unit:
//     - Add '#define VULKAN_NO_PROTOTYPES' in your config.hpp file
//     - Or as a compilation flag in your build system
//     - Or uncomment here (not recommended because you'd be modifying imgui
//     sources!)
//     - Do not simply add it in a .cpp file!
// (2) Call Vulkan_LoadFunctions() before Vulkan_Init() with
// your custom function. If you have no idea what this is, leave it alone!
// #define VULKAN_NO_PROTOTYPES

// Vulkan includes
#if defined(VULKAN_NO_PROTOTYPES) && !defined(VK_NO_PROTOTYPES)
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>

// Initialization data, for Vulkan_Init()
// [Please zero-clear before use!]
struct Vulkan_InitInfo {
  VkInstance Instance;
  VkPhysicalDevice PhysicalDevice;
  VkDevice Device;
  unsigned int QueueFamily;
  VkQueue Queue;
  VkPipelineCache PipelineCache;
  VkDescriptorPool DescriptorPool;
  unsigned int Subpass;
  unsigned int MinImageCount;        // >= 2
  unsigned int ImageCount;           // >= MinImageCount
  VkSampleCountFlagBits MSAASamples; // >= VK_SAMPLE_COUNT_1_BIT (0 -> default
                                     // to VK_SAMPLE_COUNT_1_BIT)

  // Dynamic Rendering (Optional)
  bool
      UseDynamicRendering; // Need to explicitly enable VK_KHR_dynamic_rendering
                           // extension to use this, even for Vulkan 1.3.
  VkFormat ColorAttachmentFormat; // Required for dynamic rendering

  // Allocation, Debugging
  const VkAllocationCallbacks *Allocator;
  void (*CheckVkResultFn)(VkResult err);
  VkDeviceSize MinAllocationSize; // Minimum allocation size. Set to 1024*1024
                                  // to satisfy zealous best practices
                                  // validation layer and waste a little memory.
};

// Called by user code
API bool Vulkan_Init(Vulkan_InitInfo *info, VkRenderPass render_pass);
API void Vulkan_Shutdown();
API void Vulkan_NewFrame();
API void Vulkan_RenderDrawData(DrawData *draw_data,
                               VkCommandBuffer command_buffer,
                               VkPipeline pipeline = VK_NULL_HANDLE);
API bool Vulkan_CreateFontsTexture();
API void Vulkan_DestroyFontsTexture();
API void Vulkan_SetMinImageCount(
    unsigned int
        min_image_count); // To override MinImageCount after initialization
                          // (e.g. if swap chain is recreated)

// Register a texture (VkDescriptorSet == TextureID)
API VkDescriptorSet Vulkan_AddTexture(VkSampler sampler, VkImageView image_view,
                                      VkImageLayout image_layout);
API void Vulkan_RemoveTexture(VkDescriptorSet descriptor_set);

// Optional: load Vulkan functions with a custom function loader
// This is only useful with VULKAN_NO_PROTOTYPES / VK_NO_PROTOTYPES
API bool Vulkan_LoadFunctions(PFN_vkVoidFunction (*loader_func)(
                                  const char *function_name, void *user_data),
                              void *user_data = nullptr);

//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT
// used by your own engine/app.)
//-------------------------------------------------------------------------
// You probably do NOT need to use or care about those functions.
// Those functions only exist because:
//   1) they facilitate the readability and maintenance of the multiple main.cpp
//   examples files. 2) the multi-viewport / platform window implementation
//   needs them internally.
// Generally we avoid exposing any kind of superfluous high-level helpers in the
// bindings, but it is too much code to duplicate everywhere so we exceptionally
// expose them.
//
// Your engine/app will likely _already_ have code to setup all that stuff (swap
// chain, render pass, frame buffers, etc.). You may read this code to learn
// about Vulkan, but it is recommended you use you own custom tailored code to
// do equivalent work. (The Vulkan_XXX functions do not interact with
// any of the state used by the regular Vulkan_XXX functions)
//-------------------------------------------------------------------------

struct Vulkan_Frame;
struct Vulkan_Window;

// Helpers
API void Vulkan_CreateOrResizeWindow(VkInstance instance,
                                     VkPhysicalDevice physical_device,
                                     VkDevice device, Vulkan_Window *wnd,
                                     unsigned int queue_family,
                                     const VkAllocationCallbacks *allocator,
                                     int w, int h,
                                     unsigned int min_image_count);
API void Vulkan_DestroyWindow(VkInstance instance, VkDevice device,
                              Vulkan_Window *wnd,
                              const VkAllocationCallbacks *allocator);
API VkSurfaceFormatKHR Vulkan_SelectSurfaceFormat(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    const VkFormat *request_formats, int request_formats_count,
    VkColorSpaceKHR request_color_space);
API VkPresentModeKHR Vulkan_SelectPresentMode(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    const VkPresentModeKHR *request_modes, int request_modes_count);
API int Vulkan_GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode);

// Helper structure to hold the data needed by one rendering frame
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT
// used by your own engine/app.) [Please zero-clear before use!]
struct Vulkan_Frame {
  VkCommandPool CommandPool;
  VkCommandBuffer CommandBuffer;
  VkFence Fence;
  VkImage Backbuffer;
  VkImageView BackbufferView;
  VkFramebuffer Framebuffer;
};

struct Vulkan_FrameSemaphores {
  VkSemaphore ImageAcquiredSemaphore;
  VkSemaphore RenderCompleteSemaphore;
};

// Helper structure to hold the data needed by one rendering context into one OS
// window (Used by example's main.cpp. Used by multi-viewport features. Probably
// NOT used by your own engine/app.)
struct Vulkan_Window {
  int Width;
  int Height;
  VkSwapchainKHR Swapchain;
  VkSurfaceKHR Surface;
  VkSurfaceFormatKHR SurfaceFormat;
  VkPresentModeKHR PresentMode;
  VkRenderPass RenderPass;
  VkPipeline Pipeline; // The window pipeline may uses a different VkRenderPass
                       // than the one passed in Vulkan_InitInfo
  bool UseDynamicRendering;
  bool ClearEnable;
  VkClearValue ClearValue;
  unsigned int FrameIndex; // Current frame being rendered to (0 <= FrameIndex <
                           // FrameInFlightCount)
  unsigned int ImageCount; // Number of simultaneous in-flight frames (returned
                           // by vkGetSwapchainImagesKHR, usually derived from
                           // min_image_count)
  unsigned int
      SemaphoreIndex; // Current set of swapchain wait semaphores we're
                      // using (needs to be distinct from per frame data)
  Vulkan_Frame *Frames;
  Vulkan_FrameSemaphores *FrameSemaphores;

  Vulkan_Window() {
    memset((void *)this, 0, sizeof(*this));
    PresentMode = (VkPresentModeKHR)~0; // Ensure we get an error if user
                                        // doesn't set this.
    ClearEnable = true;
  }
};

#endif // #ifndef DISABLE
