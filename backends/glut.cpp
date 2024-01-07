// gui: Platform Backend for GLUT/FreeGLUT
// This needs to be used along with a Renderer (e.g. OpenGL2)

// !!! GLUT/FreeGLUT IS OBSOLETE PREHISTORIC SOFTWARE. Using GLUT is not
// recommended unless you really miss the 90's. !!!
// !!! If someone or something is teaching you GLUT today, you are being abused.
// Please show some resistance. !!!
// !!! Nowadays, prefer using GLFW or SDL instead!

// Implemented features:
//  [X] Platform: Partial keyboard support. Since 1.87 we are using the
//  io.AddKeyEvent() function. Pass Key values to all key functions e.g.
//  Gui::IsKeyPressed(Key_Space). [Legacy GLUT values will also be
//  supported unless DISABLE_OBSOLETE_KEYIO is set]
// Missing features:
//  [ ] Platform: GLUT is unable to distinguish e.g. Backspace from CTRL+H or
//  TAB from CTRL+I [ ] Platform: Missing horizontal mouse wheel support. [ ]
//  Platform: Missing mouse cursor shape/visibility support. [ ] Platform:
//  Missing clipboard support (not supported by Glut). [ ] Platform: Missing
//  gamepad support.

#include "../gui.hpp"
#ifndef DISABLE
#include "glut.hpp"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4505) // unreferenced local function has been removed
                                // (stb stuff)
#endif

static int g_Time = 0; // Current time, in milliseconds

// Glut has 1 function for characters and one for "special keys". We map the
// characters in the 0..255 range and the keys above.
static Key GLUT_KeyToKey(int key) {
  switch (key) {
  case '\t':
    return Key_Tab;
  case 256 + GLUT_KEY_LEFT:
    return Key_LeftArrow;
  case 256 + GLUT_KEY_RIGHT:
    return Key_RightArrow;
  case 256 + GLUT_KEY_UP:
    return Key_UpArrow;
  case 256 + GLUT_KEY_DOWN:
    return Key_DownArrow;
  case 256 + GLUT_KEY_PAGE_UP:
    return Key_PageUp;
  case 256 + GLUT_KEY_PAGE_DOWN:
    return Key_PageDown;
  case 256 + GLUT_KEY_HOME:
    return Key_Home;
  case 256 + GLUT_KEY_END:
    return Key_End;
  case 256 + GLUT_KEY_INSERT:
    return Key_Insert;
  case 127:
    return Key_Delete;
  case 8:
    return Key_Backspace;
  case ' ':
    return Key_Space;
  case 13:
    return Key_Enter;
  case 27:
    return Key_Escape;
  case 39:
    return Key_Apostrophe;
  case 44:
    return Key_Comma;
  case 45:
    return Key_Minus;
  case 46:
    return Key_Period;
  case 47:
    return Key_Slash;
  case 59:
    return Key_Semicolon;
  case 61:
    return Key_Equal;
  case 91:
    return Key_LeftBracket;
  case 92:
    return Key_Backslash;
  case 93:
    return Key_RightBracket;
  case 96:
    return Key_GraveAccent;
  // case 0:                         return Key_CapsLock;
  // case 0:                         return Key_ScrollLock;
  case 256 + 0x006D:
    return Key_NumLock;
  // case 0:                         return Key_PrintScreen;
  // case 0:                         return Key_Pause;
  // case '0':                       return Key_Keypad0;
  // case '1':                       return Key_Keypad1;
  // case '2':                       return Key_Keypad2;
  // case '3':                       return Key_Keypad3;
  // case '4':                       return Key_Keypad4;
  // case '5':                       return Key_Keypad5;
  // case '6':                       return Key_Keypad6;
  // case '7':                       return Key_Keypad7;
  // case '8':                       return Key_Keypad8;
  // case '9':                       return Key_Keypad9;
  // case 46:                        return Key_KeypadDecimal;
  // case 47:                        return Key_KeypadDivide;
  case 42:
    return Key_KeypadMultiply;
  // case 45:                        return Key_KeypadSubtract;
  case 43:
    return Key_KeypadAdd;
  // case 13:                        return Key_KeypadEnter;
  // case 0:                         return Key_KeypadEqual;
  case 256 + 0x0072:
    return Key_LeftCtrl;
  case 256 + 0x0070:
    return Key_LeftShift;
  case 256 + 0x0074:
    return Key_LeftAlt;
  // case 0:                         return Key_LeftSuper;
  case 256 + 0x0073:
    return Key_RightCtrl;
  case 256 + 0x0071:
    return Key_RightShift;
  case 256 + 0x0075:
    return Key_RightAlt;
  // case 0:                         return Key_RightSuper;
  // case 0:                         return Key_Menu;
  case '0':
    return Key_0;
  case '1':
    return Key_1;
  case '2':
    return Key_2;
  case '3':
    return Key_3;
  case '4':
    return Key_4;
  case '5':
    return Key_5;
  case '6':
    return Key_6;
  case '7':
    return Key_7;
  case '8':
    return Key_8;
  case '9':
    return Key_9;
  case 'A':
  case 'a':
    return Key_A;
  case 'B':
  case 'b':
    return Key_B;
  case 'C':
  case 'c':
    return Key_C;
  case 'D':
  case 'd':
    return Key_D;
  case 'E':
  case 'e':
    return Key_E;
  case 'F':
  case 'f':
    return Key_F;
  case 'G':
  case 'g':
    return Key_G;
  case 'H':
  case 'h':
    return Key_H;
  case 'I':
  case 'i':
    return Key_I;
  case 'J':
  case 'j':
    return Key_J;
  case 'K':
  case 'k':
    return Key_K;
  case 'L':
  case 'l':
    return Key_L;
  case 'M':
  case 'm':
    return Key_M;
  case 'N':
  case 'n':
    return Key_N;
  case 'O':
  case 'o':
    return Key_O;
  case 'P':
  case 'p':
    return Key_P;
  case 'Q':
  case 'q':
    return Key_Q;
  case 'R':
  case 'r':
    return Key_R;
  case 'S':
  case 's':
    return Key_S;
  case 'T':
  case 't':
    return Key_T;
  case 'U':
  case 'u':
    return Key_U;
  case 'V':
  case 'v':
    return Key_V;
  case 'W':
  case 'w':
    return Key_W;
  case 'X':
  case 'x':
    return Key_X;
  case 'Y':
  case 'y':
    return Key_Y;
  case 'Z':
  case 'z':
    return Key_Z;
  case 256 + GLUT_KEY_F1:
    return Key_F1;
  case 256 + GLUT_KEY_F2:
    return Key_F2;
  case 256 + GLUT_KEY_F3:
    return Key_F3;
  case 256 + GLUT_KEY_F4:
    return Key_F4;
  case 256 + GLUT_KEY_F5:
    return Key_F5;
  case 256 + GLUT_KEY_F6:
    return Key_F6;
  case 256 + GLUT_KEY_F7:
    return Key_F7;
  case 256 + GLUT_KEY_F8:
    return Key_F8;
  case 256 + GLUT_KEY_F9:
    return Key_F9;
  case 256 + GLUT_KEY_F10:
    return Key_F10;
  case 256 + GLUT_KEY_F11:
    return Key_F11;
  case 256 + GLUT_KEY_F12:
    return Key_F12;
  default:
    return Key_None;
  }
}

bool GLUT_Init() {
  IO &io = Gui::GetIO();

#ifdef FREEGLUT
  io.BackendPlatformName = "glut (freeglut)";
#else
  io.BackendPlatformName = "glut";
#endif
  g_Time = 0;

  return true;
}

void GLUT_InstallFuncs() {
  glutReshapeFunc(GLUT_ReshapeFunc);
  glutMotionFunc(GLUT_MotionFunc);
  glutPassiveMotionFunc(GLUT_MotionFunc);
  glutMouseFunc(GLUT_MouseFunc);
#ifdef __FREEGLUT_EXT_H__
  glutMouseWheelFunc(GLUT_MouseWheelFunc);
#endif
  glutKeyboardFunc(GLUT_KeyboardFunc);
  glutKeyboardUpFunc(GLUT_KeyboardUpFunc);
  glutSpecialFunc(GLUT_SpecialFunc);
  glutSpecialUpFunc(GLUT_SpecialUpFunc);
}

void GLUT_Shutdown() {
  IO &io = Gui::GetIO();
  io.BackendPlatformName = nullptr;
}

void GLUT_NewFrame() {
  // Setup time step
  IO &io = Gui::GetIO();
  int current_time = glutGet(GLUT_ELAPSED_TIME);
  int delta_time_ms = (current_time - g_Time);
  if (delta_time_ms <= 0)
    delta_time_ms = 1;
  io.DeltaTime = delta_time_ms / 1000.0f;
  g_Time = current_time;
}

static void GLUT_UpdateKeyModifiers() {
  IO &io = Gui::GetIO();
  int glut_key_mods = glutGetModifiers();
  io.AddKeyEvent(Mod_Ctrl, (glut_key_mods & GLUT_ACTIVE_CTRL) != 0);
  io.AddKeyEvent(Mod_Shift, (glut_key_mods & GLUT_ACTIVE_SHIFT) != 0);
  io.AddKeyEvent(Mod_Alt, (glut_key_mods & GLUT_ACTIVE_ALT) != 0);
}

static void GLUT_AddKeyEvent(Key key, bool down, int native_keycode) {
  IO &io = Gui::GetIO();
  io.AddKeyEvent(key, down);
  io.SetKeyEventNativeData(key, native_keycode,
                           -1); // To support legacy indexing (<1.87 user code)
}

void GLUT_KeyboardFunc(unsigned char c, int x, int y) {
  // Send character to imgui
  // printf("char_down_func %d '%c'\n", c, c);
  IO &io = Gui::GetIO();
  if (c >= 32)
    io.AddInputCharacter((unsigned int)c);

  Key key = GLUT_KeyToKey(c);
  GLUT_AddKeyEvent(key, true, c);
  GLUT_UpdateKeyModifiers();
  (void)x;
  (void)y; // Unused
}

void GLUT_KeyboardUpFunc(unsigned char c, int x, int y) {
  // printf("char_up_func %d '%c'\n", c, c);
  Key key = GLUT_KeyToKey(c);
  GLUT_AddKeyEvent(key, false, c);
  GLUT_UpdateKeyModifiers();
  (void)x;
  (void)y; // Unused
}

void GLUT_SpecialFunc(int key, int x, int y) {
  // printf("key_down_func %d\n", key);
  Key _key = GLUT_KeyToKey(key + 256);
  GLUT_AddKeyEvent(_key, true, key + 256);
  GLUT_UpdateKeyModifiers();
  (void)x;
  (void)y; // Unused
}

void GLUT_SpecialUpFunc(int key, int x, int y) {
  // printf("key_up_func %d\n", key);
  Key _key = GLUT_KeyToKey(key + 256);
  GLUT_AddKeyEvent(_key, false, key + 256);
  GLUT_UpdateKeyModifiers();
  (void)x;
  (void)y; // Unused
}

void GLUT_MouseFunc(int glut_button, int state, int x, int y) {
  IO &io = Gui::GetIO();
  io.AddMousePosEvent((float)x, (float)y);
  int button = -1;
  if (glut_button == GLUT_LEFT_BUTTON)
    button = 0;
  if (glut_button == GLUT_RIGHT_BUTTON)
    button = 1;
  if (glut_button == GLUT_MIDDLE_BUTTON)
    button = 2;
  if (button != -1 && (state == GLUT_DOWN || state == GLUT_UP))
    io.AddMouseButtonEvent(button, state == GLUT_DOWN);
}

#ifdef __FREEGLUT_EXT_H__
void GLUT_MouseWheelFunc(int button, int dir, int x, int y) {
  IO &io = Gui::GetIO();
  io.AddMousePosEvent((float)x, (float)y);
  if (dir != 0)
    io.AddMouseWheelEvent(0.0f, dir > 0 ? 1.0f : -1.0f);
  (void)button; // Unused
}
#endif

void GLUT_ReshapeFunc(int w, int h) {
  IO &io = Gui::GetIO();
  io.DisplaySize = Vec2((float)w, (float)h);
}

void GLUT_MotionFunc(int x, int y) {
  IO &io = Gui::GetIO();
  io.AddMousePosEvent((float)x, (float)y);
}

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
