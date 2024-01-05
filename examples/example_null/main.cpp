// gui: "null" example application
// (compile and link imgui, create context, run headless with NO INPUTS, NO
// GRAPHICS OUTPUT) This is useful to test building, but you cannot interact
// with anything here!
#include "gui.hpp"
#include <stdio.h>

int main(int, char **) {
  CHECKVERSION();
  Gui::CreateContext();
  IO &io = Gui::GetIO();

  // Build atlas
  unsigned char *tex_pixels = nullptr;
  int tex_w, tex_h;
  io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

  for (int n = 0; n < 20; n++) {
    printf("NewFrame() %d\n", n);
    io.DisplaySize = Vec2(1920, 1080);
    io.DeltaTime = 1.0f / 60.0f;
    Gui::NewFrame();

    static float f = 0.0f;
    Gui::Text("Hello, world!");
    Gui::SliderFloat("float", &f, 0.0f, 1.0f);
    Gui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / io.Framerate, io.Framerate);
    Gui::ShowDemoWindow(nullptr);

    Gui::Render();
  }

  printf("DestroyContext()\n");
  Gui::DestroyContext();
  return 0;
}
