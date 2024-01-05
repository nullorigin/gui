// gui: Renderer Backend for OpenGL2 (legacy OpenGL, fixed pipeline)
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32,
// custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier
//  as void*/TextureID. Read the FAQ about TextureID! [X] Renderer:
//  Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

API bool OpenGL2_Init();
API void OpenGL2_Shutdown();
API void OpenGL2_NewFrame();
API void OpenGL2_RenderDrawData(DrawData *draw_data);

// Called by Init/NewFrame/Shutdown
API bool OpenGL2_CreateFontsTexture();
API void OpenGL2_DestroyFontsTexture();
API bool OpenGL2_CreateDeviceObjects();
API void OpenGL2_DestroyDeviceObjects();

#endif // #ifndef DISABLE
