// gui: Renderer Backend for DirectX12
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'D3D12_GPU_DESCRIPTOR_HANDLE' as
//  TextureID. Read the FAQ about TextureID! [X] Renderer: Large meshes
//  support (64k+ vertices) with 16-bit indices. [X] Renderer: Multi-viewport
//  support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

// Important: to compile on 32-bit systems, this backend requires code to be
// compiled with '#define TextureID unsigned long long'. See dx12.cpp file for
// details.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE
#include <dxgiformat.h> // DXGI_FORMAT

struct ID3D12Device;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

// cmd_list is the command list that the implementation will use to render imgui
// draw lists. Before calling the render function, caller must prepare cmd_list
// by resetting it and setting the appropriate render target and descriptor heap
// that contains font_srv_cpu_desc_handle/font_srv_gpu_desc_handle.
// font_srv_cpu_desc_handle and font_srv_gpu_desc_handle are handles to a single
// SRV descriptor to use for the internal font texture.
API bool DX12_Init(ID3D12Device *device, int num_frames_in_flight,
                   DXGI_FORMAT rtv_format, ID3D12DescriptorHeap *cbv_srv_heap,
                   D3D12_CPU_DESCRIPTOR_HANDLE font_srv_cpu_desc_handle,
                   D3D12_GPU_DESCRIPTOR_HANDLE font_srv_gpu_desc_handle);
API void DX12_Shutdown();
API void DX12_NewFrame();
API void DX12_RenderDrawData(DrawData *draw_data,
                             ID3D12GraphicsCommandList *graphics_command_list);

// Use if you want to reset your rendering device without losing Gui
// state.
API void DX12_InvalidateDeviceObjects();
API bool DX12_CreateDeviceObjects();

#endif // #ifndef DISABLE
