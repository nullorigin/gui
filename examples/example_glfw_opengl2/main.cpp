// Gui: standalone example application for GLFW + OpenGL2, using legacy
// fixed pipeline (GLFW is a cross-platform general purpose library for handling
// windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS,
// VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See glfw.cpp for details.

#include "glfw.hpp"
#include "gui.hpp"
#include "opengl2.hpp"
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char **) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // Create window with graphics context
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Gui GLFW+OpenGL2 example", nullptr, nullptr);
  if (window == nullptr)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

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
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Gui style
  Gui::StyleColorsDark();
  // Gui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  Style &style = Gui::GetStyle();
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[Col_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  Glfw_InitForOpenGL(window, true);
  OpenGL2_Init();

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
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); ASSERT(font != nullptr);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  Vec4 clear_color = Vec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
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

    // Start the Gui frame
    OpenGL2_NewFrame();
    Glfw_NewFrame();
    Gui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in
    // Gui::ShowDemoWindow()! You can browse its code to learn more about
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
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    // If you are using this code with non-legacy OpenGL header/contexts (which
    // you should not, prefer using opengl3.cpp!!), you may need to
    // backup/reset/restore other state, e.g. for current shader using the
    // commented lines below.
    // GLint last_program;
    // glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    // glUseProgram(0);
    OpenGL2_RenderDrawData(Gui::GetDrawData());
    // glUseProgram(last_program);

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call
    //  glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      Gui::UpdatePlatformWindows();
      Gui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
  }

  // Cleanup
  OpenGL2_Shutdown();
  Glfw_Shutdown();
  Gui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
