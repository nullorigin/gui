// gui: Renderer + Platform Backend for Allegro 5
// (Info: Allegro 5 is a cross-platform general purpose library for handling
// windows, inputs, graphics, etc.)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ALLEGRO_BITMAP*' as TextureID.
//  Read the FAQ about TextureID! [X] Platform: Keyboard support. Since 1.87
//  we are using the io.AddKeyEvent() function. Pass Key values to all key
//  functions e.g. Gui::IsKeyPressed(Key_Space). [Legacy ALLEGRO_KEY_*
//  values will also be supported unless DISABLE_OBSOLETE_KEYIO is set] [X]
//  Platform: Clipboard support (from Allegro 5.1.12) [X] Platform: Mouse cursor
//  shape and visibility. Disable with 'io.ConfigFlags |=
//  ConfigFlags_NoMouseCursorChange'.
// Missing features:
//  [ ] Renderer: Multi-viewport support (multiple windows)..
//  [ ] Renderer: The renderer is suboptimal as we need to convert vertices
//  manually. [ ] Platform: Missing gamepad support.

#include "../gui.hpp"
#ifndef DISABLE
#include "allegro5.hpp"
#include <cstring>  // memcpy
#include <stdint.h> // uint64_t

// Allegro
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#ifdef _WIN32
#include <allegro5/allegro_windows.h>
#endif
#define ALLEGRO_HAS_CLIPBOARD                                                  \
  (ALLEGRO_VERSION_INT >=                                                      \
   ((5 << 24) | (1 << 16) |                                                    \
    (12 << 8))) // Clipboard only supported from Allegro 5.1.12
#define ALLEGRO_HAS_DRAW_INDEXED_PRIM                                          \
  (ALLEGRO_VERSION_INT >=                                                      \
   ((5 << 24) | (2 << 16) |                                                    \
    (5 << 8))) // DX9 implementation of al_draw_indexed_prim() got fixed in
               // Allegro 5.2.5

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning(disable : 4127) // condition expression is constant
#endif

struct DrawVertAllegro {
  Vec2 pos;
  Vec2 uv;
  ALLEGRO_COLOR col;
};

// FIXME-OPT: Unfortunately Allegro doesn't support 32-bit packed colors so we
// have to convert them to 4 float as well..
// FIXME-OPT: Consider inlining al_map_rgba()?
// see https://github.com/liballeg/allegro5/blob/master/src/pixels.c#L554
// and
// https://github.com/liballeg/allegro5/blob/master/include/allegro5/internal/aintern_pixels.h
#define DRAW_VERT_TO_ALLEGRO(DST, SRC)                                         \
  {                                                                            \
    (DST)->pos = (SRC)->pos;                                                   \
    (DST)->uv = (SRC)->uv;                                                     \
    unsigned char *c = (unsigned char *)&(SRC)->col;                           \
    (DST)->col = al_map_rgba(c[0], c[1], c[2], c[3]);                          \
  }

// Allegro Data
struct Allegro5_Data {
  ALLEGRO_DISPLAY *Display;
  ALLEGRO_BITMAP *Texture;
  double Time;
  ALLEGRO_MOUSE_CURSOR *MouseCursorInvisible;
  ALLEGRO_VERTEX_DECL *VertexDecl;
  char *ClipboardTextData;

  Vector<DrawVertAllegro> BufVertices;
  Vector<int> BufIndices;

  Allegro5_Data() { memset((void *)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in
// this backend.
static Allegro5_Data *Allegro5_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (Allegro5_Data *)Gui::GetIO().BackendPlatformUserData
             : nullptr;
}

static void Allegro5_SetupRenderState(DrawData *draw_data) {
  // Setup blending
  al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA,
                          ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

  // Setup orthographic projection matrix
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right).
  {
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_use_transform(&transform);
    al_orthographic_transform(&transform, L, T, 1.0f, R, B, -1.0f);
    al_use_projection_transform(&transform);
  }
}

// Render function.
void Allegro5_RenderDrawData(DrawData *draw_data) {
  // Avoid rendering when minimized
  if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
    return;

  // Backup Allegro state that will be modified
  Allegro5_Data *bd = Allegro5_GetBackendData();
  ALLEGRO_TRANSFORM last_transform = *al_get_current_transform();
  ALLEGRO_TRANSFORM last_projection_transform =
      *al_get_current_projection_transform();
  int last_clip_x, last_clip_y, last_clip_w, last_clip_h;
  al_get_clipping_rectangle(&last_clip_x, &last_clip_y, &last_clip_w,
                            &last_clip_h);
  int last_blender_op, last_blender_src, last_blender_dst;
  al_get_blender(&last_blender_op, &last_blender_src, &last_blender_dst);

  // Setup desired render state
  Allegro5_SetupRenderState(draw_data);

  // Render command lists
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];

    Vector<DrawVertAllegro> &vertices = bd->BufVertices;
#if ALLEGRO_HAS_DRAW_INDEXED_PRIM
    vertices.resize(cmd_list->VtxBuffer.Size);
    for (int i = 0; i < cmd_list->VtxBuffer.Size; i++) {
      const DrawVert *src_v = &cmd_list->VtxBuffer[i];
      DrawVertAllegro *dst_v = &vertices[i];
      DRAW_VERT_TO_ALLEGRO(dst_v, src_v);
    }
    const int *indices = nullptr;
    if (sizeof(DrawIdx) == 2) {
      // FIXME-OPT: Allegro doesn't support 16-bit indices.
      // You can '#define DrawIdx int' in config.hpp to request Gui to
      // output 32-bit indices. Otherwise, we convert them from 16-bit to 32-bit
      // at runtime here, which works perfectly but is a little wasteful.
      bd->BufIndices.resize(cmd_list->IdxBuffer.Size);
      for (int i = 0; i < cmd_list->IdxBuffer.Size; ++i)
        bd->BufIndices[i] = (int)cmd_list->IdxBuffer.Data[i];
      indices = bd->BufIndices.Data;
    } else if (sizeof(DrawIdx) == 4) {
      indices = (const int *)cmd_list->IdxBuffer.Data;
    }
#else
    // Allegro's implementation of al_draw_indexed_prim() for DX9 was broken
    // until 5.2.5. Unindex buffers ourselves while converting vertex format.
    vertices.resize(cmd_list->IdxBuffer.Size);
    for (int i = 0; i < cmd_list->IdxBuffer.Size; i++) {
      const DrawVert *src_v = &cmd_list->VtxBuffer[cmd_list->IdxBuffer[i]];
      DrawVertAllegro *dst_v = &vertices[i];
      DRAW_VERT_TO_ALLEGRO(dst_v, src_v);
    }
#endif

    // Render command lists
    Vec2 clip_off = draw_data->DisplayPos;
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const DrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          Allegro5_SetupRenderState(draw_data);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        Vec2 clip_min(pcmd->ClipRect.x - clip_off.x,
                      pcmd->ClipRect.y - clip_off.y);
        Vec2 clip_max(pcmd->ClipRect.z - clip_off.x,
                      pcmd->ClipRect.w - clip_off.y);
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
          continue;

        // Apply scissor/clipping rectangle, Draw
        ALLEGRO_BITMAP *texture = (ALLEGRO_BITMAP *)pcmd->GetTexID();
        al_set_clipping_rectangle(clip_min.x, clip_min.y,
                                  clip_max.x - clip_min.x,
                                  clip_max.y - clip_min.y);
#if ALLEGRO_HAS_DRAW_INDEXED_PRIM
        al_draw_indexed_prim(&vertices[0], bd->VertexDecl, texture,
                             &indices[pcmd->IdxOffset], pcmd->ElemCount,
                             ALLEGRO_PRTRIANGLE_LIST);
#else
        al_draw_prim(&vertices[0], bd->VertexDecl, texture, pcmd->IdxOffset,
                     pcmd->IdxOffset + pcmd->ElemCount,
                     ALLEGRO_PRTRIANGLE_LIST);
#endif
      }
    }
  }

  // Restore modified Allegro state
  al_set_blender(last_blender_op, last_blender_src, last_blender_dst);
  al_set_clipping_rectangle(last_clip_x, last_clip_y, last_clip_w, last_clip_h);
  al_use_transform(&last_transform);
  al_use_projection_transform(&last_projection_transform);
}

bool Allegro5_CreateDeviceObjects() {
  // Build texture atlas
  Allegro5_Data *bd = Allegro5_GetBackendData();
  IO &io = Gui::GetIO();
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  // Create texture
  // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
  // FontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to
  // allow point/nearest sampling)
  int flags = al_get_new_bitmap_flags();
  int fmt = al_get_new_bitmap_format();
  al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP | ALLEGRO_MIN_LINEAR |
                          ALLEGRO_MAG_LINEAR);
  al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE);
  ALLEGRO_BITMAP *img = al_create_bitmap(width, height);
  al_set_new_bitmap_flags(flags);
  al_set_new_bitmap_format(fmt);
  if (!img)
    return false;

  ALLEGRO_LOCKED_REGION *locked_img =
      al_lock_bitmap(img, al_get_bitmap_format(img), ALLEGRO_LOCK_WRITEONLY);
  if (!locked_img) {
    al_destroy_bitmap(img);
    return false;
  }
  memcpy(locked_img->data, pixels, sizeof(int) * width * height);
  al_unlock_bitmap(img);

  // Convert software texture to hardware texture.
  ALLEGRO_BITMAP *cloned_img = al_clone_bitmap(img);
  al_destroy_bitmap(img);
  if (!cloned_img)
    return false;

  // Store our identifier
  io.Fonts->SetTexID((TextureID)(intptr_t)cloned_img);
  bd->Texture = cloned_img;

  // Create an invisible mouse cursor
  // Because al_hide_mouse_cursor() seems to mess up with the actual inputs..
  ALLEGRO_BITMAP *mouse_cursor = al_create_bitmap(8, 8);
  bd->MouseCursorInvisible = al_create_mouse_cursor(mouse_cursor, 0, 0);
  al_destroy_bitmap(mouse_cursor);

  return true;
}

void Allegro5_InvalidateDeviceObjects() {
  IO &io = Gui::GetIO();
  Allegro5_Data *bd = Allegro5_GetBackendData();
  if (bd->Texture) {
    io.Fonts->SetTexID(0);
    al_destroy_bitmap(bd->Texture);
    bd->Texture = nullptr;
  }
  if (bd->MouseCursorInvisible) {
    al_destroy_mouse_cursor(bd->MouseCursorInvisible);
    bd->MouseCursorInvisible = nullptr;
  }
}

#if ALLEGRO_HAS_CLIPBOARD
static const char *Allegro5_GetClipboardText(void *) {
  Allegro5_Data *bd = Allegro5_GetBackendData();
  if (bd->ClipboardTextData)
    al_free(bd->ClipboardTextData);
  bd->ClipboardTextData = al_get_clipboard_text(bd->Display);
  return bd->ClipboardTextData;
}

static void Allegro5_SetClipboardText(void *, const char *text) {
  Allegro5_Data *bd = Allegro5_GetBackendData();
  al_set_clipboard_text(bd->Display, text);
}
#endif

static Key Allegro5_KeyCodeToKey(int key_code) {
  switch (key_code) {
  case ALLEGRO_KEY_TAB:
    return Key_Tab;
  case ALLEGRO_KEY_LEFT:
    return Key_LeftArrow;
  case ALLEGRO_KEY_RIGHT:
    return Key_RightArrow;
  case ALLEGRO_KEY_UP:
    return Key_UpArrow;
  case ALLEGRO_KEY_DOWN:
    return Key_DownArrow;
  case ALLEGRO_KEY_PGUP:
    return Key_PageUp;
  case ALLEGRO_KEY_PGDN:
    return Key_PageDown;
  case ALLEGRO_KEY_HOME:
    return Key_Home;
  case ALLEGRO_KEY_END:
    return Key_End;
  case ALLEGRO_KEY_INSERT:
    return Key_Insert;
  case ALLEGRO_KEY_DELETE:
    return Key_Delete;
  case ALLEGRO_KEY_BACKSPACE:
    return Key_Backspace;
  case ALLEGRO_KEY_SPACE:
    return Key_Space;
  case ALLEGRO_KEY_ENTER:
    return Key_Enter;
  case ALLEGRO_KEY_ESCAPE:
    return Key_Escape;
  case ALLEGRO_KEY_QUOTE:
    return Key_Apostrophe;
  case ALLEGRO_KEY_COMMA:
    return Key_Comma;
  case ALLEGRO_KEY_MINUS:
    return Key_Minus;
  case ALLEGRO_KEY_FULLSTOP:
    return Key_Period;
  case ALLEGRO_KEY_SLASH:
    return Key_Slash;
  case ALLEGRO_KEY_SEMICOLON:
    return Key_Semicolon;
  case ALLEGRO_KEY_EQUALS:
    return Key_Equal;
  case ALLEGRO_KEY_OPENBRACE:
    return Key_LeftBracket;
  case ALLEGRO_KEY_BACKSLASH:
    return Key_Backslash;
  case ALLEGRO_KEY_CLOSEBRACE:
    return Key_RightBracket;
  case ALLEGRO_KEY_TILDE:
    return Key_GraveAccent;
  case ALLEGRO_KEY_CAPSLOCK:
    return Key_CapsLock;
  case ALLEGRO_KEY_SCROLLLOCK:
    return Key_ScrollLock;
  case ALLEGRO_KEY_NUMLOCK:
    return Key_NumLock;
  case ALLEGRO_KEY_PRINTSCREEN:
    return Key_PrintScreen;
  case ALLEGRO_KEY_PAUSE:
    return Key_Pause;
  case ALLEGRO_KEY_PAD_0:
    return Key_Keypad0;
  case ALLEGRO_KEY_PAD_1:
    return Key_Keypad1;
  case ALLEGRO_KEY_PAD_2:
    return Key_Keypad2;
  case ALLEGRO_KEY_PAD_3:
    return Key_Keypad3;
  case ALLEGRO_KEY_PAD_4:
    return Key_Keypad4;
  case ALLEGRO_KEY_PAD_5:
    return Key_Keypad5;
  case ALLEGRO_KEY_PAD_6:
    return Key_Keypad6;
  case ALLEGRO_KEY_PAD_7:
    return Key_Keypad7;
  case ALLEGRO_KEY_PAD_8:
    return Key_Keypad8;
  case ALLEGRO_KEY_PAD_9:
    return Key_Keypad9;
  case ALLEGRO_KEY_PAD_DELETE:
    return Key_KeypadDecimal;
  case ALLEGRO_KEY_PAD_SLASH:
    return Key_KeypadDivide;
  case ALLEGRO_KEY_PAD_ASTERISK:
    return Key_KeypadMultiply;
  case ALLEGRO_KEY_PAD_MINUS:
    return Key_KeypadSubtract;
  case ALLEGRO_KEY_PAD_PLUS:
    return Key_KeypadAdd;
  case ALLEGRO_KEY_PAD_ENTER:
    return Key_KeypadEnter;
  case ALLEGRO_KEY_PAD_EQUALS:
    return Key_KeypadEqual;
  case ALLEGRO_KEY_LCTRL:
    return Key_LeftCtrl;
  case ALLEGRO_KEY_LSHIFT:
    return Key_LeftShift;
  case ALLEGRO_KEY_ALT:
    return Key_LeftAlt;
  case ALLEGRO_KEY_LWIN:
    return Key_LeftSuper;
  case ALLEGRO_KEY_RCTRL:
    return Key_RightCtrl;
  case ALLEGRO_KEY_RSHIFT:
    return Key_RightShift;
  case ALLEGRO_KEY_ALTGR:
    return Key_RightAlt;
  case ALLEGRO_KEY_RWIN:
    return Key_RightSuper;
  case ALLEGRO_KEY_MENU:
    return Key_Menu;
  case ALLEGRO_KEY_0:
    return Key_0;
  case ALLEGRO_KEY_1:
    return Key_1;
  case ALLEGRO_KEY_2:
    return Key_2;
  case ALLEGRO_KEY_3:
    return Key_3;
  case ALLEGRO_KEY_4:
    return Key_4;
  case ALLEGRO_KEY_5:
    return Key_5;
  case ALLEGRO_KEY_6:
    return Key_6;
  case ALLEGRO_KEY_7:
    return Key_7;
  case ALLEGRO_KEY_8:
    return Key_8;
  case ALLEGRO_KEY_9:
    return Key_9;
  case ALLEGRO_KEY_A:
    return Key_A;
  case ALLEGRO_KEY_B:
    return Key_B;
  case ALLEGRO_KEY_C:
    return Key_C;
  case ALLEGRO_KEY_D:
    return Key_D;
  case ALLEGRO_KEY_E:
    return Key_E;
  case ALLEGRO_KEY_F:
    return Key_F;
  case ALLEGRO_KEY_G:
    return Key_G;
  case ALLEGRO_KEY_H:
    return Key_H;
  case ALLEGRO_KEY_I:
    return Key_I;
  case ALLEGRO_KEY_J:
    return Key_J;
  case ALLEGRO_KEY_K:
    return Key_K;
  case ALLEGRO_KEY_L:
    return Key_L;
  case ALLEGRO_KEY_M:
    return Key_M;
  case ALLEGRO_KEY_N:
    return Key_N;
  case ALLEGRO_KEY_O:
    return Key_O;
  case ALLEGRO_KEY_P:
    return Key_P;
  case ALLEGRO_KEY_Q:
    return Key_Q;
  case ALLEGRO_KEY_R:
    return Key_R;
  case ALLEGRO_KEY_S:
    return Key_S;
  case ALLEGRO_KEY_T:
    return Key_T;
  case ALLEGRO_KEY_U:
    return Key_U;
  case ALLEGRO_KEY_V:
    return Key_V;
  case ALLEGRO_KEY_W:
    return Key_W;
  case ALLEGRO_KEY_X:
    return Key_X;
  case ALLEGRO_KEY_Y:
    return Key_Y;
  case ALLEGRO_KEY_Z:
    return Key_Z;
  case ALLEGRO_KEY_F1:
    return Key_F1;
  case ALLEGRO_KEY_F2:
    return Key_F2;
  case ALLEGRO_KEY_F3:
    return Key_F3;
  case ALLEGRO_KEY_F4:
    return Key_F4;
  case ALLEGRO_KEY_F5:
    return Key_F5;
  case ALLEGRO_KEY_F6:
    return Key_F6;
  case ALLEGRO_KEY_F7:
    return Key_F7;
  case ALLEGRO_KEY_F8:
    return Key_F8;
  case ALLEGRO_KEY_F9:
    return Key_F9;
  case ALLEGRO_KEY_F10:
    return Key_F10;
  case ALLEGRO_KEY_F11:
    return Key_F11;
  case ALLEGRO_KEY_F12:
    return Key_F12;
  default:
    return Key_None;
  }
}

bool Allegro5_Init(ALLEGRO_DISPLAY *display) {
  IO &io = Gui::GetIO();
  ASSERT(io.BackendPlatformUserData == nullptr &&
         "Already initialized a platform backend!");

  // Setup backend capabilities flags
  Allegro5_Data *bd = NEW(Allegro5_Data)();
  io.BackendPlatformUserData = (void *)bd;
  io.BackendPlatformName = io.BackendRendererName = "allegro5";
  io.BackendFlags |=
      BackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
                                    // values (optional)

  bd->Display = display;

  // Create custom vertex declaration.
  // Unfortunately Allegro doesn't support 32-bit packed colors so we have to
  // convert them to 4 floats. We still use a custom declaration to use
  // 'ALLEGRO_PRTEX_COORD' instead of 'ALLEGRO_PRTEX_COORD_PIXEL' else we
  // can't do a reliable conversion.
  ALLEGRO_VERTEX_ELEMENT elems[] = {
      {ALLEGRO_PRPOSITION, ALLEGRO_PRFLOAT_2, offsetof(DrawVertAllegro, pos)},
      {ALLEGRO_PRTEX_COORD, ALLEGRO_PRFLOAT_2, offsetof(DrawVertAllegro, uv)},
      {ALLEGRO_PRCOLOR_ATTR, 0, offsetof(DrawVertAllegro, col)},
      {0, 0, 0}};
  bd->VertexDecl = al_create_vertex_decl(elems, sizeof(DrawVertAllegro));

#if ALLEGRO_HAS_CLIPBOARD
  io.SetClipboardTextFn = Allegro5_SetClipboardText;
  io.GetClipboardTextFn = Allegro5_GetClipboardText;
  io.ClipboardUserData = nullptr;
#endif

  return true;
}

void Allegro5_Shutdown() {
  Allegro5_Data *bd = Allegro5_GetBackendData();
  ASSERT(bd != nullptr &&
         "No platform backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  Allegro5_InvalidateDeviceObjects();
  if (bd->VertexDecl)
    al_destroy_vertex_decl(bd->VertexDecl);
  if (bd->ClipboardTextData)
    al_free(bd->ClipboardTextData);

  io.BackendPlatformName = io.BackendRendererName = nullptr;
  io.BackendPlatformUserData = nullptr;
  io.BackendFlags &= ~BackendFlags_HasMouseCursors;
  DELETE(bd);
}

// ev->keyboard.modifiers seems always zero so using that...
static void Allegro5_UpdateKeyModifiers() {
  IO &io = Gui::GetIO();
  ALLEGRO_KEYBOARD_STATE keys;
  al_get_keyboard_state(&keys);
  io.AddKeyEvent(Mod_Ctrl, al_key_down(&keys, ALLEGRO_KEY_LCTRL) ||
                               al_key_down(&keys, ALLEGRO_KEY_RCTRL));
  io.AddKeyEvent(Mod_Shift, al_key_down(&keys, ALLEGRO_KEY_LSHIFT) ||
                                al_key_down(&keys, ALLEGRO_KEY_RSHIFT));
  io.AddKeyEvent(Mod_Alt, al_key_down(&keys, ALLEGRO_KEY_ALT) ||
                              al_key_down(&keys, ALLEGRO_KEY_ALTGR));
  io.AddKeyEvent(Mod_Super, al_key_down(&keys, ALLEGRO_KEY_LWIN) ||
                                al_key_down(&keys, ALLEGRO_KEY_RWIN));
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// gui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to gui, and hide them from
// your application based on those two flags.
bool Allegro5_ProcessEvent(ALLEGRO_EVENT *ev) {
  IO &io = Gui::GetIO();
  Allegro5_Data *bd = Allegro5_GetBackendData();

  switch (ev->type) {
  case ALLEGRO_EVENT_MOUSE_AXES:
    if (ev->mouse.display == bd->Display) {
      io.AddMousePosEvent(ev->mouse.x, ev->mouse.y);
      io.AddMouseWheelEvent(-ev->mouse.dw, ev->mouse.dz);
    }
    return true;
  case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
  case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
    if (ev->mouse.display == bd->Display && ev->mouse.button > 0 &&
        ev->mouse.button <= 5)
      io.AddMouseButtonEvent(ev->mouse.button - 1,
                             ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
    return true;
  case ALLEGRO_EVENT_TOUCH_MOVE:
    if (ev->touch.display == bd->Display)
      io.AddMousePosEvent(ev->touch.x, ev->touch.y);
    return true;
  case ALLEGRO_EVENT_TOUCH_BEGIN:
  case ALLEGRO_EVENT_TOUCH_END:
  case ALLEGRO_EVENT_TOUCH_CANCEL:
    if (ev->touch.display == bd->Display && ev->touch.primary)
      io.AddMouseButtonEvent(0, ev->type == ALLEGRO_EVENT_TOUCH_BEGIN);
    return true;
  case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
    if (ev->mouse.display == bd->Display)
      io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    return true;
  case ALLEGRO_EVENT_KEY_CHAR:
    if (ev->keyboard.display == bd->Display)
      if (ev->keyboard.unichar != 0)
        io.AddInputCharacter((unsigned int)ev->keyboard.unichar);
    return true;
  case ALLEGRO_EVENT_KEY_DOWN:
  case ALLEGRO_EVENT_KEY_UP:
    if (ev->keyboard.display == bd->Display) {
      Allegro5_UpdateKeyModifiers();
      Key key = Allegro5_KeyCodeToKey(ev->keyboard.keycode);
      io.AddKeyEvent(key, (ev->type == ALLEGRO_EVENT_KEY_DOWN));
      io.SetKeyEventNativeData(
          key, ev->keyboard.keycode,
          -1); // To support legacy indexing (<1.87 user code)
    }
    return true;
  case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
    if (ev->display.source == bd->Display)
      io.AddFocusEvent(false);
    return true;
  case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
    if (ev->display.source == bd->Display) {
      io.AddFocusEvent(true);
#if defined(ALLEGRO_UNSTABLE)
      al_clear_keyboard_state(bd->Display);
#endif
    }
    return true;
  }
  return false;
}

static void Allegro5_UpdateMouseCursor() {
  IO &io = Gui::GetIO();
  if (io.ConfigFlags & ConfigFlags_NoMouseCursorChange)
    return;

  Allegro5_Data *bd = Allegro5_GetBackendData();
  int cursor = Gui::GetMouseCursor();
  if (io.MouseDrawCursor || cursor == MouseCursor_None) {
    // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
    al_set_mouse_cursor(bd->Display, bd->MouseCursorInvisible);
  } else {
    ALLEGRO_SYSTEM_MOUSE_CURSOR cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT;
    switch (cursor) {
    case MouseCursor_TextInput:
      cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_EDIT;
      break;
    case MouseCursor_ResizeAll:
      cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_MOVE;
      break;
    case MouseCursor_ResizeNS:
      cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_N;
      break;
    case MouseCursor_ResizeEW:
      cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_E;
      break;
    case MouseCursor_ResizeNESW:
      cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NE;
      break;
    case MouseCursor_ResizeNWSE:
      cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_NW;
      break;
    case MouseCursor_NotAllowed:
      cursor_id = ALLEGRO_SYSTEM_MOUSE_CURSOR_UNAVAILABLE;
      break;
    }
    al_set_system_mouse_cursor(bd->Display, cursor_id);
  }
}

void Allegro5_NewFrame() {
  Allegro5_Data *bd = Allegro5_GetBackendData();
  ASSERT(bd != nullptr && "Did you call Allegro5_Init()?");

  if (!bd->Texture)
    Allegro5_CreateDeviceObjects();

  IO &io = Gui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  w = al_get_display_width(bd->Display);
  h = al_get_display_height(bd->Display);
  io.DisplaySize = Vec2((float)w, (float)h);

  // Setup time step
  double current_time = al_get_time();
  io.DeltaTime =
      bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
  bd->Time = current_time;

  // Setup mouse cursor shape
  Allegro5_UpdateMouseCursor();
}

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
