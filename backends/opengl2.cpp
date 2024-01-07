// gui: Renderer Backend for OpenGL2 (legacy OpenGL, fixed pipeline)
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32,
// custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier
//  as void*/TextureID. Read the FAQ about TextureID! [X] Renderer:
//  Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#include "../gui.hpp"
#ifndef DISABLE
#include "opengl2.hpp"
#include <stdint.h> // intptr_t

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros" // warning: macro is not used
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#endif

// Include OpenGL header (without an OpenGL loader) requires a bit of fiddling
#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY                                                               \
  __stdcall // It is customary to use APIENTRY for OpenGL function pointer
            // declarations on all platforms.  Additionally, the Windows OpenGL
            // header needs APIENTRY.
#endif
#if defined(_WIN32) && !defined(WINGDIAPI)
#define WINGDIAPI __declspec(dllimport) // Some Windows OpenGL headers need this
#endif
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

struct OpenGL2_Data {
  GLuint FontTexture;

  OpenGL2_Data() { memset((void *)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendRendererUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
static OpenGL2_Data *OpenGL2_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (OpenGL2_Data *)Gui::GetIO().BackendRendererUserData
             : nullptr;
}

// Forward Declarations
static void OpenGL2_InitPlatformInterface();
static void OpenGL2_ShutdownPlatformInterface();

// Functions
bool OpenGL2_Init() {
  IO &io = Gui::GetIO();
  assert(io.BackendRendererUserData == nullptr &&
         "Already initialized a renderer backend!");

  // Setup backend capabilities flags
  OpenGL2_Data *bd = NEW(OpenGL2_Data)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "opengl2";
  io.BackendFlags |=
      BackendFlags_RendererHasViewports; // We can create multi-viewports
                                         // on the Renderer side (optional)

  if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
    OpenGL2_InitPlatformInterface();

  return true;
}

void OpenGL2_Shutdown() {
  OpenGL2_Data *bd = OpenGL2_GetBackendData();
  assert(bd != nullptr &&
         "No renderer backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  OpenGL2_ShutdownPlatformInterface();
  OpenGL2_DestroyDeviceObjects();
  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &= ~BackendFlags_RendererHasViewports;
  DELETE(bd);
}

void OpenGL2_NewFrame() {
  OpenGL2_Data *bd = OpenGL2_GetBackendData();
  assert(bd != nullptr && "Did you call OpenGL2_Init()?");

  if (!bd->FontTexture)
    OpenGL2_CreateDeviceObjects();
}

static void OpenGL2_SetupRenderState(DrawData *draw_data, int fb_width,
                                     int fb_height) {
  // Setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
  // GL_ONE_MINUS_SRC_ALPHA); // In order to composite our output buffer we need
  // to preserve alpha
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_SCISSOR_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glShadeModel(GL_SMOOTH);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // If you are using this code with non-legacy OpenGL header/contexts (which
  // you should not, prefer using opengl3.cpp!!), you may need to
  // backup/reset/restore other state, e.g. for current shader using the
  // commented lines below. (DO NOT MODIFY THIS FILE! Add the code in your
  // calling function)
  //   GLint last_program;
  //   glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  //   glUseProgram(0);
  //   OpenGL2_RenderDrawData(...);
  //   glUseProgram(last_program)
  // There are potentially many more states you could need to clear/setup that
  // we can't access from default headers. e.g. glBindBuffer(GL_ARRAY_BUFFER,
  // 0), glDisable(GL_TEXTURE_CUBE_MAP).

  // Setup viewport, orthographic projection matrix
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is
  // (0,0) for single viewport apps.
  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(draw_data->DisplayPos.x,
          draw_data->DisplayPos.x + draw_data->DisplaySize.x,
          draw_data->DisplayPos.y + draw_data->DisplaySize.y,
          draw_data->DisplayPos.y, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

// OpenGL2 Render function.
// Note that this implementation is little overcomplicated because we are
// saving/setting up/restoring every OpenGL state explicitly. This is in order
// to be able to run within an OpenGL engine that doesn't do so.
void OpenGL2_RenderDrawData(DrawData *draw_data) {
  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width =
      (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fb_width == 0 || fb_height == 0)
    return;

  // Backup GL state
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_polygon_mode[2];
  glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4];
  glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLint last_shade_model;
  glGetIntegerv(GL_SHADE_MODEL, &last_shade_model);
  GLint last_tex_env_mode;
  glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &last_tex_env_mode);
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

  // Setup desired GL state
  OpenGL2_SetupRenderState(draw_data, fb_width, fb_height);

  // Will project scissor/clipping rectangles into framebuffer space
  Vec2 clip_off = draw_data->DisplayPos; // (0,0) unless using multi-viewports
  Vec2 clip_scale =
      draw_data->FramebufferScale; // (1,1) unless using retina display which
                                   // are often (2,2)

  // Render command lists
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    const DrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
    const DrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;
    glVertexPointer(
        2, GL_FLOAT, sizeof(DrawVert),
        (const GLvoid *)((const char *)vtx_buffer + offsetof(DrawVert, pos)));
    glTexCoordPointer(
        2, GL_FLOAT, sizeof(DrawVert),
        (const GLvoid *)((const char *)vtx_buffer + offsetof(DrawVert, uv)));
    glColorPointer(
        4, GL_UNSIGNED_BYTE, sizeof(DrawVert),
        (const GLvoid *)((const char *)vtx_buffer + offsetof(DrawVert, col)));

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const DrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          OpenGL2_SetupRenderState(draw_data, fb_width, fb_height);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        Vec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
        Vec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
          continue;

        // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
        glScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y),
                  (int)(clip_max.x - clip_min.x),
                  (int)(clip_max.y - clip_min.y));

        // Bind texture, Draw
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                       sizeof(DrawIdx) == 2 ? GL_UNSIGNED_SHORT
                                            : GL_UNSIGNED_INT,
                       idx_buffer + pcmd->IdxOffset);
      }
    }
  }

  // Restore modified GL state
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]);
  glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]);
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2],
             (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1],
            (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
  glShadeModel(last_shade_model);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, last_tex_env_mode);
}

bool OpenGL2_CreateFontsTexture() {
  // Build texture atlas
  IO &io = Gui::GetIO();
  OpenGL2_Data *bd = OpenGL2_GetBackendData();
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(
      &pixels, &width,
      &height); // Load as RGBA 32-bit (75% of the memory is wasted, but default
                // font is so small) because it is more likely to be compatible
                // with user's existing shaders. If your TextureId represent a
                // higher-level concept than just a GL texture id, consider
                // calling GetTexDataAsAlpha8() instead to save on GPU memory.

  // Upload texture to graphics system
  // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
  // FontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to
  // allow point/nearest sampling)
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &bd->FontTexture);
  glBindTexture(GL_TEXTURE_2D, bd->FontTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels);

  // Store our identifier
  io.Fonts->SetTexID((TextureID)(intptr_t)bd->FontTexture);

  // Restore state
  glBindTexture(GL_TEXTURE_2D, last_texture);

  return true;
}

void OpenGL2_DestroyFontsTexture() {
  IO &io = Gui::GetIO();
  OpenGL2_Data *bd = OpenGL2_GetBackendData();
  if (bd->FontTexture) {
    glDeleteTextures(1, &bd->FontTexture);
    io.Fonts->SetTexID(0);
    bd->FontTexture = 0;
  }
}

bool OpenGL2_CreateDeviceObjects() { return OpenGL2_CreateFontsTexture(); }

void OpenGL2_DestroyDeviceObjects() { OpenGL2_DestroyFontsTexture(); }

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create
// and handle multiple viewports simultaneously. If you are new to gui or
// creating a new binding for gui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void OpenGL2_RenderWindow(Viewport *viewport, void *) {
  if (!(viewport->Flags & ViewportFlags_NoRendererClear)) {
    Vec4 clear_color = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  OpenGL2_RenderDrawData(viewport->DrawData);
}

static void OpenGL2_InitPlatformInterface() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Renderer_RenderWindow = OpenGL2_RenderWindow;
}

static void OpenGL2_ShutdownPlatformInterface() {
  Gui::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef DISABLE
