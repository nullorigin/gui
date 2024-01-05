// dear imgui: Renderer for WebGPU
// This needs to be used along with a Platform Binding (e.g. GLFW)
// (Please note that WebGPU is currently experimental, will not run on non-beta
// browsers, and may break.)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'WGPUTextureView' as TextureID.
//  Read the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices.
// Missing features:
//  [ ] Renderer: Multi-viewport support (multiple windows). Not meaningful on
//  the web.

// You can use unmodified * files in your project. See examples/
// folder for examples of using this. Prefer including the entire imgui/
// repository into your project (either as a copy or as a submodule), and only
// build the backends you need. Learn about Dear Gui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of gui.cpp

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

#include <webgpu/webgpu.h>

API bool
WGPU_Init(WGPUDevice device, int num_frames_in_flight,
          WGPUTextureFormat rt_format,
          WGPUTextureFormat depth_format = WGPUTextureFormat_Undefined);
API void WGPU_Shutdown();
API void WGPU_NewFrame();
API void WGPU_RenderDrawData(DrawData *draw_data,
                             WGPURenderPassEncoder pass_encoder);

// Use if you want to reset your rendering device without losing Dear Gui
// state.
API void WGPU_InvalidateDeviceObjects();
API bool WGPU_CreateDeviceObjects();

#endif // #ifndef DISABLE
