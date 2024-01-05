// Gui: standalone example application for Emscripten, using GLFW +
// WebGPU (Emscripten is a C++-to-javascript compiler, used to publish
// executables for the web. See https://emscripten.org/)
#include "glfw.hpp"
#include "gui.hpp"
#include "wgpu.hpp"
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <stdio.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

// Global WebGPU required states
static WGPUDevice wgpu_device = nullptr;
static WGPUSurface wgpu_surface = nullptr;
static WGPUTextureFormat wgpu_preferred_fmt = WGPUTextureFormat_RGBA8Unorm;
static WGPUSwapChain wgpu_swap_chain = nullptr;
static int wgpu_swap_chain_width = 0;
static int wgpu_swap_chain_height = 0;

// Forward declarations
static void MainLoopStep(void *window);
static bool InitWGPU();
static void print_glfw_error(int error, const char *description);
static void print_wgpu_error(WGPUErrorType error_type, const char *message,
                             void *);

// Main code
int main(int, char **) {
  glfwSetErrorCallback(print_glfw_error);
  if (!glfwInit())
    return 1;

  // Make sure GLFW does not initialize any graphics context.
  // This needs to be done explicitly later.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Gui GLFW+WebGPU example", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }

  // Initialize the WebGPU environment
  if (!InitWGPU()) {
    if (window)
      glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }
  glfwShowWindow(window);

  // Setup Gui context
  CHECKVERSION();
  Gui::CreateContext();
  IO &io = Gui::GetIO();
  (void)io;
  io.ConfigFlags |= ConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // For an Emscripten build we are disabling file-system access, so let's not
  // attempt to do a fopen() of the gui.ini file. You may manually call
  // LoadIniSettingsFromMemory() to load settings from your own storage.
  io.IniFilename = nullptr;

  // Setup Gui style
  Gui::StyleColorsDark();
  // Gui::StyleColorsLight();

  // Setup Platform/Renderer backends
  Glfw_InitForOther(window, true);
  Glfw_InstallEmscriptenCanvasResizeCallback("#canvas");
  WGPU_Init(wgpu_device, 3, wgpu_preferred_fmt, WGPUTextureFormat_Undefined);

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
  // - Emscripten allows preloading a file or folder to be accessible at
  // runtime. See Makefile for details.
  // io.Fonts->AddFontDefault();
#ifndef DISABLE_FILE_FUNCTIONS
  // io.Fonts->AddFontFromFileTTF("fonts/segoeui.ttf", 18.0f);
  io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("fonts/ProggyTiny.ttf", 10.0f);
  // Font* font = io.Fonts->AddFontFromFileTTF("fonts/ArialUni.ttf", 18.0f,
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); ASSERT(font != nullptr);
#endif

  // This function will directly return and exit the main function.
  // Make sure that no required objects get cleaned up.
  // This way we can use the browsers 'requestAnimationFrame' to control the
  // rendering.
  emscripten_set_main_loop_arg(MainLoopStep, window, 0, false);

  return 0;
}

static bool InitWGPU() {
  wgpu_device = emscripten_webgpu_get_device();
  if (!wgpu_device)
    return false;

  wgpuDeviceSetUncapturedErrorCallback(wgpu_device, print_wgpu_error, nullptr);

  // Use C++ wrapper due to misbehavior in Emscripten.
  // Some offset computation for wgpuInstanceCreateSurface in JavaScript
  // seem to be inline with struct alignments in the C++ structure
  wgpu::SurfaceDescriptorFromCanvasHTMLSelector html_surface_desc = {};
  html_surface_desc.selector = "#canvas";

  wgpu::SurfaceDescriptor surface_desc = {};
  surface_desc.nextInChain = &html_surface_desc;

  wgpu::Instance instance = wgpuCreateInstance(nullptr);
  wgpu::Surface surface = instance.CreateSurface(&surface_desc);
  wgpu::Adapter adapter = {};
  wgpu_preferred_fmt = (WGPUTextureFormat)surface.GetPreferredFormat(adapter);
  wgpu_surface = surface.Release();

  return true;
}

static void MainLoopStep(void *window) {
  IO &io = Gui::GetIO();

  glfwPollEvents();

  int width, height;
  glfwGetFramebufferSize((GLFWwindow *)window, &width, &height);

  // React to changes in screen size
  if (width != wgpu_swap_chain_width && height != wgpu_swap_chain_height) {
    WGPU_InvalidateDeviceObjects();
    if (wgpu_swap_chain)
      wgpuSwapChainRelease(wgpu_swap_chain);
    wgpu_swap_chain_width = width;
    wgpu_swap_chain_height = height;
    WGPUSwapChainDescriptor swap_chain_desc = {};
    swap_chain_desc.usage = WGPUTextureUsage_RenderAttachment;
    swap_chain_desc.format = wgpu_preferred_fmt;
    swap_chain_desc.width = width;
    swap_chain_desc.height = height;
    swap_chain_desc.presentMode = WGPUPresentMode_Fifo;
    wgpu_swap_chain =
        wgpuDeviceCreateSwapChain(wgpu_device, wgpu_surface, &swap_chain_desc);
    WGPU_CreateDeviceObjects();
  }

  // Start the Gui frame
  WGPU_NewFrame();
  Glfw_NewFrame();
  Gui::NewFrame();

  // Our state
  // (we use static, which essentially makes the variable globals, as a
  // convenience to keep the example code easy to follow)
  static bool show_demo_window = true;
  static bool show_another_window = false;
  static Vec4 clear_color = Vec4(0.45f, 0.55f, 0.60f, 1.00f);

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

  WGPURenderPassColorAttachment color_attachments = {};
  color_attachments.loadOp = WGPULoadOp_Clear;
  color_attachments.storeOp = WGPUStoreOp_Store;
  color_attachments.clearValue = {clear_color.x * clear_color.w,
                                  clear_color.y * clear_color.w,
                                  clear_color.z * clear_color.w, clear_color.w};
  color_attachments.view = wgpuSwapChainGetCurrentTextureView(wgpu_swap_chain);
  WGPURenderPassDescriptor render_pass_desc = {};
  render_pass_desc.colorAttachmentCount = 1;
  render_pass_desc.colorAttachments = &color_attachments;
  render_pass_desc.depthStencilAttachment = nullptr;

  WGPUCommandEncoderDescriptor enc_desc = {};
  WGPUCommandEncoder encoder =
      wgpuDeviceCreateCommandEncoder(wgpu_device, &enc_desc);

  WGPURenderPassEncoder pass =
      wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
  WGPU_RenderDrawData(Gui::GetDrawData(), pass);
  wgpuRenderPassEncoderEnd(pass);

  WGPUCommandBufferDescriptor cmd_buffer_desc = {};
  WGPUCommandBuffer cmd_buffer =
      wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
  WGPUQueue queue = wgpuDeviceGetQueue(wgpu_device);
  wgpuQueueSubmit(queue, 1, &cmd_buffer);
}

static void print_glfw_error(int error, const char *description) {
  printf("GLFW Error %d: %s\n", error, description);
}

static void print_wgpu_error(WGPUErrorType error_type, const char *message,
                             void *) {
  const char *error_type_lbl = "";
  switch (error_type) {
  case WGPUErrorType_Validation:
    error_type_lbl = "Validation";
    break;
  case WGPUErrorType_OutOfMemory:
    error_type_lbl = "Out of memory";
    break;
  case WGPUErrorType_Unknown:
    error_type_lbl = "Unknown";
    break;
  case WGPUErrorType_DeviceLost:
    error_type_lbl = "Device lost";
    break;
  default:
    error_type_lbl = "Unknown";
  }
  printf("%s error: %s\n", error_type_lbl, message);
}
