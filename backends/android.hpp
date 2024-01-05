// dear imgui: Platform Binding for Android native app
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
//  - FIXME: Unicode character inputs needs to be passed by Dear Gui by the
//  application (see examples/ and issue #3446)

// You can use unmodified * files in your project. See examples/
// folder for examples of using this. Prefer including the entire imgui/
// repository into your project (either as a copy or as a submodule), and only
// build the backends you need. Learn about Dear Gui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of gui.cpp

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct ANativeWindow;
struct AInputEvent;

API bool Android_Init(ANativeWindow *window);
API int32_t Android_HandleInputEvent(const AInputEvent *input_event);
API void Android_Shutdown();
API void Android_NewFrame();

#endif // #ifndef DISABLE
