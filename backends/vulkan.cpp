// gui: Renderer Backend for Vulkan
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32,
// custom..)

// Implemented features:
//  [x] Renderer: User texture binding. Use 'VkDescriptorSet' as TextureID.
//  [X] Renderer: Large meshes support (64k+ vertices) with 16-bit indices.
//  [x] Renderer: Multi-viewport / platform windows. With issues (flickering
//  when creating a new viewport).

// Important: on 32-bit systems, user texture binding is only supported if your
// imconfig file has '#define TextureID unsigned long long'. This is because we
// need TextureID to carry a 64-bit value and by default TextureID is defined as
// void*. To build this on 32-bit systems and support texture changes:
// - [Solution 1] IDE/msbuild: in "Properties/C++/Preprocessor Definitions" add
// 'TextureID=unsigned long long' (this is what we do in our .vcxproj files)
// - [Solution 2] IDE/msbuild: in "Properties/C++/Preprocessor Definitions" add
// 'USER_CONFIG="my_config.h"' and inside 'my_config.h' add '#define
// TextureID unsigned long long' and as many other options as you like.
// - [Solution 3] IDE/msbuild: edit config.hpp and add '#define TextureID
// unsigned long long' (prefer solution 2 to create your own config file!)
// - [Solution 4] command-line: add '/D TextureID=unsigned long long' to your
// cl.exe command-line (this is what we do in our batch files)

// Important note to the reader who wish to integrate vulkan.cpp/.h
// in their own engine/app.
// - Common Vulkan_XXX functions and structures are used to interface
// with vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your
//   engine/app.
// - Helper VulkanH_XXX functions and structures are only used by this
// example (main.cpp) and by
//   the backend itself (vulkan.cpp), but should PROBABLY NOT be used
//   by your own engine/app code.
// Read comments in vulkan.hpp.

#include "../gui.hpp"
#ifndef DISABLE
#include "vulkan.hpp"
#include <stdio.h>
#ifndef MAX
#define MAX(A, B) (((A) >= (B)) ? (A) : (B))
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning(disable : 4127) // condition expression is constant
#endif

// Forward Declarations
struct VulkanH_FrameRenderBuffers;
struct VulkanH_WindowRenderBuffers;
bool Vulkan_CreateDeviceObjects();
void Vulkan_DestroyDeviceObjects();
void VulkanH_DestroyFrame(VkDevice device, VulkanH_Frame *fd,
                          const VkAllocationCallbacks *allocator);
void VulkanH_DestroyFrameSemaphores(VkDevice device,
                                    VulkanH_FrameSemaphores *fsd,
                                    const VkAllocationCallbacks *allocator);
void VulkanH_DestroyFrameRenderBuffers(VkDevice device,
                                       VulkanH_FrameRenderBuffers *buffers,
                                       const VkAllocationCallbacks *allocator);
void VulkanH_DestroyWindowRenderBuffers(VkDevice device,
                                        VulkanH_WindowRenderBuffers *buffers,
                                        const VkAllocationCallbacks *allocator);
void VulkanH_DestroyAllViewportsRenderBuffers(
    VkDevice device, const VkAllocationCallbacks *allocator);
void VulkanH_CreateWindowSwapChain(VkPhysicalDevice physical_device,
                                   VkDevice device, VulkanH_Window *wd,
                                   const VkAllocationCallbacks *allocator,
                                   int w, int h, uint32_t min_image_count);
void VulkanH_CreateWindowCommandBuffers(VkPhysicalDevice physical_device,
                                        VkDevice device, VulkanH_Window *wd,
                                        uint32_t queue_family,
                                        const VkAllocationCallbacks *allocator);

// Vulkan prototypes for use with custom loaders
// (see description of VULKAN_NO_PROTOTYPES in vulkan.hpp
#ifdef VK_NO_PROTOTYPES
static bool g_FunctionsLoaded = false;
#else
static bool g_FunctionsLoaded = true;
#endif
#ifdef VK_NO_PROTOTYPES
#define VULKAN_FUNC_MAP(VULKAN_FUNC_MAP_MACRO)                                 \
  VULKAN_FUNC_MAP_MACRO(vkAllocateCommandBuffers)                              \
  VULKAN_FUNC_MAP_MACRO(vkAllocateDescriptorSets)                              \
  VULKAN_FUNC_MAP_MACRO(vkAllocateMemory)                                      \
  VULKAN_FUNC_MAP_MACRO(vkAcquireNextImageKHR)                                 \
  VULKAN_FUNC_MAP_MACRO(vkBeginCommandBuffer)                                  \
  VULKAN_FUNC_MAP_MACRO(vkBindBufferMemory)                                    \
  VULKAN_FUNC_MAP_MACRO(vkBindImageMemory)                                     \
  VULKAN_FUNC_MAP_MACRO(vkCmdBeginRenderPass)                                  \
  VULKAN_FUNC_MAP_MACRO(vkCmdBindDescriptorSets)                               \
  VULKAN_FUNC_MAP_MACRO(vkCmdBindIndexBuffer)                                  \
  VULKAN_FUNC_MAP_MACRO(vkCmdBindPipeline)                                     \
  VULKAN_FUNC_MAP_MACRO(vkCmdBindVertexBuffers)                                \
  VULKAN_FUNC_MAP_MACRO(vkCmdCopyBufferToImage)                                \
  VULKAN_FUNC_MAP_MACRO(vkCmdDrawIndexed)                                      \
  VULKAN_FUNC_MAP_MACRO(vkCmdEndRenderPass)                                    \
  VULKAN_FUNC_MAP_MACRO(vkCmdPipelineBarrier)                                  \
  VULKAN_FUNC_MAP_MACRO(vkCmdPushConstants)                                    \
  VULKAN_FUNC_MAP_MACRO(vkCmdSetScissor)                                       \
  VULKAN_FUNC_MAP_MACRO(vkCmdSetViewport)                                      \
  VULKAN_FUNC_MAP_MACRO(vkCreateBuffer)                                        \
  VULKAN_FUNC_MAP_MACRO(vkCreateCommandPool)                                   \
  VULKAN_FUNC_MAP_MACRO(vkCreateDescriptorSetLayout)                           \
  VULKAN_FUNC_MAP_MACRO(vkCreateFence)                                         \
  VULKAN_FUNC_MAP_MACRO(vkCreateFramebuffer)                                   \
  VULKAN_FUNC_MAP_MACRO(vkCreateGraphicsPipelines)                             \
  VULKAN_FUNC_MAP_MACRO(vkCreateImage)                                         \
  VULKAN_FUNC_MAP_MACRO(vkCreateImageView)                                     \
  VULKAN_FUNC_MAP_MACRO(vkCreatePipelineLayout)                                \
  VULKAN_FUNC_MAP_MACRO(vkCreateRenderPass)                                    \
  VULKAN_FUNC_MAP_MACRO(vkCreateSampler)                                       \
  VULKAN_FUNC_MAP_MACRO(vkCreateSemaphore)                                     \
  VULKAN_FUNC_MAP_MACRO(vkCreateShaderModule)                                  \
  VULKAN_FUNC_MAP_MACRO(vkCreateSwapchainKHR)                                  \
  VULKAN_FUNC_MAP_MACRO(vkDestroyBuffer)                                       \
  VULKAN_FUNC_MAP_MACRO(vkDestroyCommandPool)                                  \
  VULKAN_FUNC_MAP_MACRO(vkDestroyDescriptorSetLayout)                          \
  VULKAN_FUNC_MAP_MACRO(vkDestroyFence)                                        \
  VULKAN_FUNC_MAP_MACRO(vkDestroyFramebuffer)                                  \
  VULKAN_FUNC_MAP_MACRO(vkDestroyImage)                                        \
  VULKAN_FUNC_MAP_MACRO(vkDestroyImageView)                                    \
  VULKAN_FUNC_MAP_MACRO(vkDestroyPipeline)                                     \
  VULKAN_FUNC_MAP_MACRO(vkDestroyPipelineLayout)                               \
  VULKAN_FUNC_MAP_MACRO(vkDestroyRenderPass)                                   \
  VULKAN_FUNC_MAP_MACRO(vkDestroySampler)                                      \
  VULKAN_FUNC_MAP_MACRO(vkDestroySemaphore)                                    \
  VULKAN_FUNC_MAP_MACRO(vkDestroyShaderModule)                                 \
  VULKAN_FUNC_MAP_MACRO(vkDestroySurfaceKHR)                                   \
  VULKAN_FUNC_MAP_MACRO(vkDestroySwapchainKHR)                                 \
  VULKAN_FUNC_MAP_MACRO(vkDeviceWaitIdle)                                      \
  VULKAN_FUNC_MAP_MACRO(vkEndCommandBuffer)                                    \
  VULKAN_FUNC_MAP_MACRO(vkFlushMappedMemoryRanges)                             \
  VULKAN_FUNC_MAP_MACRO(vkFreeCommandBuffers)                                  \
  VULKAN_FUNC_MAP_MACRO(vkFreeDescriptorSets)                                  \
  VULKAN_FUNC_MAP_MACRO(vkFreeMemory)                                          \
  VULKAN_FUNC_MAP_MACRO(vkGetBufferMemoryRequirements)                         \
  VULKAN_FUNC_MAP_MACRO(vkGetImageMemoryRequirements)                          \
  VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceMemoryProperties)                   \
  VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)             \
  VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfaceFormatsKHR)                  \
  VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfacePresentModesKHR)             \
  VULKAN_FUNC_MAP_MACRO(vkGetPhysicalDeviceSurfaceSupportKHR)                  \
  VULKAN_FUNC_MAP_MACRO(vkGetSwapchainImagesKHR)                               \
  VULKAN_FUNC_MAP_MACRO(vkMapMemory)                                           \
  VULKAN_FUNC_MAP_MACRO(vkQueuePresentKHR)                                     \
  VULKAN_FUNC_MAP_MACRO(vkQueueSubmit)                                         \
  VULKAN_FUNC_MAP_MACRO(vkQueueWaitIdle)                                       \
  VULKAN_FUNC_MAP_MACRO(vkResetCommandPool)                                    \
  VULKAN_FUNC_MAP_MACRO(vkResetFences)                                         \
  VULKAN_FUNC_MAP_MACRO(vkUnmapMemory)                                         \
  VULKAN_FUNC_MAP_MACRO(vkUpdateDescriptorSets)                                \
  VULKAN_FUNC_MAP_MACRO(vkWaitForFences)

// Define function pointers
#define VULKAN_FUNC_DEF(func) static PFN_##func func;
VULKAN_FUNC_MAP(VULKAN_FUNC_DEF)
#undef VULKAN_FUNC_DEF
#endif // VK_NO_PROTOTYPES

#if defined(VK_VERSION_1_3) || defined(VK_KHR_dynamic_rendering)
#define VULKAN_HAS_DYNAMIC_RENDERING
static PFN_vkCmdBeginRenderingKHR VulkanFuncs_vkCmdBeginRenderingKHR;
static PFN_vkCmdEndRenderingKHR VulkanFuncs_vkCmdEndRenderingKHR;
#endif

// Reusable buffers used for rendering 1 current in-flight frame, for
// Vulkan_RenderDrawData() [Please zero-clear before use!]
struct VulkanH_FrameRenderBuffers {
  VkDeviceMemory VertexBufferMemory;
  VkDeviceMemory IndexBufferMemory;
  VkDeviceSize VertexBufferSize;
  VkDeviceSize IndexBufferSize;
  VkBuffer VertexBuffer;
  VkBuffer IndexBuffer;
};

// Each viewport will hold 1 VulkanH_WindowRenderBuffers
// [Please zero-clear before use!]
struct VulkanH_WindowRenderBuffers {
  uint32_t Index;
  uint32_t Count;
  VulkanH_FrameRenderBuffers *FrameRenderBuffers;
};

// For multi-viewport support:
// Helper structure we store in the void* RendererUserData field of each
// Viewport to easily retrieve our backend data.
struct Vulkan_ViewportData {
  bool WindowOwned;
  VulkanH_Window Window;                     // Used by secondary viewports only
  VulkanH_WindowRenderBuffers RenderBuffers; // Used by all viewports

  Vulkan_ViewportData() {
    WindowOwned = false;
    memset(&RenderBuffers, 0, sizeof(RenderBuffers));
  }
  ~Vulkan_ViewportData() {}
};

// Vulkan data
struct Vulkan_Data {
  Vulkan_InitInfo VulkanInitInfo;
  VkRenderPass RenderPass;
  VkDeviceSize BufferMemoryAlignment;
  VkPipelineCreateFlags PipelineCreateFlags;
  VkDescriptorSetLayout DescriptorSetLayout;
  VkPipelineLayout PipelineLayout;
  VkPipeline Pipeline;
  uint32_t Subpass;
  VkShaderModule ShaderModuleVert;
  VkShaderModule ShaderModuleFrag;

  // Font data
  VkSampler FontSampler;
  VkDeviceMemory FontMemory;
  VkImage FontImage;
  VkImageView FontView;
  VkDescriptorSet FontDescriptorSet;
  VkCommandPool FontCommandPool;
  VkCommandBuffer FontCommandBuffer;

  // Render buffers for main window
  VulkanH_WindowRenderBuffers MainWindowRenderBuffers;

  Vulkan_Data() {
    memset((void *)this, 0, sizeof(*this));
    BufferMemoryAlignment = 256;
  }
};

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

// Forward Declarations
static void Vulkan_InitPlatformInterface();
static void Vulkan_ShutdownPlatformInterface();

// backends/vulkan/glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; }
pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static uint32_t __glsl_shader_vert_spv[] = {
    0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x0000000b, 0x0000000f, 0x00000015,
    0x0000001b, 0x0000001c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00030005, 0x00000009, 0x00000000,
    0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006,
    0x00000009, 0x00000001, 0x00005655, 0x00030005, 0x0000000b, 0x0074754f,
    0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015,
    0x00565561, 0x00060005, 0x00000019, 0x505f6c67, 0x65567265, 0x78657472,
    0x00000000, 0x00060006, 0x00000019, 0x00000000, 0x505f6c67, 0x7469736f,
    0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c,
    0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075, 0x6e6f4368,
    0x6e617473, 0x00000074, 0x00050006, 0x0000001e, 0x00000000, 0x61635375,
    0x0000656c, 0x00060006, 0x0000001e, 0x00000001, 0x61725475, 0x616c736e,
    0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b,
    0x0000001e, 0x00000000, 0x00040047, 0x0000000f, 0x0000001e, 0x00000002,
    0x00040047, 0x00000015, 0x0000001e, 0x00000001, 0x00050048, 0x00000019,
    0x00000000, 0x0000000b, 0x00000000, 0x00030047, 0x00000019, 0x00000002,
    0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00050048, 0x0000001e,
    0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001,
    0x00000023, 0x00000008, 0x00030047, 0x0000001e, 0x00000002, 0x00020013,
    0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
    0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040017,
    0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007,
    0x00000008, 0x00040020, 0x0000000a, 0x00000003, 0x00000009, 0x0004003b,
    0x0000000a, 0x0000000b, 0x00000003, 0x00040015, 0x0000000c, 0x00000020,
    0x00000001, 0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020,
    0x0000000e, 0x00000001, 0x00000007, 0x0004003b, 0x0000000e, 0x0000000f,
    0x00000001, 0x00040020, 0x00000011, 0x00000003, 0x00000007, 0x0004002b,
    0x0000000c, 0x00000013, 0x00000001, 0x00040020, 0x00000014, 0x00000001,
    0x00000008, 0x0004003b, 0x00000014, 0x00000015, 0x00000001, 0x00040020,
    0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007,
    0x00040020, 0x0000001a, 0x00000003, 0x00000019, 0x0004003b, 0x0000001a,
    0x0000001b, 0x00000003, 0x0004003b, 0x00000014, 0x0000001c, 0x00000001,
    0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f,
    0x00000009, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000009,
    0x00040020, 0x00000021, 0x00000009, 0x00000008, 0x0004002b, 0x00000006,
    0x00000028, 0x00000000, 0x0004002b, 0x00000006, 0x00000029, 0x3f800000,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0004003d, 0x00000007, 0x00000010, 0x0000000f, 0x00050041,
    0x00000011, 0x00000012, 0x0000000b, 0x0000000d, 0x0003003e, 0x00000012,
    0x00000010, 0x0004003d, 0x00000008, 0x00000016, 0x00000015, 0x00050041,
    0x00000017, 0x00000018, 0x0000000b, 0x00000013, 0x0003003e, 0x00000018,
    0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041,
    0x00000021, 0x00000022, 0x00000020, 0x0000000d, 0x0004003d, 0x00000008,
    0x00000023, 0x00000022, 0x00050085, 0x00000008, 0x00000024, 0x0000001d,
    0x00000023, 0x00050041, 0x00000021, 0x00000025, 0x00000020, 0x00000013,
    0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008,
    0x00000027, 0x00000024, 0x00000026, 0x00050051, 0x00000006, 0x0000002a,
    0x00000027, 0x00000000, 0x00050051, 0x00000006, 0x0000002b, 0x00000027,
    0x00000001, 0x00070050, 0x00000007, 0x0000002c, 0x0000002a, 0x0000002b,
    0x00000028, 0x00000029, 0x00050041, 0x00000011, 0x0000002d, 0x0000001b,
    0x0000000d, 0x0003003e, 0x0000002d, 0x0000002c, 0x000100fd, 0x00010038};

// backends/vulkan/glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
    fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static uint32_t __glsl_shader_frag_spv[] = {
    0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00040005, 0x00000009, 0x6c6f4366,
    0x0000726f, 0x00030005, 0x0000000b, 0x00000000, 0x00050006, 0x0000000b,
    0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x0000000b, 0x00000001,
    0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016,
    0x78655473, 0x65727574, 0x00000000, 0x00040047, 0x00000009, 0x0000001e,
    0x00000000, 0x00040047, 0x0000000d, 0x0000001e, 0x00000000, 0x00040047,
    0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021,
    0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b,
    0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006,
    0x00000002, 0x0004001e, 0x0000000b, 0x00000007, 0x0000000a, 0x00040020,
    0x0000000c, 0x00000001, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
    0x00000001, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b,
    0x0000000e, 0x0000000f, 0x00000000, 0x00040020, 0x00000010, 0x00000001,
    0x00000007, 0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000,
    0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x00000014,
    0x00000013, 0x00040020, 0x00000015, 0x00000000, 0x00000014, 0x0004003b,
    0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e, 0x00000018,
    0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036,
    0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005,
    0x00050041, 0x00000010, 0x00000011, 0x0000000d, 0x0000000f, 0x0004003d,
    0x00000007, 0x00000012, 0x00000011, 0x0004003d, 0x00000014, 0x00000017,
    0x00000016, 0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x00000018,
    0x0004003d, 0x0000000a, 0x0000001b, 0x0000001a, 0x00050057, 0x00000007,
    0x0000001c, 0x00000017, 0x0000001b, 0x00050085, 0x00000007, 0x0000001d,
    0x00000012, 0x0000001c, 0x0003003e, 0x00000009, 0x0000001d, 0x000100fd,
    0x00010038};

//-----------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------

// Backend data stored in io.BackendRendererUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
// FIXME: multi-context support is not tested and probably dysfunctional in this
// backend.
static Vulkan_Data *Vulkan_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (Vulkan_Data *)Gui::GetIO().BackendRendererUserData
             : nullptr;
}

static uint32_t Vulkan_MemoryType(VkMemoryPropertyFlags properties,
                                  uint32_t type_bits) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  VkPhysicalDeviceMemoryProperties prop;
  vkGetPhysicalDeviceMemoryProperties(v->PhysicalDevice, &prop);
  for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
    if ((prop.memoryTypes[i].propertyFlags & properties) == properties &&
        type_bits & (1 << i))
      return i;
  return 0xFFFFFFFF; // Unable to find memoryType
}

static void check_vk_result(VkResult err) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  if (!bd)
    return;
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  if (v->CheckVkResultFn)
    v->CheckVkResultFn(err);
}

static void CreateOrResizeBuffer(VkBuffer &buffer,
                                 VkDeviceMemory &buffer_memory,
                                 VkDeviceSize &p_buffer_size, size_t new_size,
                                 VkBufferUsageFlagBits usage) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  VkResult err;
  if (buffer != VK_NULL_HANDLE)
    vkDestroyBuffer(v->Device, buffer, v->Allocator);
  if (buffer_memory != VK_NULL_HANDLE)
    vkFreeMemory(v->Device, buffer_memory, v->Allocator);

  VkDeviceSize vertex_buffer_size_aligned =
      ((new_size - 1) / bd->BufferMemoryAlignment + 1) *
      bd->BufferMemoryAlignment;
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = vertex_buffer_size_aligned;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  err = vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &buffer);
  check_vk_result(err);

  VkMemoryRequirements req;
  vkGetBufferMemoryRequirements(v->Device, buffer, &req);
  bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment)
                                  ? bd->BufferMemoryAlignment
                                  : req.alignment;
  VkDeviceSize size = MAX(v->MinAllocationSize, req.size);
  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = size;
  alloc_info.memoryTypeIndex = Vulkan_MemoryType(
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
  err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &buffer_memory);
  check_vk_result(err);

  err = vkBindBufferMemory(v->Device, buffer, buffer_memory, 0);
  check_vk_result(err);
  p_buffer_size = size;
}

static void Vulkan_SetupRenderState(DrawData *draw_data, VkPipeline pipeline,
                                    VkCommandBuffer command_buffer,
                                    VulkanH_FrameRenderBuffers *rb,
                                    int fb_width, int fb_height) {
  Vulkan_Data *bd = Vulkan_GetBackendData();

  // Bind pipeline:
  {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline);
  }

  // Bind Vertex And Index Buffer:
  if (draw_data->TotalVtxCount > 0) {
    VkBuffer vertex_buffers[1] = {rb->VertexBuffer};
    VkDeviceSize vertex_offset[1] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, vertex_offset);
    vkCmdBindIndexBuffer(command_buffer, rb->IndexBuffer, 0,
                         sizeof(DrawIdx) == 2 ? VK_INDEX_TYPE_UINT16
                                              : VK_INDEX_TYPE_UINT32);
  }

  // Setup viewport:
  {
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)fb_width;
    viewport.height = (float)fb_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
  }

  // Setup scale and translation:
  // Our visible imgui space lies from draw_data->DisplayPps (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is
  // (0,0) for single viewport apps.
  {
    float scale[2];
    scale[0] = 2.0f / draw_data->DisplaySize.x;
    scale[1] = 2.0f / draw_data->DisplaySize.y;
    float translate[2];
    translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
    translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
    vkCmdPushConstants(command_buffer, bd->PipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0,
                       sizeof(float) * 2, scale);
    vkCmdPushConstants(command_buffer, bd->PipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2,
                       sizeof(float) * 2, translate);
  }
}

// Render function
void Vulkan_RenderDrawData(DrawData *draw_data, VkCommandBuffer command_buffer,
                           VkPipeline pipeline) {
  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width =
      (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return;

  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  if (pipeline == VK_NULL_HANDLE)
    pipeline = bd->Pipeline;

  // Allocate array to store enough vertex/index buffers. Each unique viewport
  // gets its own storage.
  Vulkan_ViewportData *viewport_renderer_data =
      (Vulkan_ViewportData *)draw_data->OwnerViewport->RendererUserData;
  ASSERT(viewport_renderer_data != nullptr);
  VulkanH_WindowRenderBuffers *wrb = &viewport_renderer_data->RenderBuffers;
  if (wrb->FrameRenderBuffers == nullptr) {
    wrb->Index = 0;
    wrb->Count = v->ImageCount;
    wrb->FrameRenderBuffers = (VulkanH_FrameRenderBuffers *)ALLOC(
        sizeof(VulkanH_FrameRenderBuffers) * wrb->Count);
    memset(wrb->FrameRenderBuffers, 0,
           sizeof(VulkanH_FrameRenderBuffers) * wrb->Count);
  }
  ASSERT(wrb->Count == v->ImageCount);
  wrb->Index = (wrb->Index + 1) % wrb->Count;
  VulkanH_FrameRenderBuffers *rb = &wrb->FrameRenderBuffers[wrb->Index];

  if (draw_data->TotalVtxCount > 0) {
    // Create or resize the vertex/index buffers
    size_t vertex_size = draw_data->TotalVtxCount * sizeof(DrawVert);
    size_t index_size = draw_data->TotalIdxCount * sizeof(DrawIdx);
    if (rb->VertexBuffer == VK_NULL_HANDLE ||
        rb->VertexBufferSize < vertex_size)
      CreateOrResizeBuffer(rb->VertexBuffer, rb->VertexBufferMemory,
                           rb->VertexBufferSize, vertex_size,
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (rb->IndexBuffer == VK_NULL_HANDLE || rb->IndexBufferSize < index_size)
      CreateOrResizeBuffer(rb->IndexBuffer, rb->IndexBufferMemory,
                           rb->IndexBufferSize, index_size,
                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // Upload vertex/index data into a single contiguous GPU buffer
    DrawVert *vtx_dst = nullptr;
    DrawIdx *idx_dst = nullptr;
    VkResult err = vkMapMemory(v->Device, rb->VertexBufferMemory, 0,
                               rb->VertexBufferSize, 0, (void **)&vtx_dst);
    check_vk_result(err);
    err = vkMapMemory(v->Device, rb->IndexBufferMemory, 0, rb->IndexBufferSize,
                      0, (void **)&idx_dst);
    check_vk_result(err);
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
      const DrawList *cmd_list = draw_data->CmdLists[n];
      memcpy(vtx_dst, cmd_list->VtxBuffer.Data,
             cmd_list->VtxBuffer.Size * sizeof(DrawVert));
      memcpy(idx_dst, cmd_list->IdxBuffer.Data,
             cmd_list->IdxBuffer.Size * sizeof(DrawIdx));
      vtx_dst += cmd_list->VtxBuffer.Size;
      idx_dst += cmd_list->IdxBuffer.Size;
    }
    VkMappedMemoryRange range[2] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = rb->VertexBufferMemory;
    range[0].size = VK_WHOLE_SIZE;
    range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[1].memory = rb->IndexBufferMemory;
    range[1].size = VK_WHOLE_SIZE;
    err = vkFlushMappedMemoryRanges(v->Device, 2, range);
    check_vk_result(err);
    vkUnmapMemory(v->Device, rb->VertexBufferMemory);
    vkUnmapMemory(v->Device, rb->IndexBufferMemory);
  }

  // Setup desired Vulkan state
  Vulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width,
                          fb_height);

  // Will project scissor/clipping rectangles into framebuffer space
  Vec2 clip_off = draw_data->DisplayPos; // (0,0) unless using multi-viewports
  Vec2 clip_scale =
      draw_data->FramebufferScale; // (1,1) unless using retina display which
                                   // are often (2,2)

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own
  // offset into them)
  int global_vtx_offset = 0;
  int global_idx_offset = 0;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const DrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback != nullptr) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          Vulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb,
                                  fb_width, fb_height);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        Vec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
        Vec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

        // Clamp to viewport as vkCmdSetScissor() won't accept values that are
        // off bounds
        if (clip_min.x < 0.0f) {
          clip_min.x = 0.0f;
        }
        if (clip_min.y < 0.0f) {
          clip_min.y = 0.0f;
        }
        if (clip_max.x > fb_width) {
          clip_max.x = (float)fb_width;
        }
        if (clip_max.y > fb_height) {
          clip_max.y = (float)fb_height;
        }
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
          continue;

        // Apply scissor/clipping rectangle
        VkRect2D scissor;
        scissor.offset.x = (int32_t)(clip_min.x);
        scissor.offset.y = (int32_t)(clip_min.y);
        scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
        scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // Bind DescriptorSet with font or user texture
        VkDescriptorSet desc_set[1] = {(VkDescriptorSet)pcmd->TextureId};
        if (sizeof(TextureID) < sizeof(unsigned long long)) {
          // We don't support texture switches if TextureID hasn't been
          // redefined to be 64-bit. Do a flaky check that other textures
          // haven't been used.
          ASSERT(pcmd->TextureId == (TextureID)bd->FontDescriptorSet);
          desc_set[0] = bd->FontDescriptorSet;
        }
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                bd->PipelineLayout, 0, 1, desc_set, 0, nullptr);

        // Draw
        vkCmdDrawIndexed(command_buffer, pcmd->ElemCount, 1,
                         pcmd->IdxOffset + global_idx_offset,
                         pcmd->VtxOffset + global_vtx_offset, 0);
      }
    }
    global_idx_offset += cmd_list->IdxBuffer.Size;
    global_vtx_offset += cmd_list->VtxBuffer.Size;
  }

  // Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been
  // called. Our last values will leak into user/application rendering IF:
  // - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or
  // VK_DYNAMIC_STATE_SCISSOR dynamic state
  // - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself
  // to explicitly set that state. If you use VK_DYNAMIC_STATE_VIEWPORT or
  // VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before
  // rendering. In theory we should aim to backup/restore those values but I am
  // not sure this is possible. We perform a call to vkCmdSetScissor() to set
  // back a full viewport which is likely to fix things for 99% users but
  // technically this is not perfect. (See github #4644)
  VkRect2D scissor = {{0, 0}, {(uint32_t)fb_width, (uint32_t)fb_height}};
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

bool Vulkan_CreateFontsTexture() {
  IO &io = Gui::GetIO();
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  VkResult err;

  // Destroy existing texture (if any)
  if (bd->FontView || bd->FontImage || bd->FontMemory ||
      bd->FontDescriptorSet) {
    vkQueueWaitIdle(v->Queue);
    Vulkan_DestroyFontsTexture();
  }

  // Create command pool/buffer
  if (bd->FontCommandPool == VK_NULL_HANDLE) {
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = 0;
    info.queueFamilyIndex = v->QueueFamily;
    vkCreateCommandPool(v->Device, &info, v->Allocator, &bd->FontCommandPool);
  }
  if (bd->FontCommandBuffer == VK_NULL_HANDLE) {
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = bd->FontCommandPool;
    info.commandBufferCount = 1;
    err = vkAllocateCommandBuffers(v->Device, &info, &bd->FontCommandBuffer);
    check_vk_result(err);
  }

  // Start command buffer
  {
    err = vkResetCommandPool(v->Device, bd->FontCommandPool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(bd->FontCommandBuffer, &begin_info);
    check_vk_result(err);
  }

  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  size_t upload_size = width * height * 4 * sizeof(char);

  // Create the Image:
  {
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.extent.width = width;
    info.extent.height = height;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    err = vkCreateImage(v->Device, &info, v->Allocator, &bd->FontImage);
    check_vk_result(err);
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(v->Device, bd->FontImage, &req);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = MAX(v->MinAllocationSize, req.size);
    alloc_info.memoryTypeIndex = Vulkan_MemoryType(
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
    err =
        vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &bd->FontMemory);
    check_vk_result(err);
    err = vkBindImageMemory(v->Device, bd->FontImage, bd->FontMemory, 0);
    check_vk_result(err);
  }

  // Create the Image View:
  {
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = bd->FontImage;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.layerCount = 1;
    err = vkCreateImageView(v->Device, &info, v->Allocator, &bd->FontView);
    check_vk_result(err);
  }

  // Create the Descriptor Set:
  bd->FontDescriptorSet = (VkDescriptorSet)Vulkan_AddTexture(
      bd->FontSampler, bd->FontView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // Create the Upload Buffer:
  VkDeviceMemory upload_buffer_memory;
  VkBuffer upload_buffer;
  {
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = upload_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &upload_buffer);
    check_vk_result(err);
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(v->Device, upload_buffer, &req);
    bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment)
                                    ? bd->BufferMemoryAlignment
                                    : req.alignment;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = MAX(v->MinAllocationSize, req.size);
    alloc_info.memoryTypeIndex = Vulkan_MemoryType(
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
    err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator,
                           &upload_buffer_memory);
    check_vk_result(err);
    err = vkBindBufferMemory(v->Device, upload_buffer, upload_buffer_memory, 0);
    check_vk_result(err);
  }

  // Upload to Buffer:
  {
    char *map = nullptr;
    err = vkMapMemory(v->Device, upload_buffer_memory, 0, upload_size, 0,
                      (void **)(&map));
    check_vk_result(err);
    memcpy(map, pixels, upload_size);
    VkMappedMemoryRange range[1] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = upload_buffer_memory;
    range[0].size = upload_size;
    err = vkFlushMappedMemoryRanges(v->Device, 1, range);
    check_vk_result(err);
    vkUnmapMemory(v->Device, upload_buffer_memory);
  }

  // Copy to Image:
  {
    VkImageMemoryBarrier copy_barrier[1] = {};
    copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].image = bd->FontImage;
    copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_barrier[0].subresourceRange.levelCount = 1;
    copy_barrier[0].subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(bd->FontCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, copy_barrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(bd->FontCommandBuffer, upload_buffer, bd->FontImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier use_barrier[1] = {};
    use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].image = bd->FontImage;
    use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    use_barrier[0].subresourceRange.levelCount = 1;
    use_barrier[0].subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(bd->FontCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, use_barrier);
  }

  // Store our identifier
  io.Fonts->SetTexID((TextureID)bd->FontDescriptorSet);

  // End command buffer
  VkSubmitInfo end_info = {};
  end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  end_info.commandBufferCount = 1;
  end_info.pCommandBuffers = &bd->FontCommandBuffer;
  err = vkEndCommandBuffer(bd->FontCommandBuffer);
  check_vk_result(err);
  err = vkQueueSubmit(v->Queue, 1, &end_info, VK_NULL_HANDLE);
  check_vk_result(err);

  err = vkQueueWaitIdle(v->Queue);
  check_vk_result(err);

  vkDestroyBuffer(v->Device, upload_buffer, v->Allocator);
  vkFreeMemory(v->Device, upload_buffer_memory, v->Allocator);

  return true;
}

// You probably never need to call this, as it is called by
// Vulkan_CreateFontsTexture() and Vulkan_Shutdown().
void Vulkan_DestroyFontsTexture() {
  IO &io = Gui::GetIO();
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;

  if (bd->FontDescriptorSet) {
    Vulkan_RemoveTexture(bd->FontDescriptorSet);
    bd->FontDescriptorSet = VK_NULL_HANDLE;
    io.Fonts->SetTexID(0);
  }

  if (bd->FontView) {
    vkDestroyImageView(v->Device, bd->FontView, v->Allocator);
    bd->FontView = VK_NULL_HANDLE;
  }
  if (bd->FontImage) {
    vkDestroyImage(v->Device, bd->FontImage, v->Allocator);
    bd->FontImage = VK_NULL_HANDLE;
  }
  if (bd->FontMemory) {
    vkFreeMemory(v->Device, bd->FontMemory, v->Allocator);
    bd->FontMemory = VK_NULL_HANDLE;
  }
}

static void Vulkan_CreateShaderModules(VkDevice device,
                                       const VkAllocationCallbacks *allocator) {
  // Create the shader modules
  Vulkan_Data *bd = Vulkan_GetBackendData();
  if (bd->ShaderModuleVert == VK_NULL_HANDLE) {
    VkShaderModuleCreateInfo vert_info = {};
    vert_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vert_info.codeSize = sizeof(__glsl_shader_vert_spv);
    vert_info.pCode = (uint32_t *)__glsl_shader_vert_spv;
    VkResult err = vkCreateShaderModule(device, &vert_info, allocator,
                                        &bd->ShaderModuleVert);
    check_vk_result(err);
  }
  if (bd->ShaderModuleFrag == VK_NULL_HANDLE) {
    VkShaderModuleCreateInfo frag_info = {};
    frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    frag_info.codeSize = sizeof(__glsl_shader_frag_spv);
    frag_info.pCode = (uint32_t *)__glsl_shader_frag_spv;
    VkResult err = vkCreateShaderModule(device, &frag_info, allocator,
                                        &bd->ShaderModuleFrag);
    check_vk_result(err);
  }
}

static void Vulkan_CreatePipeline(VkDevice device,
                                  const VkAllocationCallbacks *allocator,
                                  VkPipelineCache pipelineCache,
                                  VkRenderPass renderPass,
                                  VkSampleCountFlagBits MSAASamples,
                                  VkPipeline *pipeline, uint32_t subpass) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_CreateShaderModules(device, allocator);

  VkPipelineShaderStageCreateInfo stage[2] = {};
  stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stage[0].module = bd->ShaderModuleVert;
  stage[0].pName = "main";
  stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stage[1].module = bd->ShaderModuleFrag;
  stage[1].pName = "main";

  VkVertexInputBindingDescription binding_desc[1] = {};
  binding_desc[0].stride = sizeof(DrawVert);
  binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription attribute_desc[3] = {};
  attribute_desc[0].location = 0;
  attribute_desc[0].binding = binding_desc[0].binding;
  attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_desc[0].offset = offsetof(DrawVert, pos);
  attribute_desc[1].location = 1;
  attribute_desc[1].binding = binding_desc[0].binding;
  attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_desc[1].offset = offsetof(DrawVert, uv);
  attribute_desc[2].location = 2;
  attribute_desc[2].binding = binding_desc[0].binding;
  attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
  attribute_desc[2].offset = offsetof(DrawVert, col);

  VkPipelineVertexInputStateCreateInfo vertex_info = {};
  vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_info.vertexBindingDescriptionCount = 1;
  vertex_info.pVertexBindingDescriptions = binding_desc;
  vertex_info.vertexAttributeDescriptionCount = 3;
  vertex_info.pVertexAttributeDescriptions = attribute_desc;

  VkPipelineInputAssemblyStateCreateInfo ia_info = {};
  ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineViewportStateCreateInfo viewport_info = {};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo raster_info = {};
  raster_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster_info.polygonMode = VK_POLYGON_MODE_FILL;
  raster_info.cullMode = VK_CULL_MODE_NONE;
  raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  raster_info.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo ms_info = {};
  ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms_info.rasterizationSamples =
      (MSAASamples != 0) ? MSAASamples : VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState color_attachment[1] = {};
  color_attachment[0].blendEnable = VK_TRUE;
  color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
  color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
  color_attachment[0].colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineDepthStencilStateCreateInfo depth_info = {};
  depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

  VkPipelineColorBlendStateCreateInfo blend_info = {};
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.attachmentCount = 1;
  blend_info.pAttachments = color_attachment;

  VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = (uint32_t)ARRAYSIZE(dynamic_states);
  dynamic_state.pDynamicStates = dynamic_states;

  VkGraphicsPipelineCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  info.flags = bd->PipelineCreateFlags;
  info.stageCount = 2;
  info.pStages = stage;
  info.pVertexInputState = &vertex_info;
  info.pInputAssemblyState = &ia_info;
  info.pViewportState = &viewport_info;
  info.pRasterizationState = &raster_info;
  info.pMultisampleState = &ms_info;
  info.pDepthStencilState = &depth_info;
  info.pColorBlendState = &blend_info;
  info.pDynamicState = &dynamic_state;
  info.layout = bd->PipelineLayout;
  info.renderPass = renderPass;
  info.subpass = subpass;

#ifdef VULKAN_HAS_DYNAMIC_RENDERING
  VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {};
  pipelineRenderingCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
  pipelineRenderingCreateInfo.colorAttachmentCount = 1;
  pipelineRenderingCreateInfo.pColorAttachmentFormats =
      &bd->VulkanInitInfo.ColorAttachmentFormat;
  if (bd->VulkanInitInfo.UseDynamicRendering) {
    info.pNext = &pipelineRenderingCreateInfo;
    info.renderPass = VK_NULL_HANDLE; // Just make sure it's actually nullptr.
  }
#endif

  VkResult err = vkCreateGraphicsPipelines(device, pipelineCache, 1, &info,
                                           allocator, pipeline);
  check_vk_result(err);
}

bool Vulkan_CreateDeviceObjects() {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  VkResult err;

  if (!bd->FontSampler) {
    // Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
    // FontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false'
    // to allow point/nearest sampling.
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.minLod = -1000;
    info.maxLod = 1000;
    info.maxAnisotropy = 1.0f;
    err = vkCreateSampler(v->Device, &info, v->Allocator, &bd->FontSampler);
    check_vk_result(err);
  }

  if (!bd->DescriptorSetLayout) {
    VkDescriptorSetLayoutBinding binding[1] = {};
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = binding;
    err = vkCreateDescriptorSetLayout(v->Device, &info, v->Allocator,
                                      &bd->DescriptorSetLayout);
    check_vk_result(err);
  }

  if (!bd->PipelineLayout) {
    // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full
    // 3d projection matrix
    VkPushConstantRange push_constants[1] = {};
    push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constants[0].offset = sizeof(float) * 0;
    push_constants[0].size = sizeof(float) * 4;
    VkDescriptorSetLayout set_layout[1] = {bd->DescriptorSetLayout};
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = set_layout;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = push_constants;
    err = vkCreatePipelineLayout(v->Device, &layout_info, v->Allocator,
                                 &bd->PipelineLayout);
    check_vk_result(err);
  }

  Vulkan_CreatePipeline(v->Device, v->Allocator, v->PipelineCache,
                        bd->RenderPass, v->MSAASamples, &bd->Pipeline,
                        bd->Subpass);

  return true;
}

void Vulkan_DestroyDeviceObjects() {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  VulkanH_DestroyAllViewportsRenderBuffers(v->Device, v->Allocator);
  Vulkan_DestroyFontsTexture();

  if (bd->FontCommandBuffer) {
    vkFreeCommandBuffers(v->Device, bd->FontCommandPool, 1,
                         &bd->FontCommandBuffer);
    bd->FontCommandBuffer = VK_NULL_HANDLE;
  }
  if (bd->FontCommandPool) {
    vkDestroyCommandPool(v->Device, bd->FontCommandPool, v->Allocator);
    bd->FontCommandPool = VK_NULL_HANDLE;
  }
  if (bd->ShaderModuleVert) {
    vkDestroyShaderModule(v->Device, bd->ShaderModuleVert, v->Allocator);
    bd->ShaderModuleVert = VK_NULL_HANDLE;
  }
  if (bd->ShaderModuleFrag) {
    vkDestroyShaderModule(v->Device, bd->ShaderModuleFrag, v->Allocator);
    bd->ShaderModuleFrag = VK_NULL_HANDLE;
  }
  if (bd->FontSampler) {
    vkDestroySampler(v->Device, bd->FontSampler, v->Allocator);
    bd->FontSampler = VK_NULL_HANDLE;
  }
  if (bd->DescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(v->Device, bd->DescriptorSetLayout,
                                 v->Allocator);
    bd->DescriptorSetLayout = VK_NULL_HANDLE;
  }
  if (bd->PipelineLayout) {
    vkDestroyPipelineLayout(v->Device, bd->PipelineLayout, v->Allocator);
    bd->PipelineLayout = VK_NULL_HANDLE;
  }
  if (bd->Pipeline) {
    vkDestroyPipeline(v->Device, bd->Pipeline, v->Allocator);
    bd->Pipeline = VK_NULL_HANDLE;
  }
}

bool Vulkan_LoadFunctions(PFN_vkVoidFunction (*loader_func)(
                              const char *function_name, void *user_data),
                          void *user_data) {
  // Load function pointers
  // You can use the default Vulkan loader using:
  //      Vulkan_LoadFunctions([](const char* function_name, void*) {
  //      return vkGetInstanceProcAddr(your_vk_isntance, function_name); });
  // But this would be equivalent to not setting VK_NO_PROTOTYPES.
#ifdef VK_NO_PROTOTYPES
#define VULKAN_FUNC_LOAD(func)                                                 \
  func = reinterpret_cast<decltype(func)>(loader_func(#func, user_data));      \
  if (func == nullptr)                                                         \
    return false;
  VULKAN_FUNC_MAP(VULKAN_FUNC_LOAD)
#undef VULKAN_FUNC_LOAD

#ifdef VULKAN_HAS_DYNAMIC_RENDERING
  // Manually load those two (see #5446)
  VulkanFuncs_vkCmdBeginRenderingKHR =
      reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(
          loader_func("vkCmdBeginRenderingKHR", user_data));
  VulkanFuncs_vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(
      loader_func("vkCmdEndRenderingKHR", user_data));
#endif
#else
  UNUSED(loader_func);
  UNUSED(user_data);
#endif

  g_FunctionsLoaded = true;
  return true;
}

bool Vulkan_Init(Vulkan_InitInfo *info, VkRenderPass render_pass) {
  ASSERT(g_FunctionsLoaded &&
         "Need to call Vulkan_LoadFunctions() if "
         "VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");

  if (info->UseDynamicRendering) {
#ifdef VULKAN_HAS_DYNAMIC_RENDERING
#ifndef VK_NO_PROTOTYPES
    VulkanFuncs_vkCmdBeginRenderingKHR =
        reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(
            vkGetInstanceProcAddr(info->Instance, "vkCmdBeginRenderingKHR"));
    VulkanFuncs_vkCmdEndRenderingKHR =
        reinterpret_cast<PFN_vkCmdEndRenderingKHR>(
            vkGetInstanceProcAddr(info->Instance, "vkCmdEndRenderingKHR"));
#endif
    ASSERT(VulkanFuncs_vkCmdBeginRenderingKHR != nullptr);
    ASSERT(VulkanFuncs_vkCmdEndRenderingKHR != nullptr);
#else
    ASSERT(0 && "Can't use dynamic rendering when neither VK_VERSION_1_3 or "
                "VK_KHR_dynamic_rendering is defined.");
#endif
  }

  IO &io = Gui::GetIO();
  ASSERT(io.BackendRendererUserData == nullptr &&
         "Already initialized a renderer backend!");

  // Setup backend capabilities flags
  Vulkan_Data *bd = NEW(Vulkan_Data)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "vulkan";
  io.BackendFlags |=
      BackendFlags_RendererHasVtxOffset; // We can honor the
                                         // DrawCmd::VtxOffset field,
                                         // allowing for large meshes.
  io.BackendFlags |=
      BackendFlags_RendererHasViewports; // We can create multi-viewports
                                         // on the Renderer side (optional)

  ASSERT(info->Instance != VK_NULL_HANDLE);
  ASSERT(info->PhysicalDevice != VK_NULL_HANDLE);
  ASSERT(info->Device != VK_NULL_HANDLE);
  ASSERT(info->Queue != VK_NULL_HANDLE);
  ASSERT(info->DescriptorPool != VK_NULL_HANDLE);
  ASSERT(info->MinImageCount >= 2);
  ASSERT(info->ImageCount >= info->MinImageCount);
  if (info->UseDynamicRendering == false)
    ASSERT(render_pass != VK_NULL_HANDLE);

  bd->VulkanInitInfo = *info;
  bd->RenderPass = render_pass;
  bd->Subpass = info->Subpass;

  Vulkan_CreateDeviceObjects();

  // Our render function expect RendererUserData to be storing the window render
  // buffer we need (for the main viewport we won't use ->Window)
  Viewport *main_viewport = Gui::GetMainViewport();
  main_viewport->RendererUserData = NEW(Vulkan_ViewportData)();

  if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
    Vulkan_InitPlatformInterface();

  return true;
}

void Vulkan_Shutdown() {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  ASSERT(bd != nullptr &&
         "No renderer backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  // First destroy objects in all viewports
  Vulkan_DestroyDeviceObjects();

  // Manually delete main viewport render data in-case we haven't initialized
  // for viewports
  Viewport *main_viewport = Gui::GetMainViewport();
  if (Vulkan_ViewportData *vd =
          (Vulkan_ViewportData *)main_viewport->RendererUserData)
    DELETE(vd);
  main_viewport->RendererUserData = nullptr;

  // Clean up windows
  Vulkan_ShutdownPlatformInterface();

  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &=
      ~(BackendFlags_RendererHasVtxOffset | BackendFlags_RendererHasViewports);
  DELETE(bd);
}

void Vulkan_NewFrame() {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  ASSERT(bd != nullptr && "Did you call Vulkan_Init()?");

  if (!bd->FontDescriptorSet)
    Vulkan_CreateFontsTexture();
}

void Vulkan_SetMinImageCount(uint32_t min_image_count) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  ASSERT(min_image_count >= 2);
  if (bd->VulkanInitInfo.MinImageCount == min_image_count)
    return;

  ASSERT(0); // FIXME-VIEWPORT: Unsupported. Need to recreate all swap chains!
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  VkResult err = vkDeviceWaitIdle(v->Device);
  check_vk_result(err);
  VulkanH_DestroyAllViewportsRenderBuffers(v->Device, v->Allocator);

  bd->VulkanInitInfo.MinImageCount = min_image_count;
}

// Register a texture
VkDescriptorSet Vulkan_AddTexture(VkSampler sampler, VkImageView image_view,
                                  VkImageLayout image_layout) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;

  // Create Descriptor Set:
  VkDescriptorSet descriptor_set;
  {
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = v->DescriptorPool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &bd->DescriptorSetLayout;
    VkResult err =
        vkAllocateDescriptorSets(v->Device, &alloc_info, &descriptor_set);
    check_vk_result(err);
  }

  // Update the Descriptor Set:
  {
    VkDescriptorImageInfo desc_image[1] = {};
    desc_image[0].sampler = sampler;
    desc_image[0].imageView = image_view;
    desc_image[0].imageLayout = image_layout;
    VkWriteDescriptorSet write_desc[1] = {};
    write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_desc[0].dstSet = descriptor_set;
    write_desc[0].descriptorCount = 1;
    write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_desc[0].pImageInfo = desc_image;
    vkUpdateDescriptorSets(v->Device, 1, write_desc, 0, nullptr);
  }
  return descriptor_set;
}

void Vulkan_RemoveTexture(VkDescriptorSet descriptor_set) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  vkFreeDescriptorSets(v->Device, v->DescriptorPool, 1, &descriptor_set);
}

//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT
// used by your own app.)
//-------------------------------------------------------------------------
// You probably do NOT need to use or care about those functions.
// Those functions only exist because:
//   1) they facilitate the readability and maintenance of the multiple main.cpp
//   examples files. 2) the upcoming multi-viewport feature will need them
//   internally.
// Generally we avoid exposing any kind of superfluous high-level helpers in the
// backends, but it is too much code to duplicate everywhere so we exceptionally
// expose them.
//
// Your engine/app will likely _already_ have code to setup all that stuff (swap
// chain, render pass, frame buffers, etc.). You may read this code to learn
// about Vulkan, but it is recommended you use you own custom tailored code to
// do equivalent work. (The VulkanH_XXX functions do not interact with
// any of the state used by the regular Vulkan_XXX functions)
//-------------------------------------------------------------------------

VkSurfaceFormatKHR VulkanH_SelectSurfaceFormat(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    const VkFormat *request_formats, int request_formats_count,
    VkColorSpaceKHR request_color_space) {
  ASSERT(g_FunctionsLoaded &&
         "Need to call Vulkan_LoadFunctions() if "
         "VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
  ASSERT(request_formats != nullptr);
  ASSERT(request_formats_count > 0);

  // Per Spec Format and View Format are expected to be the same unless
  // VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation Assuming that the
  // default behavior is without setting this bit, there is no need for separate
  // Swapchain image and image view format Additionally several new color spaces
  // were introduced with Vulkan Spec v1.0.40, hence we must make sure that a
  // format with the mostly available color space,
  // VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
  uint32_t avail_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count,
                                       nullptr);
  Vector<VkSurfaceFormatKHR> avail_format;
  avail_format.resize((int)avail_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count,
                                       avail_format.Data);

  // First check if only one format, VK_FORMAT_UNDEFINED, is available, which
  // would imply that any format is available
  if (avail_count == 1) {
    if (avail_format[0].format == VK_FORMAT_UNDEFINED) {
      VkSurfaceFormatKHR ret;
      ret.format = request_formats[0];
      ret.colorSpace = request_color_space;
      return ret;
    } else {
      // No point in searching another format
      return avail_format[0];
    }
  } else {
    // Request several formats, the first found will be used
    for (int request_i = 0; request_i < request_formats_count; request_i++)
      for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
        if (avail_format[avail_i].format == request_formats[request_i] &&
            avail_format[avail_i].colorSpace == request_color_space)
          return avail_format[avail_i];

    // If none of the requested image formats could be found, use the first
    // available
    return avail_format[0];
  }
}

VkPresentModeKHR VulkanH_SelectPresentMode(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    const VkPresentModeKHR *request_modes, int request_modes_count) {
  ASSERT(g_FunctionsLoaded &&
         "Need to call Vulkan_LoadFunctions() if "
         "VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
  ASSERT(request_modes != nullptr);
  ASSERT(request_modes_count > 0);

  // Request a certain mode and confirm that it is available. If not use
  // VK_PRESENT_MODE_FIFO_KHR which is mandatory
  uint32_t avail_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &avail_count, nullptr);
  Vector<VkPresentModeKHR> avail_modes;
  avail_modes.resize((int)avail_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &avail_count, avail_modes.Data);
  // for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
  //     printf("[vulkan] avail_modes[%d] = %d\n", avail_i,
  //     avail_modes[avail_i]);

  for (int request_i = 0; request_i < request_modes_count; request_i++)
    for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
      if (request_modes[request_i] == avail_modes[avail_i])
        return request_modes[request_i];

  return VK_PRESENT_MODE_FIFO_KHR; // Always available
}

void VulkanH_CreateWindowCommandBuffers(
    VkPhysicalDevice physical_device, VkDevice device, VulkanH_Window *wd,
    uint32_t queue_family, const VkAllocationCallbacks *allocator) {
  ASSERT(physical_device != VK_NULL_HANDLE && device != VK_NULL_HANDLE);
  (void)physical_device;
  (void)allocator;

  // Create Command Buffers
  VkResult err;
  for (uint32_t i = 0; i < wd->ImageCount; i++) {
    VulkanH_Frame *fd = &wd->Frames[i];
    VulkanH_FrameSemaphores *fsd = &wd->FrameSemaphores[i];
    {
      VkCommandPoolCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      info.flags = 0;
      info.queueFamilyIndex = queue_family;
      err = vkCreateCommandPool(device, &info, allocator, &fd->CommandPool);
      check_vk_result(err);
    }
    {
      VkCommandBufferAllocateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      info.commandPool = fd->CommandPool;
      info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      info.commandBufferCount = 1;
      err = vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
      check_vk_result(err);
    }
    {
      VkFenceCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      err = vkCreateFence(device, &info, allocator, &fd->Fence);
      check_vk_result(err);
    }
    {
      VkSemaphoreCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      err = vkCreateSemaphore(device, &info, allocator,
                              &fsd->ImageAcquiredSemaphore);
      check_vk_result(err);
      err = vkCreateSemaphore(device, &info, allocator,
                              &fsd->RenderCompleteSemaphore);
      check_vk_result(err);
    }
  }
}

int VulkanH_GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode) {
  if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
    return 3;
  if (present_mode == VK_PRESENT_MODE_FIFO_KHR ||
      present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
    return 2;
  if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
    return 1;
  ASSERT(0);
  return 1;
}

// Also destroy old swap chain and in-flight frames data, if any.
void VulkanH_CreateWindowSwapChain(VkPhysicalDevice physical_device,
                                   VkDevice device, VulkanH_Window *wd,
                                   const VkAllocationCallbacks *allocator,
                                   int w, int h, uint32_t min_image_count) {
  VkResult err;
  VkSwapchainKHR old_swapchain = wd->Swapchain;
  wd->Swapchain = VK_NULL_HANDLE;
  err = vkDeviceWaitIdle(device);
  check_vk_result(err);

  // We don't use VulkanH_DestroyWindow() because we want to preserve
  // the old swapchain to create the new one. Destroy old Framebuffer
  for (uint32_t i = 0; i < wd->ImageCount; i++) {
    VulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
    VulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
  }
  FREE(wd->Frames);
  FREE(wd->FrameSemaphores);
  wd->Frames = nullptr;
  wd->FrameSemaphores = nullptr;
  wd->ImageCount = 0;
  if (wd->RenderPass)
    vkDestroyRenderPass(device, wd->RenderPass, allocator);
  if (wd->Pipeline)
    vkDestroyPipeline(device, wd->Pipeline, allocator);

  // If min image count was not specified, request different count of images
  // dependent on selected present mode
  if (min_image_count == 0)
    min_image_count = VulkanH_GetMinImageCountFromPresentMode(wd->PresentMode);

  // Create Swapchain
  {
    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = wd->Surface;
    info.minImageCount = min_image_count;
    info.imageFormat = wd->SurfaceFormat.format;
    info.imageColorSpace = wd->SurfaceFormat.colorSpace;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode =
        VK_SHARING_MODE_EXCLUSIVE; // Assume that graphics family == present
                                   // family
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = wd->PresentMode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = old_swapchain;
    VkSurfaceCapabilitiesKHR cap;
    err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                    wd->Surface, &cap);
    check_vk_result(err);
    if (info.minImageCount < cap.minImageCount)
      info.minImageCount = cap.minImageCount;
    else if (cap.maxImageCount != 0 && info.minImageCount > cap.maxImageCount)
      info.minImageCount = cap.maxImageCount;

    if (cap.currentExtent.width == 0xffffffff) {
      info.imageExtent.width = wd->Width = w;
      info.imageExtent.height = wd->Height = h;
    } else {
      info.imageExtent.width = wd->Width = cap.currentExtent.width;
      info.imageExtent.height = wd->Height = cap.currentExtent.height;
    }
    err = vkCreateSwapchainKHR(device, &info, allocator, &wd->Swapchain);
    check_vk_result(err);
    err = vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount,
                                  nullptr);
    check_vk_result(err);
    VkImage backbuffers[16] = {};
    ASSERT(wd->ImageCount >= min_image_count);
    ASSERT(wd->ImageCount < ARRAYSIZE(backbuffers));
    err = vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount,
                                  backbuffers);
    check_vk_result(err);

    ASSERT(wd->Frames == nullptr);
    wd->Frames = (VulkanH_Frame *)ALLOC(sizeof(VulkanH_Frame) * wd->ImageCount);
    wd->FrameSemaphores = (VulkanH_FrameSemaphores *)ALLOC(
        sizeof(VulkanH_FrameSemaphores) * wd->ImageCount);
    memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);
    memset(wd->FrameSemaphores, 0,
           sizeof(wd->FrameSemaphores[0]) * wd->ImageCount);
    for (uint32_t i = 0; i < wd->ImageCount; i++)
      wd->Frames[i].Backbuffer = backbuffers[i];
  }
  if (old_swapchain)
    vkDestroySwapchainKHR(device, old_swapchain, allocator);

  // Create the Render Pass
  if (wd->UseDynamicRendering == false) {
    VkAttachmentDescription attachment = {};
    attachment.format = wd->SurfaceFormat.format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = wd->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                        : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    err = vkCreateRenderPass(device, &info, allocator, &wd->RenderPass);
    check_vk_result(err);

    // We do not create a pipeline by default as this is also used by examples'
    // main.cpp, but secondary viewport in multi-viewport mode may want to
    // create one with:
    // Vulkan_CreatePipeline(device, allocator, VK_NULL_HANDLE,
    // wd->RenderPass, VK_SAMPLE_COUNT_1_BIT, &wd->Pipeline, bd->Subpass);
  }

  // Create The Image Views
  {
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = wd->SurfaceFormat.format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;
    VkImageSubresourceRange image_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0,
                                           1};
    info.subresourceRange = image_range;
    for (uint32_t i = 0; i < wd->ImageCount; i++) {
      VulkanH_Frame *fd = &wd->Frames[i];
      info.image = fd->Backbuffer;
      err = vkCreateImageView(device, &info, allocator, &fd->BackbufferView);
      check_vk_result(err);
    }
  }

  // Create Framebuffer
  if (wd->UseDynamicRendering == false) {
    VkImageView attachment[1];
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = wd->RenderPass;
    info.attachmentCount = 1;
    info.pAttachments = attachment;
    info.width = wd->Width;
    info.height = wd->Height;
    info.layers = 1;
    for (uint32_t i = 0; i < wd->ImageCount; i++) {
      VulkanH_Frame *fd = &wd->Frames[i];
      attachment[0] = fd->BackbufferView;
      err = vkCreateFramebuffer(device, &info, allocator, &fd->Framebuffer);
      check_vk_result(err);
    }
  }
}

// Create or resize window
void VulkanH_CreateOrResizeWindow(VkInstance instance,
                                  VkPhysicalDevice physical_device,
                                  VkDevice device, VulkanH_Window *wd,
                                  uint32_t queue_family,
                                  const VkAllocationCallbacks *allocator,
                                  int width, int height,
                                  uint32_t min_image_count) {
  ASSERT(g_FunctionsLoaded &&
         "Need to call Vulkan_LoadFunctions() if "
         "VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");
  (void)instance;
  VulkanH_CreateWindowSwapChain(physical_device, device, wd, allocator, width,
                                height, min_image_count);
  // Vulkan_CreatePipeline(device, allocator, VK_NULL_HANDLE,
  // wd->RenderPass, VK_SAMPLE_COUNT_1_BIT, &wd->Pipeline,
  // g_VulkanInitInfo.Subpass);
  VulkanH_CreateWindowCommandBuffers(physical_device, device, wd, queue_family,
                                     allocator);
}

void VulkanH_DestroyWindow(VkInstance instance, VkDevice device,
                           VulkanH_Window *wd,
                           const VkAllocationCallbacks *allocator) {
  vkDeviceWaitIdle(
      device); // FIXME: We could wait on the Queue if we had the queue in wd->
               // (otherwise VulkanH functions can't use globals)
  // vkQueueWaitIdle(bd->Queue);

  for (uint32_t i = 0; i < wd->ImageCount; i++) {
    VulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
    VulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
  }
  FREE(wd->Frames);
  FREE(wd->FrameSemaphores);
  wd->Frames = nullptr;
  wd->FrameSemaphores = nullptr;
  vkDestroyPipeline(device, wd->Pipeline, allocator);
  vkDestroyRenderPass(device, wd->RenderPass, allocator);
  vkDestroySwapchainKHR(device, wd->Swapchain, allocator);
  vkDestroySurfaceKHR(instance, wd->Surface, allocator);

  *wd = VulkanH_Window();
}

void VulkanH_DestroyFrame(VkDevice device, VulkanH_Frame *fd,
                          const VkAllocationCallbacks *allocator) {
  vkDestroyFence(device, fd->Fence, allocator);
  vkFreeCommandBuffers(device, fd->CommandPool, 1, &fd->CommandBuffer);
  vkDestroyCommandPool(device, fd->CommandPool, allocator);
  fd->Fence = VK_NULL_HANDLE;
  fd->CommandBuffer = VK_NULL_HANDLE;
  fd->CommandPool = VK_NULL_HANDLE;

  vkDestroyImageView(device, fd->BackbufferView, allocator);
  vkDestroyFramebuffer(device, fd->Framebuffer, allocator);
}

void VulkanH_DestroyFrameSemaphores(VkDevice device,
                                    VulkanH_FrameSemaphores *fsd,
                                    const VkAllocationCallbacks *allocator) {
  vkDestroySemaphore(device, fsd->ImageAcquiredSemaphore, allocator);
  vkDestroySemaphore(device, fsd->RenderCompleteSemaphore, allocator);
  fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
}

void VulkanH_DestroyFrameRenderBuffers(VkDevice device,
                                       VulkanH_FrameRenderBuffers *buffers,
                                       const VkAllocationCallbacks *allocator) {
  if (buffers->VertexBuffer) {
    vkDestroyBuffer(device, buffers->VertexBuffer, allocator);
    buffers->VertexBuffer = VK_NULL_HANDLE;
  }
  if (buffers->VertexBufferMemory) {
    vkFreeMemory(device, buffers->VertexBufferMemory, allocator);
    buffers->VertexBufferMemory = VK_NULL_HANDLE;
  }
  if (buffers->IndexBuffer) {
    vkDestroyBuffer(device, buffers->IndexBuffer, allocator);
    buffers->IndexBuffer = VK_NULL_HANDLE;
  }
  if (buffers->IndexBufferMemory) {
    vkFreeMemory(device, buffers->IndexBufferMemory, allocator);
    buffers->IndexBufferMemory = VK_NULL_HANDLE;
  }
  buffers->VertexBufferSize = 0;
  buffers->IndexBufferSize = 0;
}

void VulkanH_DestroyWindowRenderBuffers(
    VkDevice device, VulkanH_WindowRenderBuffers *buffers,
    const VkAllocationCallbacks *allocator) {
  for (uint32_t n = 0; n < buffers->Count; n++)
    VulkanH_DestroyFrameRenderBuffers(device, &buffers->FrameRenderBuffers[n],
                                      allocator);
  FREE(buffers->FrameRenderBuffers);
  buffers->FrameRenderBuffers = nullptr;
  buffers->Index = 0;
  buffers->Count = 0;
}

void VulkanH_DestroyAllViewportsRenderBuffers(
    VkDevice device, const VkAllocationCallbacks *allocator) {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  for (int n = 0; n < platform_io.Viewports.Size; n++)
    if (Vulkan_ViewportData *vd =
            (Vulkan_ViewportData *)platform_io.Viewports[n]->RendererUserData)
      VulkanH_DestroyWindowRenderBuffers(device, &vd->RenderBuffers, allocator);
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create
// and handle multiple viewports simultaneously. If you are new to gui or
// creating a new binding for gui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void Vulkan_CreateWindow(Viewport *viewport) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_ViewportData *vd = NEW(Vulkan_ViewportData)();
  viewport->RendererUserData = vd;
  VulkanH_Window *wd = &vd->Window;
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;

  // Create surface
  PlatformIO &platform_io = Gui::GetPlatformIO();
  VkResult err = (VkResult)platform_io.Platform_CreateVkSurface(
      viewport, (unsigned long long)v->Instance, (const void *)v->Allocator,
      (unsigned long long *)&wd->Surface);
  check_vk_result(err);

  // Check for WSI support
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(v->PhysicalDevice, v->QueueFamily,
                                       wd->Surface, &res);
  if (res != VK_TRUE) {
    ASSERT(0); // Error: no WSI support on physical device
    return;
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = {
#if defined(VULKAN_HAS_DYNAMIC_RENDERING)
    v->UseDynamicRendering && v->ColorAttachmentFormat
        ? v->ColorAttachmentFormat
        : VK_FORMAT_B8G8R8A8_UNORM,
#endif
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_B8G8R8_UNORM,
    VK_FORMAT_R8G8B8_UNORM
  };
  const VkColorSpaceKHR requestSurfaceColorSpace =
      VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  wd->SurfaceFormat = VulkanH_SelectSurfaceFormat(
      v->PhysicalDevice, wd->Surface, requestSurfaceImageFormat,
      (size_t)ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

  // Select Present Mode
  // FIXME-VULKAN: Even thought mailbox seems to get us maximum framerate with a
  // single window, it halves framerate with a second window etc. (w/ Nvidia and
  // SDK 1.82.1)
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR,
                                      VK_PRESENT_MODE_IMMEDIATE_KHR,
                                      VK_PRESENT_MODE_FIFO_KHR};
  wd->PresentMode =
      VulkanH_SelectPresentMode(v->PhysicalDevice, wd->Surface,
                                &present_modes[0], ARRAYSIZE(present_modes));
  // printf("[vulkan] Secondary window selected PresentMode = %d\n",
  // wd->PresentMode);

  // Create SwapChain, RenderPass, Framebuffer, etc.
  wd->ClearEnable =
      (viewport->Flags & ViewportFlags_NoRendererClear) ? false : true;
  wd->UseDynamicRendering = v->UseDynamicRendering;
  VulkanH_CreateOrResizeWindow(v->Instance, v->PhysicalDevice, v->Device, wd,
                               v->QueueFamily, v->Allocator,
                               (int)viewport->Size.x, (int)viewport->Size.y,
                               v->MinImageCount);
  vd->WindowOwned = true;
}

static void Vulkan_DestroyWindow(Viewport *viewport) {
  // The main viewport (owned by the application) will always have
  // RendererUserData == 0 since we didn't create the data for it.
  Vulkan_Data *bd = Vulkan_GetBackendData();
  if (Vulkan_ViewportData *vd =
          (Vulkan_ViewportData *)viewport->RendererUserData) {
    Vulkan_InitInfo *v = &bd->VulkanInitInfo;
    if (vd->WindowOwned)
      VulkanH_DestroyWindow(v->Instance, v->Device, &vd->Window, v->Allocator);
    VulkanH_DestroyWindowRenderBuffers(v->Device, &vd->RenderBuffers,
                                       v->Allocator);
    DELETE(vd);
  }
  viewport->RendererUserData = nullptr;
}

static void Vulkan_SetWindowSize(Viewport *viewport, Vec2 size) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_ViewportData *vd = (Vulkan_ViewportData *)viewport->RendererUserData;
  if (vd == nullptr) // This is nullptr for the main viewport (which is left to
                     // the user/app to handle)
    return;
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  vd->Window.ClearEnable =
      (viewport->Flags & ViewportFlags_NoRendererClear) ? false : true;
  VulkanH_CreateOrResizeWindow(v->Instance, v->PhysicalDevice, v->Device,
                               &vd->Window, v->QueueFamily, v->Allocator,
                               (int)size.x, (int)size.y, v->MinImageCount);
}

static void Vulkan_RenderWindow(Viewport *viewport, void *) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_ViewportData *vd = (Vulkan_ViewportData *)viewport->RendererUserData;
  VulkanH_Window *wd = &vd->Window;
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;
  VkResult err;

  VulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
  VulkanH_FrameSemaphores *fsd = &wd->FrameSemaphores[wd->SemaphoreIndex];
  {
    {
      err = vkAcquireNextImageKHR(v->Device, wd->Swapchain, UINT64_MAX,
                                  fsd->ImageAcquiredSemaphore, VK_NULL_HANDLE,
                                  &wd->FrameIndex);
      check_vk_result(err);
      fd = &wd->Frames[wd->FrameIndex];
    }
    for (;;) {
      err = vkWaitForFences(v->Device, 1, &fd->Fence, VK_TRUE, 100);
      if (err == VK_SUCCESS)
        break;
      if (err == VK_TIMEOUT)
        continue;
      check_vk_result(err);
    }
    {
      err = vkResetCommandPool(v->Device, fd->CommandPool, 0);
      check_vk_result(err);
      VkCommandBufferBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
      check_vk_result(err);
    }
    {
      Vec4 clear_color = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
      memcpy(&wd->ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
    }
#ifdef VULKAN_HAS_DYNAMIC_RENDERING
    if (v->UseDynamicRendering) {
      // Transition swapchain image to a layout suitable for drawing.
      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      barrier.image = fd->Backbuffer;
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(fd->CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);

      VkRenderingAttachmentInfo attachmentInfo = {};
      attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
      attachmentInfo.imageView = fd->BackbufferView;
      attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
      attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachmentInfo.clearValue = wd->ClearValue;

      VkRenderingInfo renderingInfo = {};
      renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
      renderingInfo.renderArea.extent.width = wd->Width;
      renderingInfo.renderArea.extent.height = wd->Height;
      renderingInfo.layerCount = 1;
      renderingInfo.viewMask = 0;
      renderingInfo.colorAttachmentCount = 1;
      renderingInfo.pColorAttachments = &attachmentInfo;

      VulkanFuncs_vkCmdBeginRenderingKHR(fd->CommandBuffer, &renderingInfo);
    } else
#endif
    {
      VkRenderPassBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      info.renderPass = wd->RenderPass;
      info.framebuffer = fd->Framebuffer;
      info.renderArea.extent.width = wd->Width;
      info.renderArea.extent.height = wd->Height;
      info.clearValueCount =
          (viewport->Flags & ViewportFlags_NoRendererClear) ? 0 : 1;
      info.pClearValues = (viewport->Flags & ViewportFlags_NoRendererClear)
                              ? nullptr
                              : &wd->ClearValue;
      vkCmdBeginRenderPass(fd->CommandBuffer, &info,
                           VK_SUBPASS_CONTENTS_INLINE);
    }
  }

  Vulkan_RenderDrawData(viewport->DrawData, fd->CommandBuffer, wd->Pipeline);

  {
#ifdef VULKAN_HAS_DYNAMIC_RENDERING
    if (v->UseDynamicRendering) {
      VulkanFuncs_vkCmdEndRenderingKHR(fd->CommandBuffer);

      // Transition image to a layout suitable for presentation
      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      barrier.image = fd->Backbuffer;
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(fd->CommandBuffer,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrier);
    } else
#endif
    {
      vkCmdEndRenderPass(fd->CommandBuffer);
    }
    {
      VkPipelineStageFlags wait_stage =
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkSubmitInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      info.waitSemaphoreCount = 1;
      info.pWaitSemaphores = &fsd->ImageAcquiredSemaphore;
      info.pWaitDstStageMask = &wait_stage;
      info.commandBufferCount = 1;
      info.pCommandBuffers = &fd->CommandBuffer;
      info.signalSemaphoreCount = 1;
      info.pSignalSemaphores = &fsd->RenderCompleteSemaphore;

      err = vkEndCommandBuffer(fd->CommandBuffer);
      check_vk_result(err);
      err = vkResetFences(v->Device, 1, &fd->Fence);
      check_vk_result(err);
      err = vkQueueSubmit(v->Queue, 1, &info, fd->Fence);
      check_vk_result(err);
    }
  }
}

static void Vulkan_SwapBuffers(Viewport *viewport, void *) {
  Vulkan_Data *bd = Vulkan_GetBackendData();
  Vulkan_ViewportData *vd = (Vulkan_ViewportData *)viewport->RendererUserData;
  VulkanH_Window *wd = &vd->Window;
  Vulkan_InitInfo *v = &bd->VulkanInitInfo;

  VkResult err;
  uint32_t present_index = wd->FrameIndex;

  VulkanH_FrameSemaphores *fsd = &wd->FrameSemaphores[wd->SemaphoreIndex];
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &fsd->RenderCompleteSemaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &wd->Swapchain;
  info.pImageIndices = &present_index;
  err = vkQueuePresentKHR(v->Queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    VulkanH_CreateOrResizeWindow(v->Instance, v->PhysicalDevice, v->Device,
                                 &vd->Window, v->QueueFamily, v->Allocator,
                                 (int)viewport->Size.x, (int)viewport->Size.y,
                                 v->MinImageCount);
  else
    check_vk_result(err);

  wd->FrameIndex = (wd->FrameIndex + 1) %
                   wd->ImageCount; // This is for the next vkWaitForFences()
  wd->SemaphoreIndex =
      (wd->SemaphoreIndex + 1) %
      wd->ImageCount; // Now we can use the next set of semaphores
}

void Vulkan_InitPlatformInterface() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  if (Gui::GetIO().ConfigFlags & ConfigFlags_ViewportsEnable)
    ASSERT(platform_io.Platform_CreateVkSurface != nullptr &&
           "Platform needs to setup the CreateVkSurface handler.");
  platform_io.Renderer_CreateWindow = Vulkan_CreateWindow;
  platform_io.Renderer_DestroyWindow = Vulkan_DestroyWindow;
  platform_io.Renderer_SetWindowSize = Vulkan_SetWindowSize;
  platform_io.Renderer_RenderWindow = Vulkan_RenderWindow;
  platform_io.Renderer_SwapBuffers = Vulkan_SwapBuffers;
}

void Vulkan_ShutdownPlatformInterface() { Gui::DestroyPlatformWindows(); }

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
