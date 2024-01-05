// Dear Gui: standalone example application for Win32 + OpenGL 3

// Learn about Dear Gui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of gui.cpp

// This is provided for completeness, however it is strongly recommended you use
// OpenGL with SDL or GLFW.

#include "gui.hpp"
#include "opengl3.hpp"
#include "win32.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <GL/GL.h>
#include <tchar.h>
#include <windows.h>

// Data stored per platform window
struct WGL_WindowData {
  HDC hDC;
};

// Data
static HGLRC g_hRC;
static WGL_WindowData g_MainWindow;
static int g_Width;
static int g_Height;

// Forward declarations of helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData *data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData *data);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Support function for multi-viewports
// Unlike most other backend combination, we need specific hooks to combine
// Win32+OpenGL. We could in theory decide to support Win32-specific code in
// OpenGL backend via e.g. an hypothetical OpenGL3_InitForRawWin32().
static void Hook_Renderer_CreateWindow(Viewport *viewport) {
  assert(viewport->RendererUserData == NULL);

  WGL_WindowData *data = NEW(WGL_WindowData);
  CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
  viewport->RendererUserData = data;
}

static void Hook_Renderer_DestroyWindow(Viewport *viewport) {
  if (viewport->RendererUserData != NULL) {
    WGL_WindowData *data = (WGL_WindowData *)viewport->RendererUserData;
    CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
    DELETE(data);
    viewport->RendererUserData = NULL;
  }
}

static void Hook_Platform_RenderWindow(Viewport *viewport, void *) {
  // Activate the platform window DC in the OpenGL rendering context
  if (WGL_WindowData *data = (WGL_WindowData *)viewport->RendererUserData)
    wglMakeCurrent(data->hDC, g_hRC);
}

static void Hook_Renderer_SwapBuffers(Viewport *viewport, void *) {
  if (WGL_WindowData *data = (WGL_WindowData *)viewport->RendererUserData)
    ::SwapBuffers(data->hDC);
}

// Main code
int main(int, char **) {
  // Create application window
  // Win32_EnableDpiAwareness();
  WNDCLASSEXW wc = {sizeof(wc),
                    CS_OWNDC,
                    WndProc,
                    0L,
                    0L,
                    GetModuleHandle(nullptr),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    L"Gui Example",
                    nullptr};
  ::RegisterClassExW(&wc);
  HWND hwnd = ::CreateWindowW(
      wc.lpszClassName, L"Dear Gui Win32+OpenGL3 Example", WS_OVERLAPPEDWINDOW,
      100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

  // Initialize OpenGL
  if (!CreateDeviceWGL(hwnd, &g_MainWindow)) {
    CleanupDeviceWGL(hwnd, &g_MainWindow);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 1;
  }
  wglMakeCurrent(g_MainWindow.hDC, g_hRC);

  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

  // Setup Dear Gui context
  CHECKVERSION();
  Gui::CreateContext();
  IO &io = Gui::GetIO();
  (void)io;
  io.ConfigFlags |= ConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.ConfigFlags |= ConfigFlags_DockingEnable;     // Enable Docking
  io.ConfigFlags |= ConfigFlags_ViewportsEnable;   // Enable Multi-Viewport /
                                                   // Platform Windows

  // Setup Dear Gui style
  Gui::StyleColorsDark();
  // Gui::StyleColorsClassic();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  Style &style = Gui::GetStyle();
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[Col_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  Win32_InitForOpenGL(hwnd);
  OpenGL3_Init();

  // Win32+GL needs specific hooks for viewport, as there are specific things
  // needed to tie Win32 and GL api.
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
    PlatformIO &platform_io = Gui::GetPlatformIO();
    ASSERT(platform_io.Renderer_CreateWindow == NULL);
    ASSERT(platform_io.Renderer_DestroyWindow == NULL);
    ASSERT(platform_io.Renderer_SwapBuffers == NULL);
    ASSERT(platform_io.Platform_RenderWindow == NULL);
    platform_io.Renderer_CreateWindow = Hook_Renderer_CreateWindow;
    platform_io.Renderer_DestroyWindow = Hook_Renderer_DestroyWindow;
    platform_io.Renderer_SwapBuffers = Hook_Renderer_SwapBuffers;
    platform_io.Platform_RenderWindow = Hook_Platform_RenderWindow;
  }

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

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  Vec4 clear_color = Vec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  bool done = false;
  while (!done) {
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32
    // backend.
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        done = true;
    }
    if (done)
      break;

    // Start the Dear Gui frame
    OpenGL3_NewFrame();
    Win32_NewFrame();
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
    glViewport(0, 0, g_Width, g_Height);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGL3_RenderDrawData(Gui::GetDrawData());

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
      Gui::UpdatePlatformWindows();
      Gui::RenderPlatformWindowsDefault();

      // Restore the OpenGL rendering context to the main window DC, since
      // platform windows might have changed it.
      wglMakeCurrent(g_MainWindow.hDC, g_hRC);
    }

    // Present
    ::SwapBuffers(g_MainWindow.hDC);
  }

  OpenGL3_Shutdown();
  Win32_Shutdown();
  Gui::DestroyContext();

  CleanupDeviceWGL(hwnd, &g_MainWindow);
  wglDeleteContext(g_hRC);
  ::DestroyWindow(hwnd);
  ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

  return 0;
}

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData *data) {
  HDC hDc = ::GetDC(hWnd);
  PIXELFORMATDESCRIPTOR pfd = {0};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  const int pf = ::ChoosePixelFormat(hDc, &pfd);
  if (pf == 0)
    return false;
  if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
    return false;
  ::ReleaseDC(hWnd, hDc);

  data->hDC = ::GetDC(hWnd);
  if (!g_hRC)
    g_hRC = wglCreateContext(data->hDC);
  return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData *data) {
  wglMakeCurrent(nullptr, nullptr);
  ::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from win32.cpp
extern API LRESULT Win32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                        LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from
// your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (Win32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED) {
      g_Width = LOWORD(lParam);
      g_Height = HIWORD(lParam);
    }
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
