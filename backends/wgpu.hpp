// gui: Renderer for WebGPU
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

// Use if you want to reset your rendering device without losing Gui
// state.
API void WGPU_InvalidateDeviceObjects();
API bool WGPU_CreateDeviceObjects();

#endif // #ifndef DISABLE
