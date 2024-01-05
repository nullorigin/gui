// gui: Renderer Backend for modern OpenGL with shaders / programmatic
// pipeline
// - Desktop GL: 2.x 3.x 4.x
// - Embedded GL: ES 2.0 (WebGL 1.0), ES 3.0 (WebGL 2.0)
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32,
// custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier
//  as void*/TextureID. Read the FAQ about TextureID! [X] Renderer: Large
//  meshes support (64k+ vertices) with 16-bit indices (Desktop OpenGL only).
//  [X] Renderer: Multi-viewport support (multiple windows). Enable with
//  'io.ConfigFlags |= ConfigFlags_ViewportsEnable'.

// About WebGL/ES:
// - You need to '#define OPENGL_ES2' or '#define OPENGL_ES3' to use
// WebGL or OpenGL ES.
// - This is done automatically on iOS, Android and Emscripten targets.
// - For other targets, the define needs to be visible from the
// opengl3.cpp compilation unit. If unsure, define globally or in
// config.hpp.

// About GLSL version:
//  The 'glsl_version' initialization parameter should be nullptr (default) or a
//  "#version XXX" string. On computer platform the GLSL version default to
//  "#version 130". On OpenGL ES 3 platform it defaults to "#version 300 es"
//  Only override if your GL version doesn't handle this GLSL version. See GLSL
//  version table at the top of opengl3.cpp.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

// Backend API
API bool OpenGL3_Init(const char *glsl_version = nullptr);
API void OpenGL3_Shutdown();
API void OpenGL3_NewFrame();
API void OpenGL3_RenderDrawData(DrawData *draw_data);

// (Optional) Called by Init/NewFrame/Shutdown
API bool OpenGL3_CreateFontsTexture();
API void OpenGL3_DestroyFontsTexture();
API bool OpenGL3_CreateDeviceObjects();
API void OpenGL3_DestroyDeviceObjects();

// Specific OpenGL ES versions
// #define OPENGL_ES2     // Auto-detected on Emscripten
// #define OPENGL_ES3     // Auto-detected on iOS/Android

// You can explicitly select GLES2 or GLES3 API by using one of the '#define
// OPENGL_LOADER_XXX' in config.hpp or compiler command-line.
#if !defined(OPENGL_ES2) && !defined(OPENGL_ES3)

// Try to detect GLES on matching platforms
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) ||                 \
    (defined(__ANDROID__))
#define OPENGL_ES3 // iOS, Android  -> GL ES 3, "#version 300 es"
#elif defined(__EMSCRIPTEN__) || defined(__amigaos4__)
#define OPENGL_ES2 // Emscripten    -> GL ES 2, "#version 100"
#else
// Otherwise opengl3_loader.hpp will be used.
#endif

#endif

#endif // #ifndef DISABLE
