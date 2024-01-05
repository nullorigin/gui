// dear imgui: Renderer Backend for OpenGL2 (legacy OpenGL, fixed pipeline)
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32,
// custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier
//  as void*/TextureID. Read the FAQ about TextureID! [X] Renderer:
//  Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |=
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

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS,
// VBO, VAO, etc.)**
// **Prefer using the code in opengl3.cpp**
// This code is mostly provided as a reference to learn how Gui integration
// works, because it is shorter to read. If your code is using GL3+ context or
// any semi modern OpenGL calls, using this is likely to make everything more
// complicated, will require your code to reset every single OpenGL attributes
// to their initial state, and might confuse your GPU driver. The GL2 code is
// unable to reset attributes or even call e.g. "glUseProgram(0)" because they
// don't exist in that API.

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
