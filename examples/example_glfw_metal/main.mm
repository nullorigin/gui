// Gui: standalone example application for GLFW + Metal, using
// programmable pipeline (GLFW is a cross-platform general purpose library for
// handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation,
// etc.)
#include "glfw.hpp"
#include "gui.hpp"
#include "metal.hpp"
#include <stdio.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char **) {
  // Setup Gui context
  CHECKVERSION();
  Gui::CreateContext();
  IO &io = Gui::GetIO();
  (void)io;
  io.ConfigFlags |= ConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.ConfigFlags |= ConfigFlags_DockingEnable;     // Enable Docking
  io.ConfigFlags |= ConfigFlags_ViewportsEnable;   // Enable Multi-Viewport /
                                                   // Platform Windows

  // Setup style
  Gui::StyleColorsDark();
  // Gui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  Style &style = Gui::GetStyle();
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[Col_WindowBg].w = 1.0f;
  }

  // Load Fonts
  // - If no fonts are loaded, gui will use the default font. You can
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
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); assert(font != nullptr);

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // Create window with graphics context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Gui GLFW+Metal example", nullptr, nullptr);
  if (window == nullptr)
    return 1;

  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  id<MTLCommandQueue> commandQueue = [device newCommandQueue];

  // Setup Platform/Renderer backends
  Glfw_InitForOther(window, true);
  Metal_Init(device);

  NSWindow *nswin = glfwGetCocoaWindow(window);
  CAMetalLayer *layer = [CAMetalLayer layer];
  layer.device = device;
  layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  nswin.contentView.layer = layer;
  nswin.contentView.wantsLayer = YES;

  MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor new];

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    @autoreleasepool {
      // Poll and handle events (inputs, window resize, etc.)
      // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
      // tell if gui wants to use your inputs.
      // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
      // your main application, or clear/overwrite your copy of the mouse data.
      // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
      // data to your main application, or clear/overwrite your copy of the
      // keyboard data. Generally you may always pass all inputs to gui,
      // and hide them from your application based on those two flags.
      glfwPollEvents();

      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      layer.drawableSize = CGSizeMake(width, height);
      id<CAMetalDrawable> drawable = [layer nextDrawable];

      id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
      renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(
          clear_color[0] * clear_color[3], clear_color[1] * clear_color[3],
          clear_color[2] * clear_color[3], clear_color[3]);
      renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
      renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
      renderPassDescriptor.colorAttachments[0].storeAction =
          MTLStoreActionStore;
      id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer
          renderCommandEncoderWithDescriptor:renderPassDescriptor];
      [renderEncoder pushDebugGroup:@"Gui demo"];

      // Start the Gui frame
      Metal_NewFrame(renderPassDescriptor);
      Glfw_NewFrame();
      Gui::NewFrame();

      // 1. Show the big demo window (Most of the sample code is in
      // Gui::ShowDemoWindow()! You can browse its code to learn more about
      // Gui!).
      if (show_demo_window)
        Gui::ShowDemoWindow(&show_demo_window);

      // 2. Show a simple window that we create ourselves. We use a Begin/End
      // pair to create a named window.
      {
        static float f = 0.0f;
        static int counter = 0;

        Gui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                     // and append into it.

        Gui::Text("This is some useful text."); // Display some text (you can
                                                // use a format strings too)
        Gui::Checkbox("Demo Window",
                      &show_demo_window); // Edit bools storing our window
                                          // open/close state
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
                                   // window will have a closing button that
                                   // will clear the bool when clicked)
        Gui::Text("Hello from another window!");
        if (Gui::Button("Close Me"))
          show_another_window = false;
        Gui::End();
      }

      // Rendering
      Gui::Render();
      Metal_RenderDrawData(Gui::GetDrawData(), commandBuffer, renderEncoder);

      // Update and Render additional Platform Windows
      if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
        Gui::UpdatePlatformWindows();
        Gui::RenderPlatformWindowsDefault();
      }

      [renderEncoder popDebugGroup];
      [renderEncoder endEncoding];

      [commandBuffer presentDrawable:drawable];
      [commandBuffer commit];
    }
  }

  // Cleanup
  Metal_Shutdown();
  Glfw_Shutdown();
  Gui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
