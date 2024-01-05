// gui: Platform Backend for SDL2
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3,
// Vulkan..) (Info: SDL2 is a cross-platform general purpose library for
// handling windows, inputs, graphics context creation, etc.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent()
//  function. Pass Key values to all key functions e.g.
//  Gui::IsKeyPressed(Key_Space). [Legacy SDL_SCANCODE_* values will also
//  be supported unless DISABLE_OBSOLETE_KEYIO is set] [X] Platform: Gamepad
//  support. Enabled with 'io.ConfigFlags |= ConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with
//  'io.ConfigFlags |= ConfigFlags_NoMouseCursorChange'. [X] Platform:
//  Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.
// Missing features:
//  [ ] Platform: Multi-viewport + Minimized windows seems to break mouse wheel
//  events (at least under Windows). [x] Platform: Basic IME support. App needs
//  to call 'SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");' before
//  SDL_CreateWindow()!.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct SDL_Window;
struct SDL_Renderer;
typedef union SDL_Event SDL_Event;

API bool SDL2_InitForOpenGL(SDL_Window *window, void *sdl_gl_context);
API bool SDL2_InitForVulkan(SDL_Window *window);
API bool SDL2_InitForD3D(SDL_Window *window);
API bool SDL2_InitForMetal(SDL_Window *window);
API bool SDL2_InitForSDLRenderer(SDL_Window *window, SDL_Renderer *renderer);
API bool SDL2_InitForOther(SDL_Window *window);
API void SDL2_Shutdown();
API void SDL2_NewFrame();
API bool SDL2_ProcessEvent(const SDL_Event *event);

#ifndef DISABLE_OBSOLETE_FUNCTIONS
static inline void SDL2_NewFrame(SDL_Window *) {
  SDL2_NewFrame();
} // 1.84: removed unnecessary parameter
#endif

#endif // #ifndef DISABLE
