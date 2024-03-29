// gui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows,
// inputs, OpenGL/Vulkan graphics context creation, etc.) (Requires: GLFW 3.1+.
// Prefer GLFW 3.3+ for full feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen (Windows
//  only). [X] Platform: Keyboard support. Since 1.87 we are using the
//  io.AddKeyEvent() function. Pass Key values to all key functions e.g.
//  Gui::IsKeyPressed(Key_Space). [Legacy GLFW_KEY_* values will also be
//  supported unless DISABLE_OBSOLETE_KEYIO is set] [X] Platform: Gamepad
//  support. Enable with 'io.ConfigFlags |= ConfigFlags_NavEnableGamepad'.
//  [x] Platform: Mouse cursor shape and visibility. Disable with
//  'io.ConfigFlags |= ConfigFlags_NoMouseCursorChange' (note: the resizing
//  cursors requires GLFW 3.4+). [X] Platform: Multi-viewport support (multiple
//  windows). Enable with 'io.ConfigFlags |= ConfigFlags_ViewportsEnable'.

// Issues:
//  [ ] Platform: Multi-viewport support: ParentViewportID not honored, and so
//  io.ConfigViewportsNoDefaultParent has no effect (minor).

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct GLFWwindow;
struct GLFWmonitor;

API bool Glfw_InitForOpenGL(GLFWwindow *window, bool install_callbacks);
API bool Glfw_InitForVulkan(GLFWwindow *window, bool install_callbacks);
API bool Glfw_InitForOther(GLFWwindow *window, bool install_callbacks);
API void Glfw_Shutdown();
API void Glfw_NewFrame();

// Emscripten related initialization phase methods
#ifdef __EMSCRIPTEN__
API void
Glfw_InstallEmscriptenCanvasResizeCallback(const char *canvas_selector);
#endif

// GLFW callbacks install
// - When calling Init with 'install_callbacks=true':
// Glfw_InstallCallbacks() is called. GLFW callbacks will be installed
// for you. They will chain-call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': GLFW callbacks won't be
// installed. You will need to call individual function yourself from your own
// GLFW callbacks.
API void Glfw_InstallCallbacks(GLFWwindow *window);
API void Glfw_RestoreCallbacks(GLFWwindow *window);

// GFLW callbacks options:
// - Set 'chain_for_all_windows=true' to enable chaining callbacks for all
// windows (including secondary viewports created by backends or by user)
API void Glfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows);

// GLFW callbacks (individual callbacks to call yourself if you didn't install
// callbacks)
API void Glfw_WindowFocusCallback(GLFWwindow *window,
                                  int focused); // Since 1.84
API void Glfw_CursorEnterCallback(GLFWwindow *window,
                                  int entered); // Since 1.84
API void Glfw_CursorPosCallback(GLFWwindow *window, double x,
                                double y); // Since 1.87
API void Glfw_MouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
API void Glfw_ScrollCallback(GLFWwindow *window, double xoffset,
                             double yoffset);
API void Glfw_KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
API void Glfw_CharCallback(GLFWwindow *window, unsigned int c);
API void Glfw_MonitorCallback(GLFWmonitor *monitor, int event);

#endif // #ifndef DISABLE
