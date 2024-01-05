// gui: Renderer Backend for DirectX10
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D10ShaderResourceView*' as
//  TextureID. Read the FAQ about TextureID! [X] Renderer: Large meshes
//  support (64k+ vertices) with 16-bit indices. [X] Renderer: Multi-viewport
//  support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct ID3D10Device;

API bool DX10_Init(ID3D10Device *device);
API void DX10_Shutdown();
API void DX10_NewFrame();
API void DX10_RenderDrawData(DrawData *draw_data);

// Use if you want to reset your rendering device without losing Gui
// state.
API void DX10_InvalidateDeviceObjects();
API bool DX10_CreateDeviceObjects();

#endif // #ifndef DISABLE
