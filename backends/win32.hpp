// gui: Platform Backend for Windows (standard windows API for 32-bits
// AND 64-bits applications) This needs to be used along with a Renderer (e.g.
// DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core
//  gui) [X] Platform: Mouse support. Can discriminate
//  Mouse/TouchScreen/Pen. [X] Platform: Keyboard support. Since 1.87 we are
//  using the io.AddKeyEvent() function. Pass Key values to all key
//  functions e.g. Gui::IsKeyPressed(Key_Space). [Legacy VK_* values will
//  also be supported unless DISABLE_OBSOLETE_KEYIO is set] [X] Platform:
//  Gamepad support. Enabled with 'io.ConfigFlags |=
//  ConfigFlags_NavEnableGamepad'. [X] Platform: Mouse cursor shape and
//  visibility. Disable with 'io.ConfigFlags |=
//  ConfigFlags_NoMouseCursorChange'. [X] Platform: Multi-viewport support
//  (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

API bool Win32_Init(void *hwnd);
API bool Win32_InitForOpenGL(void *hwnd);
API void Win32_Shutdown();
API void Win32_NewFrame();

// Win32 message handler your application need to call.
// - Intentionally commented out in a '#if 0' block to avoid dragging
// dependencies on <windows.h> from this helper.
// - You should COPY the line below into your .cpp code to forward declare the
// function and then you can call it.
// - Call from your application's message handler. Keep calling your message
// handler unless this function returns TRUE.

#if 0
extern API LRESULT Win32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

// DPI-related helpers (optional)
// - Use to enable DPI awareness without having to create an application
// manifest.
// - Your own app may already do this via a manifest or explicit calls. This is
// mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as
// SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK
//   at compile time and Windows 8/10+ at runtime, neither we want to require
//   the user to have. So we dynamically select and load those functions to
//   avoid dependencies.
API void Win32_EnableDpiAwareness();
API float Win32_GetDpiScaleForHwnd(void *hwnd);       // HWND hwnd
API float Win32_GetDpiScaleForMonitor(void *monitor); // HMONITOR monitor

// Transparency related helpers (optional) [experimental]
// - Use to enable alpha compositing transparency with the desktop.
// - Use together with e.g. clearing your framebuffer with zero-alpha.
API void Win32_EnableAlphaCompositing(void *hwnd); // HWND hwnd

#endif // #ifndef DISABLE
