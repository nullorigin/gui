// gui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows,
// inputs, OpenGL/Vulkan graphics context creation, etc.) (Requires: GLFW 3.1+.
// Prefer GLFW 3.3+ or GLFW 3.4+ for full feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen (Windows
//  only). [X] Platform: Keyboard support. Since 1.87 we are using the
//  io.AddKeyEvent() function. Pass Key values to all key functions e.g.
//  Gui::IsKeyPressed(Key_Space). [Legacy GLFW_KEY_* values will also be
//  supported unless DISABLE_OBSOLETE_KEYIO is set] [X] Platform: Gamepad
//  support. Enable with 'io.ConfigFlags |= ConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with
//  'io.ConfigFlags |= ConfigFlags_NoMouseCursorChange' (note: the resizing
//  cursors requires GLFW 3.4+). [X] Platform: Multi-viewport support (multiple
//  windows). Enable with 'io.ConfigFlags |= ConfigFlags_ViewportsEnable'.

// Issues:
//  [ ] Platform: Multi-viewport support: ParentViewportID not honored, and so
//  io.ConfigViewportsNoDefaultParent has no effect (minor).

#include "../gui.hpp"
#ifndef DISABLE
#include "glfw.hpp"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wold-style-cast" // warning: use of old-style cast
#pragma clang diagnostic ignored                                               \
    "-Wsign-conversion" // warning: implicit conversion changes signedness
#endif

// GLFW
#include <GLFW/glfw3.h>

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // for glfwGetWin32Window()
#endif
#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h> // for glfwGetCocoaWindow()
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// We gather version tests as define in order to easily see which features are
// version-dependent.
#define GLFW_VERSION_COMBINED                                                  \
  (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION)
#define GLFW_HAS_WINDOW_TOPMOST                                                \
  (GLFW_VERSION_COMBINED >= 3200) // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED                                                \
  (GLFW_VERSION_COMBINED >= 3300) // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA                                                  \
  (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI                                               \
  (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorContentScale
#if defined(__EMSCRIPTEN__) ||                                                 \
    defined(__SWITCH__) // no Vulkan support in GLFW for Emscripten or homebrew
                        // Nintendo Switch
#define GLFW_HAS_VULKAN (0)
#else
#define GLFW_HAS_VULKAN                                                        \
  (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwCreateWindowSurface
#endif
#define GLFW_HAS_FOCUS_WINDOW                                                  \
  (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwFocusWindow
#define GLFW_HAS_FOCUS_ON_SHOW                                                 \
  (GLFW_VERSION_COMBINED >= 3300) // 3.3+ GLFW_FOCUS_ON_SHOW
#define GLFW_HAS_MONITOR_WORK_AREA                                             \
  (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorWorkarea
#define GLFW_HAS_OSX_WINDOW_POS_FIX                                            \
  (GLFW_VERSION_COMBINED >=                                                    \
   3301) // 3.3.1+ Fixed: Resizing window repositions it on MacOS #1553
#ifdef GLFW_RESIZE_NESW_CURSOR // Let's be nice to people who pulled GLFW
                               // between 2019-04-16 (3.4 define) and 2019-11-29
                               // (cursors defines) // FIXME: Remove when
                               // GLFW 3.4 is released?
#define GLFW_HAS_NEW_CURSORS                                                   \
  (GLFW_VERSION_COMBINED >=                                                    \
   3400) // 3.4+ GLFW_RESIZE_ALL_CURSOR, GLFW_RESIZE_NESW_CURSOR,
         // GLFW_RESIZE_NWSE_CURSOR, GLFW_NOT_ALLOWED_CURSOR
#else
#define GLFW_HAS_NEW_CURSORS (0)
#endif
#ifdef GLFW_MOUSE_PASSTHROUGH // Let's be nice to people who pulled GLFW between
                              // 2019-04-16 (3.4 define) and 2020-07-17
                              // (passthrough)
#define GLFW_HAS_MOUSE_PASSTHROUGH                                             \
  (GLFW_VERSION_COMBINED >= 3400) // 3.4+ GLFW_MOUSE_PASSTHROUGH
#else
#define GLFW_HAS_MOUSE_PASSTHROUGH (0)
#endif
#define GLFW_HAS_GAMEPAD_API                                                   \
  (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetGamepadState() new api
#define GLFW_HAS_GETKEYNAME                                                    \
  (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwGetKeyName()
#define GLFW_HAS_GETERROR (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetError()

// GLFW data
enum GlfwClientApi {
  GlfwClientApi_Unknown,
  GlfwClientApi_OpenGL,
  GlfwClientApi_Vulkan
};

struct Glfw_Data {
  GLFWwindow *Window;
  GlfwClientApi ClientApi;
  double Time;
  GLFWwindow *MouseWindow;
  GLFWcursor *MouseCursors[MouseCursor_COUNT];
  Vec2 LastValidMousePos;
  GLFWwindow *KeyOwnerWindows[GLFW_KEY_LAST];
  bool InstalledCallbacks;
  bool CallbacksChainForAllWindows;
  bool WantUpdateMonitors;
#ifdef __EMSCRIPTEN__
  const char *CanvasSelector;
#endif

  // Chain GLFW callbacks: our callbacks will call the user's previously
  // installed callbacks, if any.
  GLFWwindowfocusfun PrevUserCallbackWindowFocus;
  GLFWcursorposfun PrevUserCallbackCursorPos;
  GLFWcursorenterfun PrevUserCallbackCursorEnter;
  GLFWmousebuttonfun PrevUserCallbackMousebutton;
  GLFWscrollfun PrevUserCallbackScroll;
  GLFWkeyfun PrevUserCallbackKey;
  GLFWcharfun PrevUserCallbackChar;
  GLFWmonitorfun PrevUserCallbackMonitor;
#ifdef _WIN32
  WNDPROC PrevWndProc;
#endif

  Glfw_Data() { memset((void *)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in
// this backend.
// - Because glfwPollEvents() process all windows and some events may be called
// outside of it, you will need to register your own callbacks
//   (passing install_callbacks=false in Glfw_InitXXX functions), set
//   the current gui context and then call our callbacks.
// - Otherwise we may need to store a GLFWWindow* -> Context* map and
// handle this in the backend, adding a little bit of extra complexity to it.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled
// when using multi-context.
static Glfw_Data *Glfw_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (Glfw_Data *)Gui::GetIO().BackendPlatformUserData
             : nullptr;
}

// Forward Declarations
static void Glfw_UpdateMonitors();
static void Glfw_InitPlatformInterface();
static void Glfw_ShutdownPlatformInterface();

// Functions
static const char *Glfw_GetClipboardText(void *user_data) {
  return glfwGetClipboardString((GLFWwindow *)user_data);
}

static void Glfw_SetClipboardText(void *user_data, const char *text) {
  glfwSetClipboardString((GLFWwindow *)user_data, text);
}

static Key Glfw_KeyToKey(int key) {
  switch (key) {
  case GLFW_KEY_TAB:
    return Key_Tab;
  case GLFW_KEY_LEFT:
    return Key_LeftArrow;
  case GLFW_KEY_RIGHT:
    return Key_RightArrow;
  case GLFW_KEY_UP:
    return Key_UpArrow;
  case GLFW_KEY_DOWN:
    return Key_DownArrow;
  case GLFW_KEY_PAGE_UP:
    return Key_PageUp;
  case GLFW_KEY_PAGE_DOWN:
    return Key_PageDown;
  case GLFW_KEY_HOME:
    return Key_Home;
  case GLFW_KEY_END:
    return Key_End;
  case GLFW_KEY_INSERT:
    return Key_Insert;
  case GLFW_KEY_DELETE:
    return Key_Delete;
  case GLFW_KEY_BACKSPACE:
    return Key_Backspace;
  case GLFW_KEY_SPACE:
    return Key_Space;
  case GLFW_KEY_ENTER:
    return Key_Enter;
  case GLFW_KEY_ESCAPE:
    return Key_Escape;
  case GLFW_KEY_APOSTROPHE:
    return Key_Apostrophe;
  case GLFW_KEY_COMMA:
    return Key_Comma;
  case GLFW_KEY_MINUS:
    return Key_Minus;
  case GLFW_KEY_PERIOD:
    return Key_Period;
  case GLFW_KEY_SLASH:
    return Key_Slash;
  case GLFW_KEY_SEMICOLON:
    return Key_Semicolon;
  case GLFW_KEY_EQUAL:
    return Key_Equal;
  case GLFW_KEY_LEFT_BRACKET:
    return Key_LeftBracket;
  case GLFW_KEY_BACKSLASH:
    return Key_Backslash;
  case GLFW_KEY_RIGHT_BRACKET:
    return Key_RightBracket;
  case GLFW_KEY_GRAVE_ACCENT:
    return Key_GraveAccent;
  case GLFW_KEY_CAPS_LOCK:
    return Key_CapsLock;
  case GLFW_KEY_SCROLL_LOCK:
    return Key_ScrollLock;
  case GLFW_KEY_NUM_LOCK:
    return Key_NumLock;
  case GLFW_KEY_PRINT_SCREEN:
    return Key_PrintScreen;
  case GLFW_KEY_PAUSE:
    return Key_Pause;
  case GLFW_KEY_KP_0:
    return Key_Keypad0;
  case GLFW_KEY_KP_1:
    return Key_Keypad1;
  case GLFW_KEY_KP_2:
    return Key_Keypad2;
  case GLFW_KEY_KP_3:
    return Key_Keypad3;
  case GLFW_KEY_KP_4:
    return Key_Keypad4;
  case GLFW_KEY_KP_5:
    return Key_Keypad5;
  case GLFW_KEY_KP_6:
    return Key_Keypad6;
  case GLFW_KEY_KP_7:
    return Key_Keypad7;
  case GLFW_KEY_KP_8:
    return Key_Keypad8;
  case GLFW_KEY_KP_9:
    return Key_Keypad9;
  case GLFW_KEY_KP_DECIMAL:
    return Key_KeypadDecimal;
  case GLFW_KEY_KP_DIVIDE:
    return Key_KeypadDivide;
  case GLFW_KEY_KP_MULTIPLY:
    return Key_KeypadMultiply;
  case GLFW_KEY_KP_SUBTRACT:
    return Key_KeypadSubtract;
  case GLFW_KEY_KP_ADD:
    return Key_KeypadAdd;
  case GLFW_KEY_KP_ENTER:
    return Key_KeypadEnter;
  case GLFW_KEY_KP_EQUAL:
    return Key_KeypadEqual;
  case GLFW_KEY_LEFT_SHIFT:
    return Key_LeftShift;
  case GLFW_KEY_LEFT_CONTROL:
    return Key_LeftCtrl;
  case GLFW_KEY_LEFT_ALT:
    return Key_LeftAlt;
  case GLFW_KEY_LEFT_SUPER:
    return Key_LeftSuper;
  case GLFW_KEY_RIGHT_SHIFT:
    return Key_RightShift;
  case GLFW_KEY_RIGHT_CONTROL:
    return Key_RightCtrl;
  case GLFW_KEY_RIGHT_ALT:
    return Key_RightAlt;
  case GLFW_KEY_RIGHT_SUPER:
    return Key_RightSuper;
  case GLFW_KEY_MENU:
    return Key_Menu;
  case GLFW_KEY_0:
    return Key_0;
  case GLFW_KEY_1:
    return Key_1;
  case GLFW_KEY_2:
    return Key_2;
  case GLFW_KEY_3:
    return Key_3;
  case GLFW_KEY_4:
    return Key_4;
  case GLFW_KEY_5:
    return Key_5;
  case GLFW_KEY_6:
    return Key_6;
  case GLFW_KEY_7:
    return Key_7;
  case GLFW_KEY_8:
    return Key_8;
  case GLFW_KEY_9:
    return Key_9;
  case GLFW_KEY_A:
    return Key_A;
  case GLFW_KEY_B:
    return Key_B;
  case GLFW_KEY_C:
    return Key_C;
  case GLFW_KEY_D:
    return Key_D;
  case GLFW_KEY_E:
    return Key_E;
  case GLFW_KEY_F:
    return Key_F;
  case GLFW_KEY_G:
    return Key_G;
  case GLFW_KEY_H:
    return Key_H;
  case GLFW_KEY_I:
    return Key_I;
  case GLFW_KEY_J:
    return Key_J;
  case GLFW_KEY_K:
    return Key_K;
  case GLFW_KEY_L:
    return Key_L;
  case GLFW_KEY_M:
    return Key_M;
  case GLFW_KEY_N:
    return Key_N;
  case GLFW_KEY_O:
    return Key_O;
  case GLFW_KEY_P:
    return Key_P;
  case GLFW_KEY_Q:
    return Key_Q;
  case GLFW_KEY_R:
    return Key_R;
  case GLFW_KEY_S:
    return Key_S;
  case GLFW_KEY_T:
    return Key_T;
  case GLFW_KEY_U:
    return Key_U;
  case GLFW_KEY_V:
    return Key_V;
  case GLFW_KEY_W:
    return Key_W;
  case GLFW_KEY_X:
    return Key_X;
  case GLFW_KEY_Y:
    return Key_Y;
  case GLFW_KEY_Z:
    return Key_Z;
  case GLFW_KEY_F1:
    return Key_F1;
  case GLFW_KEY_F2:
    return Key_F2;
  case GLFW_KEY_F3:
    return Key_F3;
  case GLFW_KEY_F4:
    return Key_F4;
  case GLFW_KEY_F5:
    return Key_F5;
  case GLFW_KEY_F6:
    return Key_F6;
  case GLFW_KEY_F7:
    return Key_F7;
  case GLFW_KEY_F8:
    return Key_F8;
  case GLFW_KEY_F9:
    return Key_F9;
  case GLFW_KEY_F10:
    return Key_F10;
  case GLFW_KEY_F11:
    return Key_F11;
  case GLFW_KEY_F12:
    return Key_F12;
  case GLFW_KEY_F13:
    return Key_F13;
  case GLFW_KEY_F14:
    return Key_F14;
  case GLFW_KEY_F15:
    return Key_F15;
  case GLFW_KEY_F16:
    return Key_F16;
  case GLFW_KEY_F17:
    return Key_F17;
  case GLFW_KEY_F18:
    return Key_F18;
  case GLFW_KEY_F19:
    return Key_F19;
  case GLFW_KEY_F20:
    return Key_F20;
  case GLFW_KEY_F21:
    return Key_F21;
  case GLFW_KEY_F22:
    return Key_F22;
  case GLFW_KEY_F23:
    return Key_F23;
  case GLFW_KEY_F24:
    return Key_F24;
  default:
    return Key_None;
  }
}

// X11 does not include current pressed/released modifier key in 'mods' flags
// submitted by GLFW
static void Glfw_UpdateKeyModifiers(GLFWwindow *window) {
  IO &io = Gui::GetIO();
  io.AddKeyEvent(
      Mod_Ctrl, (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
                    (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
  io.AddKeyEvent(Mod_Shift,
                 (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                     (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
  io.AddKeyEvent(Mod_Alt,
                 (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) ||
                     (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
  io.AddKeyEvent(Mod_Super,
                 (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) ||
                     (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));
}

static bool Glfw_ShouldChainCallback(GLFWwindow *window) {
  Glfw_Data *bd = Glfw_GetBackendData();
  return bd->CallbacksChainForAllWindows ? true : (window == bd->Window);
}

void Glfw_MouseButtonCallback(GLFWwindow *window, int button, int action,
                              int mods) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (bd->PrevUserCallbackMousebutton != nullptr &&
      Glfw_ShouldChainCallback(window))
    bd->PrevUserCallbackMousebutton(window, button, action, mods);

  Glfw_UpdateKeyModifiers(window);

  IO &io = Gui::GetIO();
  if (button >= 0 && button < MouseButton_COUNT)
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
}

void Glfw_ScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (bd->PrevUserCallbackScroll != nullptr && Glfw_ShouldChainCallback(window))
    bd->PrevUserCallbackScroll(window, xoffset, yoffset);

#ifdef __EMSCRIPTEN__
  // Ignore GLFW events: will be processed in
  // Emscripten_WheelCallback().
  return;
#endif

  IO &io = Gui::GetIO();
  io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}

static int Glfw_TranslateUntranslatedKey(int key, int scancode) {
#if GLFW_HAS_GETKEYNAME && !defined(__EMSCRIPTEN__)
  // GLFW 3.1+ attempts to "untranslate" keys, which goes the opposite of what
  // every other framework does, making using lettered shortcuts difficult. (It
  // had reasons to do so: namely GLFW is/was more likely to be used for
  // WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1
  // change could have been done differently).
  if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
    return key;
  GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
  const char *key_name = glfwGetKeyName(key, scancode);
  glfwSetErrorCallback(prev_error_callback);
#if GLFW_HAS_GETERROR && !defined(__EMSCRIPTEN__) // Eat errors (see #5908)
  (void)glfwGetError(nullptr);
#endif
  if (key_name && key_name[0] != 0 && key_name[1] == 0) {
    const char char_names[] = "`-=[]\\,;\'./";
    const int char_keys[] = {
        GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_MINUS,         GLFW_KEY_EQUAL,
        GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH,
        GLFW_KEY_COMMA,        GLFW_KEY_SEMICOLON,     GLFW_KEY_APOSTROPHE,
        GLFW_KEY_PERIOD,       GLFW_KEY_SLASH,         0};
    ASSERT(ARRAYSIZE(char_names) == ARRAYSIZE(char_keys));
    if (key_name[0] >= '0' && key_name[0] <= '9') {
      key = GLFW_KEY_0 + (key_name[0] - '0');
    } else if (key_name[0] >= 'A' && key_name[0] <= 'Z') {
      key = GLFW_KEY_A + (key_name[0] - 'A');
    } else if (key_name[0] >= 'a' && key_name[0] <= 'z') {
      key = GLFW_KEY_A + (key_name[0] - 'a');
    } else if (const char *p = strchr(char_names, key_name[0])) {
      key = char_keys[p - char_names];
    }
  }
  // if (action == GLFW_PRESS) printf("key %d scancode %d name '%s'\n", key,
  // scancode, key_name);
#else
  UNUSED(scancode);
#endif
  return key;
}

void Glfw_KeyCallback(GLFWwindow *window, int keycode, int scancode, int action,
                      int mods) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (bd->PrevUserCallbackKey != nullptr && Glfw_ShouldChainCallback(window))
    bd->PrevUserCallbackKey(window, keycode, scancode, action, mods);

  if (action != GLFW_PRESS && action != GLFW_RELEASE)
    return;

  Glfw_UpdateKeyModifiers(window);

  if (keycode >= 0 && keycode < ARRAYSIZE(bd->KeyOwnerWindows))
    bd->KeyOwnerWindows[keycode] = (action == GLFW_PRESS) ? window : nullptr;

  keycode = Glfw_TranslateUntranslatedKey(keycode, scancode);

  IO &io = Gui::GetIO();
  Key key = Glfw_KeyToKey(keycode);
  io.AddKeyEvent(key, (action == GLFW_PRESS));
  io.SetKeyEventNativeData(
      key, keycode,
      scancode); // To support legacy indexing (<1.87 user code)
}

void Glfw_WindowFocusCallback(GLFWwindow *window, int focused) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (bd->PrevUserCallbackWindowFocus != nullptr &&
      Glfw_ShouldChainCallback(window))
    bd->PrevUserCallbackWindowFocus(window, focused);

  IO &io = Gui::GetIO();
  io.AddFocusEvent(focused != 0);
}

void Glfw_CursorPosCallback(GLFWwindow *window, double x, double y) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (bd->PrevUserCallbackCursorPos != nullptr &&
      Glfw_ShouldChainCallback(window))
    bd->PrevUserCallbackCursorPos(window, x, y);

  IO &io = Gui::GetIO();
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
    int window_x, window_y;
    glfwGetWindowPos(window, &window_x, &window_y);
    x += window_x;
    y += window_y;
  }
  io.AddMousePosEvent((float)x, (float)y);
  bd->LastValidMousePos = Vec2((float)x, (float)y);
}

// Workaround: X11 seems to send spurious Leave/Enter events which would make us
// lose our position, so we back it up and restore on Leave/Enter.
void Glfw_CursorEnterCallback(GLFWwindow *window, int entered) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (bd->PrevUserCallbackCursorEnter != nullptr &&
      Glfw_ShouldChainCallback(window))
    bd->PrevUserCallbackCursorEnter(window, entered);

  IO &io = Gui::GetIO();
  if (entered) {
    bd->MouseWindow = window;
    io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
  } else if (!entered && bd->MouseWindow == window) {
    bd->LastValidMousePos = io.MousePos;
    bd->MouseWindow = nullptr;
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
  }
}

void Glfw_CharCallback(GLFWwindow *window, unsigned int c) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (bd->PrevUserCallbackChar != nullptr && Glfw_ShouldChainCallback(window))
    bd->PrevUserCallbackChar(window, c);

  IO &io = Gui::GetIO();
  io.AddInputCharacter(c);
}

void Glfw_MonitorCallback(GLFWmonitor *, int) {
  Glfw_Data *bd = Glfw_GetBackendData();
  bd->WantUpdateMonitors = true;
}

#ifdef __EMSCRIPTEN__
static EM_BOOL Emscripten_WheelCallback(int, const EmscriptenWheelEvent *ev,
                                        void *) {
  // Mimic Emscripten_HandleWheel() in SDL.
  // Corresponding equivalent in GLFW JS emulation layer has incorrect
  // quantizing preventing small values. See #6096
  float multiplier = 0.0f;
  if (ev->deltaMode == DOM_DELTA_PIXEL) {
    multiplier = 1.0f / 100.0f;
  } // 100 pixels make up a step.
  else if (ev->deltaMode == DOM_DELTA_LINE) {
    multiplier = 1.0f / 3.0f;
  } // 3 lines make up a step.
  else if (ev->deltaMode == DOM_DELTA_PAGE) {
    multiplier = 80.0f;
  } // A page makes up 80 steps.
  float wheel_x = ev->deltaX * -multiplier;
  float wheel_y = ev->deltaY * -multiplier;
  IO &io = Gui::GetIO();
  io.AddMouseWheelEvent(wheel_x, wheel_y);
  // DEBUG_LOG("[Emsc] mode %d dx: %.2f, dy: %.2f, dz: %.2f --> feed %.2f
  // %.2f\n", (int)ev->deltaMode, ev->deltaX, ev->deltaY, ev->deltaZ, wheel_x,
  // wheel_y);
  return EM_TRUE;
}
#endif

#ifdef _WIN32
static LRESULT CALLBACK Glfw_WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                                     LPARAM lParam);
#endif

void Glfw_InstallCallbacks(GLFWwindow *window) {
  Glfw_Data *bd = Glfw_GetBackendData();
  ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
  ASSERT(bd->Window == window);

  bd->PrevUserCallbackWindowFocus =
      glfwSetWindowFocusCallback(window, Glfw_WindowFocusCallback);
  bd->PrevUserCallbackCursorEnter =
      glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
  bd->PrevUserCallbackCursorPos =
      glfwSetCursorPosCallback(window, Glfw_CursorPosCallback);
  bd->PrevUserCallbackMousebutton =
      glfwSetMouseButtonCallback(window, Glfw_MouseButtonCallback);
  bd->PrevUserCallbackScroll =
      glfwSetScrollCallback(window, Glfw_ScrollCallback);
  bd->PrevUserCallbackKey = glfwSetKeyCallback(window, Glfw_KeyCallback);
  bd->PrevUserCallbackChar = glfwSetCharCallback(window, Glfw_CharCallback);
  bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(Glfw_MonitorCallback);
  bd->InstalledCallbacks = true;
}

void Glfw_RestoreCallbacks(GLFWwindow *window) {
  Glfw_Data *bd = Glfw_GetBackendData();
  ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
  ASSERT(bd->Window == window);

  glfwSetWindowFocusCallback(window, bd->PrevUserCallbackWindowFocus);
  glfwSetCursorEnterCallback(window, bd->PrevUserCallbackCursorEnter);
  glfwSetCursorPosCallback(window, bd->PrevUserCallbackCursorPos);
  glfwSetMouseButtonCallback(window, bd->PrevUserCallbackMousebutton);
  glfwSetScrollCallback(window, bd->PrevUserCallbackScroll);
  glfwSetKeyCallback(window, bd->PrevUserCallbackKey);
  glfwSetCharCallback(window, bd->PrevUserCallbackChar);
  glfwSetMonitorCallback(bd->PrevUserCallbackMonitor);
  bd->InstalledCallbacks = false;
  bd->PrevUserCallbackWindowFocus = nullptr;
  bd->PrevUserCallbackCursorEnter = nullptr;
  bd->PrevUserCallbackCursorPos = nullptr;
  bd->PrevUserCallbackMousebutton = nullptr;
  bd->PrevUserCallbackScroll = nullptr;
  bd->PrevUserCallbackKey = nullptr;
  bd->PrevUserCallbackChar = nullptr;
  bd->PrevUserCallbackMonitor = nullptr;
}

// Set to 'true' to enable chaining installed callbacks for all windows
// (including secondary viewports created by backends or by user. This is
// 'false' by default meaning we only chain callbacks for the main viewport. We
// cannot set this to 'true' by default because user callbacks code may be not
// testing the 'window' parameter of their callback. If you set this to 'true'
// your user callback code will need to make sure you are testing the 'window'
// parameter.
void Glfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows) {
  Glfw_Data *bd = Glfw_GetBackendData();
  bd->CallbacksChainForAllWindows = chain_for_all_windows;
}

static bool Glfw_Init(GLFWwindow *window, bool install_callbacks,
                      GlfwClientApi client_api) {
  IO &io = Gui::GetIO();
  ASSERT(io.BackendPlatformUserData == nullptr &&
         "Already initialized a platform backend!");
  // printf("GLFW_VERSION: %d.%d.%d (%d)", GLFW_VERSION_MAJOR,
  // GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, GLFW_VERSION_COMBINED);

  // Setup backend capabilities flags
  Glfw_Data *bd = NEW(Glfw_Data)();
  io.BackendPlatformUserData = (void *)bd;
  io.BackendPlatformName = "glfw";
  io.BackendFlags |=
      BackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
                                    // values (optional)
  io.BackendFlags |=
      BackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos
                                   // requests (optional, rarely used)
#ifndef __EMSCRIPTEN__
  io.BackendFlags |=
      BackendFlags_PlatformHasViewports; // We can create multi-viewports
                                         // on the Platform side (optional)
#endif
#if GLFW_HAS_MOUSE_PASSTHROUGH || (GLFW_HAS_WINDOW_HOVERED && defined(_WIN32))
  io.BackendFlags |=
      BackendFlags_HasMouseHoveredViewport; // We can call
                                            // io.AddMouseViewportEvent()
                                            // with correct data (optional)
#endif

  bd->Window = window;
  bd->Time = 0.0;
  bd->WantUpdateMonitors = true;

  io.SetClipboardTextFn = Glfw_SetClipboardText;
  io.GetClipboardTextFn = Glfw_GetClipboardText;
  io.ClipboardUserData = bd->Window;

  // Create mouse cursors
  // (By design, on X11 cursors are user configurable and some cursors may be
  // missing. When a cursor doesn't exist, GLFW will emit an error which will
  // often be printed by the app, so we temporarily disable error reporting.
  // Missing cursors will return nullptr and our _UpdateMouseCursor() function
  // will use the Arrow cursor instead.)
  GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
  bd->MouseCursors[MouseCursor_Arrow] =
      glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  bd->MouseCursors[MouseCursor_TextInput] =
      glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
  bd->MouseCursors[MouseCursor_ResizeNS] =
      glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
  bd->MouseCursors[MouseCursor_ResizeEW] =
      glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
  bd->MouseCursors[MouseCursor_Hand] =
      glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
  bd->MouseCursors[MouseCursor_ResizeAll] =
      glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
  bd->MouseCursors[MouseCursor_ResizeNESW] =
      glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
  bd->MouseCursors[MouseCursor_ResizeNWSE] =
      glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
  bd->MouseCursors[MouseCursor_NotAllowed] =
      glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
  bd->MouseCursors[MouseCursor_ResizeAll] =
      glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  bd->MouseCursors[MouseCursor_ResizeNESW] =
      glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  bd->MouseCursors[MouseCursor_ResizeNWSE] =
      glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  bd->MouseCursors[MouseCursor_NotAllowed] =
      glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
  glfwSetErrorCallback(prev_error_callback);
#if GLFW_HAS_GETERROR && !defined(__EMSCRIPTEN__) // Eat errors (see #5908)
  (void)glfwGetError(nullptr);
#endif

  // Chain GLFW callbacks: our callbacks will call the user's previously
  // installed callbacks, if any.
  if (install_callbacks)
    Glfw_InstallCallbacks(window);
    // Register Emscripten Wheel callback to workaround issue in Emscripten GLFW
    // Emulation (#6096) We intentionally do not check 'if (install_callbacks)'
    // here, as some users may set it to false and call GLFW callback
    // themselves.
    // FIXME: May break chaining in case user registered their own Emscripten
    // callback?
#ifdef __EMSCRIPTEN__
  emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr,
                                false, Emscripten_WheelCallback);
#endif

  // Update monitors the first time (note: monitor callback are broken in
  // GLFW 3.2 and earlier, see github.com/glfw/glfw/issues/784)
  Glfw_UpdateMonitors();
  glfwSetMonitorCallback(Glfw_MonitorCallback);

  // Set platform dependent data in viewport
  Viewport *main_viewport = Gui::GetMainViewport();
  main_viewport->PlatformHandle = (void *)bd->Window;
#ifdef _WIN32
  main_viewport->PlatformHandleRaw = glfwGetWin32Window(bd->Window);
#elif defined(__APPLE__)
  main_viewport->PlatformHandleRaw = (void *)glfwGetCocoaWindow(bd->Window);
#else
  UNUSED(main_viewport);
#endif
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
    Glfw_InitPlatformInterface();

    // Windows: register a WndProc hook so we can intercept some messages.
#ifdef _WIN32
  bd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW(
      (HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC);
  ASSERT(bd->PrevWndProc != nullptr);
  ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC,
                      (LONG_PTR)Glfw_WndProc);
#endif

  bd->ClientApi = client_api;
  return true;
}

bool Glfw_InitForOpenGL(GLFWwindow *window, bool install_callbacks) {
  return Glfw_Init(window, install_callbacks, GlfwClientApi_OpenGL);
}

bool Glfw_InitForVulkan(GLFWwindow *window, bool install_callbacks) {
  return Glfw_Init(window, install_callbacks, GlfwClientApi_Vulkan);
}

bool Glfw_InitForOther(GLFWwindow *window, bool install_callbacks) {
  return Glfw_Init(window, install_callbacks, GlfwClientApi_Unknown);
}

void Glfw_Shutdown() {
  Glfw_Data *bd = Glfw_GetBackendData();
  ASSERT(bd != nullptr &&
         "No platform backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  Glfw_ShutdownPlatformInterface();

  if (bd->InstalledCallbacks)
    Glfw_RestoreCallbacks(bd->Window);
#ifdef __EMSCRIPTEN__
  emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr,
                                false, nullptr);
#endif

  for (int cursor_n = 0; cursor_n < MouseCursor_COUNT; cursor_n++)
    glfwDestroyCursor(bd->MouseCursors[cursor_n]);

    // Windows: restore our WndProc hook
#ifdef _WIN32
  Viewport *main_viewport = Gui::GetMainViewport();
  ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC,
                      (LONG_PTR)bd->PrevWndProc);
  bd->PrevWndProc = nullptr;
#endif

  io.BackendPlatformName = nullptr;
  io.BackendPlatformUserData = nullptr;
  io.BackendFlags &=
      ~(BackendFlags_HasMouseCursors | BackendFlags_HasSetMousePos |
        BackendFlags_HasGamepad | BackendFlags_PlatformHasViewports |
        BackendFlags_HasMouseHoveredViewport);
  DELETE(bd);
}

static void Glfw_UpdateMouseData() {
  Glfw_Data *bd = Glfw_GetBackendData();
  IO &io = Gui::GetIO();
  PlatformIO &platform_io = Gui::GetPlatformIO();

  int mouse_viewport_id = 0;
  const Vec2 mouse_pos_prev = io.MousePos;
  for (int n = 0; n < platform_io.Viewports.Size; n++) {
    Viewport *viewport = platform_io.Viewports[n];
    GLFWwindow *window = (GLFWwindow *)viewport->PlatformHandle;

#ifdef __EMSCRIPTEN__
    const bool is_window_focused = true;
#else
    const bool is_window_focused =
        glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
#endif
    if (is_window_focused) {
      // (Optional) Set OS mouse position from Gui if requested (rarely
      // used, only when ConfigFlags_NavEnableSetMousePos is enabled by
      // user) When multi-viewports are enabled, all Gui positions are
      // same as OS positions.
      if (io.WantSetMousePos)
        glfwSetCursorPos(window, (double)(mouse_pos_prev.x - viewport->Pos.x),
                         (double)(mouse_pos_prev.y - viewport->Pos.y));

      // (Optional) Fallback to provide mouse position when focused
      // (Glfw_CursorPosCallback already provides this when hovered or
      // captured)
      if (bd->MouseWindow == nullptr) {
        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
          // Single viewport mode: mouse position in client window coordinates
          // (io.MousePos is (0,0) when the mouse is on the upper-left corner of
          // the app window) Multi-viewport mode: mouse position in OS absolute
          // coordinates (io.MousePos is (0,0) when the mouse is on the
          // upper-left of the primary monitor)
          int window_x, window_y;
          glfwGetWindowPos(window, &window_x, &window_y);
          mouse_x += window_x;
          mouse_y += window_y;
        }
        bd->LastValidMousePos = Vec2((float)mouse_x, (float)mouse_y);
        io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
      }
    }

    // (Optional) When using multiple viewports: call io.AddMouseViewportEvent()
    // with the viewport the OS mouse cursor is hovering. If
    // BackendFlags_HasMouseHoveredViewport is not set by the backend,
    // imGui will ignore this field and infer the information using its flawed
    // heuristic.
    // - [X] GLFW >= 3.3 backend ON WINDOWS ONLY does correctly ignore viewports
    // with the _NoInputs flag.
    // - [!] GLFW <= 3.2 backend CANNOT correctly ignore viewports with the
    // _NoInputs flag, and CANNOT reported Hovered Viewport because of mouse
    // capture.
    //       Some backend are not able to handle that correctly. If a backend
    //       report an hovered viewport that has the _NoInputs flag (e.g. when
    //       dragging a window for docking, the viewport has the _NoInputs flag
    //       in order to allow us to find the viewport under), then Gui
    //       is forced to ignore the value reported by the backend, and use its
    //       flawed heuristic to guess the viewport behind.
    // - [X] GLFW backend correctly reports this regardless of another viewport
    // behind focused and dragged from (we need this to find a useful drag and
    // drop target).
    // FIXME: This is currently only correct on Win32. See what we do below with
    // the WM_NCHITTEST, missing an equivalent for other systems.
#if GLFW_HAS_MOUSE_PASSTHROUGH || (GLFW_HAS_WINDOW_HOVERED && defined(_WIN32))
    const bool window_no_input =
        (viewport->Flags & ViewportFlags_NoInputs) != 0;
#if GLFW_HAS_MOUSE_PASSTHROUGH
    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, window_no_input);
#endif
    if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !window_no_input)
      mouse_viewport_id = viewport->ID;
#else
    // We cannot use bd->MouseWindow maintained from CursorEnter/Leave
    // callbacks, because it is locked to the window capturing mouse.
#endif
  }

  if (io.BackendFlags & BackendFlags_HasMouseHoveredViewport)
    io.AddMouseViewportEvent(mouse_viewport_id);
}

static void Glfw_UpdateMouseCursor() {
  IO &io = Gui::GetIO();
  Glfw_Data *bd = Glfw_GetBackendData();
  if ((io.ConfigFlags & ConfigFlags_NoMouseCursorChange) ||
      glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    return;

  int cursor = Gui::GetMouseCursor();
  PlatformIO &platform_io = Gui::GetPlatformIO();
  for (int n = 0; n < platform_io.Viewports.Size; n++) {
    GLFWwindow *window = (GLFWwindow *)platform_io.Viewports[n]->PlatformHandle;
    if (cursor == MouseCursor_None || io.MouseDrawCursor) {
      // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    } else {
      // Show OS mouse cursor
      // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse
      // cursor with GLFW 3.2, but 3.3 works here.
      glfwSetCursor(window, bd->MouseCursors[cursor]
                                ? bd->MouseCursors[cursor]
                                : bd->MouseCursors[MouseCursor_Arrow]);
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

// Update gamepad inputs
static inline float Saturate(float v) {
  return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}
static void Glfw_UpdateGamepads() {
  IO &io = Gui::GetIO();
  if ((io.ConfigFlags & ConfigFlags_NavEnableGamepad) ==
      0) // FIXME: Technically feeding gamepad shouldn't depend on this now that
         // they are regular inputs.
    return;

  io.BackendFlags &= ~BackendFlags_HasGamepad;
#if GLFW_HAS_GAMEPAD_API && !defined(__EMSCRIPTEN__)
  GLFWgamepadstate gamepad;
  if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
    return;
#define MAP_BUTTON(KEY_NO, BUTTON_NO, _UNUSED)                                 \
  do {                                                                         \
    io.AddKeyEvent(KEY_NO, gamepad.buttons[BUTTON_NO] != 0);                   \
  } while (0)
#define MAP_ANALOG(KEY_NO, AXIS_NO, _UNUSED, V0, V1)                           \
  do {                                                                         \
    float v = gamepad.axes[AXIS_NO];                                           \
    v = (v - V0) / (V1 - V0);                                                  \
    io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v));                      \
  } while (0)
#else
  int axes_count = 0, buttons_count = 0;
  const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
  const unsigned char *buttons =
      glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
  if (axes_count == 0 || buttons_count == 0)
    return;
#define MAP_BUTTON(KEY_NO, _UNUSED, BUTTON_NO)                                 \
  do {                                                                         \
    io.AddKeyEvent(KEY_NO, (buttons_count > BUTTON_NO &&                       \
                            buttons[BUTTON_NO] == GLFW_PRESS));                \
  } while (0)
#define MAP_ANALOG(KEY_NO, _UNUSED, AXIS_NO, V0, V1)                           \
  do {                                                                         \
    float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0;                     \
    v = (v - V0) / (V1 - V0);                                                  \
    io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v));                      \
  } while (0)
#endif
  io.BackendFlags |= BackendFlags_HasGamepad;
  MAP_BUTTON(Key_GamepadStart, GLFW_GAMEPAD_BUTTON_START, 7);
  MAP_BUTTON(Key_GamepadBack, GLFW_GAMEPAD_BUTTON_BACK, 6);
  MAP_BUTTON(Key_GamepadFaceLeft, GLFW_GAMEPAD_BUTTON_X,
             2); // Xbox X, PS Square
  MAP_BUTTON(Key_GamepadFaceRight, GLFW_GAMEPAD_BUTTON_B,
             1); // Xbox B, PS Circle
  MAP_BUTTON(Key_GamepadFaceUp, GLFW_GAMEPAD_BUTTON_Y,
             3); // Xbox Y, PS Triangle
  MAP_BUTTON(Key_GamepadFaceDown, GLFW_GAMEPAD_BUTTON_A,
             0); // Xbox A, PS Cross
  MAP_BUTTON(Key_GamepadDpadLeft, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, 13);
  MAP_BUTTON(Key_GamepadDpadRight, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, 11);
  MAP_BUTTON(Key_GamepadDpadUp, GLFW_GAMEPAD_BUTTON_DPAD_UP, 10);
  MAP_BUTTON(Key_GamepadDpadDown, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, 12);
  MAP_BUTTON(Key_GamepadL1, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, 4);
  MAP_BUTTON(Key_GamepadR1, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, 5);
  MAP_ANALOG(Key_GamepadL2, GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, 4, -0.75f, +1.0f);
  MAP_ANALOG(Key_GamepadR2, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, 5, -0.75f, +1.0f);
  MAP_BUTTON(Key_GamepadL3, GLFW_GAMEPAD_BUTTON_LEFT_THUMB, 8);
  MAP_BUTTON(Key_GamepadR3, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, 9);
  MAP_ANALOG(Key_GamepadLStickLeft, GLFW_GAMEPAD_AXIS_LEFT_X, 0, -0.25f, -1.0f);
  MAP_ANALOG(Key_GamepadLStickRight, GLFW_GAMEPAD_AXIS_LEFT_X, 0, +0.25f,
             +1.0f);
  MAP_ANALOG(Key_GamepadLStickUp, GLFW_GAMEPAD_AXIS_LEFT_Y, 1, -0.25f, -1.0f);
  MAP_ANALOG(Key_GamepadLStickDown, GLFW_GAMEPAD_AXIS_LEFT_Y, 1, +0.25f, +1.0f);
  MAP_ANALOG(Key_GamepadRStickLeft, GLFW_GAMEPAD_AXIS_RIGHT_X, 2, -0.25f,
             -1.0f);
  MAP_ANALOG(Key_GamepadRStickRight, GLFW_GAMEPAD_AXIS_RIGHT_X, 2, +0.25f,
             +1.0f);
  MAP_ANALOG(Key_GamepadRStickUp, GLFW_GAMEPAD_AXIS_RIGHT_Y, 3, -0.25f, -1.0f);
  MAP_ANALOG(Key_GamepadRStickDown, GLFW_GAMEPAD_AXIS_RIGHT_Y, 3, +0.25f,
             +1.0f);
#undef MAP_BUTTON
#undef MAP_ANALOG
}

static void Glfw_UpdateMonitors() {
  Glfw_Data *bd = Glfw_GetBackendData();
  PlatformIO &platform_io = Gui::GetPlatformIO();
  bd->WantUpdateMonitors = false;

  int monitors_count = 0;
  GLFWmonitor **glfw_monitors = glfwGetMonitors(&monitors_count);
  if (monitors_count == 0) // Preserve existing monitor list if there are none.
                           // Happens on macOS sleeping (#5683)
    return;

  platform_io.Monitors.resize(0);
  for (int n = 0; n < monitors_count; n++) {
    PlatformMonitor monitor;
    int x, y;
    glfwGetMonitorPos(glfw_monitors[n], &x, &y);
    const GLFWvidmode *vid_mode = glfwGetVideoMode(glfw_monitors[n]);
    if (vid_mode == nullptr)
      continue; // Failed to get Video mode (e.g. Emscripten does not support
                // this function)
    monitor.MainPos = monitor.WorkPos = Vec2((float)x, (float)y);
    monitor.MainSize = monitor.WorkSize =
        Vec2((float)vid_mode->width, (float)vid_mode->height);
#if GLFW_HAS_MONITOR_WORK_AREA
    int w, h;
    glfwGetMonitorWorkarea(glfw_monitors[n], &x, &y, &w, &h);
    if (w > 0 &&
        h > 0) // Workaround a small GLFW issue reporting zero on monitor
               // changes: https://github.com/glfw/glfw/pull/1761
    {
      monitor.WorkPos = Vec2((float)x, (float)y);
      monitor.WorkSize = Vec2((float)w, (float)h);
    }
#endif
#if GLFW_HAS_PER_MONITOR_DPI
    // Warning: the validity of monitor DPI information on Windows depends on
    // the application DPI awareness settings, which generally needs to be set
    // in the manifest or at runtime.
    float x_scale, y_scale;
    glfwGetMonitorContentScale(glfw_monitors[n], &x_scale, &y_scale);
    monitor.DpiScale = x_scale;
#endif
    monitor.PlatformHandle = (void *)
        glfw_monitors[n]; // [...] GLFW doc states: "guaranteed to be valid only
                          // until the monitor configuration changes"
    platform_io.Monitors.push_back(monitor);
  }
}

void Glfw_NewFrame() {
  IO &io = Gui::GetIO();
  Glfw_Data *bd = Glfw_GetBackendData();
  ASSERT(bd != nullptr && "Did you call Glfw_InitForXXX()?");

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(bd->Window, &w, &h);
  glfwGetFramebufferSize(bd->Window, &display_w, &display_h);
  io.DisplaySize = Vec2((float)w, (float)h);
  if (w > 0 && h > 0)
    io.DisplayFramebufferScale =
        Vec2((float)display_w / (float)w, (float)display_h / (float)h);
  if (bd->WantUpdateMonitors)
    Glfw_UpdateMonitors();

  // Setup time step
  // (Accept glfwGetTime() not returning a monotonically increasing value. Seems
  // to happens on disconnecting peripherals and probably on VMs and Emscripten,
  // see #6491, #6189, #6114, #3644)
  double current_time = glfwGetTime();
  if (current_time <= bd->Time)
    current_time = bd->Time + 0.00001f;
  io.DeltaTime =
      bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
  bd->Time = current_time;

  Glfw_UpdateMouseData();
  Glfw_UpdateMouseCursor();

  // Update game controllers (if enabled and available)
  Glfw_UpdateGamepads();
}

#ifdef __EMSCRIPTEN__
static EM_BOOL Glfw_OnCanvasSizeChange(int event_type,
                                       const EmscriptenUiEvent *event,
                                       void *user_data) {
  Glfw_Data *bd = (Glfw_Data *)user_data;
  double canvas_width, canvas_height;
  emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width,
                                  &canvas_height);
  glfwSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
  return true;
}

static EM_BOOL Emscripten_FullscreenChangeCallback(
    int event_type, const EmscriptenFullscreenChangeEvent *event,
    void *user_data) {
  Glfw_Data *bd = (Glfw_Data *)user_data;
  double canvas_width, canvas_height;
  emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width,
                                  &canvas_height);
  glfwSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
  return true;
}

// 'canvas_selector' is a CSS selector. The event listener is applied to the
// first element that matches the query. STRING MUST PERSIST FOR THE APPLICATION
// DURATION. PLEASE USE A STRING LITERAL OR ENSURE POINTER WILL STAY VALID.
void Glfw_InstallEmscriptenCanvasResizeCallback(const char *canvas_selector) {
  ASSERT(canvas_selector != nullptr);
  Glfw_Data *bd = Glfw_GetBackendData();
  ASSERT(bd != nullptr && "Did you call Glfw_InitForXXX()?");

  bd->CanvasSelector = canvas_selector;
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, bd, false,
                                 Glfw_OnCanvasSizeChange);
  emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, bd,
                                           false,
                                           Emscripten_FullscreenChangeCallback);

  // Change the size of the GLFW window according to the size of the canvas
  Glfw_OnCanvasSizeChange(EMSCRIPTEN_EVENT_RESIZE, {}, bd);
}
#endif

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create
// and handle multiple viewports simultaneously. If you are new to gui or
// creating a new binding for gui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* RendererUserData field of each
// Viewport to easily retrieve our backend data.
struct Glfw_ViewportData {
  GLFWwindow *Window;
  bool WindowOwned;
  int IgnoreWindowPosEventFrame;
  int IgnoreWindowSizeEventFrame;
#ifdef _WIN32
  WNDPROC PrevWndProc;
#endif

  Glfw_ViewportData() {
    memset(this, 0, sizeof(*this));
    IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1;
  }
  ~Glfw_ViewportData() { ASSERT(Window == nullptr); }
};

static void Glfw_WindowCloseCallback(GLFWwindow *window) {
  if (Viewport *viewport = Gui::FindViewportByPlatformHandle(window))
    viewport->PlatformRequestClose = true;
}

// GLFW may dispatch window pos/size events after calling
// glfwSetWindowPos()/glfwSetWindowSize(). However: depending on the platform
// the callback may be invoked at different time:
// - on Windows it appears to be called within the
// glfwSetWindowPos()/glfwSetWindowSize() call
// - on Linux it is queued and invoked during glfwPollEvents()
// Because the event doesn't always fire on glfwSetWindowXXX() we use a frame
// counter tag to only ignore recent glfwSetWindowXXX() calls.
static void Glfw_WindowPosCallback(GLFWwindow *window, int, int) {
  if (Viewport *viewport = Gui::FindViewportByPlatformHandle(window)) {
    if (Glfw_ViewportData *vd =
            (Glfw_ViewportData *)viewport->PlatformUserData) {
      bool ignore_event =
          (Gui::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
      // data->IgnoreWindowPosEventFrame = -1;
      if (ignore_event)
        return;
    }
    viewport->PlatformRequestMove = true;
  }
}

static void Glfw_WindowSizeCallback(GLFWwindow *window, int, int) {
  if (Viewport *viewport = Gui::FindViewportByPlatformHandle(window)) {
    if (Glfw_ViewportData *vd =
            (Glfw_ViewportData *)viewport->PlatformUserData) {
      bool ignore_event =
          (Gui::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
      // data->IgnoreWindowSizeEventFrame = -1;
      if (ignore_event)
        return;
    }
    viewport->PlatformRequestResize = true;
  }
}

static void Glfw_CreateWindow(Viewport *viewport) {
  Glfw_Data *bd = Glfw_GetBackendData();
  Glfw_ViewportData *vd = NEW(Glfw_ViewportData)();
  viewport->PlatformUserData = vd;

  // GLFW 3.2 unfortunately always set focus on glfwCreateWindow() if
  // GLFW_VISIBLE is set, regardless of GLFW_FOCUSED With GLFW 3.3, the hint
  // GLFW_FOCUS_ON_SHOW fixes this problem
  glfwWindowHint(GLFW_VISIBLE, false);
  glfwWindowHint(GLFW_FOCUSED, false);
#if GLFW_HAS_FOCUS_ON_SHOW
  glfwWindowHint(GLFW_FOCUS_ON_SHOW, false);
#endif
  glfwWindowHint(GLFW_DECORATED,
                 (viewport->Flags & ViewportFlags_NoDecoration) ? false : true);
#if GLFW_HAS_WINDOW_TOPMOST
  glfwWindowHint(GLFW_FLOATING,
                 (viewport->Flags & ViewportFlags_TopMost) ? true : false);
#endif
  GLFWwindow *share_window =
      (bd->ClientApi == GlfwClientApi_OpenGL) ? bd->Window : nullptr;
  vd->Window = glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y,
                                "No Title Yet", nullptr, share_window);
  vd->WindowOwned = true;
  viewport->PlatformHandle = (void *)vd->Window;
#ifdef _WIN32
  viewport->PlatformHandleRaw = glfwGetWin32Window(vd->Window);
#elif defined(__APPLE__)
  viewport->PlatformHandleRaw = (void *)glfwGetCocoaWindow(vd->Window);
#endif
  glfwSetWindowPos(vd->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

  // Install GLFW callbacks for secondary viewports
  glfwSetWindowFocusCallback(vd->Window, Glfw_WindowFocusCallback);
  glfwSetCursorEnterCallback(vd->Window, Glfw_CursorEnterCallback);
  glfwSetCursorPosCallback(vd->Window, Glfw_CursorPosCallback);
  glfwSetMouseButtonCallback(vd->Window, Glfw_MouseButtonCallback);
  glfwSetScrollCallback(vd->Window, Glfw_ScrollCallback);
  glfwSetKeyCallback(vd->Window, Glfw_KeyCallback);
  glfwSetCharCallback(vd->Window, Glfw_CharCallback);
  glfwSetWindowCloseCallback(vd->Window, Glfw_WindowCloseCallback);
  glfwSetWindowPosCallback(vd->Window, Glfw_WindowPosCallback);
  glfwSetWindowSizeCallback(vd->Window, Glfw_WindowSizeCallback);
  if (bd->ClientApi == GlfwClientApi_OpenGL) {
    glfwMakeContextCurrent(vd->Window);
    glfwSwapInterval(0);
  }
}

static void Glfw_DestroyWindow(Viewport *viewport) {
  Glfw_Data *bd = Glfw_GetBackendData();
  if (Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData) {
    if (vd->WindowOwned) {
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
      HWND hwnd = (HWND)viewport->PlatformHandleRaw;
      ::RemovePropA(hwnd, "VIEWPORT");
#endif

      // Release any keys that were pressed in the window being destroyed and
      // are still held down, because we will not receive any release events
      // after window is destroyed.
      for (int i = 0; i < ARRAYSIZE(bd->KeyOwnerWindows); i++)
        if (bd->KeyOwnerWindows[i] == vd->Window)
          Glfw_KeyCallback(vd->Window, i, 0, GLFW_RELEASE,
                           0); // Later params are only used for main viewport,
                               // on which this function is never called.

      glfwDestroyWindow(vd->Window);
    }
    vd->Window = nullptr;
    DELETE(vd);
  }
  viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void Glfw_ShowWindow(Viewport *viewport) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;

#if defined(_WIN32)
  // GLFW hack: Hide icon from task bar
  HWND hwnd = (HWND)viewport->PlatformHandleRaw;
  if (viewport->Flags & ViewportFlags_NoTaskBarIcon) {
    LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
    ex_style &= ~WS_EX_APPWINDOW;
    ex_style |= WS_EX_TOOLWINDOW;
    ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
  }

  // GLFW hack: install hook for WM_NCHITTEST message handler
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED && defined(_WIN32)
  ::SetPropA(hwnd, "VIEWPORT", viewport);
  vd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
  ::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)Glfw_WndProc);
#endif

#if !GLFW_HAS_FOCUS_ON_SHOW
  // GLFW hack: GLFW 3.2 has a bug where glfwShowWindow() also activates/focus
  // the window. The fix was pushed to GLFW repository on 2018/01/09 and should
  // be included in GLFW 3.3 via a GLFW_FOCUS_ON_SHOW window attribute.
  // FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
  if (viewport->Flags & ViewportFlags_NoFocusOnAppearing) {
    ::ShowWindow(hwnd, SW_SHOWNA);
    return;
  }
#endif
#endif

  glfwShowWindow(vd->Window);
}

static Vec2 Glfw_GetWindowPos(Viewport *viewport) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  int x = 0, y = 0;
  glfwGetWindowPos(vd->Window, &x, &y);
  return Vec2((float)x, (float)y);
}

static void Glfw_SetWindowPos(Viewport *viewport, Vec2 pos) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  vd->IgnoreWindowPosEventFrame = Gui::GetFrameCount();
  glfwSetWindowPos(vd->Window, (int)pos.x, (int)pos.y);
}

static Vec2 Glfw_GetWindowSize(Viewport *viewport) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  int w = 0, h = 0;
  glfwGetWindowSize(vd->Window, &w, &h);
  return Vec2((float)w, (float)h);
}

static void Glfw_SetWindowSize(Viewport *viewport, Vec2 size) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
#if __APPLE__ && !GLFW_HAS_OSX_WINDOW_POS_FIX
  // Native OS windows are positioned from the bottom-left corner on macOS,
  // whereas on other platforms they are positioned from the upper-left corner.
  // GLFW makes an effort to convert macOS style coordinates, however it doesn't
  // handle it when changing size. We are manually moving the window in order
  // for changes of size to be based on the upper-left corner.
  int x, y, width, height;
  glfwGetWindowPos(vd->Window, &x, &y);
  glfwGetWindowSize(vd->Window, &width, &height);
  glfwSetWindowPos(vd->Window, x, y - height + size.y);
#endif
  vd->IgnoreWindowSizeEventFrame = Gui::GetFrameCount();
  glfwSetWindowSize(vd->Window, (int)size.x, (int)size.y);
}

static void Glfw_SetWindowTitle(Viewport *viewport, const char *title) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  glfwSetWindowTitle(vd->Window, title);
}

static void Glfw_SetWindowFocus(Viewport *viewport) {
#if GLFW_HAS_FOCUS_WINDOW
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  glfwFocusWindow(vd->Window);
#else
  // FIXME: What are the effect of not having this function? At the moment imgui
  // doesn't actually call SetWindowFocus - we set that up ahead, will answer
  // that question later.
  (void)viewport;
#endif
}

static bool Glfw_GetWindowFocus(Viewport *viewport) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  return glfwGetWindowAttrib(vd->Window, GLFW_FOCUSED) != 0;
}

static bool Glfw_GetWindowMinimized(Viewport *viewport) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  return glfwGetWindowAttrib(vd->Window, GLFW_ICONIFIED) != 0;
}

#if GLFW_HAS_WINDOW_ALPHA
static void Glfw_SetWindowAlpha(Viewport *viewport, float alpha) {
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  glfwSetWindowOpacity(vd->Window, alpha);
}
#endif

static void Glfw_RenderWindow(Viewport *viewport, void *) {
  Glfw_Data *bd = Glfw_GetBackendData();
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  if (bd->ClientApi == GlfwClientApi_OpenGL)
    glfwMakeContextCurrent(vd->Window);
}

static void Glfw_SwapBuffers(Viewport *viewport, void *) {
  Glfw_Data *bd = Glfw_GetBackendData();
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  if (bd->ClientApi == GlfwClientApi_OpenGL) {
    glfwMakeContextCurrent(vd->Window);
    glfwSwapBuffers(vd->Window);
  }
}

//--------------------------------------------------------------------------------------------------------
// Vulkan support (the Vulkan renderer needs to call a platform-side support
// function to create the surface)
//--------------------------------------------------------------------------------------------------------

// Avoid including <vulkan.h> so we can build without it
#if GLFW_HAS_VULKAN
#ifndef VULKAN_H_
#define VK_DEFINE_HANDLE(object) typedef struct object##_T *object;
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) ||             \
    defined(_M_X64) || defined(__ia64) || defined(_M_IA64) ||                  \
    defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)                              \
  typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
struct VkAllocationCallbacks;
enum VkResult { VK_RESULT_MAX_ENUM = 0x7FFFFFFF };
#endif // VULKAN_H_
extern "C" {
extern GLFWAPI VkResult glfwCreateWindowSurface(
    VkInstance instance, GLFWwindow *window,
    const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
}
static int Glfw_CreateVkSurface(Viewport *viewport,
                                unsigned long long vk_instance,
                                const void *vk_allocator,
                                unsigned long long *out_vk_surface) {
  Glfw_Data *bd = Glfw_GetBackendData();
  Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData;
  UNUSED(bd);
  ASSERT(bd->ClientApi == GlfwClientApi_Vulkan);
  VkResult err =
      glfwCreateWindowSurface((VkInstance)vk_instance, vd->Window,
                              (const VkAllocationCallbacks *)vk_allocator,
                              (VkSurfaceKHR *)out_vk_surface);
  return (int)err;
}
#endif // GLFW_HAS_VULKAN

static void Glfw_InitPlatformInterface() {
  // Register platform interface (will be coupled with a renderer interface)
  Glfw_Data *bd = Glfw_GetBackendData();
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Platform_CreateWindow = Glfw_CreateWindow;
  platform_io.Platform_DestroyWindow = Glfw_DestroyWindow;
  platform_io.Platform_ShowWindow = Glfw_ShowWindow;
  platform_io.Platform_SetWindowPos = Glfw_SetWindowPos;
  platform_io.Platform_GetWindowPos = Glfw_GetWindowPos;
  platform_io.Platform_SetWindowSize = Glfw_SetWindowSize;
  platform_io.Platform_GetWindowSize = Glfw_GetWindowSize;
  platform_io.Platform_SetWindowFocus = Glfw_SetWindowFocus;
  platform_io.Platform_GetWindowFocus = Glfw_GetWindowFocus;
  platform_io.Platform_GetWindowMinimized = Glfw_GetWindowMinimized;
  platform_io.Platform_SetWindowTitle = Glfw_SetWindowTitle;
  platform_io.Platform_RenderWindow = Glfw_RenderWindow;
  platform_io.Platform_SwapBuffers = Glfw_SwapBuffers;
#if GLFW_HAS_WINDOW_ALPHA
  platform_io.Platform_SetWindowAlpha = Glfw_SetWindowAlpha;
#endif
#if GLFW_HAS_VULKAN
  platform_io.Platform_CreateVkSurface = Glfw_CreateVkSurface;
#endif

  // Register main window handle (which is owned by the main application, not by
  // us) This is mostly for simplicity and consistency, so that our code (e.g.
  // mouse handling etc.) can use same logic for main and secondary viewports.
  Viewport *main_viewport = Gui::GetMainViewport();
  Glfw_ViewportData *vd = NEW(Glfw_ViewportData)();
  vd->Window = bd->Window;
  vd->WindowOwned = false;
  main_viewport->PlatformUserData = vd;
  main_viewport->PlatformHandle = (void *)bd->Window;
}

static void Glfw_ShutdownPlatformInterface() { Gui::DestroyPlatformWindows(); }

//-----------------------------------------------------------------------------

// WndProc hook (declared here because we will need access to
// Glfw_ViewportData)
#ifdef _WIN32
static MouseSource GetMouseSourceFromMessageExtraInfo() {
  LPARAM extra_info = ::GetMessageExtraInfo();
  if ((extra_info & 0xFFFFFF80) == 0xFF515700)
    return MouseSource_Pen;
  if ((extra_info & 0xFFFFFF80) == 0xFF515780)
    return MouseSource_TouchScreen;
  return MouseSource_Mouse;
}
static LRESULT CALLBACK Glfw_WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                                     LPARAM lParam) {
  Glfw_Data *bd = Glfw_GetBackendData();
  WNDPROC prev_wndproc = bd->PrevWndProc;
  Viewport *viewport = (Viewport *)::GetPropA(hWnd, "VIEWPORT");
  if (viewport != NULL)
    if (Glfw_ViewportData *vd = (Glfw_ViewportData *)viewport->PlatformUserData)
      prev_wndproc = vd->PrevWndProc;

  switch (msg) {
    // GLFW doesn't allow to distinguish Mouse vs TouchScreen vs Pen.
    // Add support for Win32 (based on win32), because we rely on
    // _TouchScreen info to trickle inputs differently.
  case WM_MOUSEMOVE:
  case WM_NCMOUSEMOVE:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONDBLCLK:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONDBLCLK:
  case WM_MBUTTONUP:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONDBLCLK:
  case WM_XBUTTONUP:
    Gui::GetIO().AddMouseSourceEvent(GetMouseSourceFromMessageExtraInfo());
    break;

    // We have submitted https://github.com/glfw/glfw/pull/1568 to allow GLFW to
    // support "transparent inputs". In the meanwhile we implement custom
    // per-platform workarounds here (FIXME-VIEWPORT: Implement same work-around
    // for Linux/OSX!)
#if !GLFW_HAS_MOUSE_PASSTHROUGH && GLFW_HAS_WINDOW_HOVERED
  case WM_NCHITTEST: {
    // Let mouse pass-through the window. This will allow the backend to call
    // io.AddMouseViewportEvent() properly (which is OPTIONAL). The
    // ViewportFlags_NoInputs flag is set while dragging a viewport, as
    // want to detect the window behind the one we are dragging. If you cannot
    // easily access those viewport flags from your windowing/event code: you
    // may manually synchronize its state e.g. in your main loop after calling
    // UpdatePlatformWindows(). Iterate all viewports/platform windows and pass
    // the flag to your windowing system.
    if (viewport && (viewport->Flags & ViewportFlags_NoInputs))
      return HTTRANSPARENT;
    break;
  }
#endif
  }
  return ::CallWindowProcW(prev_wndproc, hWnd, msg, wParam, lParam);
}
#endif // #ifdef _WIN32

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef DISABLE
