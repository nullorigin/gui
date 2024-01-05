// dear imgui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D11ShaderResourceView*' as
//  TextureID. Read the FAQ about TextureID! [X] Renderer: Large meshes
//  support (64k+ vertices) with 16-bit indices. [X] Renderer: Multi-viewport
//  support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

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

struct ID3D11Device;
struct ID3D11DeviceContext;

API bool DX11_Init(ID3D11Device *device, ID3D11DeviceContext *device_context);
API void DX11_Shutdown();
API void DX11_NewFrame();
API void DX11_RenderDrawData(DrawData *draw_data);

// Use if you want to reset your rendering device without losing Dear Gui
// state.
API void DX11_InvalidateDeviceObjects();
API bool DX11_CreateDeviceObjects();

#endif // #ifndef DISABLE
