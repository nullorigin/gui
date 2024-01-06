// gui: Renderer Backend for SDL_Renderer for SDL2
// (Requires: SDL 2.0.17+)

// Note how SDL_Renderer is an _optional_ component of SDL2.
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

#include "../gui.hpp"
#ifndef DISABLE
#include "sdlrenderer2.hpp"
#include <stdint.h> // intptr_t

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wsign-conversion" // warning: implicit conversion changes signedness
#endif

// SDL
#include <SDL2/SDL.h>
#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// SDL_Renderer data
struct SDLRenderer2_Data {
  SDL_Renderer *SDLRenderer;
  SDL_Texture *FontTexture;
  SDLRenderer2_Data() { memset((void *)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendRendererUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
static SDLRenderer2_Data *SDLRenderer2_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (SDLRenderer2_Data *)Gui::GetIO().BackendRendererUserData
             : nullptr;
}

// Functions
bool SDLRenderer2_Init(SDL_Renderer *renderer) {
  IO &io = Gui::GetIO();
  ASSERT(io.BackendRendererUserData == nullptr &&
         "Already initialized a renderer backend!");
  ASSERT(renderer != nullptr && "SDL_Renderer not initialized!");

  // Setup backend capabilities flags
  SDLRenderer2_Data *bd = NEW(SDLRenderer2_Data)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "sdlrenderer2";
  io.BackendFlags |=
      BackendFlags_RendererHasVtxOffset; // We can honor the
                                         // DrawCmd::VtxOffset field,
                                         // allowing for large meshes.

  bd->SDLRenderer = renderer;

  return true;
}

void SDLRenderer2_Shutdown() {
  SDLRenderer2_Data *bd = SDLRenderer2_GetBackendData();
  ASSERT(bd != nullptr &&
         "No renderer backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  SDLRenderer2_DestroyDeviceObjects();

  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &= ~BackendFlags_RendererHasVtxOffset;
  DELETE(bd);
}

static void SDLRenderer2_SetupRenderState() {
  SDLRenderer2_Data *bd = SDLRenderer2_GetBackendData();

  // Clear out any viewports and cliprect set by the user
  // FIXME: Technically speaking there are lots of other things we could
  // backup/setup/restore during our render process.
  SDL_RenderSetViewport(bd->SDLRenderer, nullptr);
  SDL_RenderSetClipRect(bd->SDLRenderer, nullptr);
}

void SDLRenderer2_NewFrame() {
  SDLRenderer2_Data *bd = SDLRenderer2_GetBackendData();
  ASSERT(bd != nullptr && "Did you call SDLRenderer2_Init()?");

  if (!bd->FontTexture)
    SDLRenderer2_CreateDeviceObjects();
}

void SDLRenderer2_RenderDrawData(DrawData *draw_data) {
  SDLRenderer2_Data *bd = SDLRenderer2_GetBackendData();

  // If there's a scale factor set by the user, use that instead
  // If the user has specified a scale factor to SDL_Renderer already via
  // SDL_RenderSetScale(), SDL will scale whatever we pass to
  // SDL_RenderGeometryRaw() by that scale factor. In that case we don't want to
  // be also scaling it ourselves here.
  float rsx = 1.0f;
  float rsy = 1.0f;
  SDL_RenderGetScale(bd->SDLRenderer, &rsx, &rsy);
  Vec2 render_scale;
  render_scale.x = (rsx == 1.0f) ? draw_data->FramebufferScale.x : 1.0f;
  render_scale.y = (rsy == 1.0f) ? draw_data->FramebufferScale.y : 1.0f;

  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width = (int)(draw_data->DisplaySize.x * render_scale.x);
  int fb_height = (int)(draw_data->DisplaySize.y * render_scale.y);
  if (fb_width == 0 || fb_height == 0)
    return;

  // Backup SDL_Renderer state that will be modified to restore it afterwards
  struct BackupSDLRendererState {
    SDL_Rect Viewport;
    bool ClipEnabled;
    SDL_Rect ClipRect;
  };
  BackupSDLRendererState old = {};
  old.ClipEnabled = SDL_RenderIsClipEnabled(bd->SDLRenderer) == SDL_TRUE;
  SDL_RenderGetViewport(bd->SDLRenderer, &old.Viewport);
  SDL_RenderGetClipRect(bd->SDLRenderer, &old.ClipRect);

  // Will project scissor/clipping rectangles into framebuffer space
  Vec2 clip_off = draw_data->DisplayPos; // (0,0) unless using multi-viewports
  Vec2 clip_scale = render_scale;

  // Render command lists
  SDLRenderer2_SetupRenderState();
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    const DrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
    const DrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const DrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          SDLRenderer2_SetupRenderState();
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        Vec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
        Vec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
        if (clip_min.x < 0.0f) {
          clip_min.x = 0.0f;
        }
        if (clip_min.y < 0.0f) {
          clip_min.y = 0.0f;
        }
        if (clip_max.x > (float)fb_width) {
          clip_max.x = (float)fb_width;
        }
        if (clip_max.y > (float)fb_height) {
          clip_max.y = (float)fb_height;
        }
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
          continue;

        SDL_Rect r = {(int)(clip_min.x), (int)(clip_min.y),
                      (int)(clip_max.x - clip_min.x),
                      (int)(clip_max.y - clip_min.y)};
        SDL_RenderSetClipRect(bd->SDLRenderer, &r);

        const float *xy =
            (const float *)(const void *)((const char *)(vtx_buffer +
                                                         pcmd->VtxOffset) +
                                          offsetof(DrawVert, pos));
        const float *uv =
            (const float *)(const void *)((const char *)(vtx_buffer +
                                                         pcmd->VtxOffset) +
                                          offsetof(DrawVert, uv));
#if SDL_VERSION_ATLEAST(2, 0, 19)
        const SDL_Color *color =
            (const SDL_Color *)(const void *)((const char *)(vtx_buffer +
                                                             pcmd->VtxOffset) +
                                              offsetof(DrawVert,
                                                       col)); // SDL 2.0.19+
#else
        const int *color =
            (const int *)(const void *)((const char *)(vtx_buffer +
                                                       pcmd->VtxOffset) +
                                        offsetof(DrawVert,
                                                 col)); // SDL 2.0.17 and 2.0.18
#endif

        // Bind texture, Draw
        SDL_Texture *tex = (SDL_Texture *)pcmd->GetTexID();
        SDL_RenderGeometryRaw(
            bd->SDLRenderer, tex, xy, (int)sizeof(DrawVert), color,
            (int)sizeof(DrawVert), uv, (int)sizeof(DrawVert),
            cmd_list->VtxBuffer.Size - pcmd->VtxOffset,
            idx_buffer + pcmd->IdxOffset, pcmd->ElemCount, sizeof(DrawIdx));
      }
    }
  }

  // Restore modified SDL_Renderer state
  SDL_RenderSetViewport(bd->SDLRenderer, &old.Viewport);
  SDL_RenderSetClipRect(bd->SDLRenderer,
                        old.ClipEnabled ? &old.ClipRect : nullptr);
}

// Called by Init/NewFrame/Shutdown
bool SDLRenderer2_CreateFontsTexture() {
  IO &io = Gui::GetIO();
  SDLRenderer2_Data *bd = SDLRenderer2_GetBackendData();

  // Build texture atlas
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
  bd->FontTexture = SDL_CreateTexture(bd->SDLRenderer, SDL_PIXELFORMAT_ABGR8888,
                                      SDL_TEXTUREACCESS_STATIC, width, height);
  if (bd->FontTexture == nullptr) {
    SDL_Log("error creating texture");
    return false;
  }
  SDL_UpdateTexture(bd->FontTexture, nullptr, pixels, 4 * width);
  SDL_SetTextureBlendMode(bd->FontTexture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(bd->FontTexture, SDL_ScaleModeLinear);

  // Store our identifier
  io.Fonts->SetTexID((TextureID)(intptr_t)bd->FontTexture);

  return true;
}

void SDLRenderer2_DestroyFontsTexture() {
  IO &io = Gui::GetIO();
  SDLRenderer2_Data *bd = SDLRenderer2_GetBackendData();
  if (bd->FontTexture) {
    io.Fonts->SetTexID(0);
    SDL_DestroyTexture(bd->FontTexture);
    bd->FontTexture = nullptr;
  }
}

bool SDLRenderer2_CreateDeviceObjects() {
  return SDLRenderer2_CreateFontsTexture();
}

void SDLRenderer2_DestroyDeviceObjects() { SDLRenderer2_DestroyFontsTexture(); }

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef DISABLE
