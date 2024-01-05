// dear imgui: Platform Backend for GLUT/FreeGLUT
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
#ifndef DISABLE
#include "../gui.hpp" // API

API bool GLUT_Init();
API void GLUT_InstallFuncs();
API void GLUT_Shutdown();
API void GLUT_NewFrame();

// You can call GLUT_InstallFuncs() to get all those functions
// installed automatically, or call them yourself from your own GLUT handlers.
// We are using the same weird names as GLUT for consistency..
//------------------------------------ GLUT name
//---------------------------------------------- Decent Name ---------
API void GLUT_ReshapeFunc(int w, int h); // ~ ResizeFunc
API void GLUT_MotionFunc(int x, int y);  // ~ MouseMoveFunc
API void GLUT_MouseFunc(int button, int state, int x,
                        int y); // ~ MouseButtonFunc
API void GLUT_MouseWheelFunc(int button, int dir, int x,
                             int y); // ~ MouseWheelFunc
API void GLUT_KeyboardFunc(unsigned char c, int x,
                           int y); // ~ CharPressedFunc
API void GLUT_KeyboardUpFunc(unsigned char c, int x,
                             int y); // ~ CharReleasedFunc
API void GLUT_SpecialFunc(int key, int x,
                          int y); // ~ KeyPressedFunc
API void GLUT_SpecialUpFunc(int key, int x,
                            int y); // ~ KeyReleasedFunc

#endif // #ifndef DISABLE
