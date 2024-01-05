// [SECTION] Forward Declarations
// [SECTION] Helpers
// [SECTION] Demo Window / ShowDemoWindow()
// - ShowDemoWindow()
// - sub section: ShowDemoWindowWidgets()
// - sub section: ShowDemoWindowLayout()
// - sub section: ShowDemoWindowPopups()
// - sub section: ShowDemoWindowTables()
// - sub section: ShowDemoWindowInputs()
// [SECTION] About Window / ShowAboutWindow()
// [SECTION] Style Editor / ShowStyleEditor()
// [SECTION] User Guide / ShowUserGuide()
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
// [SECTION] Example App: Manipulating window titles /
ShowExampleAppWindowTitles()
    // [SECTION] Example App: Custom Rendering using DrawList API /
    ShowExampleAppCustomRendering()
    // [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
    // [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()

    * /

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gui.hpp"
#ifndef DISABLE

// System includes
#include <ctype.h>  // toupper
#include <limits.h> // INT_MIN, INT_MAX
#include <math.h>   // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdint.h> // intptr_t
#include <stdio.h>  // vsnprintf, sscanf, printf
#include <stdlib.h> // NULL, malloc, free, atoi
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <inttypes.h> // PRId64/PRIu64, not avail in some MinGW headers.
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning(disable : 4127) // condition expression is constant
#pragma warning(                                                               \
    disable : 4996) // 'This function or variable may be unsafe': strcpy,
                    // strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning(                                                               \
    disable : 26451) // [Static Analyzer] Arithmetic overflow : Using operator
                     // 'xxx' on a 4 byte value and then casting the result to
                     // an 8 byte value. Cast the value to the wider type before
                     // calling operator 'xxx' to avoid overflow(io.2).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored                                               \
    "-Wunknown-warning-option" // warning: unknown warning group 'xxx' // not
                               // all warnings are known by all Clang versions
                               // and they tend to be rename-happy.. so ignoring
                               // warnings triggers new warnings on some
                               // configuration. Great!
#endif
#pragma clang diagnostic ignored                                               \
    "-Wunknown-pragmas" // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored                                               \
    "-Wold-style-cast" // warning: use of old-style cast // yes, they are more
                       // terse.
#pragma clang diagnostic ignored                                               \
    "-Wdeprecated-declarations" // warning: 'xx' is deprecated: The POSIX name
                                // for this..   // for strdup used in demo code
                                // (so user can copy & paste the code)
#pragma clang diagnostic ignored                                               \
    "-Wint-to-void-pointer-cast" // warning: cast to 'void *' from smaller
                                 // integer type
#pragma clang diagnostic ignored                                               \
    "-Wformat-security" // warning: format string is not a string literal
#pragma clang diagnostic ignored                                               \
    "-Wexit-time-destructors" // warning: declaration requires an exit-time
                              // destructor    // exit-time destruction order is
                              // undefined. if MemFree() leads to users code
                              // that has been disabled before exit it might
                              // cause problems. Gui coding style welcomes
                              // static/globals.
#pragma clang diagnostic ignored                                               \
    "-Wunused-macros" // warning: macro is not used // we define
                      // snprintf/vsnprintf on Windows so they are available,
                      // but not always used.
#pragma clang diagnostic ignored                                               \
    "-Wzero-as-null-pointer-constant" // warning: zero as null pointer constant
                                      // // some standard header variations use
                                      // #define NULL 0
#pragma clang diagnostic ignored                                               \
    "-Wdouble-promotion" // warning: implicit conversion from 'float' to
                         // 'double' when passing argument to function  // using
                         // printf() is a misery with this as C++ va_arg
                         // ellipsis changes float to double.
#pragma clang diagnostic ignored                                               \
    "-Wreserved-id-macro" // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored                                               \
    "-Wimplicit-int-float-conversion" // warning: implicit conversion from 'xxx'
                                      // to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored                                                 \
    "-Wpragmas" // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored                                                 \
    "-Wint-to-pointer-cast" // warning: cast to pointer from integer of
                            // different size
#pragma GCC diagnostic ignored                                                 \
    "-Wformat-security" // warning: format string is not a string literal
                        // (potentially insecure)
#pragma GCC diagnostic ignored                                                 \
    "-Wdouble-promotion" // warning: implicit conversion from 'float' to
                         // 'double' when passing argument to function
#pragma GCC diagnostic ignored                                                 \
    "-Wconversion" // warning: conversion to 'xxxx' from 'xxxx' may alter its
                   // value
#pragma GCC diagnostic ignored                                                 \
    "-Wmisleading-indentation" // [__GNUC__ >= 6] warning: this 'if' clause does
                               // not guard this statement      // GCC 6.0+
                               // only. See #883 on GitHub.
#endif

// Play it nice with Windows users (Update: May 2018, Notepad now supports
// Unix-style carriage returns!)
#ifdef _WIN32
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

// Helpers
#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf _vsnprintf
#endif

// Format specifiers for 64-bit values (hasn't been decently standardized before
// VS2013)
#if !defined(PRId64) && defined(_MSC_VER)
#define PRId64 "I64d"
#define PRIu64 "I64u"
#elif !defined(PRId64)
#define PRId64 "lld"
#define PRIu64 "llu"
#endif

// Helpers macros
// We normally try to not use many helpers in demo.cpp in order to make
// code easier to copy and paste, but making an exception here as those are
// largely simplifying code... In other imgui sources we can use nicer internal
// functions from internal.hpp (Min/Max) but not in the demo.
#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) >= (B)) ? (A) : (B))
#define CLAMP(V, MN, MX) ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

// Enforce cdecl calling convention for functions called by the standard
// library, in case compilation settings changed the default to e.g.
// __vectorcall
#ifndef CDECL
#ifdef _MSC_VER
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations, Helpers
//-----------------------------------------------------------------------------

#if !defined(DISABLE_DEMO_WINDOWS)

    // Forward Declarations
    static void ShowExampleAppMainMenuBar();
static void ShowExampleAppConsole(bool *p_open);
static void ShowExampleAppCustomRendering(bool *p_open);
static void ShowExampleAppDockSpace(bool *p_open);
static void ShowExampleAppDocuments(bool *p_open);
static void ShowExampleAppLog(bool *p_open);
static void ShowExampleAppLayout(bool *p_open);
static void ShowExampleAppPropertyEditor(bool *p_open);
static void ShowExampleAppSimpleOverlay(bool *p_open);
static void ShowExampleAppAutoResize(bool *p_open);
static void ShowExampleAppConstrainedResize(bool *p_open);
static void ShowExampleAppFullscreen(bool *p_open);
static void ShowExampleAppLongText(bool *p_open);
static void ShowExampleAppWindowTitles(bool *p_open);
static void ShowExampleMenuFile();

// We split the contents of the big ShowDemoWindow() function into smaller
// functions (because the link time of very large functions grow non-linearly)
static void ShowDemoWindowWidgets();
static void ShowDemoWindowLayout();
static void ShowDemoWindowPopups();
static void ShowDemoWindowTables();
static void ShowDemoWindowColumns();
static void ShowDemoWindowInputs();

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char *desc) {
  Gui::TextDisabled("(?)");
  if (Gui::BeginItemTooltip()) {
    Gui::PushTextWrapPos(Gui::GetFontSize() * 35.0f);
    Gui::TextUnformatted(desc);
    Gui::PopTextWrapPos();
    Gui::EndTooltip();
  }
}

static void ShowDockingDisabledMessage() {
  IO &io = Gui::GetIO();
  Gui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
  Gui::Text(
      "Set io.ConfigFlags |= ConfigFlags_DockingEnable in your code, or ");
  Gui::SameLine(0.0f, 0.0f);
  if (Gui::SmallButton("click here"))
    io.ConfigFlags |= ConfigFlags_DockingEnable;
}

// Helper to wire demo markers located in code to an interactive browser
typedef void (*DemoMarkerCallback)(const char *file, int line,
                                   const char *section, void *user_data);
extern DemoMarkerCallback GDemoMarkerCallback;
extern void *GDemoMarkerCallbackUserData;
DemoMarkerCallback GDemoMarkerCallback = NULL;
void *GDemoMarkerCallbackUserData = NULL;
#define DEMO_MARKER(section)                                                   \
  do {                                                                         \
    if (GDemoMarkerCallback != NULL)                                           \
      GDemoMarkerCallback(__FILE__, __LINE__, section,                         \
                          GDemoMarkerCallbackUserData);                        \
  } while (0)

//-----------------------------------------------------------------------------
// [SECTION] Demo Window / ShowDemoWindow()
//-----------------------------------------------------------------------------
// - ShowDemoWindow()
// - ShowDemoWindowWidgets()
// - ShowDemoWindowLayout()
// - ShowDemoWindowPopups()
// - ShowDemoWindowTables()
// - ShowDemoWindowColumns()
// - ShowDemoWindowInputs()
//-----------------------------------------------------------------------------

// Demonstrate most Gui features (this is big function!)
// You may execute this function to experiment with the UI and understand what
// it does. You may then search for keywords in the code when you are interested
// by a specific feature.
void Gui::ShowDemoWindow(bool *p_open) {
  // Exceptionally add an extra assert here for people confused about initial
  // Gui setup Most functions would normally just assert/crash if the
  // context is missing.
  ASSERT(Gui::GetCurrentContext() != NULL &&
         "Missing Gui context. Refer to examples app!");

  // Examples Apps (accessible from the "Examples" menu)
  static bool show_app_main_menu_bar = false;
  static bool show_app_console = false;
  static bool show_app_custom_rendering = false;
  static bool show_app_dockspace = false;
  static bool show_app_documents = false;
  static bool show_app_log = false;
  static bool show_app_layout = false;
  static bool show_app_property_editor = false;
  static bool show_app_simple_overlay = false;
  static bool show_app_auto_resize = false;
  static bool show_app_constrained_resize = false;
  static bool show_app_fullscreen = false;
  static bool show_app_long_text = false;
  static bool show_app_window_titles = false;

  if (show_app_main_menu_bar)
    ShowExampleAppMainMenuBar();
  if (show_app_dockspace)
    ShowExampleAppDockSpace(
        &show_app_dockspace); // Process the Docking app first, as explicit
                              // DockSpace() nodes needs to be submitted early
                              // (read comments near the DockSpace function)
  if (show_app_documents)
    ShowExampleAppDocuments(
        &show_app_documents); // Process the Document app next, as it may also
                              // use a DockSpace()
  if (show_app_console)
    ShowExampleAppConsole(&show_app_console);
  if (show_app_custom_rendering)
    ShowExampleAppCustomRendering(&show_app_custom_rendering);
  if (show_app_log)
    ShowExampleAppLog(&show_app_log);
  if (show_app_layout)
    ShowExampleAppLayout(&show_app_layout);
  if (show_app_property_editor)
    ShowExampleAppPropertyEditor(&show_app_property_editor);
  if (show_app_simple_overlay)
    ShowExampleAppSimpleOverlay(&show_app_simple_overlay);
  if (show_app_auto_resize)
    ShowExampleAppAutoResize(&show_app_auto_resize);
  if (show_app_constrained_resize)
    ShowExampleAppConstrainedResize(&show_app_constrained_resize);
  if (show_app_fullscreen)
    ShowExampleAppFullscreen(&show_app_fullscreen);
  if (show_app_long_text)
    ShowExampleAppLongText(&show_app_long_text);
  if (show_app_window_titles)
    ShowExampleAppWindowTitles(&show_app_window_titles);

  // Gui Tools (accessible from the "Tools" menu)
  static bool show_tool_metrics = false;
  static bool show_tool_debug_log = false;
  static bool show_tool_id_stack_tool = false;
  static bool show_tool_style_editor = false;
  static bool show_tool_about = false;

  if (show_tool_metrics)
    Gui::ShowMetricsWindow(&show_tool_metrics);
  if (show_tool_debug_log)
    Gui::ShowDebugLogWindow(&show_tool_debug_log);
  if (show_tool_id_stack_tool)
    Gui::ShowIDStackToolWindow(&show_tool_id_stack_tool);
  if (show_tool_style_editor) {
    Gui::Begin("Gui Style Editor", &show_tool_style_editor);
    Gui::ShowStyleEditor();
    Gui::End();
  }
  if (show_tool_about)
    Gui::ShowAboutWindow(&show_tool_about);

  // Demonstrate the various window flags. Typically you would just use the
  // default!
  static bool no_titlebar = false;
  static bool no_scrollbar = false;
  static bool no_menu = false;
  static bool no_move = false;
  static bool no_resize = false;
  static bool no_collapse = false;
  static bool no_close = false;
  static bool no_nav = false;
  static bool no_background = false;
  static bool no_bring_to_front = false;
  static bool no_docking = false;
  static bool unsaved_document = false;

  WindowFlags window_flags = 0;
  if (no_titlebar)
    window_flags |= WindowFlags_NoTitleBar;
  if (no_scrollbar)
    window_flags |= WindowFlags_NoScrollbar;
  if (!no_menu)
    window_flags |= WindowFlags_MenuBar;
  if (no_move)
    window_flags |= WindowFlags_NoMove;
  if (no_resize)
    window_flags |= WindowFlags_NoResize;
  if (no_collapse)
    window_flags |= WindowFlags_NoCollapse;
  if (no_nav)
    window_flags |= WindowFlags_NoNav;
  if (no_background)
    window_flags |= WindowFlags_NoBackground;
  if (no_bring_to_front)
    window_flags |= WindowFlags_NoBringToFrontOnFocus;
  if (no_docking)
    window_flags |= WindowFlags_NoDocking;
  if (unsaved_document)
    window_flags |= WindowFlags_UnsavedDocument;
  if (no_close)
    p_open = NULL; // Don't pass our bool* to Begin

  // We specify a default position/size in case there's no data in the .ini
  // file. We only do it to make the demo applications a little more welcoming,
  // but typically this isn't required.
  const Viewport *main_viewport = Gui::GetMainViewport();
  Gui::SetNextWindowPos(
      Vec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20),
      Cond_FirstUseEver);
  Gui::SetNextWindowSize(Vec2(550, 680), Cond_FirstUseEver);

  // Main body of the Demo window starts here.
  if (!Gui::Begin("Gui Demo", p_open, window_flags)) {
    // Early out if the window is collapsed, as an optimization.
    Gui::End();
    return;
  }

  // Most "big" widgets share a common width settings by default. See
  // 'Demo->Layout->Widgets Width' for details. e.g. Use 2/3 of the space for
  // widgets and 1/3 for labels (right align)
  // Gui::PushItemWidth(-Gui::GetWindowWidth() * 0.35f);
  // e.g. Leave a fixed amount of width for labels (by passing a negative
  // value), the rest goes to widgets.
  Gui::PushItemWidth(Gui::GetFontSize() * -12);

  // Menu Bar
  if (Gui::BeginMenuBar()) {
    if (Gui::BeginMenu("Menu")) {
      DEMO_MARKER("Menu/File");
      ShowExampleMenuFile();
      Gui::EndMenu();
    }
    if (Gui::BeginMenu("Examples")) {
      DEMO_MARKER("Menu/Examples");
      Gui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);

      Gui::SeparatorText("Mini apps");
      Gui::MenuItem("Console", NULL, &show_app_console);
      Gui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
      Gui::MenuItem("Dockspace", NULL, &show_app_dockspace);
      Gui::MenuItem("Documents", NULL, &show_app_documents);
      Gui::MenuItem("Log", NULL, &show_app_log);
      Gui::MenuItem("Property editor", NULL, &show_app_property_editor);
      Gui::MenuItem("Simple layout", NULL, &show_app_layout);
      Gui::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);

      Gui::SeparatorText("Concepts");
      Gui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
      Gui::MenuItem("Constrained-resizing window", NULL,
                    &show_app_constrained_resize);
      Gui::MenuItem("Fullscreen window", NULL, &show_app_fullscreen);
      Gui::MenuItem("Long text display", NULL, &show_app_long_text);
      Gui::MenuItem("Manipulating window titles", NULL,
                    &show_app_window_titles);

      Gui::EndMenu();
    }
    // if (Gui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside
    // a menu bar!
    if (Gui::BeginMenu("Tools")) {
      DEMO_MARKER("Menu/Tools");
#ifndef DISABLE_DEBUG_TOOLS
      const bool has_debug_tools = true;
#else
      const bool has_debug_tools = false;
#endif
      Gui::MenuItem("Metrics/Debugger", NULL, &show_tool_metrics,
                    has_debug_tools);
      Gui::MenuItem("Debug Log", NULL, &show_tool_debug_log, has_debug_tools);
      Gui::MenuItem("ID Stack Tool", NULL, &show_tool_id_stack_tool,
                    has_debug_tools);
      Gui::MenuItem("Style Editor", NULL, &show_tool_style_editor);
      Gui::MenuItem("About Gui", NULL, &show_tool_about);
      Gui::EndMenu();
    }
    Gui::EndMenuBar();
  }

  Gui::Text("gui says hello! (%s) (%d)", VERSION, VERSION_NUM);
  Gui::Spacing();

  DEMO_MARKER("Help");
  if (Gui::CollapsingHeader("Help")) {
    Gui::SeparatorText("ABOUT THIS DEMO:");
    Gui::BulletText(
        "Sections below are demonstrating many aspects of the library.");
    Gui::BulletText("The \"Examples\" menu above leads to more demo contents.");
    Gui::BulletText(
        "The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
        "and Metrics/Debugger (general purpose Gui debugging tool).");

    Gui::SeparatorText("PROGRAMMER GUIDE:");
    Gui::BulletText(
        "See the ShowDemoWindow() code in demo.cpp. <- you are here!");
    Gui::BulletText("See comments in gui.cpp.");
    Gui::BulletText("See example applications in the examples/ folder.");
    Gui::BulletText(
        "Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
    Gui::BulletText(
        "Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");

    Gui::SeparatorText("USER GUIDE:");
    Gui::ShowUserGuide();
  }

  DEMO_MARKER("Configuration");
  if (Gui::CollapsingHeader("Configuration")) {
    IO &io = Gui::GetIO();

    if (Gui::TreeNode("Configuration##2")) {
      Gui::SeparatorText("General");
      Gui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard", &io.ConfigFlags,
                         ConfigFlags_NavEnableKeyboard);
      Gui::SameLine();
      HelpMarker("Enable keyboard controls.");
      Gui::CheckboxFlags("io.ConfigFlags: NavEnableGamepad", &io.ConfigFlags,
                         ConfigFlags_NavEnableGamepad);
      Gui::SameLine();
      HelpMarker("Enable gamepad controls. Require backend to set "
                 "io.BackendFlags |= BackendFlags_HasGamepad.\n\nRead "
                 "instructions in gui.cpp for details.");
      Gui::CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos",
                         &io.ConfigFlags, ConfigFlags_NavEnableSetMousePos);
      Gui::SameLine();
      HelpMarker("Instruct navigation to move the mouse cursor. See comment "
                 "for ConfigFlags_NavEnableSetMousePos.");
      Gui::CheckboxFlags("io.ConfigFlags: NoMouse", &io.ConfigFlags,
                         ConfigFlags_NoMouse);
      if (io.ConfigFlags & ConfigFlags_NoMouse) {
        // The "NoMouse" option can get us stuck with a disabled mouse! Let's
        // provide an alternative way to fix it:
        if (fmodf((float)Gui::GetTime(), 0.40f) < 0.20f) {
          Gui::SameLine();
          Gui::Text("<<PRESS SPACE TO DISABLE>>");
        }
        if (Gui::IsKeyPressed(Key_Space))
          io.ConfigFlags &= ~ConfigFlags_NoMouse;
      }
      Gui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", &io.ConfigFlags,
                         ConfigFlags_NoMouseCursorChange);
      Gui::SameLine();
      HelpMarker(
          "Instruct backend to not alter mouse cursor shape and visibility.");

      Gui::CheckboxFlags("io.ConfigFlags: DockingEnable", &io.ConfigFlags,
                         ConfigFlags_DockingEnable);
      Gui::SameLine();
      if (io.ConfigDockingWithShift)
        HelpMarker(
            "Drag from window title bar or their tab to dock/undock. Hold "
            "SHIFT to enable docking.\n\nDrag from window menu button "
            "(upper-left button) to undock an entire node (all windows).");
      else
        HelpMarker(
            "Drag from window title bar or their tab to dock/undock. Hold "
            "SHIFT to disable docking.\n\nDrag from window menu button "
            "(upper-left button) to undock an entire node (all windows).");
      if (io.ConfigFlags & ConfigFlags_DockingEnable) {
        Gui::Indent();
        Gui::Checkbox("io.ConfigDockingNoSplit", &io.ConfigDockingNoSplit);
        Gui::SameLine();
        HelpMarker(
            "Simplified docking mode: disable window splitting, so docking is "
            "limited to merging multiple windows together into tab-bars.");
        Gui::Checkbox("io.ConfigDockingWithShift", &io.ConfigDockingWithShift);
        Gui::SameLine();
        HelpMarker("Enable docking when holding Shift only (allow to drop in "
                   "wider space, reduce visual noise)");
        Gui::Checkbox("io.ConfigDockingAlwaysTabBar",
                      &io.ConfigDockingAlwaysTabBar);
        Gui::SameLine();
        HelpMarker(
            "Create a docking node and tab-bar on single floating windows.");
        Gui::Checkbox("io.ConfigDockingTransparentPayload",
                      &io.ConfigDockingTransparentPayload);
        Gui::SameLine();
        HelpMarker("Make window or viewport transparent when docking and only "
                   "display docking boxes on the target viewport. Useful if "
                   "rendering of multiple viewport cannot be synced. Best used "
                   "with ConfigViewportsNoAutoMerge.");
        Gui::Unindent();
      }

      Gui::CheckboxFlags("io.ConfigFlags: ViewportsEnable", &io.ConfigFlags,
                         ConfigFlags_ViewportsEnable);
      Gui::SameLine();
      HelpMarker("[beta] Enable beta multi-viewports support. See "
                 "PlatformIO for details.");
      if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
        Gui::Indent();
        Gui::Checkbox("io.ConfigViewportsNoAutoMerge",
                      &io.ConfigViewportsNoAutoMerge);
        Gui::SameLine();
        HelpMarker("Set to make all floating imgui windows always create their "
                   "own viewport. Otherwise, they are merged into the main "
                   "host viewports when overlapping it.");
        Gui::Checkbox("io.ConfigViewportsNoTaskBarIcon",
                      &io.ConfigViewportsNoTaskBarIcon);
        Gui::SameLine();
        HelpMarker(
            "Toggling this at runtime is normally unsupported (most platform "
            "backends won't refresh the task bar icon state right away).");
        Gui::Checkbox("io.ConfigViewportsNoDecoration",
                      &io.ConfigViewportsNoDecoration);
        Gui::SameLine();
        HelpMarker(
            "Toggling this at runtime is normally unsupported (most platform "
            "backends won't refresh the decoration right away).");
        Gui::Checkbox("io.ConfigViewportsNoDefaultParent",
                      &io.ConfigViewportsNoDefaultParent);
        Gui::SameLine();
        HelpMarker(
            "Toggling this at runtime is normally unsupported (most platform "
            "backends won't refresh the parenting right away).");
        Gui::Unindent();
      }

      Gui::Checkbox("io.ConfigInputTrickleEventQueue",
                    &io.ConfigInputTrickleEventQueue);
      Gui::SameLine();
      HelpMarker(
          "Enable input queue trickling: some types of events submitted during "
          "the same frame (e.g. button down + up) will be spread over multiple "
          "frames, improving interactions with low framerates.");
      Gui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
      Gui::SameLine();
      HelpMarker(
          "Instruct Gui to render a mouse cursor itself. Note that a "
          "mouse cursor rendered via your application GPU rendering path will "
          "feel more laggy than hardware cursor, but will be more in sync with "
          "your other visuals.\n\nSome desktop applications may use both kinds "
          "of cursors (e.g. enable software cursor only when resizing/dragging "
          "something).");

      Gui::SeparatorText("Widgets");
      Gui::Checkbox("io.ConfigInputTextCursorBlink",
                    &io.ConfigInputTextCursorBlink);
      Gui::SameLine();
      HelpMarker("Enable blinking cursor (optional as some users consider it "
                 "to be distracting).");
      Gui::Checkbox("io.ConfigInputTextEnterKeepActive",
                    &io.ConfigInputTextEnterKeepActive);
      Gui::SameLine();
      HelpMarker("Pressing Enter will keep item active and select contents "
                 "(single-line only).");
      Gui::Checkbox("io.ConfigDragClickToInputText",
                    &io.ConfigDragClickToInputText);
      Gui::SameLine();
      HelpMarker("Enable turning DragXXX widgets into text input with a simple "
                 "mouse click-release (without moving).");
      Gui::Checkbox("io.ConfigWindowsResizeFromEdges",
                    &io.ConfigWindowsResizeFromEdges);
      Gui::SameLine();
      HelpMarker("Enable resizing of windows from their edges and from the "
                 "lower-left corner.\nThis requires (io.BackendFlags & "
                 "BackendFlags_HasMouseCursors) because it needs mouse "
                 "cursor feedback.");
      Gui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly",
                    &io.ConfigWindowsMoveFromTitleBarOnly);
      Gui::Checkbox("io.ConfigMacOSXBehaviors", &io.ConfigMacOSXBehaviors);
      Gui::Text("Also see Style->Rendering for rendering options.");

      Gui::SeparatorText("Debug");
      Gui::BeginDisabled();
      Gui::Checkbox("io.ConfigDebugBeginReturnValueOnce",
                    &io.ConfigDebugBeginReturnValueOnce); // .
      Gui::EndDisabled();
      Gui::SameLine();
      HelpMarker("First calls to Begin()/BeginChild() will return "
                 "false.\n\nTHIS OPTION IS DISABLED because it needs to be set "
                 "at application boot-time to make sense. Showing the disabled "
                 "option is a way to make this feature easier to discover");
      Gui::Checkbox("io.ConfigDebugBeginReturnValueLoop",
                    &io.ConfigDebugBeginReturnValueLoop);
      Gui::SameLine();
      HelpMarker("Some calls to Begin()/BeginChild() will return "
                 "false.\n\nWill cycle through window depths then repeat. "
                 "Windows should be flickering while running.");
      Gui::Checkbox("io.ConfigDebugIgnoreFocusLoss",
                    &io.ConfigDebugIgnoreFocusLoss);
      Gui::SameLine();
      HelpMarker("Option to deactivate io.AddFocusEvent(false) handling. May "
                 "facilitate interactions with a debugger when focus loss "
                 "leads to clearing inputs data.");
      Gui::Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
      Gui::SameLine();
      HelpMarker("Option to save .ini data with extra comments (particularly "
                 "helpful for Docking, but makes saving slower).");

      Gui::TreePop();
      Gui::Spacing();
    }

    DEMO_MARKER("Configuration/Backend Flags");
    if (Gui::TreeNode("Backend Flags")) {
      HelpMarker("Those flags are set by the backends (xxx files) "
                 "to specify their capabilities.\n"
                 "Here we expose them as read-only fields to avoid breaking "
                 "interactions with your backend.");

      // Make a local copy to avoid modifying actual backend flags.
      // FIXME: Maybe we need a BeginReadonly() equivalent to keep label bright?
      Gui::BeginDisabled();
      Gui::CheckboxFlags("io.BackendFlags: HasGamepad", &io.BackendFlags,
                         BackendFlags_HasGamepad);
      Gui::CheckboxFlags("io.BackendFlags: HasMouseCursors", &io.BackendFlags,
                         BackendFlags_HasMouseCursors);
      Gui::CheckboxFlags("io.BackendFlags: HasSetMousePos", &io.BackendFlags,
                         BackendFlags_HasSetMousePos);
      Gui::CheckboxFlags("io.BackendFlags: PlatformHasViewports",
                         &io.BackendFlags, BackendFlags_PlatformHasViewports);
      Gui::CheckboxFlags("io.BackendFlags: HasMouseHoveredViewport",
                         &io.BackendFlags,
                         BackendFlags_HasMouseHoveredViewport);
      Gui::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset",
                         &io.BackendFlags, BackendFlags_RendererHasVtxOffset);
      Gui::CheckboxFlags("io.BackendFlags: RendererHasViewports",
                         &io.BackendFlags, BackendFlags_RendererHasViewports);
      Gui::EndDisabled();
      Gui::TreePop();
      Gui::Spacing();
    }

    DEMO_MARKER("Configuration/Style");
    if (Gui::TreeNode("Style")) {
      HelpMarker("The same contents can be accessed in 'Tools->Style Editor' "
                 "or by calling the ShowStyleEditor() function.");
      Gui::ShowStyleEditor();
      Gui::TreePop();
      Gui::Spacing();
    }

    DEMO_MARKER("Configuration/Capture, Logging");
    if (Gui::TreeNode("Capture/Logging")) {
      HelpMarker(
          "The logging API redirects all text output so you can easily capture "
          "the content of "
          "a window or a block. Tree nodes can be automatically expanded.\n"
          "Try opening any of the contents below in this window and then click "
          "one of the \"Log To\" button.");
      Gui::LogButtons();

      HelpMarker("You can also call Gui::LogText() to output directly to the "
                 "log without a visual output.");
      if (Gui::Button("Copy \"Hello, world!\" to clipboard")) {
        Gui::LogToClipboard();
        Gui::LogText("Hello, world!");
        Gui::LogFinish();
      }
      Gui::TreePop();
    }
  }

  DEMO_MARKER("Window options");
  if (Gui::CollapsingHeader("Window options")) {
    if (Gui::BeginTable("split", 3)) {
      Gui::TableNextColumn();
      Gui::Checkbox("No titlebar", &no_titlebar);
      Gui::TableNextColumn();
      Gui::Checkbox("No scrollbar", &no_scrollbar);
      Gui::TableNextColumn();
      Gui::Checkbox("No menu", &no_menu);
      Gui::TableNextColumn();
      Gui::Checkbox("No move", &no_move);
      Gui::TableNextColumn();
      Gui::Checkbox("No resize", &no_resize);
      Gui::TableNextColumn();
      Gui::Checkbox("No collapse", &no_collapse);
      Gui::TableNextColumn();
      Gui::Checkbox("No close", &no_close);
      Gui::TableNextColumn();
      Gui::Checkbox("No nav", &no_nav);
      Gui::TableNextColumn();
      Gui::Checkbox("No background", &no_background);
      Gui::TableNextColumn();
      Gui::Checkbox("No bring to front", &no_bring_to_front);
      Gui::TableNextColumn();
      Gui::Checkbox("No docking", &no_docking);
      Gui::TableNextColumn();
      Gui::Checkbox("Unsaved document", &unsaved_document);
      Gui::EndTable();
    }
  }

  // All demo contents
  ShowDemoWindowWidgets();
  ShowDemoWindowLayout();
  ShowDemoWindowPopups();
  ShowDemoWindowTables();
  ShowDemoWindowInputs();

  // End of ShowDemoWindow()
  Gui::PopItemWidth();
  Gui::End();
}

static void ShowDemoWindowWidgets() {
  DEMO_MARKER("Widgets");
  if (!Gui::CollapsingHeader("Widgets"))
    return;

  static bool disable_all = false; // The Checkbox for that is inside the
                                   // "Disabled" section at the bottom
  if (disable_all)
    Gui::BeginDisabled();

  DEMO_MARKER("Widgets/Basic");
  if (Gui::TreeNode("Basic")) {
    Gui::SeparatorText("General");

    DEMO_MARKER("Widgets/Basic/Button");
    static int clicked = 0;
    if (Gui::Button("Button"))
      clicked++;
    if (clicked & 1) {
      Gui::SameLine();
      Gui::Text("Thanks for clicking me!");
    }

    DEMO_MARKER("Widgets/Basic/Checkbox");
    static bool check = true;
    Gui::Checkbox("checkbox", &check);

    DEMO_MARKER("Widgets/Basic/RadioButton");
    static int e = 0;
    Gui::RadioButton("radio a", &e, 0);
    Gui::SameLine();
    Gui::RadioButton("radio b", &e, 1);
    Gui::SameLine();
    Gui::RadioButton("radio c", &e, 2);

    // Color buttons, demonstrate using PushID() to add unique identifier in the
    // ID stack, and changing style.
    DEMO_MARKER("Widgets/Basic/Buttons (Colored)");
    for (int i = 0; i < 7; i++) {
      if (i > 0)
        Gui::SameLine();
      Gui::PushID(i);
      Gui::PushStyleColor(Col_Button, (Vec4)Color::HSV(i / 7.0f, 0.6f, 0.6f));
      Gui::PushStyleColor(Col_ButtonHovered,
                          (Vec4)Color::HSV(i / 7.0f, 0.7f, 0.7f));
      Gui::PushStyleColor(Col_ButtonActive,
                          (Vec4)Color::HSV(i / 7.0f, 0.8f, 0.8f));
      Gui::Button("Click");
      Gui::PopStyleColor(3);
      Gui::PopID();
    }

    // Use AlignTextToFramePadding() to align text baseline to the baseline of
    // framed widgets elements (otherwise a Text+SameLine+Button sequence will
    // have the text a little too high by default!) See 'Demo->Layout->Text
    // Baseline Alignment' for details.
    Gui::AlignTextToFramePadding();
    Gui::Text("Hold to repeat:");
    Gui::SameLine();

    // Arrow buttons with Repeater
    DEMO_MARKER("Widgets/Basic/Buttons (Repeating)");
    static int counter = 0;
    float spacing = Gui::GetStyle().ItemInnerSpacing.x;
    Gui::PushButtonRepeat(true);
    if (Gui::ArrowButton("##left", Dir_Left)) {
      counter--;
    }
    Gui::SameLine(0.0f, spacing);
    if (Gui::ArrowButton("##right", Dir_Right)) {
      counter++;
    }
    Gui::PopButtonRepeat();
    Gui::SameLine();
    Gui::Text("%d", counter);

    Gui::Button("Tooltip");
    Gui::SetItemTooltip("I am a tooltip");

    Gui::LabelText("label", "Value");

    Gui::SeparatorText("Inputs");

    {
      // To wire InputText() with std::string or any other custom string type,
      // see the "Text Input > Resize Callback" section of this demo, and the
      // misc/cpp/stdlib.h file.
      DEMO_MARKER("Widgets/Basic/InputText");
      static char str0[128] = "Hello, world!";
      Gui::InputText("input text", str0, ARRAYSIZE(str0));
      Gui::SameLine();
      HelpMarker("USER:\n"
                 "Hold SHIFT or use mouse to select text.\n"
                 "CTRL+Left/Right to word jump.\n"
                 "CTRL+A or Double-Click to select all.\n"
                 "CTRL+X,CTRL+C,CTRL+V clipboard.\n"
                 "CTRL+Z,CTRL+Y undo/redo.\n"
                 "ESCAPE to revert.\n\n"
                 "PROGRAMMER:\n"
                 "You can use the InputTextFlags_CallbackResize facility "
                 "if you need to wire InputText() "
                 "to a dynamic string type. See misc/cpp/stdlib.h for an "
                 "example (this is not demonstrated "
                 "in demo.cpp).");

      static char str1[128] = "";
      Gui::InputTextWithHint("input text (w/ hint)", "enter text here", str1,
                             ARRAYSIZE(str1));

      DEMO_MARKER("Widgets/Basic/InputInt, InputFloat");
      static int i0 = 123;
      Gui::InputInt("input int", &i0);

      static float f0 = 0.001f;
      Gui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

      static double d0 = 999999.00000001;
      Gui::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

      static float f1 = 1.e10f;
      Gui::InputFloat("input scientific", &f1, 0.0f, 0.0f, "%e");
      Gui::SameLine();
      HelpMarker("You can input value using the scientific notation,\n"
                 "  e.g. \"1e+8\" becomes \"100000000\".");

      static float vec4a[4] = {0.10f, 0.20f, 0.30f, 0.44f};
      Gui::InputFloat3("input float3", vec4a);
    }

    Gui::SeparatorText("Drags");

    {
      DEMO_MARKER("Widgets/Basic/DragInt, DragFloat");
      static int i1 = 50, i2 = 42;
      Gui::DragInt("drag int", &i1, 1);
      Gui::SameLine();
      HelpMarker("Click and drag to edit value.\n"
                 "Hold SHIFT/ALT for faster/slower edit.\n"
                 "Double-click or CTRL+click to input value.");

      Gui::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%",
                   SliderFlags_AlwaysClamp);

      static float f1 = 1.00f, f2 = 0.0067f;
      Gui::DragFloat("drag float", &f1, 0.005f);
      Gui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
    }

    Gui::SeparatorText("Sliders");

    {
      DEMO_MARKER("Widgets/Basic/SliderInt, SliderFloat");
      static int i1 = 0;
      Gui::SliderInt("slider int", &i1, -1, 3);
      Gui::SameLine();
      HelpMarker("CTRL+click to input value.");

      static float f1 = 0.123f, f2 = 0.0f;
      Gui::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
      Gui::SliderFloat("slider float (log)", &f2, -10.0f, 10.0f, "%.4f",
                       SliderFlags_Logarithmic);

      DEMO_MARKER("Widgets/Basic/SliderAngle");
      static float angle = 0.0f;
      Gui::SliderAngle("slider angle", &angle);

      // Using the format string to display a name instead of an integer.
      // Here we completely omit '%d' from the format string, so it'll only
      // display a name. This technique can also be used with DragInt().
      DEMO_MARKER("Widgets/Basic/Slider (enum)");
      enum Element {
        Element_Fire,
        Element_Earth,
        Element_Air,
        Element_Water,
        Element_COUNT
      };
      static int elem = Element_Fire;
      const char *elems_names[Element_COUNT] = {"Fire", "Earth", "Air",
                                                "Water"};
      const char *elem_name =
          (elem >= 0 && elem < Element_COUNT) ? elems_names[elem] : "Unknown";
      Gui::SliderInt("slider enum", &elem, 0, Element_COUNT - 1,
                     elem_name); // Use SliderFlags_NoInput flag to
                                 // disable CTRL+Click here.
      Gui::SameLine();
      HelpMarker("Using the format string parameter to display a name instead "
                 "of the underlying integer.");
    }

    Gui::SeparatorText("Selectors/Pickers");

    {
      DEMO_MARKER("Widgets/Basic/ColorEdit3, ColorEdit4");
      static float col1[3] = {1.0f, 0.0f, 0.2f};
      static float col2[4] = {0.4f, 0.7f, 0.0f, 0.5f};
      Gui::ColorEdit3("color 1", col1);
      Gui::SameLine();
      HelpMarker("Click on the color square to open a color picker.\n"
                 "Click and hold to use drag and drop.\n"
                 "Right-click on the color square to show options.\n"
                 "CTRL+click on individual component to input value.\n");

      Gui::ColorEdit4("color 2", col2);
    }

    {
      // Using the _simplified_ one-liner Combo() api here
      // See "Combo" section for examples of how to use the more flexible
      // BeginCombo()/EndCombo() api.
      DEMO_MARKER("Widgets/Basic/Combo");
      const char *items[] = {"AAAA",    "BBBB", "CCCC",   "DDDD",
                             "EEEE",    "FFFF", "GGGG",   "HHHH",
                             "IIIIIII", "JJJJ", "KKKKKKK"};
      static int item_current = 0;
      Gui::Combo("combo", &item_current, items, ARRAYSIZE(items));
      Gui::SameLine();
      HelpMarker("Using the simplified one-liner Combo API here.\nRefer to the "
                 "\"Combo\" section below for an explanation of how to use the "
                 "more flexible and general BeginCombo/EndCombo API.");
    }

    {
      // Using the _simplified_ one-liner ListBox() api here
      // See "List boxes" section for examples of how to use the more flexible
      // BeginListBox()/EndListBox() api.
      DEMO_MARKER("Widgets/Basic/ListBox");
      const char *items[] = {"Apple",     "Banana",     "Cherry",
                             "Kiwi",      "Mango",      "Orange",
                             "Pineapple", "Strawberry", "Watermelon"};
      static int item_current = 1;
      Gui::ListBox("listbox", &item_current, items, ARRAYSIZE(items), 4);
      Gui::SameLine();
      HelpMarker(
          "Using the simplified one-liner ListBox API here.\nRefer to the "
          "\"List boxes\" section below for an explanation of how to use the "
          "more flexible and general BeginListBox/EndListBox API.");
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Tooltips");
  if (Gui::TreeNode("Tooltips")) {
    // Tooltips are windows following the mouse. They do not take focus away.
    Gui::SeparatorText("General");

    // Typical use cases:
    // - Short-form (text only):      SetItemTooltip("Hello");
    // - Short-form (any contents):   if (BeginItemTooltip()) { Text("Hello");
    // EndTooltip(); }

    // - Full-form (text only):       if (IsItemHovered(...)) {
    // SetTooltip("Hello"); }
    // - Full-form (any contents):    if (IsItemHovered(...) && BeginTooltip())
    // { Text("Hello"); EndTooltip(); }

    HelpMarker("Tooltip are typically created by using a IsItemHovered() + "
               "SetTooltip() sequence.\n\n"
               "We provide a helper SetItemTooltip() function to perform the "
               "two with standards flags.");

    Vec2 sz = Vec2(-FLT_MIN, 0.0f);

    Gui::Button("Basic", sz);
    Gui::SetItemTooltip("I am a tooltip");

    Gui::Button("Fancy", sz);
    if (Gui::BeginItemTooltip()) {
      Gui::Text("I am a fancy tooltip");
      static float arr[] = {0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f};
      Gui::PlotLines("Curve", arr, ARRAYSIZE(arr));
      Gui::Text("Sin(time) = %f", sinf((float)Gui::GetTime()));
      Gui::EndTooltip();
    }

    Gui::SeparatorText("Always On");

    // Showcase NOT relying on a IsItemHovered() to emit a tooltip.
    // Here the tooltip is always emitted when 'always_on == true'.
    static int always_on = 0;
    Gui::RadioButton("Off", &always_on, 0);
    Gui::SameLine();
    Gui::RadioButton("Always On (Simple)", &always_on, 1);
    Gui::SameLine();
    Gui::RadioButton("Always On (Advanced)", &always_on, 2);
    if (always_on == 1)
      Gui::SetTooltip("I am following you around.");
    else if (always_on == 2 && Gui::BeginTooltip()) {
      Gui::ProgressBar(sinf((float)Gui::GetTime()) * 0.5f + 0.5f,
                       Vec2(Gui::GetFontSize() * 25, 0.0f));
      Gui::EndTooltip();
    }

    Gui::SeparatorText("Custom");

    HelpMarker("Passing HoveredFlags_ForTooltip to IsItemHovered() is the "
               "preferred way to standardize"
               "tooltip activation details across your application. You may "
               "however decide to use custom"
               "flags for a specific tooltip instance.");

    // The following examples are passed for documentation purpose but may not
    // be useful to most users. Passing HoveredFlags_ForTooltip to
    // IsItemHovered() will pull HoveredFlags flags values from
    // 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav'
    // depending on whether mouse or gamepad/keyboard is being used. With
    // default settings, HoveredFlags_ForTooltip is equivalent to
    // HoveredFlags_DelayShort + HoveredFlags_Stationary.
    Gui::Button("Manual", sz);
    if (Gui::IsItemHovered(HoveredFlags_ForTooltip))
      Gui::SetTooltip("I am a manually emitted tooltip.");

    Gui::Button("DelayNone", sz);
    if (Gui::IsItemHovered(HoveredFlags_DelayNone))
      Gui::SetTooltip("I am a tooltip with no delay.");

    Gui::Button("DelayShort", sz);
    if (Gui::IsItemHovered(HoveredFlags_DelayShort |
                           HoveredFlags_NoSharedDelay))
      Gui::SetTooltip("I am a tooltip with a short delay (%0.2f sec).",
                      Gui::GetStyle().HoverDelayShort);

    Gui::Button("DelayLong", sz);
    if (Gui::IsItemHovered(HoveredFlags_DelayNormal |
                           HoveredFlags_NoSharedDelay))
      Gui::SetTooltip("I am a tooltip with a long delay (%0.2f sec).",
                      Gui::GetStyle().HoverDelayNormal);

    Gui::Button("Stationary", sz);
    if (Gui::IsItemHovered(HoveredFlags_Stationary))
      Gui::SetTooltip(
          "I am a tooltip requiring mouse to be stationary before activating.");

    // Using HoveredFlags_ForTooltip will pull flags from
    // 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav',
    // which default value include the HoveredFlags_AllowWhenDisabled flag.
    // As a result, Set
    Gui::BeginDisabled();
    Gui::Button("Disabled item", sz);
    Gui::EndDisabled();
    if (Gui::IsItemHovered(HoveredFlags_ForTooltip))
      Gui::SetTooltip("I am a a tooltip for a disabled item.");

    Gui::TreePop();
  }

  // Testing OnceUponAFrame helper.
  // static OnceUponAFrame once;
  // for (int i = 0; i < 5; i++)
  //    if (once)
  //        Gui::Text("This will be displayed only once.");

  DEMO_MARKER("Widgets/Tree Nodes");
  if (Gui::TreeNode("Tree Nodes")) {
    DEMO_MARKER("Widgets/Tree Nodes/Basic trees");
    if (Gui::TreeNode("Basic trees")) {
      for (int i = 0; i < 5; i++) {
        // Use SetNextItemOpen() so set the default state of a node to be open.
        // We could also use TreeNodeEx() with the
        // TreeNodeFlags_DefaultOpen flag to achieve the same thing!
        if (i == 0)
          Gui::SetNextItemOpen(true, Cond_Once);

        if (Gui::TreeNode((void *)(intptr_t)i, "Child %d", i)) {
          Gui::Text("blah blah");
          Gui::SameLine();
          if (Gui::SmallButton("button")) {
          }
          Gui::TreePop();
        }
      }
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Tree Nodes/Advanced, with Selectable nodes");
    if (Gui::TreeNode("Advanced, with Selectable nodes")) {
      HelpMarker("This is a more typical looking tree with selectable nodes.\n"
                 "Click to select, CTRL+Click to toggle, click on arrows or "
                 "double-click to open.");
      static TreeNodeFlags base_flags = TreeNodeFlags_OpenOnArrow |
                                        TreeNodeFlags_OpenOnDoubleClick |
                                        TreeNodeFlags_SpanAvailWidth;
      static bool align_label_with_current_x_position = false;
      static bool test_drag_and_drop = false;
      Gui::CheckboxFlags("TreeNodeFlags_OpenOnArrow", &base_flags,
                         TreeNodeFlags_OpenOnArrow);
      Gui::CheckboxFlags("TreeNodeFlags_OpenOnDoubleClick", &base_flags,
                         TreeNodeFlags_OpenOnDoubleClick);
      Gui::CheckboxFlags("TreeNodeFlags_SpanAvailWidth", &base_flags,
                         TreeNodeFlags_SpanAvailWidth);
      Gui::SameLine();
      HelpMarker("Extend hit area to all available width instead of allowing "
                 "more items to be laid out after the node.");
      Gui::CheckboxFlags("TreeNodeFlags_SpanFullWidth", &base_flags,
                         TreeNodeFlags_SpanFullWidth);
      Gui::CheckboxFlags("TreeNodeFlags_SpanAllColumns", &base_flags,
                         TreeNodeFlags_SpanAllColumns);
      Gui::SameLine();
      HelpMarker("For use in Tables only.");
      Gui::Checkbox("Align label with current X position",
                    &align_label_with_current_x_position);
      Gui::Checkbox("Test tree node as drag source", &test_drag_and_drop);
      Gui::Text("Hello!");
      if (align_label_with_current_x_position)
        Gui::Unindent(Gui::GetTreeNodeToLabelSpacing());

      // 'selection_mask' is dumb representation of what may be user-side
      // selection state.
      //  You may retain selection state inside or outside your objects in
      //  whatever format you see fit.
      // 'node_clicked' is temporary storage of what node we have clicked to
      // process selection at the end
      /// of the loop. May be a pointer to your own node type, etc.
      static int selection_mask = (1 << 2);
      int node_clicked = -1;
      for (int i = 0; i < 6; i++) {
        // Disable the default "open on single-click behavior" + set Selected
        // flag according to our selection. To alter selection we use
        // IsItemClicked() && !IsItemToggledOpen(), so clicking on an arrow
        // doesn't alter selection.
        TreeNodeFlags node_flags = base_flags;
        const bool is_selected = (selection_mask & (1 << i)) != 0;
        if (is_selected)
          node_flags |= TreeNodeFlags_Selected;
        if (i < 3) {
          // Items 0..2 are Tree Node
          bool node_open = Gui::TreeNodeEx((void *)(intptr_t)i, node_flags,
                                           "Selectable Node %d", i);
          if (Gui::IsItemClicked() && !Gui::IsItemToggledOpen())
            node_clicked = i;
          if (test_drag_and_drop && Gui::BeginDragDropSource()) {
            Gui::SetDragDropPayload("_TREENODE", NULL, 0);
            Gui::Text("This is a drag and drop source");
            Gui::EndDragDropSource();
          }
          if (node_open) {
            Gui::BulletText("Blah blah\nBlah Blah");
            Gui::TreePop();
          }
        } else {
          // Items 3..5 are Tree Leaves
          // The only reason we use TreeNode at all is to allow selection of the
          // leaf. Otherwise we can use BulletText() or advance the cursor by
          // GetTreeNodeToLabelSpacing() and call Text().
          node_flags |= TreeNodeFlags_Leaf |
                        TreeNodeFlags_NoTreePushOnOpen; // TreeNodeFlags_Bullet
          Gui::TreeNodeEx((void *)(intptr_t)i, node_flags, "Selectable Leaf %d",
                          i);
          if (Gui::IsItemClicked() && !Gui::IsItemToggledOpen())
            node_clicked = i;
          if (test_drag_and_drop && Gui::BeginDragDropSource()) {
            Gui::SetDragDropPayload("_TREENODE", NULL, 0);
            Gui::Text("This is a drag and drop source");
            Gui::EndDragDropSource();
          }
        }
      }
      if (node_clicked != -1) {
        // Update selection state
        // (process outside of tree loop to avoid visual inconsistencies during
        // the clicking frame)
        if (Gui::GetIO().KeyCtrl)
          selection_mask ^= (1 << node_clicked); // CTRL+click to toggle
        else // if (!(selection_mask & (1 << node_clicked))) // Depending on
             // selection behavior you want, may want to preserve selection when
             // clicking on item that is part of the selection
          selection_mask = (1 << node_clicked); // Click to single-select
      }
      if (align_label_with_current_x_position)
        Gui::Indent(Gui::GetTreeNodeToLabelSpacing());
      Gui::TreePop();
    }
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Collapsing Headers");
  if (Gui::TreeNode("Collapsing Headers")) {
    static bool closable_group = true;
    Gui::Checkbox("Show 2nd header", &closable_group);
    if (Gui::CollapsingHeader("Header", TreeNodeFlags_None)) {
      Gui::Text("IsItemHovered: %d", Gui::IsItemHovered());
      for (int i = 0; i < 5; i++)
        Gui::Text("Some content %d", i);
    }
    if (Gui::CollapsingHeader("Header with a close button", &closable_group)) {
      Gui::Text("IsItemHovered: %d", Gui::IsItemHovered());
      for (int i = 0; i < 5; i++)
        Gui::Text("More content %d", i);
    }
    /*
    if (Gui::CollapsingHeader("Header with a bullet",
    TreeNodeFlags_Bullet)) Gui::Text("IsItemHovered: %d",
    Gui::IsItemHovered());
    */
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Bullets");
  if (Gui::TreeNode("Bullets")) {
    Gui::BulletText("Bullet point 1");
    Gui::BulletText("Bullet point 2\nOn multiple lines");
    if (Gui::TreeNode("Tree node")) {
      Gui::BulletText("Another bullet point");
      Gui::TreePop();
    }
    Gui::Bullet();
    Gui::Text("Bullet point 3 (two calls)");
    Gui::Bullet();
    Gui::SmallButton("Button");
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Text");
  if (Gui::TreeNode("Text")) {
    DEMO_MARKER("Widgets/Text/Colored Text");
    if (Gui::TreeNode("Colorful Text")) {
      // Using shortcut. You can use PushStyleColor()/PopStyleColor() for more
      // flexibility.
      Gui::TextColored(Vec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
      Gui::TextColored(Vec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow");
      Gui::TextDisabled("Disabled");
      Gui::SameLine();
      HelpMarker("The TextDisabled color is stored in Style.");
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Text/Word Wrapping");
    if (Gui::TreeNode("Word Wrapping")) {
      // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more
      // flexibility.
      Gui::TextWrapped("This text should automatically wrap on the edge of "
                       "the window. The current implementation "
                       "for text wrapping follows simple rules suitable for "
                       "English and possibly other languages.");
      Gui::Spacing();

      static float wrap_width = 200.0f;
      Gui::SliderFloat("Wrap width", &wrap_width, -20, 600, "%.0f");

      DrawList *draw_list = Gui::GetWindowDrawList();
      for (int n = 0; n < 2; n++) {
        Gui::Text("Test paragraph %d:", n);
        Vec2 pos = Gui::GetCursorScreenPos();
        Vec2 marker_min = Vec2(pos.x + wrap_width, pos.y);
        Vec2 marker_max =
            Vec2(pos.x + wrap_width + 10, pos.y + Gui::GetTextLineHeight());
        Gui::PushTextWrapPos(Gui::GetCursorPos().x + wrap_width);
        if (n == 0)
          Gui::Text("The lazy dog is a good dog. This paragraph should fit "
                    "within %.0f pixels. Testing a 1 character word. The "
                    "quick brown fox jumps over the lazy dog.",
                    wrap_width);
        else
          Gui::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   "
                    "ffffffff. gggggggg!hhhhhhhh");

        // Draw actual text bounding box, following by marker of our expected
        // limit (should not overlap!)
        draw_list->AddRect(Gui::GetItemRectMin(), Gui::GetItemRectMax(),
                           COL32(255, 255, 0, 255));
        draw_list->AddRectFilled(marker_min, marker_max,
                                 COL32(255, 0, 255, 255));
        Gui::PopTextWrapPos();
      }

      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Text/UTF-8 Text");
    if (Gui::TreeNode("UTF-8 Text")) {
      // UTF-8 test with Japanese characters
      // (Needs a suitable font? Try "Google Noto" or "Arial Unicode". See
      // docs/FONTS.md for details.)
      // - From C++11 you can use the u8"my text" syntax to encode literal
      // strings as UTF-8
      // - For earlier compiler, you may be able to encode your sources as UTF-8
      // (e.g. in Visual Studio, you
      //   can save your source files as 'UTF-8 without signature').
      // - FOR THIS DEMO FILE ONLY, BECAUSE WE WANT TO SUPPORT OLD COMPILERS, WE
      // ARE *NOT* INCLUDING RAW UTF-8
      //   CHARACTERS IN THIS SOURCE FILE. Instead we are encoding a few strings
      //   with hexadecimal constants. Don't do this in your application! Please
      //   use u8"text in any language" in your application!
      // Note that characters values are preserved even by InputText() if the
      // font cannot be displayed, so you can safely copy & paste garbled
      // characters into another application.
      Gui::TextWrapped("CJK text will only appear if the font was loaded "
                       "with the appropriate CJK character ranges. "
                       "Call io.Fonts->AddFontFromFileTTF() manually to load "
                       "extra character ranges. "
                       "Read docs/FONTS.md for details.");
      Gui::Text("Hiragana: "
                "\xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 "
                "(kakikukeko)"); // Normally we would use u8"blah blah" with the
                                 // proper characters directly in the string.
      Gui::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
      static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
      // static char buf[32] = u8"NIHONGO"; // <- this is how you would write it
      // with C++11, using real kanjis
      Gui::InputText("UTF-8 input", buf, ARRAYSIZE(buf));
      Gui::TreePop();
    }
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Images");
  if (Gui::TreeNode("Images")) {
    IO &io = Gui::GetIO();
    Gui::TextWrapped("Below we are displaying the font texture (which is the "
                     "only texture we have access to in this demo). "
                     "Use the 'TextureID' type as storage to pass pointers "
                     "or identifier to your own texture data. "
                     "Hover the texture for a zoomed view!");

    // Below we are displaying the font texture because it is the only texture
    // we have access to inside the demo! Remember that TextureID is just
    // storage for whatever you want it to be. It is essentially a value that
    // will be passed to the rendering backend via the DrawCmd structure.
    // If you use one of the default XXXX.cpp rendering backend, they
    // all have comments at the top of their respective source file to specify
    // what they expect to be stored in TextureID, for example:
    // - The dx11.cpp renderer expect a 'ID3D11ShaderResourceView*'
    // pointer
    // - The opengl3.cpp renderer expect a GLuint OpenGL texture
    // identifier, etc. More:
    // - If you decided that TextureID = MyEngineTexture*, then you can pass
    // your MyEngineTexture* pointers
    //   to Gui::Image(), and gather width/height through your own functions,
    //   etc.
    // - You can use ShowMetricsWindow() to inspect the draw data that are being
    // passed to your renderer,
    //   it will help you debug issues if you are confused about it.
    // - Consider using the lower-level DrawList::AddImage() API, via
    // Gui::GetWindowDrawList()->AddImage().
    TextureID my_tex_id = io.Fonts->TexID;
    float my_tex_w = (float)io.Fonts->TexWidth;
    float my_tex_h = (float)io.Fonts->TexHeight;
    {
      static bool use_text_color_for_tint = false;
      Gui::Checkbox("Use Text Color for Tint", &use_text_color_for_tint);
      Gui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
      Vec2 pos = Gui::GetCursorScreenPos();
      Vec2 uv_min = Vec2(0.0f, 0.0f); // Top-left
      Vec2 uv_max = Vec2(1.0f, 1.0f); // Lower-right
      Vec4 tint_col = use_text_color_for_tint
                          ? Gui::GetStyleColorVec4(Col_Text)
                          : Vec4(1.0f, 1.0f, 1.0f, 1.0f); // No tint
      Vec4 border_col = Gui::GetStyleColorVec4(Col_Border);
      Gui::Image(my_tex_id, Vec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col,
                 border_col);
      if (Gui::BeginItemTooltip()) {
        float region_sz = 32.0f;
        float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
        float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
        float zoom = 4.0f;
        if (region_x < 0.0f) {
          region_x = 0.0f;
        } else if (region_x > my_tex_w - region_sz) {
          region_x = my_tex_w - region_sz;
        }
        if (region_y < 0.0f) {
          region_y = 0.0f;
        } else if (region_y > my_tex_h - region_sz) {
          region_y = my_tex_h - region_sz;
        }
        Gui::Text("Min: (%.2f, %.2f)", region_x, region_y);
        Gui::Text("Max: (%.2f, %.2f)", region_x + region_sz,
                  region_y + region_sz);
        Vec2 uv0 = Vec2((region_x) / my_tex_w, (region_y) / my_tex_h);
        Vec2 uv1 = Vec2((region_x + region_sz) / my_tex_w,
                        (region_y + region_sz) / my_tex_h);
        Gui::Image(my_tex_id, Vec2(region_sz * zoom, region_sz * zoom), uv0,
                   uv1, tint_col, border_col);
        Gui::EndTooltip();
      }
    }

    DEMO_MARKER("Widgets/Images/Textured buttons");
    Gui::TextWrapped("And now some textured buttons..");
    static int pressed_count = 0;
    for (int i = 0; i < 8; i++) {
      // UV coordinates are often (0.0f, 0.0f) and (1.0f, 1.0f) to display an
      // entire textures. Here are trying to display only a 32x32 pixels area of
      // the texture, hence the UV computation.
      Gui::PushID(i);
      if (i > 0)
        Gui::PushStyleVar(StyleVar_FramePadding, Vec2(i - 1.0f, i - 1.0f));
      Vec2 size =
          Vec2(32.0f, 32.0f);      // Size of the image we want to make visible
      Vec2 uv0 = Vec2(0.0f, 0.0f); // UV coordinates for lower-left
      Vec2 uv1 =
          Vec2(32.0f / my_tex_w,
               32.0f / my_tex_h); // UV coordinates for (32,32) in our texture
      Vec4 bg_col = Vec4(0.0f, 0.0f, 0.0f, 1.0f);   // Black background
      Vec4 tint_col = Vec4(1.0f, 1.0f, 1.0f, 1.0f); // No tint
      if (Gui::ImageButton("", my_tex_id, size, uv0, uv1, bg_col, tint_col))
        pressed_count += 1;
      if (i > 0)
        Gui::PopStyleVar();
      Gui::PopID();
      Gui::SameLine();
    }
    Gui::NewLine();
    Gui::Text("Pressed %d times.", pressed_count);
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Combo");
  if (Gui::TreeNode("Combo")) {
    // Combo Boxes are also called "Dropdown" in other systems
    // Expose flags as checkbox for the demo
    static ComboFlags flags = 0;
    Gui::CheckboxFlags("ComboFlags_PopupAlignLeft", &flags,
                       ComboFlags_PopupAlignLeft);
    Gui::SameLine();
    HelpMarker("Only makes a difference if the popup is larger than the combo");
    if (Gui::CheckboxFlags("ComboFlags_NoArrowButton", &flags,
                           ComboFlags_NoArrowButton))
      flags &= ~ComboFlags_NoPreview; // Clear the other flag, as we cannot
                                      // combine both
    if (Gui::CheckboxFlags("ComboFlags_NoPreview", &flags,
                           ComboFlags_NoPreview))
      flags &= ~(ComboFlags_NoArrowButton |
                 ComboFlags_WidthFitPreview); // Clear the other flag, as
                                              // we cannot combine both
    if (Gui::CheckboxFlags("ComboFlags_WidthFitPreview", &flags,
                           ComboFlags_WidthFitPreview))
      flags &= ~ComboFlags_NoPreview;

    // Override default popup height
    if (Gui::CheckboxFlags("ComboFlags_HeightSmall", &flags,
                           ComboFlags_HeightSmall))
      flags &= ~(ComboFlags_HeightMask_ & ~ComboFlags_HeightSmall);
    if (Gui::CheckboxFlags("ComboFlags_HeightRegular", &flags,
                           ComboFlags_HeightRegular))
      flags &= ~(ComboFlags_HeightMask_ & ~ComboFlags_HeightRegular);
    if (Gui::CheckboxFlags("ComboFlags_HeightLargest", &flags,
                           ComboFlags_HeightLargest))
      flags &= ~(ComboFlags_HeightMask_ & ~ComboFlags_HeightLargest);

    // Using the generic BeginCombo() API, you have full control over how to
    // display the combo contents. (your selection data could be an index, a
    // pointer to the object, an id for the object, a flag intrusively stored in
    // the object itself, etc.)
    const char *items[] = {"AAAA", "BBBB",    "CCCC", "DDDD",   "EEEE",
                           "FFFF", "GGGG",    "HHHH", "IIII",   "JJJJ",
                           "KKKK", "LLLLLLL", "MMMM", "OOOOOOO"};
    static int item_current_idx =
        0; // Here we store our selection data as an index.
    const char *combo_preview_value =
        items[item_current_idx]; // Pass in the preview value visible before
                                 // opening the combo (it could be anything)
    if (Gui::BeginCombo("combo 1", combo_preview_value, flags)) {
      for (int n = 0; n < ARRAYSIZE(items); n++) {
        const bool is_selected = (item_current_idx == n);
        if (Gui::Selectable(items[n], is_selected))
          item_current_idx = n;

        // Set the initial focus when opening the combo (scrolling + keyboard
        // navigation focus)
        if (is_selected)
          Gui::SetItemDefaultFocus();
      }
      Gui::EndCombo();
    }

    Gui::Spacing();
    Gui::SeparatorText("One-liner variants");
    HelpMarker("Flags above don't apply to this section.");

    // Simplified one-liner Combo() API, using values packed in a single
    // constant string This is a convenience for when the selection set is small
    // and known at compile-time.
    static int item_current_2 = 0;
    Gui::Combo("combo 2 (one-liner)", &item_current_2,
               "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

    // Simplified one-liner Combo() using an array of const char*
    // This is not very useful (may obsolete): prefer using
    // BeginCombo()/EndCombo() for full control.
    static int item_current_3 = -1; // If the selection isn't within 0..count,
                                    // Combo won't display a preview
    Gui::Combo("combo 3 (array)", &item_current_3, items, ARRAYSIZE(items));

    // Simplified one-liner Combo() using an accessor function
    static int item_current_4 = 0;
    Gui::Combo(
        "combo 4 (function)", &item_current_4,
        [](void *data, int n) { return ((const char **)data)[n]; }, items,
        ARRAYSIZE(items));

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/List Boxes");
  if (Gui::TreeNode("List boxes")) {
    // BeginListBox() is essentially a thin wrapper to using
    // BeginChild()/EndChild() with the ChildFlags_FrameStyle flag for
    // stylistic changes + displaying a label. You may be tempted to simply use
    // BeginChild() directly, however note that BeginChild() requires EndChild()
    // to always be called (inconsistent with BeginListBox()/EndListBox()).

    // Using the generic BeginListBox() API, you have full control over how to
    // display the combo contents. (your selection data could be an index, a
    // pointer to the object, an id for the object, a flag intrusively stored in
    // the object itself, etc.)
    const char *items[] = {"AAAA", "BBBB",    "CCCC", "DDDD",   "EEEE",
                           "FFFF", "GGGG",    "HHHH", "IIII",   "JJJJ",
                           "KKKK", "LLLLLLL", "MMMM", "OOOOOOO"};
    static int item_current_idx =
        0; // Here we store our selection data as an index.
    if (Gui::BeginListBox("listbox 1")) {
      for (int n = 0; n < ARRAYSIZE(items); n++) {
        const bool is_selected = (item_current_idx == n);
        if (Gui::Selectable(items[n], is_selected))
          item_current_idx = n;

        // Set the initial focus when opening the combo (scrolling + keyboard
        // navigation focus)
        if (is_selected)
          Gui::SetItemDefaultFocus();
      }
      Gui::EndListBox();
    }

    // Custom size: use all width, 5 items tall
    Gui::Text("Full-width:");
    if (Gui::BeginListBox(
            "##listbox 2",
            Vec2(-FLT_MIN, 5 * Gui::GetTextLineHeightWithSpacing()))) {
      for (int n = 0; n < ARRAYSIZE(items); n++) {
        const bool is_selected = (item_current_idx == n);
        if (Gui::Selectable(items[n], is_selected))
          item_current_idx = n;

        // Set the initial focus when opening the combo (scrolling + keyboard
        // navigation focus)
        if (is_selected)
          Gui::SetItemDefaultFocus();
      }
      Gui::EndListBox();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Selectables");
  if (Gui::TreeNode("Selectables")) {
    // Selectable() has 2 overloads:
    // - The one taking "bool selected" as a read-only selection information.
    //   When Selectable() has been clicked it returns true and you can alter
    //   selection state accordingly.
    // - The one taking "bool* p_selected" as a read-write selection information
    // (convenient in some cases) The earlier is more flexible, as in real
    // application your selection may be stored in many different ways and not
    // necessarily inside a bool value (e.g. in flags within objects, as an
    // external list, etc).
    DEMO_MARKER("Widgets/Selectables/Basic");
    if (Gui::TreeNode("Basic")) {
      static bool selection[5] = {false, true, false, false};
      Gui::Selectable("1. I am selectable", &selection[0]);
      Gui::Selectable("2. I am selectable", &selection[1]);
      Gui::Selectable("3. I am selectable", &selection[2]);
      if (Gui::Selectable("4. I am double clickable", selection[3],
                          SelectableFlags_AllowDoubleClick))
        if (Gui::IsMouseDoubleClicked(0))
          selection[3] = !selection[3];
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Selectables/Single Selection");
    if (Gui::TreeNode("Selection State: Single Selection")) {
      static int selected = -1;
      for (int n = 0; n < 5; n++) {
        char buf[32];
        sprintf(buf, "Object %d", n);
        if (Gui::Selectable(buf, selected == n))
          selected = n;
      }
      Gui::TreePop();
    }
    DEMO_MARKER("Widgets/Selectables/Multiple Selection");
    if (Gui::TreeNode("Selection State: Multiple Selection")) {
      HelpMarker("Hold CTRL and click to select multiple items.");
      static bool selection[5] = {false, false, false, false, false};
      for (int n = 0; n < 5; n++) {
        char buf[32];
        sprintf(buf, "Object %d", n);
        if (Gui::Selectable(buf, selection[n])) {
          if (!Gui::GetIO().KeyCtrl) // Clear selection when CTRL is not held
            memset(selection, 0, sizeof(selection));
          selection[n] ^= 1;
        }
      }
      Gui::TreePop();
    }
    DEMO_MARKER("Widgets/Selectables/Rendering more items on the same line");
    if (Gui::TreeNode("Rendering more items on the same line")) {
      // (1) Using SetNextItemAllowOverlap()
      // (2) Using the Selectable() override that takes "bool* p_selected"
      // parameter, the bool value is toggled automatically.
      static bool selected[3] = {false, false, false};
      Gui::SetNextItemAllowOverlap();
      Gui::Selectable("main.c", &selected[0]);
      Gui::SameLine();
      Gui::SmallButton("Link 1");
      Gui::SetNextItemAllowOverlap();
      Gui::Selectable("Hello.cpp", &selected[1]);
      Gui::SameLine();
      Gui::SmallButton("Link 2");
      Gui::SetNextItemAllowOverlap();
      Gui::Selectable("Hello.h", &selected[2]);
      Gui::SameLine();
      Gui::SmallButton("Link 3");
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Selectables/In columns");
    if (Gui::TreeNode("In columns")) {
      static bool selected[10] = {};

      if (Gui::BeginTable("split1", 3,
                          TableFlags_Resizable | TableFlags_NoSavedSettings |
                              TableFlags_Borders)) {
        for (int i = 0; i < 10; i++) {
          char label[32];
          sprintf(label, "Item %d", i);
          Gui::TableNextColumn();
          Gui::Selectable(label,
                          &selected[i]); // FIXME-TABLE: Selection overlap
        }
        Gui::EndTable();
      }
      Gui::Spacing();
      if (Gui::BeginTable("split2", 3,
                          TableFlags_Resizable | TableFlags_NoSavedSettings |
                              TableFlags_Borders)) {
        for (int i = 0; i < 10; i++) {
          char label[32];
          sprintf(label, "Item %d", i);
          Gui::TableNextRow();
          Gui::TableNextColumn();
          Gui::Selectable(label, &selected[i], SelectableFlags_SpanAllColumns);
          Gui::TableNextColumn();
          Gui::Text("Some other contents");
          Gui::TableNextColumn();
          Gui::Text("123456");
        }
        Gui::EndTable();
      }
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Selectables/Grid");
    if (Gui::TreeNode("Grid")) {
      static char selected[4][4] = {
          {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

      // Add in a bit of silly fun...
      const float time = (float)Gui::GetTime();
      const bool winning_state = memchr(selected, 0, sizeof(selected)) ==
                                 NULL; // If all cells are selected...
      if (winning_state)
        Gui::PushStyleVar(StyleVar_SelectableTextAlign,
                          Vec2(0.5f + 0.5f * cosf(time * 2.0f),
                               0.5f + 0.5f * sinf(time * 3.0f)));

      for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++) {
          if (x > 0)
            Gui::SameLine();
          Gui::PushID(y * 4 + x);
          if (Gui::Selectable("Sailor", selected[y][x] != 0, 0, Vec2(50, 50))) {
            // Toggle clicked cell + toggle neighbors
            selected[y][x] ^= 1;
            if (x > 0) {
              selected[y][x - 1] ^= 1;
            }
            if (x < 3) {
              selected[y][x + 1] ^= 1;
            }
            if (y > 0) {
              selected[y - 1][x] ^= 1;
            }
            if (y < 3) {
              selected[y + 1][x] ^= 1;
            }
          }
          Gui::PopID();
        }

      if (winning_state)
        Gui::PopStyleVar();
      Gui::TreePop();
    }
    DEMO_MARKER("Widgets/Selectables/Alignment");
    if (Gui::TreeNode("Alignment")) {
      HelpMarker("By default, Selectables uses style.SelectableTextAlign but "
                 "it can be overridden on a per-item "
                 "basis using PushStyleVar(). You'll probably want to always "
                 "keep your default situation to "
                 "left-align otherwise it becomes difficult to layout multiple "
                 "items on a same line");
      static bool selected[3 * 3] = {true,  false, true,  false, true,
                                     false, true,  false, true};
      for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
          Vec2 alignment = Vec2((float)x / 2.0f, (float)y / 2.0f);
          char name[32];
          sprintf(name, "(%.1f,%.1f)", alignment.x, alignment.y);
          if (x > 0)
            Gui::SameLine();
          Gui::PushStyleVar(StyleVar_SelectableTextAlign, alignment);
          Gui::Selectable(name, &selected[3 * y + x], SelectableFlags_None,
                          Vec2(80, 80));
          Gui::PopStyleVar();
        }
      }
      Gui::TreePop();
    }
    Gui::TreePop();
  }

  // To wire InputText() with std::string or any other custom string type,
  // see the "Text Input > Resize Callback" section of this demo, and the
  // misc/cpp/stdlib.h file.
  DEMO_MARKER("Widgets/Text Input");
  if (Gui::TreeNode("Text Input")) {
    DEMO_MARKER("Widgets/Text Input/Multi-line Text Input");
    if (Gui::TreeNode("Multi-line Text Input")) {
      // Note: we are using a fixed-sized buffer for simplicity here. See
      // InputTextFlags_CallbackResize and the code in
      // misc/cpp/stdlib.h for how to setup InputText() for dynamically
      // resizing strings.
      static char text[1024 * 16] =
          "/*\n"
          " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
          " the hexadecimal encoding of one offending instruction,\n"
          " more formally, the invalid operand with locked CMPXCHG8B\n"
          " instruction bug, is a design flaw in the majority of\n"
          " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
          " processors (all in the P5 microarchitecture).\n"
          "*/\n\n"
          "label:\n"
          "\tlock cmpxchg8b eax\n";

      static InputTextFlags flags = InputTextFlags_AllowTabInput;
      HelpMarker("You can use the InputTextFlags_CallbackResize facility "
                 "if you need to wire InputTextMultiline() to a dynamic string "
                 "type. See misc/cpp/stdlib.h for an example. (This is "
                 "not demonstrated in demo.cpp because we don't want to "
                 "include <string> in here)");
      Gui::CheckboxFlags("InputTextFlags_ReadOnly", &flags,
                         InputTextFlags_ReadOnly);
      Gui::CheckboxFlags("InputTextFlags_AllowTabInput", &flags,
                         InputTextFlags_AllowTabInput);
      Gui::SameLine();
      HelpMarker("When _AllowTabInput is set, passing through the widget with "
                 "Tabbing doesn't automatically activate it, in order to also "
                 "cycling through subsequent widgets.");
      Gui::CheckboxFlags("InputTextFlags_CtrlEnterForNewLine", &flags,
                         InputTextFlags_CtrlEnterForNewLine);
      Gui::InputTextMultiline("##source", text, ARRAYSIZE(text),
                              Vec2(-FLT_MIN, Gui::GetTextLineHeight() * 16),
                              flags);
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Text Input/Filtered Text Input");
    if (Gui::TreeNode("Filtered Text Input")) {
      struct TextFilters {
        // Modify character input by altering 'data->Eventchar'
        // (InputTextFlags_CallbackCharFilter callback)
        static int FilterCasingSwap(InputTextCallbackData *data) {
          if (data->EventChar >= 'a' && data->EventChar <= 'z') {
            data->EventChar -= 'a' - 'A';
          } // Lowercase becomes uppercase
          else if (data->EventChar >= 'A' && data->EventChar <= 'Z') {
            data->EventChar += 'a' - 'A';
          } // Uppercase becomes lowercase
          return 0;
        }

        // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i',
        // otherwise return 1 (filter out)
        static int FilterLetters(InputTextCallbackData *data) {
          if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar))
            return 0;
          return 1;
        }
      };

      static char buf1[32] = "";
      Gui::InputText("default", buf1, 32);
      static char buf2[32] = "";
      Gui::InputText("decimal", buf2, 32, InputTextFlags_CharsDecimal);
      static char buf3[32] = "";
      Gui::InputText("hexadecimal", buf3, 32,
                     InputTextFlags_CharsHexadecimal |
                         InputTextFlags_CharsUppercase);
      static char buf4[32] = "";
      Gui::InputText("uppercase", buf4, 32, InputTextFlags_CharsUppercase);
      static char buf5[32] = "";
      Gui::InputText("no blank", buf5, 32, InputTextFlags_CharsNoBlank);
      static char buf6[32] = "";
      Gui::InputText("casing swap", buf6, 32, InputTextFlags_CallbackCharFilter,
                     TextFilters::FilterCasingSwap); // Use CharFilter callback
                                                     // to replace characters.
      static char buf7[32] = "";
      Gui::InputText("\"imgui\"", buf7, 32, InputTextFlags_CallbackCharFilter,
                     TextFilters::FilterLetters); // Use CharFilter callback to
                                                  // disable some characters.
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Text Input/Password input");
    if (Gui::TreeNode("Password Input")) {
      static char password[64] = "password123";
      Gui::InputText("password", password, ARRAYSIZE(password),
                     InputTextFlags_Password);
      Gui::SameLine();
      HelpMarker("Display all characters as '*'.\nDisable clipboard cut and "
                 "copy.\nDisable logging.\n");
      Gui::InputTextWithHint("password (w/ hint)", "<password>", password,
                             ARRAYSIZE(password), InputTextFlags_Password);
      Gui::InputText("password (clear)", password, ARRAYSIZE(password));
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Text Input/Completion, History, Edit Callbacks");
    if (Gui::TreeNode("Completion, History, Edit Callbacks")) {
      struct Funcs {
        static int MyCallback(InputTextCallbackData *data) {
          if (data->EventFlag == InputTextFlags_CallbackCompletion) {
            data->InsertChars(data->CursorPos, "..");
          } else if (data->EventFlag == InputTextFlags_CallbackHistory) {
            if (data->EventKey == Key_UpArrow) {
              data->DeleteChars(0, data->BufTextLen);
              data->InsertChars(0, "Pressed Up!");
              data->SelectAll();
            } else if (data->EventKey == Key_DownArrow) {
              data->DeleteChars(0, data->BufTextLen);
              data->InsertChars(0, "Pressed Down!");
              data->SelectAll();
            }
          } else if (data->EventFlag == InputTextFlags_CallbackEdit) {
            // Toggle casing of first character
            char c = data->Buf[0];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
              data->Buf[0] ^= 32;
            data->BufDirty = true;

            // Increment a counter
            int *p_int = (int *)data->UserData;
            *p_int = *p_int + 1;
          }
          return 0;
        }
      };
      static char buf1[64];
      Gui::InputText("Completion", buf1, 64, InputTextFlags_CallbackCompletion,
                     Funcs::MyCallback);
      Gui::SameLine();
      HelpMarker("Here we append \"..\" each time Tab is pressed. See "
                 "'Examples>Console' for a more meaningful demonstration of "
                 "using this callback.");

      static char buf2[64];
      Gui::InputText("History", buf2, 64, InputTextFlags_CallbackHistory,
                     Funcs::MyCallback);
      Gui::SameLine();
      HelpMarker("Here we replace and select text each time Up/Down are "
                 "pressed. See 'Examples>Console' for a more meaningful "
                 "demonstration of using this callback.");

      static char buf3[64];
      static int edit_count = 0;
      Gui::InputText("Edit", buf3, 64, InputTextFlags_CallbackEdit,
                     Funcs::MyCallback, (void *)&edit_count);
      Gui::SameLine();
      HelpMarker("Here we toggle the casing of the first character on every "
                 "edit + count edits.");
      Gui::SameLine();
      Gui::Text("(%d)", edit_count);

      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Text Input/Resize Callback");
    if (Gui::TreeNode("Resize Callback")) {
      // To wire InputText() with std::string or any other custom string type,
      // you can use the InputTextFlags_CallbackResize flag + create a
      // custom Gui::InputText() wrapper using your preferred type. See
      // misc/cpp/stdlib.h for an implementation of this using
      // std::string.
      HelpMarker("Using InputTextFlags_CallbackResize to wire your custom "
                 "string type to InputText().\n\n"
                 "See misc/cpp/stdlib.h for an implementation of this "
                 "for std::string.");
      struct Funcs {
        static int MyResizeCallback(InputTextCallbackData *data) {
          if (data->EventFlag == InputTextFlags_CallbackResize) {
            Vector<char> *my_str = (Vector<char> *)data->UserData;
            ASSERT(my_str->begin() == data->Buf);
            my_str->resize(
                data->BufSize); // NB: On resizing calls, generally
                                // data->BufSize == data->BufTextLen + 1
            data->Buf = my_str->begin();
          }
          return 0;
        }

        // Note: Because Gui:: is a namespace you would typically add your own
        // function into the namespace. For example, you code may declare a
        // function 'Gui::InputText(const char* label, MyString* my_str)'
        static bool MyInputTextMultiline(const char *label,
                                         Vector<char> *my_str,
                                         const Vec2 &size = Vec2(0, 0),
                                         InputTextFlags flags = 0) {
          ASSERT((flags & InputTextFlags_CallbackResize) == 0);
          return Gui::InputTextMultiline(
              label, my_str->begin(), (size_t)my_str->size(), size,
              flags | InputTextFlags_CallbackResize, Funcs::MyResizeCallback,
              (void *)my_str);
        }
      };

      // For this demo we are using Vector as a string container.
      // Note that because we need to store a terminating zero character, our
      // size/capacity are 1 more than usually reported by a typical string
      // class.
      static Vector<char> my_str;
      if (my_str.empty())
        my_str.push_back(0);
      Funcs::MyInputTextMultiline(
          "##MyStr", &my_str, Vec2(-FLT_MIN, Gui::GetTextLineHeight() * 16));
      Gui::Text("Data: %p\nSize: %d\nCapacity: %d", (void *)my_str.begin(),
                my_str.size(), my_str.capacity());
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Text Input/Miscellaneous");
    if (Gui::TreeNode("Miscellaneous")) {
      static char buf1[16];
      static InputTextFlags flags = InputTextFlags_EscapeClearsAll;
      Gui::CheckboxFlags("InputTextFlags_EscapeClearsAll", &flags,
                         InputTextFlags_EscapeClearsAll);
      Gui::CheckboxFlags("InputTextFlags_ReadOnly", &flags,
                         InputTextFlags_ReadOnly);
      Gui::CheckboxFlags("InputTextFlags_NoUndoRedo", &flags,
                         InputTextFlags_NoUndoRedo);
      Gui::InputText("Hello", buf1, ARRAYSIZE(buf1), flags);
      Gui::TreePop();
    }

    Gui::TreePop();
  }

  // Tabs
  DEMO_MARKER("Widgets/Tabs");
  if (Gui::TreeNode("Tabs")) {
    DEMO_MARKER("Widgets/Tabs/Basic");
    if (Gui::TreeNode("Basic")) {
      TabBarFlags tab_bar_flags = TabBarFlags_None;
      if (Gui::BeginTabBar("MyTabBar", tab_bar_flags)) {
        if (Gui::BeginTabItem("Avocado")) {
          Gui::Text("This is the Avocado tab!\nblah blah blah blah blah");
          Gui::EndTabItem();
        }
        if (Gui::BeginTabItem("Broccoli")) {
          Gui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
          Gui::EndTabItem();
        }
        if (Gui::BeginTabItem("Cucumber")) {
          Gui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
          Gui::EndTabItem();
        }
        Gui::EndTabBar();
      }
      Gui::Separator();
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Tabs/Advanced & Close Button");
    if (Gui::TreeNode("Advanced & Close Button")) {
      // Expose a couple of the available flags. In most cases you may just call
      // BeginTabBar() with no flags (0).
      static TabBarFlags tab_bar_flags = TabBarFlags_Reorderable;
      Gui::CheckboxFlags("TabBarFlags_Reorderable", &tab_bar_flags,
                         TabBarFlags_Reorderable);
      Gui::CheckboxFlags("TabBarFlags_AutoSelectNewTabs", &tab_bar_flags,
                         TabBarFlags_AutoSelectNewTabs);
      Gui::CheckboxFlags("TabBarFlags_TabListPopupButton", &tab_bar_flags,
                         TabBarFlags_TabListPopupButton);
      Gui::CheckboxFlags("TabBarFlags_NoCloseWithMiddleMouseButton",
                         &tab_bar_flags,
                         TabBarFlags_NoCloseWithMiddleMouseButton);
      if ((tab_bar_flags & TabBarFlags_FittingPolicyMask_) == 0)
        tab_bar_flags |= TabBarFlags_FittingPolicyDefault_;
      if (Gui::CheckboxFlags("TabBarFlags_FittingPolicyResizeDown",
                             &tab_bar_flags,
                             TabBarFlags_FittingPolicyResizeDown))
        tab_bar_flags &= ~(TabBarFlags_FittingPolicyMask_ ^
                           TabBarFlags_FittingPolicyResizeDown);
      if (Gui::CheckboxFlags("TabBarFlags_FittingPolicyScroll", &tab_bar_flags,
                             TabBarFlags_FittingPolicyScroll))
        tab_bar_flags &=
            ~(TabBarFlags_FittingPolicyMask_ ^ TabBarFlags_FittingPolicyScroll);

      // Tab Bar
      const char *names[4] = {"Artichoke", "Beetroot", "Celery", "Daikon"};
      static bool opened[4] = {true, true, true, true}; // Persistent user state
      for (int n = 0; n < ARRAYSIZE(opened); n++) {
        if (n > 0) {
          Gui::SameLine();
        }
        Gui::Checkbox(names[n], &opened[n]);
      }

      // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
      // the underlying bool will be set to false when the tab is closed.
      if (Gui::BeginTabBar("MyTabBar", tab_bar_flags)) {
        for (int n = 0; n < ARRAYSIZE(opened); n++)
          if (opened[n] &&
              Gui::BeginTabItem(names[n], &opened[n], TabItemFlags_None)) {
            Gui::Text("This is the %s tab!", names[n]);
            if (n & 1)
              Gui::Text("I am an odd tab.");
            Gui::EndTabItem();
          }
        Gui::EndTabBar();
      }
      Gui::Separator();
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Tabs/TabItemButton & Leading-Trailing flags");
    if (Gui::TreeNode("TabItemButton & Leading/Trailing flags")) {
      static Vector<int> active_tabs;
      static int next_tab_id = 0;
      if (next_tab_id == 0) // Initialize with some default tabs
        for (int i = 0; i < 3; i++)
          active_tabs.push_back(next_tab_id++);

      // TabItemButton() and Leading/Trailing flags are distinct features which
      // we will demo together. (It is possible to submit regular tabs with
      // Leading/Trailing flags, or TabItemButton tabs without Leading/Trailing
      // flags... but they tend to make more sense together)
      static bool show_leading_button = true;
      static bool show_trailing_button = true;
      Gui::Checkbox("Show Leading TabItemButton()", &show_leading_button);
      Gui::Checkbox("Show Trailing TabItemButton()", &show_trailing_button);

      // Expose some other flags which are useful to showcase how they interact
      // with Leading/Trailing tabs
      static TabBarFlags tab_bar_flags = TabBarFlags_AutoSelectNewTabs |
                                         TabBarFlags_Reorderable |
                                         TabBarFlags_FittingPolicyResizeDown;
      Gui::CheckboxFlags("TabBarFlags_TabListPopupButton", &tab_bar_flags,
                         TabBarFlags_TabListPopupButton);
      if (Gui::CheckboxFlags("TabBarFlags_FittingPolicyResizeDown",
                             &tab_bar_flags,
                             TabBarFlags_FittingPolicyResizeDown))
        tab_bar_flags &= ~(TabBarFlags_FittingPolicyMask_ ^
                           TabBarFlags_FittingPolicyResizeDown);
      if (Gui::CheckboxFlags("TabBarFlags_FittingPolicyScroll", &tab_bar_flags,
                             TabBarFlags_FittingPolicyScroll))
        tab_bar_flags &=
            ~(TabBarFlags_FittingPolicyMask_ ^ TabBarFlags_FittingPolicyScroll);

      if (Gui::BeginTabBar("MyTabBar", tab_bar_flags)) {
        // Demo a Leading TabItemButton(): click the "?" button to open a menu
        if (show_leading_button)
          if (Gui::TabItemButton("?",
                                 TabItemFlags_Leading | TabItemFlags_NoTooltip))
            Gui::OpenPopup("MyHelpMenu");
        if (Gui::BeginPopup("MyHelpMenu")) {
          Gui::Selectable("Hello!");
          Gui::EndPopup();
        }

        // Demo Trailing Tabs: click the "+" button to add a new tab (in your
        // app you may want to use a font icon instead of the "+") Note that we
        // submit it before the regular tabs, but because of the
        // TabItemFlags_Trailing flag it will always appear at the end.
        if (show_trailing_button)
          if (Gui::TabItemButton("+", TabItemFlags_Trailing |
                                          TabItemFlags_NoTooltip))
            active_tabs.push_back(next_tab_id++); // Add new tab

        // Submit our regular tabs
        for (int n = 0; n < active_tabs.Size;) {
          bool open = true;
          char name[16];
          snprintf(name, ARRAYSIZE(name), "%04d", active_tabs[n]);
          if (Gui::BeginTabItem(name, &open, TabItemFlags_None)) {
            Gui::Text("This is the %s tab!", name);
            Gui::EndTabItem();
          }

          if (!open)
            active_tabs.erase(active_tabs.Data + n);
          else
            n++;
        }

        Gui::EndTabBar();
      }
      Gui::Separator();
      Gui::TreePop();
    }
    Gui::TreePop();
  }

  // Plot/Graph widgets are not very good.
  // Consider using a third-party library such as Plot.
  DEMO_MARKER("Widgets/Plotting");
  if (Gui::TreeNode("Plotting")) {
    static bool animate = true;
    Gui::Checkbox("Animate", &animate);

    // Plot as lines and plot as histogram
    DEMO_MARKER("Widgets/Plotting/PlotLines, PlotHistogram");
    static float arr[] = {0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f};
    Gui::PlotLines("Frame Times", arr, ARRAYSIZE(arr));
    Gui::PlotHistogram("Histogram", arr, ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f,
                       Vec2(0, 80.0f));

    // Fill an array of contiguous float values to plot
    // Tip: If your float aren't contiguous but part of a structure, you can
    // pass a pointer to your first float and the sizeof() of your structure in
    // the "stride" parameter.
    static float values[90] = {};
    static int values_offset = 0;
    static double refresh_time = 0.0;
    if (!animate || refresh_time == 0.0)
      refresh_time = Gui::GetTime();
    while (refresh_time <
           Gui::GetTime()) // Create data at fixed 60 Hz rate for the demo
    {
      static float phase = 0.0f;
      values[values_offset] = cosf(phase);
      values_offset = (values_offset + 1) % ARRAYSIZE(values);
      phase += 0.10f * values_offset;
      refresh_time += 1.0f / 60.0f;
    }

    // Plots can display overlay texts
    // (in this example, we will display an average value)
    {
      float average = 0.0f;
      for (int n = 0; n < ARRAYSIZE(values); n++)
        average += values[n];
      average /= (float)ARRAYSIZE(values);
      char overlay[32];
      sprintf(overlay, "avg %f", average);
      Gui::PlotLines("Lines", values, ARRAYSIZE(values), values_offset, overlay,
                     -1.0f, 1.0f, Vec2(0, 80.0f));
    }

    // Use functions to generate output
    // FIXME: This is actually VERY awkward because current plot API only pass
    // in indices. We probably want an API passing floats and user provide
    // sample rate/count.
    struct Funcs {
      static float Sin(void *, int i) { return sinf(i * 0.1f); }
      static float Saw(void *, int i) { return (i & 1) ? 1.0f : -1.0f; }
    };
    static int func_type = 0, display_count = 70;
    Gui::SeparatorText("Functions");
    Gui::SetNextItemWidth(Gui::GetFontSize() * 8);
    Gui::Combo("func", &func_type, "Sin\0Saw\0");
    Gui::SameLine();
    Gui::SliderInt("Sample count", &display_count, 1, 400);
    float (*func)(void *, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
    Gui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f,
                   Vec2(0, 80));
    Gui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f,
                       1.0f, Vec2(0, 80));
    Gui::Separator();

    // Animate a simple progress bar
    DEMO_MARKER("Widgets/Plotting/ProgressBar");
    static float progress = 0.0f, progress_dir = 1.0f;
    if (animate) {
      progress += progress_dir * 0.4f * Gui::GetIO().DeltaTime;
      if (progress >= +1.1f) {
        progress = +1.1f;
        progress_dir *= -1.0f;
      }
      if (progress <= -0.1f) {
        progress = -0.1f;
        progress_dir *= -1.0f;
      }
    }

    // Typically we would use Vec2(-1.0f,0.0f) or Vec2(-FLT_MIN,0.0f) to use
    // all available width, or Vec2(width,0.0f) for a specified width.
    // Vec2(0.0f,0.0f) uses ItemWidth.
    Gui::ProgressBar(progress, Vec2(0.0f, 0.0f));
    Gui::SameLine(0.0f, Gui::GetStyle().ItemInnerSpacing.x);
    Gui::Text("Progress Bar");

    float progress_saturated = CLAMP(progress, 0.0f, 1.0f);
    char buf[32];
    sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
    Gui::ProgressBar(progress, Vec2(0.f, 0.f), buf);
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Color");
  if (Gui::TreeNode("Color/Picker Widgets")) {
    static Vec4 color = Vec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f,
                             200.0f / 255.0f);

    static bool alpha_preview = true;
    static bool alpha_half_preview = false;
    static bool drag_and_drop = true;
    static bool options_menu = true;
    static bool hdr = false;
    Gui::SeparatorText("Options");
    Gui::Checkbox("With Alpha Preview", &alpha_preview);
    Gui::Checkbox("With Half Alpha Preview", &alpha_half_preview);
    Gui::Checkbox("With Drag and Drop", &drag_and_drop);
    Gui::Checkbox("With Options Menu", &options_menu);
    Gui::SameLine();
    HelpMarker("Right-click on the individual color widget to show options.");
    Gui::Checkbox("With HDR", &hdr);
    Gui::SameLine();
    HelpMarker("Currently all this does is to lift the 0..1 limits on dragging "
               "widgets.");
    ColorEditFlags misc_flags =
        (hdr ? ColorEditFlags_HDR : 0) |
        (drag_and_drop ? 0 : ColorEditFlags_NoDragDrop) |
        (alpha_half_preview
             ? ColorEditFlags_AlphaPreviewHalf
             : (alpha_preview ? ColorEditFlags_AlphaPreview : 0)) |
        (options_menu ? 0 : ColorEditFlags_NoOptions);

    DEMO_MARKER("Widgets/Color/ColorEdit");
    Gui::SeparatorText("Inline color editor");
    Gui::Text("Color widget:");
    Gui::SameLine();
    HelpMarker("Click on the color square to open a color picker.\n"
               "CTRL+click on individual component to input value.\n");
    Gui::ColorEdit3("MyColor##1", (float *)&color, misc_flags);

    DEMO_MARKER("Widgets/Color/ColorEdit (HSV, with Alpha)");
    Gui::Text("Color widget HSV with Alpha:");
    Gui::ColorEdit4("MyColor##2", (float *)&color,
                    ColorEditFlags_DisplayHSV | misc_flags);

    DEMO_MARKER("Widgets/Color/ColorEdit (float display)");
    Gui::Text("Color widget with Float Display:");
    Gui::ColorEdit4("MyColor##2f", (float *)&color,
                    ColorEditFlags_Float | misc_flags);

    DEMO_MARKER("Widgets/Color/ColorButton (with Picker)");
    Gui::Text("Color button with Picker:");
    Gui::SameLine();
    HelpMarker("With the ColorEditFlags_NoInputs flag you can hide all "
               "the slider/text inputs.\n"
               "With the ColorEditFlags_NoLabel flag you can pass a "
               "non-empty label which will only "
               "be used for the tooltip and picker popup.");
    Gui::ColorEdit4("MyColor##3", (float *)&color,
                    ColorEditFlags_NoInputs | ColorEditFlags_NoLabel |
                        misc_flags);

    DEMO_MARKER("Widgets/Color/ColorButton (with custom Picker popup)");
    Gui::Text("Color button with Custom Picker Popup:");

    // Generate a default palette. The palette will persist and can be edited.
    static bool saved_palette_init = true;
    static Vec4 saved_palette[32] = {};
    if (saved_palette_init) {
      for (int n = 0; n < ARRAYSIZE(saved_palette); n++) {
        Gui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f, saved_palette[n].x,
                                  saved_palette[n].y, saved_palette[n].z);
        saved_palette[n].w = 1.0f; // Alpha
      }
      saved_palette_init = false;
    }

    static Vec4 backup_color;
    bool open_popup = Gui::ColorButton("MyColor##3b", color, misc_flags);
    Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
    open_popup |= Gui::Button("Palette");
    if (open_popup) {
      Gui::OpenPopup("mypicker");
      backup_color = color;
    }
    if (Gui::BeginPopup("mypicker")) {
      Gui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
      Gui::Separator();
      Gui::ColorPicker4("##picker", (float *)&color,
                        misc_flags | ColorEditFlags_NoSidePreview |
                            ColorEditFlags_NoSmallPreview);
      Gui::SameLine();

      Gui::BeginGroup(); // Lock X position
      Gui::Text("Current");
      Gui::ColorButton("##current", color,
                       ColorEditFlags_NoPicker |
                           ColorEditFlags_AlphaPreviewHalf,
                       Vec2(60, 40));
      Gui::Text("Previous");
      if (Gui::ColorButton("##previous", backup_color,
                           ColorEditFlags_NoPicker |
                               ColorEditFlags_AlphaPreviewHalf,
                           Vec2(60, 40)))
        color = backup_color;
      Gui::Separator();
      Gui::Text("Palette");
      for (int n = 0; n < ARRAYSIZE(saved_palette); n++) {
        Gui::PushID(n);
        if ((n % 8) != 0)
          Gui::SameLine(0.0f, Gui::GetStyle().ItemSpacing.y);

        ColorEditFlags palette_button_flags = ColorEditFlags_NoAlpha |
                                              ColorEditFlags_NoPicker |
                                              ColorEditFlags_NoTooltip;
        if (Gui::ColorButton("##palette", saved_palette[n],
                             palette_button_flags, Vec2(20, 20)))
          color = Vec4(saved_palette[n].x, saved_palette[n].y,
                       saved_palette[n].z, color.w); // Preserve alpha!

        // Allow user to drop colors into each palette entry. Note that
        // ColorButton() is already a drag source by default, unless specifying
        // the ColorEditFlags_NoDragDrop flag.
        if (Gui::BeginDragDropTarget()) {
          if (const Payload *payload =
                  Gui::AcceptDragDropPayload(PAYLOAD_TYPE_COLOR_3F))
            memcpy((float *)&saved_palette[n], payload->Data,
                   sizeof(float) * 3);
          if (const Payload *payload =
                  Gui::AcceptDragDropPayload(PAYLOAD_TYPE_COLOR_4F))
            memcpy((float *)&saved_palette[n], payload->Data,
                   sizeof(float) * 4);
          Gui::EndDragDropTarget();
        }

        Gui::PopID();
      }
      Gui::EndGroup();
      Gui::EndPopup();
    }

    DEMO_MARKER("Widgets/Color/ColorButton (simple)");
    Gui::Text("Color button only:");
    static bool no_border = false;
    Gui::Checkbox("ColorEditFlags_NoBorder", &no_border);
    Gui::ColorButton("MyColor##3c", *(Vec4 *)&color,
                     misc_flags | (no_border ? ColorEditFlags_NoBorder : 0),
                     Vec2(80, 80));

    DEMO_MARKER("Widgets/Color/ColorPicker");
    Gui::SeparatorText("Color picker");
    static bool alpha = true;
    static bool alpha_bar = true;
    static bool side_preview = true;
    static bool ref_color = false;
    static Vec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
    static int display_mode = 0;
    static int picker_mode = 0;
    Gui::Checkbox("With Alpha", &alpha);
    Gui::Checkbox("With Alpha Bar", &alpha_bar);
    Gui::Checkbox("With Side Preview", &side_preview);
    if (side_preview) {
      Gui::SameLine();
      Gui::Checkbox("With Ref Color", &ref_color);
      if (ref_color) {
        Gui::SameLine();
        Gui::ColorEdit4("##RefColor", &ref_color_v.x,
                        ColorEditFlags_NoInputs | misc_flags);
      }
    }
    Gui::Combo("Display Mode", &display_mode,
               "Auto/Current\0None\0RGB Only\0HSV Only\0Hex Only\0");
    Gui::SameLine();
    HelpMarker("ColorEdit defaults to displaying RGB inputs if you don't "
               "specify a display mode, "
               "but the user can change it with a right-click on those "
               "inputs.\n\nColorPicker defaults to displaying RGB+HSV+Hex "
               "if you don't specify a display mode.\n\nYou can change the "
               "defaults using SetColorEditOptions().");
    Gui::SameLine();
    HelpMarker("When not specified explicitly (Auto/Current mode), user can "
               "right-click the picker to change mode.");
    ColorEditFlags flags = misc_flags;
    if (!alpha)
      flags |= ColorEditFlags_NoAlpha; // This is by default if you call
                                       // ColorPicker3() instead of
                                       // ColorPicker4()
    if (alpha_bar)
      flags |= ColorEditFlags_AlphaBar;
    if (!side_preview)
      flags |= ColorEditFlags_NoSidePreview;
    if (picker_mode == 1)
      flags |= ColorEditFlags_PickerHueBar;
    if (picker_mode == 2)
      flags |= ColorEditFlags_PickerHueWheel;
    if (display_mode == 1)
      flags |= ColorEditFlags_NoInputs; // Disable all RGB/HSV/Hex displays
    if (display_mode == 2)
      flags |= ColorEditFlags_DisplayRGB; // Override display mode
    if (display_mode == 3)
      flags |= ColorEditFlags_DisplayHSV;
    if (display_mode == 4)
      flags |= ColorEditFlags_DisplayHex;
    Gui::ColorPicker4("MyColor##4", (float *)&color, flags,
                      ref_color ? &ref_color_v.x : NULL);

    Gui::Text("Set defaults in code:");
    Gui::SameLine();
    HelpMarker("SetColorEditOptions() is designed to allow you to set "
               "boot-time default.\n"
               "We don't have Push/Pop functions because you can force options "
               "on a per-widget basis if needed,"
               "and the user can change non-forced ones with the options "
               "menu.\nWe don't have a getter to avoid"
               "encouraging you to persistently save values that aren't "
               "forward-compatible.");
    if (Gui::Button("Default: Uint8 + HSV + Hue Bar"))
      Gui::SetColorEditOptions(ColorEditFlags_Uint8 |
                               ColorEditFlags_DisplayHSV |
                               ColorEditFlags_PickerHueBar);
    if (Gui::Button("Default: Float + HDR + Hue Wheel"))
      Gui::SetColorEditOptions(ColorEditFlags_Float | ColorEditFlags_HDR |
                               ColorEditFlags_PickerHueWheel);

    // Always both a small version of both types of pickers (to make it more
    // visible in the demo to people who are skimming quickly through it)
    Gui::Text("Both types:");
    float w = (Gui::GetContentRegionAvail().x - Gui::GetStyle().ItemSpacing.y) *
              0.40f;
    Gui::SetNextItemWidth(w);
    Gui::ColorPicker3("##MyColor##5", (float *)&color,
                      ColorEditFlags_PickerHueBar |
                          ColorEditFlags_NoSidePreview |
                          ColorEditFlags_NoInputs | ColorEditFlags_NoAlpha);
    Gui::SameLine();
    Gui::SetNextItemWidth(w);
    Gui::ColorPicker3("##MyColor##6", (float *)&color,
                      ColorEditFlags_PickerHueWheel |
                          ColorEditFlags_NoSidePreview |
                          ColorEditFlags_NoInputs | ColorEditFlags_NoAlpha);

    // HSV encoded support (to avoid RGB<>HSV round trips and singularities when
    // S==0 or V==0)
    static Vec4 color_hsv(0.23f, 1.0f, 1.0f, 1.0f); // Stored as HSV!
    Gui::Spacing();
    Gui::Text("HSV encoded colors");
    Gui::SameLine();
    HelpMarker("By default, colors are given to ColorEdit and ColorPicker in "
               "RGB, but ColorEditFlags_InputHSV"
               "allows you to store colors as HSV and pass them to ColorEdit "
               "and ColorPicker as HSV. This comes with the"
               "added benefit that you can manipulate hue values with the "
               "picker even when saturation or value are zero.");
    Gui::Text("Color widget with InputHSV:");
    Gui::ColorEdit4("HSV shown as RGB##1", (float *)&color_hsv,
                    ColorEditFlags_DisplayRGB | ColorEditFlags_InputHSV |
                        ColorEditFlags_Float);
    Gui::ColorEdit4("HSV shown as HSV##1", (float *)&color_hsv,
                    ColorEditFlags_DisplayHSV | ColorEditFlags_InputHSV |
                        ColorEditFlags_Float);
    Gui::DragFloat4("Raw HSV values", (float *)&color_hsv, 0.01f, 0.0f, 1.0f);

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Drag and Slider Flags");
  if (Gui::TreeNode("Drag/Slider Flags")) {
    // Demonstrate using advanced flags for DragXXX and SliderXXX functions.
    // Note that the flags are the same!
    static SliderFlags flags = SliderFlags_None;
    Gui::CheckboxFlags("SliderFlags_AlwaysClamp", &flags,
                       SliderFlags_AlwaysClamp);
    Gui::SameLine();
    HelpMarker("Always clamp value to min/max bounds (if any) when input "
               "manually with CTRL+Click.");
    Gui::CheckboxFlags("SliderFlags_Logarithmic", &flags,
                       SliderFlags_Logarithmic);
    Gui::SameLine();
    HelpMarker("Enable logarithmic editing (more precision for small values).");
    Gui::CheckboxFlags("SliderFlags_NoRoundToFormat", &flags,
                       SliderFlags_NoRoundToFormat);
    Gui::SameLine();
    HelpMarker(
        "Disable rounding underlying value to match precision of the format "
        "string (e.g. %.3f values are rounded to those 3 digits).");
    Gui::CheckboxFlags("SliderFlags_NoInput", &flags, SliderFlags_NoInput);
    Gui::SameLine();
    HelpMarker("Disable CTRL+Click or Enter key allowing to input text "
               "directly into the widget.");

    // Drags
    static float drag_f = 0.5f;
    static int drag_i = 50;
    Gui::Text("Underlying float value: %f", drag_f);
    Gui::DragFloat("DragFloat (0 -> 1)", &drag_f, 0.005f, 0.0f, 1.0f, "%.3f",
                   flags);
    Gui::DragFloat("DragFloat (0 -> +inf)", &drag_f, 0.005f, 0.0f, FLT_MAX,
                   "%.3f", flags);
    Gui::DragFloat("DragFloat (-inf -> 1)", &drag_f, 0.005f, -FLT_MAX, 1.0f,
                   "%.3f", flags);
    Gui::DragFloat("DragFloat (-inf -> +inf)", &drag_f, 0.005f, -FLT_MAX,
                   +FLT_MAX, "%.3f", flags);
    Gui::DragInt("DragInt (0 -> 100)", &drag_i, 0.5f, 0, 100, "%d", flags);

    // Sliders
    static float slider_f = 0.5f;
    static int slider_i = 50;
    Gui::Text("Underlying float value: %f", slider_f);
    Gui::SliderFloat("SliderFloat (0 -> 1)", &slider_f, 0.0f, 1.0f, "%.3f",
                     flags);
    Gui::SliderInt("SliderInt (0 -> 100)", &slider_i, 0, 100, "%d", flags);

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Range Widgets");
  if (Gui::TreeNode("Range Widgets")) {
    static float begin = 10, end = 90;
    static int begin_i = 100, end_i = 1000;
    Gui::DragFloatRange2("range float", &begin, &end, 0.25f, 0.0f, 100.0f,
                         "Min: %.1f %%", "Max: %.1f %%",
                         SliderFlags_AlwaysClamp);
    Gui::DragIntRange2("range int", &begin_i, &end_i, 5, 0, 1000,
                       "Min: %d units", "Max: %d units");
    Gui::DragIntRange2("range int (no bounds)", &begin_i, &end_i, 5, 0, 0,
                       "Min: %d units", "Max: %d units");
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Data Types");
  if (Gui::TreeNode("Data Types")) {
// DragScalar/InputScalar/SliderScalar functions allow various data types
// - signed/unsigned
// - 8/16/32/64-bits
// - integer/float/double
// To avoid polluting the public API with all possible combinations, we use the
// DataType enum to pass the type, and passing all arguments by pointer.
// This is the reason the test code below creates local variables to hold "zero"
// "one" etc. for each type. In practice, if you frequently use a given type
// that is not covered by the normal API entry points, you can wrap it yourself
// inside a 1 line function which can take typed argument as value instead of
// void*, and then pass their address to the generic function. For example:
//   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0,
//   const char* format = "%lld")
//   {
//      return SliderScalar(label, DataType_U64, value, &min, &max,
//      format);
//   }

// Setup limits (as helper variables so we can take their address, as explained
// above) Note: SliderScalar() functions have a maximum usable range of half the
// natural type maximum, hence the /2.
#ifndef LLONG_MIN
    S64 LLONG_MIN = -9223372036854775807LL - 1;
    S64 LLONG_MAX = 9223372036854775807LL;
    U64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
#endif
    const char s8_zero = 0, s8_one = 1, s8_fifty = 50, s8_min = -128,
               s8_max = 127;
    const U8 u8_zero = 0, u8_one = 1, u8_fifty = 50, u8_min = 0, u8_max = 255;
    const short s16_zero = 0, s16_one = 1, s16_fifty = 50, s16_min = -32768,
                s16_max = 32767;
    const U16 u16_zero = 0, u16_one = 1, u16_fifty = 50, u16_min = 0,
              u16_max = 65535;
    const S32 s32_zero = 0, s32_one = 1, s32_fifty = 50, s32_min = INT_MIN / 2,
              s32_max = INT_MAX / 2, s32_hi_a = INT_MAX / 2 - 100,
              s32_hi_b = INT_MAX / 2;
    const U32 u32_zero = 0, u32_one = 1, u32_fifty = 50, u32_min = 0,
              u32_max = UINT_MAX / 2, u32_hi_a = UINT_MAX / 2 - 100,
              u32_hi_b = UINT_MAX / 2;
    const S64 s64_zero = 0, s64_one = 1, s64_fifty = 50,
              s64_min = LLONG_MIN / 2, s64_max = LLONG_MAX / 2,
              s64_hi_a = LLONG_MAX / 2 - 100, s64_hi_b = LLONG_MAX / 2;
    const U64 u64_zero = 0, u64_one = 1, u64_fifty = 50, u64_min = 0,
              u64_max = ULLONG_MAX / 2, u64_hi_a = ULLONG_MAX / 2 - 100,
              u64_hi_b = ULLONG_MAX / 2;
    const float f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f,
                f32_hi_a = +10000000000.0f;
    const double f64_zero = 0., f64_one = 1., f64_lo_a = -1000000000000000.0,
                 f64_hi_a = +1000000000000000.0;

    // State
    static char s8_v = 127;
    static U8 u8_v = 255;
    static short s16_v = 32767;
    static U16 u16_v = 65535;
    static S32 s32_v = -1;
    static U32 u32_v = (U32)-1;
    static S64 s64_v = -1;
    static U64 u64_v = (U64)-1;
    static float f32_v = 0.123f;
    static double f64_v = 90000.01234567890123456789;

    const float drag_speed = 0.2f;
    static bool drag_clamp = false;
    DEMO_MARKER("Widgets/Data Types/Drags");
    Gui::SeparatorText("Drags");
    Gui::Checkbox("Clamp integers to 0..50", &drag_clamp);
    Gui::SameLine();
    HelpMarker("As with every widget in gui, we never modify values "
               "unless there is a user interaction.\n"
               "You can override the clamping limits by using CTRL+Click to "
               "input a value.");
    Gui::DragScalar("drag s8", DataType_S8, &s8_v, drag_speed,
                    drag_clamp ? &s8_zero : NULL,
                    drag_clamp ? &s8_fifty : NULL);
    Gui::DragScalar("drag u8", DataType_U8, &u8_v, drag_speed,
                    drag_clamp ? &u8_zero : NULL, drag_clamp ? &u8_fifty : NULL,
                    "%u ms");
    Gui::DragScalar("drag s16", DataType_S16, &s16_v, drag_speed,
                    drag_clamp ? &s16_zero : NULL,
                    drag_clamp ? &s16_fifty : NULL);
    Gui::DragScalar("drag u16", DataType_U16, &u16_v, drag_speed,
                    drag_clamp ? &u16_zero : NULL,
                    drag_clamp ? &u16_fifty : NULL, "%u ms");
    Gui::DragScalar("drag s32", DataType_S32, &s32_v, drag_speed,
                    drag_clamp ? &s32_zero : NULL,
                    drag_clamp ? &s32_fifty : NULL);
    Gui::DragScalar("drag s32 hex", DataType_S32, &s32_v, drag_speed,
                    drag_clamp ? &s32_zero : NULL,
                    drag_clamp ? &s32_fifty : NULL, "0x%08X");
    Gui::DragScalar("drag u32", DataType_U32, &u32_v, drag_speed,
                    drag_clamp ? &u32_zero : NULL,
                    drag_clamp ? &u32_fifty : NULL, "%u ms");
    Gui::DragScalar("drag s64", DataType_S64, &s64_v, drag_speed,
                    drag_clamp ? &s64_zero : NULL,
                    drag_clamp ? &s64_fifty : NULL);
    Gui::DragScalar("drag u64", DataType_U64, &u64_v, drag_speed,
                    drag_clamp ? &u64_zero : NULL,
                    drag_clamp ? &u64_fifty : NULL);
    Gui::DragScalar("drag float", DataType_Float, &f32_v, 0.005f, &f32_zero,
                    &f32_one, "%f");
    Gui::DragScalar("drag float log", DataType_Float, &f32_v, 0.005f, &f32_zero,
                    &f32_one, "%f", SliderFlags_Logarithmic);
    Gui::DragScalar("drag double", DataType_Double, &f64_v, 0.0005f, &f64_zero,
                    NULL, "%.10f grams");
    Gui::DragScalar("drag double log", DataType_Double, &f64_v, 0.0005f,
                    &f64_zero, &f64_one, "0 < %.10f < 1",
                    SliderFlags_Logarithmic);

    DEMO_MARKER("Widgets/Data Types/Sliders");
    Gui::SeparatorText("Sliders");
    Gui::SliderScalar("slider s8 full", DataType_S8, &s8_v, &s8_min, &s8_max,
                      "%d");
    Gui::SliderScalar("slider u8 full", DataType_U8, &u8_v, &u8_min, &u8_max,
                      "%u");
    Gui::SliderScalar("slider s16 full", DataType_S16, &s16_v, &s16_min,
                      &s16_max, "%d");
    Gui::SliderScalar("slider u16 full", DataType_U16, &u16_v, &u16_min,
                      &u16_max, "%u");
    Gui::SliderScalar("slider s32 low", DataType_S32, &s32_v, &s32_zero,
                      &s32_fifty, "%d");
    Gui::SliderScalar("slider s32 high", DataType_S32, &s32_v, &s32_hi_a,
                      &s32_hi_b, "%d");
    Gui::SliderScalar("slider s32 full", DataType_S32, &s32_v, &s32_min,
                      &s32_max, "%d");
    Gui::SliderScalar("slider s32 hex", DataType_S32, &s32_v, &s32_zero,
                      &s32_fifty, "0x%04X");
    Gui::SliderScalar("slider u32 low", DataType_U32, &u32_v, &u32_zero,
                      &u32_fifty, "%u");
    Gui::SliderScalar("slider u32 high", DataType_U32, &u32_v, &u32_hi_a,
                      &u32_hi_b, "%u");
    Gui::SliderScalar("slider u32 full", DataType_U32, &u32_v, &u32_min,
                      &u32_max, "%u");
    Gui::SliderScalar("slider s64 low", DataType_S64, &s64_v, &s64_zero,
                      &s64_fifty, "%" PRId64);
    Gui::SliderScalar("slider s64 high", DataType_S64, &s64_v, &s64_hi_a,
                      &s64_hi_b, "%" PRId64);
    Gui::SliderScalar("slider s64 full", DataType_S64, &s64_v, &s64_min,
                      &s64_max, "%" PRId64);
    Gui::SliderScalar("slider u64 low", DataType_U64, &u64_v, &u64_zero,
                      &u64_fifty, "%" PRIu64 " ms");
    Gui::SliderScalar("slider u64 high", DataType_U64, &u64_v, &u64_hi_a,
                      &u64_hi_b, "%" PRIu64 " ms");
    Gui::SliderScalar("slider u64 full", DataType_U64, &u64_v, &u64_min,
                      &u64_max, "%" PRIu64 " ms");
    Gui::SliderScalar("slider float low", DataType_Float, &f32_v, &f32_zero,
                      &f32_one);
    Gui::SliderScalar("slider float low log", DataType_Float, &f32_v, &f32_zero,
                      &f32_one, "%.10f", SliderFlags_Logarithmic);
    Gui::SliderScalar("slider float high", DataType_Float, &f32_v, &f32_lo_a,
                      &f32_hi_a, "%e");
    Gui::SliderScalar("slider double low", DataType_Double, &f64_v, &f64_zero,
                      &f64_one, "%.10f grams");
    Gui::SliderScalar("slider double low log", DataType_Double, &f64_v,
                      &f64_zero, &f64_one, "%.10f", SliderFlags_Logarithmic);
    Gui::SliderScalar("slider double high", DataType_Double, &f64_v, &f64_lo_a,
                      &f64_hi_a, "%e grams");

    Gui::SeparatorText("Sliders (reverse)");
    Gui::SliderScalar("slider s8 reverse", DataType_S8, &s8_v, &s8_max, &s8_min,
                      "%d");
    Gui::SliderScalar("slider u8 reverse", DataType_U8, &u8_v, &u8_max, &u8_min,
                      "%u");
    Gui::SliderScalar("slider s32 reverse", DataType_S32, &s32_v, &s32_fifty,
                      &s32_zero, "%d");
    Gui::SliderScalar("slider u32 reverse", DataType_U32, &u32_v, &u32_fifty,
                      &u32_zero, "%u");
    Gui::SliderScalar("slider s64 reverse", DataType_S64, &s64_v, &s64_fifty,
                      &s64_zero, "%" PRId64);
    Gui::SliderScalar("slider u64 reverse", DataType_U64, &u64_v, &u64_fifty,
                      &u64_zero, "%" PRIu64 " ms");

    DEMO_MARKER("Widgets/Data Types/Inputs");
    static bool inputs_step = true;
    Gui::SeparatorText("Inputs");
    Gui::Checkbox("Show step buttons", &inputs_step);
    Gui::InputScalar("input s8", DataType_S8, &s8_v,
                     inputs_step ? &s8_one : NULL, NULL, "%d");
    Gui::InputScalar("input u8", DataType_U8, &u8_v,
                     inputs_step ? &u8_one : NULL, NULL, "%u");
    Gui::InputScalar("input s16", DataType_S16, &s16_v,
                     inputs_step ? &s16_one : NULL, NULL, "%d");
    Gui::InputScalar("input u16", DataType_U16, &u16_v,
                     inputs_step ? &u16_one : NULL, NULL, "%u");
    Gui::InputScalar("input s32", DataType_S32, &s32_v,
                     inputs_step ? &s32_one : NULL, NULL, "%d");
    Gui::InputScalar("input s32 hex", DataType_S32, &s32_v,
                     inputs_step ? &s32_one : NULL, NULL, "%04X");
    Gui::InputScalar("input u32", DataType_U32, &u32_v,
                     inputs_step ? &u32_one : NULL, NULL, "%u");
    Gui::InputScalar("input u32 hex", DataType_U32, &u32_v,
                     inputs_step ? &u32_one : NULL, NULL, "%08X");
    Gui::InputScalar("input s64", DataType_S64, &s64_v,
                     inputs_step ? &s64_one : NULL);
    Gui::InputScalar("input u64", DataType_U64, &u64_v,
                     inputs_step ? &u64_one : NULL);
    Gui::InputScalar("input float", DataType_Float, &f32_v,
                     inputs_step ? &f32_one : NULL);
    Gui::InputScalar("input double", DataType_Double, &f64_v,
                     inputs_step ? &f64_one : NULL);

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Multi-component Widgets");
  if (Gui::TreeNode("Multi-component Widgets")) {
    static float vec4f[4] = {0.10f, 0.20f, 0.30f, 0.44f};
    static int vec4i[4] = {1, 5, 100, 255};

    Gui::SeparatorText("2-wide");
    Gui::InputFloat2("input float2", vec4f);
    Gui::DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f);
    Gui::SliderFloat2("slider float2", vec4f, 0.0f, 1.0f);
    Gui::InputInt2("input int2", vec4i);
    Gui::DragInt2("drag int2", vec4i, 1, 0, 255);
    Gui::SliderInt2("slider int2", vec4i, 0, 255);

    Gui::SeparatorText("3-wide");
    Gui::InputFloat3("input float3", vec4f);
    Gui::DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f);
    Gui::SliderFloat3("slider float3", vec4f, 0.0f, 1.0f);
    Gui::InputInt3("input int3", vec4i);
    Gui::DragInt3("drag int3", vec4i, 1, 0, 255);
    Gui::SliderInt3("slider int3", vec4i, 0, 255);

    Gui::SeparatorText("4-wide");
    Gui::InputFloat4("input float4", vec4f);
    Gui::DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f);
    Gui::SliderFloat4("slider float4", vec4f, 0.0f, 1.0f);
    Gui::InputInt4("input int4", vec4i);
    Gui::DragInt4("drag int4", vec4i, 1, 0, 255);
    Gui::SliderInt4("slider int4", vec4i, 0, 255);

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Vertical Sliders");
  if (Gui::TreeNode("Vertical Sliders")) {
    const float spacing = 4;
    Gui::PushStyleVar(StyleVar_ItemSpacing, Vec2(spacing, spacing));

    static int int_value = 0;
    Gui::VSliderInt("##int", Vec2(18, 160), &int_value, 0, 5);
    Gui::SameLine();

    static float values[7] = {0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f};
    Gui::PushID("set1");
    for (int i = 0; i < 7; i++) {
      if (i > 0)
        Gui::SameLine();
      Gui::PushID(i);
      Gui::PushStyleColor(Col_FrameBg, (Vec4)Color::HSV(i / 7.0f, 0.5f, 0.5f));
      Gui::PushStyleColor(Col_FrameBgHovered,
                          (Vec4)Color::HSV(i / 7.0f, 0.6f, 0.5f));
      Gui::PushStyleColor(Col_FrameBgActive,
                          (Vec4)Color::HSV(i / 7.0f, 0.7f, 0.5f));
      Gui::PushStyleColor(Col_SliderGrab,
                          (Vec4)Color::HSV(i / 7.0f, 0.9f, 0.9f));
      Gui::VSliderFloat("##v", Vec2(18, 160), &values[i], 0.0f, 1.0f, "");
      if (Gui::IsItemActive() || Gui::IsItemHovered())
        Gui::SetTooltip("%.3f", values[i]);
      Gui::PopStyleColor(4);
      Gui::PopID();
    }
    Gui::PopID();

    Gui::SameLine();
    Gui::PushID("set2");
    static float values2[4] = {0.20f, 0.80f, 0.40f, 0.25f};
    const int rows = 3;
    const Vec2 small_slider_size(
        18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
    for (int nx = 0; nx < 4; nx++) {
      if (nx > 0)
        Gui::SameLine();
      Gui::BeginGroup();
      for (int ny = 0; ny < rows; ny++) {
        Gui::PushID(nx * rows + ny);
        Gui::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f,
                          "");
        if (Gui::IsItemActive() || Gui::IsItemHovered())
          Gui::SetTooltip("%.3f", values2[nx]);
        Gui::PopID();
      }
      Gui::EndGroup();
    }
    Gui::PopID();

    Gui::SameLine();
    Gui::PushID("set3");
    for (int i = 0; i < 4; i++) {
      if (i > 0)
        Gui::SameLine();
      Gui::PushID(i);
      Gui::PushStyleVar(StyleVar_GrabMinSize, 40);
      Gui::VSliderFloat("##v", Vec2(40, 160), &values[i], 0.0f, 1.0f,
                        "%.2f\nsec");
      Gui::PopStyleVar();
      Gui::PopID();
    }
    Gui::PopID();
    Gui::PopStyleVar();
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Drag and drop");
  if (Gui::TreeNode("Drag and Drop")) {
    DEMO_MARKER("Widgets/Drag and drop/Standard widgets");
    if (Gui::TreeNode("Drag and drop in standard widgets")) {
      // ColorEdit widgets automatically act as drag source and drag target.
      // They are using standardized payload strings PAYLOAD_TYPE_COLOR_3F and
      // PAYLOAD_TYPE_COLOR_4F to allow your own widgets to use colors in their
      // drag and drop interaction. Also see 'Demo->Widgets->Color/Picker
      // Widgets->Palette' demo.
      HelpMarker("You can drag from the color squares.");
      static float col1[3] = {1.0f, 0.0f, 0.2f};
      static float col2[4] = {0.4f, 0.7f, 0.0f, 0.5f};
      Gui::ColorEdit3("color 1", col1);
      Gui::ColorEdit4("color 2", col2);
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Drag and drop/Copy-swap items");
    if (Gui::TreeNode("Drag and drop to copy/swap items")) {
      enum Mode { Mode_Copy, Mode_Move, Mode_Swap };
      static int mode = 0;
      if (Gui::RadioButton("Copy", mode == Mode_Copy)) {
        mode = Mode_Copy;
      }
      Gui::SameLine();
      if (Gui::RadioButton("Move", mode == Mode_Move)) {
        mode = Mode_Move;
      }
      Gui::SameLine();
      if (Gui::RadioButton("Swap", mode == Mode_Swap)) {
        mode = Mode_Swap;
      }
      static const char *names[9] = {"Bobby",   "Beatrice", "Betty",
                                     "Brianna", "Barry",    "Bernard",
                                     "Bibi",    "Blaine",   "Bryn"};
      for (int n = 0; n < ARRAYSIZE(names); n++) {
        Gui::PushID(n);
        if ((n % 3) != 0)
          Gui::SameLine();
        Gui::Button(names[n], Vec2(60, 60));

        // Our buttons are both drag sources and drag targets here!
        if (Gui::BeginDragDropSource(DragDropFlags_None)) {
          // Set payload to carry the index of our item (could be anything)
          Gui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));

          // Display preview (could be anything, e.g. when dragging an image we
          // could decide to display the filename and a small preview of the
          // image, etc.)
          if (mode == Mode_Copy) {
            Gui::Text("Copy %s", names[n]);
          }
          if (mode == Mode_Move) {
            Gui::Text("Move %s", names[n]);
          }
          if (mode == Mode_Swap) {
            Gui::Text("Swap %s", names[n]);
          }
          Gui::EndDragDropSource();
        }
        if (Gui::BeginDragDropTarget()) {
          if (const Payload *payload =
                  Gui::AcceptDragDropPayload("DND_DEMO_CELL")) {
            ASSERT(payload->DataSize == sizeof(int));
            int payload_n = *(const int *)payload->Data;
            if (mode == Mode_Copy) {
              names[n] = names[payload_n];
            }
            if (mode == Mode_Move) {
              names[n] = names[payload_n];
              names[payload_n] = "";
            }
            if (mode == Mode_Swap) {
              const char *tmp = names[n];
              names[n] = names[payload_n];
              names[payload_n] = tmp;
            }
          }
          Gui::EndDragDropTarget();
        }
        Gui::PopID();
      }
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Drag and Drop/Drag to reorder items (simple)");
    if (Gui::TreeNode("Drag to reorder items (simple)")) {
      // Simple reordering
      HelpMarker("We don't use the drag and drop api at all here! "
                 "Instead we query when the item is held but not hovered, and "
                 "order items accordingly.");
      static const char *item_names[] = {"Item One", "Item Two", "Item Three",
                                         "Item Four", "Item Five"};
      for (int n = 0; n < ARRAYSIZE(item_names); n++) {
        const char *item = item_names[n];
        Gui::Selectable(item);

        if (Gui::IsItemActive() && !Gui::IsItemHovered()) {
          int n_next = n + (Gui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
          if (n_next >= 0 && n_next < ARRAYSIZE(item_names)) {
            item_names[n] = item_names[n_next];
            item_names[n_next] = item;
            Gui::ResetMouseDragDelta();
          }
        }
      }
      Gui::TreePop();
    }

    DEMO_MARKER("Widgets/Drag and Drop/Tooltip at target location");
    if (Gui::TreeNode("Tooltip at target location")) {
      for (int n = 0; n < 2; n++) {
        // Drop targets
        Gui::Button(n ? "drop here##1" : "drop here##0");
        if (Gui::BeginDragDropTarget()) {
          DragDropFlags drop_target_flags =
              DragDropFlags_AcceptBeforeDelivery |
              DragDropFlags_AcceptNoPreviewTooltip;
          if (const Payload *payload = Gui::AcceptDragDropPayload(
                  PAYLOAD_TYPE_COLOR_4F, drop_target_flags)) {
            UNUSED(payload);
            Gui::SetMouseCursor(MouseCursor_NotAllowed);
            Gui::BeginTooltip();
            Gui::Text("Cannot drop here!");
            Gui::EndTooltip();
          }
          Gui::EndDragDropTarget();
        }

        // Drop source
        static Vec4 col4 = {1.0f, 0.0f, 0.2f, 1.0f};
        if (n == 0)
          Gui::ColorButton("drag me", col4);
      }
      Gui::TreePop();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Querying Item Status (Edited,Active,Hovered etc.)");
  if (Gui::TreeNode("Querying Item Status (Edited/Active/Hovered etc.)")) {
    // Select an item type
    const char *item_names[] = {"Text",
                                "Button",
                                "Button (w/ repeat)",
                                "Checkbox",
                                "SliderFloat",
                                "InputText",
                                "InputTextMultiline",
                                "InputFloat",
                                "InputFloat3",
                                "ColorEdit4",
                                "Selectable",
                                "MenuItem",
                                "TreeNode",
                                "TreeNode (w/ double-click)",
                                "Combo",
                                "ListBox"};
    static int item_type = 4;
    static bool item_disabled = false;
    Gui::Combo("Item Type", &item_type, item_names, ARRAYSIZE(item_names),
               ARRAYSIZE(item_names));
    Gui::SameLine();
    HelpMarker(
        "Testing how various types of items are interacting with the IsItemXXX "
        "functions. Note that the bool return value of most Gui function is "
        "generally equivalent to calling Gui::IsItemHovered().");
    Gui::Checkbox("Item Disabled", &item_disabled);

    // Submit selected items so we can query their status in the code following
    // it.
    bool ret = false;
    static bool b = false;
    static float col4f[4] = {1.0f, 0.5, 0.0f, 1.0f};
    static char str[16] = {};
    if (item_disabled)
      Gui::BeginDisabled(true);
    if (item_type == 0) {
      Gui::Text("ITEM: Text");
    } // Testing text items with no identifier/interaction
    if (item_type == 1) {
      ret = Gui::Button("ITEM: Button");
    } // Testing button
    if (item_type == 2) {
      Gui::PushButtonRepeat(true);
      ret = Gui::Button("ITEM: Button");
      Gui::PopButtonRepeat();
    } // Testing button (with repeater)
    if (item_type == 3) {
      ret = Gui::Checkbox("ITEM: Checkbox", &b);
    } // Testing checkbox
    if (item_type == 4) {
      ret = Gui::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f);
    } // Testing basic item
    if (item_type == 5) {
      ret = Gui::InputText("ITEM: InputText", &str[0], ARRAYSIZE(str));
    } // Testing input text (which handles tabbing)
    if (item_type == 6) {
      ret = Gui::InputTextMultiline("ITEM: InputTextMultiline", &str[0],
                                    ARRAYSIZE(str));
    } // Testing input text (which uses a child window)
    if (item_type == 7) {
      ret = Gui::InputFloat("ITEM: InputFloat", col4f, 1.0f);
    } // Testing +/- buttons on scalar input
    if (item_type == 8) {
      ret = Gui::InputFloat3("ITEM: InputFloat3", col4f);
    } // Testing multi-component items (IsItemXXX flags are reported merged)
    if (item_type == 9) {
      ret = Gui::ColorEdit4("ITEM: ColorEdit4", col4f);
    } // Testing multi-component items (IsItemXXX flags are reported merged)
    if (item_type == 10) {
      ret = Gui::Selectable("ITEM: Selectable");
    } // Testing selectable item
    if (item_type == 11) {
      ret = Gui::MenuItem("ITEM: MenuItem");
    } // Testing menu item (they use ButtonFlags_PressedOnRelease button
      // policy)
    if (item_type == 12) {
      ret = Gui::TreeNode("ITEM: TreeNode");
      if (ret)
        Gui::TreePop();
    } // Testing tree node
    if (item_type == 13) {
      ret = Gui::TreeNodeEx("ITEM: TreeNode w/ TreeNodeFlags_OpenOnDoubleClick",
                            TreeNodeFlags_OpenOnDoubleClick |
                                TreeNodeFlags_NoTreePushOnOpen);
    } // Testing tree node with ButtonFlags_PressedOnDoubleClick button
      // policy.
    if (item_type == 14) {
      const char *items[] = {"Apple", "Banana", "Cherry", "Kiwi"};
      static int current = 1;
      ret = Gui::Combo("ITEM: Combo", &current, items, ARRAYSIZE(items));
    }
    if (item_type == 15) {
      const char *items[] = {"Apple", "Banana", "Cherry", "Kiwi"};
      static int current = 1;
      ret = Gui::ListBox("ITEM: ListBox", &current, items, ARRAYSIZE(items),
                         ARRAYSIZE(items));
    }

    bool hovered_delay_none = Gui::IsItemHovered();
    bool hovered_delay_stationary = Gui::IsItemHovered(HoveredFlags_Stationary);
    bool hovered_delay_short = Gui::IsItemHovered(HoveredFlags_DelayShort);
    bool hovered_delay_normal = Gui::IsItemHovered(HoveredFlags_DelayNormal);
    bool hovered_delay_tooltip =
        Gui::IsItemHovered(HoveredFlags_ForTooltip); // = Normal + Stationary

    // Display the values of IsItemHovered() and other common item state
    // functions. Note that the HoveredFlags_XXX flags can be combined.
    // Because BulletText is an item itself and that would affect the output of
    // IsItemXXX functions, we query every state in a single call to avoid
    // storing them and to simplify the code.
    Gui::BulletText(
        "Return value = %d\n"
        "IsItemFocused() = %d\n"
        "IsItemHovered() = %d\n"
        "IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
        "IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
        "IsItemHovered(_AllowWhenOverlappedByItem) = %d\n"
        "IsItemHovered(_AllowWhenOverlappedByWindow) = %d\n"
        "IsItemHovered(_AllowWhenDisabled) = %d\n"
        "IsItemHovered(_RectOnly) = %d\n"
        "IsItemActive() = %d\n"
        "IsItemEdited() = %d\n"
        "IsItemActivated() = %d\n"
        "IsItemDeactivated() = %d\n"
        "IsItemDeactivatedAfterEdit() = %d\n"
        "IsItemVisible() = %d\n"
        "IsItemClicked() = %d\n"
        "IsItemToggledOpen() = %d\n"
        "GetItemRectMin() = (%.1f, %.1f)\n"
        "GetItemRectMax() = (%.1f, %.1f)\n"
        "GetItemRectSize() = (%.1f, %.1f)",
        ret, Gui::IsItemFocused(), Gui::IsItemHovered(),
        Gui::IsItemHovered(HoveredFlags_AllowWhenBlockedByPopup),
        Gui::IsItemHovered(HoveredFlags_AllowWhenBlockedByActiveItem),
        Gui::IsItemHovered(HoveredFlags_AllowWhenOverlappedByItem),
        Gui::IsItemHovered(HoveredFlags_AllowWhenOverlappedByWindow),
        Gui::IsItemHovered(HoveredFlags_AllowWhenDisabled),
        Gui::IsItemHovered(HoveredFlags_RectOnly), Gui::IsItemActive(),
        Gui::IsItemEdited(), Gui::IsItemActivated(), Gui::IsItemDeactivated(),
        Gui::IsItemDeactivatedAfterEdit(), Gui::IsItemVisible(),
        Gui::IsItemClicked(), Gui::IsItemToggledOpen(), Gui::GetItemRectMin().x,
        Gui::GetItemRectMin().y, Gui::GetItemRectMax().x,
        Gui::GetItemRectMax().y, Gui::GetItemRectSize().x,
        Gui::GetItemRectSize().y);
    Gui::BulletText("with Hovering Delay or Stationary test:\n"
                    "IsItemHovered() = = %d\n"
                    "IsItemHovered(_Stationary) = %d\n"
                    "IsItemHovered(_DelayShort) = %d\n"
                    "IsItemHovered(_DelayNormal) = %d\n"
                    "IsItemHovered(_Tooltip) = %d",
                    hovered_delay_none, hovered_delay_stationary,
                    hovered_delay_short, hovered_delay_normal,
                    hovered_delay_tooltip);

    if (item_disabled)
      Gui::EndDisabled();

    char buf[1] = "";
    Gui::InputText("unused", buf, ARRAYSIZE(buf), InputTextFlags_ReadOnly);
    Gui::SameLine();
    HelpMarker("This widget is only here to be able to tab-out of the widgets "
               "above and see e.g. Deactivated() status.");

    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Querying Window Status (Focused,Hovered etc.)");
  if (Gui::TreeNode("Querying Window Status (Focused/Hovered etc.)")) {
    static bool embed_all_inside_a_child_window = false;
    Gui::Checkbox(
        "Embed everything inside a child window for testing _RootWindow flag.",
        &embed_all_inside_a_child_window);
    if (embed_all_inside_a_child_window)
      Gui::BeginChild("outer_child", Vec2(0, Gui::GetFontSize() * 20.0f),
                      ChildFlags_Border);

    // Testing IsWindowFocused() function with its various flags.
    Gui::BulletText(
        "IsWindowFocused() = %d\n"
        "IsWindowFocused(_ChildWindows) = %d\n"
        "IsWindowFocused(_ChildWindows|_NoPopupHierarchy) = %d\n"
        "IsWindowFocused(_ChildWindows|_DockHierarchy) = %d\n"
        "IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
        "IsWindowFocused(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %d\n"
        "IsWindowFocused(_ChildWindows|_RootWindow|_DockHierarchy) = %d\n"
        "IsWindowFocused(_RootWindow) = %d\n"
        "IsWindowFocused(_RootWindow|_NoPopupHierarchy) = %d\n"
        "IsWindowFocused(_RootWindow|_DockHierarchy) = %d\n"
        "IsWindowFocused(_AnyWindow) = %d\n",
        Gui::IsWindowFocused(), Gui::IsWindowFocused(FocusedFlags_ChildWindows),
        Gui::IsWindowFocused(FocusedFlags_ChildWindows |
                             FocusedFlags_NoPopupHierarchy),
        Gui::IsWindowFocused(FocusedFlags_ChildWindows |
                             FocusedFlags_DockHierarchy),
        Gui::IsWindowFocused(FocusedFlags_ChildWindows |
                             FocusedFlags_RootWindow),
        Gui::IsWindowFocused(FocusedFlags_ChildWindows |
                             FocusedFlags_RootWindow |
                             FocusedFlags_NoPopupHierarchy),
        Gui::IsWindowFocused(FocusedFlags_ChildWindows |
                             FocusedFlags_RootWindow |
                             FocusedFlags_DockHierarchy),
        Gui::IsWindowFocused(FocusedFlags_RootWindow),
        Gui::IsWindowFocused(FocusedFlags_RootWindow |
                             FocusedFlags_NoPopupHierarchy),
        Gui::IsWindowFocused(FocusedFlags_RootWindow |
                             FocusedFlags_DockHierarchy),
        Gui::IsWindowFocused(FocusedFlags_AnyWindow));

    // Testing IsWindowHovered() function with its various flags.
    Gui::BulletText(
        "IsWindowHovered() = %d\n"
        "IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
        "IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
        "IsWindowHovered(_ChildWindows) = %d\n"
        "IsWindowHovered(_ChildWindows|_NoPopupHierarchy) = %d\n"
        "IsWindowHovered(_ChildWindows|_DockHierarchy) = %d\n"
        "IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
        "IsWindowHovered(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %d\n"
        "IsWindowHovered(_ChildWindows|_RootWindow|_DockHierarchy) = %d\n"
        "IsWindowHovered(_RootWindow) = %d\n"
        "IsWindowHovered(_RootWindow|_NoPopupHierarchy) = %d\n"
        "IsWindowHovered(_RootWindow|_DockHierarchy) = %d\n"
        "IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %d\n"
        "IsWindowHovered(_AnyWindow) = %d\n"
        "IsWindowHovered(_Stationary) = %d\n",
        Gui::IsWindowHovered(),
        Gui::IsWindowHovered(HoveredFlags_AllowWhenBlockedByPopup),
        Gui::IsWindowHovered(HoveredFlags_AllowWhenBlockedByActiveItem),
        Gui::IsWindowHovered(HoveredFlags_ChildWindows),
        Gui::IsWindowHovered(HoveredFlags_ChildWindows |
                             HoveredFlags_NoPopupHierarchy),
        Gui::IsWindowHovered(HoveredFlags_ChildWindows |
                             HoveredFlags_DockHierarchy),
        Gui::IsWindowHovered(HoveredFlags_ChildWindows |
                             HoveredFlags_RootWindow),
        Gui::IsWindowHovered(HoveredFlags_ChildWindows |
                             HoveredFlags_RootWindow |
                             HoveredFlags_NoPopupHierarchy),
        Gui::IsWindowHovered(HoveredFlags_ChildWindows |
                             HoveredFlags_RootWindow |
                             HoveredFlags_DockHierarchy),
        Gui::IsWindowHovered(HoveredFlags_RootWindow),
        Gui::IsWindowHovered(HoveredFlags_RootWindow |
                             HoveredFlags_NoPopupHierarchy),
        Gui::IsWindowHovered(HoveredFlags_RootWindow |
                             HoveredFlags_DockHierarchy),
        Gui::IsWindowHovered(HoveredFlags_ChildWindows |
                             HoveredFlags_AllowWhenBlockedByPopup),
        Gui::IsWindowHovered(HoveredFlags_AnyWindow),
        Gui::IsWindowHovered(HoveredFlags_Stationary));

    Gui::BeginChild("child", Vec2(0, 50), ChildFlags_Border);
    Gui::Text(
        "This is another child window for testing the _ChildWindows flag.");
    Gui::EndChild();
    if (embed_all_inside_a_child_window)
      Gui::EndChild();

    // Calling IsItemHovered() after begin returns the hovered status of the
    // title bar. This is useful in particular if you want to create a context
    // menu associated to the title bar of a window. This will also work when
    // docked into a Tab (the Tab replace the Title Bar and guarantee the same
    // properties).
    static bool test_window = false;
    Gui::Checkbox("Hovered/Active tests after Begin() for title bar testing",
                  &test_window);
    if (test_window) {
      // FIXME-DOCK: This window cannot be docked within the Gui Demo window,
      // this will cause a feedback loop and get them stuck. Could we fix this
      // through an WindowClass feature? Or an API call to tag our parent
      // as "don't skip items"?
      Gui::Begin("Title bar Hovered/Active tests", &test_window);
      if (Gui::BeginPopupContextItem()) // <-- This is using IsItemHovered()
      {
        if (Gui::MenuItem("Close")) {
          test_window = false;
        }
        Gui::EndPopup();
      }
      Gui::Text("IsItemHovered() after begin = %d (== is title bar hovered)\n"
                "IsItemActive() after begin = %d (== is window being "
                "clicked/moved)\n",
                Gui::IsItemHovered(), Gui::IsItemActive());
      Gui::End();
    }

    Gui::TreePop();
  }

  // Demonstrate BeginDisabled/EndDisabled using a checkbox located at the
  // bottom of the section (which is a bit odd: logically we'd have this
  // checkbox at the top of the section, but we don't want this feature to steal
  // that space)
  if (disable_all)
    Gui::EndDisabled();

  DEMO_MARKER("Widgets/Disable Block");
  if (Gui::TreeNode("Disable block")) {
    Gui::Checkbox("Disable entire section above", &disable_all);
    Gui::SameLine();
    HelpMarker(
        "Demonstrate using BeginDisabled()/EndDisabled() across this section.");
    Gui::TreePop();
  }

  DEMO_MARKER("Widgets/Text Filter");
  if (Gui::TreeNode("Text Filter")) {
    // Helper class to easy setup a text filter.
    // You may want to implement a more feature-full filtering scheme in your
    // own application.
    HelpMarker("Not a widget per-se, but TextFilter is a helper to "
               "perform simple filtering on text strings.");
    static TextFilter filter;
    Gui::Text("Filter usage:\n"
              "  \"\"         display all lines\n"
              "  \"xxx\"      display lines containing \"xxx\"\n"
              "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
              "  \"-xxx\"     hide lines containing \"xxx\"");
    filter.Draw();
    const char *lines[] = {"aaa1.c",   "bbb1.c",   "ccc1.c", "aaa2.cpp",
                           "bbb2.cpp", "ccc2.cpp", "abc.h",  "hello, world"};
    for (int i = 0; i < ARRAYSIZE(lines); i++)
      if (filter.PassFilter(lines[i]))
        Gui::BulletText("%s", lines[i]);
    Gui::TreePop();
  }
}

static void ShowDemoWindowLayout() {
  DEMO_MARKER("Layout");
  if (!Gui::CollapsingHeader("Layout & Scrolling"))
    return;

  DEMO_MARKER("Layout/Child windows");
  if (Gui::TreeNode("Child windows")) {
    Gui::SeparatorText("Child windows");

    HelpMarker("Use child windows to begin into a self-contained independent "
               "scrolling/clipping regions within a host window.");
    static bool disable_mouse_wheel = false;
    static bool disable_menu = false;
    Gui::Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);
    Gui::Checkbox("Disable Menu", &disable_menu);

    // Child 1: no border, enable horizontal scrollbar
    {
      WindowFlags window_flags = WindowFlags_HorizontalScrollbar;
      if (disable_mouse_wheel)
        window_flags |= WindowFlags_NoScrollWithMouse;
      Gui::BeginChild("ChildL",
                      Vec2(Gui::GetContentRegionAvail().x * 0.5f, 260),
                      ChildFlags_None, window_flags);
      for (int i = 0; i < 100; i++)
        Gui::Text("%04d: scrollable region", i);
      Gui::EndChild();
    }

    Gui::SameLine();

    // Child 2: rounded border
    {
      WindowFlags window_flags = WindowFlags_None;
      if (disable_mouse_wheel)
        window_flags |= WindowFlags_NoScrollWithMouse;
      if (!disable_menu)
        window_flags |= WindowFlags_MenuBar;
      Gui::PushStyleVar(StyleVar_ChildRounding, 5.0f);
      Gui::BeginChild("ChildR", Vec2(0, 260), ChildFlags_Border, window_flags);
      if (!disable_menu && Gui::BeginMenuBar()) {
        if (Gui::BeginMenu("Menu")) {
          ShowExampleMenuFile();
          Gui::EndMenu();
        }
        Gui::EndMenuBar();
      }
      if (Gui::BeginTable("split", 2,
                          TableFlags_Resizable | TableFlags_NoSavedSettings)) {
        for (int i = 0; i < 100; i++) {
          char buf[32];
          sprintf(buf, "%03d", i);
          Gui::TableNextColumn();
          Gui::Button(buf, Vec2(-FLT_MIN, 0.0f));
        }
        Gui::EndTable();
      }
      Gui::EndChild();
      Gui::PopStyleVar();
    }

    // Child 3: manual-resize
    Gui::SeparatorText("Manual-resize");
    {
      HelpMarker("Drag bottom border to resize. Double-click bottom border to "
                 "auto-fit to vertical contents.");
      Gui::PushStyleColor(Col_ChildBg, Gui::GetStyleColorVec4(Col_FrameBg));
      if (Gui::BeginChild(
              "ResizableChild",
              Vec2(-FLT_MIN, Gui::GetTextLineHeightWithSpacing() * 8),
              ChildFlags_Border | ChildFlags_ResizeY))
        for (int n = 0; n < 10; n++)
          Gui::Text("Line %04d", n);
      Gui::PopStyleColor();
      Gui::EndChild();
    }

    // Child 4: auto-resizing height with a limit
    Gui::SeparatorText("Auto-resize with constraints");
    {
      static int draw_lines = 3;
      static int max_height_in_lines = 10;
      Gui::SetNextItemWidth(Gui::GetFontSize() * 8);
      Gui::DragInt("Lines Count", &draw_lines, 0.2f);
      Gui::SetNextItemWidth(Gui::GetFontSize() * 8);
      Gui::DragInt("Max Height (in Lines)", &max_height_in_lines, 0.2f);

      Gui::SetNextWindowSizeConstraints(
          Vec2(0.0f, Gui::GetTextLineHeightWithSpacing() * 1),
          Vec2(FLT_MAX,
               Gui::GetTextLineHeightWithSpacing() * max_height_in_lines));
      if (Gui::BeginChild("ConstrainedChild", Vec2(-FLT_MIN, 0.0f),
                          ChildFlags_Border | ChildFlags_AutoResizeY))
        for (int n = 0; n < draw_lines; n++)
          Gui::Text("Line %04d", n);
      Gui::EndChild();
    }

    Gui::SeparatorText("Misc/Advanced");

    // Demonstrate a few extra things
    // - Changing Col_ChildBg (which is transparent black in default
    // styles)
    // - Using SetCursorPos() to position child window (the child window is an
    // item from the POV of parent window)
    //   You can also call SetNextWindowPos() to position the child window. The
    //   parent window will effectively layout from this position.
    // - Using Gui::GetItemRectMin/Max() to query the "item" state (because
    // the child window is an item from
    //   the POV of the parent window). See 'Demo->Querying Status
    //   (Edited/Active/Hovered etc.)' for details.
    {
      static int offset_x = 0;
      static bool override_bg_color = true;
      static ChildFlags child_flags =
          ChildFlags_Border | ChildFlags_ResizeX | ChildFlags_ResizeY;
      Gui::SetNextItemWidth(Gui::GetFontSize() * 8);
      Gui::DragInt("Offset X", &offset_x, 1.0f, -1000, 1000);
      Gui::Checkbox("Override ChildBg color", &override_bg_color);
      Gui::CheckboxFlags("ChildFlags_Border", &child_flags, ChildFlags_Border);
      Gui::CheckboxFlags("ChildFlags_AlwaysUseWindowPadding", &child_flags,
                         ChildFlags_AlwaysUseWindowPadding);
      Gui::CheckboxFlags("ChildFlags_ResizeX", &child_flags,
                         ChildFlags_ResizeX);
      Gui::CheckboxFlags("ChildFlags_ResizeY", &child_flags,
                         ChildFlags_ResizeY);
      Gui::CheckboxFlags("ChildFlags_FrameStyle", &child_flags,
                         ChildFlags_FrameStyle);
      Gui::SameLine();
      HelpMarker("Style the child window like a framed item: use FrameBg, "
                 "FrameRounding, FrameBorderSize, FramePadding instead of "
                 "ChildBg, ChildRounding, ChildBorderSize, WindowPadding.");
      if (child_flags & ChildFlags_FrameStyle)
        override_bg_color = false;

      Gui::SetCursorPosX(Gui::GetCursorPosX() + (float)offset_x);
      if (override_bg_color)
        Gui::PushStyleColor(Col_ChildBg, COL32(255, 0, 0, 100));
      Gui::BeginChild("Red", Vec2(200, 100), child_flags, WindowFlags_None);
      if (override_bg_color)
        Gui::PopStyleColor();

      for (int n = 0; n < 50; n++)
        Gui::Text("Some test %d", n);
      Gui::EndChild();
      bool child_is_hovered = Gui::IsItemHovered();
      Vec2 child_rect_min = Gui::GetItemRectMin();
      Vec2 child_rect_max = Gui::GetItemRectMax();
      Gui::Text("Hovered: %d", child_is_hovered);
      Gui::Text("Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)",
                child_rect_min.x, child_rect_min.y, child_rect_max.x,
                child_rect_max.y);
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Layout/Widgets Width");
  if (Gui::TreeNode("Widgets Width")) {
    static float f = 0.0f;
    static bool show_indented_items = true;
    Gui::Checkbox("Show indented items", &show_indented_items);

    // Use SetNextItemWidth() to set the width of a single upcoming item.
    // Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
    // In real code use you'll probably want to choose width values that are
    // proportional to your font size e.g. Using '20.0f * GetFontSize()' as
    // width instead of '200.0f', etc.

    Gui::Text("SetNextItemWidth/PushItemWidth(100)");
    Gui::SameLine();
    HelpMarker("Fixed width.");
    Gui::PushItemWidth(100);
    Gui::DragFloat("float##1b", &f);
    if (show_indented_items) {
      Gui::Indent();
      Gui::DragFloat("float (indented)##1b", &f);
      Gui::Unindent();
    }
    Gui::PopItemWidth();

    Gui::Text("SetNextItemWidth/PushItemWidth(-100)");
    Gui::SameLine();
    HelpMarker("Align to right edge minus 100");
    Gui::PushItemWidth(-100);
    Gui::DragFloat("float##2a", &f);
    if (show_indented_items) {
      Gui::Indent();
      Gui::DragFloat("float (indented)##2b", &f);
      Gui::Unindent();
    }
    Gui::PopItemWidth();

    Gui::Text(
        "SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5f)");
    Gui::SameLine();
    HelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within "
               "a column set)");
    Gui::PushItemWidth(Gui::GetContentRegionAvail().x * 0.5f);
    Gui::DragFloat("float##3a", &f);
    if (show_indented_items) {
      Gui::Indent();
      Gui::DragFloat("float (indented)##3b", &f);
      Gui::Unindent();
    }
    Gui::PopItemWidth();

    Gui::Text(
        "SetNextItemWidth/PushItemWidth(-GetContentRegionAvail().x * 0.5f)");
    Gui::SameLine();
    HelpMarker("Align to right edge minus half");
    Gui::PushItemWidth(-Gui::GetContentRegionAvail().x * 0.5f);
    Gui::DragFloat("float##4a", &f);
    if (show_indented_items) {
      Gui::Indent();
      Gui::DragFloat("float (indented)##4b", &f);
      Gui::Unindent();
    }
    Gui::PopItemWidth();

    // Demonstrate using PushItemWidth to surround three items.
    // Calling SetNextItemWidth() before each of them would have the same
    // effect.
    Gui::Text("SetNextItemWidth/PushItemWidth(-FLT_MIN)");
    Gui::SameLine();
    HelpMarker("Align to right edge");
    Gui::PushItemWidth(-FLT_MIN);
    Gui::DragFloat("##float5a", &f);
    if (show_indented_items) {
      Gui::Indent();
      Gui::DragFloat("float (indented)##5b", &f);
      Gui::Unindent();
    }
    Gui::PopItemWidth();

    Gui::TreePop();
  }

  DEMO_MARKER("Layout/Basic Horizontal Layout");
  if (Gui::TreeNode("Basic Horizontal Layout")) {
    Gui::TextWrapped("(Use Gui::SameLine() to keep adding items to the "
                     "right of the preceding item)");

    // Text
    DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine");
    Gui::Text("Two items: Hello");
    Gui::SameLine();
    Gui::TextColored(Vec4(1, 1, 0, 1), "Sailor");

    // Adjust spacing
    Gui::Text("More spacing: Hello");
    Gui::SameLine(0, 20);
    Gui::TextColored(Vec4(1, 1, 0, 1), "Sailor");

    // Button
    Gui::AlignTextToFramePadding();
    Gui::Text("Normal buttons");
    Gui::SameLine();
    Gui::Button("Banana");
    Gui::SameLine();
    Gui::Button("Apple");
    Gui::SameLine();
    Gui::Button("Corniflower");

    // Button
    Gui::Text("Small buttons");
    Gui::SameLine();
    Gui::SmallButton("Like this one");
    Gui::SameLine();
    Gui::Text("can fit within a text block.");

    // Aligned to arbitrary position. Easy/cheap column.
    DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (with offset)");
    Gui::Text("Aligned");
    Gui::SameLine(150);
    Gui::Text("x=150");
    Gui::SameLine(300);
    Gui::Text("x=300");
    Gui::Text("Aligned");
    Gui::SameLine(150);
    Gui::SmallButton("x=150");
    Gui::SameLine(300);
    Gui::SmallButton("x=300");

    // Checkbox
    DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (more)");
    static bool c1 = false, c2 = false, c3 = false, c4 = false;
    Gui::Checkbox("My", &c1);
    Gui::SameLine();
    Gui::Checkbox("Tailor", &c2);
    Gui::SameLine();
    Gui::Checkbox("Is", &c3);
    Gui::SameLine();
    Gui::Checkbox("Rich", &c4);

    // Various
    static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
    Gui::PushItemWidth(80);
    const char *items[] = {"AAAA", "BBBB", "CCCC", "DDDD"};
    static int item = -1;
    Gui::Combo("Combo", &item, items, ARRAYSIZE(items));
    Gui::SameLine();
    Gui::SliderFloat("X", &f0, 0.0f, 5.0f);
    Gui::SameLine();
    Gui::SliderFloat("Y", &f1, 0.0f, 5.0f);
    Gui::SameLine();
    Gui::SliderFloat("Z", &f2, 0.0f, 5.0f);
    Gui::PopItemWidth();

    Gui::PushItemWidth(80);
    Gui::Text("Lists:");
    static int selection[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
      if (i > 0)
        Gui::SameLine();
      Gui::PushID(i);
      Gui::ListBox("", &selection[i], items, ARRAYSIZE(items));
      Gui::PopID();
      // Gui::SetItemTooltip("ListBox %d hovered", i);
    }
    Gui::PopItemWidth();

    // Dummy
    DEMO_MARKER("Layout/Basic Horizontal Layout/Dummy");
    Vec2 button_sz(40, 40);
    Gui::Button("A", button_sz);
    Gui::SameLine();
    Gui::Dummy(button_sz);
    Gui::SameLine();
    Gui::Button("B", button_sz);

    // Manually wrapping
    // (we should eventually provide this as an automatic layout feature, but
    // for now you can do it manually)
    DEMO_MARKER("Layout/Basic Horizontal Layout/Manual wrapping");
    Gui::Text("Manual wrapping:");
    Style &style = Gui::GetStyle();
    int buttons_count = 20;
    float window_visible_x2 =
        Gui::GetWindowPos().x + Gui::GetWindowContentRegionMax().x;
    for (int n = 0; n < buttons_count; n++) {
      Gui::PushID(n);
      Gui::Button("Box", button_sz);
      float last_button_x2 = Gui::GetItemRectMax().x;
      float next_button_x2 =
          last_button_x2 + style.ItemSpacing.x +
          button_sz.x; // Expected position if next button was on same line
      if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
        Gui::SameLine();
      Gui::PopID();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Layout/Groups");
  if (Gui::TreeNode("Groups")) {
    HelpMarker(
        "BeginGroup() basically locks the horizontal position for new line. "
        "EndGroup() bundles the whole group so that you can use \"item\" "
        "functions such as "
        "IsItemHovered()/IsItemActive() or SameLine() etc. on the whole "
        "group.");
    Gui::BeginGroup();
    {
      Gui::BeginGroup();
      Gui::Button("AAA");
      Gui::SameLine();
      Gui::Button("BBB");
      Gui::SameLine();
      Gui::BeginGroup();
      Gui::Button("CCC");
      Gui::Button("DDD");
      Gui::EndGroup();
      Gui::SameLine();
      Gui::Button("EEE");
      Gui::EndGroup();
      Gui::SetItemTooltip("First group hovered");
    }
    // Capture the group size and create widgets using the same size
    Vec2 size = Gui::GetItemRectSize();
    const float values[5] = {0.5f, 0.20f, 0.80f, 0.60f, 0.25f};
    Gui::PlotHistogram("##values", values, ARRAYSIZE(values), 0, NULL, 0.0f,
                       1.0f, size);

    Gui::Button("ACTION",
                Vec2((size.x - Gui::GetStyle().ItemSpacing.x) * 0.5f, size.y));
    Gui::SameLine();
    Gui::Button("REACTION",
                Vec2((size.x - Gui::GetStyle().ItemSpacing.x) * 0.5f, size.y));
    Gui::EndGroup();
    Gui::SameLine();

    Gui::Button("LEVERAGE\nBUZZWORD", size);
    Gui::SameLine();

    if (Gui::BeginListBox("List", size)) {
      Gui::Selectable("Selected", true);
      Gui::Selectable("Not Selected", false);
      Gui::EndListBox();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Layout/Text Baseline Alignment");
  if (Gui::TreeNode("Text Baseline Alignment")) {
    {
      Gui::BulletText("Text baseline:");
      Gui::SameLine();
      HelpMarker("This is testing the vertical alignment that gets applied on "
                 "text to keep it aligned with widgets. "
                 "Lines only composed of text or \"small\" widgets use less "
                 "vertical space than lines with framed widgets.");
      Gui::Indent();

      Gui::Text("KO Blahblah");
      Gui::SameLine();
      Gui::Button("Some framed item");
      Gui::SameLine();
      HelpMarker("Baseline of button will look misaligned with text..");

      // If your line starts with text, call AlignTextToFramePadding() to align
      // text to upcoming widgets. (because we don't know what's coming after
      // the Text() statement, we need to move the text baseline down by
      // FramePadding.y ahead of time)
      Gui::AlignTextToFramePadding();
      Gui::Text("OK Blahblah");
      Gui::SameLine();
      Gui::Button("Some framed item");
      Gui::SameLine();
      HelpMarker("We call AlignTextToFramePadding() to vertically align the "
                 "text baseline by +FramePadding.y");

      // SmallButton() uses the same vertical padding as Text
      Gui::Button("TEST##1");
      Gui::SameLine();
      Gui::Text("TEST");
      Gui::SameLine();
      Gui::SmallButton("TEST##2");

      // If your line starts with text, call AlignTextToFramePadding() to align
      // text to upcoming widgets.
      Gui::AlignTextToFramePadding();
      Gui::Text("Text aligned to framed item");
      Gui::SameLine();
      Gui::Button("Item##1");
      Gui::SameLine();
      Gui::Text("Item");
      Gui::SameLine();
      Gui::SmallButton("Item##2");
      Gui::SameLine();
      Gui::Button("Item##3");

      Gui::Unindent();
    }

    Gui::Spacing();

    {
      Gui::BulletText("Multi-line text:");
      Gui::Indent();
      Gui::Text("One\nTwo\nThree");
      Gui::SameLine();
      Gui::Text("Hello\nWorld");
      Gui::SameLine();
      Gui::Text("Banana");

      Gui::Text("Banana");
      Gui::SameLine();
      Gui::Text("Hello\nWorld");
      Gui::SameLine();
      Gui::Text("One\nTwo\nThree");

      Gui::Button("HOP##1");
      Gui::SameLine();
      Gui::Text("Banana");
      Gui::SameLine();
      Gui::Text("Hello\nWorld");
      Gui::SameLine();
      Gui::Text("Banana");

      Gui::Button("HOP##2");
      Gui::SameLine();
      Gui::Text("Hello\nWorld");
      Gui::SameLine();
      Gui::Text("Banana");
      Gui::Unindent();
    }

    Gui::Spacing();

    {
      Gui::BulletText("Misc items:");
      Gui::Indent();

      // SmallButton() sets FramePadding to zero. Text baseline is aligned to
      // match baseline of previous Button.
      Gui::Button("80x80", Vec2(80, 80));
      Gui::SameLine();
      Gui::Button("50x50", Vec2(50, 50));
      Gui::SameLine();
      Gui::Button("Button()");
      Gui::SameLine();
      Gui::SmallButton("SmallButton()");

      // Tree
      const float spacing = Gui::GetStyle().ItemInnerSpacing.x;
      Gui::Button("Button##1");
      Gui::SameLine(0.0f, spacing);
      if (Gui::TreeNode("Node##1")) {
        // Placeholder tree data
        for (int i = 0; i < 6; i++)
          Gui::BulletText("Item %d..", i);
        Gui::TreePop();
      }

      // Vertically align text node a bit lower so it'll be vertically centered
      // with upcoming widget. Otherwise you can use SmallButton() (smaller
      // fit).
      Gui::AlignTextToFramePadding();

      // Common mistake to avoid: if we want to SameLine after TreeNode we need
      // to do it before we add other contents below the node.
      bool node_open = Gui::TreeNode("Node##2");
      Gui::SameLine(0.0f, spacing);
      Gui::Button("Button##2");
      if (node_open) {
        // Placeholder tree data
        for (int i = 0; i < 6; i++)
          Gui::BulletText("Item %d..", i);
        Gui::TreePop();
      }

      // Bullet
      Gui::Button("Button##3");
      Gui::SameLine(0.0f, spacing);
      Gui::BulletText("Bullet text");

      Gui::AlignTextToFramePadding();
      Gui::BulletText("Node");
      Gui::SameLine(0.0f, spacing);
      Gui::Button("Button##4");
      Gui::Unindent();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Layout/Scrolling");
  if (Gui::TreeNode("Scrolling")) {
    // Vertical scroll functions
    DEMO_MARKER("Layout/Scrolling/Vertical");
    HelpMarker("Use SetScrollHereY() or SetScrollFromPosY() to scroll to a "
               "given vertical position.");

    static int track_item = 50;
    static bool enable_track = true;
    static bool enable_extra_decorations = false;
    static float scroll_to_off_px = 0.0f;
    static float scroll_to_pos_px = 200.0f;

    Gui::Checkbox("Decoration", &enable_extra_decorations);

    Gui::Checkbox("Track", &enable_track);
    Gui::PushItemWidth(100);
    Gui::SameLine(140);
    enable_track |=
        Gui::DragInt("##item", &track_item, 0.25f, 0, 99, "Item = %d");

    bool scroll_to_off = Gui::Button("Scroll Offset");
    Gui::SameLine(140);
    scroll_to_off |= Gui::DragFloat("##off", &scroll_to_off_px, 1.00f, 0,
                                    FLT_MAX, "+%.0f px");

    bool scroll_to_pos = Gui::Button("Scroll To Pos");
    Gui::SameLine(140);
    scroll_to_pos |= Gui::DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10,
                                    FLT_MAX, "X/Y = %.0f px");
    Gui::PopItemWidth();

    if (scroll_to_off || scroll_to_pos)
      enable_track = false;

    Style &style = Gui::GetStyle();
    float child_w =
        (Gui::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 5;
    if (child_w < 1.0f)
      child_w = 1.0f;
    Gui::PushID("##VerticalScrolling");
    for (int i = 0; i < 5; i++) {
      if (i > 0)
        Gui::SameLine();
      Gui::BeginGroup();
      const char *names[] = {"Top", "25%", "Center", "75%", "Bottom"};
      Gui::TextUnformatted(names[i]);

      const WindowFlags child_flags =
          enable_extra_decorations ? WindowFlags_MenuBar : 0;
      const ID child_id = Gui::GetID((void *)(intptr_t)i);
      const bool child_is_visible = Gui::BeginChild(
          child_id, Vec2(child_w, 200.0f), ChildFlags_Border, child_flags);
      if (Gui::BeginMenuBar()) {
        Gui::TextUnformatted("abc");
        Gui::EndMenuBar();
      }
      if (scroll_to_off)
        Gui::SetScrollY(scroll_to_off_px);
      if (scroll_to_pos)
        Gui::SetScrollFromPosY(Gui::GetCursorStartPos().y + scroll_to_pos_px,
                               i * 0.25f);
      if (child_is_visible) // Avoid calling SetScrollHereY when running with
                            // culled items
      {
        for (int item = 0; item < 100; item++) {
          if (enable_track && item == track_item) {
            Gui::TextColored(Vec4(1, 1, 0, 1), "Item %d", item);
            Gui::SetScrollHereY(i *
                                0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
          } else {
            Gui::Text("Item %d", item);
          }
        }
      }
      float scroll_y = Gui::GetScrollY();
      float scroll_max_y = Gui::GetScrollMaxY();
      Gui::EndChild();
      Gui::Text("%.0f/%.0f", scroll_y, scroll_max_y);
      Gui::EndGroup();
    }
    Gui::PopID();

    // Horizontal scroll functions
    DEMO_MARKER("Layout/Scrolling/Horizontal");
    Gui::Spacing();
    HelpMarker("Use SetScrollHereX() or SetScrollFromPosX() to scroll to a "
               "given horizontal position.\n\n"
               "Because the clipping rectangle of most window hides half worth "
               "of WindowPadding on the "
               "left/right, using SetScrollFromPosX(+1) will usually result in "
               "clipped text whereas the "
               "equivalent SetScrollFromPosY(+1) wouldn't.");
    Gui::PushID("##HorizontalScrolling");
    for (int i = 0; i < 5; i++) {
      float child_height = Gui::GetTextLineHeight() + style.ScrollbarSize +
                           style.WindowPadding.y * 2.0f;
      WindowFlags child_flags =
          WindowFlags_HorizontalScrollbar |
          (enable_extra_decorations ? WindowFlags_AlwaysVerticalScrollbar : 0);
      ID child_id = Gui::GetID((void *)(intptr_t)i);
      bool child_is_visible = Gui::BeginChild(
          child_id, Vec2(-100, child_height), ChildFlags_Border, child_flags);
      if (scroll_to_off)
        Gui::SetScrollX(scroll_to_off_px);
      if (scroll_to_pos)
        Gui::SetScrollFromPosX(Gui::GetCursorStartPos().x + scroll_to_pos_px,
                               i * 0.25f);
      if (child_is_visible) // Avoid calling SetScrollHereY when running with
                            // culled items
      {
        for (int item = 0; item < 100; item++) {
          if (item > 0)
            Gui::SameLine();
          if (enable_track && item == track_item) {
            Gui::TextColored(Vec4(1, 1, 0, 1), "Item %d", item);
            Gui::SetScrollHereX(i *
                                0.25f); // 0.0f:left, 0.5f:center, 1.0f:right
          } else {
            Gui::Text("Item %d", item);
          }
        }
      }
      float scroll_x = Gui::GetScrollX();
      float scroll_max_x = Gui::GetScrollMaxX();
      Gui::EndChild();
      Gui::SameLine();
      const char *names[] = {"Left", "25%", "Center", "75%", "Right"};
      Gui::Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
      Gui::Spacing();
    }
    Gui::PopID();

    // Miscellaneous Horizontal Scrolling Demo
    DEMO_MARKER("Layout/Scrolling/Horizontal (more)");
    HelpMarker("Horizontal scrolling for a window is enabled via the "
               "WindowFlags_HorizontalScrollbar flag.\n\n"
               "You may want to also explicitly specify content width by using "
               "SetNextWindowContentWidth() before Begin().");
    static int lines = 7;
    Gui::SliderInt("Lines", &lines, 1, 15);
    Gui::PushStyleVar(StyleVar_FrameRounding, 3.0f);
    Gui::PushStyleVar(StyleVar_FramePadding, Vec2(2.0f, 1.0f));
    Vec2 scrolling_child_size =
        Vec2(0, Gui::GetFrameHeightWithSpacing() * 7 + 30);
    Gui::BeginChild("scrolling", scrolling_child_size, ChildFlags_Border,
                    WindowFlags_HorizontalScrollbar);
    for (int line = 0; line < lines; line++) {
      // Display random stuff. For the sake of this trivial demo we are using
      // basic Button() + SameLine() If you want to create your own time line
      // for a real application you may be better off manipulating the cursor
      // position yourself, aka using SetCursorPos/SetCursorScreenPos to
      // position the widgets yourself. You may also want to use the lower-level
      // DrawList API.
      int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
      for (int n = 0; n < num_buttons; n++) {
        if (n > 0)
          Gui::SameLine();
        Gui::PushID(n + line * 1000);
        char num_buf[16];
        sprintf(num_buf, "%d", n);
        const char *label = (!(n % 15))  ? "FizzBuzz"
                            : (!(n % 3)) ? "Fizz"
                            : (!(n % 5)) ? "Buzz"
                                         : num_buf;
        float hue = n * 0.05f;
        Gui::PushStyleColor(Col_Button, (Vec4)Color::HSV(hue, 0.6f, 0.6f));
        Gui::PushStyleColor(Col_ButtonHovered,
                            (Vec4)Color::HSV(hue, 0.7f, 0.7f));
        Gui::PushStyleColor(Col_ButtonActive,
                            (Vec4)Color::HSV(hue, 0.8f, 0.8f));
        Gui::Button(label, Vec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
        Gui::PopStyleColor(3);
        Gui::PopID();
      }
    }
    float scroll_x = Gui::GetScrollX();
    float scroll_max_x = Gui::GetScrollMaxX();
    Gui::EndChild();
    Gui::PopStyleVar(2);
    float scroll_x_delta = 0.0f;
    Gui::SmallButton("<<");
    if (Gui::IsItemActive())
      scroll_x_delta = -Gui::GetIO().DeltaTime * 1000.0f;
    Gui::SameLine();
    Gui::Text("Scroll from code");
    Gui::SameLine();
    Gui::SmallButton(">>");
    if (Gui::IsItemActive())
      scroll_x_delta = +Gui::GetIO().DeltaTime * 1000.0f;
    Gui::SameLine();
    Gui::Text("%.0f/%.0f", scroll_x, scroll_max_x);
    if (scroll_x_delta != 0.0f) {
      // Demonstrate a trick: you can use Begin to set yourself in the context
      // of another window (here we are already out of your child window)
      Gui::BeginChild("scrolling");
      Gui::SetScrollX(Gui::GetScrollX() + scroll_x_delta);
      Gui::EndChild();
    }
    Gui::Spacing();

    static bool show_horizontal_contents_size_demo_window = false;
    Gui::Checkbox("Show Horizontal contents size demo window",
                  &show_horizontal_contents_size_demo_window);

    if (show_horizontal_contents_size_demo_window) {
      static bool show_h_scrollbar = true;
      static bool show_button = true;
      static bool show_tree_nodes = true;
      static bool show_text_wrapped = false;
      static bool show_columns = true;
      static bool show_tab_bar = true;
      static bool show_child = false;
      static bool explicit_content_size = false;
      static float contents_size_x = 300.0f;
      if (explicit_content_size)
        Gui::SetNextWindowContentSize(Vec2(contents_size_x, 0.0f));
      Gui::Begin("Horizontal contents size demo window",
                 &show_horizontal_contents_size_demo_window,
                 show_h_scrollbar ? WindowFlags_HorizontalScrollbar : 0);
      DEMO_MARKER("Layout/Scrolling/Horizontal contents size demo window");
      Gui::PushStyleVar(StyleVar_ItemSpacing, Vec2(2, 0));
      Gui::PushStyleVar(StyleVar_FramePadding, Vec2(2, 0));
      HelpMarker(
          "Test of different widgets react and impact the work rectangle "
          "growing when horizontal scrolling is enabled.\n\nUse "
          "'Metrics->Tools->Show windows rectangles' to visualize rectangles.");
      Gui::Checkbox("H-scrollbar", &show_h_scrollbar);
      Gui::Checkbox("Button",
                    &show_button); // Will grow contents size (unless
                                   // explicitly overwritten)
      Gui::Checkbox("Tree nodes",
                    &show_tree_nodes); // Will grow contents size and display
                                       // highlight over full width
      Gui::Checkbox("Text wrapped",
                    &show_text_wrapped); // Will grow and use contents size
      Gui::Checkbox("Columns", &show_columns); // Will use contents size
      Gui::Checkbox("Tab bar", &show_tab_bar); // Will use contents size
      Gui::Checkbox("Child", &show_child); // Will grow and use contents size
      Gui::Checkbox("Explicit content size", &explicit_content_size);
      Gui::Text("Scroll %.1f/%.1f %.1f/%.1f", Gui::GetScrollX(),
                Gui::GetScrollMaxX(), Gui::GetScrollY(), Gui::GetScrollMaxY());
      if (explicit_content_size) {
        Gui::SameLine();
        Gui::SetNextItemWidth(100);
        Gui::DragFloat("##csx", &contents_size_x);
        Vec2 p = Gui::GetCursorScreenPos();
        Gui::GetWindowDrawList()->AddRectFilled(p, Vec2(p.x + 10, p.y + 10),
                                                COL32_WHITE);
        Gui::GetWindowDrawList()->AddRectFilled(
            Vec2(p.x + contents_size_x - 10, p.y),
            Vec2(p.x + contents_size_x, p.y + 10), COL32_WHITE);
        Gui::Dummy(Vec2(0, 10));
      }
      Gui::PopStyleVar(2);
      Gui::Separator();
      if (show_button) {
        Gui::Button("this is a 300-wide button", Vec2(300, 0));
      }
      if (show_tree_nodes) {
        bool open = true;
        if (Gui::TreeNode("this is a tree node")) {
          if (Gui::TreeNode("another one of those tree node...")) {
            Gui::Text("Some tree contents");
            Gui::TreePop();
          }
          Gui::TreePop();
        }
        Gui::CollapsingHeader("CollapsingHeader", &open);
      }
      if (show_text_wrapped) {
        Gui::TextWrapped("This text should automatically wrap on the edge of "
                         "the work rectangle.");
      }
      if (show_columns) {
        Gui::Text("Tables:");
        if (Gui::BeginTable("table", 4, TableFlags_Borders)) {
          for (int n = 0; n < 4; n++) {
            Gui::TableNextColumn();
            Gui::Text("Width %.2f", Gui::GetContentRegionAvail().x);
          }
          Gui::EndTable();
        }
        Gui::Text("Columns:");
        Gui::Columns(4);
        for (int n = 0; n < 4; n++) {
          Gui::Text("Width %.2f", Gui::GetColumnWidth());
          Gui::NextColumn();
        }
        Gui::Columns(1);
      }
      if (show_tab_bar && Gui::BeginTabBar("Hello")) {
        if (Gui::BeginTabItem("OneOneOne")) {
          Gui::EndTabItem();
        }
        if (Gui::BeginTabItem("TwoTwoTwo")) {
          Gui::EndTabItem();
        }
        if (Gui::BeginTabItem("ThreeThreeThree")) {
          Gui::EndTabItem();
        }
        if (Gui::BeginTabItem("FourFourFour")) {
          Gui::EndTabItem();
        }
        Gui::EndTabBar();
      }
      if (show_child) {
        Gui::BeginChild("child", Vec2(0, 0), ChildFlags_Border);
        Gui::EndChild();
      }
      Gui::End();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Layout/Clipping");
  if (Gui::TreeNode("Clipping")) {
    static Vec2 size(100.0f, 100.0f);
    static Vec2 offset(30.0f, 30.0f);
    Gui::DragFloat2("size", (float *)&size, 0.5f, 1.0f, 200.0f, "%.0f");
    Gui::TextWrapped("(Click and drag to scroll)");

    HelpMarker(
        "(Left) Using Gui::PushClipRect():\n"
        "Will alter Gui hit-testing logic + DrawList rendering.\n"
        "(use this if you want your clipping rectangle to affect "
        "interactions)\n\n"
        "(Center) Using DrawList::PushClipRect():\n"
        "Will alter DrawList rendering only.\n"
        "(use this as a shortcut if you are only using DrawList calls)\n\n"
        "(Right) Using DrawList::AddText() with a fine ClipRect:\n"
        "Will alter only this specific DrawList::AddText() rendering.\n"
        "This is often used internally to avoid altering the clipping "
        "rectangle and minimize draw calls.");

    for (int n = 0; n < 3; n++) {
      if (n > 0)
        Gui::SameLine();

      Gui::PushID(n);
      Gui::InvisibleButton("##canvas", size);
      if (Gui::IsItemActive() && Gui::IsMouseDragging(MouseButton_Left)) {
        offset.x += Gui::GetIO().MouseDelta.x;
        offset.y += Gui::GetIO().MouseDelta.y;
      }
      Gui::PopID();
      if (!Gui::IsItemVisible()) // Skip rendering as DrawList elements are
                                 // not clipped.
        continue;

      const Vec2 p0 = Gui::GetItemRectMin();
      const Vec2 p1 = Gui::GetItemRectMax();
      const char *text_str = "Line 1 hello\nLine 2 clip me!";
      const Vec2 text_pos = Vec2(p0.x + offset.x, p0.y + offset.y);
      DrawList *draw_list = Gui::GetWindowDrawList();
      switch (n) {
      case 0:
        Gui::PushClipRect(p0, p1, true);
        draw_list->AddRectFilled(p0, p1, COL32(90, 90, 120, 255));
        draw_list->AddText(text_pos, COL32_WHITE, text_str);
        Gui::PopClipRect();
        break;
      case 1:
        draw_list->PushClipRect(p0, p1, true);
        draw_list->AddRectFilled(p0, p1, COL32(90, 90, 120, 255));
        draw_list->AddText(text_pos, COL32_WHITE, text_str);
        draw_list->PopClipRect();
        break;
      case 2:
        Vec4 clip_rect(p0.x, p0.y, p1.x,
                       p1.y); // AddText() takes a Vec4* here so let's convert.
        draw_list->AddRectFilled(p0, p1, COL32(90, 90, 120, 255));
        draw_list->AddText(Gui::GetFont(), Gui::GetFontSize(), text_pos,
                           COL32_WHITE, text_str, NULL, 0.0f, &clip_rect);
        break;
      }
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Layout/Overlap Mode");
  if (Gui::TreeNode("Overlap Mode")) {
    static bool enable_allow_overlap = true;

    HelpMarker("Hit-testing is by default performed in item submission order, "
               "which generally is perceived as 'back-to-front'.\n\n"
               "By using SetNextItemAllowOverlap() you can notify that an item "
               "may be overlapped by another. Doing so alters the hovering "
               "logic: items using AllowOverlap mode requires an extra frame "
               "to accept hovered state.");
    Gui::Checkbox("Enable AllowOverlap", &enable_allow_overlap);

    Vec2 button1_pos = Gui::GetCursorScreenPos();
    Vec2 button2_pos = Vec2(button1_pos.x + 50.0f, button1_pos.y + 50.0f);
    if (enable_allow_overlap)
      Gui::SetNextItemAllowOverlap();
    Gui::Button("Button 1", Vec2(80, 80));
    Gui::SetCursorScreenPos(button2_pos);
    Gui::Button("Button 2", Vec2(80, 80));

    // This is typically used with width-spanning items.
    // (note that Selectable() has a dedicated flag
    // SelectableFlags_AllowOverlap, which is a shortcut for using
    // SetNextItemAllowOverlap(). For demo purpose we use
    // SetNextItemAllowOverlap() here.)
    if (enable_allow_overlap)
      Gui::SetNextItemAllowOverlap();
    Gui::Selectable("Some Selectable", false);
    Gui::SameLine();
    Gui::SmallButton("++");

    Gui::TreePop();
  }
}

static void ShowDemoWindowPopups() {
  DEMO_MARKER("Popups");
  if (!Gui::CollapsingHeader("Popups & Modal windows"))
    return;

  // The properties of popups windows are:
  // - They block normal mouse hovering detection outside them. (*)
  // - Unless modal, they can be closed by clicking anywhere outside them, or by
  // pressing ESCAPE.
  // - Their visibility state (~bool) is held internally by Gui instead
  // of being held by the programmer as
  //   we are used to with regular Begin() calls. User can manipulate the
  //   visibility state by calling OpenPopup().
  // (*) One can use IsItemHovered(HoveredFlags_AllowWhenBlockedByPopup) to
  // bypass it and detect hovering even
  //     when normally blocked by a popup.
  // Those three properties are connected. The library needs to hold their
  // visibility state BECAUSE it can close popups at any time.

  // Typical use for regular windows:
  //   bool my_tool_is_active = false; if (Gui::Button("Open"))
  //   my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool",
  //   &my_tool_is_active) { [...] } End();
  // Typical use for popups:
  //   if (Gui::Button("Open")) Gui::OpenPopup("MyPopup"); if
  //   (Gui::BeginPopup("MyPopup") { [...] EndPopup(); }

  // With popups we have to go through a library call (here OpenPopup) to
  // manipulate the visibility state. This may be a bit confusing at first but
  // it should quickly make sense. Follow on the examples below.

  DEMO_MARKER("Popups/Popups");
  if (Gui::TreeNode("Popups")) {
    Gui::TextWrapped("When a popup is active, it inhibits interacting with "
                     "windows that are behind the popup. "
                     "Clicking outside the popup closes it.");

    static int selected_fish = -1;
    const char *names[] = {"Bream", "Haddock", "Mackerel", "Pollock",
                           "Tilefish"};
    static bool toggles[] = {true, false, false, false, false};

    // Simple selection popup (if you want to show the current selection inside
    // the Button itself, you may want to build a string using the "###"
    // operator to preserve a constant ID with a variable label)
    if (Gui::Button("Select.."))
      Gui::OpenPopup("my_select_popup");
    Gui::SameLine();
    Gui::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
    if (Gui::BeginPopup("my_select_popup")) {
      Gui::SeparatorText("Aquarium");
      for (int i = 0; i < ARRAYSIZE(names); i++)
        if (Gui::Selectable(names[i]))
          selected_fish = i;
      Gui::EndPopup();
    }

    // Showing a menu with toggles
    if (Gui::Button("Toggle.."))
      Gui::OpenPopup("my_toggle_popup");
    if (Gui::BeginPopup("my_toggle_popup")) {
      for (int i = 0; i < ARRAYSIZE(names); i++)
        Gui::MenuItem(names[i], "", &toggles[i]);
      if (Gui::BeginMenu("Sub-menu")) {
        Gui::MenuItem("Click me");
        Gui::EndMenu();
      }

      Gui::Separator();
      Gui::Text("Tooltip here");
      Gui::SetItemTooltip("I am a tooltip over a popup");

      if (Gui::Button("Stacked Popup"))
        Gui::OpenPopup("another popup");
      if (Gui::BeginPopup("another popup")) {
        for (int i = 0; i < ARRAYSIZE(names); i++)
          Gui::MenuItem(names[i], "", &toggles[i]);
        if (Gui::BeginMenu("Sub-menu")) {
          Gui::MenuItem("Click me");
          if (Gui::Button("Stacked Popup"))
            Gui::OpenPopup("another popup");
          if (Gui::BeginPopup("another popup")) {
            Gui::Text("I am the last one here.");
            Gui::EndPopup();
          }
          Gui::EndMenu();
        }
        Gui::EndPopup();
      }
      Gui::EndPopup();
    }

    // Call the more complete ShowExampleMenuFile which we use in various places
    // of this demo
    if (Gui::Button("With a menu.."))
      Gui::OpenPopup("my_file_popup");
    if (Gui::BeginPopup("my_file_popup", WindowFlags_MenuBar)) {
      if (Gui::BeginMenuBar()) {
        if (Gui::BeginMenu("File")) {
          ShowExampleMenuFile();
          Gui::EndMenu();
        }
        if (Gui::BeginMenu("Edit")) {
          Gui::MenuItem("Dummy");
          Gui::EndMenu();
        }
        Gui::EndMenuBar();
      }
      Gui::Text("Hello from popup!");
      Gui::Button("This is a dummy button..");
      Gui::EndPopup();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Popups/Context menus");
  if (Gui::TreeNode("Context menus")) {
    HelpMarker("\"Context\" functions are simple helpers to associate a Popup "
               "to a given Item or Window identifier.");

    // BeginPopupContextItem() is a helper to provide common/simple popup
    // behavior of essentially doing:
    //     if (id == 0)
    //         id = GetItemID(); // Use last item id
    //     if (IsItemHovered() && IsMouseReleased(MouseButton_Right))
    //         OpenPopup(id);
    //     return BeginPopup(id);
    // For advanced uses you may want to replicate and customize this code.
    // See more details in BeginPopupContextItem().

    // Example 1
    // When used after an item that has an ID (e.g. Button), we can skip
    // providing an ID to BeginPopupContextItem(), and BeginPopupContextItem()
    // will use the last item ID as the popup ID.
    {
      const char *names[5] = {"Label1", "Label2", "Label3", "Label4", "Label5"};
      static int selected = -1;
      for (int n = 0; n < 5; n++) {
        if (Gui::Selectable(names[n], selected == n))
          selected = n;
        if (Gui::BeginPopupContextItem()) // <-- use last item id as popup id
        {
          selected = n;
          Gui::Text("This a popup for \"%s\"!", names[n]);
          if (Gui::Button("Close"))
            Gui::CloseCurrentPopup();
          Gui::EndPopup();
        }
        Gui::SetItemTooltip("Right-click to open popup");
      }
    }

    // Example 2
    // Popup on a Text() element which doesn't have an identifier: we need to
    // provide an identifier to BeginPopupContextItem(). Using an explicit
    // identifier is also convenient if you want to activate the popups from
    // different locations.
    {
      HelpMarker("Text() elements don't have stable identifiers so we need to "
                 "provide one.");
      static float value = 0.5f;
      Gui::Text("Value = %.3f <-- (1) right-click this text", value);
      if (Gui::BeginPopupContextItem("my popup")) {
        if (Gui::Selectable("Set to zero"))
          value = 0.0f;
        if (Gui::Selectable("Set to PI"))
          value = 3.1415f;
        Gui::SetNextItemWidth(-FLT_MIN);
        Gui::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
        Gui::EndPopup();
      }

      // We can also use OpenPopupOnItemClick() to toggle the visibility of a
      // given popup. Here we make it that right-clicking this other text
      // element opens the same popup as above. The popup itself will be
      // submitted by the code above.
      Gui::Text("(2) Or right-click this text");
      Gui::OpenPopupOnItemClick("my popup", PopupFlags_MouseButtonRight);

      // Back to square one: manually open the same popup.
      if (Gui::Button("(3) Or click this button"))
        Gui::OpenPopup("my popup");
    }

    // Example 3
    // When using BeginPopupContextItem() with an implicit identifier (NULL ==
    // use last item ID), we need to make sure your item identifier is stable.
    // In this example we showcase altering the item label while preserving its
    // identifier, using the ### operator (see FAQ).
    {
      HelpMarker("Showcase using a popup ID linked to item ID, with the item "
                 "having a changing label + stable ID using the ### operator.");
      static char name[32] = "Label1";
      char buf[64];
      sprintf(buf, "Button: %s###Button",
              name); // ### operator override ID ignoring the preceding label
      Gui::Button(buf);
      if (Gui::BeginPopupContextItem()) {
        Gui::Text("Edit name:");
        Gui::InputText("##edit", name, ARRAYSIZE(name));
        if (Gui::Button("Close"))
          Gui::CloseCurrentPopup();
        Gui::EndPopup();
      }
      Gui::SameLine();
      Gui::Text("(<-- right-click here)");
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Popups/Modals");
  if (Gui::TreeNode("Modals")) {
    Gui::TextWrapped("Modal windows are like popups but the user cannot "
                     "close them by clicking outside.");

    if (Gui::Button("Delete.."))
      Gui::OpenPopup("Delete?");

    // Always center this window when appearing
    Vec2 center = Gui::GetMainViewport()->GetCenter();
    Gui::SetNextWindowPos(center, Cond_Appearing, Vec2(0.5f, 0.5f));

    if (Gui::BeginPopupModal("Delete?", NULL, WindowFlags_AlwaysAutoResize)) {
      Gui::Text("All those beautiful files will be deleted.\nThis operation "
                "cannot be undone!");
      Gui::Separator();

      // static int unused_i = 0;
      // Gui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

      static bool dont_ask_me_next_time = false;
      Gui::PushStyleVar(StyleVar_FramePadding, Vec2(0, 0));
      Gui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
      Gui::PopStyleVar();

      if (Gui::Button("OK", Vec2(120, 0))) {
        Gui::CloseCurrentPopup();
      }
      Gui::SetItemDefaultFocus();
      Gui::SameLine();
      if (Gui::Button("Cancel", Vec2(120, 0))) {
        Gui::CloseCurrentPopup();
      }
      Gui::EndPopup();
    }

    if (Gui::Button("Stacked modals.."))
      Gui::OpenPopup("Stacked 1");
    if (Gui::BeginPopupModal("Stacked 1", NULL, WindowFlags_MenuBar)) {
      if (Gui::BeginMenuBar()) {
        if (Gui::BeginMenu("File")) {
          if (Gui::MenuItem("Some menu item")) {
          }
          Gui::EndMenu();
        }
        Gui::EndMenuBar();
      }
      Gui::Text("Hello from Stacked The First\nUsing "
                "style.Colors[Col_ModalWindowDimBg] behind it.");

      // Testing behavior of widgets stacking their own regular popups over the
      // modal.
      static int item = 1;
      static float color[4] = {0.4f, 0.7f, 0.0f, 0.5f};
      Gui::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
      Gui::ColorEdit4("color", color);

      if (Gui::Button("Add another modal.."))
        Gui::OpenPopup("Stacked 2");

      // Also demonstrate passing a bool* to BeginPopupModal(), this will create
      // a regular close button which will close the popup. Note that the
      // visibility state of popups is owned by imgui, so the input value of the
      // bool actually doesn't matter here.
      bool unused_open = true;
      if (Gui::BeginPopupModal("Stacked 2", &unused_open)) {
        Gui::Text("Hello from Stacked The Second!");
        if (Gui::Button("Close"))
          Gui::CloseCurrentPopup();
        Gui::EndPopup();
      }

      if (Gui::Button("Close"))
        Gui::CloseCurrentPopup();
      Gui::EndPopup();
    }

    Gui::TreePop();
  }

  DEMO_MARKER("Popups/Menus inside a regular window");
  if (Gui::TreeNode("Menus inside a regular window")) {
    Gui::TextWrapped("Below we are testing adding menu items to a regular "
                     "window. It's rather unusual but should work!");
    Gui::Separator();

    Gui::MenuItem("Menu item", "CTRL+M");
    if (Gui::BeginMenu("Menu inside a regular window")) {
      ShowExampleMenuFile();
      Gui::EndMenu();
    }
    Gui::Separator();
    Gui::TreePop();
  }
}

// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate Vector<MyItem> template if this
// structure is defined inside the demo function)
namespace {
// We are passing our own identifier to TableSetupColumn() to facilitate
// identifying columns in the sorting code. This identifier will be passed down
// into TableSortSpec::ColumnUserID. But it is possible to omit the user id
// parameter of TableSetupColumn() and just use the column index instead!
// (TableSortSpec::ColumnIndex) If you don't use sorting, you will
// generally never care about giving column an ID!
enum MyItemColumnID {
  MyItemColumnID_ID,
  MyItemColumnID_Name,
  MyItemColumnID_Action,
  MyItemColumnID_Quantity,
  MyItemColumnID_Description
};

struct MyItem {
  int ID;
  const char *Name;
  int Quantity;

  // We have a problem which is affecting _only this demo_ and should not affect
  // your code: As we don't rely on std:: or other third-party library to
  // compile gui, we only have reliable access to qsort(), however qsort
  // doesn't allow passing user data to comparing function. As a workaround, we
  // are storing the sort specs in a static/global for the comparing function to
  // access. In your own use case you would probably pass the sort specs to your
  // sorting/comparing functions directly and not use a global. We could
  // technically call Gui::TableGetSortSpecs() in CompareWithSortSpecs(), but
  // considering that this function is called very often by the sorting
  // algorithm it would be a little wasteful.
  static const TableSortSpecs *s_current_sort_specs;

  static void SortWithSortSpecs(TableSortSpecs *sort_specs, MyItem *items,
                                int items_count) {
    s_current_sort_specs =
        sort_specs; // Store in variable accessible by the sort function.
    if (items_count > 1)
      qsort(items, (size_t)items_count, sizeof(items[0]),
            MyItem::CompareWithSortSpecs);
    s_current_sort_specs = NULL;
  }

  // Compare function to be used by qsort()
  static int CDECL CompareWithSortSpecs(const void *lhs, const void *rhs) {
    const MyItem *a = (const MyItem *)lhs;
    const MyItem *b = (const MyItem *)rhs;
    for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
      // Here we identify columns using the ColumnUserID value that we ourselves
      // passed to TableSetupColumn() We could also choose to identify columns
      // based on their index (sort_spec->ColumnIndex), which is simpler!
      const TableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
      int delta = 0;
      switch (sort_spec->ColumnUserID) {
      case MyItemColumnID_ID:
        delta = (a->ID - b->ID);
        break;
      case MyItemColumnID_Name:
        delta = (strcmp(a->Name, b->Name));
        break;
      case MyItemColumnID_Quantity:
        delta = (a->Quantity - b->Quantity);
        break;
      case MyItemColumnID_Description:
        delta = (strcmp(a->Name, b->Name));
        break;
      default:
        ASSERT(0);
        break;
      }
      if (delta > 0)
        return (sort_spec->SortDirection == SortDirection_Ascending) ? +1 : -1;
      if (delta < 0)
        return (sort_spec->SortDirection == SortDirection_Ascending) ? -1 : +1;
    }

    // qsort() is instable so always return a way to differenciate items.
    // Your own compare function may want to avoid fallback on implicit sort
    // specs e.g. a Name compare if it wasn't already part of the sort specs.
    return (a->ID - b->ID);
  }
};
const TableSortSpecs *MyItem::s_current_sort_specs = NULL;
} // namespace

// Make the UI compact because there are so many fields
static void PushStyleCompact() {
  Style &style = Gui::GetStyle();
  Gui::PushStyleVar(
      StyleVar_FramePadding,
      Vec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
  Gui::PushStyleVar(
      StyleVar_ItemSpacing,
      Vec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact() { Gui::PopStyleVar(2); }

// Show a combo box with a choice of sizing policies
static void EditTableSizingFlags(TableFlags *p_flags) {
  struct EnumDesc {
    TableFlags Value;
    const char *Name;
    const char *Tooltip;
  };
  static const EnumDesc policies[] = {
      {TableFlags_None, "Default",
       "Use default sizing policy:\n- TableFlags_SizingFixedFit if "
       "ScrollX is on or if host window has "
       "WindowFlags_AlwaysAutoResize.\n- "
       "TableFlags_SizingStretchSame otherwise."},
      {TableFlags_SizingFixedFit, "TableFlags_SizingFixedFit",
       "Columns default to _WidthFixed (if resizable) or _WidthAuto (if not "
       "resizable), matching contents width."},
      {TableFlags_SizingFixedSame, "TableFlags_SizingFixedSame",
       "Columns are all the same width, matching the maximum contents "
       "width.\nImplicitly disable TableFlags_Resizable and enable "
       "TableFlags_NoKeepColumnsVisible."},
      {TableFlags_SizingStretchProp, "TableFlags_SizingStretchProp",
       "Columns default to _WidthStretch with weights proportional to their "
       "widths."},
      {TableFlags_SizingStretchSame, "TableFlags_SizingStretchSame",
       "Columns default to _WidthStretch with same weights."}};
  int idx;
  for (idx = 0; idx < ARRAYSIZE(policies); idx++)
    if (policies[idx].Value == (*p_flags & TableFlags_SizingMask_))
      break;
  const char *preview_text =
      (idx < ARRAYSIZE(policies))
          ? policies[idx].Name + (idx > 0 ? strlen("TableFlags") : 0)
          : "";
  if (Gui::BeginCombo("Sizing Policy", preview_text)) {
    for (int n = 0; n < ARRAYSIZE(policies); n++)
      if (Gui::Selectable(policies[n].Name, idx == n))
        *p_flags = (*p_flags & ~TableFlags_SizingMask_) | policies[n].Value;
    Gui::EndCombo();
  }
  Gui::SameLine();
  Gui::TextDisabled("(?)");
  if (Gui::BeginItemTooltip()) {
    Gui::PushTextWrapPos(Gui::GetFontSize() * 50.0f);
    for (int m = 0; m < ARRAYSIZE(policies); m++) {
      Gui::Separator();
      Gui::Text("%s:", policies[m].Name);
      Gui::Separator();
      Gui::SetCursorPosX(Gui::GetCursorPosX() +
                         Gui::GetStyle().IndentSpacing * 0.5f);
      Gui::TextUnformatted(policies[m].Tooltip);
    }
    Gui::PopTextWrapPos();
    Gui::EndTooltip();
  }
}

static void EditTableColumnsFlags(TableColumnFlags *p_flags) {
  Gui::CheckboxFlags("_Disabled", p_flags, TableColumnFlags_Disabled);
  Gui::SameLine();
  HelpMarker("Master disable flag (also hide from context menu)");
  Gui::CheckboxFlags("_DefaultHide", p_flags, TableColumnFlags_DefaultHide);
  Gui::CheckboxFlags("_DefaultSort", p_flags, TableColumnFlags_DefaultSort);
  if (Gui::CheckboxFlags("_WidthStretch", p_flags,
                         TableColumnFlags_WidthStretch))
    *p_flags &= ~(TableColumnFlags_WidthMask_ ^ TableColumnFlags_WidthStretch);
  if (Gui::CheckboxFlags("_WidthFixed", p_flags, TableColumnFlags_WidthFixed))
    *p_flags &= ~(TableColumnFlags_WidthMask_ ^ TableColumnFlags_WidthFixed);
  Gui::CheckboxFlags("_NoResize", p_flags, TableColumnFlags_NoResize);
  Gui::CheckboxFlags("_NoReorder", p_flags, TableColumnFlags_NoReorder);
  Gui::CheckboxFlags("_NoHide", p_flags, TableColumnFlags_NoHide);
  Gui::CheckboxFlags("_NoClip", p_flags, TableColumnFlags_NoClip);
  Gui::CheckboxFlags("_NoSort", p_flags, TableColumnFlags_NoSort);
  Gui::CheckboxFlags("_NoSortAscending", p_flags,
                     TableColumnFlags_NoSortAscending);
  Gui::CheckboxFlags("_NoSortDescending", p_flags,
                     TableColumnFlags_NoSortDescending);
  Gui::CheckboxFlags("_NoHeaderLabel", p_flags, TableColumnFlags_NoHeaderLabel);
  Gui::CheckboxFlags("_NoHeaderWidth", p_flags, TableColumnFlags_NoHeaderWidth);
  Gui::CheckboxFlags("_PreferSortAscending", p_flags,
                     TableColumnFlags_PreferSortAscending);
  Gui::CheckboxFlags("_PreferSortDescending", p_flags,
                     TableColumnFlags_PreferSortDescending);
  Gui::CheckboxFlags("_IndentEnable", p_flags, TableColumnFlags_IndentEnable);
  Gui::SameLine();
  HelpMarker("Default for column 0");
  Gui::CheckboxFlags("_IndentDisable", p_flags, TableColumnFlags_IndentDisable);
  Gui::SameLine();
  HelpMarker("Default for column >0");
  Gui::CheckboxFlags("_AngledHeader", p_flags, TableColumnFlags_AngledHeader);
}

static void ShowTableColumnsStatusFlags(TableColumnFlags flags) {
  Gui::CheckboxFlags("_IsEnabled", &flags, TableColumnFlags_IsEnabled);
  Gui::CheckboxFlags("_IsVisible", &flags, TableColumnFlags_IsVisible);
  Gui::CheckboxFlags("_IsSorted", &flags, TableColumnFlags_IsSorted);
  Gui::CheckboxFlags("_IsHovered", &flags, TableColumnFlags_IsHovered);
}

static void ShowDemoWindowTables() {
  // Gui::SetNextItemOpen(true, Cond_Once);
  DEMO_MARKER("Tables");
  if (!Gui::CollapsingHeader("Tables & Columns"))
    return;

  // Using those as a base value to create width/height that are factor of the
  // size of our font
  const float TEXT_BASE_WIDTH = Gui::CalcTextSize("A").x;
  const float TEXT_BASE_HEIGHT = Gui::GetTextLineHeightWithSpacing();

  Gui::PushID("Tables");

  int open_action = -1;
  if (Gui::Button("Expand all"))
    open_action = 1;
  Gui::SameLine();
  if (Gui::Button("Collapse all"))
    open_action = 0;
  Gui::SameLine();

  // Options
  static bool disable_indent = false;
  Gui::Checkbox("Disable tree indentation", &disable_indent);
  Gui::SameLine();
  HelpMarker("Disable the indenting of tree nodes so demo tables can use the "
             "full window width.");
  Gui::Separator();
  if (disable_indent)
    Gui::PushStyleVar(StyleVar_IndentSpacing, 0.0f);

  // About Styling of tables
  // Most settings are configured on a per-table basis via the flags passed to
  // BeginTable() and TableSetupColumns APIs. There are however a few settings
  // that a shared and part of the Style structure:
  //   style.CellPadding                          // Padding within each cell
  //   style.Colors[Col_TableHeaderBg]       // Table header background
  //   style.Colors[Col_TableBorderStrong]   // Table outer and header
  //   borders style.Colors[Col_TableBorderLight]    // Table inner borders
  //   style.Colors[Col_TableRowBg]          // Table row background when
  //   TableFlags_RowBg is enabled (even rows)
  //   style.Colors[Col_TableRowBgAlt]       // Table row background when
  //   TableFlags_RowBg is enabled (odds rows)

  // Demos
  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Basic");
  if (Gui::TreeNode("Basic")) {
    // Here we will showcase three different ways to output a table.
    // They are very simple variations of a same thing!

    // [Method 1] Using TableNextRow() to create a new row, and
    // TableSetColumnIndex() to select the column. In many situations, this is
    // the most flexible and easy to use pattern.
    HelpMarker("Using TableNextRow() + calling TableSetColumnIndex() _before_ "
               "each cell, in a loop.");
    if (Gui::BeginTable("table1", 3)) {
      for (int row = 0; row < 4; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("Row %d Column %d", row, column);
        }
      }
      Gui::EndTable();
    }

    // [Method 2] Using TableNextColumn() called multiple times, instead of
    // using a for loop + TableSetColumnIndex(). This is generally more
    // convenient when you have code manually submitting the contents of each
    // column.
    HelpMarker("Using TableNextRow() + calling TableNextColumn() _before_ each "
               "cell, manually.");
    if (Gui::BeginTable("table2", 3)) {
      for (int row = 0; row < 4; row++) {
        Gui::TableNextRow();
        Gui::TableNextColumn();
        Gui::Text("Row %d", row);
        Gui::TableNextColumn();
        Gui::Text("Some contents");
        Gui::TableNextColumn();
        Gui::Text("123.456");
      }
      Gui::EndTable();
    }

    // [Method 3] We call TableNextColumn() _before_ each cell. We never call
    // TableNextRow(), as TableNextColumn() will automatically wrap around and
    // create new rows as needed. This is generally more convenient when your
    // cells all contains the same type of data.
    HelpMarker("Only using TableNextColumn(), which tends to be convenient for "
               "tables where every cell contains the same type of contents.\n"
               "This is also more similar to the old NextColumn() function of "
               "the Columns API, and provided to facilitate the "
               "Columns->Tables API transition.");
    if (Gui::BeginTable("table3", 3)) {
      for (int item = 0; item < 14; item++) {
        Gui::TableNextColumn();
        Gui::Text("Item %d", item);
      }
      Gui::EndTable();
    }

    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Borders, background");
  if (Gui::TreeNode("Borders, background")) {
    // Expose a few Borders related flags interactively
    enum ContentsType { CT_Text, CT_FillButton };
    static TableFlags flags = TableFlags_Borders | TableFlags_RowBg;
    static bool display_headers = false;
    static int contents_type = CT_Text;

    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_RowBg", &flags, TableFlags_RowBg);
    Gui::CheckboxFlags("TableFlags_Borders", &flags, TableFlags_Borders);
    Gui::SameLine();
    HelpMarker("TableFlags_Borders\n = TableFlags_BordersInnerV\n | "
               "TableFlags_BordersOuterV\n | TableFlags_BordersInnerV\n | "
               "TableFlags_BordersOuterH");
    Gui::Indent();

    Gui::CheckboxFlags("TableFlags_BordersH", &flags, TableFlags_BordersH);
    Gui::Indent();
    Gui::CheckboxFlags("TableFlags_BordersOuterH", &flags,
                       TableFlags_BordersOuterH);
    Gui::CheckboxFlags("TableFlags_BordersInnerH", &flags,
                       TableFlags_BordersInnerH);
    Gui::Unindent();

    Gui::CheckboxFlags("TableFlags_BordersV", &flags, TableFlags_BordersV);
    Gui::Indent();
    Gui::CheckboxFlags("TableFlags_BordersOuterV", &flags,
                       TableFlags_BordersOuterV);
    Gui::CheckboxFlags("TableFlags_BordersInnerV", &flags,
                       TableFlags_BordersInnerV);
    Gui::Unindent();

    Gui::CheckboxFlags("TableFlags_BordersOuter", &flags,
                       TableFlags_BordersOuter);
    Gui::CheckboxFlags("TableFlags_BordersInner", &flags,
                       TableFlags_BordersInner);
    Gui::Unindent();

    Gui::AlignTextToFramePadding();
    Gui::Text("Cell contents:");
    Gui::SameLine();
    Gui::RadioButton("Text", &contents_type, CT_Text);
    Gui::SameLine();
    Gui::RadioButton("FillButton", &contents_type, CT_FillButton);
    Gui::Checkbox("Display headers", &display_headers);
    Gui::CheckboxFlags("TableFlags_NoBordersInBody", &flags,
                       TableFlags_NoBordersInBody);
    Gui::SameLine();
    HelpMarker("Disable vertical borders in columns Body (borders will always "
               "appear in Headers");
    PopStyleCompact();

    if (Gui::BeginTable("table1", 3, flags)) {
      // Display headers so we can inspect their interaction with borders.
      // (Headers are not the main purpose of this section of the demo, so we
      // are not elaborating on them too much. See other sections for details)
      if (display_headers) {
        Gui::TableSetupColumn("One");
        Gui::TableSetupColumn("Two");
        Gui::TableSetupColumn("Three");
        Gui::TableHeadersRow();
      }

      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          char buf[32];
          sprintf(buf, "Hello %d,%d", column, row);
          if (contents_type == CT_Text)
            Gui::TextUnformatted(buf);
          else if (contents_type == CT_FillButton)
            Gui::Button(buf, Vec2(-FLT_MIN, 0.0f));
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Resizable, stretch");
  if (Gui::TreeNode("Resizable, stretch")) {
    // By default, if we don't enable ScrollX the sizing policy for each column
    // is "Stretch" All columns maintain a sizing weight, and they will occupy
    // all available width.
    static TableFlags flags = TableFlags_SizingStretchSame |
                              TableFlags_Resizable | TableFlags_BordersOuter |
                              TableFlags_BordersV |
                              TableFlags_ContextMenuInBody;
    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_Resizable", &flags, TableFlags_Resizable);
    Gui::CheckboxFlags("TableFlags_BordersV", &flags, TableFlags_BordersV);
    Gui::SameLine();
    HelpMarker("Using the _Resizable flag automatically enables the "
               "_BordersInnerV flag as well, this is why the resize borders "
               "are still showing when unchecking this.");
    PopStyleCompact();

    if (Gui::BeginTable("table1", 3, flags)) {
      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("Hello %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Resizable, fixed");
  if (Gui::TreeNode("Resizable, fixed")) {
    // Here we use TableFlags_SizingFixedFit (even though _ScrollX is not
    // set) So columns will adopt the "Fixed" policy and will maintain a fixed
    // width regardless of the whole available width (unless table is small) If
    // there is not enough available width to fit all columns, they will however
    // be resized down.
    // FIXME-TABLE: Providing a stretch-on-init would make sense especially for
    // tables which don't have saved settings
    HelpMarker(
        "Using _Resizable + _SizingFixedFit flags.\n"
        "Fixed-width columns generally makes more sense if you want to use "
        "horizontal scrolling.\n\n"
        "Double-click a column border to auto-fit the column to its contents.");
    PushStyleCompact();
    static TableFlags flags = TableFlags_SizingFixedFit | TableFlags_Resizable |
                              TableFlags_BordersOuter | TableFlags_BordersV |
                              TableFlags_ContextMenuInBody;
    Gui::CheckboxFlags("TableFlags_NoHostExtendX", &flags,
                       TableFlags_NoHostExtendX);
    PopStyleCompact();

    if (Gui::BeginTable("table1", 3, flags)) {
      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("Hello %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Resizable, mixed");
  if (Gui::TreeNode("Resizable, mixed")) {
    HelpMarker("Using TableSetupColumn() to alter resizing policy on a "
               "per-column basis.\n\n"
               "When combining Fixed and Stretch columns, generally you only "
               "want one, maybe two trailing columns to use _WidthStretch.");
    static TableFlags flags = TableFlags_SizingFixedFit | TableFlags_RowBg |
                              TableFlags_Borders | TableFlags_Resizable |
                              TableFlags_Reorderable | TableFlags_Hideable;

    if (Gui::BeginTable("table1", 3, flags)) {
      Gui::TableSetupColumn("AAA", TableColumnFlags_WidthFixed);
      Gui::TableSetupColumn("BBB", TableColumnFlags_WidthFixed);
      Gui::TableSetupColumn("CCC", TableColumnFlags_WidthStretch);
      Gui::TableHeadersRow();
      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("%s %d,%d", (column == 2) ? "Stretch" : "Fixed", column,
                    row);
        }
      }
      Gui::EndTable();
    }
    if (Gui::BeginTable("table2", 6, flags)) {
      Gui::TableSetupColumn("AAA", TableColumnFlags_WidthFixed);
      Gui::TableSetupColumn("BBB", TableColumnFlags_WidthFixed);
      Gui::TableSetupColumn("CCC", TableColumnFlags_WidthFixed |
                                       TableColumnFlags_DefaultHide);
      Gui::TableSetupColumn("DDD", TableColumnFlags_WidthStretch);
      Gui::TableSetupColumn("EEE", TableColumnFlags_WidthStretch);
      Gui::TableSetupColumn("FFF", TableColumnFlags_WidthStretch |
                                       TableColumnFlags_DefaultHide);
      Gui::TableHeadersRow();
      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 6; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("%s %d,%d", (column >= 3) ? "Stretch" : "Fixed", column,
                    row);
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Reorderable, hideable, with headers");
  if (Gui::TreeNode("Reorderable, hideable, with headers")) {
    HelpMarker("Click and drag column headers to reorder columns.\n\n"
               "Right-click on a header to open a context menu.");
    static TableFlags flags = TableFlags_Resizable | TableFlags_Reorderable |
                              TableFlags_Hideable | TableFlags_BordersOuter |
                              TableFlags_BordersV;
    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_Resizable", &flags, TableFlags_Resizable);
    Gui::CheckboxFlags("TableFlags_Reorderable", &flags,
                       TableFlags_Reorderable);
    Gui::CheckboxFlags("TableFlags_Hideable", &flags, TableFlags_Hideable);
    Gui::CheckboxFlags("TableFlags_NoBordersInBody", &flags,
                       TableFlags_NoBordersInBody);
    Gui::CheckboxFlags("TableFlags_NoBordersInBodyUntilResize", &flags,
                       TableFlags_NoBordersInBodyUntilResize);
    Gui::SameLine();
    HelpMarker("Disable vertical borders in columns Body until hovered for "
               "resize (borders will always appear in Headers)");
    Gui::CheckboxFlags("TableFlags_HighlightHoveredColumn", &flags,
                       TableFlags_HighlightHoveredColumn);
    PopStyleCompact();

    if (Gui::BeginTable("table1", 3, flags)) {
      // Submit columns name with TableSetupColumn() and call TableHeadersRow()
      // to create a row with a header in each column. (Later we will show how
      // TableSetupColumn() has other uses, optional flags, sizing weight etc.)
      Gui::TableSetupColumn("One");
      Gui::TableSetupColumn("Two");
      Gui::TableSetupColumn("Three");
      Gui::TableHeadersRow();
      for (int row = 0; row < 6; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("Hello %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }

    // Use outer_size.x == 0.0f instead of default to make the table as tight as
    // possible (only valid when no scrolling and no stretch column)
    if (Gui::BeginTable("table2", 3, flags | TableFlags_SizingFixedFit,
                        Vec2(0.0f, 0.0f))) {
      Gui::TableSetupColumn("One");
      Gui::TableSetupColumn("Two");
      Gui::TableSetupColumn("Three");
      Gui::TableHeadersRow();
      for (int row = 0; row < 6; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("Fixed %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Padding");
  if (Gui::TreeNode("Padding")) {
    // First example: showcase use of padding flags and effect of
    // BorderOuterV/BorderInnerV on X padding. We don't expose
    // BorderOuterH/BorderInnerH here because they have no effect on X padding.
    HelpMarker("We often want outer padding activated when any using features "
               "which makes the edges of a column visible:\n"
               "e.g.:\n"
               "- BorderOuterV\n"
               "- any form of row selection\n"
               "Because of this, activating BorderOuterV sets the default to "
               "PadOuterX. Using PadOuterX or NoPadOuterX you can override the "
               "default.\n\n"
               "Actual padding values are using style.CellPadding.\n\n"
               "In this demo we don't show horizontal borders to emphasize how "
               "they don't affect default horizontal padding.");

    static TableFlags flags1 = TableFlags_BordersV;
    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_PadOuterX", &flags1, TableFlags_PadOuterX);
    Gui::SameLine();
    HelpMarker("Enable outer-most padding (default if "
               "TableFlags_BordersOuterV is set)");
    Gui::CheckboxFlags("TableFlags_NoPadOuterX", &flags1,
                       TableFlags_NoPadOuterX);
    Gui::SameLine();
    HelpMarker("Disable outer-most padding (default if "
               "TableFlags_BordersOuterV is not set)");
    Gui::CheckboxFlags("TableFlags_NoPadInnerX", &flags1,
                       TableFlags_NoPadInnerX);
    Gui::SameLine();
    HelpMarker(
        "Disable inner padding between columns (double inner padding if "
        "BordersOuterV is on, single inner padding if BordersOuterV is off)");
    Gui::CheckboxFlags("TableFlags_BordersOuterV", &flags1,
                       TableFlags_BordersOuterV);
    Gui::CheckboxFlags("TableFlags_BordersInnerV", &flags1,
                       TableFlags_BordersInnerV);
    static bool show_headers = false;
    Gui::Checkbox("show_headers", &show_headers);
    PopStyleCompact();

    if (Gui::BeginTable("table_padding", 3, flags1)) {
      if (show_headers) {
        Gui::TableSetupColumn("One");
        Gui::TableSetupColumn("Two");
        Gui::TableSetupColumn("Three");
        Gui::TableHeadersRow();
      }

      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          if (row == 0) {
            Gui::Text("Avail %.2f", Gui::GetContentRegionAvail().x);
          } else {
            char buf[32];
            sprintf(buf, "Hello %d,%d", column, row);
            Gui::Button(buf, Vec2(-FLT_MIN, 0.0f));
          }
          // if (Gui::TableGetColumnFlags() & TableColumnFlags_IsHovered)
          //     Gui::TableSetBgColor(TableBgTarget_CellBg, COL32(0,
          //     100, 0, 255));
        }
      }
      Gui::EndTable();
    }

    // Second example: set style.CellPadding to (0.0) or a custom value.
    // FIXME-TABLE: Vertical border effectively not displayed the same way as
    // horizontal one...
    HelpMarker("Setting style.CellPadding to (0,0) or a custom value.");
    static TableFlags flags2 = TableFlags_Borders | TableFlags_RowBg;
    static Vec2 cell_padding(0.0f, 0.0f);
    static bool show_widget_frame_bg = true;

    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_Borders", &flags2, TableFlags_Borders);
    Gui::CheckboxFlags("TableFlags_BordersH", &flags2, TableFlags_BordersH);
    Gui::CheckboxFlags("TableFlags_BordersV", &flags2, TableFlags_BordersV);
    Gui::CheckboxFlags("TableFlags_BordersInner", &flags2,
                       TableFlags_BordersInner);
    Gui::CheckboxFlags("TableFlags_BordersOuter", &flags2,
                       TableFlags_BordersOuter);
    Gui::CheckboxFlags("TableFlags_RowBg", &flags2, TableFlags_RowBg);
    Gui::CheckboxFlags("TableFlags_Resizable", &flags2, TableFlags_Resizable);
    Gui::Checkbox("show_widget_frame_bg", &show_widget_frame_bg);
    Gui::SliderFloat2("CellPadding", &cell_padding.x, 0.0f, 10.0f, "%.0f");
    PopStyleCompact();

    Gui::PushStyleVar(StyleVar_CellPadding, cell_padding);
    if (Gui::BeginTable("table_padding_2", 3, flags2)) {
      static char text_bufs[3 * 5][16]; // Mini text storage for 3x5 cells
      static bool init = true;
      if (!show_widget_frame_bg)
        Gui::PushStyleColor(Col_FrameBg, 0);
      for (int cell = 0; cell < 3 * 5; cell++) {
        Gui::TableNextColumn();
        if (init)
          strcpy(text_bufs[cell], "edit me");
        Gui::SetNextItemWidth(-FLT_MIN);
        Gui::PushID(cell);
        Gui::InputText("##cell", text_bufs[cell], ARRAYSIZE(text_bufs[cell]));
        Gui::PopID();
      }
      if (!show_widget_frame_bg)
        Gui::PopStyleColor();
      init = false;
      Gui::EndTable();
    }
    Gui::PopStyleVar();

    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Explicit widths");
  if (Gui::TreeNode("Sizing policies")) {
    static TableFlags flags1 = TableFlags_BordersV | TableFlags_BordersOuterH |
                               TableFlags_RowBg | TableFlags_ContextMenuInBody;
    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_Resizable", &flags1, TableFlags_Resizable);
    Gui::CheckboxFlags("TableFlags_NoHostExtendX", &flags1,
                       TableFlags_NoHostExtendX);
    PopStyleCompact();

    static TableFlags sizing_policy_flags[4] = {
        TableFlags_SizingFixedFit, TableFlags_SizingFixedSame,
        TableFlags_SizingStretchProp, TableFlags_SizingStretchSame};
    for (int table_n = 0; table_n < 4; table_n++) {
      Gui::PushID(table_n);
      Gui::SetNextItemWidth(TEXT_BASE_WIDTH * 30);
      EditTableSizingFlags(&sizing_policy_flags[table_n]);

      // To make it easier to understand the different sizing policy,
      // For each policy: we display one table where the columns have equal
      // contents width, and one where the columns have different contents
      // width.
      if (Gui::BeginTable("table1", 3, sizing_policy_flags[table_n] | flags1)) {
        for (int row = 0; row < 3; row++) {
          Gui::TableNextRow();
          Gui::TableNextColumn();
          Gui::Text("Oh dear");
          Gui::TableNextColumn();
          Gui::Text("Oh dear");
          Gui::TableNextColumn();
          Gui::Text("Oh dear");
        }
        Gui::EndTable();
      }
      if (Gui::BeginTable("table2", 3, sizing_policy_flags[table_n] | flags1)) {
        for (int row = 0; row < 3; row++) {
          Gui::TableNextRow();
          Gui::TableNextColumn();
          Gui::Text("AAAA");
          Gui::TableNextColumn();
          Gui::Text("BBBBBBBB");
          Gui::TableNextColumn();
          Gui::Text("CCCCCCCCCCCC");
        }
        Gui::EndTable();
      }
      Gui::PopID();
    }

    Gui::Spacing();
    Gui::TextUnformatted("Advanced");
    Gui::SameLine();
    HelpMarker("This section allows you to interact and see the effect of "
               "various sizing policies depending on whether Scroll is enabled "
               "and the contents of your columns.");

    enum ContentsType {
      CT_ShowWidth,
      CT_ShortText,
      CT_LongText,
      CT_Button,
      CT_FillButton,
      CT_InputText
    };
    static TableFlags flags = TableFlags_ScrollY | TableFlags_Borders |
                              TableFlags_RowBg | TableFlags_Resizable;
    static int contents_type = CT_ShowWidth;
    static int column_count = 3;

    PushStyleCompact();
    Gui::PushID("Advanced");
    Gui::PushItemWidth(TEXT_BASE_WIDTH * 30);
    EditTableSizingFlags(&flags);
    Gui::Combo(
        "Contents", &contents_type,
        "Show width\0Short Text\0Long Text\0Button\0Fill Button\0InputText\0");
    if (contents_type == CT_FillButton) {
      Gui::SameLine();
      HelpMarker("Be mindful that using right-alignment (e.g. size.x = "
                 "-FLT_MIN) creates a feedback loop where contents width can "
                 "feed into auto-column width can feed into contents width.");
    }
    Gui::DragInt("Columns", &column_count, 0.1f, 1, 64, "%d",
                 SliderFlags_AlwaysClamp);
    Gui::CheckboxFlags("TableFlags_Resizable", &flags, TableFlags_Resizable);
    Gui::CheckboxFlags("TableFlags_PreciseWidths", &flags,
                       TableFlags_PreciseWidths);
    Gui::SameLine();
    HelpMarker("Disable distributing remainder width to stretched columns "
               "(width allocation on a 100-wide table with 3 columns: Without "
               "this flag: 33,33,34. With this flag: 33,33,33). With larger "
               "number of columns, resizing will appear to be less smooth.");
    Gui::CheckboxFlags("TableFlags_ScrollX", &flags, TableFlags_ScrollX);
    Gui::CheckboxFlags("TableFlags_ScrollY", &flags, TableFlags_ScrollY);
    Gui::CheckboxFlags("TableFlags_NoClip", &flags, TableFlags_NoClip);
    Gui::PopItemWidth();
    Gui::PopID();
    PopStyleCompact();

    if (Gui::BeginTable("table2", column_count, flags,
                        Vec2(0.0f, TEXT_BASE_HEIGHT * 7))) {
      for (int cell = 0; cell < 10 * column_count; cell++) {
        Gui::TableNextColumn();
        int column = Gui::TableGetColumnIndex();
        int row = Gui::TableGetRowIndex();

        Gui::PushID(cell);
        char label[32];
        static char text_buf[32] = "";
        sprintf(label, "Hello %d,%d", column, row);
        switch (contents_type) {
        case CT_ShortText:
          Gui::TextUnformatted(label);
          break;
        case CT_LongText:
          Gui::Text("Some %s text %d,%d\nOver two lines..",
                    column == 0 ? "long" : "longeeer", column, row);
          break;
        case CT_ShowWidth:
          Gui::Text("W: %.1f", Gui::GetContentRegionAvail().x);
          break;
        case CT_Button:
          Gui::Button(label);
          break;
        case CT_FillButton:
          Gui::Button(label, Vec2(-FLT_MIN, 0.0f));
          break;
        case CT_InputText:
          Gui::SetNextItemWidth(-FLT_MIN);
          Gui::InputText("##", text_buf, ARRAYSIZE(text_buf));
          break;
        }
        Gui::PopID();
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Vertical scrolling, with clipping");
  if (Gui::TreeNode("Vertical scrolling, with clipping")) {
    HelpMarker(
        "Here we activate ScrollY, which will create a child window container "
        "to allow hosting scrollable contents.\n\nWe also demonstrate using "
        "ListClipper to virtualize the submission of many items.");
    static TableFlags flags = TableFlags_ScrollY | TableFlags_RowBg |
                              TableFlags_BordersOuter | TableFlags_BordersV |
                              TableFlags_Resizable | TableFlags_Reorderable |
                              TableFlags_Hideable;

    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_ScrollY", &flags, TableFlags_ScrollY);
    PopStyleCompact();

    // When using ScrollX or ScrollY we need to specify a size for our table
    // container! Otherwise by default the table will fit all available space,
    // like a BeginChild() call.
    Vec2 outer_size = Vec2(0.0f, TEXT_BASE_HEIGHT * 8);
    if (Gui::BeginTable("table_scrolly", 3, flags, outer_size)) {
      Gui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      Gui::TableSetupColumn("One", TableColumnFlags_None);
      Gui::TableSetupColumn("Two", TableColumnFlags_None);
      Gui::TableSetupColumn("Three", TableColumnFlags_None);
      Gui::TableHeadersRow();

      // Demonstrate using clipper for large vertical lists
      ListClipper clipper;
      clipper.Begin(1000);
      while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
          Gui::TableNextRow();
          for (int column = 0; column < 3; column++) {
            Gui::TableSetColumnIndex(column);
            Gui::Text("Hello %d,%d", column, row);
          }
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Horizontal scrolling");
  if (Gui::TreeNode("Horizontal scrolling")) {
    HelpMarker(
        "When ScrollX is enabled, the default sizing policy becomes "
        "TableFlags_SizingFixedFit, "
        "as automatically stretching columns doesn't make much sense with "
        "horizontal scrolling.\n\n"
        "Also note that as of the current version, you will almost always want "
        "to enable ScrollY along with ScrollX,"
        "because the container window won't automatically extend vertically to "
        "fix contents (this may be improved in future versions).");
    static TableFlags flags = TableFlags_ScrollX | TableFlags_ScrollY |
                              TableFlags_RowBg | TableFlags_BordersOuter |
                              TableFlags_BordersV | TableFlags_Resizable |
                              TableFlags_Reorderable | TableFlags_Hideable;
    static int freeze_cols = 1;
    static int freeze_rows = 1;

    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_Resizable", &flags, TableFlags_Resizable);
    Gui::CheckboxFlags("TableFlags_ScrollX", &flags, TableFlags_ScrollX);
    Gui::CheckboxFlags("TableFlags_ScrollY", &flags, TableFlags_ScrollY);
    Gui::SetNextItemWidth(Gui::GetFrameHeight());
    Gui::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL,
                 SliderFlags_NoInput);
    Gui::SetNextItemWidth(Gui::GetFrameHeight());
    Gui::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL,
                 SliderFlags_NoInput);
    PopStyleCompact();

    // When using ScrollX or ScrollY we need to specify a size for our table
    // container! Otherwise by default the table will fit all available space,
    // like a BeginChild() call.
    Vec2 outer_size = Vec2(0.0f, TEXT_BASE_HEIGHT * 8);
    if (Gui::BeginTable("table_scrollx", 7, flags, outer_size)) {
      Gui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
      Gui::TableSetupColumn(
          "Line #",
          TableColumnFlags_NoHide); // Make the first column not hideable
                                    // to match our use of
                                    // TableSetupScrollFreeze()
      Gui::TableSetupColumn("One");
      Gui::TableSetupColumn("Two");
      Gui::TableSetupColumn("Three");
      Gui::TableSetupColumn("Four");
      Gui::TableSetupColumn("Five");
      Gui::TableSetupColumn("Six");
      Gui::TableHeadersRow();
      for (int row = 0; row < 20; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 7; column++) {
          // Both TableNextColumn() and TableSetColumnIndex() return true when a
          // column is visible or performing width measurement. Because here we
          // know that:
          // - A) all our columns are contributing the same to row height
          // - B) column 0 is always visible,
          // We only always submit this one column and can skip others.
          // More advanced per-column clipping behaviors may benefit from
          // polling the status flags via TableGetColumnFlags().
          if (!Gui::TableSetColumnIndex(column) && column > 0)
            continue;
          if (column == 0)
            Gui::Text("Line %d", row);
          else
            Gui::Text("Hello world %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }

    Gui::Spacing();
    Gui::TextUnformatted("Stretch + ScrollX");
    Gui::SameLine();
    HelpMarker("Showcase using Stretch columns + ScrollX together: "
               "this is rather unusual and only makes sense when specifying an "
               "'inner_width' for the table!\n"
               "Without an explicit value, inner_width is == outer_size.x and "
               "therefore using Stretch columns + ScrollX together doesn't "
               "make sense.");
    static TableFlags flags2 = TableFlags_SizingStretchSame |
                               TableFlags_ScrollX | TableFlags_ScrollY |
                               TableFlags_BordersOuter | TableFlags_RowBg |
                               TableFlags_ContextMenuInBody;
    static float inner_width = 1000.0f;
    PushStyleCompact();
    Gui::PushID("flags3");
    Gui::PushItemWidth(TEXT_BASE_WIDTH * 30);
    Gui::CheckboxFlags("TableFlags_ScrollX", &flags2, TableFlags_ScrollX);
    Gui::DragFloat("inner_width", &inner_width, 1.0f, 0.0f, FLT_MAX, "%.1f");
    Gui::PopItemWidth();
    Gui::PopID();
    PopStyleCompact();
    if (Gui::BeginTable("table2", 7, flags2, outer_size, inner_width)) {
      for (int cell = 0; cell < 20 * 7; cell++) {
        Gui::TableNextColumn();
        Gui::Text("Hello world %d,%d", Gui::TableGetColumnIndex(),
                  Gui::TableGetRowIndex());
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Columns flags");
  if (Gui::TreeNode("Columns flags")) {
    // Create a first table just to show all the options/flags we want to make
    // visible in our example!
    const int column_count = 3;
    const char *column_names[column_count] = {"One", "Two", "Three"};
    static TableColumnFlags column_flags[column_count] = {
        TableColumnFlags_DefaultSort, TableColumnFlags_None,
        TableColumnFlags_DefaultHide};
    static TableColumnFlags column_flags_out[column_count] = {
        0, 0, 0}; // Output from TableGetColumnFlags()

    if (Gui::BeginTable("table_columns_flags_checkboxes", column_count,
                        TableFlags_None)) {
      PushStyleCompact();
      for (int column = 0; column < column_count; column++) {
        Gui::TableNextColumn();
        Gui::PushID(column);
        Gui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong
                                        // text baseline propagation across
                                        // columns
        Gui::Text("'%s'", column_names[column]);
        Gui::Spacing();
        Gui::Text("Input flags:");
        EditTableColumnsFlags(&column_flags[column]);
        Gui::Spacing();
        Gui::Text("Output flags:");
        Gui::BeginDisabled();
        ShowTableColumnsStatusFlags(column_flags_out[column]);
        Gui::EndDisabled();
        Gui::PopID();
      }
      PopStyleCompact();
      Gui::EndTable();
    }

    // Create the real table we care about for the example!
    // We use a scrolling table to be able to showcase the difference between
    // the _IsEnabled and _IsVisible flags above, otherwise in a non-scrolling
    // table columns are always visible (unless using
    // TableFlags_NoKeepColumnsVisible + resizing the parent window down)
    const TableFlags flags = TableFlags_SizingFixedFit | TableFlags_ScrollX |
                             TableFlags_ScrollY | TableFlags_RowBg |
                             TableFlags_BordersOuter | TableFlags_BordersV |
                             TableFlags_Resizable | TableFlags_Reorderable |
                             TableFlags_Hideable | TableFlags_Sortable;
    Vec2 outer_size = Vec2(0.0f, TEXT_BASE_HEIGHT * 9);
    if (Gui::BeginTable("table_columns_flags", column_count, flags,
                        outer_size)) {
      bool has_angled_header = false;
      for (int column = 0; column < column_count; column++) {
        has_angled_header |=
            (column_flags[column] & TableColumnFlags_AngledHeader) != 0;
        Gui::TableSetupColumn(column_names[column], column_flags[column]);
      }
      if (has_angled_header)
        Gui::TableAngledHeadersRow();
      Gui::TableHeadersRow();
      for (int column = 0; column < column_count; column++)
        column_flags_out[column] = Gui::TableGetColumnFlags(column);
      float indent_step = (float)((int)TEXT_BASE_WIDTH / 2);
      for (int row = 0; row < 8; row++) {
        Gui::Indent(
            indent_step); // Add some indentation to demonstrate usage of
                          // per-column IndentEnable/IndentDisable flags.
        Gui::TableNextRow();
        for (int column = 0; column < column_count; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("%s %s", (column == 0) ? "Indented" : "Hello",
                    Gui::TableGetColumnName(column));
        }
      }
      Gui::Unindent(indent_step * 8.0f);

      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Columns widths");
  if (Gui::TreeNode("Columns widths")) {
    HelpMarker("Using TableSetupColumn() to setup default width.");

    static TableFlags flags1 =
        TableFlags_Borders | TableFlags_NoBordersInBodyUntilResize;
    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_Resizable", &flags1, TableFlags_Resizable);
    Gui::CheckboxFlags("TableFlags_NoBordersInBodyUntilResize", &flags1,
                       TableFlags_NoBordersInBodyUntilResize);
    PopStyleCompact();
    if (Gui::BeginTable("table1", 3, flags1)) {
      // We could also set TableFlags_SizingFixedFit on the table and all
      // columns will default to TableColumnFlags_WidthFixed.
      Gui::TableSetupColumn("one", TableColumnFlags_WidthFixed,
                            100.0f); // Default to 100.0f
      Gui::TableSetupColumn("two", TableColumnFlags_WidthFixed,
                            200.0f); // Default to 200.0f
      Gui::TableSetupColumn("three",
                            TableColumnFlags_WidthFixed); // Default to auto
      Gui::TableHeadersRow();
      for (int row = 0; row < 4; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableSetColumnIndex(column);
          if (row == 0)
            Gui::Text("(w: %5.1f)", Gui::GetContentRegionAvail().x);
          else
            Gui::Text("Hello %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }

    HelpMarker("Using TableSetupColumn() to setup explicit width.\n\nUnless "
               "_NoKeepColumnsVisible is set, fixed columns with set width may "
               "still be shrunk down if there's not enough space in the host.");

    static TableFlags flags2 = TableFlags_None;
    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_NoKeepColumnsVisible", &flags2,
                       TableFlags_NoKeepColumnsVisible);
    Gui::CheckboxFlags("TableFlags_BordersInnerV", &flags2,
                       TableFlags_BordersInnerV);
    Gui::CheckboxFlags("TableFlags_BordersOuterV", &flags2,
                       TableFlags_BordersOuterV);
    PopStyleCompact();
    if (Gui::BeginTable("table2", 4, flags2)) {
      // We could also set TableFlags_SizingFixedFit on the table and all
      // columns will default to TableColumnFlags_WidthFixed.
      Gui::TableSetupColumn("", TableColumnFlags_WidthFixed, 100.0f);
      Gui::TableSetupColumn("", TableColumnFlags_WidthFixed,
                            TEXT_BASE_WIDTH * 15.0f);
      Gui::TableSetupColumn("", TableColumnFlags_WidthFixed,
                            TEXT_BASE_WIDTH * 30.0f);
      Gui::TableSetupColumn("", TableColumnFlags_WidthFixed,
                            TEXT_BASE_WIDTH * 15.0f);
      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 4; column++) {
          Gui::TableSetColumnIndex(column);
          if (row == 0)
            Gui::Text("(w: %5.1f)", Gui::GetContentRegionAvail().x);
          else
            Gui::Text("Hello %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Nested tables");
  if (Gui::TreeNode("Nested tables")) {
    HelpMarker("This demonstrates embedding a table into another table cell.");

    if (Gui::BeginTable("table_nested1", 2,
                        TableFlags_Borders | TableFlags_Resizable |
                            TableFlags_Reorderable | TableFlags_Hideable)) {
      Gui::TableSetupColumn("A0");
      Gui::TableSetupColumn("A1");
      Gui::TableHeadersRow();

      Gui::TableNextColumn();
      Gui::Text("A0 Row 0");
      {
        float rows_height = TEXT_BASE_HEIGHT * 2;
        if (Gui::BeginTable("table_nested2", 2,
                            TableFlags_Borders | TableFlags_Resizable |
                                TableFlags_Reorderable | TableFlags_Hideable)) {
          Gui::TableSetupColumn("B0");
          Gui::TableSetupColumn("B1");
          Gui::TableHeadersRow();

          Gui::TableNextRow(TableRowFlags_None, rows_height);
          Gui::TableNextColumn();
          Gui::Text("B0 Row 0");
          Gui::TableNextColumn();
          Gui::Text("B1 Row 0");
          Gui::TableNextRow(TableRowFlags_None, rows_height);
          Gui::TableNextColumn();
          Gui::Text("B0 Row 1");
          Gui::TableNextColumn();
          Gui::Text("B1 Row 1");

          Gui::EndTable();
        }
      }
      Gui::TableNextColumn();
      Gui::Text("A1 Row 0");
      Gui::TableNextColumn();
      Gui::Text("A0 Row 1");
      Gui::TableNextColumn();
      Gui::Text("A1 Row 1");
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Row height");
  if (Gui::TreeNode("Row height")) {
    HelpMarker(
        "You can pass a 'min_row_height' to TableNextRow().\n\nRows are padded "
        "with 'style.CellPadding.y' on top and bottom, so effectively the "
        "minimum row height will always be >= 'style.CellPadding.y * "
        "2.0f'.\n\nWe cannot honor a _maximum_ row height as that would "
        "require a unique clipping rectangle per row.");
    if (Gui::BeginTable("table_row_height", 1, TableFlags_Borders)) {
      for (int row = 0; row < 8; row++) {
        float min_row_height = (float)(int)(TEXT_BASE_HEIGHT * 0.30f * row);
        Gui::TableNextRow(TableRowFlags_None, min_row_height);
        Gui::TableNextColumn();
        Gui::Text("min_row_height = %.2f", min_row_height);
      }
      Gui::EndTable();
    }

    HelpMarker(
        "Showcase using SameLine(0,0) to share Current Line Height between "
        "cells.\n\nPlease note that Tables Row Height is not the same thing as "
        "Current Line Height, as a table cell may contains multiple lines.");
    if (Gui::BeginTable("table_share_lineheight", 2, TableFlags_Borders)) {
      Gui::TableNextRow();
      Gui::TableNextColumn();
      Gui::ColorButton("##1", Vec4(0.13f, 0.26f, 0.40f, 1.0f),
                       ColorEditFlags_None, Vec2(40, 40));
      Gui::TableNextColumn();
      Gui::Text("Line 1");
      Gui::Text("Line 2");

      Gui::TableNextRow();
      Gui::TableNextColumn();
      Gui::ColorButton("##2", Vec4(0.13f, 0.26f, 0.40f, 1.0f),
                       ColorEditFlags_None, Vec2(40, 40));
      Gui::TableNextColumn();
      Gui::SameLine(0.0f, 0.0f); // Reuse line height from previous column
      Gui::Text("Line 1, with SameLine(0,0)");
      Gui::Text("Line 2");

      Gui::EndTable();
    }

    HelpMarker("Showcase altering CellPadding.y between rows. Note that "
               "CellPadding.x is locked for the entire table.");
    if (Gui::BeginTable("table_changing_cellpadding_y", 1,
                        TableFlags_Borders)) {
      Style &style = Gui::GetStyle();
      for (int row = 0; row < 8; row++) {
        if ((row % 3) == 2)
          Gui::PushStyleVar(StyleVar_CellPadding,
                            Vec2(style.CellPadding.x, 20.0f));
        Gui::TableNextRow(TableRowFlags_None);
        Gui::TableNextColumn();
        Gui::Text("CellPadding.y = %.2f", style.CellPadding.y);
        if ((row % 3) == 2)
          Gui::PopStyleVar();
      }
      Gui::EndTable();
    }

    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Outer size");
  if (Gui::TreeNode("Outer size")) {
    // Showcasing use of TableFlags_NoHostExtendX and
    // TableFlags_NoHostExtendY Important to that note how the two flags
    // have slightly different behaviors!
    Gui::Text("Using NoHostExtendX and NoHostExtendY:");
    PushStyleCompact();
    static TableFlags flags = TableFlags_Borders | TableFlags_Resizable |
                              TableFlags_ContextMenuInBody | TableFlags_RowBg |
                              TableFlags_SizingFixedFit |
                              TableFlags_NoHostExtendX;
    Gui::CheckboxFlags("TableFlags_NoHostExtendX", &flags,
                       TableFlags_NoHostExtendX);
    Gui::SameLine();
    HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x "
               "value.\n\nOnly available when ScrollX/ScrollY are disabled and "
               "Stretch columns are not used.");
    Gui::CheckboxFlags("TableFlags_NoHostExtendY", &flags,
                       TableFlags_NoHostExtendY);
    Gui::SameLine();
    HelpMarker("Make outer height stop exactly at outer_size.y (prevent "
               "auto-extending table past the limit).\n\nOnly available when "
               "ScrollX/ScrollY are disabled. Data below the limit will be "
               "clipped and not visible.");
    PopStyleCompact();

    Vec2 outer_size = Vec2(0.0f, TEXT_BASE_HEIGHT * 5.5f);
    if (Gui::BeginTable("table1", 3, flags, outer_size)) {
      for (int row = 0; row < 10; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableNextColumn();
          Gui::Text("Cell %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }
    Gui::SameLine();
    Gui::Text("Hello!");

    Gui::Spacing();

    Gui::Text("Using explicit size:");
    if (Gui::BeginTable("table2", 3, TableFlags_Borders | TableFlags_RowBg,
                        Vec2(TEXT_BASE_WIDTH * 30, 0.0f))) {
      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          Gui::TableNextColumn();
          Gui::Text("Cell %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }
    Gui::SameLine();
    if (Gui::BeginTable("table3", 3, TableFlags_Borders | TableFlags_RowBg,
                        Vec2(TEXT_BASE_WIDTH * 30, 0.0f))) {
      for (int row = 0; row < 3; row++) {
        Gui::TableNextRow(0, TEXT_BASE_HEIGHT * 1.5f);
        for (int column = 0; column < 3; column++) {
          Gui::TableNextColumn();
          Gui::Text("Cell %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }

    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Background color");
  if (Gui::TreeNode("Background color")) {
    static TableFlags flags = TableFlags_RowBg;
    static int row_bg_type = 1;
    static int row_bg_target = 1;
    static int cell_bg_type = 1;

    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_Borders", &flags, TableFlags_Borders);
    Gui::CheckboxFlags("TableFlags_RowBg", &flags, TableFlags_RowBg);
    Gui::SameLine();
    HelpMarker("TableFlags_RowBg automatically sets RowBg0 to alternative "
               "colors pulled from the Style.");
    Gui::Combo("row bg type", (int *)&row_bg_type, "None\0Red\0Gradient\0");
    Gui::Combo("row bg target", (int *)&row_bg_target, "RowBg0\0RowBg1\0");
    Gui::SameLine();
    HelpMarker("Target RowBg0 to override the alternating odd/even "
               "colors,\nTarget RowBg1 to blend with them.");
    Gui::Combo("cell bg type", (int *)&cell_bg_type, "None\0Blue\0");
    Gui::SameLine();
    HelpMarker("We are colorizing cells to B1->C2 here.");
    ASSERT(row_bg_type >= 0 && row_bg_type <= 2);
    ASSERT(row_bg_target >= 0 && row_bg_target <= 1);
    ASSERT(cell_bg_type >= 0 && cell_bg_type <= 1);
    PopStyleCompact();

    if (Gui::BeginTable("table1", 5, flags)) {
      for (int row = 0; row < 6; row++) {
        Gui::TableNextRow();

        // Demonstrate setting a row background color with
        // 'Gui::TableSetBgColor(TableBgTarget_RowBgX, ...)' We use a
        // transparent color so we can see the one behind in case our target is
        // RowBg1 and RowBg0 was already targeted by the TableFlags_RowBg
        // flag.
        if (row_bg_type != 0) {
          U32 row_bg_color = Gui::GetColorU32(
              row_bg_type == 1 ? Vec4(0.7f, 0.3f, 0.3f, 0.65f)
                               : Vec4(0.2f + row * 0.1f, 0.2f, 0.2f,
                                      0.65f)); // Flat or Gradient?
          Gui::TableSetBgColor(TableBgTarget_RowBg0 + row_bg_target,
                               row_bg_color);
        }

        // Fill cells
        for (int column = 0; column < 5; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("%c%c", 'A' + row, '0' + column);

          // Change background of Cells B1->C2
          // Demonstrate setting a cell background color with
          // 'Gui::TableSetBgColor(TableBgTarget_CellBg, ...)' (the
          // CellBg color will be blended over the RowBg and ColumnBg colors) We
          // can also pass a column number as a third parameter to
          // TableSetBgColor() and do this outside the column loop.
          if (row >= 1 && row <= 2 && column >= 1 && column <= 2 &&
              cell_bg_type == 1) {
            U32 cell_bg_color = Gui::GetColorU32(Vec4(0.3f, 0.3f, 0.7f, 0.65f));
            Gui::TableSetBgColor(TableBgTarget_CellBg, cell_bg_color);
          }
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Tree view");
  if (Gui::TreeNode("Tree view")) {
    static TableFlags flags = TableFlags_BordersV | TableFlags_BordersOuterH |
                              TableFlags_Resizable | TableFlags_RowBg |
                              TableFlags_NoBordersInBody;

    static TreeNodeFlags tree_node_flags = TreeNodeFlags_SpanAllColumns;
    Gui::CheckboxFlags("TreeNodeFlags_SpanFullWidth", &tree_node_flags,
                       TreeNodeFlags_SpanFullWidth);
    Gui::CheckboxFlags("TreeNodeFlags_SpanAllColumns", &tree_node_flags,
                       TreeNodeFlags_SpanAllColumns);

    HelpMarker("See \"Columns flags\" section to configure how indentation is "
               "applied to individual columns.");
    if (Gui::BeginTable("3ways", 3, flags)) {
      // The first column will use the default _WidthStretch when ScrollX is Off
      // and _WidthFixed when ScrollX is On
      Gui::TableSetupColumn("Name", TableColumnFlags_NoHide);
      Gui::TableSetupColumn("Size", TableColumnFlags_WidthFixed,
                            TEXT_BASE_WIDTH * 12.0f);
      Gui::TableSetupColumn("Type", TableColumnFlags_WidthFixed,
                            TEXT_BASE_WIDTH * 18.0f);
      Gui::TableHeadersRow();

      // Simple storage to output a dummy file-system.
      struct MyTreeNode {
        const char *Name;
        const char *Type;
        int Size;
        int ChildIdx;
        int ChildCount;
        static void DisplayNode(const MyTreeNode *node,
                                const MyTreeNode *all_nodes) {
          Gui::TableNextRow();
          Gui::TableNextColumn();
          const bool is_folder = (node->ChildCount > 0);
          if (is_folder) {
            bool open = Gui::TreeNodeEx(node->Name, tree_node_flags);
            Gui::TableNextColumn();
            Gui::TextDisabled("--");
            Gui::TableNextColumn();
            Gui::TextUnformatted(node->Type);
            if (open) {
              for (int child_n = 0; child_n < node->ChildCount; child_n++)
                DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
              Gui::TreePop();
            }
          } else {
            Gui::TreeNodeEx(node->Name, tree_node_flags | TreeNodeFlags_Leaf |
                                            TreeNodeFlags_Bullet |
                                            TreeNodeFlags_NoTreePushOnOpen);
            Gui::TableNextColumn();
            Gui::Text("%d", node->Size);
            Gui::TableNextColumn();
            Gui::TextUnformatted(node->Type);
          }
        }
      };
      static const MyTreeNode nodes[] = {
          {"Root", "Folder", -1, 1, 3},                                    // 0
          {"Music", "Folder", -1, 4, 2},                                   // 1
          {"Textures", "Folder", -1, 6, 3},                                // 2
          {"desktop.ini", "System file", 1024, -1, -1},                    // 3
          {"File1_a.wav", "Audio file", 123000, -1, -1},                   // 4
          {"File1_b.wav", "Audio file", 456000, -1, -1},                   // 5
          {"Image001.png", "Image file", 203128, -1, -1},                  // 6
          {"Copy of Image001.png", "Image file", 203256, -1, -1},          // 7
          {"Copy of Image001 (Final2).png", "Image file", 203512, -1, -1}, // 8
      };

      MyTreeNode::DisplayNode(&nodes[0], nodes);

      Gui::EndTable();
    }
    Gui::TreePop();
  }

  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Item width");
  if (Gui::TreeNode("Item width")) {
    HelpMarker(
        "Showcase using PushItemWidth() and how it is preserved on a "
        "per-column basis.\n\n"
        "Note that on auto-resizing non-resizable fixed columns, querying the "
        "content width for e.g. right-alignment doesn't make sense.");
    if (Gui::BeginTable("table_item_width", 3, TableFlags_Borders)) {
      Gui::TableSetupColumn("small");
      Gui::TableSetupColumn("half");
      Gui::TableSetupColumn("right-align");
      Gui::TableHeadersRow();

      for (int row = 0; row < 3; row++) {
        Gui::TableNextRow();
        if (row == 0) {
          // Setup ItemWidth once (instead of setting up every time, which is
          // also possible but less efficient)
          Gui::TableSetColumnIndex(0);
          Gui::PushItemWidth(TEXT_BASE_WIDTH * 3.0f); // Small
          Gui::TableSetColumnIndex(1);
          Gui::PushItemWidth(-Gui::GetContentRegionAvail().x * 0.5f);
          Gui::TableSetColumnIndex(2);
          Gui::PushItemWidth(-FLT_MIN); // Right-aligned
        }

        // Draw our contents
        static float dummy_f = 0.0f;
        Gui::PushID(row);
        Gui::TableSetColumnIndex(0);
        Gui::SliderFloat("float0", &dummy_f, 0.0f, 1.0f);
        Gui::TableSetColumnIndex(1);
        Gui::SliderFloat("float1", &dummy_f, 0.0f, 1.0f);
        Gui::TableSetColumnIndex(2);
        Gui::SliderFloat("##float2", &dummy_f, 0.0f,
                         1.0f); // No visible label since right-aligned
        Gui::PopID();
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  // Demonstrate using TableHeader() calls instead of TableHeadersRow()
  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Custom headers");
  if (Gui::TreeNode("Custom headers")) {
    const int COLUMNS_COUNT = 3;
    if (Gui::BeginTable("table_custom_headers", COLUMNS_COUNT,
                        TableFlags_Borders | TableFlags_Reorderable |
                            TableFlags_Hideable)) {
      Gui::TableSetupColumn("Apricot");
      Gui::TableSetupColumn("Banana");
      Gui::TableSetupColumn("Cherry");

      // Dummy entire-column selection storage
      // FIXME: It would be nice to actually demonstrate full-featured selection
      // using those checkbox.
      static bool column_selected[3] = {};

      // Instead of calling TableHeadersRow() we'll submit custom headers
      // ourselves
      Gui::TableNextRow(TableRowFlags_Headers);
      for (int column = 0; column < COLUMNS_COUNT; column++) {
        Gui::TableSetColumnIndex(column);
        const char *column_name = Gui::TableGetColumnName(
            column); // Retrieve name passed to TableSetupColumn()
        Gui::PushID(column);
        Gui::PushStyleVar(StyleVar_FramePadding, Vec2(0, 0));
        Gui::Checkbox("##checkall", &column_selected[column]);
        Gui::PopStyleVar();
        Gui::SameLine(0.0f, Gui::GetStyle().ItemInnerSpacing.x);
        Gui::TableHeader(column_name);
        Gui::PopID();
      }

      for (int row = 0; row < 5; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < 3; column++) {
          char buf[32];
          sprintf(buf, "Cell %d,%d", column, row);
          Gui::TableSetColumnIndex(column);
          Gui::Selectable(buf, column_selected[column]);
        }
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  // Demonstrate using TableColumnFlags_AngledHeader flag to create angled
  // headers
  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Angled headers");
  if (Gui::TreeNode("Angled headers")) {
    const char *column_names[] = {"Track",   "cabasa",  "ride",    "smash",
                                  "tom-hi",  "tom-mid", "tom-low", "hihat-o",
                                  "hihat-c", "snare-s", "snare-c", "clap",
                                  "rim",     "kick"};
    const int columns_count = ARRAYSIZE(column_names);
    const int rows_count = 12;

    static TableFlags table_flags =
        TableFlags_SizingFixedFit | TableFlags_ScrollX | TableFlags_ScrollY |
        TableFlags_BordersOuter | TableFlags_BordersInnerH |
        TableFlags_Hideable | TableFlags_Resizable | TableFlags_Reorderable |
        TableFlags_HighlightHoveredColumn;
    static bool bools[columns_count * rows_count] =
        {}; // Dummy storage selection storage
    static int frozen_cols = 1;
    static int frozen_rows = 2;
    Gui::CheckboxFlags("_ScrollX", &table_flags, TableFlags_ScrollX);
    Gui::CheckboxFlags("_ScrollY", &table_flags, TableFlags_ScrollY);
    Gui::CheckboxFlags("_NoBordersInBody", &table_flags,
                       TableFlags_NoBordersInBody);
    Gui::CheckboxFlags("_HighlightHoveredColumn", &table_flags,
                       TableFlags_HighlightHoveredColumn);
    Gui::SetNextItemWidth(Gui::GetFontSize() * 8);
    Gui::SliderInt("Frozen columns", &frozen_cols, 0, 2);
    Gui::SetNextItemWidth(Gui::GetFontSize() * 8);
    Gui::SliderInt("Frozen rows", &frozen_rows, 0, 2);

    if (Gui::BeginTable("table_angled_headers", columns_count, table_flags,
                        Vec2(0.0f, TEXT_BASE_HEIGHT * 12))) {
      Gui::TableSetupColumn(column_names[0], TableColumnFlags_NoHide |
                                                 TableColumnFlags_NoReorder);
      for (int n = 1; n < columns_count; n++)
        Gui::TableSetupColumn(column_names[n], TableColumnFlags_AngledHeader |
                                                   TableColumnFlags_WidthFixed);
      Gui::TableSetupScrollFreeze(frozen_cols, frozen_rows);

      Gui::TableAngledHeadersRow(); // Draw angled headers for all columns
                                    // with the
                                    // TableColumnFlags_AngledHeader
                                    // flag.
      Gui::TableHeadersRow(); // Draw remaining headers and allow access to
                              // context-menu and other functions.
      for (int row = 0; row < rows_count; row++) {
        Gui::PushID(row);
        Gui::TableNextRow();
        Gui::TableSetColumnIndex(0);
        Gui::AlignTextToFramePadding();
        Gui::Text("Track %d", row);
        for (int column = 1; column < columns_count; column++)
          if (Gui::TableSetColumnIndex(column)) {
            Gui::PushID(column);
            Gui::Checkbox("", &bools[row * columns_count + column]);
            Gui::PopID();
          }
        Gui::PopID();
      }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  // Demonstrate creating custom context menus inside columns, while playing it
  // nice with context menus provided by TableHeadersRow()/TableHeader()
  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Context menus");
  if (Gui::TreeNode("Context menus")) {
    HelpMarker("By default, right-clicking over a "
               "TableHeadersRow()/TableHeader() line will open the default "
               "context-menu.\nUsing TableFlags_ContextMenuInBody we also "
               "allow right-clicking over columns body.");
    static TableFlags flags1 = TableFlags_Resizable | TableFlags_Reorderable |
                               TableFlags_Hideable | TableFlags_Borders |
                               TableFlags_ContextMenuInBody;

    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_ContextMenuInBody", &flags1,
                       TableFlags_ContextMenuInBody);
    PopStyleCompact();

    // Context Menus: first example
    // [1.1] Right-click on the TableHeadersRow() line to open the default table
    // context menu. [1.2] Right-click in columns also open the default table
    // context menu (if TableFlags_ContextMenuInBody is set)
    const int COLUMNS_COUNT = 3;
    if (Gui::BeginTable("table_context_menu", COLUMNS_COUNT, flags1)) {
      Gui::TableSetupColumn("One");
      Gui::TableSetupColumn("Two");
      Gui::TableSetupColumn("Three");

      // [1.1]] Right-click on the TableHeadersRow() line to open the default
      // table context menu.
      Gui::TableHeadersRow();

      // Submit dummy contents
      for (int row = 0; row < 4; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < COLUMNS_COUNT; column++) {
          Gui::TableSetColumnIndex(column);
          Gui::Text("Cell %d,%d", column, row);
        }
      }
      Gui::EndTable();
    }

    // Context Menus: second example
    // [2.1] Right-click on the TableHeadersRow() line to open the default table
    // context menu. [2.2] Right-click on the ".." to open a custom popup [2.3]
    // Right-click in columns to open another custom popup
    HelpMarker("Demonstrate mixing table context menu (over header), item "
               "context button (over button) and custom per-colum context menu "
               "(over column body).");
    TableFlags flags2 = TableFlags_Resizable | TableFlags_SizingFixedFit |
                        TableFlags_Reorderable | TableFlags_Hideable |
                        TableFlags_Borders;
    if (Gui::BeginTable("table_context_menu_2", COLUMNS_COUNT, flags2)) {
      Gui::TableSetupColumn("One");
      Gui::TableSetupColumn("Two");
      Gui::TableSetupColumn("Three");

      // [2.1] Right-click on the TableHeadersRow() line to open the default
      // table context menu.
      Gui::TableHeadersRow();
      for (int row = 0; row < 4; row++) {
        Gui::TableNextRow();
        for (int column = 0; column < COLUMNS_COUNT; column++) {
          // Submit dummy contents
          Gui::TableSetColumnIndex(column);
          Gui::Text("Cell %d,%d", column, row);
          Gui::SameLine();

          // [2.2] Right-click on the ".." to open a custom popup
          Gui::PushID(row * COLUMNS_COUNT + column);
          Gui::SmallButton("..");
          if (Gui::BeginPopupContextItem()) {
            Gui::Text("This is the popup for Button(\"..\") in Cell %d,%d",
                      column, row);
            if (Gui::Button("Close"))
              Gui::CloseCurrentPopup();
            Gui::EndPopup();
          }
          Gui::PopID();
        }
      }

      // [2.3] Right-click anywhere in columns to open another custom popup
      // (instead of testing for !IsAnyItemHovered() we could also call
      // OpenPopup() with PopupFlags_NoOpenOverExistingPopup to manage
      // popup priority as the popups triggers, here "are we hovering a column"
      // are overlapping)
      int hovered_column = -1;
      for (int column = 0; column < COLUMNS_COUNT + 1; column++) {
        Gui::PushID(column);
        if (Gui::TableGetColumnFlags(column) & TableColumnFlags_IsHovered)
          hovered_column = column;
        if (hovered_column == column && !Gui::IsAnyItemHovered() &&
            Gui::IsMouseReleased(1))
          Gui::OpenPopup("MyPopup");
        if (Gui::BeginPopup("MyPopup")) {
          if (column == COLUMNS_COUNT)
            Gui::Text("This is a custom popup for unused space after the "
                      "last column.");
          else
            Gui::Text("This is a custom popup for Column %d", column);
          if (Gui::Button("Close"))
            Gui::CloseCurrentPopup();
          Gui::EndPopup();
        }
        Gui::PopID();
      }

      Gui::EndTable();
      Gui::Text("Hovered column: %d", hovered_column);
    }
    Gui::TreePop();
  }

  // Demonstrate creating multiple tables with the same ID
  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Synced instances");
  if (Gui::TreeNode("Synced instances")) {
    HelpMarker("Multiple tables with the same identifier will share their "
               "settings, width, visibility, order etc.");

    static TableFlags flags = TableFlags_Resizable | TableFlags_Reorderable |
                              TableFlags_Hideable | TableFlags_Borders |
                              TableFlags_SizingFixedFit |
                              TableFlags_NoSavedSettings;
    Gui::CheckboxFlags("TableFlags_ScrollY", &flags, TableFlags_ScrollY);
    Gui::CheckboxFlags("TableFlags_SizingFixedFit", &flags,
                       TableFlags_SizingFixedFit);
    Gui::CheckboxFlags("TableFlags_HighlightHoveredColumn", &flags,
                       TableFlags_HighlightHoveredColumn);
    for (int n = 0; n < 3; n++) {
      char buf[32];
      sprintf(buf, "Synced Table %d", n);
      bool open = Gui::CollapsingHeader(buf, TreeNodeFlags_DefaultOpen);
      if (open && Gui::BeginTable(
                      "Table", 3, flags,
                      Vec2(0.0f, Gui::GetTextLineHeightWithSpacing() * 5))) {
        Gui::TableSetupColumn("One");
        Gui::TableSetupColumn("Two");
        Gui::TableSetupColumn("Three");
        Gui::TableHeadersRow();
        const int cell_count =
            (n == 1)
                ? 27
                : 9; // Make second table have a scrollbar to verify that
                     // additional decoration is not affecting column positions.
        for (int cell = 0; cell < cell_count; cell++) {
          Gui::TableNextColumn();
          Gui::Text("this cell %d", cell);
        }
        Gui::EndTable();
      }
    }
    Gui::TreePop();
  }

  // Demonstrate using Sorting facilities
  // This is a simplified version of the "Advanced" example, where we mostly
  // focus on the code necessary to handle sorting. Note that the "Advanced"
  // example also showcase manually triggering a sort (e.g. if item quantities
  // have been modified)
  static const char *template_items_names[] = {
      "Banana",     "Apple", "Cherry",  "Watermelon", "Grapefruit",
      "Strawberry", "Mango", "Kiwi",    "Orange",     "Pineapple",
      "Blueberry",  "Plum",  "Coconut", "Pear",       "Apricot"};
  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Sorting");
  if (Gui::TreeNode("Sorting")) {
    // Create item list
    static Vector<MyItem> items;
    if (items.Size == 0) {
      items.resize(50, MyItem());
      for (int n = 0; n < items.Size; n++) {
        const int template_n = n % ARRAYSIZE(template_items_names);
        MyItem &item = items[n];
        item.ID = n;
        item.Name = template_items_names[template_n];
        item.Quantity = (n * n - n) % 20; // Assign default quantities
      }
    }

    // Options
    static TableFlags flags = TableFlags_Resizable | TableFlags_Reorderable |
                              TableFlags_Hideable | TableFlags_Sortable |
                              TableFlags_SortMulti | TableFlags_RowBg |
                              TableFlags_BordersOuter | TableFlags_BordersV |
                              TableFlags_NoBordersInBody | TableFlags_ScrollY;
    PushStyleCompact();
    Gui::CheckboxFlags("TableFlags_SortMulti", &flags, TableFlags_SortMulti);
    Gui::SameLine();
    HelpMarker("When sorting is enabled: hold shift when clicking headers to "
               "sort on multiple column. TableGetSortSpecs() may return specs "
               "where (SpecsCount > 1).");
    Gui::CheckboxFlags("TableFlags_SortTristate", &flags,
                       TableFlags_SortTristate);
    Gui::SameLine();
    HelpMarker(
        "When sorting is enabled: allow no sorting, disable default sorting. "
        "TableGetSortSpecs() may return specs where (SpecsCount == 0).");
    PopStyleCompact();

    if (Gui::BeginTable("table_sorting", 4, flags,
                        Vec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f)) {
      // Declare columns
      // We use the "user_id" parameter of TableSetupColumn() to specify a user
      // id that will be stored in the sort specifications. This is so our sort
      // function can identify a column given our own identifier. We could also
      // identify them based on their index! Demonstrate using a mixture of
      // flags among available sort-related flags:
      // - TableColumnFlags_DefaultSort
      // - TableColumnFlags_NoSort / TableColumnFlags_NoSortAscending
      // / TableColumnFlags_NoSortDescending
      // - TableColumnFlags_PreferSortAscending /
      // TableColumnFlags_PreferSortDescending
      Gui::TableSetupColumn(
          "ID", TableColumnFlags_DefaultSort | TableColumnFlags_WidthFixed,
          0.0f, MyItemColumnID_ID);
      Gui::TableSetupColumn("Name", TableColumnFlags_WidthFixed, 0.0f,
                            MyItemColumnID_Name);
      Gui::TableSetupColumn(
          "Action", TableColumnFlags_NoSort | TableColumnFlags_WidthFixed, 0.0f,
          MyItemColumnID_Action);
      Gui::TableSetupColumn("Quantity",
                            TableColumnFlags_PreferSortDescending |
                                TableColumnFlags_WidthStretch,
                            0.0f, MyItemColumnID_Quantity);
      Gui::TableSetupScrollFreeze(0, 1); // Make row always visible
      Gui::TableHeadersRow();

      // Sort our data if sort specs have been changed!
      if (TableSortSpecs *sort_specs = Gui::TableGetSortSpecs())
        if (sort_specs->SpecsDirty) {
          MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
          sort_specs->SpecsDirty = false;
        }

      // Demonstrate using clipper for large vertical lists
      ListClipper clipper;
      clipper.Begin(items.Size);
      while (clipper.Step())
        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd;
             row_n++) {
          // Display a data item
          MyItem *item = &items[row_n];
          Gui::PushID(item->ID);
          Gui::TableNextRow();
          Gui::TableNextColumn();
          Gui::Text("%04d", item->ID);
          Gui::TableNextColumn();
          Gui::TextUnformatted(item->Name);
          Gui::TableNextColumn();
          Gui::SmallButton("None");
          Gui::TableNextColumn();
          Gui::Text("%d", item->Quantity);
          Gui::PopID();
        }
      Gui::EndTable();
    }
    Gui::TreePop();
  }

  // In this example we'll expose most table flags and settings.
  // For specific flags and settings refer to the corresponding section for more
  // detailed explanation. This section is mostly useful to experiment with
  // combining certain flags or settings with each others.
  // Gui::SetNextItemOpen(true, Cond_Once); // [DEBUG]
  if (open_action != -1)
    Gui::SetNextItemOpen(open_action != 0);
  DEMO_MARKER("Tables/Advanced");
  if (Gui::TreeNode("Advanced")) {
    static TableFlags flags =
        TableFlags_Resizable | TableFlags_Reorderable | TableFlags_Hideable |
        TableFlags_Sortable | TableFlags_SortMulti | TableFlags_RowBg |
        TableFlags_Borders | TableFlags_NoBordersInBody | TableFlags_ScrollX |
        TableFlags_ScrollY | TableFlags_SizingFixedFit;
    static TableColumnFlags columns_base_flags = TableColumnFlags_None;

    enum ContentsType {
      CT_Text,
      CT_Button,
      CT_SmallButton,
      CT_FillButton,
      CT_Selectable,
      CT_SelectableSpanRow
    };
    static int contents_type = CT_SelectableSpanRow;
    const char *contents_type_names[] = {
        "Text",       "Button",     "SmallButton",
        "FillButton", "Selectable", "Selectable (span row)"};
    static int freeze_cols = 1;
    static int freeze_rows = 1;
    static int items_count = ARRAYSIZE(template_items_names) * 2;
    static Vec2 outer_size_value = Vec2(0.0f, TEXT_BASE_HEIGHT * 12);
    static float row_min_height = 0.0f;          // Auto
    static float inner_width_with_scroll = 0.0f; // Auto-extend
    static bool outer_size_enabled = true;
    static bool show_headers = true;
    static bool show_wrapped_text = false;
    // static TextFilter filter;
    // Gui::SetNextItemOpen(true, Cond_Once); // FIXME-TABLE: Enabling
    // this results in initial clipped first pass on table which tend to affect
    // column sizing
    if (Gui::TreeNode("Options")) {
      // Make the UI compact because there are so many fields
      PushStyleCompact();
      Gui::PushItemWidth(TEXT_BASE_WIDTH * 28.0f);

      if (Gui::TreeNodeEx("Features:", TreeNodeFlags_DefaultOpen)) {
        Gui::CheckboxFlags("TableFlags_Resizable", &flags,
                           TableFlags_Resizable);
        Gui::CheckboxFlags("TableFlags_Reorderable", &flags,
                           TableFlags_Reorderable);
        Gui::CheckboxFlags("TableFlags_Hideable", &flags, TableFlags_Hideable);
        Gui::CheckboxFlags("TableFlags_Sortable", &flags, TableFlags_Sortable);
        Gui::CheckboxFlags("TableFlags_NoSavedSettings", &flags,
                           TableFlags_NoSavedSettings);
        Gui::CheckboxFlags("TableFlags_ContextMenuInBody", &flags,
                           TableFlags_ContextMenuInBody);
        Gui::TreePop();
      }

      if (Gui::TreeNodeEx("Decorations:", TreeNodeFlags_DefaultOpen)) {
        Gui::CheckboxFlags("TableFlags_RowBg", &flags, TableFlags_RowBg);
        Gui::CheckboxFlags("TableFlags_BordersV", &flags, TableFlags_BordersV);
        Gui::CheckboxFlags("TableFlags_BordersOuterV", &flags,
                           TableFlags_BordersOuterV);
        Gui::CheckboxFlags("TableFlags_BordersInnerV", &flags,
                           TableFlags_BordersInnerV);
        Gui::CheckboxFlags("TableFlags_BordersH", &flags, TableFlags_BordersH);
        Gui::CheckboxFlags("TableFlags_BordersOuterH", &flags,
                           TableFlags_BordersOuterH);
        Gui::CheckboxFlags("TableFlags_BordersInnerH", &flags,
                           TableFlags_BordersInnerH);
        Gui::CheckboxFlags("TableFlags_NoBordersInBody", &flags,
                           TableFlags_NoBordersInBody);
        Gui::SameLine();
        HelpMarker("Disable vertical borders in columns Body (borders will "
                   "always appear in Headers");
        Gui::CheckboxFlags("TableFlags_NoBordersInBodyUntilResize", &flags,
                           TableFlags_NoBordersInBodyUntilResize);
        Gui::SameLine();
        HelpMarker("Disable vertical borders in columns Body until hovered for "
                   "resize (borders will always appear in Headers)");
        Gui::TreePop();
      }

      if (Gui::TreeNodeEx("Sizing:", TreeNodeFlags_DefaultOpen)) {
        EditTableSizingFlags(&flags);
        Gui::SameLine();
        HelpMarker(
            "In the Advanced demo we override the policy of each column so "
            "those table-wide settings have less effect that typical.");
        Gui::CheckboxFlags("TableFlags_NoHostExtendX", &flags,
                           TableFlags_NoHostExtendX);
        Gui::SameLine();
        HelpMarker("Make outer width auto-fit to columns, overriding "
                   "outer_size.x value.\n\nOnly available when ScrollX/ScrollY "
                   "are disabled and Stretch columns are not used.");
        Gui::CheckboxFlags("TableFlags_NoHostExtendY", &flags,
                           TableFlags_NoHostExtendY);
        Gui::SameLine();
        HelpMarker("Make outer height stop exactly at outer_size.y (prevent "
                   "auto-extending table past the limit).\n\nOnly available "
                   "when ScrollX/ScrollY are disabled. Data below the limit "
                   "will be clipped and not visible.");
        Gui::CheckboxFlags("TableFlags_NoKeepColumnsVisible", &flags,
                           TableFlags_NoKeepColumnsVisible);
        Gui::SameLine();
        HelpMarker("Only available if ScrollX is disabled.");
        Gui::CheckboxFlags("TableFlags_PreciseWidths", &flags,
                           TableFlags_PreciseWidths);
        Gui::SameLine();
        HelpMarker(
            "Disable distributing remainder width to stretched columns (width "
            "allocation on a 100-wide table with 3 columns: Without this flag: "
            "33,33,34. With this flag: 33,33,33). With larger number of "
            "columns, resizing will appear to be less smooth.");
        Gui::CheckboxFlags("TableFlags_NoClip", &flags, TableFlags_NoClip);
        Gui::SameLine();
        HelpMarker(
            "Disable clipping rectangle for every individual columns (reduce "
            "draw command count, items will be able to overflow into other "
            "columns). Generally incompatible with ScrollFreeze options.");
        Gui::TreePop();
      }

      if (Gui::TreeNodeEx("Padding:", TreeNodeFlags_DefaultOpen)) {
        Gui::CheckboxFlags("TableFlags_PadOuterX", &flags,
                           TableFlags_PadOuterX);
        Gui::CheckboxFlags("TableFlags_NoPadOuterX", &flags,
                           TableFlags_NoPadOuterX);
        Gui::CheckboxFlags("TableFlags_NoPadInnerX", &flags,
                           TableFlags_NoPadInnerX);
        Gui::TreePop();
      }

      if (Gui::TreeNodeEx("Scrolling:", TreeNodeFlags_DefaultOpen)) {
        Gui::CheckboxFlags("TableFlags_ScrollX", &flags, TableFlags_ScrollX);
        Gui::SameLine();
        Gui::SetNextItemWidth(Gui::GetFrameHeight());
        Gui::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL,
                     SliderFlags_NoInput);
        Gui::CheckboxFlags("TableFlags_ScrollY", &flags, TableFlags_ScrollY);
        Gui::SameLine();
        Gui::SetNextItemWidth(Gui::GetFrameHeight());
        Gui::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL,
                     SliderFlags_NoInput);
        Gui::TreePop();
      }

      if (Gui::TreeNodeEx("Sorting:", TreeNodeFlags_DefaultOpen)) {
        Gui::CheckboxFlags("TableFlags_SortMulti", &flags,
                           TableFlags_SortMulti);
        Gui::SameLine();
        HelpMarker("When sorting is enabled: hold shift when clicking headers "
                   "to sort on multiple column. TableGetSortSpecs() may return "
                   "specs where (SpecsCount > 1).");
        Gui::CheckboxFlags("TableFlags_SortTristate", &flags,
                           TableFlags_SortTristate);
        Gui::SameLine();
        HelpMarker("When sorting is enabled: allow no sorting, disable default "
                   "sorting. TableGetSortSpecs() may return specs where "
                   "(SpecsCount == 0).");
        Gui::TreePop();
      }

      if (Gui::TreeNodeEx("Headers:", TreeNodeFlags_DefaultOpen)) {
        Gui::Checkbox("show_headers", &show_headers);
        Gui::CheckboxFlags("TableFlags_HighlightHoveredColumn", &flags,
                           TableFlags_HighlightHoveredColumn);
        Gui::CheckboxFlags("TableColumnFlags_AngledHeader", &columns_base_flags,
                           TableColumnFlags_AngledHeader);
        Gui::SameLine();
        HelpMarker(
            "Enable AngledHeader on all columns. Best enabled on selected "
            "narrow columns (see \"Angled headers\" section of the demo).");
        Gui::TreePop();
      }

      if (Gui::TreeNodeEx("Other:", TreeNodeFlags_DefaultOpen)) {
        Gui::Checkbox("show_wrapped_text", &show_wrapped_text);

        Gui::DragFloat2("##OuterSize", &outer_size_value.x);
        Gui::SameLine(0.0f, Gui::GetStyle().ItemInnerSpacing.x);
        Gui::Checkbox("outer_size", &outer_size_enabled);
        Gui::SameLine();
        HelpMarker("If scrolling is disabled (ScrollX and ScrollY not set):\n"
                   "- The table is output directly in the parent window.\n"
                   "- OuterSize.x < 0.0f will right-align the table.\n"
                   "- OuterSize.x = 0.0f will narrow fit the table unless "
                   "there are any Stretch columns.\n"
                   "- OuterSize.y then becomes the minimum size for the table, "
                   "which will extend vertically if there are more rows "
                   "(unless NoHostExtendY is set).");

        // From a user point of view we will tend to use 'inner_width'
        // differently depending on whether our table is embedding scrolling. To
        // facilitate toying with this demo we will actually pass 0.0f to the
        // BeginTable() when ScrollX is disabled.
        Gui::DragFloat("inner_width (when ScrollX active)",
                       &inner_width_with_scroll, 1.0f, 0.0f, FLT_MAX);

        Gui::DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);
        Gui::SameLine();
        HelpMarker("Specify height of the Selectable item.");

        Gui::DragInt("items_count", &items_count, 0.1f, 0, 9999);
        Gui::Combo("items_type (first column)", &contents_type,
                   contents_type_names, ARRAYSIZE(contents_type_names));
        // filter.Draw("filter");
        Gui::TreePop();
      }

      Gui::PopItemWidth();
      PopStyleCompact();
      Gui::Spacing();
      Gui::TreePop();
    }

    // Update item list if we changed the number of items
    static Vector<MyItem> items;
    static Vector<int> selection;
    static bool items_need_sort = false;
    if (items.Size != items_count) {
      items.resize(items_count, MyItem());
      for (int n = 0; n < items_count; n++) {
        const int template_n = n % ARRAYSIZE(template_items_names);
        MyItem &item = items[n];
        item.ID = n;
        item.Name = template_items_names[template_n];
        item.Quantity = (template_n == 3)   ? 10
                        : (template_n == 4) ? 20
                                            : 0; // Assign default quantities
      }
    }

    const DrawList *parent_draw_list = Gui::GetWindowDrawList();
    const int parent_draw_list_draw_cmd_count =
        parent_draw_list->CmdBuffer.Size;
    Vec2 table_scroll_cur, table_scroll_max; // For debug display
    const DrawList *table_draw_list = NULL;  // "

    // Submit table
    const float inner_width_to_use =
        (flags & TableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
    if (Gui::BeginTable("table_advanced", 6, flags,
                        outer_size_enabled ? outer_size_value : Vec2(0, 0),
                        inner_width_to_use)) {
      // Declare columns
      // We use the "user_id" parameter of TableSetupColumn() to specify a user
      // id that will be stored in the sort specifications. This is so our sort
      // function can identify a column given our own identifier. We could also
      // identify them based on their index!
      Gui::TableSetupColumn("ID",
                            columns_base_flags | TableColumnFlags_DefaultSort |
                                TableColumnFlags_WidthFixed |
                                TableColumnFlags_NoHide,
                            0.0f, MyItemColumnID_ID);
      Gui::TableSetupColumn("Name",
                            columns_base_flags | TableColumnFlags_WidthFixed,
                            0.0f, MyItemColumnID_Name);
      Gui::TableSetupColumn("Action",
                            columns_base_flags | TableColumnFlags_NoSort |
                                TableColumnFlags_WidthFixed,
                            0.0f, MyItemColumnID_Action);
      Gui::TableSetupColumn("Quantity",
                            columns_base_flags |
                                TableColumnFlags_PreferSortDescending,
                            0.0f, MyItemColumnID_Quantity);
      Gui::TableSetupColumn("Description",
                            columns_base_flags |
                                ((flags & TableFlags_NoHostExtendX)
                                     ? 0
                                     : TableColumnFlags_WidthStretch),
                            0.0f, MyItemColumnID_Description);
      Gui::TableSetupColumn("Hidden", columns_base_flags |
                                          TableColumnFlags_DefaultHide |
                                          TableColumnFlags_NoSort);
      Gui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

      // Sort our data if sort specs have been changed!
      TableSortSpecs *sort_specs = Gui::TableGetSortSpecs();
      if (sort_specs && sort_specs->SpecsDirty)
        items_need_sort = true;
      if (sort_specs && items_need_sort && items.Size > 1) {
        MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
        sort_specs->SpecsDirty = false;
      }
      items_need_sort = false;

      // Take note of whether we are currently sorting based on the Quantity
      // field, we will use this to trigger sorting when we know the data of
      // this column has been modified.
      const bool sorts_specs_using_quantity =
          (Gui::TableGetColumnFlags(3) & TableColumnFlags_IsSorted) != 0;

      // Show headers
      if (show_headers &&
          (columns_base_flags & TableColumnFlags_AngledHeader) != 0)
        Gui::TableAngledHeadersRow();
      if (show_headers)
        Gui::TableHeadersRow();

      // Show data
      // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we
      // have the buttons here?
      Gui::PushButtonRepeat(true);
#if 1
      // Demonstrate using clipper for large vertical lists
      ListClipper clipper;
      clipper.Begin(items.Size);
      while (clipper.Step()) {
        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd;
             row_n++)
#else
      // Without clipper
      {
        for (int row_n = 0; row_n < items.Size; row_n++)
#endif
        {
          MyItem *item = &items[row_n];
          // if (!filter.PassFilter(item->Name))
          //     continue;

          const bool item_is_selected = selection.contains(item->ID);
          Gui::PushID(item->ID);
          Gui::TableNextRow(TableRowFlags_None, row_min_height);

          // For the demo purpose we can select among different type of items
          // submitted in the first column
          Gui::TableSetColumnIndex(0);
          char label[32];
          sprintf(label, "%04d", item->ID);
          if (contents_type == CT_Text)
            Gui::TextUnformatted(label);
          else if (contents_type == CT_Button)
            Gui::Button(label);
          else if (contents_type == CT_SmallButton)
            Gui::SmallButton(label);
          else if (contents_type == CT_FillButton)
            Gui::Button(label, Vec2(-FLT_MIN, 0.0f));
          else if (contents_type == CT_Selectable ||
                   contents_type == CT_SelectableSpanRow) {
            SelectableFlags selectable_flags =
                (contents_type == CT_SelectableSpanRow)
                    ? SelectableFlags_SpanAllColumns |
                          SelectableFlags_AllowOverlap
                    : SelectableFlags_None;
            if (Gui::Selectable(label, item_is_selected, selectable_flags,
                                Vec2(0, row_min_height))) {
              if (Gui::GetIO().KeyCtrl) {
                if (item_is_selected)
                  selection.find_erase_unsorted(item->ID);
                else
                  selection.push_back(item->ID);
              } else {
                selection.clear();
                selection.push_back(item->ID);
              }
            }
          }

          if (Gui::TableSetColumnIndex(1))
            Gui::TextUnformatted(item->Name);

          // Here we demonstrate marking our data set as needing to be sorted
          // again if we modified a quantity, and we are currently sorting on
          // the column showing the Quantity. To avoid triggering a sort while
          // holding the button, we only trigger it when the button has been
          // released. You will probably need a more advanced system in your
          // code if you want to automatically sort when a specific entry
          // changes.
          if (Gui::TableSetColumnIndex(2)) {
            if (Gui::SmallButton("Chop")) {
              item->Quantity += 1;
            }
            if (sorts_specs_using_quantity && Gui::IsItemDeactivated()) {
              items_need_sort = true;
            }
            Gui::SameLine();
            if (Gui::SmallButton("Eat")) {
              item->Quantity -= 1;
            }
            if (sorts_specs_using_quantity && Gui::IsItemDeactivated()) {
              items_need_sort = true;
            }
          }

          if (Gui::TableSetColumnIndex(3))
            Gui::Text("%d", item->Quantity);

          Gui::TableSetColumnIndex(4);
          if (show_wrapped_text)
            Gui::TextWrapped("Lorem ipsum dolor sit amet");
          else
            Gui::Text("Lorem ipsum dolor sit amet");

          if (Gui::TableSetColumnIndex(5))
            Gui::Text("1234");

          Gui::PopID();
        }
      }
      Gui::PopButtonRepeat();

      // Store some info to display debug details below
      table_scroll_cur = Vec2(Gui::GetScrollX(), Gui::GetScrollY());
      table_scroll_max = Vec2(Gui::GetScrollMaxX(), Gui::GetScrollMaxY());
      table_draw_list = Gui::GetWindowDrawList();
      Gui::EndTable();
    }
    static bool show_debug_details = false;
    Gui::Checkbox("Debug details", &show_debug_details);
    if (show_debug_details && table_draw_list) {
      Gui::SameLine(0.0f, 0.0f);
      const int table_draw_list_draw_cmd_count =
          table_draw_list->CmdBuffer.Size;
      if (table_draw_list == parent_draw_list)
        Gui::Text(": DrawCmd: +%d (in same window)",
                  table_draw_list_draw_cmd_count -
                      parent_draw_list_draw_cmd_count);
      else
        Gui::Text(
            ": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
            table_draw_list_draw_cmd_count - 1, table_scroll_cur.x,
            table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
    }
    Gui::TreePop();
  }

  Gui::PopID();

  ShowDemoWindowColumns();

  if (disable_indent)
    Gui::PopStyleVar();
}

// Demonstrate old/legacy Columns API!
// [2020: Columns are under-featured and not maintained. Prefer using the more
// flexible and powerful BeginTable() API!]
static void ShowDemoWindowColumns() {
  DEMO_MARKER("Columns (legacy API)");
  bool open = Gui::TreeNode("Legacy Columns API");
  Gui::SameLine();
  HelpMarker("Columns() is an old API! Prefer using the more flexible and "
             "powerful BeginTable() API!");
  if (!open)
    return;

  // Basic columns
  DEMO_MARKER("Columns (legacy API)/Basic");
  if (Gui::TreeNode("Basic")) {
    Gui::Text("Without border:");
    Gui::Columns(3, "mycolumns3", false); // 3-ways, no border
    Gui::Separator();
    for (int n = 0; n < 14; n++) {
      char label[32];
      sprintf(label, "Item %d", n);
      if (Gui::Selectable(label)) {
      }
      // if (Gui::Button(label, Vec2(-FLT_MIN,0.0f))) {}
      Gui::NextColumn();
    }
    Gui::Columns(1);
    Gui::Separator();

    Gui::Text("With border:");
    Gui::Columns(4, "mycolumns"); // 4-ways, with border
    Gui::Separator();
    Gui::Text("ID");
    Gui::NextColumn();
    Gui::Text("Name");
    Gui::NextColumn();
    Gui::Text("Path");
    Gui::NextColumn();
    Gui::Text("Hovered");
    Gui::NextColumn();
    Gui::Separator();
    const char *names[3] = {"One", "Two", "Three"};
    const char *paths[3] = {"/path/one", "/path/two", "/path/three"};
    static int selected = -1;
    for (int i = 0; i < 3; i++) {
      char label[32];
      sprintf(label, "%04d", i);
      if (Gui::Selectable(label, selected == i, SelectableFlags_SpanAllColumns))
        selected = i;
      bool hovered = Gui::IsItemHovered();
      Gui::NextColumn();
      Gui::Text(names[i]);
      Gui::NextColumn();
      Gui::Text(paths[i]);
      Gui::NextColumn();
      Gui::Text("%d", hovered);
      Gui::NextColumn();
    }
    Gui::Columns(1);
    Gui::Separator();
    Gui::TreePop();
  }

  DEMO_MARKER("Columns (legacy API)/Borders");
  if (Gui::TreeNode("Borders")) {
    // NB: Future columns API should allow automatic horizontal borders.
    static bool h_borders = true;
    static bool v_borders = true;
    static int columns_count = 4;
    const int lines_count = 3;
    Gui::SetNextItemWidth(Gui::GetFontSize() * 8);
    Gui::DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d columns");
    if (columns_count < 2)
      columns_count = 2;
    Gui::SameLine();
    Gui::Checkbox("horizontal", &h_borders);
    Gui::SameLine();
    Gui::Checkbox("vertical", &v_borders);
    Gui::Columns(columns_count, NULL, v_borders);
    for (int i = 0; i < columns_count * lines_count; i++) {
      if (h_borders && Gui::GetColumnIndex() == 0)
        Gui::Separator();
      Gui::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
      Gui::Text("Width %.2f", Gui::GetColumnWidth());
      Gui::Text("Avail %.2f", Gui::GetContentRegionAvail().x);
      Gui::Text("Offset %.2f", Gui::GetColumnOffset());
      Gui::Text("Long text that is likely to clip");
      Gui::Button("Button", Vec2(-FLT_MIN, 0.0f));
      Gui::NextColumn();
    }
    Gui::Columns(1);
    if (h_borders)
      Gui::Separator();
    Gui::TreePop();
  }

  // Create multiple items in a same cell before switching to next column
  DEMO_MARKER("Columns (legacy API)/Mixed items");
  if (Gui::TreeNode("Mixed items")) {
    Gui::Columns(3, "mixed");
    Gui::Separator();

    Gui::Text("Hello");
    Gui::Button("Banana");
    Gui::NextColumn();

    Gui::Text("Gui");
    Gui::Button("Apple");
    static float foo = 1.0f;
    Gui::InputFloat("red", &foo, 0.05f, 0, "%.3f");
    Gui::Text("An extra line here.");
    Gui::NextColumn();

    Gui::Text("Sailor");
    Gui::Button("Corniflower");
    static float bar = 1.0f;
    Gui::InputFloat("blue", &bar, 0.05f, 0, "%.3f");
    Gui::NextColumn();

    if (Gui::CollapsingHeader("Category A")) {
      Gui::Text("Blah blah blah");
    }
    Gui::NextColumn();
    if (Gui::CollapsingHeader("Category B")) {
      Gui::Text("Blah blah blah");
    }
    Gui::NextColumn();
    if (Gui::CollapsingHeader("Category C")) {
      Gui::Text("Blah blah blah");
    }
    Gui::NextColumn();
    Gui::Columns(1);
    Gui::Separator();
    Gui::TreePop();
  }

  // Word wrapping
  DEMO_MARKER("Columns (legacy API)/Word-wrapping");
  if (Gui::TreeNode("Word-wrapping")) {
    Gui::Columns(2, "word-wrapping");
    Gui::Separator();
    Gui::TextWrapped("The quick brown fox jumps over the lazy dog.");
    Gui::TextWrapped("Hello Left");
    Gui::NextColumn();
    Gui::TextWrapped("The quick brown fox jumps over the lazy dog.");
    Gui::TextWrapped("Hello Right");
    Gui::Columns(1);
    Gui::Separator();
    Gui::TreePop();
  }

  DEMO_MARKER("Columns (legacy API)/Horizontal Scrolling");
  if (Gui::TreeNode("Horizontal Scrolling")) {
    Gui::SetNextWindowContentSize(Vec2(1500.0f, 0.0f));
    Vec2 child_size = Vec2(0, Gui::GetFontSize() * 20.0f);
    Gui::BeginChild("##ScrollingRegion", child_size, ChildFlags_None,
                    WindowFlags_HorizontalScrollbar);
    Gui::Columns(10);

    // Also demonstrate using clipper for large vertical lists
    int ITEMS_COUNT = 2000;
    ListClipper clipper;
    clipper.Begin(ITEMS_COUNT);
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        for (int j = 0; j < 10; j++) {
          Gui::Text("Line %d Column %d...", i, j);
          Gui::NextColumn();
        }
    }
    Gui::Columns(1);
    Gui::EndChild();
    Gui::TreePop();
  }

  DEMO_MARKER("Columns (legacy API)/Tree");
  if (Gui::TreeNode("Tree")) {
    Gui::Columns(2, "tree", true);
    for (int x = 0; x < 3; x++) {
      bool open1 = Gui::TreeNode((void *)(intptr_t)x, "Node%d", x);
      Gui::NextColumn();
      Gui::Text("Node contents");
      Gui::NextColumn();
      if (open1) {
        for (int y = 0; y < 3; y++) {
          bool open2 = Gui::TreeNode((void *)(intptr_t)y, "Node%d.%d", x, y);
          Gui::NextColumn();
          Gui::Text("Node contents");
          if (open2) {
            Gui::Text("Even more contents");
            if (Gui::TreeNode("Tree in column")) {
              Gui::Text("The quick brown fox jumps over the lazy dog");
              Gui::TreePop();
            }
          }
          Gui::NextColumn();
          if (open2)
            Gui::TreePop();
        }
        Gui::TreePop();
      }
    }
    Gui::Columns(1);
    Gui::TreePop();
  }

  Gui::TreePop();
}

static void ShowDemoWindowInputs() {
  DEMO_MARKER("Inputs & Focus");
  if (Gui::CollapsingHeader("Inputs & Focus")) {
    IO &io = Gui::GetIO();

    // Display inputs submitted to IO
    DEMO_MARKER("Inputs & Focus/Inputs");
    Gui::SetNextItemOpen(true, Cond_Once);
    if (Gui::TreeNode("Inputs")) {
      HelpMarker("This is a simplified view. See more detailed input state:\n"
                 "- in 'Tools->Metrics/Debugger->Inputs'.\n"
                 "- in 'Tools->Debug Log->IO'.");
      if (Gui::IsMousePosValid())
        Gui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
      else
        Gui::Text("Mouse pos: <INVALID>");
      Gui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
      Gui::Text("Mouse down:");
      for (int i = 0; i < ARRAYSIZE(io.MouseDown); i++)
        if (Gui::IsMouseDown(i)) {
          Gui::SameLine();
          Gui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
        }
      Gui::Text("Mouse wheel: %.1f", io.MouseWheel);

      // We iterate both legacy native range and named Key ranges, which is
      // a little odd but this allows displaying the data for old/new backends.
      // User code should never have to go through such hoops! You can generally
      // iterate between Key_NamedKey_BEGIN and Key_NamedKey_END.
#ifdef DISABLE_OBSOLETE_KEYIO
      struct funcs {
        static bool IsLegacyNativeDupe(Key) { return false; }
      };
      Key start_key = Key_NamedKey_BEGIN;
#else
      struct funcs {
        static bool IsLegacyNativeDupe(Key key) {
          return key >= 0 && key < 512 && Gui::GetIO().KeyMap[key] != -1;
        }
      }; // Hide Native<>Key duplicates when both exists in the array
      Key start_key = (Key)0;
#endif
      Gui::Text("Keys down:");
      for (Key key = start_key; key < Key_NamedKey_END; key = (Key)(key + 1)) {
        if (funcs::IsLegacyNativeDupe(key) || !Gui::IsKeyDown(key))
          continue;
        Gui::SameLine();
        Gui::Text((key < Key_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d",
                  Gui::GetKeyName(key), key);
      }
      Gui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "",
                io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "",
                io.KeySuper ? "SUPER " : "");
      Gui::Text("Chars queue:");
      for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
        Wchar c = io.InputQueueCharacters[i];
        Gui::SameLine();
        Gui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c);
      } // FIXME: We should convert 'c' to UTF-8 here but the functions are not
        // public.

      Gui::TreePop();
    }

    // Display IO output flags
    DEMO_MARKER("Inputs & Focus/Outputs");
    Gui::SetNextItemOpen(true, Cond_Once);
    if (Gui::TreeNode("Outputs")) {
      HelpMarker("The value of io.WantCaptureMouse and io.WantCaptureKeyboard "
                 "are normally set by Gui "
                 "to instruct your application of how to route inputs. "
                 "Typically, when a value is true, it means "
                 "Gui wants the corresponding inputs and we expect the "
                 "underlying application to ignore them.\n\n"
                 "The most typical case is: when hovering a window, Gui "
                 "set io.WantCaptureMouse to true, "
                 "and underlying application should ignore mouse inputs (in "
                 "practice there are many and more subtle "
                 "rules leading to how those flags are set).");
      Gui::Text("io.WantCaptureMouse: %d", io.WantCaptureMouse);
      Gui::Text("io.WantCaptureMouseUnlessPopupClose: %d",
                io.WantCaptureMouseUnlessPopupClose);
      Gui::Text("io.WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
      Gui::Text("io.WantTextInput: %d", io.WantTextInput);
      Gui::Text("io.WantSetMousePos: %d", io.WantSetMousePos);
      Gui::Text("io.NavActive: %d, io.NavVisible: %d", io.NavActive,
                io.NavVisible);

      DEMO_MARKER("Inputs & Focus/Outputs/WantCapture override");
      if (Gui::TreeNode("WantCapture override")) {
        HelpMarker("Hovering the colored canvas will override "
                   "io.WantCaptureXXX fields.\n"
                   "Notice how normally (when set to none), the value of "
                   "io.WantCaptureKeyboard would be false when hovering and "
                   "true when clicking.");
        static int capture_override_mouse = -1;
        static int capture_override_keyboard = -1;
        const char *capture_override_desc[] = {"None", "Set to false",
                                               "Set to true"};
        Gui::SetNextItemWidth(Gui::GetFontSize() * 15);
        Gui::SliderInt("SetNextFrameWantCaptureMouse() on hover",
                       &capture_override_mouse, -1, +1,
                       capture_override_desc[capture_override_mouse + 1],
                       SliderFlags_AlwaysClamp);
        Gui::SetNextItemWidth(Gui::GetFontSize() * 15);
        Gui::SliderInt("SetNextFrameWantCaptureKeyboard() on hover",
                       &capture_override_keyboard, -1, +1,
                       capture_override_desc[capture_override_keyboard + 1],
                       SliderFlags_AlwaysClamp);

        Gui::ColorButton("##panel", Vec4(0.7f, 0.1f, 0.7f, 1.0f),
                         ColorEditFlags_NoTooltip | ColorEditFlags_NoDragDrop,
                         Vec2(128.0f, 96.0f)); // Dummy item
        if (Gui::IsItemHovered() && capture_override_mouse != -1)
          Gui::SetNextFrameWantCaptureMouse(capture_override_mouse == 1);
        if (Gui::IsItemHovered() && capture_override_keyboard != -1)
          Gui::SetNextFrameWantCaptureKeyboard(capture_override_keyboard == 1);

        Gui::TreePop();
      }
      Gui::TreePop();
    }

    // Display mouse cursors
    DEMO_MARKER("Inputs & Focus/Mouse Cursors");
    if (Gui::TreeNode("Mouse Cursors")) {
      const char *mouse_cursors_names[] = {
          "Arrow",      "TextInput",  "ResizeAll", "ResizeNS",  "ResizeEW",
          "ResizeNESW", "ResizeNWSE", "Hand",      "NotAllowed"};
      ASSERT(ARRAYSIZE(mouse_cursors_names) == MouseCursor_COUNT);

      MouseCursor current = Gui::GetMouseCursor();
      Gui::Text("Current mouse cursor = %d: %s", current,
                mouse_cursors_names[current]);
      Gui::BeginDisabled(true);
      Gui::CheckboxFlags("io.BackendFlags: HasMouseCursors", &io.BackendFlags,
                         BackendFlags_HasMouseCursors);
      Gui::EndDisabled();

      Gui::Text("Hover to see mouse cursors:");
      Gui::SameLine();
      HelpMarker("Your application can render a different mouse cursor based "
                 "on what Gui::GetMouseCursor() returns. "
                 "If software cursor rendering (io.MouseDrawCursor) is set "
                 "Gui will draw the right cursor for you, "
                 "otherwise your backend needs to handle it.");
      for (int i = 0; i < MouseCursor_COUNT; i++) {
        char label[32];
        sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
        Gui::Bullet();
        Gui::Selectable(label, false);
        if (Gui::IsItemHovered())
          Gui::SetMouseCursor(i);
      }
      Gui::TreePop();
    }

    DEMO_MARKER("Inputs & Focus/Tabbing");
    if (Gui::TreeNode("Tabbing")) {
      Gui::Text("Use TAB/SHIFT+TAB to cycle through keyboard editable fields.");
      static char buf[32] = "hello";
      Gui::InputText("1", buf, ARRAYSIZE(buf));
      Gui::InputText("2", buf, ARRAYSIZE(buf));
      Gui::InputText("3", buf, ARRAYSIZE(buf));
      Gui::PushTabStop(false);
      Gui::InputText("4 (tab skip)", buf, ARRAYSIZE(buf));
      Gui::SameLine();
      HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
      Gui::PopTabStop();
      Gui::InputText("5", buf, ARRAYSIZE(buf));
      Gui::TreePop();
    }

    DEMO_MARKER("Inputs & Focus/Focus from code");
    if (Gui::TreeNode("Focus from code")) {
      bool focus_1 = Gui::Button("Focus on 1");
      Gui::SameLine();
      bool focus_2 = Gui::Button("Focus on 2");
      Gui::SameLine();
      bool focus_3 = Gui::Button("Focus on 3");
      int has_focus = 0;
      static char buf[128] = "click on a button to set focus";

      if (focus_1)
        Gui::SetKeyboardFocusHere();
      Gui::InputText("1", buf, ARRAYSIZE(buf));
      if (Gui::IsItemActive())
        has_focus = 1;

      if (focus_2)
        Gui::SetKeyboardFocusHere();
      Gui::InputText("2", buf, ARRAYSIZE(buf));
      if (Gui::IsItemActive())
        has_focus = 2;

      Gui::PushTabStop(false);
      if (focus_3)
        Gui::SetKeyboardFocusHere();
      Gui::InputText("3 (tab skip)", buf, ARRAYSIZE(buf));
      if (Gui::IsItemActive())
        has_focus = 3;
      Gui::SameLine();
      HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
      Gui::PopTabStop();

      if (has_focus)
        Gui::Text("Item with focus: %d", has_focus);
      else
        Gui::Text("Item with focus: <none>");

      // Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
      static float f3[3] = {0.0f, 0.0f, 0.0f};
      int focus_ahead = -1;
      if (Gui::Button("Focus on X")) {
        focus_ahead = 0;
      }
      Gui::SameLine();
      if (Gui::Button("Focus on Y")) {
        focus_ahead = 1;
      }
      Gui::SameLine();
      if (Gui::Button("Focus on Z")) {
        focus_ahead = 2;
      }
      if (focus_ahead != -1)
        Gui::SetKeyboardFocusHere(focus_ahead);
      Gui::SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);

      Gui::TextWrapped("NB: Cursor & selection are preserved when refocusing "
                       "last used item in code.");
      Gui::TreePop();
    }

    DEMO_MARKER("Inputs & Focus/Dragging");
    if (Gui::TreeNode("Dragging")) {
      Gui::TextWrapped("You can use Gui::GetMouseDragDelta(0) to query for "
                       "the dragged amount on any widget.");
      for (int button = 0; button < 3; button++) {
        Gui::Text("IsMouseDragging(%d):", button);
        Gui::Text("  w/ default threshold: %d,", Gui::IsMouseDragging(button));
        Gui::Text("  w/ zero threshold: %d,",
                  Gui::IsMouseDragging(button, 0.0f));
        Gui::Text("  w/ large threshold: %d,",
                  Gui::IsMouseDragging(button, 20.0f));
      }

      Gui::Button("Drag Me");
      if (Gui::IsItemActive())
        Gui::GetForegroundDrawList()->AddLine(
            io.MouseClickedPos[0], io.MousePos, Gui::GetColorU32(Col_Button),
            4.0f); // Draw a line between the button and the mouse cursor

      // Drag operations gets "unlocked" when the mouse has moved past a certain
      // threshold (the default threshold is stored in io.MouseDragThreshold).
      // You can request a lower or higher threshold using the second parameter
      // of IsMouseDragging() and GetMouseDragDelta().
      Vec2 value_raw = Gui::GetMouseDragDelta(0, 0.0f);
      Vec2 value_with_lock_threshold = Gui::GetMouseDragDelta(0);
      Vec2 mouse_delta = io.MouseDelta;
      Gui::Text("GetMouseDragDelta(0):");
      Gui::Text("  w/ default threshold: (%.1f, %.1f)",
                value_with_lock_threshold.x, value_with_lock_threshold.y);
      Gui::Text("  w/ zero threshold: (%.1f, %.1f)", value_raw.x, value_raw.y);
      Gui::Text("io.MouseDelta: (%.1f, %.1f)", mouse_delta.x, mouse_delta.y);
      Gui::TreePop();
    }
  }
}

//-----------------------------------------------------------------------------
// [SECTION] About Window / ShowAboutWindow()
// Access from Gui Demo -> Tools -> About
//-----------------------------------------------------------------------------

void Gui::ShowAboutWindow(bool *p_open) {
  if (!Gui::Begin("About Gui", p_open, WindowFlags_AlwaysAutoResize)) {
    Gui::End();
    return;
  }
  DEMO_MARKER("Tools/About Gui");
  Gui::Text("Gui %s (%d)", VERSION, VERSION_NUM);
  Gui::Separator();
  Gui::Text("By Omar Cornut and all Gui contributors.");
  Gui::Text("Gui is licensed under the MIT License, see LICENSE for "
            "more information.");
  Gui::Text(
      "If your company uses this, please consider sponsoring the project!");

  static bool show_config_info = false;
  Gui::Checkbox("Config/Build Information", &show_config_info);
  if (show_config_info) {
    IO &io = Gui::GetIO();
    Style &style = Gui::GetStyle();

    bool copy_to_clipboard = Gui::Button("Copy to clipboard");
    Vec2 child_size = Vec2(0, Gui::GetTextLineHeightWithSpacing() * 18);
    Gui::BeginChild(Gui::GetID("cfg_infos"), child_size, ChildFlags_FrameStyle);
    if (copy_to_clipboard) {
      Gui::LogToClipboard();
      Gui::LogText("```\n"); // Back quotes will make text appears without
                             // formatting when pasting on GitHub
    }

    Gui::Text("Gui %s (%d)", VERSION, VERSION_NUM);
    Gui::Separator();
    Gui::Text("sizeof(size_t): %d, sizeof(DrawIdx): %d, sizeof(DrawVert): %d",
              (int)sizeof(size_t), (int)sizeof(DrawIdx), (int)sizeof(DrawVert));
    Gui::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef DISABLE_OBSOLETE_FUNCTIONS
    Gui::Text("define: DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef DISABLE_OBSOLETE_KEYIO
    Gui::Text("define: DISABLE_OBSOLETE_KEYIO");
#endif
#ifdef DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
    Gui::Text("define: DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
    Gui::Text("define: DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef DISABLE_WIN32_FUNCTIONS
    Gui::Text("define: DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef DISABLE_DEFAULT_FORMAT_FUNCTIONS
    Gui::Text("define: DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef DISABLE_DEFAULT_MATH_FUNCTIONS
    Gui::Text("define: DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef DISABLE_DEFAULT_FILE_FUNCTIONS
    Gui::Text("define: DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef DISABLE_FILE_FUNCTIONS
    Gui::Text("define: DISABLE_FILE_FUNCTIONS");
#endif
#ifdef DISABLE_DEFAULT_ALLOCATORS
    Gui::Text("define: DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef USE_BGRA_PACKED_COLOR
    Gui::Text("define: USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
    Gui::Text("define: _WIN32");
#endif
#ifdef _WIN64
    Gui::Text("define: _WIN64");
#endif
#ifdef __linux__
    Gui::Text("define: __linux__");
#endif
#ifdef __APPLE__
    Gui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
    Gui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
    Gui::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
    Gui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
    Gui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
    Gui::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
    Gui::Text("define: __clang_version__=%s", __clang_version__);
#endif
#ifdef __EMSCRIPTEN__
    Gui::Text("define: __EMSCRIPTEN__");
#endif
#ifdef HAS_VIEWPORT
    Gui::Text("define: HAS_VIEWPORT");
#endif
#ifdef HAS_DOCK
    Gui::Text("define: HAS_DOCK");
#endif
    Gui::Separator();
    Gui::Text("io.BackendPlatformName: %s",
              io.BackendPlatformName ? io.BackendPlatformName : "NULL");
    Gui::Text("io.BackendRendererName: %s",
              io.BackendRendererName ? io.BackendRendererName : "NULL");
    Gui::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
    if (io.ConfigFlags & ConfigFlags_NavEnableKeyboard)
      Gui::Text(" NavEnableKeyboard");
    if (io.ConfigFlags & ConfigFlags_NavEnableGamepad)
      Gui::Text(" NavEnableGamepad");
    if (io.ConfigFlags & ConfigFlags_NavEnableSetMousePos)
      Gui::Text(" NavEnableSetMousePos");
    if (io.ConfigFlags & ConfigFlags_NavNoCaptureKeyboard)
      Gui::Text(" NavNoCaptureKeyboard");
    if (io.ConfigFlags & ConfigFlags_NoMouse)
      Gui::Text(" NoMouse");
    if (io.ConfigFlags & ConfigFlags_NoMouseCursorChange)
      Gui::Text(" NoMouseCursorChange");
    if (io.ConfigFlags & ConfigFlags_DockingEnable)
      Gui::Text(" DockingEnable");
    if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
      Gui::Text(" ViewportsEnable");
    if (io.ConfigFlags & ConfigFlags_DpiEnableScaleViewports)
      Gui::Text(" DpiEnableScaleViewports");
    if (io.ConfigFlags & ConfigFlags_DpiEnableScaleFonts)
      Gui::Text(" DpiEnableScaleFonts");
    if (io.MouseDrawCursor)
      Gui::Text("io.MouseDrawCursor");
    if (io.ConfigViewportsNoAutoMerge)
      Gui::Text("io.ConfigViewportsNoAutoMerge");
    if (io.ConfigViewportsNoTaskBarIcon)
      Gui::Text("io.ConfigViewportsNoTaskBarIcon");
    if (io.ConfigViewportsNoDecoration)
      Gui::Text("io.ConfigViewportsNoDecoration");
    if (io.ConfigViewportsNoDefaultParent)
      Gui::Text("io.ConfigViewportsNoDefaultParent");
    if (io.ConfigDockingNoSplit)
      Gui::Text("io.ConfigDockingNoSplit");
    if (io.ConfigDockingWithShift)
      Gui::Text("io.ConfigDockingWithShift");
    if (io.ConfigDockingAlwaysTabBar)
      Gui::Text("io.ConfigDockingAlwaysTabBar");
    if (io.ConfigDockingTransparentPayload)
      Gui::Text("io.ConfigDockingTransparentPayload");
    if (io.ConfigMacOSXBehaviors)
      Gui::Text("io.ConfigMacOSXBehaviors");
    if (io.ConfigInputTextCursorBlink)
      Gui::Text("io.ConfigInputTextCursorBlink");
    if (io.ConfigWindowsResizeFromEdges)
      Gui::Text("io.ConfigWindowsResizeFromEdges");
    if (io.ConfigWindowsMoveFromTitleBarOnly)
      Gui::Text("io.ConfigWindowsMoveFromTitleBarOnly");
    if (io.ConfigMemoryCompactTimer >= 0.0f)
      Gui::Text("io.ConfigMemoryCompactTimer = %.1f",
                io.ConfigMemoryCompactTimer);
    Gui::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
    if (io.BackendFlags & BackendFlags_HasGamepad)
      Gui::Text(" HasGamepad");
    if (io.BackendFlags & BackendFlags_HasMouseCursors)
      Gui::Text(" HasMouseCursors");
    if (io.BackendFlags & BackendFlags_HasSetMousePos)
      Gui::Text(" HasSetMousePos");
    if (io.BackendFlags & BackendFlags_PlatformHasViewports)
      Gui::Text(" PlatformHasViewports");
    if (io.BackendFlags & BackendFlags_HasMouseHoveredViewport)
      Gui::Text(" HasMouseHoveredViewport");
    if (io.BackendFlags & BackendFlags_RendererHasVtxOffset)
      Gui::Text(" RendererHasVtxOffset");
    if (io.BackendFlags & BackendFlags_RendererHasViewports)
      Gui::Text(" RendererHasViewports");
    Gui::Separator();
    Gui::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d",
              io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexWidth,
              io.Fonts->TexHeight);
    Gui::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
    Gui::Text("io.DisplayFramebufferScale: %.2f,%.2f",
              io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    Gui::Separator();
    Gui::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x,
              style.WindowPadding.y);
    Gui::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
    Gui::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x,
              style.FramePadding.y);
    Gui::Text("style.FrameRounding: %.2f", style.FrameRounding);
    Gui::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
    Gui::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x,
              style.ItemSpacing.y);
    Gui::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x,
              style.ItemInnerSpacing.y);

    if (copy_to_clipboard) {
      Gui::LogText("\n```\n");
      Gui::LogFinish();
    }
    Gui::EndChild();
  }
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Editor / ShowStyleEditor()
//-----------------------------------------------------------------------------
// - ShowFontSelector()
// - ShowStyleSelector()
// - ShowStyleEditor()
//-----------------------------------------------------------------------------

// Forward declare ShowFontAtlas() which isn't worth putting in public API yet
namespace Gui {
API void ShowFontAtlas(FontAtlas *atlas);
}

// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is the more
// flexible one.
void Gui::ShowFontSelector(const char *label) {
  IO &io = Gui::GetIO();
  Font *font_current = Gui::GetFont();
  if (Gui::BeginCombo(label, font_current->GetDebugName())) {
    for (Font *font : io.Fonts->Fonts) {
      Gui::PushID((void *)font);
      if (Gui::Selectable(font->GetDebugName(), font == font_current))
        io.FontDefault = font;
      Gui::PopID();
    }
    Gui::EndCombo();
  }
  Gui::SameLine();
  HelpMarker("- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
             "- The font atlas is built when calling "
             "io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
             "- Read FAQ and docs/FONTS.md for more details.\n"
             "- If you need to add/remove fonts at runtime (e.g. for DPI "
             "change), do it before calling NewFrame().");
}

// Demo helper function to select among default colors. See ShowStyleEditor()
// for more advanced options. Here we use the simplified Combo() api that packs
// items into a single literal string. Useful for quick combo boxes where the
// choices are known locally.
bool Gui::ShowStyleSelector(const char *label) {
  static int style_idx = -1;
  if (Gui::Combo(label, &style_idx, "Dark\0Light\0Classic\0")) {
    switch (style_idx) {
    case 0:
      Gui::StyleColorsDark();
      break;
    case 1:
      Gui::StyleColorsLight();
      break;
    case 2:
      Gui::StyleColorsClassic();
      break;
    }
    return true;
  }
  return false;
}

void Gui::ShowStyleEditor(Style *ref) {
  DEMO_MARKER("Tools/Style Editor");
  // You can pass in a reference Style structure to compare to, revert to
  // and save to (without a reference style pointer, we will use one compared
  // locally as a reference)
  Style &style = Gui::GetStyle();
  static Style ref_saved_style;

  // Default to using internal storage as reference
  static bool init = true;
  if (init && ref == NULL)
    ref_saved_style = style;
  init = false;
  if (ref == NULL)
    ref = &ref_saved_style;

  Gui::PushItemWidth(Gui::GetWindowWidth() * 0.50f);

  if (Gui::ShowStyleSelector("Colors##Selector"))
    ref_saved_style = style;
  Gui::ShowFontSelector("Fonts##Selector");

  // Simplified Settings (expose floating-pointer border sizes as boolean
  // representing 0.0f or 1.0f)
  if (Gui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f,
                       "%.0f"))
    style.GrabRounding = style.FrameRounding; // Make GrabRounding always the
                                              // same value as FrameRounding
  {
    bool border = (style.WindowBorderSize > 0.0f);
    if (Gui::Checkbox("WindowBorder", &border)) {
      style.WindowBorderSize = border ? 1.0f : 0.0f;
    }
  }
  Gui::SameLine();
  {
    bool border = (style.FrameBorderSize > 0.0f);
    if (Gui::Checkbox("FrameBorder", &border)) {
      style.FrameBorderSize = border ? 1.0f : 0.0f;
    }
  }
  Gui::SameLine();
  {
    bool border = (style.PopupBorderSize > 0.0f);
    if (Gui::Checkbox("PopupBorder", &border)) {
      style.PopupBorderSize = border ? 1.0f : 0.0f;
    }
  }

  // Save/Revert button
  if (Gui::Button("Save Ref"))
    *ref = ref_saved_style = style;
  Gui::SameLine();
  if (Gui::Button("Revert Ref"))
    style = *ref;
  Gui::SameLine();
  HelpMarker("Save/Revert in local non-persistent storage. Default Colors "
             "definition are not affected. "
             "Use \"Export\" below to save them somewhere.");

  Gui::Separator();

  if (Gui::BeginTabBar("##tabs", TabBarFlags_None)) {
    if (Gui::BeginTabItem("Sizes")) {
      Gui::SeparatorText("Main");
      Gui::SliderFloat2("WindowPadding", (float *)&style.WindowPadding, 0.0f,
                        20.0f, "%.0f");
      Gui::SliderFloat2("FramePadding", (float *)&style.FramePadding, 0.0f,
                        20.0f, "%.0f");
      Gui::SliderFloat2("ItemSpacing", (float *)&style.ItemSpacing, 0.0f, 20.0f,
                        "%.0f");
      Gui::SliderFloat2("ItemInnerSpacing", (float *)&style.ItemInnerSpacing,
                        0.0f, 20.0f, "%.0f");
      Gui::SliderFloat2("TouchExtraPadding", (float *)&style.TouchExtraPadding,
                        0.0f, 10.0f, "%.0f");
      Gui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f,
                       "%.0f");
      Gui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f,
                       "%.0f");
      Gui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");

      Gui::SeparatorText("Borders");
      Gui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f,
                       "%.0f");
      Gui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f,
                       "%.0f");
      Gui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f,
                       "%.0f");
      Gui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f,
                       "%.0f");
      Gui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f,
                       "%.0f");
      Gui::SliderFloat("TabBarBorderSize", &style.TabBarBorderSize, 0.0f, 2.0f,
                       "%.0f");

      Gui::SeparatorText("Rounding");
      Gui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f,
                       "%.0f");
      Gui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f,
                       "%.0f");
      Gui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f,
                       "%.0f");
      Gui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f,
                       "%.0f");
      Gui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f,
                       12.0f, "%.0f");
      Gui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f,
                       "%.0f");
      Gui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");

      Gui::SeparatorText("Tables");
      Gui::SliderFloat2("CellPadding", (float *)&style.CellPadding, 0.0f, 20.0f,
                        "%.0f");
      Gui::SliderAngle("TableAngledHeadersAngle",
                       &style.TableAngledHeadersAngle, -50.0f, +50.0f);

      Gui::SeparatorText("Widgets");
      Gui::SliderFloat2("WindowTitleAlign", (float *)&style.WindowTitleAlign,
                        0.0f, 1.0f, "%.2f");
      int window_menu_button_position = style.WindowMenuButtonPosition + 1;
      if (Gui::Combo("WindowMenuButtonPosition",
                     (int *)&window_menu_button_position,
                     "None\0Left\0Right\0"))
        style.WindowMenuButtonPosition = window_menu_button_position - 1;
      Gui::Combo("ColorButtonPosition", (int *)&style.ColorButtonPosition,
                 "Left\0Right\0");
      Gui::SliderFloat2("ButtonTextAlign", (float *)&style.ButtonTextAlign,
                        0.0f, 1.0f, "%.2f");
      Gui::SameLine();
      HelpMarker(
          "Alignment applies when a button is larger than its text content.");
      Gui::SliderFloat2("SelectableTextAlign",
                        (float *)&style.SelectableTextAlign, 0.0f, 1.0f,
                        "%.2f");
      Gui::SameLine();
      HelpMarker("Alignment applies when a selectable is larger than its text "
                 "content.");
      Gui::SliderFloat("SeparatorTextBorderSize",
                       &style.SeparatorTextBorderSize, 0.0f, 10.0f, "%.0f");
      Gui::SliderFloat2("SeparatorTextAlign",
                        (float *)&style.SeparatorTextAlign, 0.0f, 1.0f, "%.2f");
      Gui::SliderFloat2("SeparatorTextPadding",
                        (float *)&style.SeparatorTextPadding, 0.0f, 40.0f,
                        "%.0f");
      Gui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f,
                       12.0f, "%.0f");

      Gui::SeparatorText("Docking");
      Gui::SliderFloat("DockingSplitterSize", &style.DockingSeparatorSize, 0.0f,
                       12.0f, "%.0f");

      Gui::SeparatorText("Tooltips");
      for (int n = 0; n < 2; n++)
        if (Gui::TreeNodeEx(n == 0 ? "HoverFlagsForTooltipMouse"
                                   : "HoverFlagsForTooltipNav")) {
          HoveredFlags *p = (n == 0) ? &style.HoverFlagsForTooltipMouse
                                     : &style.HoverFlagsForTooltipNav;
          Gui::CheckboxFlags("HoveredFlags_DelayNone", p,
                             HoveredFlags_DelayNone);
          Gui::CheckboxFlags("HoveredFlags_DelayShort", p,
                             HoveredFlags_DelayShort);
          Gui::CheckboxFlags("HoveredFlags_DelayNormal", p,
                             HoveredFlags_DelayNormal);
          Gui::CheckboxFlags("HoveredFlags_Stationary", p,
                             HoveredFlags_Stationary);
          Gui::CheckboxFlags("HoveredFlags_NoSharedDelay", p,
                             HoveredFlags_NoSharedDelay);
          Gui::TreePop();
        }

      Gui::SeparatorText("Misc");
      Gui::SliderFloat2("DisplaySafeAreaPadding",
                        (float *)&style.DisplaySafeAreaPadding, 0.0f, 30.0f,
                        "%.0f");
      Gui::SameLine();
      HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a "
                 "TV where scaling has not been configured).");

      Gui::EndTabItem();
    }

    if (Gui::BeginTabItem("Colors")) {
      static int output_dest = 0;
      static bool output_only_modified = true;
      if (Gui::Button("Export")) {
        if (output_dest == 0)
          Gui::LogToClipboard();
        else
          Gui::LogToTTY();
        Gui::LogText("Vec4* colors = Gui::GetStyle().Colors;" NEWLINE);
        for (int i = 0; i < Col_COUNT; i++) {
          const Vec4 &col = style.Colors[i];
          const char *name = Gui::GetStyleColorName(i);
          if (!output_only_modified ||
              memcmp(&col, &ref->Colors[i], sizeof(Vec4)) != 0)
            Gui::LogText("colors[Col_%s]%*s= Vec4(%.2ff, %.2ff, "
                         "%.2ff, %.2ff);" NEWLINE,
                         name, 23 - (int)strlen(name), "", col.x, col.y, col.z,
                         col.w);
        }
        Gui::LogFinish();
      }
      Gui::SameLine();
      Gui::SetNextItemWidth(120);
      Gui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
      Gui::SameLine();
      Gui::Checkbox("Only Modified Colors", &output_only_modified);

      static TextFilter filter;
      filter.Draw("Filter colors", Gui::GetFontSize() * 16);

      static ColorEditFlags alpha_flags = 0;
      if (Gui::RadioButton("Opaque", alpha_flags == ColorEditFlags_None)) {
        alpha_flags = ColorEditFlags_None;
      }
      Gui::SameLine();
      if (Gui::RadioButton("Alpha",
                           alpha_flags == ColorEditFlags_AlphaPreview)) {
        alpha_flags = ColorEditFlags_AlphaPreview;
      }
      Gui::SameLine();
      if (Gui::RadioButton("Both",
                           alpha_flags == ColorEditFlags_AlphaPreviewHalf)) {
        alpha_flags = ColorEditFlags_AlphaPreviewHalf;
      }
      Gui::SameLine();
      HelpMarker("In the color list:\n"
                 "Left-click on color square to open color picker,\n"
                 "Right-click to open edit options menu.");

      Gui::SetNextWindowSizeConstraints(
          Vec2(0.0f, Gui::GetTextLineHeightWithSpacing() * 10),
          Vec2(FLT_MAX, FLT_MAX));
      Gui::BeginChild("##colors", Vec2(0, 0), ChildFlags_Border,
                      WindowFlags_AlwaysVerticalScrollbar |
                          WindowFlags_AlwaysHorizontalScrollbar |
                          WindowFlags_NavFlattened);
      Gui::PushItemWidth(Gui::GetFontSize() * -12);
      for (int i = 0; i < Col_COUNT; i++) {
        const char *name = Gui::GetStyleColorName(i);
        if (!filter.PassFilter(name))
          continue;
        Gui::PushID(i);
        if (Gui::Button("?"))
          Gui::DebugFlashStyleColor((Col)i);
        Gui::SetItemTooltip(
            "Flash given color to identify places where it is used.");
        Gui::SameLine();
        Gui::ColorEdit4("##color", (float *)&style.Colors[i],
                        ColorEditFlags_AlphaBar | alpha_flags);
        if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(Vec4)) != 0) {
          // Tips: in a real user application, you may want to merge and use an
          // icon font into the main font, so instead of "Save"/"Revert" you'd
          // use icons! Read the FAQ and docs/FONTS.md about using icon fonts.
          // It's really easy and super convenient!
          Gui::SameLine(0.0f, style.ItemInnerSpacing.x);
          if (Gui::Button("Save")) {
            ref->Colors[i] = style.Colors[i];
          }
          Gui::SameLine(0.0f, style.ItemInnerSpacing.x);
          if (Gui::Button("Revert")) {
            style.Colors[i] = ref->Colors[i];
          }
        }
        Gui::SameLine(0.0f, style.ItemInnerSpacing.x);
        Gui::TextUnformatted(name);
        Gui::PopID();
      }
      Gui::PopItemWidth();
      Gui::EndChild();

      Gui::EndTabItem();
    }

    if (Gui::BeginTabItem("Fonts")) {
      IO &io = Gui::GetIO();
      FontAtlas *atlas = io.Fonts;
      HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
      Gui::ShowFontAtlas(atlas);

      // Post-baking font scaling. Note that this is NOT the nice way of scaling
      // fonts, read below. (we enforce hard clamping manually as by default
      // DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
      const float MIN_SCALE = 0.3f;
      const float MAX_SCALE = 2.0f;
      HelpMarker(
          "Those are old settings provided for convenience.\n"
          "However, the _correct_ way of scaling your UI is currently to "
          "reload your font at the designed size, "
          "rebuild the font atlas, and call style.ScaleAllSizes() on a "
          "reference Style structure.\n"
          "Using those settings here will give you poor quality results.");
      static float window_scale = 1.0f;
      Gui::PushItemWidth(Gui::GetFontSize() * 8);
      if (Gui::DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE,
                         MAX_SCALE, "%.2f",
                         SliderFlags_AlwaysClamp)) // Scale only this window
        Gui::SetWindowFontScale(window_scale);
      Gui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE,
                     MAX_SCALE, "%.2f",
                     SliderFlags_AlwaysClamp); // Scale everything
      Gui::PopItemWidth();

      Gui::EndTabItem();
    }

    if (Gui::BeginTabItem("Rendering")) {
      Gui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
      Gui::SameLine();
      HelpMarker("When disabling anti-aliasing lines, you'll probably want to "
                 "disable borders in your style as well.");

      Gui::Checkbox("Anti-aliased lines use texture",
                    &style.AntiAliasedLinesUseTex);
      Gui::SameLine();
      HelpMarker("Faster lines using texture data. Require backend to render "
                 "with bilinear filtering (not point/nearest filtering).");

      Gui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
      Gui::PushItemWidth(Gui::GetFontSize() * 8);
      Gui::DragFloat("Curve Tessellation Tolerance",
                     &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
      if (style.CurveTessellationTol < 0.10f)
        style.CurveTessellationTol = 0.10f;

      // When editing the "Circle Segment Max Error" value, draw a preview of
      // its effect on auto-tessellated circles.
      Gui::DragFloat("Circle Tessellation Max Error",
                     &style.CircleTessellationMaxError, 0.005f, 0.10f, 5.0f,
                     "%.2f", SliderFlags_AlwaysClamp);
      const bool show_samples = Gui::IsItemActive();
      if (show_samples)
        Gui::SetNextWindowPos(Gui::GetCursorScreenPos());
      if (show_samples && Gui::BeginTooltip()) {
        Gui::TextUnformatted("(R = radius, N = number of segments)");
        Gui::Spacing();
        DrawList *draw_list = Gui::GetWindowDrawList();
        const float min_widget_width = Gui::CalcTextSize("N: MMM\nR: MMM").x;
        for (int n = 0; n < 8; n++) {
          const float RAD_MIN = 5.0f;
          const float RAD_MAX = 70.0f;
          const float rad =
              RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);

          Gui::BeginGroup();

          Gui::Text("R: %.f\nN: %d", rad,
                    draw_list->_CalcCircleAutoSegmentCount(rad));

          const float canvas_width = MAX(min_widget_width, rad * 2.0f);
          const float offset_x = floorf(canvas_width * 0.5f);
          const float offset_y = floorf(RAD_MAX);

          const Vec2 p1 = Gui::GetCursorScreenPos();
          draw_list->AddCircle(Vec2(p1.x + offset_x, p1.y + offset_y), rad,
                               Gui::GetColorU32(Col_Text));
          Gui::Dummy(Vec2(canvas_width, RAD_MAX * 2));

          /*
          const Vec2 p2 = Gui::GetCursorScreenPos();
          draw_list->AddCircleFilled(Vec2(p2.x + offset_x, p2.y + offset_y),
          rad, Gui::GetColorU32(Col_Text));
          Gui::Dummy(Vec2(canvas_width, RAD_MAX * 2));
          */

          Gui::EndGroup();
          Gui::SameLine();
        }
        Gui::EndTooltip();
      }
      Gui::SameLine();
      HelpMarker("When drawing circle primitives with \"num_segments == 0\" "
                 "tesselation will be calculated automatically.");

      Gui::DragFloat(
          "Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f,
          "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero
                   // alpha clips all widgets). But application code could have
                   // a toggle to switch between zero and non-zero.
      Gui::DragFloat("Disabled Alpha", &style.DisabledAlpha, 0.005f, 0.0f, 1.0f,
                     "%.2f");
      Gui::SameLine();
      HelpMarker("Additional alpha multiplier for disabled items (multiply "
                 "over current value of Alpha).");
      Gui::PopItemWidth();

      Gui::EndTabItem();
    }

    Gui::EndTabBar();
  }

  Gui::PopItemWidth();
}

//-----------------------------------------------------------------------------
// [SECTION] User Guide / ShowUserGuide()
//-----------------------------------------------------------------------------

void Gui::ShowUserGuide() {
  IO &io = Gui::GetIO();
  Gui::BulletText("Double-click on title bar to collapse window.");
  Gui::BulletText("Click and drag on lower corner to resize window\n"
                  "(double-click to auto fit window to its contents).");
  Gui::BulletText("CTRL+Click on a slider or drag box to input value as text.");
  Gui::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
  Gui::BulletText("CTRL+Tab to select a window.");
  if (io.FontAllowUserScaling)
    Gui::BulletText("CTRL+Mouse Wheel to zoom window contents.");
  Gui::BulletText("While inputing text:\n");
  Gui::Indent();
  Gui::BulletText("CTRL+Left/Right to word jump.");
  Gui::BulletText("CTRL+A or double-click to select all.");
  Gui::BulletText("CTRL+X/C/V to use clipboard cut/copy/paste.");
  Gui::BulletText("CTRL+Z,CTRL+Y to undo/redo.");
  Gui::BulletText("ESCAPE to revert.");
  Gui::Unindent();
  Gui::BulletText("With keyboard navigation enabled:");
  Gui::Indent();
  Gui::BulletText("Arrow keys to navigate.");
  Gui::BulletText("Space to activate a widget.");
  Gui::BulletText("Return to input text into a widget.");
  Gui::BulletText(
      "Escape to deactivate a widget, close popup, exit child window.");
  Gui::BulletText("Alt to jump to the menu layer of a window.");
  Gui::Unindent();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
//-----------------------------------------------------------------------------
// - ShowExampleAppMainMenuBar()
// - ShowExampleMenuFile()
//-----------------------------------------------------------------------------

// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window (which needs the
// WindowFlags_MenuBar flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of
// the main viewport + call BeginMenuBar() into it.
static void ShowExampleAppMainMenuBar() {
  if (Gui::BeginMainMenuBar()) {
    if (Gui::BeginMenu("File")) {
      ShowExampleMenuFile();
      Gui::EndMenu();
    }
    if (Gui::BeginMenu("Edit")) {
      if (Gui::MenuItem("Undo", "CTRL+Z")) {
      }
      if (Gui::MenuItem("Redo", "CTRL+Y", false, false)) {
      } // Disabled item
      Gui::Separator();
      if (Gui::MenuItem("Cut", "CTRL+X")) {
      }
      if (Gui::MenuItem("Copy", "CTRL+C")) {
      }
      if (Gui::MenuItem("Paste", "CTRL+V")) {
      }
      Gui::EndMenu();
    }
    Gui::EndMainMenuBar();
  }
}

// Note that shortcuts are currently provided for display only
// (future version will add explicit flags to BeginMenu() to request processing
// shortcuts)
static void ShowExampleMenuFile() {
  DEMO_MARKER("Examples/Menu");
  Gui::MenuItem("(demo menu)", NULL, false, false);
  if (Gui::MenuItem("New")) {
  }
  if (Gui::MenuItem("Open", "Ctrl+O")) {
  }
  if (Gui::BeginMenu("Open Recent")) {
    Gui::MenuItem("fish_hat.c");
    Gui::MenuItem("fish_hat.inl");
    Gui::MenuItem("fish_hat.h");
    if (Gui::BeginMenu("More..")) {
      Gui::MenuItem("Hello");
      Gui::MenuItem("Sailor");
      if (Gui::BeginMenu("Recurse..")) {
        ShowExampleMenuFile();
        Gui::EndMenu();
      }
      Gui::EndMenu();
    }
    Gui::EndMenu();
  }
  if (Gui::MenuItem("Save", "Ctrl+S")) {
  }
  if (Gui::MenuItem("Save As..")) {
  }

  Gui::Separator();
  DEMO_MARKER("Examples/Menu/Options");
  if (Gui::BeginMenu("Options")) {
    static bool enabled = true;
    Gui::MenuItem("Enabled", "", &enabled);
    Gui::BeginChild("child", Vec2(0, 60), ChildFlags_Border);
    for (int i = 0; i < 10; i++)
      Gui::Text("Scrolling Text %d", i);
    Gui::EndChild();
    static float f = 0.5f;
    static int n = 0;
    Gui::SliderFloat("Value", &f, 0.0f, 1.0f);
    Gui::InputFloat("Input", &f, 0.1f);
    Gui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
    Gui::EndMenu();
  }

  DEMO_MARKER("Examples/Menu/Colors");
  if (Gui::BeginMenu("Colors")) {
    float sz = Gui::GetTextLineHeight();
    for (int i = 0; i < Col_COUNT; i++) {
      const char *name = Gui::GetStyleColorName((Col)i);
      Vec2 p = Gui::GetCursorScreenPos();
      Gui::GetWindowDrawList()->AddRectFilled(p, Vec2(p.x + sz, p.y + sz),
                                              Gui::GetColorU32((Col)i));
      Gui::Dummy(Vec2(sz, sz));
      Gui::SameLine();
      Gui::MenuItem(name);
    }
    Gui::EndMenu();
  }

  // Here we demonstrate appending again to the "Options" menu (which we already
  // created above) Of course in this demo it is a little bit silly that this
  // function calls BeginMenu("Options") twice. In a real code-base using it
  // would make senses to use this feature from very different code locations.
  if (Gui::BeginMenu("Options")) // <-- Append!
  {
    DEMO_MARKER("Examples/Menu/Append to an existing menu");
    static bool b = true;
    Gui::Checkbox("SomeOption", &b);
    Gui::EndMenu();
  }

  if (Gui::BeginMenu("Disabled", false)) // Disabled
  {
    ASSERT(0);
  }
  if (Gui::MenuItem("Checked", NULL, true)) {
  }
  Gui::Separator();
  if (Gui::MenuItem("Quit", "Alt+F4")) {
  }
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering,
// completion and history. For the console example, we are using a more C++ like
// approach of declaring a class to hold both data and functions.
struct ExampleAppConsole {
  char InputBuf[256];
  Vector<char *> Items;
  Vector<const char *> Commands;
  Vector<char *> History;
  int HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
  TextFilter Filter;
  bool AutoScroll;
  bool ScrollToBottom;

  ExampleAppConsole() {
    DEMO_MARKER("Examples/Console");
    ClearLog();
    memset(InputBuf, 0, sizeof(InputBuf));
    HistoryPos = -1;

    // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to
    // "CL" and display multiple matches.
    Commands.push_back("HELP");
    Commands.push_back("HISTORY");
    Commands.push_back("CLEAR");
    Commands.push_back("CLASSIFY");
    AutoScroll = true;
    ScrollToBottom = false;
    AddLog("Welcome to Gui!");
  }
  ~ExampleAppConsole() {
    ClearLog();
    for (int i = 0; i < History.Size; i++)
      free(History[i]);
  }

  // Portable helpers
  static int Stricmp(const char *s1, const char *s2) {
    int d;
    while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
      s1++;
      s2++;
    }
    return d;
  }
  static int Strnicmp(const char *s1, const char *s2, int n) {
    int d = 0;
    while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
      s1++;
      s2++;
      n--;
    }
    return d;
  }
  static char *Strdup(const char *s) {
    ASSERT(s);
    size_t len = strlen(s) + 1;
    void *buf = malloc(len);
    ASSERT(buf);
    return (char *)memcpy(buf, (const void *)s, len);
  }
  static void Strtrim(char *s) {
    char *str_end = s + strlen(s);
    while (str_end > s && str_end[-1] == ' ')
      str_end--;
    *str_end = 0;
  }

  void ClearLog() {
    for (int i = 0; i < Items.Size; i++)
      free(Items[i]);
    Items.clear();
  }

  void AddLog(const char *fmt, ...) FMTARGS(2) {
    // FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, ARRAYSIZE(buf), fmt, args);
    buf[ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    Items.push_back(Strdup(buf));
  }

  void Draw(const char *title, bool *p_open) {
    Gui::SetNextWindowSize(Vec2(520, 600), Cond_FirstUseEver);
    if (!Gui::Begin(title, p_open)) {
      Gui::End();
      return;
    }

    // As a specific feature guaranteed by the library, after calling Begin()
    // the last Item represent the title bar. So e.g. IsItemHovered() will
    // return true when hovering the title bar. Here we create a context menu
    // only available from the title bar.
    if (Gui::BeginPopupContextItem()) {
      if (Gui::MenuItem("Close Console"))
        *p_open = false;
      Gui::EndPopup();
    }

    Gui::TextWrapped(
        "This example implements a console with basic coloring, completion "
        "(TAB key) and history (Up/Down keys). A more elaborate "
        "implementation may want to store entries along with extra data such "
        "as timestamp, emitter, etc.");
    Gui::TextWrapped("Enter 'HELP' for help.");

    // TODO: display items starting from the bottom

    if (Gui::SmallButton("Add Debug Text")) {
      AddLog("%d some text", Items.Size);
      AddLog("some more text");
      AddLog("display very important message here!");
    }
    Gui::SameLine();
    if (Gui::SmallButton("Add Debug Error")) {
      AddLog("[error] something went wrong");
    }
    Gui::SameLine();
    if (Gui::SmallButton("Clear")) {
      ClearLog();
    }
    Gui::SameLine();
    bool copy_to_clipboard = Gui::SmallButton("Copy");
    // static float t = 0.0f; if (Gui::GetTime() - t > 0.02f) { t =
    // Gui::GetTime(); AddLog("Spam %f", t); }

    Gui::Separator();

    // Options menu
    if (Gui::BeginPopup("Options")) {
      Gui::Checkbox("Auto-scroll", &AutoScroll);
      Gui::EndPopup();
    }

    // Options, Filter
    if (Gui::Button("Options"))
      Gui::OpenPopup("Options");
    Gui::SameLine();
    Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
    Gui::Separator();

    // Reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve =
        Gui::GetStyle().ItemSpacing.y + Gui::GetFrameHeightWithSpacing();
    if (Gui::BeginChild("ScrollingRegion", Vec2(0, -footer_height_to_reserve),
                        ChildFlags_None, WindowFlags_HorizontalScrollbar)) {
      if (Gui::BeginPopupContextWindow()) {
        if (Gui::Selectable("Clear"))
          ClearLog();
        Gui::EndPopup();
      }

      // Display every line as a separate entry so we can change their color or
      // add custom widgets. If you only want raw text you can use
      // Gui::TextUnformatted(log.begin(), log.end()); NB- if you have
      // thousands of entries this approach may be too inefficient and may
      // require user-side clipping to only process visible items. The clipper
      // will automatically measure the height of your first item and then
      // "seek" to display only items in the visible area.
      // To use the clipper we can replace your standard loop:
      //      for (int i = 0; i < Items.Size; i++)
      //   With:
      //      ListClipper clipper;
      //      clipper.Begin(Items.Size);
      //      while (clipper.Step())
      //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
      // - That your items are evenly spaced (same height)
      // - That you have cheap random access to your elements (you can access
      // them given their index,
      //   without processing all the ones before)
      // You cannot this code as-is if a filter is active because it breaks the
      // 'cheap random-access' property. We would need random-access on the
      // post-filtered list. A typical application wanting coarse clipping and
      // filtering may want to pre-compute an array of indices or offsets of
      // items that passed the filtering test, recomputing this array when user
      // changes the filter, and appending newly elements as they are inserted.
      // This is left as a task to the user until we can manage to improve this
      // example code! If your items are of variable height:
      // - Split them into same height items would be simpler and facilitate
      // random-seeking into your list.
      // - Consider using manual call to IsRectVisible() and skipping extraneous
      // decoration from your items.
      Gui::PushStyleVar(StyleVar_ItemSpacing, Vec2(4, 1)); // Tighten spacing
      if (copy_to_clipboard)
        Gui::LogToClipboard();
      for (const char *item : Items) {
        if (!Filter.PassFilter(item))
          continue;

        // Normally you would store more information in your item than just a
        // string. (e.g. make Items[] an array of structure, store color/type
        // etc.)
        Vec4 color;
        bool has_color = false;
        if (strstr(item, "[error]")) {
          color = Vec4(1.0f, 0.4f, 0.4f, 1.0f);
          has_color = true;
        } else if (strncmp(item, "# ", 2) == 0) {
          color = Vec4(1.0f, 0.8f, 0.6f, 1.0f);
          has_color = true;
        }
        if (has_color)
          Gui::PushStyleColor(Col_Text, color);
        Gui::TextUnformatted(item);
        if (has_color)
          Gui::PopStyleColor();
      }
      if (copy_to_clipboard)
        Gui::LogFinish();

      // Keep up at the bottom of the scroll region if we were already at the
      // bottom at the beginning of the frame. Using a scrollbar or mouse-wheel
      // will take away from the bottom edge.
      if (ScrollToBottom ||
          (AutoScroll && Gui::GetScrollY() >= Gui::GetScrollMaxY()))
        Gui::SetScrollHereY(1.0f);
      ScrollToBottom = false;

      Gui::PopStyleVar();
    }
    Gui::EndChild();
    Gui::Separator();

    // Command-line
    bool reclaim_focus = false;
    InputTextFlags input_text_flags =
        InputTextFlags_EnterReturnsTrue | InputTextFlags_EscapeClearsAll |
        InputTextFlags_CallbackCompletion | InputTextFlags_CallbackHistory;
    if (Gui::InputText("Input", InputBuf, ARRAYSIZE(InputBuf), input_text_flags,
                       &TextEditCallbackStub, (void *)this)) {
      char *s = InputBuf;
      Strtrim(s);
      if (s[0])
        ExecCommand(s);
      strcpy(s, "");
      reclaim_focus = true;
    }

    // Auto-focus on window apparition
    Gui::SetItemDefaultFocus();
    if (reclaim_focus)
      Gui::SetKeyboardFocusHere(-1); // Auto focus previous widget

    Gui::End();
  }

  void ExecCommand(const char *command_line) {
    AddLog("# %s\n", command_line);

    // Insert into history. First find match and delete it so it can be pushed
    // to the back. This isn't trying to be smart or optimal.
    HistoryPos = -1;
    for (int i = History.Size - 1; i >= 0; i--)
      if (Stricmp(History[i], command_line) == 0) {
        free(History[i]);
        History.erase(History.begin() + i);
        break;
      }
    History.push_back(Strdup(command_line));

    // Process command
    if (Stricmp(command_line, "CLEAR") == 0) {
      ClearLog();
    } else if (Stricmp(command_line, "HELP") == 0) {
      AddLog("Commands:");
      for (int i = 0; i < Commands.Size; i++)
        AddLog("- %s", Commands[i]);
    } else if (Stricmp(command_line, "HISTORY") == 0) {
      int first = History.Size - 10;
      for (int i = first > 0 ? first : 0; i < History.Size; i++)
        AddLog("%3d: %s\n", i, History[i]);
    } else {
      AddLog("Unknown command: '%s'\n", command_line);
    }

    // On command input, we scroll to bottom even if AutoScroll==false
    ScrollToBottom = true;
  }

  // In C++11 you'd be better off using lambdas for this sort of forwarding
  // callbacks
  static int TextEditCallbackStub(InputTextCallbackData *data) {
    ExampleAppConsole *console = (ExampleAppConsole *)data->UserData;
    return console->TextEditCallback(data);
  }

  int TextEditCallback(InputTextCallbackData *data) {
    // AddLog("cursor: %d, selection: %d-%d", data->CursorPos,
    // data->SelectionStart, data->SelectionEnd);
    switch (data->EventFlag) {
    case InputTextFlags_CallbackCompletion: {
      // Example of TEXT COMPLETION

      // Locate beginning of current word
      const char *word_end = data->Buf + data->CursorPos;
      const char *word_start = word_end;
      while (word_start > data->Buf) {
        const char c = word_start[-1];
        if (c == ' ' || c == '\t' || c == ',' || c == ';')
          break;
        word_start--;
      }

      // Build a list of candidates
      Vector<const char *> candidates;
      for (int i = 0; i < Commands.Size; i++)
        if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) ==
            0)
          candidates.push_back(Commands[i]);

      if (candidates.Size == 0) {
        // No match
        AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start),
               word_start);
      } else if (candidates.Size == 1) {
        // Single match. Delete the beginning of the word and replace it
        // entirely so we've got nice casing.
        data->DeleteChars((int)(word_start - data->Buf),
                          (int)(word_end - word_start));
        data->InsertChars(data->CursorPos, candidates[0]);
        data->InsertChars(data->CursorPos, " ");
      } else {
        // Multiple matches. Complete as much as we can..
        // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and
        // "CLASSIFY" as matches.
        int match_len = (int)(word_end - word_start);
        for (;;) {
          int c = 0;
          bool all_candidates_matches = true;
          for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
            if (i == 0)
              c = toupper(candidates[i][match_len]);
            else if (c == 0 || c != toupper(candidates[i][match_len]))
              all_candidates_matches = false;
          if (!all_candidates_matches)
            break;
          match_len++;
        }

        if (match_len > 0) {
          data->DeleteChars((int)(word_start - data->Buf),
                            (int)(word_end - word_start));
          data->InsertChars(data->CursorPos, candidates[0],
                            candidates[0] + match_len);
        }

        // List matches
        AddLog("Possible matches:\n");
        for (int i = 0; i < candidates.Size; i++)
          AddLog("- %s\n", candidates[i]);
      }

      break;
    }
    case InputTextFlags_CallbackHistory: {
      // Example of HISTORY
      const int prev_history_pos = HistoryPos;
      if (data->EventKey == Key_UpArrow) {
        if (HistoryPos == -1)
          HistoryPos = History.Size - 1;
        else if (HistoryPos > 0)
          HistoryPos--;
      } else if (data->EventKey == Key_DownArrow) {
        if (HistoryPos != -1)
          if (++HistoryPos >= History.Size)
            HistoryPos = -1;
      }

      // A better implementation would preserve the data on the current input
      // line along with cursor position.
      if (prev_history_pos != HistoryPos) {
        const char *history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
        data->DeleteChars(0, data->BufTextLen);
        data->InsertChars(0, history_str);
      }
    }
    }
    return 0;
  }
};

static void ShowExampleAppConsole(bool *p_open) {
  static ExampleAppConsole console;
  console.Draw("Example: Console", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
//-----------------------------------------------------------------------------

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog {
  TextBuffer Buf;
  TextFilter Filter;
  Vector<int> LineOffsets; // Index to lines offset. We maintain this with
                           // AddLog() calls.
  bool AutoScroll;         // Keep scrolling if already at the bottom.

  ExampleAppLog() {
    AutoScroll = true;
    Clear();
  }

  void Clear() {
    Buf.clear();
    LineOffsets.clear();
    LineOffsets.push_back(0);
  }

  void AddLog(const char *fmt, ...) FMTARGS(2) {
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n')
        LineOffsets.push_back(old_size + 1);
  }

  void Draw(const char *title, bool *p_open = NULL) {
    if (!Gui::Begin(title, p_open)) {
      Gui::End();
      return;
    }

    // Options menu
    if (Gui::BeginPopup("Options")) {
      Gui::Checkbox("Auto-scroll", &AutoScroll);
      Gui::EndPopup();
    }

    // Main window
    if (Gui::Button("Options"))
      Gui::OpenPopup("Options");
    Gui::SameLine();
    bool clear = Gui::Button("Clear");
    Gui::SameLine();
    bool copy = Gui::Button("Copy");
    Gui::SameLine();
    Filter.Draw("Filter", -100.0f);

    Gui::Separator();

    if (Gui::BeginChild("scrolling", Vec2(0, 0), ChildFlags_None,
                        WindowFlags_HorizontalScrollbar)) {
      if (clear)
        Clear();
      if (copy)
        Gui::LogToClipboard();

      Gui::PushStyleVar(StyleVar_ItemSpacing, Vec2(0, 0));
      const char *buf = Buf.begin();
      const char *buf_end = Buf.end();
      if (Filter.IsActive()) {
        // In this example we don't use the clipper when Filter is enabled.
        // This is because we don't have random access to the result of our
        // filter. A real application processing logs with ten of thousands of
        // entries may want to store the result of search/filter.. especially if
        // the filtering function is not trivial (e.g. reg-exp).
        for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
          const char *line_start = buf + LineOffsets[line_no];
          const char *line_end = (line_no + 1 < LineOffsets.Size)
                                     ? (buf + LineOffsets[line_no + 1] - 1)
                                     : buf_end;
          if (Filter.PassFilter(line_start, line_end))
            Gui::TextUnformatted(line_start, line_end);
        }
      } else {
        // The simplest and easy way to display the entire buffer:
        //   Gui::TextUnformatted(buf_begin, buf_end);
        // And it'll just work. TextUnformatted() has specialization for large
        // blob of text and will fast-forward to skip non-visible lines. Here we
        // instead demonstrate using the clipper to only process lines that are
        // within the visible area.
        // If you have tens of thousands of items and their processing cost is
        // non-negligible, coarse clipping them on your side is recommended.
        // Using ListClipper requires
        // - A) random access into your data
        // - B) items all being the  same height,
        // both of which we can handle since we have an array pointing to the
        // beginning of each line of text. When using the filter (in the block
        // of code above) we don't have random access into the data to display
        // anymore, which is why we don't use the clipper. Storing or skimming
        // through the search result would make it possible (and would be
        // recommended if you want to search through tens of thousands of
        // entries).
        ListClipper clipper;
        clipper.Begin(LineOffsets.Size);
        while (clipper.Step()) {
          for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd;
               line_no++) {
            const char *line_start = buf + LineOffsets[line_no];
            const char *line_end = (line_no + 1 < LineOffsets.Size)
                                       ? (buf + LineOffsets[line_no + 1] - 1)
                                       : buf_end;
            Gui::TextUnformatted(line_start, line_end);
          }
        }
        clipper.End();
      }
      Gui::PopStyleVar();

      // Keep up at the bottom of the scroll region if we were already at the
      // bottom at the beginning of the frame. Using a scrollbar or mouse-wheel
      // will take away from the bottom edge.
      if (AutoScroll && Gui::GetScrollY() >= Gui::GetScrollMaxY())
        Gui::SetScrollHereY(1.0f);
    }
    Gui::EndChild();
    Gui::End();
  }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool *p_open) {
  static ExampleAppLog log;

  // For the demo: add a debug button _BEFORE_ the normal log window contents
  // We take advantage of a rarely used feature: multiple calls to Begin()/End()
  // are appending to the _same_ window. Most of the contents of the window will
  // be added by the log.Draw() call.
  Gui::SetNextWindowSize(Vec2(500, 400), Cond_FirstUseEver);
  Gui::Begin("Example: Log", p_open);
  DEMO_MARKER("Examples/Log");
  if (Gui::SmallButton("[Debug] Add 5 entries")) {
    static int counter = 0;
    const char *categories[3] = {"info", "warn", "error"};
    const char *words[] = {"Bumfuzzled",    "Cattywampus",  "Snickersnee",
                           "Abibliophobia", "Absquatulate", "Nincompoop",
                           "Pauciloquent"};
    for (int n = 0; n < 5; n++) {
      const char *category = categories[counter % ARRAYSIZE(categories)];
      const char *word = words[counter % ARRAYSIZE(words)];
      log.AddLog(
          "[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
          Gui::GetFrameCount(), category, Gui::GetTime(), word);
      counter++;
    }
  }
  Gui::End();

  // Actually call in the regular Log helper (which will Begin() into the same
  // window as we just did)
  log.Draw("Example: Log", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool *p_open) {
  Gui::SetNextWindowSize(Vec2(500, 440), Cond_FirstUseEver);
  if (Gui::Begin("Example: Simple layout", p_open, WindowFlags_MenuBar)) {
    DEMO_MARKER("Examples/Simple layout");
    if (Gui::BeginMenuBar()) {
      if (Gui::BeginMenu("File")) {
        if (Gui::MenuItem("Close", "Ctrl+W")) {
          *p_open = false;
        }
        Gui::EndMenu();
      }
      Gui::EndMenuBar();
    }

    // Left
    static int selected = 0;
    {
      Gui::BeginChild("left pane", Vec2(150, 0),
                      ChildFlags_Border | ChildFlags_ResizeX);
      for (int i = 0; i < 100; i++) {
        // FIXME: Good candidate to use SelectableFlags_SelectOnNav
        char label[128];
        sprintf(label, "MyObject %d", i);
        if (Gui::Selectable(label, selected == i))
          selected = i;
      }
      Gui::EndChild();
    }
    Gui::SameLine();

    // Right
    {
      Gui::BeginGroup();
      Gui::BeginChild(
          "item view",
          Vec2(0, -Gui::GetFrameHeightWithSpacing())); // Leave room for 1
                                                       // line below us
      Gui::Text("MyObject: %d", selected);
      Gui::Separator();
      if (Gui::BeginTabBar("##Tabs", TabBarFlags_None)) {
        if (Gui::BeginTabItem("Description")) {
          Gui::TextWrapped(
              "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
              "eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
          Gui::EndTabItem();
        }
        if (Gui::BeginTabItem("Details")) {
          Gui::Text("ID: 0123456789");
          Gui::EndTabItem();
        }
        Gui::EndTabBar();
      }
      Gui::EndChild();
      if (Gui::Button("Revert")) {
      }
      Gui::SameLine();
      if (Gui::Button("Save")) {
      }
      Gui::EndGroup();
    }
  }
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
//-----------------------------------------------------------------------------

static void ShowPlaceholderObject(const char *prefix, int uid) {
  // Use object uid as identifier. Most commonly you could also use the object
  // pointer as a base ID.
  Gui::PushID(uid);

  // Text and Tree nodes are less high than framed widgets, using
  // AlignTextToFramePadding() we add vertical spacing to make the tree lines
  // equal high.
  Gui::TableNextRow();
  Gui::TableSetColumnIndex(0);
  Gui::AlignTextToFramePadding();
  bool node_open = Gui::TreeNode("Object", "%s_%u", prefix, uid);
  Gui::TableSetColumnIndex(1);
  Gui::Text("my sailor is rich");

  if (node_open) {
    static float placeholder_members[8] = {0.0f,    0.0f,   1.0f,
                                           3.1416f, 100.0f, 999.0f};
    for (int i = 0; i < 8; i++) {
      Gui::PushID(i); // Use field index as identifier.
      if (i < 2) {
        ShowPlaceholderObject("Child", 424242);
      } else {
        // Here we use a TreeNode to highlight on hover (we could use e.g.
        // Selectable as well)
        Gui::TableNextRow();
        Gui::TableSetColumnIndex(0);
        Gui::AlignTextToFramePadding();
        TreeNodeFlags flags = TreeNodeFlags_Leaf |
                              TreeNodeFlags_NoTreePushOnOpen |
                              TreeNodeFlags_Bullet;
        Gui::TreeNodeEx("Field", flags, "Field_%d", i);

        Gui::TableSetColumnIndex(1);
        Gui::SetNextItemWidth(-FLT_MIN);
        if (i >= 5)
          Gui::InputFloat("##value", &placeholder_members[i], 1.0f);
        else
          Gui::DragFloat("##value", &placeholder_members[i], 0.01f);
        Gui::NextColumn();
      }
      Gui::PopID();
    }
    Gui::TreePop();
  }
  Gui::PopID();
}

// Demonstrate create a simple property editor.
// This demo is a bit lackluster nowadays, would be nice to improve.
static void ShowExampleAppPropertyEditor(bool *p_open) {
  Gui::SetNextWindowSize(Vec2(430, 450), Cond_FirstUseEver);
  if (!Gui::Begin("Example: Property editor", p_open)) {
    Gui::End();
    return;
  }

  DEMO_MARKER("Examples/Property Editor");
  HelpMarker("This example shows how you may implement a property editor using "
             "two columns.\n"
             "All objects/fields data are dummies here.\n");

  Gui::PushStyleVar(StyleVar_FramePadding, Vec2(2, 2));
  if (Gui::BeginTable("##split", 2,
                      TableFlags_BordersOuter | TableFlags_Resizable |
                          TableFlags_ScrollY)) {
    Gui::TableSetupScrollFreeze(0, 1);
    Gui::TableSetupColumn("Object");
    Gui::TableSetupColumn("Contents");
    Gui::TableHeadersRow();

    // Iterate placeholder objects (all the same data)
    for (int obj_i = 0; obj_i < 4; obj_i++)
      ShowPlaceholderObject("Object", obj_i);

    Gui::EndTable();
  }
  Gui::PopStyleVar();
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
//-----------------------------------------------------------------------------

// Demonstrate/test rendering huge amount of text, and the incidence of
// clipping.
static void ShowExampleAppLongText(bool *p_open) {
  Gui::SetNextWindowSize(Vec2(520, 600), Cond_FirstUseEver);
  if (!Gui::Begin("Example: Long text display", p_open)) {
    Gui::End();
    return;
  }
  DEMO_MARKER("Examples/Long text display");

  static int test_type = 0;
  static TextBuffer log;
  static int lines = 0;
  Gui::Text("Printing unusually long amount of text.");
  Gui::Combo("Test type", &test_type,
             "Single call to TextUnformatted()\0"
             "Multiple calls to Text(), clipped\0"
             "Multiple calls to Text(), not clipped (slow)\0");
  Gui::Text("Buffer contents: %d lines, %d bytes", lines, log.size());
  if (Gui::Button("Clear")) {
    log.clear();
    lines = 0;
  }
  Gui::SameLine();
  if (Gui::Button("Add 1000 lines")) {
    for (int i = 0; i < 1000; i++)
      log.appendf("%i The quick brown fox jumps over the lazy dog\n",
                  lines + i);
    lines += 1000;
  }
  Gui::BeginChild("Log");
  switch (test_type) {
  case 0:
    // Single call to TextUnformatted() with a big buffer
    Gui::TextUnformatted(log.begin(), log.end());
    break;
  case 1: {
    // Multiple calls to Text(), manually coarsely clipped - demonstrate how to
    // use the ListClipper helper.
    Gui::PushStyleVar(StyleVar_ItemSpacing, Vec2(0, 0));
    ListClipper clipper;
    clipper.Begin(lines);
    while (clipper.Step())
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        Gui::Text("%i The quick brown fox jumps over the lazy dog", i);
    Gui::PopStyleVar();
    break;
  }
  case 2:
    // Multiple calls to Text(), not clipped (slow)
    Gui::PushStyleVar(StyleVar_ItemSpacing, Vec2(0, 0));
    for (int i = 0; i < lines; i++)
      Gui::Text("%i The quick brown fox jumps over the lazy dog", i);
    Gui::PopStyleVar();
    break;
  }
  Gui::EndChild();
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window which gets auto-resized according to its
// content.
static void ShowExampleAppAutoResize(bool *p_open) {
  if (!Gui::Begin("Example: Auto-resizing window", p_open,
                  WindowFlags_AlwaysAutoResize)) {
    Gui::End();
    return;
  }
  DEMO_MARKER("Examples/Auto-resizing window");

  static int lines = 10;
  Gui::TextUnformatted(
      "Window will resize every-frame to the size of its content.\n"
      "Note that you probably don't want to query the window size to\n"
      "output your content because that would create a feedback loop.");
  Gui::SliderInt("Number of lines", &lines, 1, 20);
  for (int i = 0; i < lines; i++)
    Gui::Text("%*sThis is line %d", i * 4, "",
              i); // Pad with space to extend size horizontally
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window with custom resize constraints.
// Note that size constraints currently don't work on a docked window (when in
// 'docking' branch)
static void ShowExampleAppConstrainedResize(bool *p_open) {
  struct CustomConstraints {
    // Helper functions to demonstrate programmatic constraints
    // FIXME: This doesn't take account of decoration size (e.g. title bar),
    // library should make this easier.
    // FIXME: None of the three demos works consistently when resizing from
    // borders.
    static void AspectRatio(SizeCallbackData *data) {
      float aspect_ratio = *(float *)data->UserData;
      data->DesiredSize.y = (float)(int)(data->DesiredSize.x / aspect_ratio);
    }
    static void Square(SizeCallbackData *data) {
      data->DesiredSize.x = data->DesiredSize.y =
          MAX(data->DesiredSize.x, data->DesiredSize.y);
    }
    static void Step(SizeCallbackData *data) {
      float step = *(float *)data->UserData;
      data->DesiredSize = Vec2((int)(data->DesiredSize.x / step + 0.5f) * step,
                               (int)(data->DesiredSize.y / step + 0.5f) * step);
    }
  };

  const char *test_desc[] = {
      "Between 100x100 and 500x500",
      "At least 100x100",
      "Resize vertical + lock current width",
      "Resize horizontal + lock current height",
      "Width Between 400 and 500",
      "Height at least 400",
      "Custom: Aspect Ratio 16:9",
      "Custom: Always Square",
      "Custom: Fixed Steps (100)",
  };

  // Options
  static bool auto_resize = false;
  static bool window_padding = true;
  static int type = 6; // Aspect Ratio
  static int display_lines = 10;

  // Submit constraint
  float aspect_ratio = 16.0f / 9.0f;
  float fixed_step = 100.0f;
  if (type == 0)
    Gui::SetNextWindowSizeConstraints(
        Vec2(100, 100), Vec2(500, 500)); // Between 100x100 and 500x500
  if (type == 1)
    Gui::SetNextWindowSizeConstraints(
        Vec2(100, 100), Vec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
  if (type == 2)
    Gui::SetNextWindowSizeConstraints(
        Vec2(-1, 0), Vec2(-1, FLT_MAX)); // Resize vertical + lock current width
  if (type == 3)
    Gui::SetNextWindowSizeConstraints(
        Vec2(0, -1),
        Vec2(FLT_MAX, -1)); // Resize horizontal + lock current height
  if (type == 4)
    Gui::SetNextWindowSizeConstraints(
        Vec2(400, -1), Vec2(500, -1)); // Width Between and 400 and 500
  if (type == 5)
    Gui::SetNextWindowSizeConstraints(Vec2(-1, 500),
                                      Vec2(-1, FLT_MAX)); // Height at least 400
  if (type == 6)
    Gui::SetNextWindowSizeConstraints(Vec2(0, 0), Vec2(FLT_MAX, FLT_MAX),
                                      CustomConstraints::AspectRatio,
                                      (void *)&aspect_ratio); // Aspect ratio
  if (type == 7)
    Gui::SetNextWindowSizeConstraints(
        Vec2(0, 0), Vec2(FLT_MAX, FLT_MAX),
        CustomConstraints::Square); // Always Square
  if (type == 8)
    Gui::SetNextWindowSizeConstraints(Vec2(0, 0), Vec2(FLT_MAX, FLT_MAX),
                                      CustomConstraints::Step,
                                      (void *)&fixed_step); // Fixed Step

  // Submit window
  if (!window_padding)
    Gui::PushStyleVar(StyleVar_WindowPadding, Vec2(0.0f, 0.0f));
  const WindowFlags window_flags =
      auto_resize ? WindowFlags_AlwaysAutoResize : 0;
  const bool window_open =
      Gui::Begin("Example: Constrained Resize", p_open, window_flags);
  if (!window_padding)
    Gui::PopStyleVar();
  if (window_open) {
    DEMO_MARKER("Examples/Constrained Resizing window");
    if (Gui::GetIO().KeyShift) {
      // Display a dummy viewport (in your real app you would likely use
      // ImageButton() to display a texture.
      Vec2 avail_size = Gui::GetContentRegionAvail();
      Vec2 pos = Gui::GetCursorScreenPos();
      Gui::ColorButton("viewport", Vec4(0.5f, 0.2f, 0.5f, 1.0f),
                       ColorEditFlags_NoTooltip | ColorEditFlags_NoDragDrop,
                       avail_size);
      Gui::SetCursorScreenPos(Vec2(pos.x + 10, pos.y + 10));
      Gui::Text("%.2f x %.2f", avail_size.x, avail_size.y);
    } else {
      Gui::Text("(Hold SHIFT to display a dummy viewport)");
      if (Gui::IsWindowDocked())
        Gui::Text(
            "Warning: Sizing Constraints won't work if the window is docked!");
      if (Gui::Button("Set 200x200")) {
        Gui::SetWindowSize(Vec2(200, 200));
      }
      Gui::SameLine();
      if (Gui::Button("Set 500x500")) {
        Gui::SetWindowSize(Vec2(500, 500));
      }
      Gui::SameLine();
      if (Gui::Button("Set 800x200")) {
        Gui::SetWindowSize(Vec2(800, 200));
      }
      Gui::SetNextItemWidth(Gui::GetFontSize() * 20);
      Gui::Combo("Constraint", &type, test_desc, ARRAYSIZE(test_desc));
      Gui::SetNextItemWidth(Gui::GetFontSize() * 20);
      Gui::DragInt("Lines", &display_lines, 0.2f, 1, 100);
      Gui::Checkbox("Auto-resize", &auto_resize);
      Gui::Checkbox("Window padding", &window_padding);
      for (int i = 0; i < display_lines; i++)
        Gui::Text(
            "%*sHello, sailor! Making this line long enough for the example.",
            i * 4, "");
    }
  }
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool *p_open) {
  static int location = 0;
  IO &io = Gui::GetIO();
  WindowFlags window_flags = WindowFlags_NoDecoration | WindowFlags_NoDocking |
                             WindowFlags_AlwaysAutoResize |
                             WindowFlags_NoSavedSettings |
                             WindowFlags_NoFocusOnAppearing | WindowFlags_NoNav;
  if (location >= 0) {
    const float PAD = 10.0f;
    const Viewport *viewport = Gui::GetMainViewport();
    Vec2 work_pos =
        viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    Vec2 work_size = viewport->WorkSize;
    Vec2 window_pos, window_pos_pivot;
    window_pos.x =
        (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
    window_pos.y =
        (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
    window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
    window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
    Gui::SetNextWindowPos(window_pos, Cond_Always, window_pos_pivot);
    Gui::SetNextWindowViewport(viewport->ID);
    window_flags |= WindowFlags_NoMove;
  } else if (location == -2) {
    // Center window
    Gui::SetNextWindowPos(Gui::GetMainViewport()->GetCenter(), Cond_Always,
                          Vec2(0.5f, 0.5f));
    window_flags |= WindowFlags_NoMove;
  }
  Gui::SetNextWindowBgAlpha(0.35f); // Transparent background
  if (Gui::Begin("Example: Simple overlay", p_open, window_flags)) {
    DEMO_MARKER("Examples/Simple Overlay");
    Gui::Text("Simple overlay\n"
              "(right-click to change position)");
    Gui::Separator();
    if (Gui::IsMousePosValid())
      Gui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
    else
      Gui::Text("Mouse Position: <invalid>");
    if (Gui::BeginPopupContextWindow()) {
      if (Gui::MenuItem("Custom", NULL, location == -1))
        location = -1;
      if (Gui::MenuItem("Center", NULL, location == -2))
        location = -2;
      if (Gui::MenuItem("Top-left", NULL, location == 0))
        location = 0;
      if (Gui::MenuItem("Top-right", NULL, location == 1))
        location = 1;
      if (Gui::MenuItem("Bottom-left", NULL, location == 2))
        location = 2;
      if (Gui::MenuItem("Bottom-right", NULL, location == 3))
        location = 3;
      if (p_open && Gui::MenuItem("Close"))
        *p_open = false;
      Gui::EndPopup();
    }
  }
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
//-----------------------------------------------------------------------------

// Demonstrate creating a window covering the entire screen/viewport
static void ShowExampleAppFullscreen(bool *p_open) {
  static bool use_work_area = true;
  static WindowFlags flags = WindowFlags_NoDecoration | WindowFlags_NoMove |
                             WindowFlags_NoSavedSettings;

  // We demonstrate using the full viewport area or the work area (without
  // menu-bars, task-bars etc.) Based on your use case you may want one or the
  // other.
  const Viewport *viewport = Gui::GetMainViewport();
  Gui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
  Gui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

  if (Gui::Begin("Example: Fullscreen window", p_open, flags)) {
    Gui::Checkbox("Use work area instead of main area", &use_work_area);
    Gui::SameLine();
    HelpMarker(
        "Main Area = entire viewport,\nWork Area = entire viewport minus "
        "sections used by the main menu bars, task bars etc.\n\nEnable the "
        "main-menu bar in Examples menu to see the difference.");

    Gui::CheckboxFlags("WindowFlags_NoBackground", &flags,
                       WindowFlags_NoBackground);
    Gui::CheckboxFlags("WindowFlags_NoDecoration", &flags,
                       WindowFlags_NoDecoration);
    Gui::Indent();
    Gui::CheckboxFlags("WindowFlags_NoTitleBar", &flags,
                       WindowFlags_NoTitleBar);
    Gui::CheckboxFlags("WindowFlags_NoCollapse", &flags,
                       WindowFlags_NoCollapse);
    Gui::CheckboxFlags("WindowFlags_NoScrollbar", &flags,
                       WindowFlags_NoScrollbar);
    Gui::Unindent();

    if (p_open && Gui::Button("Close this window"))
      *p_open = false;
  }
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Manipulating Window Titles /
// ShowExampleAppWindowTitles()
//-----------------------------------------------------------------------------

// Demonstrate the use of "##" and "###" in identifiers to manipulate ID
// generation. This applies to all regular items as well. Read FAQ section "How
// can I have multiple widgets with the same label?" for details.
static void ShowExampleAppWindowTitles(bool *) {
  const Viewport *viewport = Gui::GetMainViewport();
  const Vec2 base_pos = viewport->Pos;

  // By default, Windows are uniquely identified by their title.
  // You can use the "##" and "###" markers to manipulate the display/ID.

  // Using "##" to display same title but have unique identifier.
  Gui::SetNextWindowPos(Vec2(base_pos.x + 100, base_pos.y + 100),
                        Cond_FirstUseEver);
  Gui::Begin("Same title as another window##1");
  DEMO_MARKER("Examples/Manipulating window titles");
  Gui::Text("This is window 1.\nMy title is the same as window 2, but my "
            "identifier is unique.");
  Gui::End();

  Gui::SetNextWindowPos(Vec2(base_pos.x + 100, base_pos.y + 200),
                        Cond_FirstUseEver);
  Gui::Begin("Same title as another window##2");
  Gui::Text("This is window 2.\nMy title is the same as window 1, but my "
            "identifier is unique.");
  Gui::End();

  // Using "###" to display a changing title but keep a static identifier
  // "AnimatedTitle"
  char buf[128];
  sprintf(buf, "Animated title %c %d###AnimatedTitle",
          "|/-\\"[(int)(Gui::GetTime() / 0.25f) & 3], Gui::GetFrameCount());
  Gui::SetNextWindowPos(Vec2(base_pos.x + 100, base_pos.y + 300),
                        Cond_FirstUseEver);
  Gui::Begin(buf);
  Gui::Text("This window has a changing title.");
  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using DrawList API /
// ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------

// Demonstrate using the low-level DrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool *p_open) {
  if (!Gui::Begin("Example: Custom rendering", p_open)) {
    Gui::End();
    return;
  }
  DEMO_MARKER("Examples/Custom Rendering");

  // Tip: If you do a lot of custom rendering, you probably want to use your own
  // geometrical types and benefit of overloaded operators, etc. Define
  // VEC2_CLASS_EXTRA in config.hpp to create implicit conversions between
  // your types and Vec2/Vec4. Gui defines overloaded operators but
  // they are internal to gui.cpp and not exposed outside (to avoid messing
  // with your types) In this example we are not using the maths operators!

  if (Gui::BeginTabBar("##TabBar")) {
    if (Gui::BeginTabItem("Primitives")) {
      Gui::PushItemWidth(-Gui::GetFontSize() * 15);
      DrawList *draw_list = Gui::GetWindowDrawList();

      // Draw gradients
      // (note that those are currently exacerbating our sRGB/Linear issues)
      // Calling Gui::GetColorU32() multiplies the given colors by the current
      // Style Alpha, but you may pass the COL32() directly as well..
      Gui::Text("Gradients");
      Vec2 gradient_size = Vec2(Gui::CalcItemWidth(), Gui::GetFrameHeight());
      {
        Vec2 p0 = Gui::GetCursorScreenPos();
        Vec2 p1 = Vec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
        U32 col_a = Gui::GetColorU32(COL32(0, 0, 0, 255));
        U32 col_b = Gui::GetColorU32(COL32(255, 255, 255, 255));
        draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
        Gui::InvisibleButton("##gradient1", gradient_size);
      }
      {
        Vec2 p0 = Gui::GetCursorScreenPos();
        Vec2 p1 = Vec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
        U32 col_a = Gui::GetColorU32(COL32(0, 255, 0, 255));
        U32 col_b = Gui::GetColorU32(COL32(255, 0, 0, 255));
        draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
        Gui::InvisibleButton("##gradient2", gradient_size);
      }

      // Draw a bunch of primitives
      Gui::Text("All primitives");
      static float sz = 36.0f;
      static float thickness = 3.0f;
      static int ngon_sides = 6;
      static bool circle_segments_override = false;
      static int circle_segments_override_v = 12;
      static bool curve_segments_override = false;
      static int curve_segments_override_v = 8;
      static Vec4 colf = Vec4(1.0f, 1.0f, 0.4f, 1.0f);
      Gui::DragFloat("Size", &sz, 0.2f, 2.0f, 100.0f, "%.0f");
      Gui::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
      Gui::SliderInt("N-gon sides", &ngon_sides, 3, 12);
      Gui::Checkbox("##circlesegmentoverride", &circle_segments_override);
      Gui::SameLine(0.0f, Gui::GetStyle().ItemInnerSpacing.x);
      circle_segments_override |= Gui::SliderInt(
          "Circle segments override", &circle_segments_override_v, 3, 40);
      Gui::Checkbox("##curvessegmentoverride", &curve_segments_override);
      Gui::SameLine(0.0f, Gui::GetStyle().ItemInnerSpacing.x);
      curve_segments_override |= Gui::SliderInt(
          "Curves segments override", &curve_segments_override_v, 3, 40);
      Gui::ColorEdit4("Color", &colf.x);

      const Vec2 p = Gui::GetCursorScreenPos();
      const U32 col = Color(colf);
      const float spacing = 10.0f;
      const DrawFlags corners_tl_br =
          DrawFlags_RoundCornersTopLeft | DrawFlags_RoundCornersBottomRight;
      const float rounding = sz / 5.0f;
      const int circle_segments =
          circle_segments_override ? circle_segments_override_v : 0;
      const int curve_segments =
          curve_segments_override ? curve_segments_override_v : 0;
      float x = p.x + 4.0f;
      float y = p.y + 4.0f;
      for (int n = 0; n < 2; n++) {
        // First line uses a thickness of 1.0f, second line uses the
        // configurable thickness
        float th = (n == 0) ? 1.0f : thickness;
        draw_list->AddNgon(Vec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col,
                           ngon_sides, th);
        x += sz + spacing; // N-gon
        draw_list->AddCircle(Vec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col,
                             circle_segments, th);
        x += sz + spacing; // Circle
        draw_list->AddEllipse(Vec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f,
                              sz * 0.3f, col, -0.3f, circle_segments, th);
        x += sz + spacing; // Ellipse
        draw_list->AddRect(Vec2(x, y), Vec2(x + sz, y + sz), col, 0.0f,
                           DrawFlags_None, th);
        x += sz + spacing; // Square
        draw_list->AddRect(Vec2(x, y), Vec2(x + sz, y + sz), col, rounding,
                           DrawFlags_None, th);
        x += sz + spacing; // Square with all rounded corners
        draw_list->AddRect(Vec2(x, y), Vec2(x + sz, y + sz), col, rounding,
                           corners_tl_br, th);
        x += sz + spacing; // Square with two rounded corners
        draw_list->AddTriangle(Vec2(x + sz * 0.5f, y),
                               Vec2(x + sz, y + sz - 0.5f),
                               Vec2(x, y + sz - 0.5f), col, th);
        x += sz + spacing; // Triangle
        // draw_list->AddTriangle(Vec2(x+sz*0.2f,y), Vec2(x, y+sz-0.5f),
        // Vec2(x+sz*0.4f, y+sz-0.5f), col, th);x+= sz*0.4f + spacing; // Thin
        // triangle
        draw_list->AddLine(Vec2(x, y), Vec2(x + sz, y), col, th);
        x += sz + spacing; // Horizontal line (note: drawing a filled rectangle
                           // will be faster!)
        draw_list->AddLine(Vec2(x, y), Vec2(x, y + sz), col, th);
        x += spacing; // Vertical line (note: drawing a filled rectangle will be
                      // faster!)
        draw_list->AddLine(Vec2(x, y), Vec2(x + sz, y + sz), col, th);
        x += sz + spacing; // Diagonal line

        // Quadratic Bezier Curve (3 control points)
        Vec2 cp3[3] = {Vec2(x, y + sz * 0.6f),
                       Vec2(x + sz * 0.5f, y - sz * 0.4f),
                       Vec2(x + sz, y + sz)};
        draw_list->AddBezierQuadratic(cp3[0], cp3[1], cp3[2], col, th,
                                      curve_segments);
        x += sz + spacing;

        // Cubic Bezier Curve (4 control points)
        Vec2 cp4[4] = {Vec2(x, y), Vec2(x + sz * 1.3f, y + sz * 0.3f),
                       Vec2(x + sz - sz * 1.3f, y + sz - sz * 0.3f),
                       Vec2(x + sz, y + sz)};
        draw_list->AddBezierCubic(cp4[0], cp4[1], cp4[2], cp4[3], col, th,
                                  curve_segments);

        x = p.x + 4;
        y += sz + spacing;
      }
      draw_list->AddNgonFilled(Vec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f,
                               col, ngon_sides);
      x += sz + spacing; // N-gon
      draw_list->AddCircleFilled(Vec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f,
                                 col, circle_segments);
      x += sz + spacing; // Circle
      draw_list->AddEllipseFilled(Vec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f,
                                  sz * 0.3f, col, -0.3f, circle_segments);
      x += sz + spacing; // Ellipse
      draw_list->AddRectFilled(Vec2(x, y), Vec2(x + sz, y + sz), col);
      x += sz + spacing; // Square
      draw_list->AddRectFilled(Vec2(x, y), Vec2(x + sz, y + sz), col, 10.0f);
      x += sz + spacing; // Square with all rounded corners
      draw_list->AddRectFilled(Vec2(x, y), Vec2(x + sz, y + sz), col, 10.0f,
                               corners_tl_br);
      x += sz + spacing; // Square with two rounded corners
      draw_list->AddTriangleFilled(Vec2(x + sz * 0.5f, y),
                                   Vec2(x + sz, y + sz - 0.5f),
                                   Vec2(x, y + sz - 0.5f), col);
      x += sz + spacing; // Triangle
      // draw_list->AddTriangleFilled(Vec2(x+sz*0.2f,y), Vec2(x, y+sz-0.5f),
      // Vec2(x+sz*0.4f, y+sz-0.5f), col); x += sz*0.4f + spacing; // Thin
      // triangle
      draw_list->AddRectFilled(Vec2(x, y), Vec2(x + sz, y + thickness), col);
      x += sz + spacing; // Horizontal line (faster than AddLine, but only
                         // handle integer thickness)
      draw_list->AddRectFilled(Vec2(x, y), Vec2(x + thickness, y + sz), col);
      x += spacing * 2.0f; // Vertical line (faster than AddLine, but only
                           // handle integer thickness)
      draw_list->AddRectFilled(Vec2(x, y), Vec2(x + 1, y + 1), col);
      x += sz; // Pixel (faster than AddLine)
      draw_list->AddRectFilledMultiColor(
          Vec2(x, y), Vec2(x + sz, y + sz), COL32(0, 0, 0, 255),
          COL32(255, 0, 0, 255), COL32(255, 255, 0, 255),
          COL32(0, 255, 0, 255));

      Gui::Dummy(Vec2((sz + spacing) * 11.2f, (sz + spacing) * 3.0f));
      Gui::PopItemWidth();
      Gui::EndTabItem();
    }

    if (Gui::BeginTabItem("Canvas")) {
      static Vector<Vec2> points;
      static Vec2 scrolling(0.0f, 0.0f);
      static bool opt_enable_grid = true;
      static bool opt_enable_context_menu = true;
      static bool adding_line = false;

      Gui::Checkbox("Enable grid", &opt_enable_grid);
      Gui::Checkbox("Enable context menu", &opt_enable_context_menu);
      Gui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to "
                "scroll, click for context menu.");

      // Typically you would use a BeginChild()/EndChild() pair to benefit from
      // a clipping region + own scrolling. Here we demonstrate that this can be
      // replaced by simple offsetting + custom drawing +
      // PushClipRect/PopClipRect() calls. To use a child window instead we
      // could use, e.g:
      //      Gui::PushStyleVar(StyleVar_WindowPadding, Vec2(0, 0)); //
      //      Disable padding Gui::PushStyleColor(Col_ChildBg,
      //      COL32(50, 50, 50, 255));  // Set a background color
      //      Gui::BeginChild("canvas", Vec2(0.0f, 0.0f),
      //      ChildFlags_Border, WindowFlags_NoMove);
      //      Gui::PopStyleColor();
      //      Gui::PopStyleVar();
      //      [...]
      //      Gui::EndChild();

      // Using InvisibleButton() as a convenience 1) it will advance the layout
      // cursor and 2) allows us to use IsItemHovered()/IsItemActive()
      Vec2 canvas_p0 = Gui::GetCursorScreenPos(); // DrawList API uses
                                                  // screen coordinates!
      Vec2 canvas_sz =
          Gui::GetContentRegionAvail(); // Resize canvas to what's available
      if (canvas_sz.x < 50.0f)
        canvas_sz.x = 50.0f;
      if (canvas_sz.y < 50.0f)
        canvas_sz.y = 50.0f;
      Vec2 canvas_p1 =
          Vec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

      // Draw border and background color
      IO &io = Gui::GetIO();
      DrawList *draw_list = Gui::GetWindowDrawList();
      draw_list->AddRectFilled(canvas_p0, canvas_p1, COL32(50, 50, 50, 255));
      draw_list->AddRect(canvas_p0, canvas_p1, COL32(255, 255, 255, 255));

      // This will catch our interactions
      Gui::InvisibleButton("canvas", canvas_sz,
                           ButtonFlags_MouseButtonLeft |
                               ButtonFlags_MouseButtonRight);
      const bool is_hovered = Gui::IsItemHovered(); // Hovered
      const bool is_active = Gui::IsItemActive();   // Held
      const Vec2 origin(canvas_p0.x + scrolling.x,
                        canvas_p0.y + scrolling.y); // Lock scrolled origin
      const Vec2 mouse_pos_in_canvas(io.MousePos.x - origin.x,
                                     io.MousePos.y - origin.y);

      // Add first and second point
      if (is_hovered && !adding_line && Gui::IsMouseClicked(MouseButton_Left)) {
        points.push_back(mouse_pos_in_canvas);
        points.push_back(mouse_pos_in_canvas);
        adding_line = true;
      }
      if (adding_line) {
        points.back() = mouse_pos_in_canvas;
        if (!Gui::IsMouseDown(MouseButton_Left))
          adding_line = false;
      }

      // Pan (we use a zero mouse threshold when there's no context menu)
      // You may decide to make that threshold dynamic based on whether the
      // mouse is hovering something etc.
      const float mouse_threshold_for_pan =
          opt_enable_context_menu ? -1.0f : 0.0f;
      if (is_active &&
          Gui::IsMouseDragging(MouseButton_Right, mouse_threshold_for_pan)) {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
      }

      // Context menu (under default mouse threshold)
      Vec2 drag_delta = Gui::GetMouseDragDelta(MouseButton_Right);
      if (opt_enable_context_menu && drag_delta.x == 0.0f &&
          drag_delta.y == 0.0f)
        Gui::OpenPopupOnItemClick("context", PopupFlags_MouseButtonRight);
      if (Gui::BeginPopup("context")) {
        if (adding_line)
          points.resize(points.size() - 2);
        adding_line = false;
        if (Gui::MenuItem("Remove one", NULL, false, points.Size > 0)) {
          points.resize(points.size() - 2);
        }
        if (Gui::MenuItem("Remove all", NULL, false, points.Size > 0)) {
          points.clear();
        }
        Gui::EndPopup();
      }

      // Draw grid + all lines in the canvas
      draw_list->PushClipRect(canvas_p0, canvas_p1, true);
      if (opt_enable_grid) {
        const float GRID_STEP = 64.0f;
        for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x;
             x += GRID_STEP)
          draw_list->AddLine(Vec2(canvas_p0.x + x, canvas_p0.y),
                             Vec2(canvas_p0.x + x, canvas_p1.y),
                             COL32(200, 200, 200, 40));
        for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y;
             y += GRID_STEP)
          draw_list->AddLine(Vec2(canvas_p0.x, canvas_p0.y + y),
                             Vec2(canvas_p1.x, canvas_p0.y + y),
                             COL32(200, 200, 200, 40));
      }
      for (int n = 0; n < points.Size; n += 2)
        draw_list->AddLine(
            Vec2(origin.x + points[n].x, origin.y + points[n].y),
            Vec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y),
            COL32(255, 255, 0, 255), 2.0f);
      draw_list->PopClipRect();

      Gui::EndTabItem();
    }

    if (Gui::BeginTabItem("BG/FG draw lists")) {
      static bool draw_bg = true;
      static bool draw_fg = true;
      Gui::Checkbox("Draw in Background draw list", &draw_bg);
      Gui::SameLine();
      HelpMarker("The Background draw list will be rendered below every  "
                 "Gui windows.");
      Gui::Checkbox("Draw in Foreground draw list", &draw_fg);
      Gui::SameLine();
      HelpMarker("The Foreground draw list will be rendered over every  "
                 "Gui windows.");
      Vec2 window_pos = Gui::GetWindowPos();
      Vec2 window_size = Gui::GetWindowSize();
      Vec2 window_center = Vec2(window_pos.x + window_size.x * 0.5f,
                                window_pos.y + window_size.y * 0.5f);
      if (draw_bg)
        Gui::GetBackgroundDrawList()->AddCircle(
            window_center, window_size.x * 0.6f, COL32(255, 0, 0, 200), 0,
            10 + 4);
      if (draw_fg)
        Gui::GetForegroundDrawList()->AddCircle(
            window_center, window_size.y * 0.6f, COL32(0, 255, 0, 200), 0, 10);
      Gui::EndTabItem();
    }

    // Demonstrate out-of-order rendering via channels splitting
    // We use functions in DrawList as each draw list contains a convenience
    // splitter, but you can also instantiate your own DrawListSplitter if you
    // need to nest them.
    if (Gui::BeginTabItem("Draw Channels")) {
      DrawList *draw_list = Gui::GetWindowDrawList();
      {
        Gui::Text("Blue shape is drawn first: appears in back");
        Gui::Text("Red shape is drawn after: appears in front");
        Vec2 p0 = Gui::GetCursorScreenPos();
        draw_list->AddRectFilled(Vec2(p0.x, p0.y), Vec2(p0.x + 50, p0.y + 50),
                                 COL32(0, 0, 255, 255)); // Blue
        draw_list->AddRectFilled(Vec2(p0.x + 25, p0.y + 25),
                                 Vec2(p0.x + 75, p0.y + 75),
                                 COL32(255, 0, 0, 255)); // Red
        Gui::Dummy(Vec2(75, 75));
      }
      Gui::Separator();
      {
        Gui::Text(
            "Blue shape is drawn first, into channel 1: appears in front");
        Gui::Text("Red shape is drawn after, into channel 0: appears in back");
        Vec2 p1 = Gui::GetCursorScreenPos();

        // Create 2 channels and draw a Blue shape THEN a Red shape.
        // You can create any number of channels. Tables API use 1 channel per
        // column in order to better batch draw calls.
        draw_list->ChannelsSplit(2);
        draw_list->ChannelsSetCurrent(1);
        draw_list->AddRectFilled(Vec2(p1.x, p1.y), Vec2(p1.x + 50, p1.y + 50),
                                 COL32(0, 0, 255, 255)); // Blue
        draw_list->ChannelsSetCurrent(0);
        draw_list->AddRectFilled(Vec2(p1.x + 25, p1.y + 25),
                                 Vec2(p1.x + 75, p1.y + 75),
                                 COL32(255, 0, 0, 255)); // Red

        // Flatten/reorder channels. Red shape is in channel 0 and it appears
        // below the Blue shape in channel 1. This works by copying draw indices
        // only (vertices are not copied).
        draw_list->ChannelsMerge();
        Gui::Dummy(Vec2(75, 75));
        Gui::Text(
            "After reordering, contents of channel 0 appears below channel 1.");
      }
      Gui::EndTabItem();
    }

    Gui::EndTabBar();
  }

  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
//-----------------------------------------------------------------------------

// Demonstrate using DockSpace() to create an explicit docking node within an
// existing window. Note: You can use most Docking facilities without calling
// any API. You DO NOT need to call DockSpace() to use Docking!
// - Drag from window title bar or their tab to dock/undock. Hold SHIFT to
// disable docking.
// - Drag from window menu button (upper-left button) to undock an entire node
// (all windows).
// - When io.ConfigDockingWithShift == true, you instead need to hold SHIFT to
// enable docking. About dockspaces:
// - Use DockSpace() to create an explicit dock node _within_ an existing
// window.
// - Use DockSpaceOverViewport() to create an explicit dock node covering the
// screen or a specific viewport.
//   This is often used with DockNodeFlags_PassthruCentralNode.
// - Important: Dockspaces need to be submitted _before_ any window they can
// host. Submit it early in your frame! (*)
// - Important: Dockspaces need to be kept alive if hidden, otherwise windows
// docked into it will be undocked.
//   e.g. if you have multiple tabs with a dockspace inside each tab: submit the
//   non-visible dockspaces with DockNodeFlags_KeepAliveOnly.
// (*) because of this constraint, the implicit \"Debug\" window can not be
// docked into an explicit DockSpace() node, because that window is submitted as
// part of the part of the NewFrame() call. An easy workaround is that you can
// create your own implicit "Debug##2" window after calling DockSpace() and
// leave it in the window stack for anyone to use.
void ShowExampleAppDockSpace(bool *p_open) {
  // READ THIS !!!
  // TL;DR; this demo is more complicated than what most users you would
  // normally use. If we remove all options we are showcasing, this demo would
  // become:
  //     void ShowExampleAppDockSpace()
  //     {
  //         Gui::DockSpaceOverViewport(Gui::GetMainViewport());
  //     }
  // In most cases you should be able to just call DockSpaceOverViewport() and
  // ignore all the code below! In this specific demo, we are not using
  // DockSpaceOverViewport() because:
  // - (1) we allow the host window to be floating/moveable instead of filling
  // the viewport (when opt_fullscreen == false)
  // - (2) we allow the host window to have padding (when opt_padding == true)
  // - (3) we expose many flags and need a way to have them visible.
  // - (4) we have a local menu bar in the host window (vs. you could use
  // BeginMainMenuBar() + DockSpaceOverViewport()
  //      in your code, but we don't here because we allow the window to be
  //      floating)

  static bool opt_fullscreen = true;
  static bool opt_padding = false;
  static DockNodeFlags dockspace_flags = DockNodeFlags_None;

  // We are using the WindowFlags_NoDocking flag to make the parent window
  // not dockable into, because it would be confusing to have two docking
  // targets within each others.
  WindowFlags window_flags = WindowFlags_MenuBar | WindowFlags_NoDocking;
  if (opt_fullscreen) {
    const Viewport *viewport = Gui::GetMainViewport();
    Gui::SetNextWindowPos(viewport->WorkPos);
    Gui::SetNextWindowSize(viewport->WorkSize);
    Gui::SetNextWindowViewport(viewport->ID);
    Gui::PushStyleVar(StyleVar_WindowRounding, 0.0f);
    Gui::PushStyleVar(StyleVar_WindowBorderSize, 0.0f);
    window_flags |= WindowFlags_NoTitleBar | WindowFlags_NoCollapse |
                    WindowFlags_NoResize | WindowFlags_NoMove;
    window_flags |= WindowFlags_NoBringToFrontOnFocus | WindowFlags_NoNavFocus;
  } else {
    dockspace_flags &= ~DockNodeFlags_PassthruCentralNode;
  }

  // When using DockNodeFlags_PassthruCentralNode, DockSpace() will render
  // our background and handle the pass-thru hole, so we ask Begin() to not
  // render a background.
  if (dockspace_flags & DockNodeFlags_PassthruCentralNode)
    window_flags |= WindowFlags_NoBackground;

  // Important: note that we proceed even if Begin() returns false (aka window
  // is collapsed). This is because we want to keep our DockSpace() active. If a
  // DockSpace() is inactive, all active windows docked into it will lose their
  // parent and become undocked. We cannot preserve the docking relationship
  // between an active window and an inactive docking, otherwise any change of
  // dockspace/settings would lead to windows being stuck in limbo and never
  // being visible.
  if (!opt_padding)
    Gui::PushStyleVar(StyleVar_WindowPadding, Vec2(0.0f, 0.0f));
  Gui::Begin("DockSpace Demo", p_open, window_flags);
  if (!opt_padding)
    Gui::PopStyleVar();

  if (opt_fullscreen)
    Gui::PopStyleVar(2);

  // Submit the DockSpace
  IO &io = Gui::GetIO();
  if (io.ConfigFlags & ConfigFlags_DockingEnable) {
    ID dockspace_id = Gui::GetID("MyDockSpace");
    Gui::DockSpace(dockspace_id, Vec2(0.0f, 0.0f), dockspace_flags);
  } else {
    ShowDockingDisabledMessage();
  }

  if (Gui::BeginMenuBar()) {
    if (Gui::BeginMenu("Options")) {
      // Disabling fullscreen would allow the window to be moved to the front of
      // other windows, which we can't undo at the moment without finer window
      // depth/z control.
      Gui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
      Gui::MenuItem("Padding", NULL, &opt_padding);
      Gui::Separator();

      if (Gui::MenuItem("Flag: NoDockingOverCentralNode", "",
                        (dockspace_flags &
                         DockNodeFlags_NoDockingOverCentralNode) != 0)) {
        dockspace_flags ^= DockNodeFlags_NoDockingOverCentralNode;
      }
      if (Gui::MenuItem("Flag: NoDockingSplit", "",
                        (dockspace_flags & DockNodeFlags_NoDockingSplit) !=
                            0)) {
        dockspace_flags ^= DockNodeFlags_NoDockingSplit;
      }
      if (Gui::MenuItem("Flag: NoUndocking", "",
                        (dockspace_flags & DockNodeFlags_NoUndocking) != 0)) {
        dockspace_flags ^= DockNodeFlags_NoUndocking;
      }
      if (Gui::MenuItem("Flag: NoResize", "",
                        (dockspace_flags & DockNodeFlags_NoResize) != 0)) {
        dockspace_flags ^= DockNodeFlags_NoResize;
      }
      if (Gui::MenuItem("Flag: AutoHideTabBar", "",
                        (dockspace_flags & DockNodeFlags_AutoHideTabBar) !=
                            0)) {
        dockspace_flags ^= DockNodeFlags_AutoHideTabBar;
      }
      if (Gui::MenuItem("Flag: PassthruCentralNode", "",
                        (dockspace_flags & DockNodeFlags_PassthruCentralNode) !=
                            0,
                        opt_fullscreen)) {
        dockspace_flags ^= DockNodeFlags_PassthruCentralNode;
      }
      Gui::Separator();

      if (Gui::MenuItem("Close", NULL, false, p_open != NULL))
        *p_open = false;
      Gui::EndMenu();
    }
    HelpMarker(
        "When docking is enabled, you can ALWAYS dock MOST window into "
        "another! Try it now!"
        "\n"
        "- Drag from window title bar or their tab to dock/undock."
        "\n"
        "- Drag from window menu button (upper-left button) to undock an "
        "entire node (all windows)."
        "\n"
        "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == "
        "false, default)"
        "\n"
        "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)"
        "\n"
        "This demo app has nothing to do with enabling docking!"
        "\n\n"
        "This demo app only demonstrate the use of Gui::DockSpace() which "
        "allows you to manually create a docking node _within_ another window."
        "\n\n"
        "Read comments in ShowExampleAppDockSpace() for more details.");

    Gui::EndMenuBar();
  }

  Gui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
//-----------------------------------------------------------------------------

// Simplified structure to mimic a Document model
struct MyDocument {
  const char *Name; // Document title
  bool Open; // Set when open (we keep an array of all available documents to
             // simplify demo code!)
  bool OpenPrev;  // Copy of Open from last update.
  bool Dirty;     // Set when the document has been modified
  bool WantClose; // Set when the document
  Vec4 Color;     // An arbitrary variable associated to the document

  MyDocument(const char *name, bool open = true,
             const Vec4 &color = Vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
    Name = name;
    Open = OpenPrev = open;
    Dirty = false;
    WantClose = false;
    Color = color;
  }
  void DoOpen() { Open = true; }
  void DoQueueClose() { WantClose = true; }
  void DoForceClose() {
    Open = false;
    Dirty = false;
  }
  void DoSave() { Dirty = false; }

  // Display placeholder contents for the Document
  static void DisplayContents(MyDocument *doc) {
    Gui::PushID(doc);
    Gui::Text("Document \"%s\"", doc->Name);
    Gui::PushStyleColor(Col_Text, doc->Color);
    Gui::TextWrapped(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
        "eiusmod tempor incididunt ut labore et dolore magna aliqua.");
    Gui::PopStyleColor();
    if (Gui::Button("Modify", Vec2(100, 0)))
      doc->Dirty = true;
    Gui::SameLine();
    if (Gui::Button("Save", Vec2(100, 0)))
      doc->DoSave();
    Gui::ColorEdit3("color",
                    &doc->Color.x); // Useful to test drag and drop and
                                    // hold-dragged-to-open-tab behavior.
    Gui::PopID();
  }

  // Display context menu for the Document
  static void DisplayContextMenu(MyDocument *doc) {
    if (!Gui::BeginPopupContextItem())
      return;

    char buf[256];
    sprintf(buf, "Save %s", doc->Name);
    if (Gui::MenuItem(buf, "CTRL+S", false, doc->Open))
      doc->DoSave();
    if (Gui::MenuItem("Close", "CTRL+W", false, doc->Open))
      doc->DoQueueClose();
    Gui::EndPopup();
  }
};

struct ExampleAppDocuments {
  Vector<MyDocument> Documents;

  ExampleAppDocuments() {
    Documents.push_back(
        MyDocument("Lettuce", true, Vec4(0.4f, 0.8f, 0.4f, 1.0f)));
    Documents.push_back(
        MyDocument("Eggplant", true, Vec4(0.8f, 0.5f, 1.0f, 1.0f)));
    Documents.push_back(
        MyDocument("Carrot", true, Vec4(1.0f, 0.8f, 0.5f, 1.0f)));
    Documents.push_back(
        MyDocument("Tomato", false, Vec4(1.0f, 0.3f, 0.4f, 1.0f)));
    Documents.push_back(MyDocument("A Rather Long Title", false));
    Documents.push_back(MyDocument("Some Document", false));
  }
};

// [Optional] Notify the system of Tabs/Windows closure that happened outside
// the regular tab interface. If a tab has been closed programmatically (aka
// closed from another source such as the Checkbox() in the demo, as opposed to
// clicking on the regular tab closing button) and stops being submitted, it
// will take a frame for the tab bar to notice its absence. During this frame
// there will be a gap in the tab bar, and if the tab that has disappeared was
// the selected one, the tab bar will report no selected tab during the frame.
// This will effectively give the impression of a flicker for one frame. We call
// SetTabItemClosed() to manually notify the Tab Bar or Docking system of
// removed tabs to avoid this glitch. Note that this completely optional, and
// only affect tab bars with the TabBarFlags_Reorderable flag.
static void NotifyOfDocumentsClosedElsewhere(ExampleAppDocuments &app) {
  for (MyDocument &doc : app.Documents) {
    if (!doc.Open && doc.OpenPrev)
      Gui::SetTabItemClosed(doc.Name);
    doc.OpenPrev = doc.Open;
  }
}

void ShowExampleAppDocuments(bool *p_open) {
  static ExampleAppDocuments app;

  // Options
  enum Target {
    Target_None,
    Target_Tab, // Create documents as local tab into a local tab bar
    Target_DockSpaceAndWindow // Create documents as regular windows, and create
                              // an embedded dockspace
  };
  static Target opt_target = Target_Tab;
  static bool opt_reorderable = true;
  static TabBarFlags opt_fitting_flags = TabBarFlags_FittingPolicyDefault_;

  // When (opt_target == Target_DockSpaceAndWindow) there is the possibily that
  // one of our child Document window (e.g. "Eggplant") that we emit gets docked
  // into the same spot as the parent window ("Example: Documents"). This would
  // create a problematic feedback loop because selecting the "Eggplant" tab
  // would make the "Example: Documents" tab not visible, which in turn would
  // stop submitting the "Eggplant" window. We avoid this problem by submitting
  // our documents window even if our parent window is not currently visible.
  // Another solution may be to make the "Example: Documents" window use the
  // WindowFlags_NoDocking.

  bool window_contents_visible =
      Gui::Begin("Example: Documents", p_open, WindowFlags_MenuBar);
  if (!window_contents_visible && opt_target != Target_DockSpaceAndWindow) {
    Gui::End();
    return;
  }

  // Menu
  if (Gui::BeginMenuBar()) {
    if (Gui::BeginMenu("File")) {
      int open_count = 0;
      for (MyDocument &doc : app.Documents)
        open_count += doc.Open ? 1 : 0;

      if (Gui::BeginMenu("Open", open_count < app.Documents.Size)) {
        for (MyDocument &doc : app.Documents)
          if (!doc.Open && Gui::MenuItem(doc.Name))
            doc.DoOpen();
        Gui::EndMenu();
      }
      if (Gui::MenuItem("Close All Documents", NULL, false, open_count > 0))
        for (MyDocument &doc : app.Documents)
          doc.DoQueueClose();
      if (Gui::MenuItem("Exit", "Ctrl+F4") && p_open)
        *p_open = false;
      Gui::EndMenu();
    }
    Gui::EndMenuBar();
  }

  // [Debug] List documents with one checkbox for each
  for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++) {
    MyDocument &doc = app.Documents[doc_n];
    if (doc_n > 0)
      Gui::SameLine();
    Gui::PushID(&doc);
    if (Gui::Checkbox(doc.Name, &doc.Open))
      if (!doc.Open)
        doc.DoForceClose();
    Gui::PopID();
  }
  Gui::PushItemWidth(Gui::GetFontSize() * 12);
  Gui::Combo("Output", (int *)&opt_target,
             "None\0TabBar+Tabs\0DockSpace+Window\0");
  Gui::PopItemWidth();
  bool redock_all = false;
  if (opt_target == Target_Tab) {
    Gui::SameLine();
    Gui::Checkbox("Reorderable Tabs", &opt_reorderable);
  }
  if (opt_target == Target_DockSpaceAndWindow) {
    Gui::SameLine();
    redock_all = Gui::Button("Redock all");
  }

  Gui::Separator();

  // About the WindowFlags_UnsavedDocument /
  // TabItemFlags_UnsavedDocument flags. They have multiple effects:
  // - Display a dot next to the title.
  // - Tab is selected when clicking the X close button.
  // - Closure is not assumed (will wait for user to stop submitting the tab).
  //   Otherwise closure is assumed when pressing the X, so if you keep
  //   submitting the tab may reappear at end of tab bar. We need to assume
  //   closure by default otherwise waiting for "lack of submission" on the next
  //   frame would leave an empty hole for one-frame, both in the tab-bar and in
  //   tab-contents when closing a tab/window. The rarely used
  //   SetTabItemClosed() function is a way to notify of programmatic closure to
  //   avoid the one-frame hole.

  // Tabs
  if (opt_target == Target_Tab) {
    TabBarFlags tab_bar_flags =
        (opt_fitting_flags) | (opt_reorderable ? TabBarFlags_Reorderable : 0);
    if (Gui::BeginTabBar("##tabs", tab_bar_flags)) {
      if (opt_reorderable)
        NotifyOfDocumentsClosedElsewhere(app);

      // [DEBUG] Stress tests
      // if ((Gui::GetFrameCount() % 30) == 0) docs[1].Open ^= 1; // [DEBUG]
      // Automatically show/hide a tab. Test various interactions e.g. dragging
      // with this on. if (Gui::GetIO().KeyCtrl)
      // Gui::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test
      // SetTabItemSelected(), probably not very useful as-is anyway..

      // Submit Tabs
      for (MyDocument &doc : app.Documents) {
        if (!doc.Open)
          continue;

        TabItemFlags tab_flags = (doc.Dirty ? TabItemFlags_UnsavedDocument : 0);
        bool visible = Gui::BeginTabItem(doc.Name, &doc.Open, tab_flags);

        // Cancel attempt to close when unsaved add to save queue so we can
        // display a popup.
        if (!doc.Open && doc.Dirty) {
          doc.Open = true;
          doc.DoQueueClose();
        }

        MyDocument::DisplayContextMenu(&doc);
        if (visible) {
          MyDocument::DisplayContents(&doc);
          Gui::EndTabItem();
        }
      }

      Gui::EndTabBar();
    }
  } else if (opt_target == Target_DockSpaceAndWindow) {
    if (Gui::GetIO().ConfigFlags & ConfigFlags_DockingEnable) {
      NotifyOfDocumentsClosedElsewhere(app);

      // Create a DockSpace node where any window can be docked
      ID dockspace_id = Gui::GetID("MyDockSpace");
      Gui::DockSpace(dockspace_id);

      // Create Windows
      for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++) {
        MyDocument *doc = &app.Documents[doc_n];
        if (!doc->Open)
          continue;

        Gui::SetNextWindowDockID(dockspace_id,
                                 redock_all ? Cond_Always : Cond_FirstUseEver);
        WindowFlags window_flags =
            (doc->Dirty ? WindowFlags_UnsavedDocument : 0);
        bool visible = Gui::Begin(doc->Name, &doc->Open, window_flags);

        // Cancel attempt to close when unsaved add to save queue so we can
        // display a popup.
        if (!doc->Open && doc->Dirty) {
          doc->Open = true;
          doc->DoQueueClose();
        }

        MyDocument::DisplayContextMenu(doc);
        if (visible)
          MyDocument::DisplayContents(doc);

        Gui::End();
      }
    } else {
      ShowDockingDisabledMessage();
    }
  }

  // Early out other contents
  if (!window_contents_visible) {
    Gui::End();
    return;
  }

  // Update closing queue
  static Vector<MyDocument *> close_queue;
  if (close_queue.empty()) {
    // Close queue is locked once we started a popup
    for (MyDocument &doc : app.Documents)
      if (doc.WantClose) {
        doc.WantClose = false;
        close_queue.push_back(&doc);
      }
  }

  // Display closing confirmation UI
  if (!close_queue.empty()) {
    int close_queue_unsaved_documents = 0;
    for (int n = 0; n < close_queue.Size; n++)
      if (close_queue[n]->Dirty)
        close_queue_unsaved_documents++;

    if (close_queue_unsaved_documents == 0) {
      // Close documents when all are unsaved
      for (int n = 0; n < close_queue.Size; n++)
        close_queue[n]->DoForceClose();
      close_queue.clear();
    } else {
      if (!Gui::IsPopupOpen("Save?"))
        Gui::OpenPopup("Save?");
      if (Gui::BeginPopupModal("Save?", NULL, WindowFlags_AlwaysAutoResize)) {
        Gui::Text("Save change to the following items?");
        float item_height = Gui::GetTextLineHeightWithSpacing();
        if (Gui::BeginChild(Gui::GetID("frame"),
                            Vec2(-FLT_MIN, 6.25f * item_height),
                            ChildFlags_FrameStyle)) {
          for (int n = 0; n < close_queue.Size; n++)
            if (close_queue[n]->Dirty)
              Gui::Text("%s", close_queue[n]->Name);
        }
        Gui::EndChild();

        Vec2 button_size(Gui::GetFontSize() * 7.0f, 0.0f);
        if (Gui::Button("Yes", button_size)) {
          for (int n = 0; n < close_queue.Size; n++) {
            if (close_queue[n]->Dirty)
              close_queue[n]->DoSave();
            close_queue[n]->DoForceClose();
          }
          close_queue.clear();
          Gui::CloseCurrentPopup();
        }
        Gui::SameLine();
        if (Gui::Button("No", button_size)) {
          for (int n = 0; n < close_queue.Size; n++)
            close_queue[n]->DoForceClose();
          close_queue.clear();
          Gui::CloseCurrentPopup();
        }
        Gui::SameLine();
        if (Gui::Button("Cancel", button_size)) {
          close_queue.clear();
          Gui::CloseCurrentPopup();
        }
        Gui::EndPopup();
      }
    }
  }

  Gui::End();
}

// End of Demo code
#else

    void Gui::ShowAboutWindow(bool *) {
}
void Gui::ShowDemoWindow(bool *) {}
void Gui::ShowUserGuide() {}
void Gui::ShowStyleEditor(Style *) {}

#endif

#endif // #ifndef DISABLE
