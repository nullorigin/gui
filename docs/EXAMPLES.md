_(You may browse this at https://github.com/ocornut/imgui/blob/master/docs/EXAMPLES.md or view this file with any Markdown viewer)_

## Dear Gui: Examples

**The [examples/](https://github.com/ocornut/imgui/blob/master/examples) folder example applications (standalone, ready-to-build) for variety of
platforms and graphics APIs.** They all use standard backends from the [backends/](https://github.com/ocornut/imgui/blob/master/backends) folder (see [BACKENDS.md](https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md)).

The purpose of Examples is to showcase integration with backends, let you try Dear Gui, and guide you toward
integrating Dear Gui in your own application/game/engine.
**Once Dear Gui is setup and running, run and refer to `Gui::ShowDemoWindow()` in demo.cpp for usage of the end-user API.**

You can find Windows binaries for some of those example applications at:
  https://www.dearimgui.com/binaries


### Getting Started

Integration in a typical existing application, should take <20 lines when using standard backends.

```cpp
At initialization:
  call Gui::CreateContext()
  call XXXX_Init() for each backend.

At the beginning of your frame:
  call XXXX_NewFrame() for each backend.
  call Gui::NewFrame()

At the end of your frame:
  call Gui::Render()
  call XXXX_RenderDrawData() for your Renderer backend.

At shutdown:
  call XXXX_Shutdown() for each backend.
  call Gui::DestroyContext()
```

Example (using [backends/win32.cpp](https://github.com/ocornut/imgui/blob/master/backends/win32.cpp) + [backends/dx11.cpp](https://github.com/ocornut/imgui/blob/master/backends/dx11.cpp)):

```cpp
// Create a Dear Gui context, setup some options
Gui::CreateContext();
IO& io = Gui::GetIO();
io.ConfigFlags |= ConfigFlags_NavEnableKeyboard; // Enable some options

// Initialize Platform + Renderer backends (here: using win32.cpp + dx11.cpp)
Win32_Init(my_hwnd);
DX11_Init(my_d3d_device, my_d3d_device_context);

// Application main loop
while (true)
{
    // Beginning of frame: update Renderer + Platform backend, start Dear Gui frame
    DX11_NewFrame();
    Win32_NewFrame();
    Gui::NewFrame();

    // Any application code here
    Gui::Text("Hello, world!");

    // End of frame: render Dear Gui
    Gui::Render();
    DX11_RenderDrawData(Gui::GetDrawData());

    // Swap
    g_pSwapChain->Present(1, 0);
}

// Shutdown
DX11_Shutdown();
Win32_Shutdown();
Gui::DestroyContext();
```

Please read 'PROGRAMMER GUIDE' in gui.cpp for notes on how to setup Dear Gui in your codebase.
Please read the comments and instruction at the top of each file.
Please read FAQ at https://www.dearimgui.com/faq

If you are using any of the backends provided here, you can add the backends/xxxx(.cpp,.h)
files to your project and use as-in. Each xxxx.cpp file comes with its own individual
Changelog, so if you want to update them later it will be easier to catch up with what changed.


### Examples Applications

[example_allegro5/](https://github.com/ocornut/imgui/blob/master/examples/example_allegro5/) <BR>
Allegro 5 example. <BR>
= main.cpp + allegro5.cpp

[example_android_opengl3/](https://github.com/ocornut/imgui/blob/master/examples/example_android_opengl3/) <BR>
Android + OpenGL3 (ES) example. <BR>
= main.cpp + android.cpp + opengl3.cpp

[example_apple_metal/](https://github.com/ocornut/imgui/blob/master/examples/example_metal/) <BR>
OSX & iOS + Metal example. <BR>
= main.m + osx.mm + metal.mm <BR>
It is based on the "cross-platform" game template provided with Xcode as of Xcode 9.
(NB: osx.mm is currently not as feature complete as other platforms backends.
You may prefer to use the GLFW Or SDL backends, which will also support Windows and Linux.)

[example_apple_opengl2/](https://github.com/ocornut/imgui/blob/master/examples/example_apple_opengl2/) <BR>
OSX + OpenGL2 example. <BR>
= main.mm + osx.mm + opengl2.cpp <BR>
(NB: osx.mm is currently not as feature complete as other platforms backends.
 You may prefer to use the GLFW Or SDL backends, which will also support Windows and Linux.)

[example_emscripten_wgpu/](https://github.com/ocornut/imgui/blob/master/examples/example_emscripten_wgpu/) <BR>
Emcripten + GLFW + WebGPU example. <BR>
= main.cpp + glfw.cpp + wgpu.cpp
Note that the 'example_glfw_opengl3' and 'example_sdl2_opengl3' examples also supports Emscripten!

[example_glfw_metal/](https://github.com/ocornut/imgui/blob/master/examples/example_glfw_metal/) <BR>
GLFW (Mac) + Metal example. <BR>
= main.mm + glfw.cpp + metal.mm

[example_glfw_opengl2/](https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl2/) <BR>
GLFW + OpenGL2 example (legacy, fixed pipeline). <BR>
= main.cpp + glfw.cpp + opengl2.cpp <BR>
**DO NOT USE THIS IF YOUR CODE/ENGINE IS USING MODERN GL or WEBGL (SHADERS, VBO, VAO, etc.)** <BR>
This code is mostly provided as a reference to learn about Dear Gui integration, because it is shorter.
If your code is using GL3+ context or any semi modern GL calls, using this renderer is likely to
make things more complicated, will require your code to reset many GL attributes to their initial
state, and might confuse your GPU driver. One star, not recommended.

[example_glfw_opengl3/](https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/) <BR>
GLFW (Win32, Mac, Linux) + OpenGL3+/ES2/ES3 example (modern, programmable pipeline). <BR>
= main.cpp + glfw.cpp + opengl3.cpp <BR>
This uses more modern GL calls and custom shaders.<BR>
This support building with Emscripten and targetting WebGL.<BR>
Prefer using that if you are using modern GL or WebGL in your application.

[example_glfw_vulkan/](https://github.com/ocornut/imgui/blob/master/examples/example_glfw_vulkan/) <BR>
GLFW (Win32, Mac, Linux) + Vulkan example. <BR>
= main.cpp + glfw.cpp + vulkan.cpp <BR>
This is quite long and tedious, because: Vulkan.
For this example, the main.cpp file exceptionally use helpers function from vulkan.hpp/cpp.

[example_glut_opengl2/](https://github.com/ocornut/imgui/blob/master/examples/example_glut_opengl2/) <BR>
GLUT (e.g., FreeGLUT on Linux/Windows, GLUT framework on OSX) + OpenGL2 example. <BR>
= main.cpp + glut.cpp + opengl2.cpp <BR>
Note that GLUT/FreeGLUT is largely obsolete software, prefer using GLFW or SDL.

[example_null/](https://github.com/ocornut/imgui/blob/master/examples/example_null/) <BR>
Null example, compile and link imgui, create context, run headless with no inputs and no graphics output. <BR>
= main.cpp <BR>
This is used to quickly test compilation of core imgui files in as many setups as possible.
Because this application doesn't create a window nor a graphic context, there's no graphics output.

[example_sdl2_directx11/](https://github.com/ocornut/imgui/blob/master/examples/example_sdl2_directx11/) <BR>
SDL2 + DirectX11 example, Windows only. <BR>
= main.cpp + sdl2.cpp + dx11.cpp <BR>
This to demonstrate usage of DirectX with SDL2.

[example_sdl2_metal/](https://github.com/ocornut/imgui/blob/master/examples/example_sdl2_metal/) <BR>
SDL2 + Metal example, Mac only. <BR>
= main.mm + sdl2.cpp + metal.mm

[example_sdl2_opengl2/](https://github.com/ocornut/imgui/blob/master/examples/example_sdl2_opengl2/) <BR>
SDL2 (Win32, Mac, Linux etc.) + OpenGL example (legacy, fixed pipeline). <BR>
= main.cpp + sdl2.cpp + opengl2.cpp <BR>
**DO NOT USE OPENGL2 CODE IF YOUR CODE/ENGINE IS USING GL OR WEBGL (SHADERS, VBO, VAO, etc.)** <BR>
This code is mostly provided as a reference to learn about Dear Gui integration, because it is shorter.
If your code is using GL3+ context or any semi modern GL calls, using this renderer is likely to
make things more complicated, will require your code to reset many GL attributes to their initial
state, and might confuse your GPU driver. One star, not recommended.

[example_sdl2_opengl3/](https://github.com/ocornut/imgui/blob/master/examples/example_sdl2_opengl3/) <BR>
SDL2 (Win32, Mac, Linux, etc.) + OpenGL3+/ES2/ES3 example. <BR>
= main.cpp + sdl2.cpp + opengl3.cpp <BR>
This uses more modern GL calls and custom shaders. <BR>
This support building with Emscripten and targetting WebGL.<BR>
Prefer using that if you are using modern GL or WebGL in your application.

[example_sdl2_sdlrenderer2/](https://github.com/ocornut/imgui/blob/master/examples/example_sdl2_sdlrenderer2/) <BR>
SDL2 (Win32, Mac, Linux, etc.) + SDL_Renderer for SDL2 (most graphics backends are supported underneath) <BR>
= main.cpp + sdl2.cpp + sdlrenderer.cpp <BR>
This requires SDL 2.0.18+ (released November 2021) <BR>

[example_sdl2_vulkan/](https://github.com/ocornut/imgui/blob/master/examples/example_sdl2_vulkan/) <BR>
SDL2 (Win32, Mac, Linux, etc.) + Vulkan example. <BR>
= main.cpp + sdl2.cpp + vulkan.cpp <BR>
This is quite long and tedious, because: Vulkan. <BR>
For this example, the main.cpp file exceptionally use helpers function from vulkan.hpp/cpp.

[example_win32_directx9/](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx9/) <BR>
DirectX9 example, Windows only. <BR>
= main.cpp + win32.cpp + dx9.cpp

[example_win32_directx10/](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx10/) <BR>
DirectX10 example, Windows only. <BR>
= main.cpp + win32.cpp + dx10.cpp

[example_win32_directx11/](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx11/) <BR>
DirectX11 example, Windows only. <BR>
= main.cpp + win32.cpp + dx11.cpp

[example_win32_directx12/](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx12/) <BR>
DirectX12 example, Windows only. <BR>
= main.cpp + win32.cpp + dx12.cpp <BR>
This is quite long and tedious, because: DirectX12.

[example_win32_opengl3/](https://github.com/ocornut/imgui/blob/master/examples/example_win32_opengl3/) <BR>
Raw Windows + OpenGL3 + example (modern, programmable pipeline) <BR>
= main.cpp + win32.cpp + opengl3.cpp <BR>


### Miscellaneous

**Building**

Unfortunately nowadays it is still tedious to create and maintain portable build files using external
libraries (the kind we're using here to create a window and render 3D triangles) without relying on
third party software and build systems. For most examples here we choose to provide:
 - Makefiles for Linux/OSX
 - Batch files for Visual Studio 2008+
 - A .sln project file for Visual Studio 2012+
 - Xcode project files for the Apple examples
Please let us know if they don't work with your setup!
You can probably just import the xxx.cpp/.h files into your own codebase or compile those
directly with a command-line compiler.

If you are interested in using Cmake to build and links examples, see:
  https://github.com/ocornut/imgui/pull/1713 and https://github.com/ocornut/imgui/pull/3027

**About mouse cursor latency**

Dear Gui has no particular extra lag for most behaviors,
e.g. the last value passed to 'io.AddMousePosEvent()' before NewFrame() will result in windows being moved
to the right spot at the time of EndFrame()/Render(). At 60 FPS your experience should be pleasant.

However, consider that OS mouse cursors are typically drawn through a very specific hardware accelerated
path and will feel smoother than the majority of contents rendered via regular graphics API (including,
but not limited to Dear Gui windows). Because UI rendering and interaction happens on the same plane
as the mouse, that disconnect may be jarring to particularly sensitive users.
You may experiment with enabling the io.MouseDrawCursor flag to request Dear Gui to draw a mouse cursor
using the regular graphics API, to help you visualize the difference between a "hardware" cursor and a
regularly rendered software cursor.
However, rendering a mouse cursor at 60 FPS will feel sluggish so you likely won't want to enable that at
all times. It might be beneficial for the user experience to switch to a software rendered cursor _only_
when an interactive drag is in progress.

Note that some setup or GPU drivers are likely to be causing extra display lag depending on their settings.
If you feel that dragging windows feels laggy and you are not sure what the cause is: try to build a simple
drawing a flat 2D shape directly under the mouse cursor!
