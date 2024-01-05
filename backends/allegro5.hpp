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
//  [ ] Renderer: The renderer is suboptimal as we need to unindex our buffers
//  and convert vertices manually. [ ] Platform: Missing gamepad support.

#pragma once
#include "../gui.hpp" // API
#ifndef DISABLE

struct ALLEGRO_DISPLAY;
union ALLEGRO_EVENT;

API bool Allegro5_Init(ALLEGRO_DISPLAY *display);
API void Allegro5_Shutdown();
API void Allegro5_NewFrame();
API void Allegro5_RenderDrawData(DrawData *draw_data);
API bool Allegro5_ProcessEvent(ALLEGRO_EVENT *event);

// Use if you want to reset your rendering device without losing Gui
// state.
API bool Allegro5_CreateDeviceObjects();
API void Allegro5_InvalidateDeviceObjects();

#endif // #ifndef DISABLE
