// dear imgui: Platform Backend for SDL2
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3,
// Vulkan..) (Info: SDL2 is a cross-platform general purpose library for
// handling windows, inputs, graphics context creation, etc.) (Prefer SDL 2.0.5+
// for full feature support.)

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

// You can use unmodified * files in your project. See examples/
// folder for examples of using this. Prefer including the entire imgui/
// repository into your project (either as a copy or as a submodule), and only
// build the backends you need. Learn about Dear Gui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of gui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2023-XX-XX: Platform: Added support for multiple windows via the
//  PlatformIO interface. 2023-10-05: Inputs: Added support for extra
//  Key values: F13 to F24 function keys, app back/forward keys.
//  2023-04-06: Inputs: Avoid calling SDL_StartTextInput()/SDL_StopTextInput()
//  as they don't only pertain to IME. It's unclear exactly what their relation
//  is to IME. (#6306) 2023-04-04: Inputs: Added support for
//  io.AddMouseSourceEvent() to discriminate
//  MouseSource_Mouse/MouseSource_TouchScreen. (#2702) 2023-02-23:
//  Accept SDL_GetPerformanceCounter() not returning a monotonically increasing
//  value. (#6189, #6114, #3644) 2023-02-07: Implement IME handler
//  (io.SetPlatformImeDataFn will call
//  SDL_SetTextInputRect()/SDL_StartTextInput()). 2023-02-07: *BREAKING CHANGE*
//  Renamed this backend file from sdl.cpp/.h to
//  sdl2.cpp/.h in prevision for the future release of SDL3.
//  2023-02-02: Avoid calling SDL_SetCursor() when cursor has not changed, as
//  the function is surprisingly costly on Mac with latest SDL (may be fixed in
//  next SDL version). 2023-02-02: Added support for SDL 2.0.18+
//  preciseX/preciseY mouse wheel data for smooth scrolling + Scaling X value on
//  Emscripten (bug?). (#4019, #6096) 2023-02-02: Removed SDL_MOUSEWHEEL value
//  clamping, as values seem correct in latest Emscripten. (#4019) 2023-02-01:
//  Flipping SDL_MOUSEWHEEL 'wheel.x' value to match other backends and offer
//  consistent horizontal scrolling direction. (#4019, #6096, #1463) 2022-10-11:
//  Using 'nullptr' instead of 'NULL' as per our switch to C++11. 2022-09-26:
//  Inputs: Disable SDL 2.0.22 new "auto capture" (SDL_HINT_MOUSE_AUTO_CAPTURE)
//  which prevents drag and drop across windows for multi-viewport support +
//  don't capture when drag and dropping. (#5710) 2022-09-26: Inputs: Renamed
//  Key_ModXXX introduced in 1.87 to Mod_XXX (old names still
//  supported). 2022-03-22: Inputs: Fix mouse position issues when dragging
//  outside of boundaries. SDL_CaptureMouse() erroneously still gives out LEAVE
//  events when hovering OS decorations. 2022-03-22: Inputs: Added support for
//  extra mouse buttons (SDL_BUTTON_X1/SDL_BUTTON_X2). 2022-02-04: Added
//  SDL_Renderer* parameter to SDL2_InitForSDLRenderer(), so we can
//  use SDL_GetRendererOutputSize() instead of SDL_GL_GetDrawableSize() when
//  bound to a SDL_Renderer. 2022-01-26: Inputs: replaced short-lived
//  io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using
//  Key_ModXXX flags. Sorry for the confusion. 2021-01-20: Inputs: calling
//  new io.AddKeyAnalogEvent() for gamepad support, instead of writing directly
//  to io.NavInputs[]. 2022-01-17: Inputs: calling new io.AddMousePosEvent(),
//  io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+). 2022-01-17:
//  Inputs: always update key mods next and before key event (not in NewFrame)
//  to fix input queue with very low framerates. 2022-01-12: Update mouse inputs
//  using SDL_MOUSEMOTION/SDL_WINDOWEVENT_LEAVE + fallback to provide it when
//  focused but not hovered/captured. More standard and will allow us to pass it
//  to future input queue API. 2022-01-12: Maintain our own copy of
//  MouseButtonsDown mask instead of using Gui::IsAnyMouseDown() which will be
//  obsoleted. 2022-01-10: Inputs: calling new io.AddKeyEvent(),
//  io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for
//  full Key range. 2021-08-17: Calling io.AddFocusEvent() on
//  SDL_WINDOWEVENT_FOCUS_GAINED/SDL_WINDOWEVENT_FOCUS_LOST. 2021-07-29: Inputs:
//  MousePos is correctly reported when the host platform window is hovered but
//  not focused (using SDL_GetMouseFocus() + SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH,
//  requires SDL 2.0.5+) 2021-06:29: *BREAKING CHANGE* Removed 'SDL_Window*
//  window' parameter to SDL2_NewFrame() which was unnecessary.
//  2021-06-29: Reorganized backend to pull data from a single structure to
//  facilitate usage with multiple-contexts (all g_XXXX access changed to
//  bd->XXXX). 2021-03-22: Rework global mouse pos availability check listing
//  supported platforms explicitly, effectively fixing mouse access on Raspberry
//  Pi. (#2837, #3950) 2020-05-25: Misc: Report a zero display-size when window
//  is minimized, to be consistent with other backends. 2020-02-20: Inputs:
//  Fixed mapping for Key_KeyPadEnter (using SDL_SCANCODE_KP_ENTER instead
//  of SDL_SCANCODE_RETURN2). 2019-12-17: Inputs: On Wayland, use
//  SDL_GetMouseState (because there is no global mouse state). 2019-12-05:
//  Inputs: Added support for MouseCursor_NotAllowed mouse cursor.
//  2019-07-21: Inputs: Added mapping for Key_KeyPadEnter.
//  2019-04-23: Inputs: Added support for SDL_GameController (if
//  ConfigFlags_NavEnableGamepad is set by user application). 2019-03-12:
//  Misc: Preserve DisplayFramebufferScale when main window is minimized.
//  2018-12-21: Inputs: Workaround for Android/iOS which don't seem to handle
//  focus related calls. 2018-11-30: Misc: Setting up io.BackendPlatformName so
//  it can be displayed in the About Window. 2018-11-14: Changed the signature
//  of SDL2_ProcessEvent() to take a 'const SDL_Event*'. 2018-08-01:
//  Inputs: Workaround for Emscripten which doesn't seem to handle focus related
//  calls. 2018-06-29: Inputs: Added support for the MouseCursor_Hand
//  cursor. 2018-06-08: Misc: Extracted sdl.cpp/.h away from the old
//  combined SDL2+OpenGL/Vulkan examples. 2018-06-08: Misc:
//  SDL2_InitForOpenGL() now takes a SDL_GLContext parameter.
//  2018-05-09: Misc: Fixed clipboard paste memory leak (we didn't call
//  SDL_FreeMemory on the data returned by SDL_GetClipboardText). 2018-03-20:
//  Misc: Setup io.BackendFlags BackendFlags_HasMouseCursors flag + honor
//  ConfigFlags_NoMouseCursorChange flag. 2018-02-16: Inputs: Added support
//  for mouse cursors, honoring Gui::GetMouseCursor() value. 2018-02-06: Misc:
//  Removed call to Gui::Shutdown() which is not available from 1.60 WIP, user
//  needs to call CreateContext/DestroyContext themselves. 2018-02-06: Inputs:
//  Added mapping for Key_Space. 2018-02-05: Misc: Using
//  SDL_GetPerformanceCounter() instead of SDL_GetTicks() to be able to handle
//  very high framerate (1000+ FPS). 2018-02-05: Inputs: Keyboard mapping is
//  using scancodes everywhere instead of a confusing mixture of keycodes and
//  scancodes. 2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-19: Inputs: When available (SDL 2.0.4+) using SDL_CaptureMouse() to
//  retrieve coordinates outside of client area when dragging. Otherwise
//  (SDL 2.0.3 and before) testing for SDL_WINDOW_INPUT_FOCUS instead of
//  SDL_WINDOW_MOUSE_FOCUS. 2018-01-18: Inputs: Added mapping for
//  Key_Insert. 2017-08-25: Inputs: MousePos set to -FLT_MAX,-FLT_MAX when
//  mouse is unavailable/missing (instead of -1,-1). 2016-10-15: Misc: Added a
//  void* user_data parameter to Clipboard function handlers.

#include "../gui.hpp"
#ifndef DISABLE
#include "sdl2.hpp"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wimplicit-int-float-conversion" // warning: implicit conversion from 'xxx'
                                      // to 'float' may lose precision
#endif

// SDL
// (the multi-viewports feature requires SDL features supported from SDL 2.0.4+.
// SDL 2.0.5+ is highly recommended)
#include <SDL.h>
#include <SDL_syswm.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if SDL_VERSION_ATLEAST(2, 0, 4) && !defined(__EMSCRIPTEN__) &&                \
    !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS) &&         \
    !defined(__amigaos4__)
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE 1
#else
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE 0
#endif
#define SDL_HAS_WINDOW_ALPHA SDL_VERSION_ATLEAST(2, 0, 5)
#define SDL_HAS_ALWAYS_ON_TOP SDL_VERSION_ATLEAST(2, 0, 5)
#define SDL_HAS_USABLE_DISPLAY_BOUNDS SDL_VERSION_ATLEAST(2, 0, 5)
#define SDL_HAS_PER_MONITOR_DPI SDL_VERSION_ATLEAST(2, 0, 4)
#define SDL_HAS_VULKAN SDL_VERSION_ATLEAST(2, 0, 6)
#define SDL_HAS_DISPLAY_EVENT SDL_VERSION_ATLEAST(2, 0, 9)
#if !SDL_HAS_VULKAN
static const Uint32 SDL_WINDOW_VULKAN = 0x10000000;
#endif

// SDL Data
struct SDL2_Data {
  SDL_Window *Window;
  SDL_Renderer *Renderer;
  Uint64 Time;
  Uint32 MouseWindowID;
  int MouseButtonsDown;
  SDL_Cursor *MouseCursors[MouseCursor_COUNT];
  SDL_Cursor *LastMouseCursor;
  int PendingMouseLeaveFrame;
  char *ClipboardTextData;
  bool MouseCanUseGlobalState;
  bool MouseCanReportHoveredViewport; // This is hard to use/unreliable on SDL
                                      // so we'll set
                                      // BackendFlags_HasMouseHoveredViewport
                                      // dynamically based on state.
  bool UseVulkan;
  bool WantUpdateMonitors;

  SDL2_Data() { memset((void *)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for
// multiple Dear Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Dear Gui context + multiple windows)
// instead of multiple Dear Gui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in
// this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled
// when using multi-context.
static SDL2_Data *SDL2_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (SDL2_Data *)Gui::GetIO().BackendPlatformUserData
             : nullptr;
}

// Forward Declarations
static void SDL2_UpdateMonitors();
static void SDL2_InitPlatformInterface(SDL_Window *window,
                                       void *sdl_gl_context);
static void SDL2_ShutdownPlatformInterface();

// Functions
static const char *SDL2_GetClipboardText(void *) {
  SDL2_Data *bd = SDL2_GetBackendData();
  if (bd->ClipboardTextData)
    SDL_free(bd->ClipboardTextData);
  bd->ClipboardTextData = SDL_GetClipboardText();
  return bd->ClipboardTextData;
}

static void SDL2_SetClipboardText(void *, const char *text) {
  SDL_SetClipboardText(text);
}

// Note: native IME will only display if user calls
// SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1") _before_ SDL_CreateWindow().
static void SDL2_SetPlatformImeData(Viewport *viewport, PlatformImeData *data) {
  if (data->WantVisible) {
    SDL_Rect r;
    r.x = (int)(data->InputPos.x - viewport->Pos.x);
    r.y = (int)(data->InputPos.y - viewport->Pos.y + data->InputLineHeight);
    r.w = 1;
    r.h = (int)data->InputLineHeight;
    SDL_SetTextInputRect(&r);
  }
}

static Key SDL2_KeycodeToKey(int keycode) {
  switch (keycode) {
  case SDLK_TAB:
    return Key_Tab;
  case SDLK_LEFT:
    return Key_LeftArrow;
  case SDLK_RIGHT:
    return Key_RightArrow;
  case SDLK_UP:
    return Key_UpArrow;
  case SDLK_DOWN:
    return Key_DownArrow;
  case SDLK_PAGEUP:
    return Key_PageUp;
  case SDLK_PAGEDOWN:
    return Key_PageDown;
  case SDLK_HOME:
    return Key_Home;
  case SDLK_END:
    return Key_End;
  case SDLK_INSERT:
    return Key_Insert;
  case SDLK_DELETE:
    return Key_Delete;
  case SDLK_BACKSPACE:
    return Key_Backspace;
  case SDLK_SPACE:
    return Key_Space;
  case SDLK_RETURN:
    return Key_Enter;
  case SDLK_ESCAPE:
    return Key_Escape;
  case SDLK_QUOTE:
    return Key_Apostrophe;
  case SDLK_COMMA:
    return Key_Comma;
  case SDLK_MINUS:
    return Key_Minus;
  case SDLK_PERIOD:
    return Key_Period;
  case SDLK_SLASH:
    return Key_Slash;
  case SDLK_SEMICOLON:
    return Key_Semicolon;
  case SDLK_EQUALS:
    return Key_Equal;
  case SDLK_LEFTBRACKET:
    return Key_LeftBracket;
  case SDLK_BACKSLASH:
    return Key_Backslash;
  case SDLK_RIGHTBRACKET:
    return Key_RightBracket;
  case SDLK_BACKQUOTE:
    return Key_GraveAccent;
  case SDLK_CAPSLOCK:
    return Key_CapsLock;
  case SDLK_SCROLLLOCK:
    return Key_ScrollLock;
  case SDLK_NUMLOCKCLEAR:
    return Key_NumLock;
  case SDLK_PRINTSCREEN:
    return Key_PrintScreen;
  case SDLK_PAUSE:
    return Key_Pause;
  case SDLK_KP_0:
    return Key_Keypad0;
  case SDLK_KP_1:
    return Key_Keypad1;
  case SDLK_KP_2:
    return Key_Keypad2;
  case SDLK_KP_3:
    return Key_Keypad3;
  case SDLK_KP_4:
    return Key_Keypad4;
  case SDLK_KP_5:
    return Key_Keypad5;
  case SDLK_KP_6:
    return Key_Keypad6;
  case SDLK_KP_7:
    return Key_Keypad7;
  case SDLK_KP_8:
    return Key_Keypad8;
  case SDLK_KP_9:
    return Key_Keypad9;
  case SDLK_KP_PERIOD:
    return Key_KeypadDecimal;
  case SDLK_KP_DIVIDE:
    return Key_KeypadDivide;
  case SDLK_KP_MULTIPLY:
    return Key_KeypadMultiply;
  case SDLK_KP_MINUS:
    return Key_KeypadSubtract;
  case SDLK_KP_PLUS:
    return Key_KeypadAdd;
  case SDLK_KP_ENTER:
    return Key_KeypadEnter;
  case SDLK_KP_EQUALS:
    return Key_KeypadEqual;
  case SDLK_LCTRL:
    return Key_LeftCtrl;
  case SDLK_LSHIFT:
    return Key_LeftShift;
  case SDLK_LALT:
    return Key_LeftAlt;
  case SDLK_LGUI:
    return Key_LeftSuper;
  case SDLK_RCTRL:
    return Key_RightCtrl;
  case SDLK_RSHIFT:
    return Key_RightShift;
  case SDLK_RALT:
    return Key_RightAlt;
  case SDLK_RGUI:
    return Key_RightSuper;
  case SDLK_APPLICATION:
    return Key_Menu;
  case SDLK_0:
    return Key_0;
  case SDLK_1:
    return Key_1;
  case SDLK_2:
    return Key_2;
  case SDLK_3:
    return Key_3;
  case SDLK_4:
    return Key_4;
  case SDLK_5:
    return Key_5;
  case SDLK_6:
    return Key_6;
  case SDLK_7:
    return Key_7;
  case SDLK_8:
    return Key_8;
  case SDLK_9:
    return Key_9;
  case SDLK_a:
    return Key_A;
  case SDLK_b:
    return Key_B;
  case SDLK_c:
    return Key_C;
  case SDLK_d:
    return Key_D;
  case SDLK_e:
    return Key_E;
  case SDLK_f:
    return Key_F;
  case SDLK_g:
    return Key_G;
  case SDLK_h:
    return Key_H;
  case SDLK_i:
    return Key_I;
  case SDLK_j:
    return Key_J;
  case SDLK_k:
    return Key_K;
  case SDLK_l:
    return Key_L;
  case SDLK_m:
    return Key_M;
  case SDLK_n:
    return Key_N;
  case SDLK_o:
    return Key_O;
  case SDLK_p:
    return Key_P;
  case SDLK_q:
    return Key_Q;
  case SDLK_r:
    return Key_R;
  case SDLK_s:
    return Key_S;
  case SDLK_t:
    return Key_T;
  case SDLK_u:
    return Key_U;
  case SDLK_v:
    return Key_V;
  case SDLK_w:
    return Key_W;
  case SDLK_x:
    return Key_X;
  case SDLK_y:
    return Key_Y;
  case SDLK_z:
    return Key_Z;
  case SDLK_F1:
    return Key_F1;
  case SDLK_F2:
    return Key_F2;
  case SDLK_F3:
    return Key_F3;
  case SDLK_F4:
    return Key_F4;
  case SDLK_F5:
    return Key_F5;
  case SDLK_F6:
    return Key_F6;
  case SDLK_F7:
    return Key_F7;
  case SDLK_F8:
    return Key_F8;
  case SDLK_F9:
    return Key_F9;
  case SDLK_F10:
    return Key_F10;
  case SDLK_F11:
    return Key_F11;
  case SDLK_F12:
    return Key_F12;
  case SDLK_F13:
    return Key_F13;
  case SDLK_F14:
    return Key_F14;
  case SDLK_F15:
    return Key_F15;
  case SDLK_F16:
    return Key_F16;
  case SDLK_F17:
    return Key_F17;
  case SDLK_F18:
    return Key_F18;
  case SDLK_F19:
    return Key_F19;
  case SDLK_F20:
    return Key_F20;
  case SDLK_F21:
    return Key_F21;
  case SDLK_F22:
    return Key_F22;
  case SDLK_F23:
    return Key_F23;
  case SDLK_F24:
    return Key_F24;
  case SDLK_AC_BACK:
    return Key_AppBack;
  case SDLK_AC_FORWARD:
    return Key_AppForward;
  }
  return Key_None;
}

static void SDL2_UpdateKeyModifiers(SDL_Keymod sdl_key_mods) {
  IO &io = Gui::GetIO();
  io.AddKeyEvent(Mod_Ctrl, (sdl_key_mods & KMOD_CTRL) != 0);
  io.AddKeyEvent(Mod_Shift, (sdl_key_mods & KMOD_SHIFT) != 0);
  io.AddKeyEvent(Mod_Alt, (sdl_key_mods & KMOD_ALT) != 0);
  io.AddKeyEvent(Mod_Super, (sdl_key_mods & KMOD_GUI) != 0);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from
// your application based on those two flags. If you have multiple SDL events
// and some of them are not meant to be used by dear imgui, you may need to
// filter events based on their windowID field.
bool SDL2_ProcessEvent(const SDL_Event *event) {
  IO &io = Gui::GetIO();
  SDL2_Data *bd = SDL2_GetBackendData();

  switch (event->type) {
  case SDL_MOUSEMOTION: {
    Vec2 mouse_pos((float)event->motion.x, (float)event->motion.y);
    if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
      int window_x, window_y;
      SDL_GetWindowPosition(SDL_GetWindowFromID(event->motion.windowID),
                            &window_x, &window_y);
      mouse_pos.x += window_x;
      mouse_pos.y += window_y;
    }
    io.AddMouseSourceEvent(event->motion.which == SDL_TOUCH_MOUSEID
                               ? MouseSource_TouchScreen
                               : MouseSource_Mouse);
    io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
    return true;
  }
  case SDL_MOUSEWHEEL: {
    // DEBUG_LOG("wheel %.2f %.2f, precise %.2f %.2f\n", (float)event->wheel.x,
    // (float)event->wheel.y, event->wheel.preciseX, event->wheel.preciseY);
#if SDL_VERSION_ATLEAST(2, 0, 18) // If this fails to compile on Emscripten:
                                  // update to latest Emscripten!
    float wheel_x = -event->wheel.preciseX;
    float wheel_y = event->wheel.preciseY;
#else
    float wheel_x = -(float)event->wheel.x;
    float wheel_y = (float)event->wheel.y;
#endif
#ifdef __EMSCRIPTEN__
    wheel_x /= 100.0f;
#endif
    io.AddMouseSourceEvent(event->wheel.which == SDL_TOUCH_MOUSEID
                               ? MouseSource_TouchScreen
                               : MouseSource_Mouse);
    io.AddMouseWheelEvent(wheel_x, wheel_y);
    return true;
  }
  case SDL_MOUSEBUTTONDOWN:
  case SDL_MOUSEBUTTONUP: {
    int mouse_button = -1;
    if (event->button.button == SDL_BUTTON_LEFT) {
      mouse_button = 0;
    }
    if (event->button.button == SDL_BUTTON_RIGHT) {
      mouse_button = 1;
    }
    if (event->button.button == SDL_BUTTON_MIDDLE) {
      mouse_button = 2;
    }
    if (event->button.button == SDL_BUTTON_X1) {
      mouse_button = 3;
    }
    if (event->button.button == SDL_BUTTON_X2) {
      mouse_button = 4;
    }
    if (mouse_button == -1)
      break;
    io.AddMouseSourceEvent(event->button.which == SDL_TOUCH_MOUSEID
                               ? MouseSource_TouchScreen
                               : MouseSource_Mouse);
    io.AddMouseButtonEvent(mouse_button, (event->type == SDL_MOUSEBUTTONDOWN));
    bd->MouseButtonsDown = (event->type == SDL_MOUSEBUTTONDOWN)
                               ? (bd->MouseButtonsDown | (1 << mouse_button))
                               : (bd->MouseButtonsDown & ~(1 << mouse_button));
    return true;
  }
  case SDL_TEXTINPUT: {
    io.AddInputCharactersUTF8(event->text.text);
    return true;
  }
  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    SDL2_UpdateKeyModifiers((SDL_Keymod)event->key.keysym.mod);
    Key key = SDL2_KeycodeToKey(event->key.keysym.sym);
    io.AddKeyEvent(key, (event->type == SDL_KEYDOWN));
    io.SetKeyEventNativeData(
        key, event->key.keysym.sym, event->key.keysym.scancode,
        event->key.keysym.scancode); // To support legacy indexing (<1.87 user
                                     // code). Legacy backend uses SDLK_*** as
                                     // indices to IsKeyXXX() functions.
    return true;
  }
#if SDL_HAS_DISPLAY_EVENT
  case SDL_DISPLAYEVENT: {
    // 2.0.26 has
    // SDL_DISPLAYEVENT_CONNECTED/SDL_DISPLAYEVENT_DISCONNECTED/SDL_DISPLAYEVENT_ORIENTATION,
    // so change of DPI/Scaling are not reflected in this event. (SDL3 has it)
    bd->WantUpdateMonitors = true;
    return true;
  }
#endif
  case SDL_WINDOWEVENT: {
    // - When capturing mouse, SDL will send a bunch of conflicting LEAVE/ENTER
    // event on every mouse move, but the final ENTER tends to be right.
    // - However we won't get a correct LEAVE event for a captured window.
    // - In some cases, when detaching a window from main viewport SDL may send
    // SDL_WINDOWEVENT_ENTER one frame too late,
    //   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag
    //   operation by clear mouse position. This is why we delay process the
    //   SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
    Uint8 window_event = event->window.event;
    if (window_event == SDL_WINDOWEVENT_ENTER) {
      bd->MouseWindowID = event->window.windowID;
      bd->PendingMouseLeaveFrame = 0;
    }
    if (window_event == SDL_WINDOWEVENT_LEAVE)
      bd->PendingMouseLeaveFrame = Gui::GetFrameCount() + 1;
    if (window_event == SDL_WINDOWEVENT_FOCUS_GAINED)
      io.AddFocusEvent(true);
    else if (window_event == SDL_WINDOWEVENT_FOCUS_LOST)
      io.AddFocusEvent(false);
    if (window_event == SDL_WINDOWEVENT_CLOSE ||
        window_event == SDL_WINDOWEVENT_MOVED ||
        window_event == SDL_WINDOWEVENT_RESIZED)
      if (Viewport *viewport = Gui::FindViewportByPlatformHandle(
              (void *)SDL_GetWindowFromID(event->window.windowID))) {
        if (window_event == SDL_WINDOWEVENT_CLOSE)
          viewport->PlatformRequestClose = true;
        if (window_event == SDL_WINDOWEVENT_MOVED)
          viewport->PlatformRequestMove = true;
        if (window_event == SDL_WINDOWEVENT_RESIZED)
          viewport->PlatformRequestResize = true;
        return true;
      }
    return true;
  }
  }
  return false;
}

static bool SDL2_Init(SDL_Window *window, SDL_Renderer *renderer,
                      void *sdl_gl_context) {
  IO &io = Gui::GetIO();
  ASSERT(io.BackendPlatformUserData == nullptr &&
         "Already initialized a platform backend!");

  // Check and store if we are on a SDL backend that supports global mouse
  // position
  // ("wayland" and "rpi" don't support it, but we chose to use a white-list
  // instead of a black-list)
  bool mouse_can_use_global_state = false;
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
  const char *sdl_backend = SDL_GetCurrentVideoDriver();
  const char *global_mouse_whitelist[] = {"windows", "cocoa", "x11", "DIVE",
                                          "VMAN"};
  for (int n = 0; n < ARRAYSIZE(global_mouse_whitelist); n++)
    if (strncmp(sdl_backend, global_mouse_whitelist[n],
                strlen(global_mouse_whitelist[n])) == 0)
      mouse_can_use_global_state = true;
#endif

  // Setup backend capabilities flags
  SDL2_Data *bd = NEW(SDL2_Data)();
  io.BackendPlatformUserData = (void *)bd;
  io.BackendPlatformName = "sdl2";
  io.BackendFlags |=
      BackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values
                                    // (optional)
  io.BackendFlags |=
      BackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos
                                   // requests (optional, rarely used)
  if (mouse_can_use_global_state)
    io.BackendFlags |=
        BackendFlags_PlatformHasViewports; // We can create multi-viewports
                                           // on the Platform side
                                           // (optional)

  bd->Window = window;
  bd->Renderer = renderer;

  // SDL on Linux/OSX doesn't report events for unfocused windows (see
  // https://github.com/ocornut/imgui/issues/4960) We will use
  // 'MouseCanReportHoveredViewport' to set
  // 'BackendFlags_HasMouseHoveredViewport' dynamically each frame.
  bd->MouseCanUseGlobalState = mouse_can_use_global_state;
#ifndef __APPLE__
  bd->MouseCanReportHoveredViewport = bd->MouseCanUseGlobalState;
#else
  bd->MouseCanReportHoveredViewport = false;
#endif
  bd->WantUpdateMonitors = true;

  io.SetClipboardTextFn = SDL2_SetClipboardText;
  io.GetClipboardTextFn = SDL2_GetClipboardText;
  io.ClipboardUserData = nullptr;
  io.SetPlatformImeDataFn = SDL2_SetPlatformImeData;

  // Load mouse cursors
  bd->MouseCursors[MouseCursor_Arrow] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  bd->MouseCursors[MouseCursor_TextInput] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
  bd->MouseCursors[MouseCursor_ResizeAll] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  bd->MouseCursors[MouseCursor_ResizeNS] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  bd->MouseCursors[MouseCursor_ResizeEW] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  bd->MouseCursors[MouseCursor_ResizeNESW] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
  bd->MouseCursors[MouseCursor_ResizeNWSE] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
  bd->MouseCursors[MouseCursor_Hand] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  bd->MouseCursors[MouseCursor_NotAllowed] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

  // Set platform dependent data in viewport
  // Our mouse update function expect PlatformHandle to be filled for the main
  // viewport
  Viewport *main_viewport = Gui::GetMainViewport();
  main_viewport->PlatformHandle = (void *)window;
  main_viewport->PlatformHandleRaw = nullptr;
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWindowWMInfo(window, &info)) {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    main_viewport->PlatformHandleRaw = (void *)info.info.win.window;
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
    main_viewport->PlatformHandleRaw = (void *)info.info.cocoa.window;
#endif
  }

  // From 2.0.5: Set SDL hint to receive mouse click events on window focus,
  // otherwise SDL doesn't emit the event. Without this, when clicking to gain
  // focus, our widgets wouldn't activate even though they showed as hovered.
  // (This is unfortunately a global SDL setting, so enabling it might have a
  // side-effect on your application. It is unlikely to make a difference, but
  // if your app absolutely needs to ignore the initial on-focus click: you can
  // ignore SDL_MOUSEBUTTONDOWN events coming right after a
  // SDL_WINDOWEVENT_FOCUS_GAINED)
#ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif

  // From 2.0.18: Enable native IME.
  // IMPORTANT: This is used at the time of SDL_CreateWindow() so this will only
  // affects secondary windows, if any. For the main window to be affected, your
  // application needs to call this manually before calling SDL_CreateWindow().
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // From 2.0.22: Disable auto-capture, this is preventing drag and drop across
  // multiple windows (see #5710)
#ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
  SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
#endif

  // We need SDL_CaptureMouse(), SDL_GetGlobalMouseState() from SDL 2.0.4+ to
  // support multiple viewports. We left the call to
  // SDL2_InitPlatformInterface() outside of #ifdef to avoid
  // unused-function warnings.
  if ((io.ConfigFlags & ConfigFlags_ViewportsEnable) &&
      (io.BackendFlags & BackendFlags_PlatformHasViewports))
    SDL2_InitPlatformInterface(window, sdl_gl_context);

  return true;
}

bool SDL2_InitForOpenGL(SDL_Window *window, void *sdl_gl_context) {
  return SDL2_Init(window, nullptr, sdl_gl_context);
}

bool SDL2_InitForVulkan(SDL_Window *window) {
#if !SDL_HAS_VULKAN
  ASSERT(0 && "Unsupported");
#endif
  if (!SDL2_Init(window, nullptr, nullptr))
    return false;
  SDL2_Data *bd = SDL2_GetBackendData();
  bd->UseVulkan = true;
  return true;
}

bool SDL2_InitForD3D(SDL_Window *window) {
#if !defined(_WIN32)
  ASSERT(0 && "Unsupported");
#endif
  return SDL2_Init(window, nullptr, nullptr);
}

bool SDL2_InitForMetal(SDL_Window *window) {
  return SDL2_Init(window, nullptr, nullptr);
}

bool SDL2_InitForSDLRenderer(SDL_Window *window, SDL_Renderer *renderer) {
  return SDL2_Init(window, renderer, nullptr);
}

bool SDL2_InitForOther(SDL_Window *window) {
  return SDL2_Init(window, nullptr, nullptr);
}

void SDL2_Shutdown() {
  SDL2_Data *bd = SDL2_GetBackendData();
  ASSERT(bd != nullptr &&
         "No platform backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  SDL2_ShutdownPlatformInterface();

  if (bd->ClipboardTextData)
    SDL_free(bd->ClipboardTextData);
  for (MouseCursor cursor_n = 0; cursor_n < MouseCursor_COUNT; cursor_n++)
    SDL_FreeCursor(bd->MouseCursors[cursor_n]);
  bd->LastMouseCursor = nullptr;

  io.BackendPlatformName = nullptr;
  io.BackendPlatformUserData = nullptr;
  io.BackendFlags &=
      ~(BackendFlags_HasMouseCursors | BackendFlags_HasSetMousePos |
        BackendFlags_HasGamepad | BackendFlags_PlatformHasViewports |
        BackendFlags_HasMouseHoveredViewport);
  DELETE(bd);
}

// This code is incredibly messy because some of the functions we need for full
// viewport support are not available in SDL < 2.0.4.
static void SDL2_UpdateMouseData() {
  SDL2_Data *bd = SDL2_GetBackendData();
  IO &io = Gui::GetIO();

  // We forward mouse input when hovered or captured (via SDL_MOUSEMOTION) or
  // when focused (below)
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
  // SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL
  // window boundaries shouldn't e.g. trigger other operations outside
  SDL_CaptureMouse((bd->MouseButtonsDown != 0) ? SDL_TRUE : SDL_FALSE);
  SDL_Window *focused_window = SDL_GetKeyboardFocus();
  const bool is_app_focused =
      (focused_window &&
       (bd->Window == focused_window ||
        Gui::FindViewportByPlatformHandle((void *)focused_window)));
#else
  SDL_Window *focused_window = bd->Window;
  const bool is_app_focused =
      (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_INPUT_FOCUS) !=
      0; // SDL 2.0.3 and non-windowed systems: single-viewport only
#endif

  if (is_app_focused) {
    // (Optional) Set OS mouse position from Dear Gui if requested (rarely
    // used, only when ConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos) {
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
      if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
        SDL_WarpMouseGlobal((int)io.MousePos.x, (int)io.MousePos.y);
      else
#endif
        SDL_WarpMouseInWindow(bd->Window, (int)io.MousePos.x,
                              (int)io.MousePos.y);
    }

    // (Optional) Fallback to provide mouse position when focused
    // (SDL_MOUSEMOTION already provides this when hovered or captured)
    if (bd->MouseCanUseGlobalState && bd->MouseButtonsDown == 0) {
      // Single-viewport mode: mouse position in client window coordinates
      // (io.MousePos is (0,0) when the mouse is on the upper-left corner of the
      // app window) Multi-viewport mode: mouse position in OS absolute
      // coordinates (io.MousePos is (0,0) when the mouse is on the upper-left
      // of the primary monitor)
      int mouse_x, mouse_y, window_x, window_y;
      SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
      if (!(io.ConfigFlags & ConfigFlags_ViewportsEnable)) {
        SDL_GetWindowPosition(focused_window, &window_x, &window_y);
        mouse_x -= window_x;
        mouse_y -= window_y;
      }
      io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
    }
  }

  // (Optional) When using multiple viewports: call io.AddMouseViewportEvent()
  // with the viewport the OS mouse cursor is hovering. If
  // BackendFlags_HasMouseHoveredViewport is not set by the backend, Dear
  // imGui will ignore this field and infer the information using its flawed
  // heuristic.
  // - [!] SDL backend does NOT correctly ignore viewports with the _NoInputs
  // flag.
  //       Some backend are not able to handle that correctly. If a backend
  //       report an hovered viewport that has the _NoInputs flag (e.g. when
  //       dragging a window for docking, the viewport has the _NoInputs flag in
  //       order to allow us to find the viewport under), then Dear Gui is
  //       forced to ignore the value reported by the backend, and use its
  //       flawed heuristic to guess the viewport behind.
  // - [X] SDL backend correctly reports this regardless of another viewport
  // behind focused and dragged from (we need this to find a useful drag and
  // drop target).
  if (io.BackendFlags & BackendFlags_HasMouseHoveredViewport) {
    ID mouse_viewport_id = 0;
    if (SDL_Window *sdl_mouse_window = SDL_GetWindowFromID(bd->MouseWindowID))
      if (Viewport *mouse_viewport =
              Gui::FindViewportByPlatformHandle((void *)sdl_mouse_window))
        mouse_viewport_id = mouse_viewport->ID;
    io.AddMouseViewportEvent(mouse_viewport_id);
  }
}

static void SDL2_UpdateMouseCursor() {
  IO &io = Gui::GetIO();
  if (io.ConfigFlags & ConfigFlags_NoMouseCursorChange)
    return;
  SDL2_Data *bd = SDL2_GetBackendData();

  MouseCursor cursor = Gui::GetMouseCursor();
  if (io.MouseDrawCursor || cursor == MouseCursor_None) {
    // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
    SDL_ShowCursor(SDL_FALSE);
  } else {
    // Show OS mouse cursor
    SDL_Cursor *expected_cursor = bd->MouseCursors[cursor]
                                      ? bd->MouseCursors[cursor]
                                      : bd->MouseCursors[MouseCursor_Arrow];
    if (bd->LastMouseCursor != expected_cursor) {
      SDL_SetCursor(expected_cursor); // SDL function doesn't have an early out
                                      // (see #6113)
      bd->LastMouseCursor = expected_cursor;
    }
    SDL_ShowCursor(SDL_TRUE);
  }
}

static void SDL2_UpdateGamepads() {
  IO &io = Gui::GetIO();
  if ((io.ConfigFlags & ConfigFlags_NavEnableGamepad) ==
      0) // FIXME: Technically feeding gamepad shouldn't depend on this now that
         // they are regular inputs.
    return;

  // Get gamepad
  io.BackendFlags &= ~BackendFlags_HasGamepad;
  SDL_GameController *game_controller = SDL_GameControllerOpen(0);
  if (!game_controller)
    return;
  io.BackendFlags |= BackendFlags_HasGamepad;

// Update gamepad inputs
#define SATURATE(V) (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
#define MAP_BUTTON(KEY_NO, BUTTON_NO)                                          \
  {                                                                            \
    io.AddKeyEvent(                                                            \
        KEY_NO, SDL_GameControllerGetButton(game_controller, BUTTON_NO) != 0); \
  }
#define MAP_ANALOG(KEY_NO, AXIS_NO, V0, V1)                                    \
  {                                                                            \
    float vn =                                                                 \
        (float)(SDL_GameControllerGetAxis(game_controller, AXIS_NO) - V0) /    \
        (float)(V1 - V0);                                                      \
    vn = SATURATE(vn);                                                         \
    io.AddKeyAnalogEvent(KEY_NO, vn > 0.1f, vn);                               \
  }
  const int thumb_dead_zone =
      8000; // SDL_gamecontroller.h suggests using this value.
  MAP_BUTTON(Key_GamepadStart, SDL_CONTROLLER_BUTTON_START);
  MAP_BUTTON(Key_GamepadBack, SDL_CONTROLLER_BUTTON_BACK);
  MAP_BUTTON(Key_GamepadFaceLeft,
             SDL_CONTROLLER_BUTTON_X); // Xbox X, PS Square
  MAP_BUTTON(Key_GamepadFaceRight,
             SDL_CONTROLLER_BUTTON_B); // Xbox B, PS Circle
  MAP_BUTTON(Key_GamepadFaceUp,
             SDL_CONTROLLER_BUTTON_Y); // Xbox Y, PS Triangle
  MAP_BUTTON(Key_GamepadFaceDown,
             SDL_CONTROLLER_BUTTON_A); // Xbox A, PS Cross
  MAP_BUTTON(Key_GamepadDpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
  MAP_BUTTON(Key_GamepadDpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
  MAP_BUTTON(Key_GamepadDpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
  MAP_BUTTON(Key_GamepadDpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
  MAP_BUTTON(Key_GamepadL1, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
  MAP_BUTTON(Key_GamepadR1, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
  MAP_ANALOG(Key_GamepadL2, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 0.0f, 32767);
  MAP_ANALOG(Key_GamepadR2, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0.0f, 32767);
  MAP_BUTTON(Key_GamepadL3, SDL_CONTROLLER_BUTTON_LEFTSTICK);
  MAP_BUTTON(Key_GamepadR3, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
  MAP_ANALOG(Key_GamepadLStickLeft, SDL_CONTROLLER_AXIS_LEFTX, -thumb_dead_zone,
             -32768);
  MAP_ANALOG(Key_GamepadLStickRight, SDL_CONTROLLER_AXIS_LEFTX,
             +thumb_dead_zone, +32767);
  MAP_ANALOG(Key_GamepadLStickUp, SDL_CONTROLLER_AXIS_LEFTY, -thumb_dead_zone,
             -32768);
  MAP_ANALOG(Key_GamepadLStickDown, SDL_CONTROLLER_AXIS_LEFTY, +thumb_dead_zone,
             +32767);
  MAP_ANALOG(Key_GamepadRStickLeft, SDL_CONTROLLER_AXIS_RIGHTX,
             -thumb_dead_zone, -32768);
  MAP_ANALOG(Key_GamepadRStickRight, SDL_CONTROLLER_AXIS_RIGHTX,
             +thumb_dead_zone, +32767);
  MAP_ANALOG(Key_GamepadRStickUp, SDL_CONTROLLER_AXIS_RIGHTY, -thumb_dead_zone,
             -32768);
  MAP_ANALOG(Key_GamepadRStickDown, SDL_CONTROLLER_AXIS_RIGHTY,
             +thumb_dead_zone, +32767);
#undef MAP_BUTTON
#undef MAP_ANALOG
}

// FIXME: Note that doesn't update with DPI/Scaling change only as SDL2 doesn't
// have an event for it (SDL3 has).
static void SDL2_UpdateMonitors() {
  SDL2_Data *bd = SDL2_GetBackendData();
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Monitors.resize(0);
  bd->WantUpdateMonitors = false;
  int display_count = SDL_GetNumVideoDisplays();
  for (int n = 0; n < display_count; n++) {
    // Warning: the validity of monitor DPI information on Windows depends on
    // the application DPI awareness settings, which generally needs to be set
    // in the manifest or at runtime.
    PlatformMonitor monitor;
    SDL_Rect r;
    SDL_GetDisplayBounds(n, &r);
    monitor.MainPos = monitor.WorkPos = Vec2((float)r.x, (float)r.y);
    monitor.MainSize = monitor.WorkSize = Vec2((float)r.w, (float)r.h);
#if SDL_HAS_USABLE_DISPLAY_BOUNDS
    SDL_GetDisplayUsableBounds(n, &r);
    monitor.WorkPos = Vec2((float)r.x, (float)r.y);
    monitor.WorkSize = Vec2((float)r.w, (float)r.h);
#endif
#if SDL_HAS_PER_MONITOR_DPI
    // FIXME-VIEWPORT: On MacOS SDL reports actual monitor DPI scale, ignoring
    // OS configuration. We may want to set
    //  DpiScale to cocoa_window.backingScaleFactor here.
    float dpi = 0.0f;
    if (!SDL_GetDisplayDPI(n, &dpi, nullptr, nullptr))
      monitor.DpiScale = dpi / 96.0f;
#endif
    monitor.PlatformHandle = (void *)(intptr_t)n;
    platform_io.Monitors.push_back(monitor);
  }
}

void SDL2_NewFrame() {
  SDL2_Data *bd = SDL2_GetBackendData();
  ASSERT(bd != nullptr && "Did you call SDL2_Init()?");
  IO &io = Gui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  SDL_GetWindowSize(bd->Window, &w, &h);
  if (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_MINIMIZED)
    w = h = 0;
  if (bd->Renderer != nullptr)
    SDL_GetRendererOutputSize(bd->Renderer, &display_w, &display_h);
  else
    SDL_GL_GetDrawableSize(bd->Window, &display_w, &display_h);
  io.DisplaySize = Vec2((float)w, (float)h);
  if (w > 0 && h > 0)
    io.DisplayFramebufferScale =
        Vec2((float)display_w / w, (float)display_h / h);

  // Update monitors
  if (bd->WantUpdateMonitors)
    SDL2_UpdateMonitors();

  // Setup time step (we don't use SDL_GetTicks() because it is using
  // millisecond resolution) (Accept SDL_GetPerformanceCounter() not returning a
  // monotonically increasing value. Happens in VMs and Emscripten, see #6189,
  // #6114, #3644)
  static Uint64 frequency = SDL_GetPerformanceFrequency();
  Uint64 current_time = SDL_GetPerformanceCounter();
  if (current_time <= bd->Time)
    current_time = bd->Time + 1;
  io.DeltaTime = bd->Time > 0
                     ? (float)((double)(current_time - bd->Time) / frequency)
                     : (float)(1.0f / 60.0f);
  bd->Time = current_time;

  if (bd->PendingMouseLeaveFrame &&
      bd->PendingMouseLeaveFrame >= Gui::GetFrameCount() &&
      bd->MouseButtonsDown == 0) {
    bd->MouseWindowID = 0;
    bd->PendingMouseLeaveFrame = 0;
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
  }

  // Our io.AddMouseViewportEvent() calls will only be valid when not capturing.
  // Technically speaking testing for 'bd->MouseButtonsDown == 0' would be more
  // rygorous, but testing for payload reduces noise and potential side-effects.
  if (bd->MouseCanReportHoveredViewport && Gui::GetDragDropPayload() == nullptr)
    io.BackendFlags |= BackendFlags_HasMouseHoveredViewport;
  else
    io.BackendFlags &= ~BackendFlags_HasMouseHoveredViewport;

  SDL2_UpdateMouseData();
  SDL2_UpdateMouseCursor();

  // Update game controllers (if enabled and available)
  SDL2_UpdateGamepads();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create
// and handle multiple viewports simultaneously. If you are new to dear imgui or
// creating a new binding for dear imgui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* RendererUserData field of each
// Viewport to easily retrieve our backend data.
struct SDL2_ViewportData {
  SDL_Window *Window;
  Uint32 WindowID;
  bool WindowOwned;
  SDL_GLContext GLContext;

  SDL2_ViewportData() {
    Window = nullptr;
    WindowID = 0;
    WindowOwned = false;
    GLContext = nullptr;
  }
  ~SDL2_ViewportData() { ASSERT(Window == nullptr && GLContext == nullptr); }
};

static void SDL2_CreateWindow(Viewport *viewport) {
  SDL2_Data *bd = SDL2_GetBackendData();
  SDL2_ViewportData *vd = NEW(SDL2_ViewportData)();
  viewport->PlatformUserData = vd;

  Viewport *main_viewport = Gui::GetMainViewport();
  SDL2_ViewportData *main_viewport_data =
      (SDL2_ViewportData *)main_viewport->PlatformUserData;

  // Share GL resources with main context
  bool use_opengl = (main_viewport_data->GLContext != nullptr);
  SDL_GLContext backup_context = nullptr;
  if (use_opengl) {
    backup_context = SDL_GL_GetCurrentContext();
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    SDL_GL_MakeCurrent(main_viewport_data->Window,
                       main_viewport_data->GLContext);
  }

  Uint32 sdl_flags = 0;
  sdl_flags |=
      use_opengl ? SDL_WINDOW_OPENGL : (bd->UseVulkan ? SDL_WINDOW_VULKAN : 0);
  sdl_flags |= SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_ALLOW_HIGHDPI;
  sdl_flags |= SDL_WINDOW_HIDDEN;
  sdl_flags |= (viewport->Flags & ViewportFlags_NoDecoration)
                   ? SDL_WINDOW_BORDERLESS
                   : 0;
  sdl_flags |=
      (viewport->Flags & ViewportFlags_NoDecoration) ? 0 : SDL_WINDOW_RESIZABLE;
#if !defined(_WIN32)
  // See SDL hack in SDL2_ShowWindow().
  sdl_flags |= (viewport->Flags & ViewportFlags_NoTaskBarIcon)
                   ? SDL_WINDOW_SKIP_TASKBAR
                   : 0;
#endif
#if SDL_HAS_ALWAYS_ON_TOP
  sdl_flags |=
      (viewport->Flags & ViewportFlags_TopMost) ? SDL_WINDOW_ALWAYS_ON_TOP : 0;
#endif
  vd->Window = SDL_CreateWindow("No Title Yet", (int)viewport->Pos.x,
                                (int)viewport->Pos.y, (int)viewport->Size.x,
                                (int)viewport->Size.y, sdl_flags);
  vd->WindowOwned = true;
  if (use_opengl) {
    vd->GLContext = SDL_GL_CreateContext(vd->Window);
    SDL_GL_SetSwapInterval(0);
  }
  if (use_opengl && backup_context)
    SDL_GL_MakeCurrent(vd->Window, backup_context);

  viewport->PlatformHandle = (void *)vd->Window;
  viewport->PlatformHandleRaw = nullptr;
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWindowWMInfo(vd->Window, &info)) {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    viewport->PlatformHandleRaw = info.info.win.window;
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
    viewport->PlatformHandleRaw = (void *)info.info.cocoa.window;
#endif
  }
}

static void SDL2_DestroyWindow(Viewport *viewport) {
  if (SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData) {
    if (vd->GLContext && vd->WindowOwned)
      SDL_GL_DeleteContext(vd->GLContext);
    if (vd->Window && vd->WindowOwned)
      SDL_DestroyWindow(vd->Window);
    vd->GLContext = nullptr;
    vd->Window = nullptr;
    DELETE(vd);
  }
  viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void SDL2_ShowWindow(Viewport *viewport) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
#if defined(_WIN32)
  HWND hwnd = (HWND)viewport->PlatformHandleRaw;

  // SDL hack: Hide icon from task bar
  // Note: SDL 2.0.6+ has a SDL_WINDOW_SKIP_TASKBAR flag which is supported
  // under Windows but the way it create the window breaks our seamless
  // transition.
  if (viewport->Flags & ViewportFlags_NoTaskBarIcon) {
    LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
    ex_style &= ~WS_EX_APPWINDOW;
    ex_style |= WS_EX_TOOLWINDOW;
    ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
  }

  // SDL hack: SDL always activate/focus windows :/
  if (viewport->Flags & ViewportFlags_NoFocusOnAppearing) {
    ::ShowWindow(hwnd, SW_SHOWNA);
    return;
  }
#endif

  SDL_ShowWindow(vd->Window);
}

static Vec2 SDL2_GetWindowPos(Viewport *viewport) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  int x = 0, y = 0;
  SDL_GetWindowPosition(vd->Window, &x, &y);
  return Vec2((float)x, (float)y);
}

static void SDL2_SetWindowPos(Viewport *viewport, Vec2 pos) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowPosition(vd->Window, (int)pos.x, (int)pos.y);
}

static Vec2 SDL2_GetWindowSize(Viewport *viewport) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  int w = 0, h = 0;
  SDL_GetWindowSize(vd->Window, &w, &h);
  return Vec2((float)w, (float)h);
}

static void SDL2_SetWindowSize(Viewport *viewport, Vec2 size) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowSize(vd->Window, (int)size.x, (int)size.y);
}

static void SDL2_SetWindowTitle(Viewport *viewport, const char *title) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowTitle(vd->Window, title);
}

#if SDL_HAS_WINDOW_ALPHA
static void SDL2_SetWindowAlpha(Viewport *viewport, float alpha) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowOpacity(vd->Window, alpha);
}
#endif

static void SDL2_SetWindowFocus(Viewport *viewport) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  SDL_RaiseWindow(vd->Window);
}

static bool SDL2_GetWindowFocus(Viewport *viewport) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

static bool SDL2_GetWindowMinimized(Viewport *viewport) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_MINIMIZED) != 0;
}

static void SDL2_RenderWindow(Viewport *viewport, void *) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  if (vd->GLContext)
    SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
}

static void SDL2_SwapBuffers(Viewport *viewport, void *) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  if (vd->GLContext) {
    SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
    SDL_GL_SwapWindow(vd->Window);
  }
}

// Vulkan support (the Vulkan renderer needs to call a platform-side support
// function to create the surface) SDL is graceful enough to _not_ need
// <vulkan/vulkan.h> so we can safely include this.
#if SDL_HAS_VULKAN
#include <SDL_vulkan.h>
static int SDL2_CreateVkSurface(Viewport *viewport, U64 vk_instance,
                                const void *vk_allocator, U64 *out_vk_surface) {
  SDL2_ViewportData *vd = (SDL2_ViewportData *)viewport->PlatformUserData;
  (void)vk_allocator;
  SDL_bool ret = SDL_Vulkan_CreateSurface(vd->Window, (VkInstance)vk_instance,
                                          (VkSurfaceKHR *)out_vk_surface);
  return ret ? 0 : 1; // ret ? VK_SUCCESS : VK_NOT_READY
}
#endif // SDL_HAS_VULKAN

static void SDL2_InitPlatformInterface(SDL_Window *window,
                                       void *sdl_gl_context) {
  // Register platform interface (will be coupled with a renderer interface)
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Platform_CreateWindow = SDL2_CreateWindow;
  platform_io.Platform_DestroyWindow = SDL2_DestroyWindow;
  platform_io.Platform_ShowWindow = SDL2_ShowWindow;
  platform_io.Platform_SetWindowPos = SDL2_SetWindowPos;
  platform_io.Platform_GetWindowPos = SDL2_GetWindowPos;
  platform_io.Platform_SetWindowSize = SDL2_SetWindowSize;
  platform_io.Platform_GetWindowSize = SDL2_GetWindowSize;
  platform_io.Platform_SetWindowFocus = SDL2_SetWindowFocus;
  platform_io.Platform_GetWindowFocus = SDL2_GetWindowFocus;
  platform_io.Platform_GetWindowMinimized = SDL2_GetWindowMinimized;
  platform_io.Platform_SetWindowTitle = SDL2_SetWindowTitle;
  platform_io.Platform_RenderWindow = SDL2_RenderWindow;
  platform_io.Platform_SwapBuffers = SDL2_SwapBuffers;
#if SDL_HAS_WINDOW_ALPHA
  platform_io.Platform_SetWindowAlpha = SDL2_SetWindowAlpha;
#endif
#if SDL_HAS_VULKAN
  platform_io.Platform_CreateVkSurface = SDL2_CreateVkSurface;
#endif

  // Register main window handle (which is owned by the main application, not by
  // us) This is mostly for simplicity and consistency, so that our code (e.g.
  // mouse handling etc.) can use same logic for main and secondary viewports.
  Viewport *main_viewport = Gui::GetMainViewport();
  SDL2_ViewportData *vd = NEW(SDL2_ViewportData)();
  vd->Window = window;
  vd->WindowID = SDL_GetWindowID(window);
  vd->WindowOwned = false;
  vd->GLContext = sdl_gl_context;
  main_viewport->PlatformUserData = vd;
  main_viewport->PlatformHandle = vd->Window;
}

static void SDL2_ShutdownPlatformInterface() { Gui::DestroyPlatformWindows(); }

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef DISABLE
