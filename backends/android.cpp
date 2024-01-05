// gui: Platform Binding for Android native app
// This needs to be used along with the OpenGL 3 Renderer (opengl3)

// Implemented features:
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent()
//  function. Pass Key values to all key functions e.g.
//  Gui::IsKeyPressed(Key_Space). [Legacy AKEYCODE_* values will also be
//  supported unless DISABLE_OBSOLETE_KEYIO is set] [X] Platform: Mouse support.
//  Can discriminate Mouse/TouchScreen/Pen.
// Missing features:
//  [ ] Platform: Clipboard support.
//  [ ] Platform: Gamepad support. Enable with 'io.ConfigFlags |=
//  ConfigFlags_NavEnableGamepad'. [ ] Platform: Mouse cursor shape and
//  visibility. Disable with 'io.ConfigFlags |=
//  ConfigFlags_NoMouseCursorChange'. FIXME: Check if this is even possible
//  with Android. [ ] Platform: Multi-viewport support (multiple windows). Not
//  meaningful on Android.
// Important:
//  - Consider using SDL or GLFW backend on Android, which will be more
//  full-featured than this.
//  - FIXME: On-screen keyboard currently needs to be enabled by the application
//  (see examples/ and issue #3446)
//  - FIXME: Unicode character inputs needs to be passed by Gui by the
//  application (see examples/ and issue #3446)
#include "../gui.hpp"
#ifndef DISABLE
#include "android.hpp"
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <android/native_window.h>
#include <time.h>

// Android data
static double g_Time = 0.0;
static ANativeWindow *g_Window;
static char g_LogTag[] = "Example";

static Key Android_KeyCodeToKey(int32_t key_code) {
  switch (key_code) {
  case AKEYCODE_TAB:
    return Key_Tab;
  case AKEYCODE_DPAD_LEFT:
    return Key_LeftArrow;
  case AKEYCODE_DPAD_RIGHT:
    return Key_RightArrow;
  case AKEYCODE_DPAD_UP:
    return Key_UpArrow;
  case AKEYCODE_DPAD_DOWN:
    return Key_DownArrow;
  case AKEYCODE_PAGE_UP:
    return Key_PageUp;
  case AKEYCODE_PAGE_DOWN:
    return Key_PageDown;
  case AKEYCODE_MOVE_HOME:
    return Key_Home;
  case AKEYCODE_MOVE_END:
    return Key_End;
  case AKEYCODE_INSERT:
    return Key_Insert;
  case AKEYCODE_FORWARD_DEL:
    return Key_Delete;
  case AKEYCODE_DEL:
    return Key_Backspace;
  case AKEYCODE_SPACE:
    return Key_Space;
  case AKEYCODE_ENTER:
    return Key_Enter;
  case AKEYCODE_ESCAPE:
    return Key_Escape;
  case AKEYCODE_APOSTROPHE:
    return Key_Apostrophe;
  case AKEYCODE_COMMA:
    return Key_Comma;
  case AKEYCODE_MINUS:
    return Key_Minus;
  case AKEYCODE_PERIOD:
    return Key_Period;
  case AKEYCODE_SLASH:
    return Key_Slash;
  case AKEYCODE_SEMICOLON:
    return Key_Semicolon;
  case AKEYCODE_EQUALS:
    return Key_Equal;
  case AKEYCODE_LEFT_BRACKET:
    return Key_LeftBracket;
  case AKEYCODE_BACKSLASH:
    return Key_Backslash;
  case AKEYCODE_RIGHT_BRACKET:
    return Key_RightBracket;
  case AKEYCODE_GRAVE:
    return Key_GraveAccent;
  case AKEYCODE_CAPS_LOCK:
    return Key_CapsLock;
  case AKEYCODE_SCROLL_LOCK:
    return Key_ScrollLock;
  case AKEYCODE_NUM_LOCK:
    return Key_NumLock;
  case AKEYCODE_SYSRQ:
    return Key_PrintScreen;
  case AKEYCODE_BREAK:
    return Key_Pause;
  case AKEYCODE_NUMPAD_0:
    return Key_Keypad0;
  case AKEYCODE_NUMPAD_1:
    return Key_Keypad1;
  case AKEYCODE_NUMPAD_2:
    return Key_Keypad2;
  case AKEYCODE_NUMPAD_3:
    return Key_Keypad3;
  case AKEYCODE_NUMPAD_4:
    return Key_Keypad4;
  case AKEYCODE_NUMPAD_5:
    return Key_Keypad5;
  case AKEYCODE_NUMPAD_6:
    return Key_Keypad6;
  case AKEYCODE_NUMPAD_7:
    return Key_Keypad7;
  case AKEYCODE_NUMPAD_8:
    return Key_Keypad8;
  case AKEYCODE_NUMPAD_9:
    return Key_Keypad9;
  case AKEYCODE_NUMPAD_DOT:
    return Key_KeypadDecimal;
  case AKEYCODE_NUMPAD_DIVIDE:
    return Key_KeypadDivide;
  case AKEYCODE_NUMPAD_MULTIPLY:
    return Key_KeypadMultiply;
  case AKEYCODE_NUMPAD_SUBTRACT:
    return Key_KeypadSubtract;
  case AKEYCODE_NUMPAD_ADD:
    return Key_KeypadAdd;
  case AKEYCODE_NUMPAD_ENTER:
    return Key_KeypadEnter;
  case AKEYCODE_NUMPAD_EQUALS:
    return Key_KeypadEqual;
  case AKEYCODE_CTRL_LEFT:
    return Key_LeftCtrl;
  case AKEYCODE_SHIFT_LEFT:
    return Key_LeftShift;
  case AKEYCODE_ALT_LEFT:
    return Key_LeftAlt;
  case AKEYCODE_META_LEFT:
    return Key_LeftSuper;
  case AKEYCODE_CTRL_RIGHT:
    return Key_RightCtrl;
  case AKEYCODE_SHIFT_RIGHT:
    return Key_RightShift;
  case AKEYCODE_ALT_RIGHT:
    return Key_RightAlt;
  case AKEYCODE_META_RIGHT:
    return Key_RightSuper;
  case AKEYCODE_MENU:
    return Key_Menu;
  case AKEYCODE_0:
    return Key_0;
  case AKEYCODE_1:
    return Key_1;
  case AKEYCODE_2:
    return Key_2;
  case AKEYCODE_3:
    return Key_3;
  case AKEYCODE_4:
    return Key_4;
  case AKEYCODE_5:
    return Key_5;
  case AKEYCODE_6:
    return Key_6;
  case AKEYCODE_7:
    return Key_7;
  case AKEYCODE_8:
    return Key_8;
  case AKEYCODE_9:
    return Key_9;
  case AKEYCODE_A:
    return Key_A;
  case AKEYCODE_B:
    return Key_B;
  case AKEYCODE_C:
    return Key_C;
  case AKEYCODE_D:
    return Key_D;
  case AKEYCODE_E:
    return Key_E;
  case AKEYCODE_F:
    return Key_F;
  case AKEYCODE_G:
    return Key_G;
  case AKEYCODE_H:
    return Key_H;
  case AKEYCODE_I:
    return Key_I;
  case AKEYCODE_J:
    return Key_J;
  case AKEYCODE_K:
    return Key_K;
  case AKEYCODE_L:
    return Key_L;
  case AKEYCODE_M:
    return Key_M;
  case AKEYCODE_N:
    return Key_N;
  case AKEYCODE_O:
    return Key_O;
  case AKEYCODE_P:
    return Key_P;
  case AKEYCODE_Q:
    return Key_Q;
  case AKEYCODE_R:
    return Key_R;
  case AKEYCODE_S:
    return Key_S;
  case AKEYCODE_T:
    return Key_T;
  case AKEYCODE_U:
    return Key_U;
  case AKEYCODE_V:
    return Key_V;
  case AKEYCODE_W:
    return Key_W;
  case AKEYCODE_X:
    return Key_X;
  case AKEYCODE_Y:
    return Key_Y;
  case AKEYCODE_Z:
    return Key_Z;
  case AKEYCODE_F1:
    return Key_F1;
  case AKEYCODE_F2:
    return Key_F2;
  case AKEYCODE_F3:
    return Key_F3;
  case AKEYCODE_F4:
    return Key_F4;
  case AKEYCODE_F5:
    return Key_F5;
  case AKEYCODE_F6:
    return Key_F6;
  case AKEYCODE_F7:
    return Key_F7;
  case AKEYCODE_F8:
    return Key_F8;
  case AKEYCODE_F9:
    return Key_F9;
  case AKEYCODE_F10:
    return Key_F10;
  case AKEYCODE_F11:
    return Key_F11;
  case AKEYCODE_F12:
    return Key_F12;
  default:
    return Key_None;
  }
}

int32_t Android_HandleInputEvent(const AInputEvent *input_event) {
  IO &io = Gui::GetIO();
  int32_t event_type = AInputEvent_getType(input_event);
  switch (event_type) {
  case AINPUT_EVENT_TYPE_KEY: {
    int32_t event_key_code = AKeyEvent_getKeyCode(input_event);
    int32_t event_scan_code = AKeyEvent_getScanCode(input_event);
    int32_t event_action = AKeyEvent_getAction(input_event);
    int32_t event_meta_state = AKeyEvent_getMetaState(input_event);

    io.AddKeyEvent(Mod_Ctrl, (event_meta_state & AMETA_CTRL_ON) != 0);
    io.AddKeyEvent(Mod_Shift, (event_meta_state & AMETA_SHIFT_ON) != 0);
    io.AddKeyEvent(Mod_Alt, (event_meta_state & AMETA_ALT_ON) != 0);
    io.AddKeyEvent(Mod_Super, (event_meta_state & AMETA_META_ON) != 0);

    switch (event_action) {
    // FIXME: AKEY_EVENT_ACTION_DOWN and AKEY_EVENT_ACTION_UP occur at once as
    // soon as a touch pointer goes up from a key. We use a simple key event
    // queue/ and process one event per key per frame in
    // Android_NewFrame().
    case AKEY_EVENT_ACTION_DOWN:
    case AKEY_EVENT_ACTION_UP: {
      Key key = Android_KeyCodeToKey(event_key_code);
      if (key != Key_None) {
        io.AddKeyEvent(key, event_action == AKEY_EVENT_ACTION_DOWN);
        io.SetKeyEventNativeData(key, event_key_code, event_scan_code);
      }

      break;
    }
    default:
      break;
    }
    break;
  }
  case AINPUT_EVENT_TYPE_MOTION: {
    int32_t event_action = AMotionEvent_getAction(input_event);
    int32_t event_pointer_index =
        (event_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
        AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
    event_action &= AMOTION_EVENT_ACTION_MASK;

    switch (AMotionEvent_getToolType(input_event, event_pointer_index)) {
    case AMOTION_EVENT_TOOL_TYPE_MOUSE:
      io.AddMouseSourceEvent(MouseSource_Mouse);
      break;
    case AMOTION_EVENT_TOOL_TYPE_STYLUS:
    case AMOTION_EVENT_TOOL_TYPE_ERASER:
      io.AddMouseSourceEvent(MouseSource_Pen);
      break;
    case AMOTION_EVENT_TOOL_TYPE_FINGER:
    default:
      io.AddMouseSourceEvent(MouseSource_TouchScreen);
      break;
    }

    switch (event_action) {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_UP: {
      // Physical mouse buttons (and probably other physical devices) also
      // invoke the actions AMOTION_EVENT_ACTION_DOWN/_UP, but we have to
      // process them separately to identify the actual button pressed. This is
      // done below via AMOTION_EVENT_ACTION_BUTTON_PRESS/_RELEASE. Here, we
      // only process "FINGER" input (and "UNKNOWN", as a fallback).
      int tool_type =
          AMotionEvent_getToolType(input_event, event_pointer_index);
      if (tool_type == AMOTION_EVENT_TOOL_TYPE_FINGER ||
          tool_type == AMOTION_EVENT_TOOL_TYPE_UNKNOWN) {
        io.AddMousePosEvent(
            AMotionEvent_getX(input_event, event_pointer_index),
            AMotionEvent_getY(input_event, event_pointer_index));
        io.AddMouseButtonEvent(0, event_action == AMOTION_EVENT_ACTION_DOWN);
      }
      break;
    }
    case AMOTION_EVENT_ACTION_BUTTON_PRESS:
    case AMOTION_EVENT_ACTION_BUTTON_RELEASE: {
      int32_t button_state = AMotionEvent_getButtonState(input_event);
      io.AddMouseButtonEvent(0, (button_state & AMOTION_EVENT_BUTTON_PRIMARY) !=
                                    0);
      io.AddMouseButtonEvent(
          1, (button_state & AMOTION_EVENT_BUTTON_SECONDARY) != 0);
      io.AddMouseButtonEvent(
          2, (button_state & AMOTION_EVENT_BUTTON_TERTIARY) != 0);
      break;
    }
    case AMOTION_EVENT_ACTION_HOVER_MOVE: // Hovering: Tool moves while NOT
                                          // pressed (such as a physical mouse)
    case AMOTION_EVENT_ACTION_MOVE:       // Touch pointer moves while DOWN
      io.AddMousePosEvent(AMotionEvent_getX(input_event, event_pointer_index),
                          AMotionEvent_getY(input_event, event_pointer_index));
      break;
    case AMOTION_EVENT_ACTION_SCROLL:
      io.AddMouseWheelEvent(
          AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_HSCROLL,
                                    event_pointer_index),
          AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_VSCROLL,
                                    event_pointer_index));
      break;
    default:
      break;
    }
  }
    return 1;
  default:
    break;
  }

  return 0;
}

bool Android_Init(ANativeWindow *window) {
  g_Window = window;
  g_Time = 0.0;

  // Setup backend capabilities flags
  IO &io = Gui::GetIO();
  io.BackendPlatformName = "android";

  return true;
}

void Android_Shutdown() {
  IO &io = Gui::GetIO();
  io.BackendPlatformName = nullptr;
}

void Android_NewFrame() {
  IO &io = Gui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  int32_t window_width = ANativeWindow_getWidth(g_Window);
  int32_t window_height = ANativeWindow_getHeight(g_Window);
  int display_width = window_width;
  int display_height = window_height;

  io.DisplaySize = Vec2((float)window_width, (float)window_height);
  if (window_width > 0 && window_height > 0)
    io.DisplayFramebufferScale = Vec2((float)display_width / window_width,
                                      (float)display_height / window_height);

  // Setup time step
  struct timespec current_timespec;
  clock_gettime(CLOCK_MONOTONIC, &current_timespec);
  double current_time = (double)(current_timespec.tv_sec) +
                        (current_timespec.tv_nsec / 1000000000.0);
  io.DeltaTime =
      g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
  g_Time = current_time;
}

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
