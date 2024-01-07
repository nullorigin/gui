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

#include "../gui.hpp"
#ifndef DISABLE
#include "sdl3.hpp"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wimplicit-int-float-conversion" // warning: implicit conversion from 'xxx'
                                      // to 'float' may lose precision
#endif

// SDL
#include <SDL3/SDL.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) &&                       \
    !(defined(__APPLE__) && TARGET_OS_IOS) && !defined(__amigaos4__)
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE 1
#else
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE 0
#endif

// SDL Data
struct SDL3_Data {
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

  SDL3_Data() { memset((void *)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in
// this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled
// when using multi-context.
static SDL3_Data *SDL3_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (SDL3_Data *)Gui::GetIO().BackendPlatformUserData
             : nullptr;
}

// Forward Declarations
static void SDL3_UpdateMonitors();
static void SDL3_InitPlatformInterface(SDL_Window *window,
                                       void *sdl_gl_context);
static void SDL3_ShutdownPlatformInterface();

// Functions
static const char *SDL3_GetClipboardText(void *) {
  SDL3_Data *bd = SDL3_GetBackendData();
  if (bd->ClipboardTextData)
    SDL_free(bd->ClipboardTextData);
  bd->ClipboardTextData = SDL_GetClipboardText();
  return bd->ClipboardTextData;
}

static void SDL3_SetClipboardText(void *, const char *text) {
  SDL_SetClipboardText(text);
}

static void SDL3_SetPlatformImeData(Viewport *viewport, PlatformImeData *data) {
  if (data->WantVisible) {
    SDL_Rect r;
    r.x = (int)(data->InputPos.x - viewport->Pos.x);
    r.y = (int)(data->InputPos.y - viewport->Pos.y + data->InputLineHeight);
    r.w = 1;
    r.h = (int)data->InputLineHeight;
    SDL_SetTextInputRect(&r);
  }
}

static Key SDL3_KeycodeToKey(int keycode) {
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

static void SDL3_UpdateKeyModifiers(SDL_Keymod sdl_key_mods) {
  IO &io = Gui::GetIO();
  io.AddKeyEvent(Mod_Ctrl, (sdl_key_mods & SDL_KMOD_CTRL) != 0);
  io.AddKeyEvent(Mod_Shift, (sdl_key_mods & SDL_KMOD_SHIFT) != 0);
  io.AddKeyEvent(Mod_Alt, (sdl_key_mods & SDL_KMOD_ALT) != 0);
  io.AddKeyEvent(Mod_Super, (sdl_key_mods & SDL_KMOD_GUI) != 0);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// gui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to gui, and hide them from
// your application based on those two flags. If you have multiple SDL events
// and some of them are not meant to be used by gui, you may need to
// filter events based on their windowID field.
bool SDL3_ProcessEvent(const SDL_Event *event) {
  IO &io = Gui::GetIO();
  SDL3_Data *bd = SDL3_GetBackendData();

  switch (event->type) {
  case SDL_EVENT_MOUSE_MOTION: {
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
  case SDL_EVENT_MOUSE_WHEEL: {
    // DEBUG_LOG("wheel %.2f %.2f, precise %.2f %.2f\n", (float)event->wheel.x,
    // (float)event->wheel.y, event->wheel.preciseX, event->wheel.preciseY);
    float wheel_x = -event->wheel.x;
    float wheel_y = event->wheel.y;
#ifdef __EMSCRIPTEN__
    wheel_x /= 100.0f;
#endif
    io.AddMouseSourceEvent(event->wheel.which == SDL_TOUCH_MOUSEID
                               ? MouseSource_TouchScreen
                               : MouseSource_Mouse);
    io.AddMouseWheelEvent(wheel_x, wheel_y);
    return true;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP: {
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
    io.AddMouseButtonEvent(mouse_button,
                           (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN));
    bd->MouseButtonsDown = (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                               ? (bd->MouseButtonsDown | (1 << mouse_button))
                               : (bd->MouseButtonsDown & ~(1 << mouse_button));
    return true;
  }
  case SDL_EVENT_TEXT_INPUT: {
    io.AddInputCharactersUTF8(event->text.text);
    return true;
  }
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP: {
    SDL3_UpdateKeyModifiers((SDL_Keymod)event->key.keysym.mod);
    Key key = SDL3_KeycodeToKey(event->key.keysym.sym);
    io.AddKeyEvent(key, (event->type == SDL_EVENT_KEY_DOWN));
    io.SetKeyEventNativeData(
        key, event->key.keysym.sym, event->key.keysym.scancode,
        event->key.keysym.scancode); // To support legacy indexing (<1.87 user
                                     // code). Legacy backend uses SDLK_*** as
                                     // indices to IsKeyXXX() functions.
    return true;
  }
  case SDL_EVENT_DISPLAY_ORIENTATION:
  case SDL_EVENT_DISPLAY_ADDED:
  case SDL_EVENT_DISPLAY_REMOVED:
  case SDL_EVENT_DISPLAY_MOVED:
  case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED: {
    bd->WantUpdateMonitors = true;
    return true;
  }
  case SDL_EVENT_WINDOW_MOUSE_ENTER: {
    bd->MouseWindowID = event->window.windowID;
    bd->PendingMouseLeaveFrame = 0;
    return true;
  }
  // - In some cases, when detaching a window from main viewport SDL may send
  // SDL_WINDOWEVENT_ENTER one frame too late,
  //   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag
  //   operation by clear mouse position. This is why we delay process the
  //   SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
  // FIXME: Unconfirmed whether this is still needed with SDL3.
  case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
    bd->PendingMouseLeaveFrame = Gui::GetFrameCount() + 1;
    return true;
  }
  case SDL_EVENT_WINDOW_FOCUS_GAINED:
    io.AddFocusEvent(true);
    return true;
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    io.AddFocusEvent(false);
    return true;
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
  case SDL_EVENT_WINDOW_MOVED:
  case SDL_EVENT_WINDOW_RESIZED:
    if (Viewport *viewport = Gui::FindViewportByPlatformHandle(
            (void *)SDL_GetWindowFromID(event->window.windowID))) {
      if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        viewport->PlatformRequestClose = true;
      if (event->type == SDL_EVENT_WINDOW_MOVED)
        viewport->PlatformRequestMove = true;
      if (event->type == SDL_EVENT_WINDOW_RESIZED)
        viewport->PlatformRequestResize = true;
      return true;
    }
    return true;
  }
  return false;
}

static void SDL3_SetupPlatformHandles(Viewport *viewport, SDL_Window *window) {
  viewport->PlatformHandle = window;
  viewport->PlatformHandleRaw = nullptr;
#if defined(__WIN32__) && !defined(__WINRT__)
  viewport->PlatformHandleRaw = (HWND)SDL_GetProperty(
      SDL_GetWindowProperties(window), "SDL.window.win32.hwnd", nullptr);
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
  viewport->PlatformHandleRaw = (void *)SDL_GetProperty(
      SDL_GetWindowProperties(window), "SDL.window.cocoa.window", nullptr);
#endif
}

static bool SDL3_Init(SDL_Window *window, SDL_Renderer *renderer,
                      void *sdl_gl_context) {
  IO &io = Gui::GetIO();
  ASSERT(io.BackendPlatformUserData == nullptr &&
         "Already initialized a platform backend!");
  UNUSED(sdl_gl_context); // Unused in this branch

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
  SDL3_Data *bd = NEW(SDL3_Data)();
  io.BackendPlatformUserData = (void *)bd;
  io.BackendPlatformName = "sdl3";
  io.BackendFlags |=
      BackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
                                    // values (optional)
  io.BackendFlags |=
      BackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos
                                   // requests (optional, rarely used)
  if (mouse_can_use_global_state)
    io.BackendFlags |=
        BackendFlags_PlatformHasViewports; // We can create
                                           // multi-viewports on the
                                           // Platform side (optional)

  bd->Window = window;
  bd->Renderer = renderer;

  // SDL on Linux/OSX doesn't report events for unfocused windows.
  // We will use 'MouseCanReportHoveredViewport' to set
  // 'BackendFlags_HasMouseHoveredViewport' dynamically each frame.
  bd->MouseCanUseGlobalState = mouse_can_use_global_state;
#ifndef __APPLE__
  bd->MouseCanReportHoveredViewport = bd->MouseCanUseGlobalState;
#else
  bd->MouseCanReportHoveredViewport = false;
#endif
  bd->WantUpdateMonitors = true;

  io.SetClipboardTextFn = SDL3_SetClipboardText;
  io.GetClipboardTextFn = SDL3_GetClipboardText;
  io.ClipboardUserData = nullptr;
  io.SetPlatformImeDataFn = SDL3_SetPlatformImeData;

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
  SDL3_SetupPlatformHandles(main_viewport, window);

  // From 2.0.5: Set SDL hint to receive mouse click events on window focus,
  // otherwise SDL doesn't emit the event. Without this, when clicking to gain
  // focus, our widgets wouldn't activate even though they showed as hovered.
  // (This is unfortunately a global SDL setting, so enabling it might have a
  // side-effect on your application. It is unlikely to make a difference, but
  // if your app absolutely needs to ignore the initial on-focus click: you can
  // ignore SDL_EVENT_MOUSE_BUTTON_DOWN events coming right after a
  // SDL_WINDOWEVENT_FOCUS_GAINED)
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

  // From 2.0.22: Disable auto-capture, this is preventing drag and drop across
  // multiple windows (see #5710)
  SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");

  // SDL 3.x : see https://github.com/libsdl-org/SDL/issues/6659
  SDL_SetHint("SDL_BORDERLESS_WINDOWED_STYLE", "0");

  // We need SDL_CaptureMouse(), SDL_GetGlobalMouseState() from SDL 2.0.4+ to
  // support multiple viewports. We left the call to
  // SDL3_InitPlatformInterface() outside of #ifdef to avoid
  // unused-function warnings.
  if ((io.ConfigFlags & ConfigFlags_ViewportsEnable) &&
      (io.BackendFlags & BackendFlags_PlatformHasViewports))
    SDL3_InitPlatformInterface(window, sdl_gl_context);

  return true;
}

bool SDL3_InitForOpenGL(SDL_Window *window, void *sdl_gl_context) {
  return SDL3_Init(window, nullptr, sdl_gl_context);
}

bool SDL3_InitForVulkan(SDL_Window *window) {
  if (!SDL3_Init(window, nullptr, nullptr))
    return false;
  SDL3_Data *bd = SDL3_GetBackendData();
  bd->UseVulkan = true;
  return true;
}

bool SDL3_InitForD3D(SDL_Window *window) {
#if !defined(_WIN32)
  ASSERT(0 && "Unsupported");
#endif
  return SDL3_Init(window, nullptr, nullptr);
}

bool SDL3_InitForMetal(SDL_Window *window) {
  return SDL3_Init(window, nullptr, nullptr);
}

bool SDL3_InitForSDLRenderer(SDL_Window *window, SDL_Renderer *renderer) {
  return SDL3_Init(window, renderer, nullptr);
}

bool SDL3_InitForOther(SDL_Window *window) {
  return SDL3_Init(window, nullptr, nullptr);
}

void SDL3_Shutdown() {
  SDL3_Data *bd = SDL3_GetBackendData();
  ASSERT(bd != nullptr &&
         "No platform backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  SDL3_ShutdownPlatformInterface();

  if (bd->ClipboardTextData)
    SDL_free(bd->ClipboardTextData);
  for (MouseCursor cursor_n = 0; cursor_n < MouseCursor_COUNT; cursor_n++)
    SDL_DestroyCursor(bd->MouseCursors[cursor_n]);
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
static void SDL3_UpdateMouseData() {
  SDL3_Data *bd = SDL3_GetBackendData();
  IO &io = Gui::GetIO();

  // We forward mouse input when hovered or captured (via
  // SDL_EVENT_MOUSE_MOTION) or when focused (below)
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
    // (Optional) Set OS mouse position from Gui if requested (rarely
    // used, only when ConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos) {
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
      if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
        SDL_WarpMouseGlobal(io.MousePos.x, io.MousePos.y);
      else
#endif
        SDL_WarpMouseInWindow(bd->Window, io.MousePos.x, io.MousePos.y);
    }

    // (Optional) Fallback to provide mouse position when focused
    // (SDL_EVENT_MOUSE_MOTION already provides this when hovered or captured)
    if (bd->MouseCanUseGlobalState && bd->MouseButtonsDown == 0) {
      // Single-viewport mode: mouse position in client window coordinates
      // (io.MousePos is (0,0) when the mouse is on the upper-left corner of the
      // app window) Multi-viewport mode: mouse position in OS absolute
      // coordinates (io.MousePos is (0,0) when the mouse is on the upper-left
      // of the primary monitor)
      float mouse_x, mouse_y;
      int window_x, window_y;
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
  // BackendFlags_HasMouseHoveredViewport is not set by the backend,
  // imGui will ignore this field and infer the information using its flawed
  // heuristic.
  // - [!] SDL backend does NOT correctly ignore viewports with the _NoInputs
  // flag.
  //       Some backend are not able to handle that correctly. If a backend
  //       report an hovered viewport that has the _NoInputs flag (e.g. when
  //       dragging a window for docking, the viewport has the _NoInputs flag in
  //       order to allow us to find the viewport under), then Gui is
  //       forced to ignore the value reported by the backend, and use its
  //       flawed heuristic to guess the viewport behind.
  // - [X] SDL backend correctly reports this regardless of another viewport
  // behind focused and dragged from (we need this to find a useful drag and
  // drop target).
  if (io.BackendFlags & BackendFlags_HasMouseHoveredViewport) {
    int mouse_viewport_id = 0;
    if (SDL_Window *sdl_mouse_window = SDL_GetWindowFromID(bd->MouseWindowID))
      if (Viewport *mouse_viewport =
              Gui::FindViewportByPlatformHandle((void *)sdl_mouse_window))
        mouse_viewport_id = mouse_viewport->ID;
    io.AddMouseViewportEvent(mouse_viewport_id);
  }
}

static void SDL3_UpdateMouseCursor() {
  IO &io = Gui::GetIO();
  if (io.ConfigFlags & ConfigFlags_NoMouseCursorChange)
    return;
  SDL3_Data *bd = SDL3_GetBackendData();

  int cursor = Gui::GetMouseCursor();
  if (io.MouseDrawCursor || cursor == MouseCursor_None) {
    // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
    SDL_HideCursor();
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
    SDL_ShowCursor();
  }
}

static void SDL3_UpdateGamepads() {
  IO &io = Gui::GetIO();
  if ((io.ConfigFlags & ConfigFlags_NavEnableGamepad) ==
      0) // FIXME: Technically feeding gamepad shouldn't depend on this now that
         // they are regular inputs.
    return;

  // Get gamepad
  io.BackendFlags &= ~BackendFlags_HasGamepad;
  SDL_Gamepad *gamepad = SDL_OpenGamepad(0);
  if (!gamepad)
    return;
  io.BackendFlags |= BackendFlags_HasGamepad;

// Update gamepad inputs
#define SATURATE(V) (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
#define MAP_BUTTON(KEY_NO, BUTTON_NO)                                          \
  { io.AddKeyEvent(KEY_NO, SDL_GetGamepadButton(gamepad, BUTTON_NO) != 0); }
#define MAP_ANALOG(KEY_NO, AXIS_NO, V0, V1)                                    \
  {                                                                            \
    float vn =                                                                 \
        (float)(SDL_GetGamepadAxis(gamepad, AXIS_NO) - V0) / (float)(V1 - V0); \
    vn = SATURATE(vn);                                                         \
    io.AddKeyAnalogEvent(KEY_NO, vn > 0.1f, vn);                               \
  }
  const int thumb_dead_zone =
      8000; // SDL_gamecontroller.h suggests using this value.
  MAP_BUTTON(Key_GamepadStart, SDL_GAMEPAD_BUTTON_START);
  MAP_BUTTON(Key_GamepadBack, SDL_GAMEPAD_BUTTON_BACK);
  MAP_BUTTON(Key_GamepadFaceLeft,
             SDL_GAMEPAD_BUTTON_WEST); // Xbox X, PS Square
  MAP_BUTTON(Key_GamepadFaceRight,
             SDL_GAMEPAD_BUTTON_EAST); // Xbox B, PS Circle
  MAP_BUTTON(Key_GamepadFaceUp,
             SDL_GAMEPAD_BUTTON_NORTH); // Xbox Y, PS Triangle
  MAP_BUTTON(Key_GamepadFaceDown,
             SDL_GAMEPAD_BUTTON_SOUTH); // Xbox A, PS Cross
  MAP_BUTTON(Key_GamepadDpadLeft, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
  MAP_BUTTON(Key_GamepadDpadRight, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
  MAP_BUTTON(Key_GamepadDpadUp, SDL_GAMEPAD_BUTTON_DPAD_UP);
  MAP_BUTTON(Key_GamepadDpadDown, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
  MAP_BUTTON(Key_GamepadL1, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
  MAP_BUTTON(Key_GamepadR1, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
  MAP_ANALOG(Key_GamepadL2, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, 0.0f, 32767);
  MAP_ANALOG(Key_GamepadR2, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.0f, 32767);
  MAP_BUTTON(Key_GamepadL3, SDL_GAMEPAD_BUTTON_LEFT_STICK);
  MAP_BUTTON(Key_GamepadR3, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
  MAP_ANALOG(Key_GamepadLStickLeft, SDL_GAMEPAD_AXIS_LEFTX, -thumb_dead_zone,
             -32768);
  MAP_ANALOG(Key_GamepadLStickRight, SDL_GAMEPAD_AXIS_LEFTX, +thumb_dead_zone,
             +32767);
  MAP_ANALOG(Key_GamepadLStickUp, SDL_GAMEPAD_AXIS_LEFTY, -thumb_dead_zone,
             -32768);
  MAP_ANALOG(Key_GamepadLStickDown, SDL_GAMEPAD_AXIS_LEFTY, +thumb_dead_zone,
             +32767);
  MAP_ANALOG(Key_GamepadRStickLeft, SDL_GAMEPAD_AXIS_RIGHTX, -thumb_dead_zone,
             -32768);
  MAP_ANALOG(Key_GamepadRStickRight, SDL_GAMEPAD_AXIS_RIGHTX, +thumb_dead_zone,
             +32767);
  MAP_ANALOG(Key_GamepadRStickUp, SDL_GAMEPAD_AXIS_RIGHTY, -thumb_dead_zone,
             -32768);
  MAP_ANALOG(Key_GamepadRStickDown, SDL_GAMEPAD_AXIS_RIGHTY, +thumb_dead_zone,
             +32767);
#undef MAP_BUTTON
#undef MAP_ANALOG
}

static void SDL3_UpdateMonitors() {
  SDL3_Data *bd = SDL3_GetBackendData();
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Monitors.resize(0);
  bd->WantUpdateMonitors = false;

  int display_count;
  SDL_DisplayID *displays = SDL_GetDisplays(&display_count);
  for (int n = 0; n < display_count; n++) {
    // Warning: the validity of monitor DPI information on Windows depends on
    // the application DPI awareness settings, which generally needs to be set
    // in the manifest or at runtime.
    SDL_DisplayID display_id = displays[n];
    PlatformMonitor monitor;
    SDL_Rect r;
    SDL_GetDisplayBounds(display_id, &r);
    monitor.MainPos = monitor.WorkPos = Vec2((float)r.x, (float)r.y);
    monitor.MainSize = monitor.WorkSize = Vec2((float)r.w, (float)r.h);
    SDL_GetDisplayUsableBounds(display_id, &r);
    monitor.WorkPos = Vec2((float)r.x, (float)r.y);
    monitor.WorkSize = Vec2((float)r.w, (float)r.h);
    // FIXME-VIEWPORT: On MacOS SDL reports actual monitor DPI scale, ignoring
    // OS configuration. We may want to set
    //  DpiScale to cocoa_window.backingScaleFactor here.
    monitor.DpiScale = SDL_GetDisplayContentScale(display_id);
    monitor.PlatformHandle = (void *)(intptr_t)n;
    platform_io.Monitors.push_back(monitor);
  }
}

void SDL3_NewFrame() {
  SDL3_Data *bd = SDL3_GetBackendData();
  ASSERT(bd != nullptr && "Did you call SDL3_Init()?");
  IO &io = Gui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  SDL_GetWindowSize(bd->Window, &w, &h);
  if (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_MINIMIZED)
    w = h = 0;
  SDL_GetWindowSizeInPixels(bd->Window, &display_w, &display_h);
  io.DisplaySize = Vec2((float)w, (float)h);
  if (w > 0 && h > 0)
    io.DisplayFramebufferScale =
        Vec2((float)display_w / w, (float)display_h / h);

  // Update monitors
  if (bd->WantUpdateMonitors)
    SDL3_UpdateMonitors();

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

  SDL3_UpdateMouseData();
  SDL3_UpdateMouseCursor();

  // Update game controllers (if enabled and available)
  SDL3_UpdateGamepads();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create
// and handle multiple viewports simultaneously. If you are new to gui or
// creating a new binding for gui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* RendererUserData field of each
// Viewport to easily retrieve our backend data.
struct SDL3_ViewportData {
  SDL_Window *Window;
  Uint32 WindowID;
  bool WindowOwned;
  SDL_GLContext GLContext;

  SDL3_ViewportData() {
    Window = nullptr;
    WindowID = 0;
    WindowOwned = false;
    GLContext = nullptr;
  }
  ~SDL3_ViewportData() { ASSERT(Window == nullptr && GLContext == nullptr); }
};

static void SDL3_CreateWindow(Viewport *viewport) {
  SDL3_Data *bd = SDL3_GetBackendData();
  SDL3_ViewportData *vd = NEW(SDL3_ViewportData)();
  viewport->PlatformUserData = vd;

  Viewport *main_viewport = Gui::GetMainViewport();
  SDL3_ViewportData *main_viewport_data =
      (SDL3_ViewportData *)main_viewport->PlatformUserData;

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
  sdl_flags |= SDL_GetWindowFlags(bd->Window);
  sdl_flags |= (viewport->Flags & ViewportFlags_NoDecoration)
                   ? SDL_WINDOW_BORDERLESS
                   : 0;
  sdl_flags |=
      (viewport->Flags & ViewportFlags_NoDecoration) ? 0 : SDL_WINDOW_RESIZABLE;
#if !defined(_WIN32)
  // See SDL hack in SDL3_ShowWindow().
  sdl_flags |=
      (viewport->Flags & ViewportFlags_NoTaskBarIcon) ? SDL_WINDOW_UTILITY : 0;
#endif
  sdl_flags |=
      (viewport->Flags & ViewportFlags_TopMost) ? SDL_WINDOW_ALWAYS_ON_TOP : 0;
  vd->Window = SDL_CreateWindow("No Title Yet", (int)viewport->Size.x,
                                (int)viewport->Size.y, sdl_flags);
  SDL_SetWindowPosition(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);
  vd->WindowOwned = true;
  if (use_opengl) {
    vd->GLContext = SDL_GL_CreateContext(vd->Window);
    SDL_GL_SetSwapInterval(0);
  }
  if (use_opengl && backup_context)
    SDL_GL_MakeCurrent(vd->Window, backup_context);

  SDL3_SetupPlatformHandles(viewport, vd->Window);
}

static void SDL3_DestroyWindow(Viewport *viewport) {
  if (SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData) {
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

static void SDL3_ShowWindow(Viewport *viewport) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
#if defined(_WIN32)
  HWND hwnd = (HWND)viewport->PlatformHandleRaw;

  // SDL hack: Hide icon from task bar
  // Note: SDL 3.0.0+ has a SDL_WINDOW_UTILITY flag which is supported under
  // Windows but the way it create the window breaks our seamless transition.
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

static Vec2 SDL3_GetWindowPos(Viewport *viewport) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  int x = 0, y = 0;
  SDL_GetWindowPosition(vd->Window, &x, &y);
  return Vec2((float)x, (float)y);
}

static void SDL3_SetWindowPos(Viewport *viewport, Vec2 pos) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowPosition(vd->Window, (int)pos.x, (int)pos.y);
}

static Vec2 SDL3_GetWindowSize(Viewport *viewport) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  int w = 0, h = 0;
  SDL_GetWindowSize(vd->Window, &w, &h);
  return Vec2((float)w, (float)h);
}

static void SDL3_SetWindowSize(Viewport *viewport, Vec2 size) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowSize(vd->Window, (int)size.x, (int)size.y);
}

static void SDL3_SetWindowTitle(Viewport *viewport, const char *title) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowTitle(vd->Window, title);
}

static void SDL3_SetWindowAlpha(Viewport *viewport, float alpha) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  SDL_SetWindowOpacity(vd->Window, alpha);
}

static void SDL3_SetWindowFocus(Viewport *viewport) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  SDL_RaiseWindow(vd->Window);
}

static bool SDL3_GetWindowFocus(Viewport *viewport) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

static bool SDL3_GetWindowMinimized(Viewport *viewport) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  return (SDL_GetWindowFlags(vd->Window) & SDL_WINDOW_MINIMIZED) != 0;
}

static void SDL3_RenderWindow(Viewport *viewport, void *) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  if (vd->GLContext)
    SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
}

static void SDL3_SwapBuffers(Viewport *viewport, void *) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  if (vd->GLContext) {
    SDL_GL_MakeCurrent(vd->Window, vd->GLContext);
    SDL_GL_SwapWindow(vd->Window);
  }
}

// Vulkan support (the Vulkan renderer needs to call a platform-side support
// function to create the surface) SDL is graceful enough to _not_ need
// <vulkan/vulkan.h> so we can safely include this.
#include <SDL3/SDL_vulkan.h>
static int SDL3_CreateVkSurface(Viewport *viewport,
                                unsigned long long vk_instance,
                                const void *vk_allocator,
                                unsigned long long *out_vk_surface) {
  SDL3_ViewportData *vd = (SDL3_ViewportData *)viewport->PlatformUserData;
  (void)vk_allocator;
  SDL_bool ret = SDL_Vulkan_CreateSurface(vd->Window, (VkInstance)vk_instance,
                                          (VkAllocationCallbacks *)vk_allocator,
                                          (VkSurfaceKHR *)out_vk_surface);
  return ret ? 0 : 1; // ret ? VK_SUCCESS : VK_NOT_READY
}

static void SDL3_InitPlatformInterface(SDL_Window *window,
                                       void *sdl_gl_context) {
  // Register platform interface (will be coupled with a renderer interface)
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Platform_CreateWindow = SDL3_CreateWindow;
  platform_io.Platform_DestroyWindow = SDL3_DestroyWindow;
  platform_io.Platform_ShowWindow = SDL3_ShowWindow;
  platform_io.Platform_SetWindowPos = SDL3_SetWindowPos;
  platform_io.Platform_GetWindowPos = SDL3_GetWindowPos;
  platform_io.Platform_SetWindowSize = SDL3_SetWindowSize;
  platform_io.Platform_GetWindowSize = SDL3_GetWindowSize;
  platform_io.Platform_SetWindowFocus = SDL3_SetWindowFocus;
  platform_io.Platform_GetWindowFocus = SDL3_GetWindowFocus;
  platform_io.Platform_GetWindowMinimized = SDL3_GetWindowMinimized;
  platform_io.Platform_SetWindowTitle = SDL3_SetWindowTitle;
  platform_io.Platform_RenderWindow = SDL3_RenderWindow;
  platform_io.Platform_SwapBuffers = SDL3_SwapBuffers;
  platform_io.Platform_SetWindowAlpha = SDL3_SetWindowAlpha;
  platform_io.Platform_CreateVkSurface = SDL3_CreateVkSurface;

  // Register main window handle (which is owned by the main application, not by
  // us) This is mostly for simplicity and consistency, so that our code (e.g.
  // mouse handling etc.) can use same logic for main and secondary viewports.
  Viewport *main_viewport = Gui::GetMainViewport();
  SDL3_ViewportData *vd = NEW(SDL3_ViewportData)();
  vd->Window = window;
  vd->WindowID = SDL_GetWindowID(window);
  vd->WindowOwned = false;
  vd->GLContext = sdl_gl_context;
  main_viewport->PlatformUserData = vd;
  main_viewport->PlatformHandle = vd->Window;
}

static void SDL3_ShutdownPlatformInterface() { Gui::DestroyPlatformWindows(); }

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef DISABLE
