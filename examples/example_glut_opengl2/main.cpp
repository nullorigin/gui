// Gui: standalone example application for GLUT/FreeGLUT + OpenGL2, using
// legacy fixed pipeline

// !!! GLUT/FreeGLUT IS OBSOLETE PREHISTORIC SOFTWARE. Using GLUT is not
// recommended unless you really miss the 90's. !!!
// !!! If someone or something is teaching you GLUT today, you are being abused.
// Please show some resistance. !!!
// !!! Nowadays, prefer using GLFW or SDL instead!

// On Windows, you can install Freeglut using vcpkg:
//   git clone https://github.com/Microsoft/vcpkg
//   cd vcpkg
//   bootstrap - vcpkg.bat
//   vcpkg install freeglut --triplet=x86-windows   ; for win32
//   vcpkg install freeglut --triplet=x64-windows   ; for win64
//   vcpkg integrate install                        ; register include and libs
//   in Visual Studio

#include "glut.hpp"
#include "gui.hpp"
#include "opengl2.hpp"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4505) // unreferenced local function has been removed
#endif

// Forward declarations of helper functions
void MainLoopStep();

// Our state
static bool show_demo_window = true;
static bool show_another_window = false;
static Vec4 clear_color = Vec4(0.45f, 0.55f, 0.60f, 1.00f);

int main(int argc, char **argv) {
  // Create GLUT window
  glutInit(&argc, argv);
#ifdef __FREEGLUT_EXT_H__
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
  glutInitWindowSize(1280, 720);
  glutCreateWindow("Gui GLUT+OpenGL2 Example");

  // Setup GLUT display function
  // We will also call GLUT_InstallFuncs() to get all the other
  // functions installed for us, otherwise it is possible to install our own
  // functions and call the glut.hpp functions ourselves.
  glutDisplayFunc(MainLoopStep);

  // Setup Gui context
  CHECKVERSION();
  Gui::CreateContext();
  IO &io = Gui::GetIO();
  (void)io;
  io.ConfigFlags |= ConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ConfigFlags_DockingEnable;     // Enable Docking

  // Setup Gui style
  Gui::StyleColorsDark();
  // Gui::StyleColorsLight();

  // Setup Platform/Renderer backends
  // FIXME: Consider reworking this example to install our own GLUT funcs +
  // forward calls GLUT_XXX ones, instead of using
  // GLUT_InstallFuncs().
  GLUT_Init();
  OpenGL2_Init();

  // Install GLUT handlers (glutReshapeFunc(), glutMotionFunc(),
  // glutPassiveMotionFunc(), glutMouseFunc(), glutKeyboardFunc() etc.) You can
  // read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear
  // imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
  // your main application, or clear/overwrite your copy of the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data
  // to your main application, or clear/overwrite your copy of the keyboard
  // data. Generally you may always pass all inputs to gui, and hide them
  // from your application based on those two flags.
  GLUT_InstallFuncs();

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

  // Main loop
  glutMainLoop();

  // Cleanup
  OpenGL2_Shutdown();
  GLUT_Shutdown();
  Gui::DestroyContext();

  return 0;
}

void MainLoopStep() {
  // Start the Gui frame
  OpenGL2_NewFrame();
  GLUT_NewFrame();
  Gui::NewFrame();
  IO &io = Gui::GetIO();

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

    Gui::Begin("Hello, world!"); // Create a window called "Hello, world!" and
                                 // append into it.

    Gui::Text("This is some useful text."); // Display some text (you can use
                                            // a format strings too)
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
    Gui::Begin("Another Window",
               &show_another_window); // Pass a pointer to our bool variable
                                      // (the window will have a closing button
                                      // that will clear the bool when clicked)
    Gui::Text("Hello from another window!");
    if (Gui::Button("Close Me"))
      show_another_window = false;
    Gui::End();
  }

  // Rendering
  Gui::Render();
  glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  // glUseProgram(0); // You may want this if using this code in an OpenGL 3+
  // context where shaders may be bound, but prefer using the GL3+ code.
  OpenGL2_RenderDrawData(Gui::GetDrawData());

  glutSwapBuffers();
  glutPostRedisplay();
}
