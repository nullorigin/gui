// gui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D11ShaderResourceView*' as
//  TextureID. Read the FAQ about TextureID! [X] Renderer: Large meshes
//  support (64k+ vertices) with 16-bit indices. [X] Renderer: Multi-viewport
//  support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct ID3D11Device;
struct ID3D11DeviceContext;

API bool DX11_Init(ID3D11Device *device, ID3D11DeviceContext *device_context);
API void DX11_Shutdown();
API void DX11_NewFrame();
API void DX11_RenderDrawData(DrawData *draw_data);

// Use if you want to reset your rendering device without losing Gui
// state.
API void DX11_InvalidateDeviceObjects();
API bool DX11_CreateDeviceObjects();

#endif // #ifndef DISABLE
