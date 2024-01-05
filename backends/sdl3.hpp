// gui: Platform Backend for SDL3 (*EXPERIMENTAL*)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3,
// Vulkan..) (Info: SDL3 is a cross-platform general purpose library for
// handling windows, inputs, graphics context creation, etc.) (IMPORTANT:
// SDL 3.0.0 is NOT YET RELEASED. IT IS POSSIBLE THAT ITS SPECS/API WILL CHANGE
// BEFORE RELEASE)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent()
//  function. Pass Key values to all key functions e.g.
//  Gui::IsKeyPressed(Key_Space). [Legacy SDL_SCANCODE_* values will also
//  be supported unless DISABLE_OBSOLETE_KEYIO is set] [X] Platform: Gamepad
//  support. Enabled with 'io.ConfigFlags |= ConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with
//  'io.ConfigFlags |= ConfigFlags_NoMouseCursorChange'. [x] Platform:
//  Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable' -> the OS animation effect when window
//  gets created/destroyed is problematic. SDL2 backend doesn't have issue.
// Missing features:
//  [ ] Platform: Multi-viewport + Minimized windows seems to break mouse wheel
//  events (at least under Windows). [x] Platform: Basic IME support. Position
//  somehow broken in SDL3 + app needs to call
//  'SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");' before SDL_CreateWindow()!.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct SDL_Window;
struct SDL_Renderer;
typedef union SDL_Event SDL_Event;

API bool SDL3_InitForOpenGL(SDL_Window *window, void *sdl_gl_context);
API bool SDL3_InitForVulkan(SDL_Window *window);
API bool SDL3_InitForD3D(SDL_Window *window);
API bool SDL3_InitForMetal(SDL_Window *window);
API bool SDL3_InitForSDLRenderer(SDL_Window *window, SDL_Renderer *renderer);
API bool SDL3_InitForOther(SDL_Window *window);
API void SDL3_Shutdown();
API void SDL3_NewFrame();
API bool SDL3_ProcessEvent(const SDL_Event *event);

#endif // #ifndef DISABLE
