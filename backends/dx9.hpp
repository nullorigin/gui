// gui: Renderer Backend for DirectX9
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as TextureID.
//  Read the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices. [X] Renderer: Multi-viewport support
//  (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct IDirect3DDevice9;

API bool DX9_Init(IDirect3DDevice9 *device);
API void DX9_Shutdown();
API void DX9_NewFrame();
API void DX9_RenderDrawData(DrawData *draw_data);

// Use if you want to reset your rendering device without losing Gui
// state.
API bool DX9_CreateDeviceObjects();
API void DX9_InvalidateDeviceObjects();

#endif // #ifndef DISABLE
