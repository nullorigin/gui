// gui: Platform Backend for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan,
// Metal..)
// - Not well tested. If you want a portable application, prefer using the GLFW
// or SDL platform Backends on Mac.
// - Requires linking with the GameController framework ("-framework
// GameController").

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with
//  'io.ConfigFlags |= ConfigFlags_NoMouseCursorChange'. [X] Platform:
//  Mouse support. Can discriminate Mouse/Pen. [X] Platform: Keyboard support.
//  Since 1.87 we are using the io.AddKeyEvent() function. Pass Key values
//  to all key functions e.g. Gui::IsKeyPressed(Key_Space). [Legacy kVK_*
//  values will also be supported unless DISABLE_OBSOLETE_KEYIO is set] [X]
//  Platform: OSX clipboard is supported within core Gui (no specific
//  code in this backend). [X] Platform: Gamepad support. Enabled with
//  'io.ConfigFlags |= ConfigFlags_NavEnableGamepad'. [X] Platform: IME
//  support. [X] Platform: Multi-viewport / platform windows.

#include "../gui.hpp" // API
#ifndef DISABLE

#ifdef __OBJC__

@class NSEvent;
@class NSView;

API bool OSX_Init(NSView *_Nonnull view);
API void OSX_Shutdown();
API void OSX_NewFrame(NSView *_Nullable view);

#endif

//-----------------------------------------------------------------------------
// C++ API
//-----------------------------------------------------------------------------

#ifdef METAL_CPP_EXTENSIONS
// #include <AppKit/AppKit.hpp>
#ifndef __OBJC__

API bool OSX_Init(void *_Nonnull view);
API void OSX_Shutdown();
API void OSX_NewFrame(void *_Nullable view);

#endif
#endif

#endif // #ifndef DISABLE
