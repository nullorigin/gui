// dear imgui: Renderer Backend for DirectX9
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as TextureID.
//  Read the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices. [X] Renderer: Multi-viewport support
//  (multiple windows). Enable with 'io.ConfigFlags |=
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

struct IDirect3DDevice9;

API bool DX9_Init(IDirect3DDevice9 *device);
API void DX9_Shutdown();
API void DX9_NewFrame();
API void DX9_RenderDrawData(DrawData *draw_data);

// Use if you want to reset your rendering device without losing Dear Gui
// state.
API bool DX9_CreateDeviceObjects();
API void DX9_InvalidateDeviceObjects();

#endif // #ifndef DISABLE
