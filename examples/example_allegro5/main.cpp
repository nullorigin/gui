// Dear Gui: standalone example application for Allegro 5

// Learn about Dear Gui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of gui.cpp

// On Windows, you can install Allegro5 using vcpkg:
//   git clone https://github.com/Microsoft/vcpkg
//   cd vcpkg
//   bootstrap - vcpkg.bat
//   vcpkg install allegro5 --triplet=x86-windows   ; for win32
//   vcpkg install allegro5 --triplet=x64-windows   ; for win64
//   vcpkg integrate install                        ; register include and libs
//   in Visual Studio

#include "allegro5.hpp"
#include "gui.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdint.h>

int main(int, char **) {
  // Setup Allegro
  al_init();
  al_install_keyboard();
  al_install_mouse();
  al_init_primitives_addon();
  al_set_new_display_flags(ALLEGRO_RESIZABLE);
  ALLEGRO_DISPLAY *display = al_create_display(1280, 720);
  al_set_window_title(display, "Dear Gui Allegro 5 example");
  ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
  al_register_event_source(queue, al_get_display_event_source(display));
  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_mouse_event_source());

  // Setup Dear Gui context
  CHECKVERSION();
  Gui::CreateContext();
  IO &io = Gui::GetIO();
  (void)io;
  io.ConfigFlags |= ConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ConfigFlags_DockingEnable;     // Enable Docking

  // Setup Dear Gui style
  Gui::StyleColorsDark();
  // Gui::StyleColorsLight();

  // Setup Platform/Renderer backends
  Allegro5_Init(display);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use Gui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the Font* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling FontAtlas::Build()/GetTexDataAsXXXX(), which
  // XXXX_NewFrame below will call.
  // - Use '#define ENABLE_FREETYPE' in your imconfig file to use Freetype for
  // higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // Font* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); ASSERT(font != nullptr);

  bool show_demo_window = true;
  bool show_another_window = false;
  Vec4 clear_color = Vec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  bool running = true;
  while (running) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application, or clear/overwrite your copy of the
    // keyboard data. Generally you may always pass all inputs to dear imgui,
    // and hide them from your application based on those two flags.
    ALLEGRO_EVENT ev;
    while (al_get_next_event(queue, &ev)) {
      Allegro5_ProcessEvent(&ev);
      if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        running = false;
      if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
        Allegro5_InvalidateDeviceObjects();
        al_acknowledge_resize(display);
        Allegro5_CreateDeviceObjects();
      }
    }

    // Start the Dear Gui frame
    Allegro5_NewFrame();
    Gui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in
    // Gui::ShowDemoWindow()! You can browse its code to learn more about Dear
    // Gui!).
    if (show_demo_window)
      Gui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      Gui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                   // and append into it.

      Gui::Text("This is some useful text."); // Display some text (you can
                                              // use a format strings too)
      Gui::Checkbox(
          "Demo Window",
          &show_demo_window); // Edit bools storing our window open/close state
      Gui::Checkbox("Another Window", &show_another_window);

      Gui::SliderFloat("float", &f, 0.0f,
                       1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
      Gui::ColorEdit3(
          "clear color",
          (float *)&clear_color); // Edit 3 floats representing a color

      if (Gui::Button("Button")) // Buttons return true when clicked (most
                                 // widgets return true when edited/activated)
        counter++;
      Gui::SameLine();
      Gui::Text("counter = %d", counter);

      Gui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);
      Gui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
      Gui::Begin(
          "Another Window",
          &show_another_window); // Pass a pointer to our bool variable (the
                                 // window will have a closing button that will
                                 // clear the bool when clicked)
      Gui::Text("Hello from another window!");
      if (Gui::Button("Close Me"))
        show_another_window = false;
      Gui::End();
    }

    // Rendering
    Gui::Render();
    al_clear_to_color(al_map_rgba_f(
        clear_color.x * clear_color.w, clear_color.y * clear_color.w,
        clear_color.z * clear_color.w, clear_color.w));
    Allegro5_RenderDrawData(Gui::GetDrawData());
    al_flip_display();
  }

  // Cleanup
  Allegro5_Shutdown();
  Gui::DestroyContext();
  al_destroy_event_queue(queue);
  al_destroy_display(display);

  return 0;
}
