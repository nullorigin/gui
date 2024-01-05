// dear imgui: Renderer Backend for SDL_Renderer for SDL3
// (Requires: SDL 3.0.0+)

// Note how SDL_Renderer is an _optional_ component of SDL3.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and
// SDL+OpenGL on Linux/OSX. If your application will want to render any non
// trivial amount of graphics other than UI, please be aware that SDL_Renderer
// currently offers a limited graphic API to the end-user and it might be
// difficult to step out of those boundaries.

// Implemented features:
//  [X] Renderer: User texture binding. Use 'SDL_Texture*' as TextureID. Read
//  the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices.
// Missing features:
//  [ ] Renderer: Multi-viewport support (multiple windows).

// You can copy and use unmodified * files in your project. See
// examples/ folder for examples of using this. Prefer including the entire
// imgui/ repository into your project (either as a copy or as a submodule), and
// only build the backends you need. Learn about Dear Gui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of gui.cpp

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct SDL_Renderer;

API bool SDLRenderer3_Init(SDL_Renderer *renderer);
API void SDLRenderer3_Shutdown();
API void SDLRenderer3_NewFrame();
API void SDLRenderer3_RenderDrawData(DrawData *draw_data);

// Called by Init/NewFrame/Shutdown
API bool SDLRenderer3_CreateFontsTexture();
API void SDLRenderer3_DestroyFontsTexture();
API bool SDLRenderer3_CreateDeviceObjects();
API void SDLRenderer3_DestroyDeviceObjects();

#endif // #ifndef DISABLE
