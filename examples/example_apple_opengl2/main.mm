// Dear Gui: standalone example application for OSX + OpenGL2, using legacy
// fixed pipeline

// Learn about Dear Gui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of gui.cpp

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#include "gui.hpp"
#include "opengl2.hpp"
#include "osx.hpp"

//-----------------------------------------------------------------------------------
// AppView
//-----------------------------------------------------------------------------------

@interface AppView : NSOpenGLView {
  NSTimer *animationTimer;
}
@end

@implementation AppView

- (void)prepareOpenGL {
  [super prepareOpenGL];

#ifndef DEBUG
  GLint swapInterval = 1;
  [[self openGLContext] setValues:&swapInterval
                     forParameter:NSOpenGLCPSwapInterval];
  if (swapInterval == 0)
    NSLog(@"Error: Cannot set swap interval.");
#endif
}

- (void)initialize {
  // Setup Dear Gui context
  // FIXME: This example doesn't have proper cleanup...
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
  // Gui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  Style &style = Gui::GetStyle();
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[Col_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  OSX_Init(self);
  OpenGL2_Init();

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
}

- (void)updateAndDrawDemoView {
  // Start the Dear Gui frame
  IO &io = Gui::GetIO();
  OpenGL2_NewFrame();
  OSX_NewFrame(self);
  Gui::NewFrame();

  // Our state (make them static = more or less global) as a convenience to keep
  // the example terse.
  static bool show_demo_window = true;
  static bool show_another_window = false;
  static Vec4 clear_color = Vec4(0.45f, 0.55f, 0.60f, 1.00f);

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
  DrawData *draw_data = Gui::GetDrawData();

  [[self openGLContext] makeCurrentContext];
  GLsizei width =
      (GLsizei)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  GLsizei height =
      (GLsizei)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  glViewport(0, 0, width, height);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);

  OpenGL2_RenderDrawData(draw_data);

  // Update and Render additional Platform Windows
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
    Gui::UpdatePlatformWindows();
    Gui::RenderPlatformWindowsDefault();
  }

  // Present
  [[self openGLContext] flushBuffer];

  if (!animationTimer)
    animationTimer =
        [NSTimer scheduledTimerWithTimeInterval:0.017
                                         target:self
                                       selector:@selector(animationTimerFired:)
                                       userInfo:nil
                                        repeats:YES];
}

- (void)reshape {
  [super reshape];
  [[self openGLContext] update];
  [self updateAndDrawDemoView];
}
- (void)drawRect:(NSRect)bounds {
  [self updateAndDrawDemoView];
}
- (void)animationTimerFired:(NSTimer *)timer {
  [self setNeedsDisplay:YES];
}
- (void)dealloc {
  animationTimer = nil;
}

@end

//-----------------------------------------------------------------------------------
// AppDelegate
//-----------------------------------------------------------------------------------

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic, readonly) NSWindow *window;
@end

@implementation AppDelegate
@synthesize window = _window;

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)theApplication {
  return YES;
}

- (NSWindow *)window {
  if (_window != nil)
    return (_window);

  NSRect viewRect = NSMakeRect(100.0, 100.0, 100.0 + 1280.0, 100 + 720.0);

  _window = [[NSWindow alloc]
      initWithContentRect:viewRect
                styleMask:NSWindowStyleMaskTitled |
                          NSWindowStyleMaskMiniaturizable |
                          NSWindowStyleMaskResizable | NSWindowStyleMaskClosable
                  backing:NSBackingStoreBuffered
                    defer:YES];
  [_window setTitle:@"Dear Gui OSX+OpenGL2 Example"];
  [_window setAcceptsMouseMovedEvents:YES];
  [_window setOpaque:YES];
  [_window makeKeyAndOrderFront:NSApp];

  return (_window);
}

- (void)setupMenu {
  NSMenu *mainMenuBar = [[NSMenu alloc] init];
  NSMenu *appMenu;
  NSMenuItem *menuItem;

  appMenu = [[NSMenu alloc] initWithTitle:@"Dear Gui OSX+OpenGL2 Example"];
  menuItem = [appMenu addItemWithTitle:@"Quit Dear Gui OSX+OpenGL2 Example"
                                action:@selector(terminate:)
                         keyEquivalent:@"q"];
  [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];

  menuItem = [[NSMenuItem alloc] init];
  [menuItem setSubmenu:appMenu];

  [mainMenuBar addItem:menuItem];

  appMenu = nil;
  [NSApp setMainMenu:mainMenuBar];
}

- (void)dealloc {
  _window = nil;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  // Make the application a foreground application (else it won't receive
  // keyboard events)
  ProcessSerialNumber psn = {0, kCurrentProcess};
  TransformProcessType(&psn, kProcessTransformToForegroundApplication);

  // Menu
  [self setupMenu];

  NSOpenGLPixelFormatAttribute attrs[] = {NSOpenGLPFADoubleBuffer,
                                          NSOpenGLPFADepthSize, 32, 0};

  NSOpenGLPixelFormat *format =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  AppView *view = [[AppView alloc] initWithFrame:self.window.frame
                                     pixelFormat:format];
  format = nil;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
  if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
    [view setWantsBestResolutionOpenGLSurface:YES];
#endif // MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
  [self.window setContentView:view];

  if ([view openGLContext] == nil)
    NSLog(@"No OpenGL Context!");

  [view initialize];
}

@end

//-----------------------------------------------------------------------------------
// Application main() function
//-----------------------------------------------------------------------------------

int main(int argc, const char *argv[]) {
  @autoreleasepool {
    NSApp = [NSApplication sharedApplication];
    AppDelegate *delegate = [[AppDelegate alloc] init];
    [[NSApplication sharedApplication] setDelegate:delegate];
    [NSApp run];
  }
  return NSApplicationMain(argc, argv);
}
