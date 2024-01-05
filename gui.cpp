//  [SECTION] INCLUDES
//  [SECTION] FORWARD DECLARATIONS
//  [SECTION] CONTEXT AND MEMORY ALLOCATORS
//  [SECTION] USER FACING STRUCTURES (Style, IO)
//  [SECTION] MISC HELPERS/UTILITIES (Geometry functions)
//  [SECTION] MISC HELPERS/UTILITIES (String, Format, Hash functions)
//  [SECTION] MISC HELPERS/UTILITIES (File functions)
//  [SECTION] MISC HELPERS/UTILITIES (Text* functions)
//  [SECTION] MISC HELPERS/UTILITIES (Color functions)
//  [SECTION] Storage
//  [SECTION] TextFilter
//  [SECTION] TextBuffer, TextIndex
//  [SECTION] ListClipper
//  [SECTION] STYLING
//  [SECTION] RENDER HELPERS
//  [SECTION] INITIALIZATION, SHUTDOWN
//  [SECTION] MAIN CODE (most of the code! lots of stuff, needs tidying up!)
//  [SECTION] INPUTS
//  [SECTION] ERROR CHECKING
//  [SECTION] LAYOUT
//  [SECTION] SCROLLING
//  [SECTION] TOOLTIPS
//  [SECTION] POPUPS
//  [SECTION] KEYBOARD/GAMEPAD NAVIGATION
//  [SECTION] DRAG AND DROP
//  [SECTION] LOGGING/CAPTURING
//  [SECTION] SETTINGS
//  [SECTION] LOCALIZATION
//  [SECTION] VIEWPORTS, PLATFORM WINDOWS
//  [SECTION] DOCKING
//  [SECTION] PLATFORM DEPENDENT HELPERS
//  [SECTION] METRICS/DEBUGGER WINDOW
//  [SECTION] DEBUG LOG WINDOW
//  [SECTION] OTHER DEBUG TOOLS (ITEM PICKER, ID STACK TOOL)
//-------------------------------------------------------------------------
//  [SECTION] INCLUDES
//-------------------------------------------------------------------------

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef DEFINE_MATH_OPERATORS
#define DEFINE_MATH_OPERATORS
#endif

#include "gui.hpp"
#ifndef DISABLE
#include "internal.hpp"

// System includes
#include <stdint.h> // intptr_t
#include <stdio.h>  // vsnprintf, sscanf, printf

// [Windows] On non-Visual Studio compilers, we default to
// DISABLE_WIN32_DEFAULT_IME_FUNCTIONS unless explicitly enabled
#if defined(_WIN32) && !defined(_MSC_VER) &&                                   \
    !defined(ENABLE_WIN32_DEFAULT_IME_FUNCTIONS) &&                            \
    !defined(DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)
#define DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif

// [Windows] OS specific includes (optional)
#if defined(_WIN32) && defined(DISABLE_DEFAULT_FILE_FUNCTIONS) &&              \
    defined(DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS) &&                      \
    defined(DISABLE_WIN32_DEFAULT_IME_FUNCTIONS) &&                            \
    !defined(DISABLE_WIN32_FUNCTIONS)
#define DISABLE_WIN32_FUNCTIONS
#endif
#if defined(_WIN32) && !defined(DISABLE_WIN32_FUNCTIONS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef __MINGW32__
#include <Windows.h> // _wfopen, OpenClipboard
#else
#include <windows.h>
#endif
#if defined(WINAPI_FAMILY) &&                                                  \
    (WINAPI_FAMILY ==                                                          \
     WINAPI_FAMILY_APP) // UWP doesn't have all Win32 functions
#define DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#define DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif
#endif

// [Apple] OS specific includes
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning(disable : 4127) // condition expression is constant
#pragma warning(                                                               \
    disable : 4996) // 'This function or variable may be unsafe': strcpy,
                    // strdup, sprintf, vsnprintf, sscanf, fopen
#if defined(_MSC_VER) && _MSC_VER >= 1922 // MSVC 2019 16.2 or later
#pragma warning(disable : 5054) // operator '|': deprecated between enumerations
                                // of different types
#endif
#pragma warning(                                                               \
    disable : 26451) // [Static Analyzer] Arithmetic overflow : Using operator
                     // 'xxx' on a 4 byte value and then casting the result to
                     // an 8 byte value. Cast the value to the wider type before
                     // calling operator 'xxx' to avoid overflow(io.2).
#pragma warning(                                                               \
    disable : 26495) // [Static Analyzer] Variable 'XXX' is uninitialized.
                     // Always initialize a member variable (type.6).
#pragma warning(                                                               \
    disable : 26812) // [Static Analyzer] The enum type 'xxx' is unscoped.
                     // Prefer 'enum class' over 'enum' (Enum.3).
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
    "-Wfloat-equal" // warning: comparing floating point with == or != is unsafe
                    // // storing and comparing against same constants
                    // (typically 0.0f) is ok.
#pragma clang diagnostic ignored                                               \
    "-Wformat-nonliteral" // warning: format string is not a string literal //
                          // passing non-literal to vsnformat(). yes, user
                          // passing incorrect format strings can crash the
                          // code.
#pragma clang diagnostic ignored                                               \
    "-Wexit-time-destructors" // warning: declaration requires an exit-time
                              // destructor     // exit-time destruction order
                              // is undefined. if MemFree() leads to users code
                              // that has been disabled before exit it might
                              // cause problems. Gui coding style welcomes
                              // static/globals.
#pragma clang diagnostic ignored                                               \
    "-Wglobal-constructors" // warning: declaration requires a global destructor
                            // // similar to above, not sure what the exact
                            // difference is.
#pragma clang diagnostic ignored                                               \
    "-Wsign-conversion" // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored                                               \
    "-Wformat-pedantic" // warning: format specifies type 'void *' but the
                        // argument has type 'xxxx *' // unreasonable, would
                        // lead to casting every %p arg to void*. probably
                        // enabled by -pedantic.
#pragma clang diagnostic ignored                                               \
    "-Wint-to-void-pointer-cast" // warning: cast to 'void *' from smaller
                                 // integer type 'int'
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
    "-Wimplicit-int-float-conversion" // warning: implicit conversion from 'xxx'
                                      // to 'float' may lose precision
#elif defined(__GNUC__)
// We disable -Wpragmas because GCC doesn't provide a has_warning equivalent and
// some forks/patches may not follow the warning/version association.
#pragma GCC diagnostic ignored                                                 \
    "-Wpragmas" // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored                                                 \
    "-Wunused-function" // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored                                                 \
    "-Wint-to-pointer-cast" // warning: cast to pointer from integer of
                            // different size
#pragma GCC diagnostic ignored                                                 \
    "-Wformat" // warning: format '%p' expects argument of type 'void*', but
               // argument 6 has type 'Window*'
#pragma GCC diagnostic ignored                                                 \
    "-Wdouble-promotion" // warning: implicit conversion from 'float' to
                         // 'double' when passing argument to function
#pragma GCC diagnostic ignored                                                 \
    "-Wconversion" // warning: conversion to 'xxxx' from 'xxxx' may alter its
                   // value
#pragma GCC diagnostic ignored                                                 \
    "-Wformat-nonliteral" // warning: format not a string literal, format string
                          // not checked
#pragma GCC diagnostic ignored                                                 \
    "-Wstrict-overflow" // warning: assuming signed overflow does not occur when
                        // assuming that (X - c) > X is always false
#pragma GCC diagnostic ignored                                                 \
    "-Wclass-memaccess" // [__GNUC__ >= 8] warning: 'memset/memcpy'
                        // clearing/writing an object of type 'xxxx' with no
                        // trivial copy-assignment; use assignment or
                        // value-initialization instead
#endif

// Debug options
#define DEBUG_NAV_SCORING                                                      \
  0 // Display navigation scoring preview when hovering items. Display last
    // moving direction matches when holding CTRL
#define DEBUG_NAV_RECTS                                                        \
  0 // Display the reference navigation rectangle for each window

// When using CTRL+TAB (or Gamepad Square+L/R) we delay the visual a little in
// order to reduce visual noise doing a fast switch.
static const float NAV_WINDOWING_HIGHLIGHT_DELAY =
    0.20f; // Time before the highlight and screen dimming starts fading in
static const float NAV_WINDOWING_LIST_APPEAR_DELAY =
    0.15f; // Time before the window list starts to appear

// Window resizing from edges (when io.ConfigWindowsResizeFromEdges = true and
// BackendFlags_HasMouseCursors is set in io.BackendFlags by backend)
static const float WINDOWS_HOVER_PADDING =
    4.0f; // Extend outside window for hovering/resizing (maxxed with
          // TouchPadding) and inside windows for borders. Affect
          // FindHoveredWindow().
static const float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER =
    0.04f; // Reduce visual noise by only highlighting the border after a
           // certain time.
static const float WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER =
    0.70f; // Lock scrolled window (so it doesn't pick child windows that are
           // scrolling through) for a certain time, unless mouse moved.

// Tooltip offset
static const Vec2 TOOLTIP_DEFAULT_OFFSET =
    Vec2(16, 10); // Multiplied by g.Style.MouseCursorScale

// Docking
static const float DOCKING_TRANSPARENT_PAYLOAD_ALPHA =
    0.50f; // For use with io.ConfigDockingTransparentPayload. Apply to Viewport
           // _or_ WindowBg in host viewport.

//-------------------------------------------------------------------------
// [SECTION] FORWARD DECLARATIONS
//-------------------------------------------------------------------------

static void SetCurrentWindow(Window *window);
static void FindHoveredWindow();
static Window *CreateNewWindow(const char *name, WindowFlags flags);
static Vec2 CalcNextScrollFromScrollTargetAndClamp(Window *window);

static void AddWindowToSortBuffer(Vector<Window *> *out_sorted_windows,
                                  Window *window);

// Settings
static void WindowSettingsHandler_ClearAll(Context *, SettingsHandler *);
static void *WindowSettingsHandler_ReadOpen(Context *, SettingsHandler *,
                                            const char *name);
static void WindowSettingsHandler_ReadLine(Context *, SettingsHandler *,
                                           void *entry, const char *line);
static void WindowSettingsHandler_ApplyAll(Context *, SettingsHandler *);
static void WindowSettingsHandler_WriteAll(Context *, SettingsHandler *,
                                           TextBuffer *buf);

// Platform Dependents default implementation for IO functions
static const char *GetClipboardTextFn_DefaultImpl(void *user_data_ctx);
static void SetClipboardTextFn_DefaultImpl(void *user_data_ctx,
                                           const char *text);
static void SetPlatformImeDataFn_DefaultImpl(Viewport *viewport,
                                             PlatformImeData *data);

namespace Gui {
// Navigation
static void NavUpdate();
static void NavUpdateWindowing();
static void NavUpdateWindowingOverlay();
static void NavUpdateCancelRequest();
static void NavUpdateCreateMoveRequest();
static void NavUpdateCreateTabbingRequest();
static float NavUpdatePageUpPageDown();
static inline void NavUpdateAnyRequestFlag();
static void NavUpdateCreateWrappingRequest();
static void NavEndFrame();
static bool NavScoreItem(NavItemData *result);
static void NavApplyItemToResult(NavItemData *result);
static void NavProcessItem();
static void NavProcessItemForTabbingRequest(ID id, ItemFlags item_flags,
                                            NavMoveFlags move_flags);
static Vec2 NavCalcPreferredRefPos();
static void NavSaveLastChildNavWindowIntoParent(Window *nav_window);
static Window *NavRestoreLastChildNavWindow(Window *window);
static void NavRestoreLayer(NavLayer layer);
static int FindWindowFocusIndex(Window *window);

// Error Checking and Debug Tools
static void ErrorCheckNewFrameSanityChecks();
static void ErrorCheckEndFrameSanityChecks();
static void UpdateDebugToolItemPicker();
static void UpdateDebugToolStackQueries();
static void UpdateDebugToolFlashStyleColor();

// Inputs
static void UpdateKeyboardInputs();
static void UpdateMouseInputs();
static void UpdateMouseWheel();
static void UpdateKeyRoutingTable(KeyRoutingTable *rt);

// Misc
static void UpdateSettings();
static int UpdateWindowManualResize(Window *window, const Vec2 &size_auto_fit,
                                    int *border_hovered, int *border_held,
                                    int resize_grip_count,
                                    U32 resize_grip_col[4],
                                    const Rect &visibility_rect);
static void RenderWindowOuterBorders(Window *window);
static void RenderWindowDecorations(Window *window, const Rect &title_bar_rect,
                                    bool title_bar_is_highlight,
                                    bool handle_borders_and_resize_grips,
                                    int resize_grip_count,
                                    const U32 resize_grip_col[4],
                                    float resize_grip_draw_size);
static void RenderWindowTitleBarContents(Window *window,
                                         const Rect &title_bar_rect,
                                         const char *name, bool *p_open);
static void RenderDimmedBackgroundBehindWindow(Window *window, U32 col);
static void RenderDimmedBackgrounds();

// Viewports
const ID VIEWPORT_DEFAULT_ID =
    0x11111111; // Using an arbitrary constant instead of e.g.
                // HashStr("ViewportDefault", 0); so it's easier to spot in
                // the debugger. The exact value doesn't matter.
static ViewportP *AddUpdateViewport(Window *window, ID id,
                                    const Vec2 &platform_pos, const Vec2 &size,
                                    ViewportFlags flags);
static void DestroyViewport(ViewportP *viewport);
static void UpdateViewportsNewFrame();
static void UpdateViewportsEndFrame();
static void WindowSelectViewport(Window *window);
static void WindowSyncOwnedViewport(Window *window,
                                    Window *parent_window_in_stack);
static bool UpdateTryMergeWindowIntoHostViewport(Window *window,
                                                 ViewportP *host_viewport);
static bool UpdateTryMergeWindowIntoHostViewports(Window *window);
static bool GetWindowAlwaysWantOwnViewport(Window *window);
static int FindPlatformMonitorForPos(const Vec2 &pos);
static int FindPlatformMonitorForRect(const Rect &r);
static void UpdateViewportPlatformMonitor(ViewportP *viewport);

} // namespace Gui

//-----------------------------------------------------------------------------
// [SECTION] CONTEXT AND MEMORY ALLOCATORS
//-----------------------------------------------------------------------------

// DLL users:
// - Heaps and globals are not shared across DLL boundaries!
// - You will need to call SetCurrentContext() + SetAllocatorFunctions() for
// each static/DLL boundary you are calling from.
// - Same applies for hot-reloading mechanisms that are reliant on reloading DLL
// (note that many hot-reloading mechanisms work without DLL).
// - Using Gui via a shared library is not recommended, because of
// function call overhead and because we don't guarantee backward nor forward
// ABI compatibility.
// - Confused? In a debugger: add GGui to your watch window and notice how its
// value changes depending on your current location (which DLL boundary you are
// in).

// Current context pointer. Implicitly used by all Gui functions. Always
// assumed to be != NULL.
// - Gui::CreateContext() will automatically set this pointer if it is NULL.
//   Change to a different context by calling Gui::SetCurrentContext().
// - Important: Gui functions are not thread-safe because of this
// pointer.
//   If you want thread-safety to allow N threads to access N different
//   contexts:
//   - Change this variable to use thread local storage so each thread can refer
//   to a different context, in your config.hpp:
//         struct Context;
//         extern thread_local Context* MyTLS;
//         #define GGui MyTLS
//     And then define MyTLS in one of your cpp files. Note that
//     thread_local is a C++11 keyword, earlier C++ uses compiler-specific
//     keyword.
//   - Future development aims to make this context pointer explicit to all
//   calls.
//   - If you need a finite number of contexts, you may compile and use multiple
//   instances of the Gui code from a different namespace.
// - DLL users: read comments above.
#ifndef GGui
Context *GGui = NULL;
#endif

// Memory Allocator functions. Use SetAllocatorFunctions() to change them.
// - You probably don't want to modify that mid-program, and if you use
// global/static e.g. Vector<> instances you may need to keep them accessible
// during program destruction.
// - DLL users: read comments above.
#ifndef DISABLE_DEFAULT_ALLOCATORS
static void *MallocWrapper(size_t size, void *user_data) {
  UNUSED(user_data);
  return malloc(size);
}
static void FreeWrapper(void *ptr, void *user_data) {
  UNUSED(user_data);
  free(ptr);
}
#else
static void *MallocWrapper(size_t size, void *user_data) {
  UNUSED(user_data);
  UNUSED(size);
  ASSERT(0);
  return NULL;
}
static void FreeWrapper(void *ptr, void *user_data) {
  UNUSED(user_data);
  UNUSED(ptr);
  ASSERT(0);
}
#endif
static MemAllocFunc GAllocatorAllocFunc = MallocWrapper;
static MemFreeFunc GAllocatorFreeFunc = FreeWrapper;
static void *GAllocatorUserData = NULL;

//-----------------------------------------------------------------------------
// [SECTION] USER FACING STRUCTURES (Style, IO)
//-----------------------------------------------------------------------------

Style::Style() {
  Alpha = 1.0f; // Global alpha applies to everything in Gui.
  DisabledAlpha =
      0.60f; // Additional alpha multiplier applied by BeginDisabled(). Multiply
             // over current value of Alpha.
  WindowPadding = Vec2(8, 8); // Padding within a window
  WindowRounding = 0.0f; // Radius of window corners rounding. Set to 0.0f to
                         // have rectangular windows. Large values tend to lead
                         // to variety of artifacts and are not recommended.
  WindowBorderSize = 1.0f; // Thickness of border around windows. Generally set
                           // to 0.0f or 1.0f. Other values not well tested.
  WindowMinSize = Vec2(32, 32);        // Minimum window size
  WindowTitleAlign = Vec2(0.0f, 0.5f); // Alignment for title bar text
  WindowMenuButtonPosition =
      Dir_Left; // Position of the collapsing/docking button in the title
                // bar (left/right). Defaults to Dir_Left.
  ChildRounding = 0.0f; // Radius of child window corners rounding. Set to 0.0f
                        // to have rectangular child windows
  ChildBorderSize = 1.0f; // Thickness of border around child windows. Generally
                          // set to 0.0f or 1.0f. Other values not well tested.
  PopupRounding = 0.0f; // Radius of popup window corners rounding. Set to 0.0f
                        // to have rectangular child windows
  PopupBorderSize =
      1.0f; // Thickness of border around popup or tooltip windows. Generally
            // set to 0.0f or 1.0f. Other values not well tested.
  FramePadding =
      Vec2(4, 3); // Padding within a framed rectangle (used by most widgets)
  FrameRounding = 0.0f; // Radius of frame corners rounding. Set to 0.0f to have
                        // rectangular frames (used by most widgets).
  FrameBorderSize = 0.0f; // Thickness of border around frames. Generally set to
                          // 0.0f or 1.0f. Other values not well tested.
  ItemSpacing =
      Vec2(8, 4); // Horizontal and vertical spacing between widgets/lines
  ItemInnerSpacing =
      Vec2(4, 4); // Horizontal and vertical spacing between within elements
                  // of a composed widget (e.g. a slider and its label)
  CellPadding = Vec2(4, 2); // Padding within a table cell. CellPadding.y may
                            // be altered between different rows.
  TouchExtraPadding =
      Vec2(0, 0); // Expand reactive bounding box for touch-based system where
                  // touch position is not accurate enough. Unfortunately we
                  // don't sort widgets so priority on overlap will always be
                  // given to the first widget. So don't grow this too much!
  IndentSpacing = 21.0f; // Horizontal spacing when e.g. entering a tree node.
                         // Generally == (FontSize + FramePadding.x*2).
  ColumnsMinSpacing = 6.0f; // Minimum horizontal spacing between two columns.
                            // Preferably > (FramePadding.x + 1).
  ScrollbarSize = 14.0f;    // Width of the vertical scrollbar, Height of the
                            // horizontal scrollbar
  ScrollbarRounding = 9.0f; // Radius of grab corners rounding for scrollbar
  GrabMinSize =
      12.0f; // Minimum width/height of a grab box for slider/scrollbar
  GrabRounding = 0.0f; // Radius of grabs corners rounding. Set to 0.0f to have
                       // rectangular slider grabs.
  LogSliderDeadzone = 4.0f; // The size in pixels of the dead-zone around zero
                            // on logarithmic sliders that cross zero.
  TabRounding = 4.0f;   // Radius of upper corners of a tab. Set to 0.0f to have
                        // rectangular tabs.
  TabBorderSize = 0.0f; // Thickness of border around tabs.
  TabMinWidthForCloseButton =
      0.0f; // Minimum width for close button to appear on an unselected tab
            // when hovered. Set to 0.0f to always show when hovering, set to
            // FLT_MAX to never show close button unless selected.
  TabBarBorderSize = 1.0f; // Thickness of tab-bar separator, which takes on the
                           // tab active color to denote focus.
  TableAngledHeadersAngle =
      35.0f * (PI / 180.0f); // Angle of angled headers (supported values
                             // range from -50 degrees to +50 degrees).
  ColorButtonPosition =
      Dir_Right; // Side of the color button in the ColorEdit4 widget
                 // (left/right). Defaults to Dir_Right.
  ButtonTextAlign = Vec2(
      0.5f, 0.5f); // Alignment of button text when button is larger than text.
  SelectableTextAlign = Vec2(
      0.0f,
      0.0f); // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left
             // aligned). It's generally important to keep this left-aligned if
             // you want to lay multiple items on a same line.
  SeparatorTextBorderSize = 3.0f; // Thickkness of border in SeparatorText()
  SeparatorTextAlign =
      Vec2(0.0f, 0.5f); // Alignment of text within the separator. Defaults to
                        // (0.0f, 0.5f) (left aligned, center).
  SeparatorTextPadding =
      Vec2(20.0f, 3.f); // Horizontal offset of text from each edge of the
                        // separator + spacing on other axis. Generally small
                        // values. .y is recommended to be == FramePadding.y.
  DisplayWindowPadding =
      Vec2(19, 19); // Window position are clamped to be visible within the
                    // display area or monitors by at least this amount. Only
                    // applies to regular windows.
  DisplaySafeAreaPadding =
      Vec2(3, 3); // If you cannot see the edge of your screen (e.g. on a TV)
                  // increase the safe area padding. Covers popups/tooltips as
                  // well regular windows.
  DockingSeparatorSize =
      2.0f; // Thickness of resizing border between docked windows
  MouseCursorScale =
      1.0f; // Scale software rendered mouse cursor (when io.MouseDrawCursor is
            // enabled). May be removed later.
  AntiAliasedLines = true; // Enable anti-aliased lines/borders. Disable if you
                           // are really tight on CPU/GPU.
  AntiAliasedLinesUseTex =
      true; // Enable anti-aliased lines/borders using textures where possible.
            // Require backend to render with bilinear filtering (NOT
            // point/nearest filtering).
  AntiAliasedFill = true; // Enable anti-aliased filled shapes (rounded
                          // rectangles, circles, etc.).
  CurveTessellationTol =
      1.25f; // Tessellation tolerance when using PathBezierCurveTo() without a
             // specific number of segments. Decrease for highly tessellated
             // curves (higher quality, more polygons), increase to reduce
             // quality.
  CircleTessellationMaxError =
      0.30f; // Maximum error (in pixels) allowed when using
             // AddCircle()/AddCircleFilled() or drawing rounded corner
             // rectangles with no explicit segment count specified. Decrease
             // for higher quality but more geometry.

  // Behaviors
  HoverStationaryDelay =
      0.15f; // Delay for IsItemHovered(HoveredFlags_Stationary). Time
             // required to consider mouse stationary.
  HoverDelayShort = 0.15f; // Delay for IsItemHovered(HoveredFlags_DelayShort).
                           // Usually used along with HoverStationaryDelay.
  HoverDelayNormal =
      0.40f; // Delay for IsItemHovered(HoveredFlags_DelayNormal). "
  HoverFlagsForTooltipMouse =
      HoveredFlags_Stationary | HoveredFlags_DelayShort |
      HoveredFlags_AllowWhenDisabled; // Default flags when using
                                      // IsItemHovered(HoveredFlags_ForTooltip)
                                      // or
                                      // BeginItemTooltip()/SetItemTooltip()
                                      // while using mouse.
  HoverFlagsForTooltipNav =
      HoveredFlags_NoSharedDelay | HoveredFlags_DelayNormal |
      HoveredFlags_AllowWhenDisabled; // Default flags when using
                                      // IsItemHovered(HoveredFlags_ForTooltip)
                                      // or
                                      // BeginItemTooltip()/SetItemTooltip()
                                      // while using keyboard/gamepad.

  // Default theme
  Gui::StyleColorsDark(this);
}

// To scale your entire UI (e.g. if you want your app to use High DPI or
// generally be DPI aware) you may use this helper function. Scaling the fonts
// is done separately and is up to you. Important: This operation is lossy
// because we round all sizes to integer. If you need to change your scale
// multiples, call this over a freshly initialized Style structure rather
// than scaling multiple times.
void Style::ScaleAllSizes(float scale_factor) {
  WindowPadding = Trunc(WindowPadding * scale_factor);
  WindowRounding = Trunc(WindowRounding * scale_factor);
  WindowMinSize = Trunc(WindowMinSize * scale_factor);
  ChildRounding = Trunc(ChildRounding * scale_factor);
  PopupRounding = Trunc(PopupRounding * scale_factor);
  FramePadding = Trunc(FramePadding * scale_factor);
  FrameRounding = Trunc(FrameRounding * scale_factor);
  ItemSpacing = Trunc(ItemSpacing * scale_factor);
  ItemInnerSpacing = Trunc(ItemInnerSpacing * scale_factor);
  CellPadding = Trunc(CellPadding * scale_factor);
  TouchExtraPadding = Trunc(TouchExtraPadding * scale_factor);
  IndentSpacing = Trunc(IndentSpacing * scale_factor);
  ColumnsMinSpacing = Trunc(ColumnsMinSpacing * scale_factor);
  ScrollbarSize = Trunc(ScrollbarSize * scale_factor);
  ScrollbarRounding = Trunc(ScrollbarRounding * scale_factor);
  GrabMinSize = Trunc(GrabMinSize * scale_factor);
  GrabRounding = Trunc(GrabRounding * scale_factor);
  LogSliderDeadzone = Trunc(LogSliderDeadzone * scale_factor);
  TabRounding = Trunc(TabRounding * scale_factor);
  TabMinWidthForCloseButton =
      (TabMinWidthForCloseButton != FLT_MAX)
          ? Trunc(TabMinWidthForCloseButton * scale_factor)
          : FLT_MAX;
  SeparatorTextPadding = Trunc(SeparatorTextPadding * scale_factor);
  DockingSeparatorSize = Trunc(DockingSeparatorSize * scale_factor);
  DisplayWindowPadding = Trunc(DisplayWindowPadding * scale_factor);
  DisplaySafeAreaPadding = Trunc(DisplaySafeAreaPadding * scale_factor);
  MouseCursorScale = Trunc(MouseCursorScale * scale_factor);
}

IO::IO() {
  // Most fields are initialized with zero
  memset(this, 0, sizeof(*this));
  STATIC_ASSERT(ARRAYSIZE(IO::MouseDown) == MouseButton_COUNT &&
                ARRAYSIZE(IO::MouseClicked) == MouseButton_COUNT);

  // Settings
  ConfigFlags = ConfigFlags_None;
  BackendFlags = BackendFlags_None;
  DisplaySize = Vec2(-1.0f, -1.0f);
  DeltaTime = 1.0f / 60.0f;
  IniSavingRate = 5.0f;
  IniFilename = "gui.ini"; // Important: "gui.ini" is relative to current
                           // working dir, most apps will want to lock this to
                           // an absolute path (e.g. same path as executables).
  LogFilename = "log.txt";
#ifndef DISABLE_OBSOLETE_KEYIO
  for (int i = 0; i < Key_COUNT; i++)
    KeyMap[i] = -1;
#endif
  UserData = NULL;

  Fonts = NULL;
  FontGlobalScale = 1.0f;
  FontDefault = NULL;
  FontAllowUserScaling = false;
  DisplayFramebufferScale = Vec2(1.0f, 1.0f);

  // Docking options (when ConfigFlags_DockingEnable is set)
  ConfigDockingNoSplit = false;
  ConfigDockingWithShift = false;
  ConfigDockingAlwaysTabBar = false;
  ConfigDockingTransparentPayload = false;

  // Viewport options (when ConfigFlags_ViewportsEnable is set)
  ConfigViewportsNoAutoMerge = false;
  ConfigViewportsNoTaskBarIcon = false;
  ConfigViewportsNoDecoration = true;
  ConfigViewportsNoDefaultParent = false;

  // Miscellaneous options
  MouseDrawCursor = false;
#ifdef __APPLE__
  ConfigMacOSXBehaviors =
      true; // Set Mac OS X style defaults based on __APPLE__ compile time flag
#else
  ConfigMacOSXBehaviors = false;
#endif
  ConfigInputTrickleEventQueue = true;
  ConfigInputTextCursorBlink = true;
  ConfigInputTextEnterKeepActive = false;
  ConfigDragClickToInputText = false;
  ConfigWindowsResizeFromEdges = true;
  ConfigWindowsMoveFromTitleBarOnly = false;
  ConfigMemoryCompactTimer = 60.0f;
  ConfigDebugBeginReturnValueOnce = false;
  ConfigDebugBeginReturnValueLoop = false;

  // Inputs Behaviors
  MouseDoubleClickTime = 0.30f;
  MouseDoubleClickMaxDist = 6.0f;
  MouseDragThreshold = 6.0f;
  KeyRepeatDelay = 0.275f;
  KeyRepeatRate = 0.050f;

  // Platform Functions
  // Note: Initialize() will setup default clipboard/ime handlers.
  BackendPlatformName = BackendRendererName = NULL;
  BackendPlatformUserData = BackendRendererUserData = BackendLanguageUserData =
      NULL;
  PlatformLocaleDecimalPoint = '.';

  // Input (NB: we already have memset zero the entire structure!)
  MousePos = Vec2(-FLT_MAX, -FLT_MAX);
  MousePosPrev = Vec2(-FLT_MAX, -FLT_MAX);
  MouseSource = MouseSource_Mouse;
  for (int i = 0; i < ARRAYSIZE(MouseDownDuration); i++)
    MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
  for (int i = 0; i < ARRAYSIZE(KeysData); i++) {
    KeysData[i].DownDuration = KeysData[i].DownDurationPrev = -1.0f;
  }
  AppAcceptingEvents = true;
  BackendUsingLegacyKeyArrays = (S8)-1;
  BackendUsingLegacyNavInputArray =
      true; // assume using legacy array until proven wrong
}

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the
// WM_CHAR message
// FIXME: Should in theory be called "AddCharacterEvent()" to be consistent with
// new API
void IO::AddInputCharacter(unsigned int c) {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;
  if (c == 0 || !AppAcceptingEvents)
    return;

  InputEvent e;
  e.Type = InputEventType_Text;
  e.Source = InputSource_Keyboard;
  e.EventId = g.InputEventsNextEventId++;
  e.Text.Char = c;
  g.InputEventsQueue.push_back(e);
}

// UTF16 strings use surrogate pairs to encode codepoints >= 0x10000, so
// we should save the high surrogate.
void IO::AddInputCharacterUTF16(Wchar16 c) {
  if ((c == 0 && InputQueueSurrogate == 0) || !AppAcceptingEvents)
    return;

  if ((c & 0xFC00) == 0xD800) // High surrogate, must save
  {
    if (InputQueueSurrogate != 0)
      AddInputCharacter(UNICODE_CODEPOINT_INVALID);
    InputQueueSurrogate = c;
    return;
  }

  Wchar cp = c;
  if (InputQueueSurrogate != 0) {
    if ((c & 0xFC00) != 0xDC00) // Invalid low surrogate
    {
      AddInputCharacter(UNICODE_CODEPOINT_INVALID);
    } else {
#if UNICODE_CODEPOINT_MAX == 0xFFFF
      cp = UNICODE_CODEPOINT_INVALID; // Codepoint will not fit in Wchar
#else
      cp = (Wchar)(((InputQueueSurrogate - 0xD800) << 10) + (c - 0xDC00) +
                   0x10000);
#endif
    }

    InputQueueSurrogate = 0;
  }
  AddInputCharacter((unsigned)cp);
}

void IO::AddInputCharactersUTF8(const char *utf8_chars) {
  if (!AppAcceptingEvents)
    return;
  while (*utf8_chars != 0) {
    unsigned int c = 0;
    utf8_chars += TextCharFromUtf8(&c, utf8_chars, NULL);
    AddInputCharacter(c);
  }
}

// Clear all incoming events.
void IO::ClearEventsQueue() {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;
  g.InputEventsQueue.clear();
}

// Clear current keyboard/mouse/gamepad state + current frame text input buffer.
// Equivalent to releasing all keys/buttons.
void IO::ClearInputKeys() {
#ifndef DISABLE_OBSOLETE_KEYIO
  memset(KeysDown, 0, sizeof(KeysDown));
#endif
  for (int n = 0; n < ARRAYSIZE(KeysData); n++) {
    KeysData[n].Down = false;
    KeysData[n].DownDuration = -1.0f;
    KeysData[n].DownDurationPrev = -1.0f;
  }
  KeyCtrl = KeyShift = KeyAlt = KeySuper = false;
  KeyMods = Mod_None;
  MousePos = Vec2(-FLT_MAX, -FLT_MAX);
  for (int n = 0; n < ARRAYSIZE(MouseDown); n++) {
    MouseDown[n] = false;
    MouseDownDuration[n] = MouseDownDurationPrev[n] = -1.0f;
  }
  MouseWheel = MouseWheelH = 0.0f;
  InputQueueCharacters.resize(0); // Behavior of old ClearInputCharacters().
}

// Removed this as it is ambiguous/misleading and generally incorrect to use
// with the existence of a higher-level input queue. Current frame character
// buffer is now also cleared by ClearInputKeys().
#ifndef DISABLE_OBSOLETE_FUNCTIONS
void IO::ClearInputCharacters() { InputQueueCharacters.resize(0); }
#endif

static InputEvent *FindLatestInputEvent(Context *ctx, InputEventType type,
                                        int arg = -1) {
  Context &g = *ctx;
  for (int n = g.InputEventsQueue.Size - 1; n >= 0; n--) {
    InputEvent *e = &g.InputEventsQueue[n];
    if (e->Type != type)
      continue;
    if (type == InputEventType_Key && e->Key.Key != arg)
      continue;
    if (type == InputEventType_MouseButton && e->MouseButton.Button != arg)
      continue;
    return e;
  }
  return NULL;
}

// Queue a new key down/up event.
// - Key key:       Translated key (as in, generally Key_A matches the
// key end-user would use to emit an 'A' character)
// - bool down:          Is the key down? use false to signify a key release.
// - float analog_value: 0.0f..1.0f
// IMPORTANT: THIS FUNCTION AND OTHER "ADD" GRABS THE CONTEXT FROM OUR INSTANCE.
// WE NEED TO ENSURE THAT ALL FUNCTION CALLS ARE FULLFILLING THIS, WHICH IS WHY
// GetKeyData() HAS AN EXPLICIT CONTEXT.
void IO::AddKeyAnalogEvent(Key key, bool down, float analog_value) {
  // if (e->Down) { DEBUG_LOG_IO("AddKeyEvent() Key='%s' %d, NativeKeycode = %d,
  // NativeScancode = %d\n", Gui::GetKeyName(e->Key), e->Down,
  // e->NativeKeycode, e->NativeScancode); }
  ASSERT(Ctx != NULL);
  if (key == Key_None || !AppAcceptingEvents)
    return;
  Context &g = *Ctx;
  ASSERT(Gui::IsNamedKeyOrModKey(
      key)); // Backend needs to pass a valid Key_ constant. 0..511 values
             // are legacy native key codes which are not accepted by this API.
  ASSERT(Gui::IsAliasKey(key) ==
         false); // Backend cannot submit Key_MouseXXX values they are
                 // automatically inferred from AddMouseXXX() events.
  ASSERT(key != Mod_Shortcut); // We could easily support the translation
                               // here but it seems saner to not accept it
                               // (TestEngine perform a translation itself)

  // Verify that backend isn't mixing up using new io.AddKeyEvent() api and old
  // io.KeysDown[] + io.KeyMap[] data.
#ifndef DISABLE_OBSOLETE_KEYIO
  ASSERT(
      (BackendUsingLegacyKeyArrays == -1 || BackendUsingLegacyKeyArrays == 0) &&
      "Backend needs to either only use io.AddKeyEvent(), either only fill "
      "legacy io.KeysDown[] + io.KeyMap[]. Not both!");
  if (BackendUsingLegacyKeyArrays == -1)
    for (int n = Key_NamedKey_BEGIN; n < Key_NamedKey_END; n++)
      ASSERT(KeyMap[n] == -1 &&
             "Backend needs to either only use io.AddKeyEvent(), either "
             "only fill legacy io.KeysDown[] + io.KeyMap[]. Not both!");
  BackendUsingLegacyKeyArrays = 0;
#endif
  if (Gui::IsGamepadKey(key))
    BackendUsingLegacyNavInputArray = false;

  // Filter duplicate (in particular: key mods and gamepad analog values are
  // commonly spammed)
  const InputEvent *latest_event =
      FindLatestInputEvent(&g, InputEventType_Key, (int)key);
  const KeyData *key_data = Gui::GetKeyData(&g, key);
  const bool latest_key_down =
      latest_event ? latest_event->Key.Down : key_data->Down;
  const float latest_key_analog =
      latest_event ? latest_event->Key.AnalogValue : key_data->AnalogValue;
  if (latest_key_down == down && latest_key_analog == analog_value)
    return;

  // Add event
  InputEvent e;
  e.Type = InputEventType_Key;
  e.Source =
      Gui::IsGamepadKey(key) ? InputSource_Gamepad : InputSource_Keyboard;
  e.EventId = g.InputEventsNextEventId++;
  e.Key.Key = key;
  e.Key.Down = down;
  e.Key.AnalogValue = analog_value;
  g.InputEventsQueue.push_back(e);
}

void IO::AddKeyEvent(Key key, bool down) {
  if (!AppAcceptingEvents)
    return;
  AddKeyAnalogEvent(key, down, down ? 1.0f : 0.0f);
}

// [Optional] Call after AddKeyEvent().
// Specify native keycode, scancode + Specify index for legacy <1.87 IsKeyXXX()
// functions with native indices. If you are writing a backend in 2022 or don't
// use IsKeyXXX() with native values that are not Key values, you can avoid
// calling this.
void IO::SetKeyEventNativeData(Key key, int native_keycode, int native_scancode,
                               int native_legacy_index) {
  if (key == Key_None)
    return;
  ASSERT(Gui::IsNamedKey(key)); // >= 512
  ASSERT(native_legacy_index == -1 ||
         Gui::IsLegacyKey((Key)native_legacy_index)); // >= 0 && <= 511
  UNUSED(native_keycode);                             // Yet unused
  UNUSED(native_scancode);                            // Yet unused

  // Build native->imgui map so old user code can still call key functions with
  // native 0..511 values.
#ifndef DISABLE_OBSOLETE_KEYIO
  const int legacy_key =
      (native_legacy_index != -1) ? native_legacy_index : native_keycode;
  if (!Gui::IsLegacyKey((Key)legacy_key))
    return;
  KeyMap[legacy_key] = key;
  KeyMap[key] = legacy_key;
#else
  UNUSED(key);
  UNUSED(native_legacy_index);
#endif
}

// Set master flag for accepting key/mouse/text events (default to true). Useful
// if you have native dialog boxes that are interrupting your application
// loop/refresh, and you want to disable events being queued while your app is
// frozen.
void IO::SetAppAcceptingEvents(bool accepting_events) {
  AppAcceptingEvents = accepting_events;
}

// Queue a mouse move event
void IO::AddMousePosEvent(float x, float y) {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;
  if (!AppAcceptingEvents)
    return;

  // Apply same flooring as UpdateMouseInputs()
  Vec2 pos((x > -FLT_MAX) ? Floor(x) : x, (y > -FLT_MAX) ? Floor(y) : y);

  // Filter duplicate
  const InputEvent *latest_event =
      FindLatestInputEvent(&g, InputEventType_MousePos);
  const Vec2 latest_pos = latest_event ? Vec2(latest_event->MousePos.PosX,
                                              latest_event->MousePos.PosY)
                                       : g.IO.MousePos;
  if (latest_pos.x == pos.x && latest_pos.y == pos.y)
    return;

  InputEvent e;
  e.Type = InputEventType_MousePos;
  e.Source = InputSource_Mouse;
  e.EventId = g.InputEventsNextEventId++;
  e.MousePos.PosX = pos.x;
  e.MousePos.PosY = pos.y;
  e.MousePos.MouseSource = g.InputEventsNextMouseSource;
  g.InputEventsQueue.push_back(e);
}

void IO::AddMouseButtonEvent(int mouse_button, bool down) {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;
  ASSERT(mouse_button >= 0 && mouse_button < MouseButton_COUNT);
  if (!AppAcceptingEvents)
    return;

  // Filter duplicate
  const InputEvent *latest_event =
      FindLatestInputEvent(&g, InputEventType_MouseButton, (int)mouse_button);
  const bool latest_button_down = latest_event ? latest_event->MouseButton.Down
                                               : g.IO.MouseDown[mouse_button];
  if (latest_button_down == down)
    return;

  InputEvent e;
  e.Type = InputEventType_MouseButton;
  e.Source = InputSource_Mouse;
  e.EventId = g.InputEventsNextEventId++;
  e.MouseButton.Button = mouse_button;
  e.MouseButton.Down = down;
  e.MouseButton.MouseSource = g.InputEventsNextMouseSource;
  g.InputEventsQueue.push_back(e);
}

// Queue a mouse wheel event (some mouse/API may only have a Y component)
void IO::AddMouseWheelEvent(float wheel_x, float wheel_y) {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;

  // Filter duplicate (unlike most events, wheel values are relative and easy to
  // filter)
  if (!AppAcceptingEvents || (wheel_x == 0.0f && wheel_y == 0.0f))
    return;

  InputEvent e;
  e.Type = InputEventType_MouseWheel;
  e.Source = InputSource_Mouse;
  e.EventId = g.InputEventsNextEventId++;
  e.MouseWheel.WheelX = wheel_x;
  e.MouseWheel.WheelY = wheel_y;
  e.MouseWheel.MouseSource = g.InputEventsNextMouseSource;
  g.InputEventsQueue.push_back(e);
}

// This is not a real event, the data is latched in order to be stored in actual
// Mouse events. This is so that duplicate events (e.g. Windows sending
// extraneous WM_MOUSEMOVE) gets filtered and are not leading to actual source
// changes.
void IO::AddMouseSourceEvent(MouseSource source) {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;
  g.InputEventsNextMouseSource = source;
}

void IO::AddMouseViewportEvent(ID viewport_id) {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;
  // ASSERT(g.IO.BackendFlags & BackendFlags_HasMouseHoveredViewport);
  if (!AppAcceptingEvents)
    return;

  // Filter duplicate
  const InputEvent *latest_event =
      FindLatestInputEvent(&g, InputEventType_MouseViewport);
  const ID latest_viewport_id =
      latest_event ? latest_event->MouseViewport.HoveredViewportID
                   : g.IO.MouseHoveredViewport;
  if (latest_viewport_id == viewport_id)
    return;

  InputEvent e;
  e.Type = InputEventType_MouseViewport;
  e.Source = InputSource_Mouse;
  e.MouseViewport.HoveredViewportID = viewport_id;
  g.InputEventsQueue.push_back(e);
}

void IO::AddFocusEvent(bool focused) {
  ASSERT(Ctx != NULL);
  Context &g = *Ctx;

  // Filter duplicate
  const InputEvent *latest_event =
      FindLatestInputEvent(&g, InputEventType_Focus);
  const bool latest_focused =
      latest_event ? latest_event->AppFocused.Focused : !g.IO.AppFocusLost;
  if (latest_focused == focused || (ConfigDebugIgnoreFocusLoss && !focused))
    return;

  InputEvent e;
  e.Type = InputEventType_Focus;
  e.EventId = g.InputEventsNextEventId++;
  e.AppFocused.Focused = focused;
  g.InputEventsQueue.push_back(e);
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Geometry functions)
//-----------------------------------------------------------------------------

Vec2 BezierCubicClosestPoint(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                             const Vec2 &p4, const Vec2 &p, int num_segments) {
  ASSERT(num_segments > 0); // Use BezierCubicClosestPointCasteljau()
  Vec2 p_last = p1;
  Vec2 p_closest;
  float p_closest_dist2 = FLT_MAX;
  float t_step = 1.0f / (float)num_segments;
  for (int i_step = 1; i_step <= num_segments; i_step++) {
    Vec2 p_current = BezierCubicCalc(p1, p2, p3, p4, t_step * i_step);
    Vec2 p_line = LineClosestPoint(p_last, p_current, p);
    float dist2 = LengthSqr(p - p_line);
    if (dist2 < p_closest_dist2) {
      p_closest = p_line;
      p_closest_dist2 = dist2;
    }
    p_last = p_current;
  }
  return p_closest;
}

// Closely mimics PathBezierToCasteljau() in draw.cpp
static void BezierCubicClosestPointCasteljauStep(
    const Vec2 &p, Vec2 &p_closest, Vec2 &p_last, float &p_closest_dist2,
    float x1, float y1, float x2, float y2, float x3, float y3, float x4,
    float y4, float tess_tol, int level) {
  float dx = x4 - x1;
  float dy = y4 - y1;
  float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
  float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
  d2 = (d2 >= 0) ? d2 : -d2;
  d3 = (d3 >= 0) ? d3 : -d3;
  if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy)) {
    Vec2 p_current(x4, y4);
    Vec2 p_line = LineClosestPoint(p_last, p_current, p);
    float dist2 = LengthSqr(p - p_line);
    if (dist2 < p_closest_dist2) {
      p_closest = p_line;
      p_closest_dist2 = dist2;
    }
    p_last = p_current;
  } else if (level < 10) {
    float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
    float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
    float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
    float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
    float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
    float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
    BezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2,
                                         x1, y1, x12, y12, x123, y123, x1234,
                                         y1234, tess_tol, level + 1);
    BezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2,
                                         x1234, y1234, x234, y234, x34, y34, x4,
                                         y4, tess_tol, level + 1);
  }
}

// tess_tol is generally the same value you would find in
// Gui::GetStyle().CurveTessellationTol Because those XXX functions are
// lower-level than Gui:: we cannot access this value automatically.
Vec2 BezierCubicClosestPointCasteljau(const Vec2 &p1, const Vec2 &p2,
                                      const Vec2 &p3, const Vec2 &p4,
                                      const Vec2 &p, float tess_tol) {
  ASSERT(tess_tol > 0.0f);
  Vec2 p_last = p1;
  Vec2 p_closest;
  float p_closest_dist2 = FLT_MAX;
  BezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2,
                                       p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x,
                                       p4.y, tess_tol, 0);
  return p_closest;
}

Vec2 LineClosestPoint(const Vec2 &a, const Vec2 &b, const Vec2 &p) {
  Vec2 ap = p - a;
  Vec2 ab_dir = b - a;
  float dot = ap.x * ab_dir.x + ap.y * ab_dir.y;
  if (dot < 0.0f)
    return a;
  float ab_len_sqr = ab_dir.x * ab_dir.x + ab_dir.y * ab_dir.y;
  if (dot > ab_len_sqr)
    return b;
  return a + ab_dir * dot / ab_len_sqr;
}

bool TriangleContainsPoint(const Vec2 &a, const Vec2 &b, const Vec2 &c,
                           const Vec2 &p) {
  bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
  bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
  bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
  return ((b1 == b2) && (b2 == b3));
}

void TriangleBarycentricCoords(const Vec2 &a, const Vec2 &b, const Vec2 &c,
                               const Vec2 &p, float &out_u, float &out_v,
                               float &out_w) {
  Vec2 v0 = b - a;
  Vec2 v1 = c - a;
  Vec2 v2 = p - a;
  const float denom = v0.x * v1.y - v1.x * v0.y;
  out_v = (v2.x * v1.y - v1.x * v2.y) / denom;
  out_w = (v0.x * v2.y - v2.x * v0.y) / denom;
  out_u = 1.0f - out_v - out_w;
}

Vec2 TriangleClosestPoint(const Vec2 &a, const Vec2 &b, const Vec2 &c,
                          const Vec2 &p) {
  Vec2 proj_ab = LineClosestPoint(a, b, p);
  Vec2 proj_bc = LineClosestPoint(b, c, p);
  Vec2 proj_ca = LineClosestPoint(c, a, p);
  float dist2_ab = LengthSqr(p - proj_ab);
  float dist2_bc = LengthSqr(p - proj_bc);
  float dist2_ca = LengthSqr(p - proj_ca);
  float m = Min(dist2_ab, Min(dist2_bc, dist2_ca));
  if (m == dist2_ab)
    return proj_ab;
  if (m == dist2_bc)
    return proj_bc;
  return proj_ca;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (String, Format, Hash functions)
//-----------------------------------------------------------------------------

// Consider using _stricmp/_strnicmp under Windows or strcasecmp/strncasecmp. We
// don't actually use either Stricmp/Strnicmp in the codebase any more.
int Stricmp(const char *str1, const char *str2) {
  int d;
  while ((d = ToUpper(*str2) - ToUpper(*str1)) == 0 && *str1) {
    str1++;
    str2++;
  }
  return d;
}

int Strnicmp(const char *str1, const char *str2, size_t count) {
  int d = 0;
  while (count > 0 && (d = ToUpper(*str2) - ToUpper(*str1)) == 0 && *str1) {
    str1++;
    str2++;
    count--;
  }
  return d;
}

void Strncpy(char *dst, const char *src, size_t count) {
  if (count < 1)
    return;
  if (count > 1)
    strncpy(dst, src, count - 1);
  dst[count - 1] = 0;
}

char *Strdup(const char *str) {
  size_t len = strlen(str);
  void *buf = ALLOC(len + 1);
  return (char *)memcpy(buf, (const void *)str, len + 1);
}

char *Strdupcpy(char *dst, size_t *p_dst_size, const char *src) {
  size_t dst_buf_size = p_dst_size ? *p_dst_size : strlen(dst) + 1;
  size_t src_size = strlen(src) + 1;
  if (dst_buf_size < src_size) {
    FREE(dst);
    dst = (char *)ALLOC(src_size);
    if (p_dst_size)
      *p_dst_size = src_size;
  }
  return (char *)memcpy(dst, (const void *)src, src_size);
}

const char *StrchrRange(const char *str, const char *str_end, char c) {
  const char *p = (const char *)memchr(str, (int)c, str_end - str);
  return p;
}

int StrlenW(const Wchar *str) {
  // return (int)wcslen((const wchar_t*)str);  // FIXME-OPT: Could use this when
  // wchar_t are 16-bit
  int n = 0;
  while (*str++)
    n++;
  return n;
}

// Find end-of-line. Return pointer will point to either first \n, either
// str_end.
const char *StreolRange(const char *str, const char *str_end) {
  const char *p = (const char *)memchr(str, '\n', str_end - str);
  return p ? p : str_end;
}

const Wchar *StrbolW(const Wchar *buf_mid_line,
                     const Wchar *buf_begin) // find beginning-of-line
{
  while (buf_mid_line > buf_begin && buf_mid_line[-1] != '\n')
    buf_mid_line--;
  return buf_mid_line;
}

const char *Stristr(const char *haystack, const char *haystack_end,
                    const char *needle, const char *needle_end) {
  if (!needle_end)
    needle_end = needle + strlen(needle);

  const char un0 = (char)ToUpper(*needle);
  while ((!haystack_end && *haystack) ||
         (haystack_end && haystack < haystack_end)) {
    if (ToUpper(*haystack) == un0) {
      const char *b = needle + 1;
      for (const char *a = haystack + 1; b < needle_end; a++, b++)
        if (ToUpper(*a) != ToUpper(*b))
          break;
      if (b == needle_end)
        return haystack;
    }
    haystack++;
  }
  return NULL;
}

// Trim str by offsetting contents when there's leading data + writing a \0 at
// the trailing position. We use this in situation where the cost is negligible.
void StrTrimBlanks(char *buf) {
  char *p = buf;
  while (p[0] == ' ' || p[0] == '\t') // Leading blanks
    p++;
  char *p_start = p;
  while (*p != 0) // Find end of string
    p++;
  while (p > p_start && (p[-1] == ' ' || p[-1] == '\t')) // Trailing blanks
    p--;
  if (p_start != buf) // Copy memory if we had leading blanks
    memmove(buf, p_start, p - p_start);
  buf[p - p_start] = 0; // Zero terminate
}

const char *StrSkipBlank(const char *str) {
  while (str[0] == ' ' || str[0] == '\t')
    str++;
  return str;
}

// A) MSVC version appears to return -1 on overflow, whereas glibc appears to
// return total count (which may be >= buf_size). Ideally we would test for only
// one of those limits at runtime depending on the behavior the vsnprintf(), but
// trying to deduct it at compile time sounds like a pandora can of worm. B)
// When buf==NULL vsnprintf() will return the output size.
#ifndef DISABLE_DEFAULT_FORMAT_FUNCTIONS

// We support sprintf which is much faster.
// USE_SPRINTF to use our default wrapper, or set
// DISABLE_DEFAULT_FORMAT_FUNCTIONS and setup the wrapper yourself. (FIXME-OPT:
// Some of our high-level operations such as TextBuffer::appendfv() are
// designed using two-passes worst case, which probably could be improved using
// the stbsp_vsprintfcb() function.)
#ifdef USE_SPRINTF
#ifndef DISABLE_SPRINTF_IMPLEMENTATION
#define SPRINTF_IMPLEMENTATION
#endif
#ifdef SPRINTF_FILENAME
#include SPRINTF_FILENAME
#else
#include "sprintf.h"
#endif
#endif // #ifdef USE_SPRINTF

#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf _vsnprintf
#endif

int FormatString(char *buf, size_t buf_size, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
#ifdef USE_SPRINTF
  int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
  int w = vsnprintf(buf, buf_size, fmt, args);
#endif
  va_end(args);
  if (buf == NULL)
    return w;
  if (w == -1 || w >= (int)buf_size)
    w = (int)buf_size - 1;
  buf[w] = 0;
  return w;
}

int FormatStringV(char *buf, size_t buf_size, const char *fmt, va_list args) {
#ifdef USE_SPRINTF
  int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
  int w = vsnprintf(buf, buf_size, fmt, args);
#endif
  if (buf == NULL)
    return w;
  if (w == -1 || w >= (int)buf_size)
    w = (int)buf_size - 1;
  buf[w] = 0;
  return w;
}
#endif // #ifdef DISABLE_DEFAULT_FORMAT_FUNCTIONS

void FormatStringToTempBuffer(const char **out_buf, const char **out_buf_end,
                              const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FormatStringToTempBufferV(out_buf, out_buf_end, fmt, args);
  va_end(args);
}

void FormatStringToTempBufferV(const char **out_buf, const char **out_buf_end,
                               const char *fmt, va_list args) {
  Context &g = *GGui;
  if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0) {
    const char *buf =
        va_arg(args, const char *); // Skip formatting when using "%s"
    if (buf == NULL)
      buf = "(null)";
    *out_buf = buf;
    if (out_buf_end) {
      *out_buf_end = buf + strlen(buf);
    }
  } else if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '*' && fmt[3] == 's' &&
             fmt[4] == 0) {
    int buf_len = va_arg(args, int); // Skip formatting when using "%.*s"
    const char *buf = va_arg(args, const char *);
    if (buf == NULL) {
      buf = "(null)";
      buf_len = Min(buf_len, 6);
    }
    *out_buf = buf;
    *out_buf_end = buf + buf_len; // Disallow not passing 'out_buf_end' here.
                                  // User is expected to use it.
  } else {
    int buf_len =
        FormatStringV(g.TempBuffer.Data, g.TempBuffer.Size, fmt, args);
    *out_buf = g.TempBuffer.Data;
    if (out_buf_end) {
      *out_buf_end = g.TempBuffer.Data + buf_len;
    }
  }
}

// CRC32 needs a 1KB lookup table (not cache friendly)
// Although the code to generate the table is simple and shorter than the table
// itself, using a const table allows us to easily:
// - avoid an unnecessary branch/memory tap, - keep the HashXXX functions
// usable by static constructors, - make it thread-safe.
static const U32 GCrc32LookupTable[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

// Known size hash
// It is ok to call HashData on a string with known length but the ###
// operator won't be supported.
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access
// 1KB. Need to do proper measurements.
ID HashData(const void *data_p, size_t data_size, ID seed) {
  U32 crc = ~seed;
  const unsigned char *data = (const unsigned char *)data_p;
  const U32 *crc32_lut = GCrc32LookupTable;
  while (data_size-- != 0)
    crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ *data++];
  return ~crc;
}

// Zero-terminated string hash, with support for ### to reset back to seed value
// We support a syntax of "label###id" where only "###id" is included in the
// hash, and only "label" gets displayed. Because this syntax is rarely used we
// are optimizing for the common case.
// - If we reach ### in the string we discard the hash so far and reset to the
// seed.
// - We don't do 'current += 2; continue;' after handling ### to keep the code
// smaller/faster (measured ~10% diff in Debug build)
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access
// 1KB. Need to do proper measurements.
ID HashStr(const char *data_p, size_t data_size, ID seed) {
  seed = ~seed;
  U32 crc = seed;
  const unsigned char *data = (const unsigned char *)data_p;
  const U32 *crc32_lut = GCrc32LookupTable;
  if (data_size != 0) {
    while (data_size-- != 0) {
      unsigned char c = *data++;
      if (c == '#' && data_size >= 2 && data[0] == '#' && data[1] == '#')
        crc = seed;
      crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
    }
  } else {
    while (unsigned char c = *data++) {
      if (c == '#' && data[0] == '#' && data[1] == '#')
        crc = seed;
      crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
    }
  }
  return ~crc;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (File functions)
//-----------------------------------------------------------------------------

// Default file functions
#ifndef DISABLE_DEFAULT_FILE_FUNCTIONS

FileHandle FileOpen(const char *filename, const char *mode) {
#if defined(_WIN32) && !defined(DISABLE_WIN32_FUNCTIONS) &&                    \
    !defined(__CYGWIN__) && !defined(__GNUC__)
  // We need a fopen() wrapper because MSVC/Windows fopen doesn't handle UTF-8
  // filenames. Previously we used TextCountCharsFromUtf8/TextStrFromUtf8
  // here but we now need to support Wchar16 and Wchar32!
  const int filename_wsize =
      ::MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
  const int mode_wsize = ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
  Context &g = *GGui;
  g.TempBuffer.reserve((filename_wsize + mode_wsize) * sizeof(wchar_t));
  wchar_t *buf = (wchar_t *)(void *)g.TempBuffer.Data;
  ::MultiByteToWideChar(CP_UTF8, 0, filename, -1, (wchar_t *)&buf[0],
                        filename_wsize);
  ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, (wchar_t *)&buf[filename_wsize],
                        mode_wsize);
  return ::_wfopen((const wchar_t *)&buf[0],
                   (const wchar_t *)&buf[filename_wsize]);
#else
  return fopen(filename, mode);
#endif
}

// We should in theory be using fseeko()/ftello() with off_t and
// _fseeki64()/_ftelli64() with __int64, waiting for the PR that does that in a
// very portable pre-C++11 zero-warnings way.
bool FileClose(FileHandle f) { return fclose(f) == 0; }
U64 FileGetSize(FileHandle f) {
  long off = 0, sz = 0;
  return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) &&
          (sz = ftell(f)) != -1 && !fseek(f, off, SEEK_SET))
             ? (U64)sz
             : (U64)-1;
}
U64 FileRead(void *data, U64 sz, U64 count, FileHandle f) {
  return fread(data, (size_t)sz, (size_t)count, f);
}
U64 FileWrite(const void *data, U64 sz, U64 count, FileHandle f) {
  return fwrite(data, (size_t)sz, (size_t)count, f);
}
#endif // #ifndef DISABLE_DEFAULT_FILE_FUNCTIONS

// Helper: Load file content into memory
// Memory allocated with ALLOC(), must be freed by user using FREE() ==
// Gui::MemFree() This can't really be used with "rt" because fseek size won't
// match read size.
void *FileLoadToMemory(const char *filename, const char *mode,
                       size_t *out_file_size, int padding_bytes) {
  ASSERT(filename && mode);
  if (out_file_size)
    *out_file_size = 0;

  FileHandle f;
  if ((f = FileOpen(filename, mode)) == NULL)
    return NULL;

  size_t file_size = (size_t)FileGetSize(f);
  if (file_size == (size_t)-1) {
    FileClose(f);
    return NULL;
  }

  void *file_data = ALLOC(file_size + padding_bytes);
  if (file_data == NULL) {
    FileClose(f);
    return NULL;
  }
  if (FileRead(file_data, 1, file_size, f) != file_size) {
    FileClose(f);
    FREE(file_data);
    return NULL;
  }
  if (padding_bytes > 0)
    memset((void *)(((char *)file_data) + file_size), 0, (size_t)padding_bytes);

  FileClose(f);
  if (out_file_size)
    *out_file_size = file_size;

  return file_data;
}

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Text* functions)
//-----------------------------------------------------------------------------

MSVC_RUNTIME_CHECKS_OFF

// Convert UTF-8 to 32-bit character, process single character input.
// A nearly-branchless UTF-8 decoder, based on work of Christopher Wellons
// (https://github.com/skeeto/branchless-utf8). We handle UTF-8 decoding error
// by skipping forward.
int TextCharFromUtf8(unsigned int *out_char, const char *in_text,
                     const char *in_text_end) {
  static const char lengths[32] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                   0, 0, 2, 2, 2, 2, 3, 3, 4, 0};
  static const int masks[] = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
  static const uint32_t mins[] = {0x400000, 0, 0x80, 0x800, 0x10000};
  static const int shiftc[] = {0, 18, 12, 6, 0};
  static const int shifte[] = {0, 6, 4, 2, 0};
  int len = lengths[*(const unsigned char *)in_text >> 3];
  int wanted = len + (len ? 0 : 1);

  if (in_text_end == NULL)
    in_text_end =
        in_text + wanted; // Max length, nulls will be taken into account.

  // Copy at most 'len' bytes, stop copying at 0 or past in_text_end. Branch
  // predictor does a good job here, so it is fast even with excessive
  // branching.
  unsigned char s[4];
  s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
  s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
  s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
  s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

  // Assume a four-byte character and load four bytes. Unused bits are shifted
  // out.
  *out_char = (uint32_t)(s[0] & masks[len]) << 18;
  *out_char |= (uint32_t)(s[1] & 0x3f) << 12;
  *out_char |= (uint32_t)(s[2] & 0x3f) << 6;
  *out_char |= (uint32_t)(s[3] & 0x3f) << 0;
  *out_char >>= shiftc[len];

  // Accumulate the various error conditions.
  int e = 0;
  e = (*out_char < mins[len]) << 6;              // non-canonical encoding
  e |= ((*out_char >> 11) == 0x1b) << 7;         // surrogate half?
  e |= (*out_char > UNICODE_CODEPOINT_MAX) << 8; // out of range?
  e |= (s[1] & 0xc0) >> 2;
  e |= (s[2] & 0xc0) >> 4;
  e |= (s[3]) >> 6;
  e ^= 0x2a; // top two bits of each tail byte correct?
  e >>= shifte[len];

  if (e) {
    // No bytes are consumed when *in_text == 0 || in_text == in_text_end.
    // One byte is consumed in case of invalid first byte of in_text.
    // All available bytes (at most `len` bytes) are consumed on
    // incomplete/invalid second to last bytes. Invalid or incomplete input may
    // consume less bytes than wanted, therefore every byte has to be inspected
    // in s.
    wanted = Min(wanted, !!s[0] + !!s[1] + !!s[2] + !!s[3]);
    *out_char = UNICODE_CODEPOINT_INVALID;
  }

  return wanted;
}

int TextStrFromUtf8(Wchar *buf, int buf_size, const char *in_text,
                    const char *in_text_end, const char **in_text_remaining) {
  Wchar *buf_out = buf;
  Wchar *buf_end = buf + buf_size;
  while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) &&
         *in_text) {
    unsigned int c;
    in_text += TextCharFromUtf8(&c, in_text, in_text_end);
    *buf_out++ = (Wchar)c;
  }
  *buf_out = 0;
  if (in_text_remaining)
    *in_text_remaining = in_text;
  return (int)(buf_out - buf);
}

int TextCountCharsFromUtf8(const char *in_text, const char *in_text_end) {
  int char_count = 0;
  while ((!in_text_end || in_text < in_text_end) && *in_text) {
    unsigned int c;
    in_text += TextCharFromUtf8(&c, in_text, in_text_end);
    char_count++;
  }
  return char_count;
}

// Based on to_utf8() from github.com/nothings/stb/
static inline int TextCharToUtf8_inline(char *buf, int buf_size,
                                        unsigned int c) {
  if (c < 0x80) {
    buf[0] = (char)c;
    return 1;
  }
  if (c < 0x800) {
    if (buf_size < 2)
      return 0;
    buf[0] = (char)(0xc0 + (c >> 6));
    buf[1] = (char)(0x80 + (c & 0x3f));
    return 2;
  }
  if (c < 0x10000) {
    if (buf_size < 3)
      return 0;
    buf[0] = (char)(0xe0 + (c >> 12));
    buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
    buf[2] = (char)(0x80 + ((c) & 0x3f));
    return 3;
  }
  if (c <= 0x10FFFF) {
    if (buf_size < 4)
      return 0;
    buf[0] = (char)(0xf0 + (c >> 18));
    buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
    buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
    buf[3] = (char)(0x80 + ((c) & 0x3f));
    return 4;
  }
  // Invalid code point, the max unicode is 0x10FFFF
  return 0;
}

const char *TextCharToUtf8(char out_buf[5], unsigned int c) {
  int count = TextCharToUtf8_inline(out_buf, 5, c);
  out_buf[count] = 0;
  return out_buf;
}

// Not optimal but we very rarely use this function.
int TextCountUtf8BytesFromChar(const char *in_text, const char *in_text_end) {
  unsigned int unused = 0;
  return TextCharFromUtf8(&unused, in_text, in_text_end);
}

static inline int TextCountUtf8BytesFromChar(unsigned int c) {
  if (c < 0x80)
    return 1;
  if (c < 0x800)
    return 2;
  if (c < 0x10000)
    return 3;
  if (c <= 0x10FFFF)
    return 4;
  return 3;
}

int TextStrToUtf8(char *out_buf, int out_buf_size, const Wchar *in_text,
                  const Wchar *in_text_end) {
  char *buf_p = out_buf;
  const char *buf_end = out_buf + out_buf_size;
  while (buf_p < buf_end - 1 && (!in_text_end || in_text < in_text_end) &&
         *in_text) {
    unsigned int c = (unsigned int)(*in_text++);
    if (c < 0x80)
      *buf_p++ = (char)c;
    else
      buf_p += TextCharToUtf8_inline(buf_p, (int)(buf_end - buf_p - 1), c);
  }
  *buf_p = 0;
  return (int)(buf_p - out_buf);
}

int TextCountUtf8BytesFromStr(const Wchar *in_text, const Wchar *in_text_end) {
  int bytes_count = 0;
  while ((!in_text_end || in_text < in_text_end) && *in_text) {
    unsigned int c = (unsigned int)(*in_text++);
    if (c < 0x80)
      bytes_count++;
    else
      bytes_count += TextCountUtf8BytesFromChar(c);
  }
  return bytes_count;
}

const char *TextFindPreviousUtf8Codepoint(const char *in_text_start,
                                          const char *in_text_curr) {
  while (in_text_curr > in_text_start) {
    in_text_curr--;
    if ((*in_text_curr & 0xC0) != 0x80)
      return in_text_curr;
  }
  return in_text_start;
}

MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] MISC HELPERS/UTILITIES (Color functions)
// Note: The Convert functions are early design which are not consistent with
// other API.
//-----------------------------------------------------------------------------

API U32 AlphaBlendColors(U32 col_a, U32 col_b) {
  float t = ((col_b >> COL32_A_SHIFT) & 0xFF) / 255.f;
  int r = Lerp((int)(col_a >> COL32_R_SHIFT) & 0xFF,
               (int)(col_b >> COL32_R_SHIFT) & 0xFF, t);
  int g = Lerp((int)(col_a >> COL32_G_SHIFT) & 0xFF,
               (int)(col_b >> COL32_G_SHIFT) & 0xFF, t);
  int b = Lerp((int)(col_a >> COL32_B_SHIFT) & 0xFF,
               (int)(col_b >> COL32_B_SHIFT) & 0xFF, t);
  return COL32(r, g, b, 0xFF);
}

Vec4 Gui::ColorConvertU32ToFloat4(U32 in) {
  float s = 1.0f / 255.0f;
  return Vec4(
      ((in >> COL32_R_SHIFT) & 0xFF) * s, ((in >> COL32_G_SHIFT) & 0xFF) * s,
      ((in >> COL32_B_SHIFT) & 0xFF) * s, ((in >> COL32_A_SHIFT) & 0xFF) * s);
}

U32 Gui::ColorConvertFloat4ToU32(const Vec4 &in) {
  U32 out;
  out = ((U32)F32_TO_INT8_SAT(in.x)) << COL32_R_SHIFT;
  out |= ((U32)F32_TO_INT8_SAT(in.y)) << COL32_G_SHIFT;
  out |= ((U32)F32_TO_INT8_SAT(in.z)) << COL32_B_SHIFT;
  out |= ((U32)F32_TO_INT8_SAT(in.w)) << COL32_A_SHIFT;
  return out;
}

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]),
// from Foley & van Dam p592 Optimized
// http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
void Gui::ColorConvertRGBtoHSV(float r, float g, float b, float &out_h,
                               float &out_s, float &out_v) {
  float K = 0.f;
  if (g < b) {
    Swap(g, b);
    K = -1.f;
  }
  if (r < g) {
    Swap(r, g);
    K = -2.f / 6.f - K;
  }

  const float chroma = r - (g < b ? g : b);
  out_h = Fabs(K + (g - b) / (6.f * chroma + 1e-20f));
  out_s = chroma / (r + 1e-20f);
  out_v = r;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]),
// from Foley & van Dam p593 also http://en.wikipedia.org/wiki/HSL_and_HSV
void Gui::ColorConvertHSVtoRGB(float h, float s, float v, float &out_r,
                               float &out_g, float &out_b) {
  if (s == 0.0f) {
    // gray
    out_r = out_g = out_b = v;
    return;
  }

  h = Fmod(h, 1.0f) / (60.0f / 360.0f);
  int i = (int)h;
  float f = h - (float)i;
  float p = v * (1.0f - s);
  float q = v * (1.0f - s * f);
  float t = v * (1.0f - s * (1.0f - f));

  switch (i) {
  case 0:
    out_r = v;
    out_g = t;
    out_b = p;
    break;
  case 1:
    out_r = q;
    out_g = v;
    out_b = p;
    break;
  case 2:
    out_r = p;
    out_g = v;
    out_b = t;
    break;
  case 3:
    out_r = p;
    out_g = q;
    out_b = v;
    break;
  case 4:
    out_r = t;
    out_g = p;
    out_b = v;
    break;
  case 5:
  default:
    out_r = v;
    out_g = p;
    out_b = q;
    break;
  }
}

//-----------------------------------------------------------------------------
// [SECTION] Storage
// Helper: Key->value storage
//-----------------------------------------------------------------------------

// std::lower_bound but without the bullshit
static Storage::StoragePair *LowerBound(Vector<Storage::StoragePair> &data,
                                        ID key) {
  Storage::StoragePair *first = data.Data;
  Storage::StoragePair *last = data.Data + data.Size;
  size_t count = (size_t)(last - first);
  while (count > 0) {
    size_t count2 = count >> 1;
    Storage::StoragePair *mid = first + count2;
    if (mid->key < key) {
      first = ++mid;
      count -= count2 + 1;
    } else {
      count = count2;
    }
  }
  return first;
}

// For quicker full rebuild of a storage (instead of an incremental one), you
// may add all your contents and then sort once.
void Storage::BuildSortByKey() {
  struct StaticFunc {
    static int CDECL PairComparerByID(const void *lhs, const void *rhs) {
      // We can't just do a subtraction because qsort uses signed integers and
      // subtracting our ID doesn't play well with that.
      if (((const StoragePair *)lhs)->key > ((const StoragePair *)rhs)->key)
        return +1;
      if (((const StoragePair *)lhs)->key < ((const StoragePair *)rhs)->key)
        return -1;
      return 0;
    }
  };
  Qsort(Data.Data, (size_t)Data.Size, sizeof(StoragePair),
        StaticFunc::PairComparerByID);
}

int Storage::GetInt(ID key, int default_val) const {
  StoragePair *it = LowerBound(const_cast<Vector<StoragePair> &>(Data), key);
  if (it == Data.end() || it->key != key)
    return default_val;
  return it->val_i;
}

bool Storage::GetBool(ID key, bool default_val) const {
  return GetInt(key, default_val ? 1 : 0) != 0;
}

float Storage::GetFloat(ID key, float default_val) const {
  StoragePair *it = LowerBound(const_cast<Vector<StoragePair> &>(Data), key);
  if (it == Data.end() || it->key != key)
    return default_val;
  return it->val_f;
}

void *Storage::GetVoidPtr(ID key) const {
  StoragePair *it = LowerBound(const_cast<Vector<StoragePair> &>(Data), key);
  if (it == Data.end() || it->key != key)
    return NULL;
  return it->val_p;
}

// References are only valid until a new value is added to the storage. Calling
// a Set***() function or a Get***Ref() function invalidates the pointer.
int *Storage::GetIntRef(ID key, int default_val) {
  StoragePair *it = LowerBound(Data, key);
  if (it == Data.end() || it->key != key)
    it = Data.insert(it, StoragePair(key, default_val));
  return &it->val_i;
}

bool *Storage::GetBoolRef(ID key, bool default_val) {
  return (bool *)GetIntRef(key, default_val ? 1 : 0);
}

float *Storage::GetFloatRef(ID key, float default_val) {
  StoragePair *it = LowerBound(Data, key);
  if (it == Data.end() || it->key != key)
    it = Data.insert(it, StoragePair(key, default_val));
  return &it->val_f;
}

void **Storage::GetVoidPtrRef(ID key, void *default_val) {
  StoragePair *it = LowerBound(Data, key);
  if (it == Data.end() || it->key != key)
    it = Data.insert(it, StoragePair(key, default_val));
  return &it->val_p;
}

// FIXME-OPT: Need a way to reuse the result of lower_bound when doing
// GetInt()/SetInt() - not too bad because it only happens on explicit
// interaction (maximum one a frame)
void Storage::SetInt(ID key, int val) {
  StoragePair *it = LowerBound(Data, key);
  if (it == Data.end() || it->key != key)
    Data.insert(it, StoragePair(key, val));
  else
    it->val_i = val;
}

void Storage::SetBool(ID key, bool val) { SetInt(key, val ? 1 : 0); }

void Storage::SetFloat(ID key, float val) {
  StoragePair *it = LowerBound(Data, key);
  if (it == Data.end() || it->key != key)
    Data.insert(it, StoragePair(key, val));
  else
    it->val_f = val;
}

void Storage::SetVoidPtr(ID key, void *val) {
  StoragePair *it = LowerBound(Data, key);
  if (it == Data.end() || it->key != key)
    Data.insert(it, StoragePair(key, val));
  else
    it->val_p = val;
}

void Storage::SetAllInt(int v) {
  for (int i = 0; i < Data.Size; i++)
    Data[i].val_i = v;
}

//-----------------------------------------------------------------------------
// [SECTION] TextFilter
//-----------------------------------------------------------------------------

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
TextFilter::TextFilter(const char *default_filter) //-V1077
{
  InputBuf[0] = 0;
  CountGrep = 0;
  if (default_filter) {
    Strncpy(InputBuf, default_filter, ARRAYSIZE(InputBuf));
    Build();
  }
}

bool TextFilter::Draw(const char *label, float width) {
  if (width != 0.0f)
    Gui::SetNextItemWidth(width);
  bool value_changed = Gui::InputText(label, InputBuf, ARRAYSIZE(InputBuf));
  if (value_changed)
    Build();
  return value_changed;
}

void TextFilter::TextRange::split(char separator,
                                  Vector<TextRange> *out) const {
  out->resize(0);
  const char *wb = b;
  const char *we = wb;
  while (we < e) {
    if (*we == separator) {
      out->push_back(TextRange(wb, we));
      wb = we + 1;
    }
    we++;
  }
  if (wb != we)
    out->push_back(TextRange(wb, we));
}

void TextFilter::Build() {
  Filters.resize(0);
  TextRange input_range(InputBuf, InputBuf + strlen(InputBuf));
  input_range.split(',', &Filters);

  CountGrep = 0;
  for (TextRange &f : Filters) {
    while (f.b < f.e && CharIsBlankA(f.b[0]))
      f.b++;
    while (f.e > f.b && CharIsBlankA(f.e[-1]))
      f.e--;
    if (f.empty())
      continue;
    if (f.b[0] != '-')
      CountGrep += 1;
  }
}

bool TextFilter::PassFilter(const char *text, const char *text_end) const {
  if (Filters.empty())
    return true;

  if (text == NULL)
    text = "";

  for (const TextRange &f : Filters) {
    if (f.empty())
      continue;
    if (f.b[0] == '-') {
      // Subtract
      if (Stristr(text, text_end, f.b + 1, f.e) != NULL)
        return false;
    } else {
      // Grep
      if (Stristr(text, text_end, f.b, f.e) != NULL)
        return true;
    }
  }

  // Implicit * grep
  if (CountGrep == 0)
    return true;

  return false;
}

//-----------------------------------------------------------------------------
// [SECTION] TextBuffer, TextIndex
//-----------------------------------------------------------------------------

// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to
// 2013 doesn't have it.
#ifndef va_copy
#if defined(__GNUC__) || defined(__clang__)
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#else
#define va_copy(dest, src) (dest = src)
#endif
#endif

char TextBuffer::EmptyString[1] = {0};

void TextBuffer::append(const char *str, const char *str_end) {
  int len = str_end ? (int)(str_end - str) : (int)strlen(str);

  // Add zero-terminator the first time
  const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
  const int needed_sz = write_off + len;
  if (write_off + len >= Buf.Capacity) {
    int new_capacity = Buf.Capacity * 2;
    Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
  }

  Buf.resize(needed_sz);
  memcpy(&Buf[write_off - 1], str, (size_t)len);
  Buf[write_off - 1 + len] = 0;
}

void TextBuffer::appendf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  appendfv(fmt, args);
  va_end(args);
}

// Helper: Text buffer for logging/accumulating text
void TextBuffer::appendfv(const char *fmt, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);

  int len = FormatStringV(NULL, 0, fmt,
                          args); // FIXME-OPT: could do a first pass write
                                 // attempt, likely successful on first pass.
  if (len <= 0) {
    va_end(args_copy);
    return;
  }

  // Add zero-terminator the first time
  const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
  const int needed_sz = write_off + len;
  if (write_off + len >= Buf.Capacity) {
    int new_capacity = Buf.Capacity * 2;
    Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
  }

  Buf.resize(needed_sz);
  FormatStringV(&Buf[write_off - 1], (size_t)len + 1, fmt, args_copy);
  va_end(args_copy);
}

void TextIndex::append(const char *base, int old_size, int new_size) {
  ASSERT(old_size >= 0 && new_size >= old_size && new_size >= EndOffset);
  if (old_size == new_size)
    return;
  if (EndOffset == 0 || base[EndOffset - 1] == '\n')
    LineOffsets.push_back(EndOffset);
  const char *base_end = base + new_size;
  for (const char *p = base + old_size;
       (p = (const char *)memchr(p, '\n', base_end - p)) != 0;)
    if (++p < base_end) // Don't push a trailing offset on last \n
      LineOffsets.push_back((int)(intptr_t)(p - base));
  EndOffset = Max(EndOffset, new_size);
}

//-----------------------------------------------------------------------------
// [SECTION] ListClipper
//-----------------------------------------------------------------------------

// FIXME-TABLE: This prevents us from using ListClipper _inside_ a table
// cell. The problem we have is that without a Begin/End scheme for rows using
// the clipper is ambiguous.
static bool GetSkipItemForListClipping() {
  Context &g = *GGui;
  return (g.CurrentTable ? g.CurrentTable->HostSkipItems
                         : g.CurrentWindow->SkipItems);
}

static void ListClipper_SortAndFuseRanges(Vector<ListClipperRange> &ranges,
                                          int offset = 0) {
  if (ranges.Size - offset <= 1)
    return;

  // Helper to order ranges and fuse them together if possible (bubble sort is
  // fine as we are only sorting 2-3 entries)
  for (int sort_end = ranges.Size - offset - 1; sort_end > 0; --sort_end)
    for (int i = offset; i < sort_end + offset; ++i)
      if (ranges[i].Min > ranges[i + 1].Min)
        Swap(ranges[i], ranges[i + 1]);

  // Now fuse ranges together as much as possible.
  for (int i = 1 + offset; i < ranges.Size; i++) {
    ASSERT(!ranges[i].PosToIndexConvert && !ranges[i - 1].PosToIndexConvert);
    if (ranges[i - 1].Max < ranges[i].Min)
      continue;
    ranges[i - 1].Min = Min(ranges[i - 1].Min, ranges[i].Min);
    ranges[i - 1].Max = Max(ranges[i - 1].Max, ranges[i].Max);
    ranges.erase(ranges.Data + i);
    i--;
  }
}

static void ListClipper_SeekCursorAndSetupPrevLine(float pos_y,
                                                   float line_height) {
  // Set cursor position and a few other things so that SetScrollHereY() and
  // Columns() can work when seeking cursor.
  // FIXME: It is problematic that we have to do that here, because
  // custom/equivalent end-user code would stumble on the same issue. The
  // clipper should probably have a final step to display the last item in a
  // regular manner, maybe with an opt-out flag for data sets which may have
  // costly seek?
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  float off_y = pos_y - window->DC.CursorPos.y;
  window->DC.CursorPos.y = pos_y;
  window->DC.CursorMaxPos.y =
      Max(window->DC.CursorMaxPos.y, pos_y - g.Style.ItemSpacing.y);
  window->DC.CursorPosPrevLine.y =
      window->DC.CursorPos.y -
      line_height; // Setting those fields so that SetScrollHereY() can properly
                   // function after the end of our clipper usage.
  window->DC.PrevLineSize.y =
      (line_height -
       g.Style.ItemSpacing
           .y); // If we end up needing more accurate data (to e.g. use
                // SameLine) we may as well make the clipper have a fourth step
                // to let user process and display the last item in their list.
  if (OldColumns *columns = window->DC.CurrentColumns)
    columns->LineMinY =
        window->DC.CursorPos
            .y; // Setting this so that cell Y position are set properly
  if (Table *table = g.CurrentTable) {
    if (table->IsInsideRow)
      Gui::TableEndRow(table);
    table->RowPosY2 = window->DC.CursorPos.y;
    const int row_increase = (int)((off_y / line_height) + 0.5f);
    // table->CurrentRow += row_increase; // Can't do without fixing
    // TableEndRow()
    table->RowBgColorCounter += row_increase;
  }
}

static void ListClipper_SeekCursorForItem(ListClipper *clipper, int item_n) {
  // StartPosY starts from ItemsFrozen hence the subtraction
  // Perform the add and multiply with double to allow seeking through larger
  // ranges
  ListClipperData *data = (ListClipperData *)clipper->TempData;
  float pos_y =
      (float)((double)clipper->StartPosY + data->LossynessOffset +
              (double)(item_n - data->ItemsFrozen) * clipper->ItemsHeight);
  ListClipper_SeekCursorAndSetupPrevLine(pos_y, clipper->ItemsHeight);
}

ListClipper::ListClipper() { memset(this, 0, sizeof(*this)); }

ListClipper::~ListClipper() { End(); }

void ListClipper::Begin(int items_count, float items_height) {
  if (Ctx == NULL)
    Ctx = Gui::GetCurrentContext();

  Context &g = *Ctx;
  Window *window = g.CurrentWindow;
  DEBUG_LOG_CLIPPER("Clipper: Begin(%d,%.2f) in '%s'\n", items_count,
                    items_height, window->Name);

  if (Table *table = g.CurrentTable)
    if (table->IsInsideRow)
      Gui::TableEndRow(table);

  StartPosY = window->DC.CursorPos.y;
  ItemsHeight = items_height;
  ItemsCount = items_count;
  DisplayStart = -1;
  DisplayEnd = 0;

  // Acquire temporary buffer
  if (++g.ClipperTempDataStacked > g.ClipperTempData.Size)
    g.ClipperTempData.resize(g.ClipperTempDataStacked, ListClipperData());
  ListClipperData *data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
  data->Reset(this);
  data->LossynessOffset = window->DC.CursorStartPosLossyness.y;
  TempData = data;
}

void ListClipper::End() {
  if (ListClipperData *data = (ListClipperData *)TempData) {
    // In theory here we should assert that we are already at the right
    // position, but it seems saner to just seek at the end and not assert/crash
    // the user.
    Context &g = *Ctx;
    DEBUG_LOG_CLIPPER("Clipper: End() in '%s'\n", g.CurrentWindow->Name);
    if (ItemsCount >= 0 && ItemsCount < INT_MAX && DisplayStart >= 0)
      ListClipper_SeekCursorForItem(this, ItemsCount);

    // Restore temporary buffer and fix back pointers which may be invalidated
    // when nesting
    ASSERT(data->ListClipper == this);
    data->StepNo = data->Ranges.Size;
    if (--g.ClipperTempDataStacked > 0) {
      data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
      data->ListClipper->TempData = data;
    }
    TempData = NULL;
  }
  ItemsCount = -1;
}

void ListClipper::IncludeItemsByIndex(int item_begin, int item_end) {
  ListClipperData *data = (ListClipperData *)TempData;
  ASSERT(DisplayStart < 0); // Only allowed after Begin() and if there has
                            // not been a specified range yet.
  ASSERT(item_begin <= item_end);
  if (item_begin < item_end)
    data->Ranges.push_back(ListClipperRange::FromIndices(item_begin, item_end));
}

static bool ListClipper_StepInternal(ListClipper *clipper) {
  Context &g = *clipper->Ctx;
  Window *window = g.CurrentWindow;
  ListClipperData *data = (ListClipperData *)clipper->TempData;
  ASSERT(data != NULL && "Called ListClipper::Step() too many times, "
                         "or before ListClipper::Begin() ?");

  Table *table = g.CurrentTable;
  if (table && table->IsInsideRow)
    Gui::TableEndRow(table);

  // No items
  if (clipper->ItemsCount == 0 || GetSkipItemForListClipping())
    return false;

  // While we are in frozen row state, keep displaying items one by one,
  // unclipped
  // FIXME: Could be stored as a table-agnostic state.
  if (data->StepNo == 0 && table != NULL && !table->IsUnfrozenRows) {
    clipper->DisplayStart = data->ItemsFrozen;
    clipper->DisplayEnd = Min(data->ItemsFrozen + 1, clipper->ItemsCount);
    if (clipper->DisplayStart < clipper->DisplayEnd)
      data->ItemsFrozen++;
    return true;
  }

  // Step 0: Let you process the first element (regardless of it being visible
  // or not, so we can measure the element height)
  bool calc_clipping = false;
  if (data->StepNo == 0) {
    clipper->StartPosY = window->DC.CursorPos.y;
    if (clipper->ItemsHeight <= 0.0f) {
      // Submit the first item (or range) so we can measure its height
      // (generally the first range is 0..1)
      data->Ranges.push_front(ListClipperRange::FromIndices(
          data->ItemsFrozen, data->ItemsFrozen + 1));
      clipper->DisplayStart = Max(data->Ranges[0].Min, data->ItemsFrozen);
      clipper->DisplayEnd = Min(data->Ranges[0].Max, clipper->ItemsCount);
      data->StepNo = 1;
      return true;
    }
    calc_clipping = true; // If on the first step with known item height,
                          // calculate clipping.
  }

  // Step 1: Let the clipper infer height from first range
  if (clipper->ItemsHeight <= 0.0f) {
    ASSERT(data->StepNo == 1);
    if (table)
      ASSERT(table->RowPosY1 == clipper->StartPosY &&
             table->RowPosY2 == window->DC.CursorPos.y);

    clipper->ItemsHeight = (window->DC.CursorPos.y - clipper->StartPosY) /
                           (float)(clipper->DisplayEnd - clipper->DisplayStart);
    bool affected_by_floating_point_precision =
        IsFloatAboveGuaranteedIntegerPrecision(clipper->StartPosY) ||
        IsFloatAboveGuaranteedIntegerPrecision(window->DC.CursorPos.y);
    if (affected_by_floating_point_precision)
      clipper->ItemsHeight =
          window->DC.PrevLineSize.y +
          g.Style.ItemSpacing
              .y; // FIXME: Technically wouldn't allow multi-line entries.

    ASSERT(clipper->ItemsHeight > 0.0f &&
           "Unable to calculate item height! First item hasn't moved the "
           "cursor vertically!");
    calc_clipping = true; // If item height had to be calculated, calculate
                          // clipping afterwards.
  }

  // Step 0 or 1: Calculate the actual ranges of visible elements.
  const int already_submitted = clipper->DisplayEnd;
  if (calc_clipping) {
    if (g.LogEnabled) {
      // If logging is active, do not perform any clipping
      data->Ranges.push_back(
          ListClipperRange::FromIndices(0, clipper->ItemsCount));
    } else {
      // Add range selected to be included for navigation
      const bool is_nav_request =
          (g.NavMoveScoringItems && g.NavWindow &&
           g.NavWindow->RootWindowForNav == window->RootWindowForNav);
      if (is_nav_request)
        data->Ranges.push_back(ListClipperRange::FromPositions(
            g.NavScoringNoClipRect.Min.y, g.NavScoringNoClipRect.Max.y, 0, 0));
      if (is_nav_request && (g.NavMoveFlags & NavMoveFlags_IsTabbing) &&
          g.NavTabbingDir == -1)
        data->Ranges.push_back(ListClipperRange::FromIndices(
            clipper->ItemsCount - 1, clipper->ItemsCount));

      // Add focused/active item
      Rect nav_rect_abs =
          Gui::WindowRectRelToAbs(window, window->NavRectRel[0]);
      if (g.NavId != 0 && window->NavLastIds[0] == g.NavId)
        data->Ranges.push_back(ListClipperRange::FromPositions(
            nav_rect_abs.Min.y, nav_rect_abs.Max.y, 0, 0));

      // Add visible range
      const int off_min =
          (is_nav_request && g.NavMoveClipDir == Dir_Up) ? -1 : 0;
      const int off_max =
          (is_nav_request && g.NavMoveClipDir == Dir_Down) ? 1 : 0;
      data->Ranges.push_back(ListClipperRange::FromPositions(
          window->ClipRect.Min.y, window->ClipRect.Max.y, off_min, off_max));
    }

    // Convert position ranges to item index ranges
    // - Very important: when a starting position is after our maximum item, we
    // set Min to (ItemsCount - 1). This allows us to handle most forms of
    // wrapping.
    // - Due to how Selectable extra padding they tend to be "unaligned" with
    // exact unit in the item list,
    //   which with the flooring/ceiling tend to lead to 2 items instead of one
    //   being submitted.
    for (ListClipperRange &range : data->Ranges)
      if (range.PosToIndexConvert) {
        int m1 = (int)(((double)range.Min - window->DC.CursorPos.y -
                        data->LossynessOffset) /
                       clipper->ItemsHeight);
        int m2 = (int)((((double)range.Max - window->DC.CursorPos.y -
                         data->LossynessOffset) /
                        clipper->ItemsHeight) +
                       0.999999f);
        range.Min = Clamp(already_submitted + m1 + range.PosToIndexOffsetMin,
                          already_submitted, clipper->ItemsCount - 1);
        range.Max = Clamp(already_submitted + m2 + range.PosToIndexOffsetMax,
                          range.Min + 1, clipper->ItemsCount);
        range.PosToIndexConvert = false;
      }
    ListClipper_SortAndFuseRanges(data->Ranges, data->StepNo);
  }

  // Step 0+ (if item height is given in advance) or 1+: Display the next range
  // in line.
  while (data->StepNo < data->Ranges.Size) {
    clipper->DisplayStart =
        Max(data->Ranges[data->StepNo].Min, already_submitted);
    clipper->DisplayEnd =
        Min(data->Ranges[data->StepNo].Max, clipper->ItemsCount);
    if (clipper->DisplayStart > already_submitted) //-V1051
      ListClipper_SeekCursorForItem(clipper, clipper->DisplayStart);
    data->StepNo++;
    if (clipper->DisplayStart == clipper->DisplayEnd &&
        data->StepNo < data->Ranges.Size)
      continue;
    return true;
  }

  // After the last step: Let the clipper validate that we have reached the
  // expected Y position (corresponding to element DisplayEnd), Advance the
  // cursor to the end of the list and then returns 'false' to end the loop.
  if (clipper->ItemsCount < INT_MAX)
    ListClipper_SeekCursorForItem(clipper, clipper->ItemsCount);

  return false;
}

bool ListClipper::Step() {
  Context &g = *Ctx;
  bool need_items_height = (ItemsHeight <= 0.0f);
  bool ret = ListClipper_StepInternal(this);
  if (ret && (DisplayStart == DisplayEnd))
    ret = false;
  if (g.CurrentTable && g.CurrentTable->IsUnfrozenRows == false)
    DEBUG_LOG_CLIPPER("Clipper: Step(): inside frozen table row.\n");
  if (need_items_height && ItemsHeight > 0.0f)
    DEBUG_LOG_CLIPPER("Clipper: Step(): computed ItemsHeight: %.2f.\n",
                      ItemsHeight);
  if (ret) {
    DEBUG_LOG_CLIPPER("Clipper: Step(): display %d to %d.\n", DisplayStart,
                      DisplayEnd);
  } else {
    DEBUG_LOG_CLIPPER("Clipper: Step(): End.\n");
    End();
  }
  return ret;
}

//-----------------------------------------------------------------------------
// [SECTION] STYLING
//-----------------------------------------------------------------------------

Style &Gui::GetStyle() {
  ASSERT(GGui != NULL &&
         "No current context. Did you call Gui::CreateContext() and "
         "Gui::SetCurrentContext() ?");
  return GGui->Style;
}

U32 Gui::GetColorU32(Col idx, float alpha_mul) {
  Style &style = GGui->Style;
  Vec4 c = style.Colors[idx];
  c.w *= style.Alpha * alpha_mul;
  return ColorConvertFloat4ToU32(c);
}

U32 Gui::GetColorU32(const Vec4 &col) {
  Style &style = GGui->Style;
  Vec4 c = col;
  c.w *= style.Alpha;
  return ColorConvertFloat4ToU32(c);
}

const Vec4 &Gui::GetStyleColorVec4(Col idx) {
  Style &style = GGui->Style;
  return style.Colors[idx];
}

U32 Gui::GetColorU32(U32 col) {
  Style &style = GGui->Style;
  if (style.Alpha >= 1.0f)
    return col;
  U32 a = (col & COL32_A_MASK) >> COL32_A_SHIFT;
  a = (U32)(a * style.Alpha); // We don't need to clamp 0..255 because
                              // Style.Alpha is in 0..1 range.
  return (col & ~COL32_A_MASK) | (a << COL32_A_SHIFT);
}

// FIXME: This may incur a round-trip (if the end user got their data from a
// float4) but eventually we aim to store the in-flight colors as U32
void Gui::PushStyleColor(Col idx, U32 col) {
  Context &g = *GGui;
  ColorMod backup;
  backup.Col = idx;
  backup.BackupValue = g.Style.Colors[idx];
  g.ColorStack.push_back(backup);
  if (g.DebugFlashStyleColorIdx != idx)
    g.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void Gui::PushStyleColor(Col idx, const Vec4 &col) {
  Context &g = *GGui;
  ColorMod backup;
  backup.Col = idx;
  backup.BackupValue = g.Style.Colors[idx];
  g.ColorStack.push_back(backup);
  if (g.DebugFlashStyleColorIdx != idx)
    g.Style.Colors[idx] = col;
}

void Gui::PopStyleColor(int count) {
  Context &g = *GGui;
  if (g.ColorStack.Size < count) {
    ASSERT_USER_ERROR(
        g.ColorStack.Size > count,
        "Calling PopStyleColor() too many times: stack underflow.");
    count = g.ColorStack.Size;
  }
  while (count > 0) {
    ColorMod &backup = g.ColorStack.back();
    g.Style.Colors[backup.Col] = backup.BackupValue;
    g.ColorStack.pop_back();
    count--;
  }
}

static const Col GWindowDockStyleColors[WindowDockStyleCol_COUNT] = {
    Col_Text,      Col_Tab,          Col_TabHovered,
    Col_TabActive, Col_TabUnfocused, Col_TabUnfocusedActive};

static const DataVarInfo GStyleVarInfo[] = {
    {DataType_Float, 1, (U32)offsetof(Style, Alpha)}, // StyleVar_Alpha
    {DataType_Float, 1,
     (U32)offsetof(Style, DisabledAlpha)}, // StyleVar_DisabledAlpha
    {DataType_Float, 2,
     (U32)offsetof(Style, WindowPadding)}, // StyleVar_WindowPadding
    {DataType_Float, 1,
     (U32)offsetof(Style, WindowRounding)}, // StyleVar_WindowRounding
    {DataType_Float, 1,
     (U32)offsetof(Style, WindowBorderSize)}, // StyleVar_WindowBorderSize
    {DataType_Float, 2,
     (U32)offsetof(Style, WindowMinSize)}, // StyleVar_WindowMinSize
    {DataType_Float, 2,
     (U32)offsetof(Style, WindowTitleAlign)}, // StyleVar_WindowTitleAlign
    {DataType_Float, 1,
     (U32)offsetof(Style, ChildRounding)}, // StyleVar_ChildRounding
    {DataType_Float, 1,
     (U32)offsetof(Style, ChildBorderSize)}, // StyleVar_ChildBorderSize
    {DataType_Float, 1,
     (U32)offsetof(Style, PopupRounding)}, // StyleVar_PopupRounding
    {DataType_Float, 1,
     (U32)offsetof(Style, PopupBorderSize)}, // StyleVar_PopupBorderSize
    {DataType_Float, 2,
     (U32)offsetof(Style, FramePadding)}, // StyleVar_FramePadding
    {DataType_Float, 1,
     (U32)offsetof(Style, FrameRounding)}, // StyleVar_FrameRounding
    {DataType_Float, 1,
     (U32)offsetof(Style, FrameBorderSize)}, // StyleVar_FrameBorderSize
    {DataType_Float, 2,
     (U32)offsetof(Style, ItemSpacing)}, // StyleVar_ItemSpacing
    {DataType_Float, 2,
     (U32)offsetof(Style, ItemInnerSpacing)}, // StyleVar_ItemInnerSpacing
    {DataType_Float, 1,
     (U32)offsetof(Style, IndentSpacing)}, // StyleVar_IndentSpacing
    {DataType_Float, 2,
     (U32)offsetof(Style, CellPadding)}, // StyleVar_CellPadding
    {DataType_Float, 1,
     (U32)offsetof(Style, ScrollbarSize)}, // StyleVar_ScrollbarSize
    {DataType_Float, 1,
     (U32)offsetof(Style, ScrollbarRounding)}, // StyleVar_ScrollbarRounding
    {DataType_Float, 1,
     (U32)offsetof(Style, GrabMinSize)}, // StyleVar_GrabMinSize
    {DataType_Float, 1,
     (U32)offsetof(Style, GrabRounding)}, // StyleVar_GrabRounding
    {DataType_Float, 1,
     (U32)offsetof(Style, TabRounding)}, // StyleVar_TabRounding
    {DataType_Float, 1,
     (U32)offsetof(Style, TabBarBorderSize)}, // StyleVar_TabBarBorderSize
    {DataType_Float, 2,
     (U32)offsetof(Style, ButtonTextAlign)}, // StyleVar_ButtonTextAlign
    {DataType_Float, 2,
     (U32)offsetof(Style, SelectableTextAlign)}, // StyleVar_SelectableTextAlign
    {DataType_Float, 1,
     (U32)offsetof(
         Style, SeparatorTextBorderSize)}, // StyleVar_SeparatorTextBorderSize
    {DataType_Float, 2,
     (U32)offsetof(Style, SeparatorTextAlign)}, // StyleVar_SeparatorTextAlign
    {DataType_Float, 2,
     (U32)offsetof(Style,
                   SeparatorTextPadding)}, // StyleVar_SeparatorTextPadding
    {DataType_Float, 1,
     (U32)offsetof(Style,
                   DockingSeparatorSize)}, // StyleVar_DockingSeparatorSize
};

const DataVarInfo *Gui::GetStyleVarInfo(StyleVar idx) {
  ASSERT(idx >= 0 && idx < StyleVar_COUNT);
  STATIC_ASSERT(ARRAYSIZE(GStyleVarInfo) == StyleVar_COUNT);
  return &GStyleVarInfo[idx];
}

void Gui::PushStyleVar(StyleVar idx, float val) {
  Context &g = *GGui;
  const DataVarInfo *var_info = GetStyleVarInfo(idx);
  if (var_info->Type == DataType_Float && var_info->Count == 1) {
    float *pvar = (float *)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(StyleMod(idx, *pvar));
    *pvar = val;
    return;
  }
  ASSERT_USER_ERROR(0, "Called PushStyleVar() variant with wrong type!");
}

void Gui::PushStyleVar(StyleVar idx, const Vec2 &val) {
  Context &g = *GGui;
  const DataVarInfo *var_info = GetStyleVarInfo(idx);
  if (var_info->Type == DataType_Float && var_info->Count == 2) {
    Vec2 *pvar = (Vec2 *)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(StyleMod(idx, *pvar));
    *pvar = val;
    return;
  }
  ASSERT_USER_ERROR(0, "Called PushStyleVar() variant with wrong type!");
}

void Gui::PopStyleVar(int count) {
  Context &g = *GGui;
  if (g.StyleVarStack.Size < count) {
    ASSERT_USER_ERROR(g.StyleVarStack.Size > count,
                      "Calling PopStyleVar() too many times: stack underflow.");
    count = g.StyleVarStack.Size;
  }
  while (count > 0) {
    // We avoid a generic memcpy(data, &backup.Backup..,
    // GDataTypeSize[info->Type] * info->Count), the overhead in Debug is not
    // worth it.
    StyleMod &backup = g.StyleVarStack.back();
    const DataVarInfo *info = GetStyleVarInfo(backup.VarIdx);
    void *data = info->GetVarPtr(&g.Style);
    if (info->Type == DataType_Float && info->Count == 1) {
      ((float *)data)[0] = backup.BackupFloat[0];
    } else if (info->Type == DataType_Float && info->Count == 2) {
      ((float *)data)[0] = backup.BackupFloat[0];
      ((float *)data)[1] = backup.BackupFloat[1];
    }
    g.StyleVarStack.pop_back();
    count--;
  }
}

const char *Gui::GetStyleColorName(Col idx) {
  // Create switch-case from enum with regexp: Col_{.*}, --> case
  // Col_\1: return "\1";
  switch (idx) {
  case Col_Text:
    return "Text";
  case Col_TextDisabled:
    return "TextDisabled";
  case Col_WindowBg:
    return "WindowBg";
  case Col_ChildBg:
    return "ChildBg";
  case Col_PopupBg:
    return "PopupBg";
  case Col_Border:
    return "Border";
  case Col_BorderShadow:
    return "BorderShadow";
  case Col_FrameBg:
    return "FrameBg";
  case Col_FrameBgHovered:
    return "FrameBgHovered";
  case Col_FrameBgActive:
    return "FrameBgActive";
  case Col_TitleBg:
    return "TitleBg";
  case Col_TitleBgActive:
    return "TitleBgActive";
  case Col_TitleBgCollapsed:
    return "TitleBgCollapsed";
  case Col_MenuBarBg:
    return "MenuBarBg";
  case Col_ScrollbarBg:
    return "ScrollbarBg";
  case Col_ScrollbarGrab:
    return "ScrollbarGrab";
  case Col_ScrollbarGrabHovered:
    return "ScrollbarGrabHovered";
  case Col_ScrollbarGrabActive:
    return "ScrollbarGrabActive";
  case Col_CheckMark:
    return "CheckMark";
  case Col_SliderGrab:
    return "SliderGrab";
  case Col_SliderGrabActive:
    return "SliderGrabActive";
  case Col_Button:
    return "Button";
  case Col_ButtonHovered:
    return "ButtonHovered";
  case Col_ButtonActive:
    return "ButtonActive";
  case Col_Header:
    return "Header";
  case Col_HeaderHovered:
    return "HeaderHovered";
  case Col_HeaderActive:
    return "HeaderActive";
  case Col_Separator:
    return "Separator";
  case Col_SeparatorHovered:
    return "SeparatorHovered";
  case Col_SeparatorActive:
    return "SeparatorActive";
  case Col_ResizeGrip:
    return "ResizeGrip";
  case Col_ResizeGripHovered:
    return "ResizeGripHovered";
  case Col_ResizeGripActive:
    return "ResizeGripActive";
  case Col_Tab:
    return "Tab";
  case Col_TabHovered:
    return "TabHovered";
  case Col_TabActive:
    return "TabActive";
  case Col_TabUnfocused:
    return "TabUnfocused";
  case Col_TabUnfocusedActive:
    return "TabUnfocusedActive";
  case Col_DockingPreview:
    return "DockingPreview";
  case Col_DockingEmptyBg:
    return "DockingEmptyBg";
  case Col_PlotLines:
    return "PlotLines";
  case Col_PlotLinesHovered:
    return "PlotLinesHovered";
  case Col_PlotHistogram:
    return "PlotHistogram";
  case Col_PlotHistogramHovered:
    return "PlotHistogramHovered";
  case Col_TableHeaderBg:
    return "TableHeaderBg";
  case Col_TableBorderStrong:
    return "TableBorderStrong";
  case Col_TableBorderLight:
    return "TableBorderLight";
  case Col_TableRowBg:
    return "TableRowBg";
  case Col_TableRowBgAlt:
    return "TableRowBgAlt";
  case Col_TextSelectedBg:
    return "TextSelectedBg";
  case Col_DragDropTarget:
    return "DragDropTarget";
  case Col_NavHighlight:
    return "NavHighlight";
  case Col_NavWindowingHighlight:
    return "NavWindowingHighlight";
  case Col_NavWindowingDimBg:
    return "NavWindowingDimBg";
  case Col_ModalWindowDimBg:
    return "ModalWindowDimBg";
  }
  ASSERT(0);
  return "Unknown";
}

//-----------------------------------------------------------------------------
// [SECTION] RENDER HELPERS
// Some of those (internal) functions are currently quite a legacy mess - their
// signature and behavior will change, we need a nicer separation between
// low-level functions and high-level functions relying on the Gui context.
// Also see draw.cpp for some more which have been reworked to not rely on
// Gui:: context.
//-----------------------------------------------------------------------------

const char *Gui::FindRenderedTextEnd(const char *text, const char *text_end) {
  const char *text_display_end = text;
  if (!text_end)
    text_end = (const char *)-1;

  while (text_display_end < text_end && *text_display_end != '\0' &&
         (text_display_end[0] != '#' || text_display_end[1] != '#'))
    text_display_end++;
  return text_display_end;
}

// Internal Gui functions to render text
// RenderText***() functions calls DrawList::AddText() calls
// BitmapFont::RenderText()
void Gui::RenderText(Vec2 pos, const char *text, const char *text_end,
                     bool hide_text_after_hash) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  // Hide anything after a '##' string
  const char *text_display_end;
  if (hide_text_after_hash) {
    text_display_end = FindRenderedTextEnd(text, text_end);
  } else {
    if (!text_end)
      text_end = text + strlen(text); // FIXME-OPT
    text_display_end = text_end;
  }

  if (text != text_display_end) {
    window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(Col_Text),
                              text, text_display_end);
    if (g.LogEnabled)
      LogRenderedText(&pos, text, text_display_end);
  }
}

void Gui::RenderTextWrapped(Vec2 pos, const char *text, const char *text_end,
                            float wrap_width) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  if (!text_end)
    text_end = text + strlen(text); // FIXME-OPT

  if (text != text_end) {
    window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(Col_Text),
                              text, text_end, wrap_width);
    if (g.LogEnabled)
      LogRenderedText(&pos, text, text_end);
  }
}

// Default clip_rect uses (pos_min,pos_max)
// Handle clipping on CPU immediately (vs typically let the GPU clip the
// triangles that are overlapping the clipping rectangle edges)
// FIXME-OPT: Since we have or calculate text_size we could coarse clip whole
// block immediately, especally for text above draw_list->DrawList. Effectively
// as this is called from widget doing their own coarse clipping it's not very
// valuable presently. Next time function will take better advantage of the
// render function taking size into account for coarse clipping.
void Gui::RenderTextClippedEx(DrawList *draw_list, const Vec2 &pos_min,
                              const Vec2 &pos_max, const char *text,
                              const char *text_display_end,
                              const Vec2 *text_size_if_known, const Vec2 &align,
                              const Rect *clip_rect) {
  // Perform CPU side clipping for single clipped element to avoid using scissor
  // state
  Vec2 pos = pos_min;
  const Vec2 text_size =
      text_size_if_known ? *text_size_if_known
                         : CalcTextSize(text, text_display_end, false, 0.0f);

  const Vec2 *clip_min = clip_rect ? &clip_rect->Min : &pos_min;
  const Vec2 *clip_max = clip_rect ? &clip_rect->Max : &pos_max;
  bool need_clipping = (pos.x + text_size.x >= clip_max->x) ||
                       (pos.y + text_size.y >= clip_max->y);
  if (clip_rect) // If we had no explicit clipping rectangle then pos==clip_min
    need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

  // Align whole block. We should defer that to the better rendering function
  // when we'll have support for individual line alignment.
  if (align.x > 0.0f)
    pos.x = Max(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
  if (align.y > 0.0f)
    pos.y = Max(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

  // Render
  if (need_clipping) {
    Vec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
    draw_list->AddText(NULL, 0.0f, pos, GetColorU32(Col_Text), text,
                       text_display_end, 0.0f, &fine_clip_rect);
  } else {
    draw_list->AddText(NULL, 0.0f, pos, GetColorU32(Col_Text), text,
                       text_display_end, 0.0f, NULL);
  }
}

void Gui::RenderTextClipped(const Vec2 &pos_min, const Vec2 &pos_max,
                            const char *text, const char *text_end,
                            const Vec2 *text_size_if_known, const Vec2 &align,
                            const Rect *clip_rect) {
  // Hide anything after a '##' string
  const char *text_display_end = FindRenderedTextEnd(text, text_end);
  const int text_len = (int)(text_display_end - text);
  if (text_len == 0)
    return;

  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  RenderTextClippedEx(window->DrawList, pos_min, pos_max, text,
                      text_display_end, text_size_if_known, align, clip_rect);
  if (g.LogEnabled)
    LogRenderedText(&pos_min, text, text_display_end);
}

// Another overly complex function until we reorganize everything into a nice
// all-in-one helper. This is made more complex because we have dissociated the
// layout rectangle (pos_min..pos_max) which define _where_ the ellipsis is,
// from actual clipping of text and limit of the ellipsis display. This is
// because in the context of tabs we selectively hide part of the text when the
// Close Button appears, but we don't want the ellipsis to move.
void Gui::RenderTextEllipsis(DrawList *draw_list, const Vec2 &pos_min,
                             const Vec2 &pos_max, float clip_max_x,
                             float ellipsis_max_x, const char *text,
                             const char *text_end_full,
                             const Vec2 *text_size_if_known) {
  Context &g = *GGui;
  if (text_end_full == NULL)
    text_end_full = FindRenderedTextEnd(text);
  const Vec2 text_size = text_size_if_known
                             ? *text_size_if_known
                             : CalcTextSize(text, text_end_full, false, 0.0f);

  // draw_list->AddLine(Vec2(pos_max.x, pos_min.y - 4), Vec2(pos_max.x,
  // pos_max.y + 4), COL32(0, 0, 255, 255));
  // draw_list->AddLine(Vec2(ellipsis_max_x, pos_min.y-2),
  // Vec2(ellipsis_max_x, pos_max.y+2), COL32(0, 255, 0, 255));
  // draw_list->AddLine(Vec2(clip_max_x, pos_min.y), Vec2(clip_max_x,
  // pos_max.y), COL32(255, 0, 0, 255));
  //  FIXME: We could technically remove (last_glyph->AdvanceX - last_glyph->X1)
  //  from text_size.x here and save a few pixels.
  if (text_size.x > pos_max.x - pos_min.x) {
    // Hello wo...
    // |       |   |
    // min   max   ellipsis_max
    //          <-> this is generally some padding value

    const Font *font = draw_list->_Data->Font;
    const float font_size = draw_list->_Data->FontSize;
    const float font_scale = font_size / font->FontSize;
    const char *text_end_ellipsis = NULL;
    const float ellipsis_width = font->EllipsisWidth * font_scale;

    // We can now claim the space between pos_max.x and ellipsis_max.x
    const float text_avail_width = Max(
        (Max(pos_max.x, ellipsis_max_x) - ellipsis_width) - pos_min.x, 1.0f);
    float text_size_clipped_x =
        font->CalcTextSizeA(font_size, text_avail_width, 0.0f, text,
                            text_end_full, &text_end_ellipsis)
            .x;
    if (text == text_end_ellipsis && text_end_ellipsis < text_end_full) {
      // Always display at least 1 character if there's no room for character +
      // ellipsis
      text_end_ellipsis =
          text + TextCountUtf8BytesFromChar(text, text_end_full);
      text_size_clipped_x =
          font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text, text_end_ellipsis)
              .x;
    }
    while (text_end_ellipsis > text && CharIsBlankA(text_end_ellipsis[-1])) {
      // Trim trailing space before ellipsis (FIXME: Supporting non-ascii blanks
      // would be nice, for this we need a function to backtrack in UTF-8 text)
      text_end_ellipsis--;
      text_size_clipped_x -=
          font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text_end_ellipsis,
                              text_end_ellipsis + 1)
              .x; // Ascii blanks are always 1 byte
    }

    // Render text, render ellipsis
    RenderTextClippedEx(draw_list, pos_min, Vec2(clip_max_x, pos_max.y), text,
                        text_end_ellipsis, &text_size, Vec2(0.0f, 0.0f));
    Vec2 ellipsis_pos = Trunc(Vec2(pos_min.x + text_size_clipped_x, pos_min.y));
    if (ellipsis_pos.x + ellipsis_width <= ellipsis_max_x)
      for (int i = 0; i < font->EllipsisCharCount;
           i++, ellipsis_pos.x += font->EllipsisCharStep * font_scale)
        font->RenderChar(draw_list, font_size, ellipsis_pos,
                         GetColorU32(Col_Text), font->EllipsisChar);
  } else {
    RenderTextClippedEx(draw_list, pos_min, Vec2(clip_max_x, pos_max.y), text,
                        text_end_full, &text_size, Vec2(0.0f, 0.0f));
  }

  if (g.LogEnabled)
    LogRenderedText(&pos_min, text, text_end_full);
}

// Render a rectangle shaped with optional rounding and borders
void Gui::RenderFrame(Vec2 p_min, Vec2 p_max, U32 fill_col, bool border,
                      float rounding) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
  const float border_size = g.Style.FrameBorderSize;
  if (border && border_size > 0.0f) {
    window->DrawList->AddRect(p_min + Vec2(1, 1), p_max + Vec2(1, 1),
                              GetColorU32(Col_BorderShadow), rounding, 0,
                              border_size);
    window->DrawList->AddRect(p_min, p_max, GetColorU32(Col_Border), rounding,
                              0, border_size);
  }
}

void Gui::RenderFrameBorder(Vec2 p_min, Vec2 p_max, float rounding) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  const float border_size = g.Style.FrameBorderSize;
  if (border_size > 0.0f) {
    window->DrawList->AddRect(p_min + Vec2(1, 1), p_max + Vec2(1, 1),
                              GetColorU32(Col_BorderShadow), rounding, 0,
                              border_size);
    window->DrawList->AddRect(p_min, p_max, GetColorU32(Col_Border), rounding,
                              0, border_size);
  }
}

void Gui::RenderNavHighlight(const Rect &bb, ID id, NavHighlightFlags flags) {
  Context &g = *GGui;
  if (id != g.NavId)
    return;
  if (g.NavDisableHighlight && !(flags & NavHighlightFlags_AlwaysDraw))
    return;
  Window *window = g.CurrentWindow;
  if (window->DC.NavHideHighlightOneFrame)
    return;

  float rounding =
      (flags & NavHighlightFlags_NoRounding) ? 0.0f : g.Style.FrameRounding;
  Rect display_rect = bb;
  display_rect.ClipWith(window->ClipRect);
  if (flags & NavHighlightFlags_TypeDefault) {
    const float THICKNESS = 2.0f;
    const float DISTANCE = 3.0f + THICKNESS * 0.5f;
    display_rect.Expand(Vec2(DISTANCE, DISTANCE));
    bool fully_visible = window->ClipRect.Contains(display_rect);
    if (!fully_visible)
      window->DrawList->PushClipRect(display_rect.Min, display_rect.Max);
    window->DrawList->AddRect(
        display_rect.Min + Vec2(THICKNESS * 0.5f, THICKNESS * 0.5f),
        display_rect.Max - Vec2(THICKNESS * 0.5f, THICKNESS * 0.5f),
        GetColorU32(Col_NavHighlight), rounding, 0, THICKNESS);
    if (!fully_visible)
      window->DrawList->PopClipRect();
  }
  if (flags & NavHighlightFlags_TypeThin) {
    window->DrawList->AddRect(display_rect.Min, display_rect.Max,
                              GetColorU32(Col_NavHighlight), rounding, 0, 1.0f);
  }
}

void Gui::RenderMouseCursor(Vec2 base_pos, float base_scale,
                            MouseCursor mouse_cursor, U32 col_fill,
                            U32 col_border, U32 col_shadow) {
  Context &g = *GGui;
  ASSERT(mouse_cursor > MouseCursor_None && mouse_cursor < MouseCursor_COUNT);
  FontAtlas *font_atlas = g.DrawListSharedData.Font->ContainerAtlas;
  for (ViewportP *viewport : g.Viewports) {
    // We scale cursor with current viewport/monitor, however Windows 10 for its
    // own hardware cursor seems to be using a different scale factor.
    Vec2 offset, size, uv[4];
    if (!font_atlas->GetMouseCursorTexData(mouse_cursor, &offset, &size, &uv[0],
                                           &uv[2]))
      continue;
    const Vec2 pos = base_pos - offset;
    const float scale = base_scale * viewport->DpiScale;
    if (!viewport->GetMainRect().Overlaps(
            Rect(pos, pos + Vec2(size.x + 2, size.y + 2) * scale)))
      continue;
    DrawList *draw_list = GetForegroundDrawList(viewport);
    TextureID tex_id = font_atlas->TexID;
    draw_list->PushTextureID(tex_id);
    draw_list->AddImage(tex_id, pos + Vec2(1, 0) * scale,
                        pos + (Vec2(1, 0) + size) * scale, uv[2], uv[3],
                        col_shadow);
    draw_list->AddImage(tex_id, pos + Vec2(2, 0) * scale,
                        pos + (Vec2(2, 0) + size) * scale, uv[2], uv[3],
                        col_shadow);
    draw_list->AddImage(tex_id, pos, pos + size * scale, uv[2], uv[3],
                        col_border);
    draw_list->AddImage(tex_id, pos, pos + size * scale, uv[0], uv[1],
                        col_fill);
    draw_list->PopTextureID();
  }
}

//-----------------------------------------------------------------------------
// [SECTION] INITIALIZATION, SHUTDOWN
//-----------------------------------------------------------------------------

// Internal state access - if you want to share Gui state between modules
// (e.g. DLL) or allocate it yourself Note that we still point to some static
// data and members (such as GFontAtlas), so the state instance you end up using
// will point to the static data within its module
Context *Gui::GetCurrentContext() { return GGui; }

void Gui::SetCurrentContext(Context *ctx) {
#ifdef SET_CURRENT_CONTEXT_FUNC
  SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want
                                 // to have control over this.
#else
  GGui = ctx;
#endif
}

void Gui::SetAllocatorFunctions(MemAllocFunc alloc_func, MemFreeFunc free_func,
                                void *user_data) {
  GAllocatorAllocFunc = alloc_func;
  GAllocatorFreeFunc = free_func;
  GAllocatorUserData = user_data;
}

// This is provided to facilitate copying allocators from one static/DLL
// boundary to another (e.g. retrieve default allocator of your executable
// address space)
void Gui::GetAllocatorFunctions(MemAllocFunc *p_alloc_func,
                                MemFreeFunc *p_free_func, void **p_user_data) {
  *p_alloc_func = GAllocatorAllocFunc;
  *p_free_func = GAllocatorFreeFunc;
  *p_user_data = GAllocatorUserData;
}

Context *Gui::CreateContext(FontAtlas *shared_font_atlas) {
  Context *prev_ctx = GetCurrentContext();
  Context *ctx = NEW(Context)(shared_font_atlas);
  SetCurrentContext(ctx);
  Initialize();
  if (prev_ctx != NULL)
    SetCurrentContext(
        prev_ctx); // Restore previous context if any, else keep new one.
  return ctx;
}

void Gui::DestroyContext(Context *ctx) {
  Context *prev_ctx = GetCurrentContext();
  if (ctx == NULL) //-V1051
    ctx = prev_ctx;
  SetCurrentContext(ctx);
  Shutdown();
  SetCurrentContext((prev_ctx != ctx) ? prev_ctx : NULL);
  DELETE(ctx);
}

// IMPORTANT: ###xxx suffixes must be same in ALL languages
static const LocEntry GLocalizationEntriesEnUS[] = {
    {LocKey_VersionStr, "Gui " VERSION " (" STRINGIFY(VERSION_NUM) ")"},
    {LocKey_TableSizeOne, "Size column to fit###SizeOne"},
    {LocKey_TableSizeAllFit, "Size all columns to fit###SizeAll"},
    {LocKey_TableSizeAllDefault, "Size all columns to default###SizeAll"},
    {LocKey_TableResetOrder, "Reset order###ResetOrder"},
    {LocKey_WindowingMainMenuBar, "(Main menu bar)"},
    {LocKey_WindowingPopup, "(Popup)"},
    {LocKey_WindowingUntitled, "(Untitled)"},
    {LocKey_DockingHideTabBar, "Hide tab bar###HideTabBar"},
    {LocKey_DockingHoldShiftToDock, "Hold SHIFT to enable Docking window."},
    {LocKey_DockingDragToUndockOrMoveNode,
     "Click and drag to move or undock whole node."},
};

void Gui::Initialize() {
  Context &g = *GGui;
  ASSERT(!g.Initialized && !g.SettingsLoaded);

  // Add .ini handle for Window and Table types
  {
    SettingsHandler ini_handler;
    ini_handler.TypeName = "Window";
    ini_handler.TypeHash = HashStr("Window");
    ini_handler.ClearAllFn = WindowSettingsHandler_ClearAll;
    ini_handler.ReadOpenFn = WindowSettingsHandler_ReadOpen;
    ini_handler.ReadLineFn = WindowSettingsHandler_ReadLine;
    ini_handler.ApplyAllFn = WindowSettingsHandler_ApplyAll;
    ini_handler.WriteAllFn = WindowSettingsHandler_WriteAll;
    AddSettingsHandler(&ini_handler);
  }
  TableSettingsAddSettingsHandler();

  // Setup default localization table
  LocalizeRegisterEntries(GLocalizationEntriesEnUS,
                          ARRAYSIZE(GLocalizationEntriesEnUS));

  // Setup default platform clipboard/IME handlers.
  g.IO.GetClipboardTextFn =
      GetClipboardTextFn_DefaultImpl; // Platform dependent default
                                      // implementations
  g.IO.SetClipboardTextFn = SetClipboardTextFn_DefaultImpl;
  g.IO.ClipboardUserData =
      (void *)&g; // Default implementation use the Context as user data
                  // (ideally those would be arguments to the function)
  g.IO.SetPlatformImeDataFn = SetPlatformImeDataFn_DefaultImpl;

  // Create default viewport
  ViewportP *viewport = NEW(ViewportP)();
  viewport->ID = VIEWPORT_DEFAULT_ID;
  viewport->Idx = 0;
  viewport->PlatformWindowCreated = true;
  viewport->Flags = ViewportFlags_OwnedByApp;
  g.Viewports.push_back(viewport);
  g.TempBuffer.resize(1024 * 3 + 1, 0);
  g.ViewportCreatedCount++;
  g.PlatformIO.Viewports.push_back(g.Viewports[0]);

#ifdef HAS_DOCK
  // Initialize Docking
  DockContextInitialize(&g);
#endif

  g.Initialized = true;
}

// This function is merely here to free heap allocations.
void Gui::Shutdown() {
  Context &g = *GGui;
  ASSERT_USER_ERROR(g.IO.BackendPlatformUserData == NULL,
                    "Forgot to shutdown Platform backend?");
  ASSERT_USER_ERROR(g.IO.BackendRendererUserData == NULL,
                    "Forgot to shutdown Renderer backend?");

  // The fonts atlas can be used prior to calling NewFrame(), so we clear it
  // even if g.Initialized is FALSE (which would happen if we never called
  // NewFrame)
  if (g.IO.Fonts && g.FontAtlasOwnedByContext) {
    g.IO.Fonts->Locked = false;
    DELETE(g.IO.Fonts);
  }
  g.IO.Fonts = NULL;
  g.DrawListSharedData.TempBuffer.clear();

  // Cleanup of other data are conditional on actually having initialized
  // Gui.
  if (!g.Initialized)
    return;

  // Save settings (unless we haven't attempted to load them:
  // CreateContext/DestroyContext without a call to NewFrame shouldn't save an
  // empty file)
  if (g.SettingsLoaded && g.IO.IniFilename != NULL)
    SaveIniSettingsToDisk(g.IO.IniFilename);

  // Destroy platform windows
  DestroyPlatformWindows();

  // Shutdown extensions
  DockContextShutdown(&g);

  CallContextHooks(&g, ContextHookType_Shutdown);

  // Clear everything else
  g.Windows.clear_delete();
  g.WindowsFocusOrder.clear();
  g.WindowsTempSortBuffer.clear();
  g.CurrentWindow = NULL;
  g.CurrentWindowStack.clear();
  g.WindowsById.Clear();
  g.NavWindow = NULL;
  g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;
  g.ActiveIdWindow = g.ActiveIdPreviousFrameWindow = NULL;
  g.MovingWindow = NULL;

  g.KeysRoutingTable.Clear();

  g.ColorStack.clear();
  g.StyleVarStack.clear();
  g.FontStack.clear();
  g.OpenPopupStack.clear();
  g.BeginPopupStack.clear();
  g.NavTreeNodeStack.clear();

  g.CurrentViewport = g.MouseViewport = g.MouseLastHoveredViewport = NULL;
  g.Viewports.clear_delete();

  g.TabBars.Clear();
  g.CurrentTabBarStack.clear();
  g.ShrinkWidthBuffer.clear();

  g.ClipperTempData.clear_destruct();

  g.Tables.Clear();
  g.TablesTempData.clear_destruct();
  g.DrawChannelsTempMergeBuffer.clear();

  g.ClipboardHandlerData.clear();
  g.MenusIdSubmittedThisFrame.clear();
  g.InputTextState.ClearFreeMemory();
  g.InputTextDeactivatedState.ClearFreeMemory();

  g.SettingsWindows.clear();
  g.SettingsHandlers.clear();

  if (g.LogFile) {
#ifndef DISABLE_TTY_FUNCTIONS
    if (g.LogFile != stdout)
#endif
      FileClose(g.LogFile);
    g.LogFile = NULL;
  }
  g.LogBuffer.clear();
  g.DebugLogBuf.clear();
  g.DebugLogIndex.clear();

  g.Initialized = false;
}

// No specific ordering/dependency support, will see as needed
ID Gui::AddContextHook(Context *ctx, const ContextHook *hook) {
  Context &g = *ctx;
  ASSERT(hook->Callback != NULL && hook->HookId == 0 &&
         hook->Type != ContextHookType_PendingRemoval_);
  g.Hooks.push_back(*hook);
  g.Hooks.back().HookId = ++g.HookIdNext;
  return g.HookIdNext;
}

// Deferred removal, avoiding issue with changing vector while iterating it
void Gui::RemoveContextHook(Context *ctx, ID hook_id) {
  Context &g = *ctx;
  ASSERT(hook_id != 0);
  for (ContextHook &hook : g.Hooks)
    if (hook.HookId == hook_id)
      hook.Type = ContextHookType_PendingRemoval_;
}

// Call context hooks (used by e.g. test engine)
// We assume a small number of hooks so all stored in same array
void Gui::CallContextHooks(Context *ctx, ContextHookType hook_type) {
  Context &g = *ctx;
  for (ContextHook &hook : g.Hooks)
    if (hook.Type == hook_type)
      hook.Callback(&g, &hook);
}

//-----------------------------------------------------------------------------
// [SECTION] MAIN CODE (most of the code! lots of stuff, needs tidying up!)
//-----------------------------------------------------------------------------

// Window is mostly a dumb struct. It merely has a constructor and a few
// helper methods
Window::Window(Context *ctx, const char *name) : DrawListInst(NULL) {
  memset(this, 0, sizeof(*this));
  Ctx = ctx;
  Name = Strdup(name);
  NameBufLen = (int)strlen(name) + 1;
  ID = HashStr(name);
  IDStack.push_back(ID);
  ViewportAllowPlatformMonitorExtend = -1;
  ViewportPos = Vec2(FLT_MAX, FLT_MAX);
  MoveId = GetID("#MOVE");
  TabId = GetID("#TAB");
  ScrollTarget = Vec2(FLT_MAX, FLT_MAX);
  ScrollTargetCenterRatio = Vec2(0.5f, 0.5f);
  AutoFitFramesX = AutoFitFramesY = -1;
  AutoPosLastDirection = Dir_None;
  SetWindowPosAllowFlags = SetWindowSizeAllowFlags =
      SetWindowCollapsedAllowFlags = SetWindowDockAllowFlags = 0;
  SetWindowPosVal = SetWindowPosPivot = Vec2(FLT_MAX, FLT_MAX);
  LastFrameActive = -1;
  LastFrameJustFocused = -1;
  LastTimeActive = -1.0f;
  FontWindowScale = FontDpiScale = 1.0f;
  SettingsOffset = -1;
  DockOrder = -1;
  DrawList = &DrawListInst;
  DrawList->_Data = &Ctx->DrawListSharedData;
  DrawList->_OwnerName = Name;
  NavPreferredScoringPosRel[0] = NavPreferredScoringPosRel[1] =
      Vec2(FLT_MAX, FLT_MAX);
  PLACEMENT_NEW(&WindowClass) WindowClass();
}

Window::~Window() {
  ASSERT(DrawList == &DrawListInst);
  DELETE(Name);
  ColumnsStorage.clear_destruct();
}

ID Window::GetID(const char *str, const char *str_end) {
  ID seed = IDStack.back();
  ID id = HashStr(str, str_end ? (str_end - str) : 0, seed);
  Context &g = *Ctx;
  if (g.DebugHookIdInfo == id)
    Gui::DebugHookIdInfo(id, DataType_String, str, str_end);
  return id;
}

ID Window::GetID(const void *ptr) {
  ID seed = IDStack.back();
  ID id = HashData(&ptr, sizeof(void *), seed);
  Context &g = *Ctx;
  if (g.DebugHookIdInfo == id)
    Gui::DebugHookIdInfo(id, DataType_Pointer, ptr, NULL);
  return id;
}

ID Window::GetID(int n) {
  ID seed = IDStack.back();
  ID id = HashData(&n, sizeof(n), seed);
  Context &g = *Ctx;
  if (g.DebugHookIdInfo == id)
    Gui::DebugHookIdInfo(id, DataType_S32, (void *)(intptr_t)n, NULL);
  return id;
}

// This is only used in rare/specific situations to manufacture an ID out of
// nowhere.
ID Window::GetIDFromRectangle(const Rect &r_abs) {
  ID seed = IDStack.back();
  Rect r_rel = Gui::WindowRectAbsToRel(this, r_abs);
  ID id = HashData(&r_rel, sizeof(r_rel), seed);
  return id;
}

static void SetCurrentWindow(Window *window) {
  Context &g = *GGui;
  g.CurrentWindow = window;
  g.CurrentTable = window && window->DC.CurrentTableIdx != -1
                       ? g.Tables.GetByIndex(window->DC.CurrentTableIdx)
                       : NULL;
  if (window) {
    g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
    Gui::NavUpdateCurrentWindowIsScrollPushableX();
  }
}

void Gui::GcCompactTransientMiscBuffers() {
  Context &g = *GGui;
  g.ItemFlagsStack.clear();
  g.GroupStack.clear();
  TableGcCompactSettings();
}

// Free up/compact internal window buffers, we can use this when a window
// becomes unused. Not freed:
// - Window, WindowSettings, Name, StateStorage, ColumnsStorage (may
// hold useful data) This should have no noticeable visual effect. When the
// window reappear however, expect new allocation/buffer growth/copy cost.
void Gui::GcCompactTransientWindowBuffers(Window *window) {
  window->MemoryCompacted = true;
  window->MemoryDrawListIdxCapacity = window->DrawList->IdxBuffer.Capacity;
  window->MemoryDrawListVtxCapacity = window->DrawList->VtxBuffer.Capacity;
  window->IDStack.clear();
  window->DrawList->_ClearFreeMemory();
  window->DC.ChildWindows.clear();
  window->DC.ItemWidthStack.clear();
  window->DC.TextWrapPosStack.clear();
}

void Gui::GcAwakeTransientWindowBuffers(Window *window) {
  // We stored capacity of the DrawList buffer to reduce growth-caused
  // allocation/copy when awakening. The other buffers tends to amortize much
  // faster.
  window->MemoryCompacted = false;
  window->DrawList->IdxBuffer.reserve(window->MemoryDrawListIdxCapacity);
  window->DrawList->VtxBuffer.reserve(window->MemoryDrawListVtxCapacity);
  window->MemoryDrawListIdxCapacity = window->MemoryDrawListVtxCapacity = 0;
}

void Gui::SetActiveID(ID id, Window *window) {
  Context &g = *GGui;

  // Clear previous active id
  if (g.ActiveId != 0) {
    // While most behaved code would make an effort to not steal active id
    // during window move/drag operations, we at least need to be resilient to
    // it. Canceling the move is rather aggressive and users of 'master' branch
    // may prefer the weird ill-defined half working situation ('docking' did
    // assert), so may need to rework that.
    if (g.MovingWindow != NULL && g.ActiveId == g.MovingWindow->MoveId) {
      DEBUG_LOG_ACTIVEID("SetActiveID() cancel MovingWindow\n");
      g.MovingWindow = NULL;
    }

    // This could be written in a more general way (e.g associate a hook to
    // ActiveId), but since this is currently quite an exception we'll leave it
    // as is. One common scenario leading to this is: pressing Key
    // ->NavMoveRequestApplyResult() -> ClearActiveId()
    if (g.InputTextState.ID == g.ActiveId)
      InputTextDeactivateHook(g.ActiveId);
  }

  // Set active id
  g.ActiveIdIsJustActivated = (g.ActiveId != id);
  if (g.ActiveIdIsJustActivated) {
    DEBUG_LOG_ACTIVEID("SetActiveID() old:0x%08X (window \"%s\") -> new:0x%08X "
                       "(window \"%s\")\n",
                       g.ActiveId,
                       g.ActiveIdWindow ? g.ActiveIdWindow->Name : "", id,
                       window ? window->Name : "");
    g.ActiveIdTimer = 0.0f;
    g.ActiveIdHasBeenPressedBefore = false;
    g.ActiveIdHasBeenEditedBefore = false;
    g.ActiveIdMouseButton = -1;
    if (id != 0) {
      g.LastActiveId = id;
      g.LastActiveIdTimer = 0.0f;
    }
  }
  g.ActiveId = id;
  g.ActiveIdAllowOverlap = false;
  g.ActiveIdNoClearOnFocusLoss = false;
  g.ActiveIdWindow = window;
  g.ActiveIdHasBeenEditedThisFrame = false;
  if (id) {
    g.ActiveIdIsAlive = id;
    g.ActiveIdSource = (g.NavActivateId == id || g.NavJustMovedToId == id)
                           ? g.NavInputSource
                           : InputSource_Mouse;
    ASSERT(g.ActiveIdSource != InputSource_None);
  }

  // Clear declaration of inputs claimed by the widget
  // (Please note that this is WIP and not all keys/inputs are thoroughly
  // declared by all widgets yet)
  g.ActiveIdUsingNavDirMask = 0x00;
  g.ActiveIdUsingAllKeyboardKeys = false;
#ifndef DISABLE_OBSOLETE_KEYIO
  g.ActiveIdUsingNavInputMask = 0x00;
#endif
}

void Gui::ClearActiveID() {
  SetActiveID(0, NULL); // g.ActiveId = 0;
}

void Gui::SetHoveredID(ID id) {
  Context &g = *GGui;
  g.HoveredId = id;
  g.HoveredIdAllowOverlap = false;
  if (id != 0 && g.HoveredIdPreviousFrame != id)
    g.HoveredIdTimer = g.HoveredIdNotActiveTimer = 0.0f;
}

ID Gui::GetHoveredID() {
  Context &g = *GGui;
  return g.HoveredId ? g.HoveredId : g.HoveredIdPreviousFrame;
}

// This is called by ItemAdd().
// Code not using ItemAdd() may need to call this manually otherwise ActiveId
// will be cleared. In VERSION_NUM < 18717 this was called by GetID().
void Gui::KeepAliveID(ID id) {
  Context &g = *GGui;
  if (g.ActiveId == id)
    g.ActiveIdIsAlive = id;
  if (g.ActiveIdPreviousFrame == id)
    g.ActiveIdPreviousFrameIsAlive = true;
}

void Gui::MarkItemEdited(ID id) {
  // This marking is solely to be able to provide info for
  // IsItemDeactivatedAfterEdit(). ActiveId might have been released by the time
  // we call this (as in the typical press/release button behavior) but still
  // need to fill the data.
  Context &g = *GGui;
  if (g.LockMarkEdited > 0)
    return;
  if (g.ActiveId == id || g.ActiveId == 0) {
    g.ActiveIdHasBeenEditedThisFrame = true;
    g.ActiveIdHasBeenEditedBefore = true;
  }

  // We accept a MarkItemEdited() on drag and drop targets.
  // accept 'ActiveIdPreviousFrame == id' for InputText() returning an edit
  // after it has been taken ActiveId away (#4714)
  ASSERT(g.DragDropActive || g.ActiveId == id || g.ActiveId == 0 ||
         g.ActiveIdPreviousFrame == id);

  // ASSERT(g.CurrentWindow->DC.LastItemId == id);
  g.LastItemData.StatusFlags |= ItemStatusFlags_Edited;
}

bool Gui::IsWindowContentHoverable(Window *window, HoveredFlags flags) {
  // An active popup disable hovering on other windows (apart from its own
  // children)
  // FIXME-OPT: This could be cached/stored within the window.
  Context &g = *GGui;
  if (g.NavWindow)
    if (Window *focused_root_window = g.NavWindow->RootWindowDockTree)
      if (focused_root_window->WasActive &&
          focused_root_window != window->RootWindowDockTree) {
        // For the purpose of those flags we differentiate "standard popup" from
        // "modal popup" NB: The 'else' is important because Modal windows are
        // also Popups.
        bool want_inhibit = false;
        if (focused_root_window->Flags & WindowFlags_Modal)
          want_inhibit = true;
        else if ((focused_root_window->Flags & WindowFlags_Popup) &&
                 !(flags & HoveredFlags_AllowWhenBlockedByPopup))
          want_inhibit = true;

        // Inhibit hover unless the window is within the stack of our
        // modal/popup
        if (want_inhibit)
          if (!IsWindowWithinBeginStackOf(window->RootWindow,
                                          focused_root_window))
            return false;
      }

  // Filter by viewport
  if (window->Viewport != g.MouseViewport)
    if (g.MovingWindow == NULL ||
        window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree)
      return false;

  return true;
}

static inline float CalcDelayFromHoveredFlags(HoveredFlags flags) {
  Context &g = *GGui;
  if (flags & HoveredFlags_DelayNormal)
    return g.Style.HoverDelayNormal;
  if (flags & HoveredFlags_DelayShort)
    return g.Style.HoverDelayShort;
  return 0.0f;
}

static HoveredFlags ApplyHoverFlagsForTooltip(HoveredFlags user_flags,
                                              HoveredFlags shared_flags) {
  // Allow instance flags to override shared flags
  if (user_flags & (HoveredFlags_DelayNone | HoveredFlags_DelayShort |
                    HoveredFlags_DelayNormal))
    shared_flags &= ~(HoveredFlags_DelayNone | HoveredFlags_DelayShort |
                      HoveredFlags_DelayNormal);
  return user_flags | shared_flags;
}

// This is roughly matching the behavior of internal-facing ItemHoverable()
// - we allow hovering to be true when ActiveId==window->MoveID, so that
// clicking on non-interactive items such as a Text() item still returns true
// with IsItemHovered()
// - this should work even for non-interactive items that have no ID, so we
// cannot use LastItemId
bool Gui::IsItemHovered(HoveredFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT((flags & ~HoveredFlags_AllowedMaskForIsItemHovered) == 0 &&
         "Invalid flags for IsItemHovered()!");

  if (g.NavDisableMouseHover && !g.NavDisableHighlight &&
      !(flags & HoveredFlags_NoNavOverride)) {
    if ((g.LastItemData.InFlags & ItemFlags_Disabled) &&
        !(flags & HoveredFlags_AllowWhenDisabled))
      return false;
    if (!IsItemFocused())
      return false;

    if (flags & HoveredFlags_ForTooltip)
      flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipNav);
  } else {
    // Test for bounding box overlap, as updated as ItemAdd()
    ItemStatusFlags status_flags = g.LastItemData.StatusFlags;
    if (!(status_flags & ItemStatusFlags_HoveredRect))
      return false;

    if (flags & HoveredFlags_ForTooltip)
      flags =
          ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);

    ASSERT((flags & (HoveredFlags_AnyWindow | HoveredFlags_RootWindow |
                     HoveredFlags_ChildWindows | HoveredFlags_NoPopupHierarchy |
                     HoveredFlags_DockHierarchy)) ==
           0); // Flags not supported by this function

    // Done with rectangle culling so we can perform heavier checks now
    // Test if we are hovering the right window (our window could be behind
    // another window) [2021/03/02] Reworked / reverted the revert, finally.
    // Note we want e.g. BeginGroup/ItemAdd/EndGroup to work as well. (#3851)
    // [2017/10/16] Reverted commit 344d48be3 and testing RootWindow instead. I
    // believe it is correct to NOT test for RootWindow but this leaves us
    // unable to use IsItemHovered() after EndChild() itself. Until a solution
    // is found I believe reverting to the test from 2017/09/27 is safe since
    // this was the test that has been running for a long while.
    if (g.HoveredWindow != window &&
        (status_flags & ItemStatusFlags_HoveredWindow) == 0)
      if ((flags & HoveredFlags_AllowWhenOverlappedByWindow) == 0)
        return false;

    // Test if another item is active (e.g. being dragged)
    const ID id = g.LastItemData.ID;
    if ((flags & HoveredFlags_AllowWhenBlockedByActiveItem) == 0)
      if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
        if (g.ActiveId != window->MoveId && g.ActiveId != window->TabId)
          return false;

    // Test if interactions on this window are blocked by an active popup or
    // modal. The HoveredFlags_AllowWhenBlockedByPopup flag will be tested
    // here.
    if (!IsWindowContentHoverable(window, flags) &&
        !(g.LastItemData.InFlags & ItemFlags_NoWindowHoverableCheck))
      return false;

    // Test if the item is disabled
    if ((g.LastItemData.InFlags & ItemFlags_Disabled) &&
        !(flags & HoveredFlags_AllowWhenDisabled))
      return false;

    // Special handling for calling after Begin() which represent the title bar
    // or tab. When the window is skipped/collapsed (SkipItems==true) that last
    // item (always ->MoveId submitted by Begin) will never be overwritten so we
    // need to detect the case.
    if (id == window->MoveId && window->WriteAccessed)
      return false;

    // Test if using AllowOverlap and overlapped
    if ((g.LastItemData.InFlags & ItemFlags_AllowOverlap) && id != 0)
      if ((flags & HoveredFlags_AllowWhenOverlappedByItem) == 0)
        if (g.HoveredIdPreviousFrame != g.LastItemData.ID)
          return false;
  }

  // Handle hover delay
  // (some ideas: https://www.nngroup.com/articles/timing-exposing-content)
  const float delay = CalcDelayFromHoveredFlags(flags);
  if (delay > 0.0f || (flags & HoveredFlags_Stationary)) {
    ID hover_delay_id = (g.LastItemData.ID != 0)
                            ? g.LastItemData.ID
                            : window->GetIDFromRectangle(g.LastItemData.Rect);
    if ((flags & HoveredFlags_NoSharedDelay) &&
        (g.HoverItemDelayIdPreviousFrame != hover_delay_id))
      g.HoverItemDelayTimer = 0.0f;
    g.HoverItemDelayId = hover_delay_id;

    // When changing hovered item we requires a bit of stationary delay before
    // activating hover timer, but once unlocked on a given item we also moving.
    // if (g.HoverDelayTimer >= delay && (g.HoverDelayTimer - g.IO.DeltaTime <
    // delay || g.MouseStationaryTimer - g.IO.DeltaTime <
    // g.Style.HoverStationaryDelay)) { DEBUG_LOG("HoverDelayTimer = %f/%f,
    // MouseStationaryTimer = %f\n", g.HoverDelayTimer, delay,
    // g.MouseStationaryTimer); }
    if ((flags & HoveredFlags_Stationary) != 0 &&
        g.HoverItemUnlockedStationaryId != hover_delay_id)
      return false;

    if (g.HoverItemDelayTimer < delay)
      return false;
  }

  return true;
}

// Internal facing ItemHoverable() used when submitting widgets. Differs
// slightly from IsItemHovered(). (this does not rely on LastItemData it can be
// called from a ButtonBehavior() call not following an ItemAdd() call)
// FIXME-LEGACY: the 'ItemFlags item_flags' parameter was added on
// 2023-06-28. If you used this in your legacy/custom widgets code:
// - Commonly: if your ItemHoverable() call comes after an ItemAdd() call: pass
// 'item_flags = g.LastItemData.InFlags'.
// - Rare: otherwise you may pass 'item_flags = 0' (ItemFlags_None) unless
// you want to benefit from special behavior handled by ItemHoverable.
bool Gui::ItemHoverable(const Rect &bb, ID id, ItemFlags item_flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (g.HoveredWindow != window)
    return false;
  if (!IsMouseHoveringRect(bb.Min, bb.Max))
    return false;

  if (g.HoveredId != 0 && g.HoveredId != id && !g.HoveredIdAllowOverlap)
    return false;
  if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
    return false;

  // Done with rectangle culling so we can perform heavier checks now.
  if (!(item_flags & ItemFlags_NoWindowHoverableCheck) &&
      !IsWindowContentHoverable(window, HoveredFlags_None)) {
    g.HoveredIdDisabled = true;
    return false;
  }

  // We exceptionally allow this function to be called with id==0 to allow using
  // it for easy high-level hover test in widgets code. We could also decide to
  // split this function is two.
  if (id != 0) {
    // Drag source doesn't report as hovered
    if (g.DragDropActive && g.DragDropPayload.SourceId == id &&
        !(g.DragDropSourceFlags & DragDropFlags_SourceNoDisableHover))
      return false;

    SetHoveredID(id);

    // AllowOverlap mode (rarely used) requires previous frame HoveredId to be
    // null or to match. This allows using patterns where a later submitted
    // widget overlaps a previous one. Generally perceived as a front-to-back
    // hit-test.
    if (item_flags & ItemFlags_AllowOverlap) {
      g.HoveredIdAllowOverlap = true;
      if (g.HoveredIdPreviousFrame != id)
        return false;
    }
  }

  // When disabled we'll return false but still set HoveredId
  if (item_flags & ItemFlags_Disabled) {
    // Release active id if turning disabled
    if (g.ActiveId == id && id != 0)
      ClearActiveID();
    g.HoveredIdDisabled = true;
    return false;
  }

  if (id != 0) {
    // [DEBUG] Item Picker tool!
    // We perform the check here because SetHoveredID() is not frequently called
    // (1~ time a frame), making the cost of this tool near-zero. We can get
    // slightly better call-stack and support picking non-hovered items if we
    // performed the test in ItemAdd(), but that would incur a small runtime
    // cost.
    if (g.DebugItemPickerActive && g.HoveredIdPreviousFrame == id)
      GetForegroundDrawList()->AddRect(bb.Min, bb.Max, COL32(255, 255, 0, 255));
    if (g.DebugItemPickerBreakId == id)
      DEBUG_BREAK();
  }

  if (g.NavDisableMouseHover)
    return false;

  return true;
}

// FIXME: This is inlined/duplicated in ItemAdd()
bool Gui::IsClippedEx(const Rect &bb, ID id) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (!bb.Overlaps(window->ClipRect))
    if (id == 0 || (id != g.ActiveId && id != g.NavId))
      if (!g.LogEnabled)
        return true;
  return false;
}

// This is also inlined in ItemAdd()
// Note: if ItemStatusFlags_HasDisplayRect is set, user needs to set
// g.LastItemData.DisplayRect.
void Gui::SetLastItemData(ID item_id, ItemFlags in_flags,
                          ItemStatusFlags item_flags, const Rect &item_rect) {
  Context &g = *GGui;
  g.LastItemData.ID = item_id;
  g.LastItemData.InFlags = in_flags;
  g.LastItemData.StatusFlags = item_flags;
  g.LastItemData.Rect = g.LastItemData.NavRect = item_rect;
}

float Gui::CalcWrapWidthForPos(const Vec2 &pos, float wrap_pos_x) {
  if (wrap_pos_x < 0.0f)
    return 0.0f;

  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (wrap_pos_x == 0.0f) {
    // We could decide to setup a default wrapping max point for auto-resizing
    // windows, or have auto-wrap (with unspecified wrapping pos) behave as a
    // ContentSize extending function?
    // if (window->Hidden && (window->Flags &
    // WindowFlags_AlwaysAutoResize))
    //    wrap_pos_x = Max(window->WorkRect.Min.x + g.FontSize * 10.0f,
    //    window->WorkRect.Max.x);
    // else
    wrap_pos_x = window->WorkRect.Max.x;
  } else if (wrap_pos_x > 0.0f) {
    wrap_pos_x +=
        window->Pos.x -
        window->Scroll.x; // wrap_pos_x is provided is window local space
  }

  return Max(wrap_pos_x - pos.x, 1.0f);
}

// ALLOC() == Gui::MemAlloc()
void *Gui::MemAlloc(size_t size) {
  void *ptr = (*GAllocatorAllocFunc)(size, GAllocatorUserData);
#ifndef DISABLE_DEBUG_TOOLS
  if (Context *ctx = GGui)
    DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, size);
#endif
  return ptr;
}

// FREE() == Gui::MemFree()
void Gui::MemFree(void *ptr) {
#ifndef DISABLE_DEBUG_TOOLS
  if (ptr != NULL)
    if (Context *ctx = GGui)
      DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, (size_t)-1);
#endif
  return (*GAllocatorFreeFunc)(ptr, GAllocatorUserData);
}

// We record the number of allocation in recent frames, as a way to
// audit/sanitize our guiding principles of "no allocations on idle/repeating
// frames"
void Gui::DebugAllocHook(DebugAllocInfo *info, int frame_count, void *ptr,
                         size_t size) {
  DebugAllocEntry *entry = &info->LastEntriesBuf[info->LastEntriesIdx];
  UNUSED(ptr);
  if (entry->FrameCount != frame_count) {
    info->LastEntriesIdx =
        (info->LastEntriesIdx + 1) % ARRAYSIZE(info->LastEntriesBuf);
    entry = &info->LastEntriesBuf[info->LastEntriesIdx];
    entry->FrameCount = frame_count;
    entry->AllocCount = entry->FreeCount = 0;
  }
  if (size != (size_t)-1) {
    entry->AllocCount++;
    info->TotalAllocCount++;
    // printf("[%05d] MemAlloc(%d) -> 0x%p\n", frame_count, size, ptr);
  } else {
    entry->FreeCount++;
    info->TotalFreeCount++;
    // printf("[%05d] MemFree(0x%p)\n", frame_count, ptr);
  }
}

const char *Gui::GetClipboardText() {
  Context &g = *GGui;
  return g.IO.GetClipboardTextFn
             ? g.IO.GetClipboardTextFn(g.IO.ClipboardUserData)
             : "";
}

void Gui::SetClipboardText(const char *text) {
  Context &g = *GGui;
  if (g.IO.SetClipboardTextFn)
    g.IO.SetClipboardTextFn(g.IO.ClipboardUserData, text);
}

const char *Gui::GetVersion() { return VERSION; }

IO &Gui::GetIO() {
  ASSERT(GGui != NULL &&
         "No current context. Did you call Gui::CreateContext() and "
         "Gui::SetCurrentContext() ?");
  return GGui->IO;
}

PlatformIO &Gui::GetPlatformIO() {
  ASSERT(GGui != NULL &&
         "No current context. Did you call Gui::CreateContext() or "
         "Gui::SetCurrentContext()?");
  return GGui->PlatformIO;
}

// Pass this to your backend rendering function! Valid after Render() and until
// the next call to NewFrame()
DrawData *Gui::GetDrawData() {
  Context &g = *GGui;
  ViewportP *viewport = g.Viewports[0];
  return viewport->DrawDataP.Valid ? &viewport->DrawDataP : NULL;
}

double Gui::GetTime() { return GGui->Time; }

int Gui::GetFrameCount() { return GGui->FrameCount; }

static DrawList *GetViewportBgFgDrawList(ViewportP *viewport,
                                         size_t drawlist_no,
                                         const char *drawlist_name) {
  // Create the draw list on demand, because they are not frequently used for
  // all viewports
  Context &g = *GGui;
  ASSERT(drawlist_no < ARRAYSIZE(viewport->BgFgDrawLists));
  DrawList *draw_list = viewport->BgFgDrawLists[drawlist_no];
  if (draw_list == NULL) {
    draw_list = NEW(DrawList)(&g.DrawListSharedData);
    draw_list->_OwnerName = drawlist_name;
    viewport->BgFgDrawLists[drawlist_no] = draw_list;
  }

  // Our DrawList system requires that there is always a command
  if (viewport->BgFgDrawListsLastFrame[drawlist_no] != g.FrameCount) {
    draw_list->_ResetForNewFrame();
    draw_list->PushTextureID(g.IO.Fonts->TexID);
    draw_list->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size,
                            false);
    viewport->BgFgDrawListsLastFrame[drawlist_no] = g.FrameCount;
  }
  return draw_list;
}

DrawList *Gui::GetBackgroundDrawList(Viewport *viewport) {
  return GetViewportBgFgDrawList((ViewportP *)viewport, 0, "##Background");
}

DrawList *Gui::GetBackgroundDrawList() {
  Context &g = *GGui;
  return GetBackgroundDrawList(g.CurrentWindow->Viewport);
}

DrawList *Gui::GetForegroundDrawList(Viewport *viewport) {
  return GetViewportBgFgDrawList((ViewportP *)viewport, 1, "##Foreground");
}

DrawList *Gui::GetForegroundDrawList() {
  Context &g = *GGui;
  return GetForegroundDrawList(g.CurrentWindow->Viewport);
}

DrawListSharedData *Gui::GetDrawListSharedData() {
  return &GGui->DrawListSharedData;
}

void Gui::StartMouseMovingWindow(Window *window) {
  // Set ActiveId even if the _NoMove flag is set. Without it, dragging away
  // from a window with _NoMove would activate hover on other windows. We _also_
  // call this when clicking in a window empty space when
  // io.ConfigWindowsMoveFromTitleBarOnly is set, but clear g.MovingWindow
  // afterward. This is because we want ActiveId to be set even when the window
  // is not permitted to move.
  Context &g = *GGui;
  FocusWindow(window);
  SetActiveID(window->MoveId, window);
  g.NavDisableHighlight = true;
  g.ActiveIdClickOffset =
      g.IO.MouseClickedPos[0] - window->RootWindowDockTree->Pos;
  g.ActiveIdNoClearOnFocusLoss = true;
  SetActiveIdUsingAllKeyboardKeys();

  bool can_move_window = true;
  if ((window->Flags & WindowFlags_NoMove) ||
      (window->RootWindowDockTree->Flags & WindowFlags_NoMove))
    can_move_window = false;
  if (DockNode *node = window->DockNodeAsHost)
    if (node->VisibleWindow &&
        (node->VisibleWindow->Flags & WindowFlags_NoMove))
      can_move_window = false;
  if (can_move_window)
    g.MovingWindow = window;
}

// We use 'undock == false' when dragging from title bar to allow moving groups
// of floating nodes without undocking them.
void Gui::StartMouseMovingWindowOrNode(Window *window, DockNode *node,
                                       bool undock) {
  Context &g = *GGui;
  bool can_undock_node = false;
  if (undock && node != NULL && node->VisibleWindow &&
      (node->VisibleWindow->Flags & WindowFlags_NoMove) == 0 &&
      (node->MergedFlags & DockNodeFlags_NoUndocking) == 0) {
    // Can undock if:
    // - part of a hierarchy with more than one visible node (if only one is
    // visible, we'll just move the root window)
    // - part of a dockspace node hierarchy: so we can undock the last single
    // visible node too (trivia: undocking from a fixed/central node will create
    // a new node and copy windows)
    DockNode *root_node = DockNodeGetRootNode(node);
    if (root_node->OnlyNodeWithWindows != node ||
        root_node->CentralNode != NULL) // -V1051 PVS-Studio thinks node should
                                        // be root_node and is wrong about that.
      can_undock_node = true;
  }

  const bool clicked = IsMouseClicked(0);
  const bool dragging = IsMouseDragging(0);
  if (can_undock_node && dragging)
    DockContextQueueUndockNode(
        &g, node); // Will lead to DockNodeStartMouseMovingWindow() ->
                   // StartMouseMovingWindow() being called next frame
  else if (!can_undock_node && (clicked || dragging) &&
           g.MovingWindow != window)
    StartMouseMovingWindow(window);
}

// Handle mouse moving window
// Note: moving window with the navigation keys (Square + d-pad / CTRL+TAB +
// Arrows) are processed in NavUpdateWindowing()
// FIXME: We don't have strong guarantee that g.MovingWindow stay synched with
// g.ActiveId == g.MovingWindow->MoveId. This is currently enforced by the fact
// that BeginDragDropSource() is setting all g.ActiveIdUsingXXXX flags to
// inhibit navigation inputs, but if we should more thoroughly test cases where
// g.ActiveId or g.MovingWindow gets changed and not the other.
void Gui::UpdateMouseMovingWindowNewFrame() {
  Context &g = *GGui;
  if (g.MovingWindow != NULL) {
    // We actually want to move the root window. g.MovingWindow == window we
    // clicked on (could be a child window). We track it to preserve Focus and
    // so that generally ActiveIdWindow == MovingWindow and ActiveId ==
    // MovingWindow->MoveId for consistency.
    KeepAliveID(g.ActiveId);
    ASSERT(g.MovingWindow && g.MovingWindow->RootWindowDockTree);
    Window *moving_window = g.MovingWindow->RootWindowDockTree;

    // When a window stop being submitted while being dragged, it may will its
    // viewport until next Begin()
    const bool window_disappared =
        (!moving_window->WasActive && !moving_window->Active);
    if (g.IO.MouseDown[0] && IsMousePosValid(&g.IO.MousePos) &&
        !window_disappared) {
      Vec2 pos = g.IO.MousePos - g.ActiveIdClickOffset;
      if (moving_window->Pos.x != pos.x || moving_window->Pos.y != pos.y) {
        SetWindowPos(moving_window, pos, Cond_Always);
        if (moving_window->Viewport &&
            moving_window
                ->ViewportOwned) // Synchronize viewport immediately because
                                 // some overlays may relies on clipping
                                 // rectangle before we Begin() into the window.
        {
          moving_window->Viewport->Pos = pos;
          moving_window->Viewport->UpdateWorkRect();
        }
      }
      FocusWindow(g.MovingWindow);
    } else {
      if (!window_disappared) {
        // Try to merge the window back into the main viewport.
        // This works because MouseViewport should be != MovingWindow->Viewport
        // on release (as per code in UpdateViewports)
        if (g.ConfigFlagsCurrFrame & ConfigFlags_ViewportsEnable)
          UpdateTryMergeWindowIntoHostViewport(moving_window, g.MouseViewport);

        // Restore the mouse viewport so that we don't hover the viewport
        // _under_ the moved window during the frame we released the mouse
        // button.
        if (moving_window->Viewport && !IsDragDropPayloadBeingAccepted())
          g.MouseViewport = moving_window->Viewport;

        // Clear the NoInput window flag set by the Viewport system
        if (moving_window->Viewport)
          moving_window->Viewport->Flags &= ~ViewportFlags_NoInputs;
      }

      g.MovingWindow = NULL;
      ClearActiveID();
    }
  } else {
    // When clicking/dragging from a window that has the _NoMove flag, we still
    // set the ActiveId in order to prevent hovering others.
    if (g.ActiveIdWindow && g.ActiveIdWindow->MoveId == g.ActiveId) {
      KeepAliveID(g.ActiveId);
      if (!g.IO.MouseDown[0])
        ClearActiveID();
    }
  }
}

// Initiate moving window when clicking on empty space or title bar.
// Handle left-click and right-click focus.
void Gui::UpdateMouseMovingWindowEndFrame() {
  Context &g = *GGui;
  if (g.ActiveId != 0 || g.HoveredId != 0)
    return;

  // Unless we just made a window/popup appear
  if (g.NavWindow && g.NavWindow->Appearing)
    return;

  // Click on empty space to focus window and start moving
  // (after we're done with all our widgets, so e.g. clicking on docking tab-bar
  // which have set HoveredId already and not get us here!)
  if (g.IO.MouseClicked[0]) {
    // Handle the edge case of a popup being closed while clicking in its empty
    // space. If we try to focus it, FocusWindow() > ClosePopupsOverWindow()
    // will accidentally close any parent popups because they are not linked
    // together any more.
    Window *root_window = g.HoveredWindow ? g.HoveredWindow->RootWindow : NULL;
    const bool is_closed_popup =
        root_window && (root_window->Flags & WindowFlags_Popup) &&
        !IsPopupOpen(root_window->PopupId, PopupFlags_AnyPopupLevel);

    if (root_window != NULL && !is_closed_popup) {
      StartMouseMovingWindow(g.HoveredWindow); //-V595

      // Cancel moving if clicked outside of title bar
      if (g.IO.ConfigWindowsMoveFromTitleBarOnly)
        if (!(root_window->Flags & WindowFlags_NoTitleBar) ||
            root_window->DockIsActive)
          if (!root_window->TitleBarRect().Contains(g.IO.MouseClickedPos[0]))
            g.MovingWindow = NULL;

      // Cancel moving if clicked over an item which was disabled or inhibited
      // by popups (note that we know HoveredId == 0 already)
      if (g.HoveredIdDisabled)
        g.MovingWindow = NULL;
    } else if (root_window == NULL && g.NavWindow != NULL) {
      // Clicking on void disable focus
      FocusWindow(NULL, FocusRequestFlags_UnlessBelowModal);
    }
  }

  // With right mouse button we close popups without changing focus based on
  // where the mouse is aimed Instead, focus will be restored to the window
  // under the bottom-most closed popup. (The left mouse button path calls
  // FocusWindow on the hovered window, which will lead
  // NewFrame->ClosePopupsOverWindow to trigger)
  if (g.IO.MouseClicked[1]) {
    // Find the top-most window between HoveredWindow and the top-most Modal
    // Window. This is where we can trim the popup stack.
    Window *modal = GetTopMostPopupModal();
    bool hovered_window_above_modal =
        g.HoveredWindow &&
        (modal == NULL || IsWindowAbove(g.HoveredWindow, modal));
    ClosePopupsOverWindow(hovered_window_above_modal ? g.HoveredWindow : modal,
                          true);
  }
}

// This is called during NewFrame()->UpdateViewportsNewFrame() only.
// Need to keep in sync with SetWindowPos()
static void TranslateWindow(Window *window, const Vec2 &delta) {
  window->Pos += delta;
  window->ClipRect.Translate(delta);
  window->OuterRectClipped.Translate(delta);
  window->InnerRect.Translate(delta);
  window->DC.CursorPos += delta;
  window->DC.CursorStartPos += delta;
  window->DC.CursorMaxPos += delta;
  window->DC.IdealMaxPos += delta;
}

static void ScaleWindow(Window *window, float scale) {
  Vec2 origin = window->Viewport->Pos;
  window->Pos = Floor((window->Pos - origin) * scale + origin);
  window->Size = Trunc(window->Size * scale);
  window->SizeFull = Trunc(window->SizeFull * scale);
  window->ContentSize = Trunc(window->ContentSize * scale);
}

static bool IsWindowActiveAndVisible(Window *window) {
  return (window->Active) && (!window->Hidden);
}

// The reason this is exposed in internal.hpp is: on touch-based system that
// don't have hovering, we want to dispatch inputs to the right target (imgui vs
// imgui+app)
void Gui::UpdateHoveredWindowAndCaptureFlags() {
  Context &g = *GGui;
  IO &io = g.IO;
  g.WindowsHoverPadding =
      Max(g.Style.TouchExtraPadding,
          Vec2(WINDOWS_HOVER_PADDING, WINDOWS_HOVER_PADDING));

  // Find the window hovered by mouse:
  // - Child windows can extend beyond the limit of their parent so we need to
  // derive HoveredRootWindow from HoveredWindow.
  // - When moving a window we can skip the search, which also conveniently
  // bypasses the fact that window->WindowRectClipped is lagging as this point
  // of the frame.
  // - We also support the moved window toggling the NoInputs flag after moving
  // has started in order to be able to detect windows below it, which is useful
  // for e.g. docking mechanisms.
  bool clear_hovered_windows = false;
  FindHoveredWindow();
  ASSERT(g.HoveredWindow == NULL || g.HoveredWindow == g.MovingWindow ||
         g.HoveredWindow->Viewport == g.MouseViewport);

  // Modal windows prevents mouse from hovering behind them.
  Window *modal_window = GetTopMostPopupModal();
  if (modal_window && g.HoveredWindow &&
      !IsWindowWithinBeginStackOf(
          g.HoveredWindow->RootWindow,
          modal_window)) // FIXME-MERGE: RootWindowDockTree ?
    clear_hovered_windows = true;

  // Disabled mouse?
  if (io.ConfigFlags & ConfigFlags_NoMouse)
    clear_hovered_windows = true;

  // We track click ownership. When clicked outside of a window the click is
  // owned by the application and won't report hovering nor request capture even
  // while dragging over our windows afterward.
  const bool has_open_popup = (g.OpenPopupStack.Size > 0);
  const bool has_open_modal = (modal_window != NULL);
  int mouse_earliest_down = -1;
  bool mouse_any_down = false;
  for (int i = 0; i < ARRAYSIZE(io.MouseDown); i++) {
    if (io.MouseClicked[i]) {
      io.MouseDownOwned[i] = (g.HoveredWindow != NULL) || has_open_popup;
      io.MouseDownOwnedUnlessPopupClose[i] =
          (g.HoveredWindow != NULL) || has_open_modal;
    }
    mouse_any_down |= io.MouseDown[i];
    if (io.MouseDown[i])
      if (mouse_earliest_down == -1 ||
          io.MouseClickedTime[i] < io.MouseClickedTime[mouse_earliest_down])
        mouse_earliest_down = i;
  }
  const bool mouse_avail =
      (mouse_earliest_down == -1) || io.MouseDownOwned[mouse_earliest_down];
  const bool mouse_avail_unless_popup_close =
      (mouse_earliest_down == -1) ||
      io.MouseDownOwnedUnlessPopupClose[mouse_earliest_down];

  // If mouse was first clicked outside of Gui bounds we also cancel out
  // hovering.
  // FIXME: For patterns of drag and drop across OS windows, we may need to
  // rework/remove this test (first committed 311c0ca9 on 2015/02)
  const bool mouse_dragging_extern_payload =
      g.DragDropActive &&
      (g.DragDropSourceFlags & DragDropFlags_SourceExtern) != 0;
  if (!mouse_avail && !mouse_dragging_extern_payload)
    clear_hovered_windows = true;

  if (clear_hovered_windows)
    g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;

  // Update io.WantCaptureMouse for the user application (true = dispatch mouse
  // info to Gui only, false = dispatch mouse to Gui + underlying
  // app) Update io.WantCaptureMouseAllowPopupClose (experimental) to give a
  // chance for app to react to popup closure with a drag
  if (g.WantCaptureMouseNextFrame != -1) {
    io.WantCaptureMouse = io.WantCaptureMouseUnlessPopupClose =
        (g.WantCaptureMouseNextFrame != 0);
  } else {
    io.WantCaptureMouse =
        (mouse_avail && (g.HoveredWindow != NULL || mouse_any_down)) ||
        has_open_popup;
    io.WantCaptureMouseUnlessPopupClose =
        (mouse_avail_unless_popup_close &&
         (g.HoveredWindow != NULL || mouse_any_down)) ||
        has_open_modal;
  }

  // Update io.WantCaptureKeyboard for the user application (true = dispatch
  // keyboard info to Gui only, false = dispatch keyboard info to
  // Gui + underlying app)
  io.WantCaptureKeyboard = (g.ActiveId != 0) || (modal_window != NULL);
  if (io.NavActive && (io.ConfigFlags & ConfigFlags_NavEnableKeyboard) &&
      !(io.ConfigFlags & ConfigFlags_NavNoCaptureKeyboard))
    io.WantCaptureKeyboard = true;
  if (g.WantCaptureKeyboardNextFrame != -1) // Manual override
    io.WantCaptureKeyboard = (g.WantCaptureKeyboardNextFrame != 0);

  // Update io.WantTextInput flag, this is to allow systems without a keyboard
  // (e.g. mobile, hand-held) to show a software keyboard if possible
  io.WantTextInput = (g.WantTextInputNextFrame != -1)
                         ? (g.WantTextInputNextFrame != 0)
                         : false;
}

void Gui::NewFrame() {
  ASSERT(GGui != NULL &&
         "No current context. Did you call Gui::CreateContext() and "
         "Gui::SetCurrentContext() ?");
  Context &g = *GGui;

  // Remove pending delete hooks before frame start.
  // This deferred removal avoid issues of removal while iterating the hook
  // vector
  for (int n = g.Hooks.Size - 1; n >= 0; n--)
    if (g.Hooks[n].Type == ContextHookType_PendingRemoval_)
      g.Hooks.erase(&g.Hooks[n]);

  CallContextHooks(&g, ContextHookType_NewFramePre);

  // Check and assert for various common IO and Configuration mistakes
  g.ConfigFlagsLastFrame = g.ConfigFlagsCurrFrame;
  ErrorCheckNewFrameSanityChecks();
  g.ConfigFlagsCurrFrame = g.IO.ConfigFlags;

  // Load settings on first frame, save settings when modified (after a delay)
  UpdateSettings();

  g.Time += g.IO.DeltaTime;
  g.WithinFrameScope = true;
  g.FrameCount += 1;
  g.TooltipOverrideCount = 0;
  g.WindowsActiveCount = 0;
  g.MenusIdSubmittedThisFrame.resize(0);

  // Calculate frame-rate for the user, as a purely luxurious feature
  g.FramerateSecPerFrameAccum +=
      g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
  g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
  g.FramerateSecPerFrameIdx =
      (g.FramerateSecPerFrameIdx + 1) % ARRAYSIZE(g.FramerateSecPerFrame);
  g.FramerateSecPerFrameCount =
      Min(g.FramerateSecPerFrameCount + 1, ARRAYSIZE(g.FramerateSecPerFrame));
  g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f)
                       ? (1.0f / (g.FramerateSecPerFrameAccum /
                                  (float)g.FramerateSecPerFrameCount))
                       : FLT_MAX;

  // Process input queue (trickle as many events as possible), turn events into
  // writes to IO structure
  g.InputEventsTrail.resize(0);
  UpdateInputEvents(g.IO.ConfigInputTrickleEventQueue);

  // Update viewports (after processing input queue, so io.MouseHoveredViewport
  // is set)
  UpdateViewportsNewFrame();

  // Setup current font and draw list shared data
  // FIXME-VIEWPORT: the concept of a single ClipRectFullscreen is not ideal!
  g.IO.Fonts->Locked = true;
  SetCurrentFont(GetDefaultFont());
  ASSERT(g.Font->IsLoaded());
  Rect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (ViewportP *viewport : g.Viewports)
    virtual_space.Add(viewport->GetMainRect());
  g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
  g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
  g.DrawListSharedData.SetCircleTessellationMaxError(
      g.Style.CircleTessellationMaxError);
  g.DrawListSharedData.InitialFlags = DrawListFlags_None;
  if (g.Style.AntiAliasedLines)
    g.DrawListSharedData.InitialFlags |= DrawListFlags_AntiAliasedLines;
  if (g.Style.AntiAliasedLinesUseTex &&
      !(g.Font->ContainerAtlas->Flags & FontAtlasFlags_NoBakedLines))
    g.DrawListSharedData.InitialFlags |= DrawListFlags_AntiAliasedLinesUseTex;
  if (g.Style.AntiAliasedFill)
    g.DrawListSharedData.InitialFlags |= DrawListFlags_AntiAliasedFill;
  if (g.IO.BackendFlags & BackendFlags_RendererHasVtxOffset)
    g.DrawListSharedData.InitialFlags |= DrawListFlags_AllowVtxOffset;

  // Mark rendering data as invalid to prevent user who may have a handle on it
  // to use it.
  for (ViewportP *viewport : g.Viewports) {
    viewport->DrawData = NULL;
    viewport->DrawDataP.Valid = false;
  }

  // Drag and drop keep the source ID alive so even if the source disappear our
  // state is consistent
  if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
    KeepAliveID(g.DragDropPayload.SourceId);

  // Update HoveredId data
  if (!g.HoveredIdPreviousFrame)
    g.HoveredIdTimer = 0.0f;
  if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
    g.HoveredIdNotActiveTimer = 0.0f;
  if (g.HoveredId)
    g.HoveredIdTimer += g.IO.DeltaTime;
  if (g.HoveredId && g.ActiveId != g.HoveredId)
    g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
  g.HoveredIdPreviousFrame = g.HoveredId;
  g.HoveredId = 0;
  g.HoveredIdAllowOverlap = false;
  g.HoveredIdDisabled = false;

  // Clear ActiveID if the item is not alive anymore.
  // In 1.87, the common most call to KeepAliveID() was moved from GetID() to
  // ItemAdd(). As a result, custom widget using ButtonBehavior() _without_
  // ItemAdd() need to call KeepAliveID() themselves.
  if (g.ActiveId != 0 && g.ActiveIdIsAlive != g.ActiveId &&
      g.ActiveIdPreviousFrame == g.ActiveId) {
    DEBUG_LOG_ACTIVEID(
        "NewFrame(): ClearActiveID() because it isn't marked alive anymore!\n");
    ClearActiveID();
  }

  // Update ActiveId data (clear reference to active widget if the widget isn't
  // alive anymore)
  if (g.ActiveId)
    g.ActiveIdTimer += g.IO.DeltaTime;
  g.LastActiveIdTimer += g.IO.DeltaTime;
  g.ActiveIdPreviousFrame = g.ActiveId;
  g.ActiveIdPreviousFrameWindow = g.ActiveIdWindow;
  g.ActiveIdPreviousFrameHasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
  g.ActiveIdIsAlive = 0;
  g.ActiveIdHasBeenEditedThisFrame = false;
  g.ActiveIdPreviousFrameIsAlive = false;
  g.ActiveIdIsJustActivated = false;
  if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
    g.TempInputId = 0;
  if (g.ActiveId == 0) {
    g.ActiveIdUsingNavDirMask = 0x00;
    g.ActiveIdUsingAllKeyboardKeys = false;
#ifndef DISABLE_OBSOLETE_KEYIO
    g.ActiveIdUsingNavInputMask = 0x00;
#endif
  }

#ifndef DISABLE_OBSOLETE_KEYIO
  if (g.ActiveId == 0)
    g.ActiveIdUsingNavInputMask = 0;
  else if (g.ActiveIdUsingNavInputMask != 0) {
    // If your custom widget code used:                 {
    // g.ActiveIdUsingNavInputMask |= (1 << NavInput_Cancel); } Since
    // VERSION_NUM >= 18804 it should be:   { SetKeyOwner(Key_Escape,
    // g.ActiveId); SetKeyOwner(Key_NavGamepadCancel, g.ActiveId); }
    if (g.ActiveIdUsingNavInputMask & (1 << NavInput_Cancel))
      SetKeyOwner(Key_Escape, g.ActiveId);
    if (g.ActiveIdUsingNavInputMask & ~(1 << NavInput_Cancel))
      ASSERT(0); // Other values unsupported
  }
#endif

  // Record when we have been stationary as this state is preserved while over
  // same item.
  // FIXME: The way this is expressed means user cannot alter
  // HoverStationaryDelay during the frame to use varying values. To allow this
  // we should store HoverItemMaxStationaryTime+ID and perform the >= check in
  // IsItemHovered() function.
  if (g.HoverItemDelayId != 0 &&
      g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
    g.HoverItemUnlockedStationaryId = g.HoverItemDelayId;
  else if (g.HoverItemDelayId == 0)
    g.HoverItemUnlockedStationaryId = 0;
  if (g.HoveredWindow != NULL &&
      g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
    g.HoverWindowUnlockedStationaryId = g.HoveredWindow->ID;
  else if (g.HoveredWindow == NULL)
    g.HoverWindowUnlockedStationaryId = 0;

  // Update hover delay for IsItemHovered() with delays and tooltips
  g.HoverItemDelayIdPreviousFrame = g.HoverItemDelayId;
  if (g.HoverItemDelayId != 0) {
    g.HoverItemDelayTimer += g.IO.DeltaTime;
    g.HoverItemDelayClearTimer = 0.0f;
    g.HoverItemDelayId = 0;
  } else if (g.HoverItemDelayTimer > 0.0f) {
    // This gives a little bit of leeway before clearing the hover timer,
    // allowing mouse to cross gaps We could expose 0.25f as
    // style.HoverClearDelay but I am not sure of the logic yet, this is
    // particularly subtle.
    g.HoverItemDelayClearTimer += g.IO.DeltaTime;
    if (g.HoverItemDelayClearTimer >=
        Max(0.25f, g.IO.DeltaTime *
                       2.0f)) // ~7 frames at 30 Hz + allow for low framerate
      g.HoverItemDelayTimer = g.HoverItemDelayClearTimer =
          0.0f; // May want a decaying timer, in which case need to clamp at max
                // first, based on max of caller last requested timer.
  }

  // Drag and drop
  g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
  g.DragDropAcceptIdCurr = 0;
  g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
  g.DragDropWithinSource = false;
  g.DragDropWithinTarget = false;
  g.DragDropHoldJustPressedId = 0;

  // Close popups on focus lost (currently wip/opt-in)
  // if (g.IO.AppFocusLost)
  //    ClosePopupsExceptModals();

  // Update keyboard input state
  UpdateKeyboardInputs();

  // ASSERT(g.IO.KeyCtrl == IsKeyDown(Key_LeftCtrl) ||
  // IsKeyDown(Key_RightCtrl)); ASSERT(g.IO.KeyShift ==
  // IsKeyDown(Key_LeftShift) || IsKeyDown(Key_RightShift));
  // ASSERT(g.IO.KeyAlt == IsKeyDown(Key_LeftAlt) ||
  // IsKeyDown(Key_RightAlt)); ASSERT(g.IO.KeySuper ==
  // IsKeyDown(Key_LeftSuper) || IsKeyDown(Key_RightSuper));

  // Update gamepad/keyboard navigation
  NavUpdate();

  // Update mouse input state
  UpdateMouseInputs();

  // Undocking
  // (needs to be before UpdateMouseMovingWindowNewFrame so the window is
  // already offset and following the mouse on the detaching frame)
  DockContextNewFrameUpdateUndocking(&g);

  // Find hovered window
  // (needs to be before UpdateMouseMovingWindowNewFrame so we fill
  // g.HoveredWindowUnderMovingWindow on the mouse release frame)
  UpdateHoveredWindowAndCaptureFlags();

  // Handle user moving window with mouse (at the beginning of the frame to
  // avoid input lag or sheering)
  UpdateMouseMovingWindowNewFrame();

  // Background darkening/whitening
  if (GetTopMostPopupModal() != NULL ||
      (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
    g.DimBgRatio = Min(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
  else
    g.DimBgRatio = Max(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

  g.MouseCursor = MouseCursor_Arrow;
  g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame =
      g.WantTextInputNextFrame = -1;

  // Platform IME data: reset for the frame
  g.PlatformImeDataPrev = g.PlatformImeData;
  g.PlatformImeData.WantVisible = false;

  // Mouse wheel scrolling, scale
  UpdateMouseWheel();

  // Mark all windows as not visible and compact unused memory.
  ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size);
  const float memory_compact_start_time =
      (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f)
          ? FLT_MAX
          : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
  for (Window *window : g.Windows) {
    window->WasActive = window->Active;
    window->Active = false;
    window->WriteAccessed = false;
    window->BeginCountPreviousFrame = window->BeginCount;
    window->BeginCount = 0;

    // Garbage collect transient buffers of recently unused windows
    if (!window->WasActive && !window->MemoryCompacted &&
        window->LastTimeActive < memory_compact_start_time)
      GcCompactTransientWindowBuffers(window);
  }

  // Garbage collect transient buffers of recently unused tables
  for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
    if (g.TablesLastTimeActive[i] >= 0.0f &&
        g.TablesLastTimeActive[i] < memory_compact_start_time)
      TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
  for (TableTempData &table_temp_data : g.TablesTempData)
    if (table_temp_data.LastTimeActive >= 0.0f &&
        table_temp_data.LastTimeActive < memory_compact_start_time)
      TableGcCompactTransientBuffers(&table_temp_data);
  if (g.GcCompactAll)
    GcCompactTransientMiscBuffers();
  g.GcCompactAll = false;

  // Closing the focused window restore focus to the first active root window in
  // descending z-order
  if (g.NavWindow && !g.NavWindow->WasActive)
    FocusTopMostWindowUnderOne(NULL, NULL, NULL,
                               FocusRequestFlags_RestoreFocusedChild);

  // No window should be open at the beginning of the frame.
  // But in order to allow the user to call NewFrame() multiple times without
  // calling Render(), we are doing an explicit clear.
  g.CurrentWindowStack.resize(0);
  g.BeginPopupStack.resize(0);
  g.ItemFlagsStack.resize(0);
  g.ItemFlagsStack.push_back(ItemFlags_None);
  g.GroupStack.resize(0);

  // Docking
  DockContextNewFrameUpdateDocking(&g);

  // [DEBUG] Update debug features
  UpdateDebugToolItemPicker();
  UpdateDebugToolStackQueries();
  UpdateDebugToolFlashStyleColor();
  if (g.DebugLocateFrames > 0 && --g.DebugLocateFrames == 0)
    g.DebugLocateId = 0;
  if (g.DebugLogClipperAutoDisableFrames > 0 &&
      --g.DebugLogClipperAutoDisableFrames == 0) {
    DebugLog("(Debug Log: Auto-disabled DebugLogFlags_EventClipper after "
             "2 frames)\n");
    g.DebugLogFlags &= ~DebugLogFlags_EventClipper;
  }

  // Create implicit/fallback window - which we will only render it if the user
  // has added something to it. We don't use "Debug" to avoid colliding with
  // user trying to create a "Debug" window with custom flags. This fallback is
  // particularly important as it prevents Gui:: calls from crashing.
  g.WithinFrameScopeWithImplicitWindow = true;
  SetNextWindowSize(Vec2(400, 400), Cond_FirstUseEver);
  Begin("Debug##Default");
  ASSERT(g.CurrentWindow->IsFallbackWindow == true);

  // [DEBUG] When io.ConfigDebugBeginReturnValue is set, we make
  // Begin()/BeginChild() return false at different level of the window-stack,
  // allowing to validate correct Begin/End behavior in user code.
  if (g.IO.ConfigDebugBeginReturnValueLoop)
    g.DebugBeginReturnValueCullDepth =
        (g.DebugBeginReturnValueCullDepth == -1)
            ? 0
            : ((g.DebugBeginReturnValueCullDepth +
                ((g.FrameCount % 4) == 0 ? 1 : 0)) %
               10);
  else
    g.DebugBeginReturnValueCullDepth = -1;

  CallContextHooks(&g, ContextHookType_NewFramePost);
}

// FIXME: Add a more explicit sort order in the window structure.
static int CDECL ChildWindowComparer(const void *lhs, const void *rhs) {
  const Window *const a = *(const Window *const *)lhs;
  const Window *const b = *(const Window *const *)rhs;
  if (int d = (a->Flags & WindowFlags_Popup) - (b->Flags & WindowFlags_Popup))
    return d;
  if (int d =
          (a->Flags & WindowFlags_Tooltip) - (b->Flags & WindowFlags_Tooltip))
    return d;
  return (a->BeginOrderWithinParent - b->BeginOrderWithinParent);
}

static void AddWindowToSortBuffer(Vector<Window *> *out_sorted_windows,
                                  Window *window) {
  out_sorted_windows->push_back(window);
  if (window->Active) {
    int count = window->DC.ChildWindows.Size;
    Qsort(window->DC.ChildWindows.Data, (size_t)count, sizeof(Window *),
          ChildWindowComparer);
    for (int i = 0; i < count; i++) {
      Window *child = window->DC.ChildWindows[i];
      if (child->Active)
        AddWindowToSortBuffer(out_sorted_windows, child);
    }
  }
}

static void AddWindowToDrawData(Window *window, int layer) {
  Context &g = *GGui;
  ViewportP *viewport = window->Viewport;
  ASSERT(viewport != NULL);
  g.IO.MetricsRenderWindows++;
  if (window->DrawList->_Splitter._Count > 1)
    window->DrawList->ChannelsMerge(); // Merge if user forgot to merge back.
                                       // Also required in Docking branch for
                                       // WindowFlags_DockNodeHost windows.
  Gui::AddDrawListToDrawDataEx(&viewport->DrawDataP,
                               viewport->DrawDataBuilder.Layers[layer],
                               window->DrawList);
  for (Window *child : window->DC.ChildWindows)
    if (IsWindowActiveAndVisible(
            child)) // Clipped children may have been marked not active
      AddWindowToDrawData(child, layer);
}

static inline int GetWindowDisplayLayer(Window *window) {
  return (window->Flags & WindowFlags_Tooltip) ? 1 : 0;
}

// Layer is locked for the root window, however child windows may use a
// different viewport (e.g. extruding menu)
static inline void AddRootWindowToDrawData(Window *window) {
  AddWindowToDrawData(window, GetWindowDisplayLayer(window));
}

static void FlattenDrawDataIntoSingleLayer(DrawDataBuilder *builder) {
  int n = builder->Layers[0]->Size;
  int full_size = n;
  for (int i = 1; i < ARRAYSIZE(builder->Layers); i++)
    full_size += builder->Layers[i]->Size;
  builder->Layers[0]->resize(full_size);
  for (int layer_n = 1; layer_n < ARRAYSIZE(builder->Layers); layer_n++) {
    Vector<DrawList *> *layer = builder->Layers[layer_n];
    if (layer->empty())
      continue;
    memcpy(builder->Layers[0]->Data + n, layer->Data,
           layer->Size * sizeof(DrawList *));
    n += layer->Size;
    layer->resize(0);
  }
}

static void InitViewportDrawData(ViewportP *viewport) {
  IO &io = Gui::GetIO();
  DrawData *draw_data = &viewport->DrawDataP;

  viewport->DrawData = draw_data; // Make publicly accessible
  viewport->DrawDataBuilder.Layers[0] = &draw_data->CmdLists;
  viewport->DrawDataBuilder.Layers[1] = &viewport->DrawDataBuilder.LayerData1;
  viewport->DrawDataBuilder.Layers[0]->resize(0);
  viewport->DrawDataBuilder.Layers[1]->resize(0);

  // When minimized, we report draw_data->DisplaySize as zero to be consistent
  // with non-viewport mode, and to allow applications/backends to easily skip
  // rendering.
  // FIXME: Note that we however do NOT attempt to report "zero drawlist /
  // vertices" into the DrawData structure. This is because the work has been
  // done already, and its wasted! We should fix that and add optimizations for
  // it earlier in the pipeline, rather than pretend to hide the data at the end
  // of the pipeline.
  const bool is_minimized = (viewport->Flags & ViewportFlags_IsMinimized) != 0;

  draw_data->Valid = true;
  draw_data->CmdListsCount = 0;
  draw_data->TotalVtxCount = draw_data->TotalIdxCount = 0;
  draw_data->DisplayPos = viewport->Pos;
  draw_data->DisplaySize = is_minimized ? Vec2(0.0f, 0.0f) : viewport->Size;
  draw_data->FramebufferScale =
      io.DisplayFramebufferScale; // FIXME-VIEWPORT: This may vary on a
                                  // per-monitor/viewport basis?
  draw_data->OwnerViewport = viewport;
}

// Push a clipping rectangle for both Gui logic (hit-testing etc.) and
// low-level DrawList rendering.
// - When using this function it is sane to ensure that float are perfectly
// rounded to integer values,
//   so that e.g. (int)(max.x-min.x) in user's render produce correct result.
// - If the code here changes, may need to update code of functions like
// NextColumn() and PushColumnClipRect():
//   some frequently called functions which to modify both channels and clipping
//   simultaneously tend to use the more specialized
//   SetWindowClipRectBeforeSetChannel() to avoid extraneous updates of
//   underlying DrawCmds.
void Gui::PushClipRect(const Vec2 &clip_rect_min, const Vec2 &clip_rect_max,
                       bool intersect_with_current_clip_rect) {
  Window *window = GetCurrentWindow();
  window->DrawList->PushClipRect(clip_rect_min, clip_rect_max,
                                 intersect_with_current_clip_rect);
  window->ClipRect = window->DrawList->_ClipRectStack.back();
}

void Gui::PopClipRect() {
  Window *window = GetCurrentWindow();
  window->DrawList->PopClipRect();
  window->ClipRect = window->DrawList->_ClipRectStack.back();
}

static Window *FindFrontMostVisibleChildWindow(Window *window) {
  for (int n = window->DC.ChildWindows.Size - 1; n >= 0; n--)
    if (IsWindowActiveAndVisible(window->DC.ChildWindows[n]))
      return FindFrontMostVisibleChildWindow(window->DC.ChildWindows[n]);
  return window;
}

static void Gui::RenderDimmedBackgroundBehindWindow(Window *window, U32 col) {
  if ((col & COL32_A_MASK) == 0)
    return;

  ViewportP *viewport = window->Viewport;
  Rect viewport_rect = viewport->GetMainRect();

  // Draw behind window by moving the draw command at the FRONT of the draw list
  {
    // Draw list have been trimmed already, hence the explicit recreation of a
    // draw command if missing.
    // FIXME: This is creating complication, might be simpler if we could inject
    // a drawlist in drawdata at a given position and not attempt to manipulate
    // DrawCmd order.
    DrawList *draw_list = window->RootWindowDockTree->DrawList;
    draw_list->ChannelsMerge();
    if (draw_list->CmdBuffer.Size == 0)
      draw_list->AddDrawCmd();
    draw_list->PushClipRect(
        viewport_rect.Min - Vec2(1, 1), viewport_rect.Max + Vec2(1, 1),
        false); // FIXME: Need to stricty ensure DrawCmd are not merged
                // (ElemCount==6 checks below will verify that)
    draw_list->AddRectFilled(viewport_rect.Min, viewport_rect.Max, col);
    DrawCmd cmd = draw_list->CmdBuffer.back();
    ASSERT(cmd.ElemCount == 6);
    draw_list->CmdBuffer.pop_back();
    draw_list->CmdBuffer.push_front(cmd);
    draw_list->AddDrawCmd(); // We need to create a command as
                             // CmdBuffer.back().IdxOffset won't be correct if
                             // we append to same command.
    draw_list->PopClipRect();
  }

  // Draw over sibling docking nodes in a same docking tree
  if (window->RootWindow->DockIsActive) {
    DrawList *draw_list =
        FindFrontMostVisibleChildWindow(window->RootWindowDockTree)->DrawList;
    draw_list->ChannelsMerge();
    if (draw_list->CmdBuffer.Size == 0)
      draw_list->AddDrawCmd();
    draw_list->PushClipRect(viewport_rect.Min, viewport_rect.Max, false);
    RenderRectFilledWithHole(
        draw_list, window->RootWindowDockTree->Rect(),
        window->RootWindow->Rect(), col,
        0.0f); // window->RootWindowDockTree->WindowRounding);
    draw_list->PopClipRect();
  }
}

Window *
Gui::FindBottomMostVisibleWindowWithinBeginStack(Window *parent_window) {
  Context &g = *GGui;
  Window *bottom_most_visible_window = parent_window;
  for (int i = FindWindowDisplayIndex(parent_window); i >= 0; i--) {
    Window *window = g.Windows[i];
    if (window->Flags & WindowFlags_ChildWindow)
      continue;
    if (!IsWindowWithinBeginStackOf(window, parent_window))
      break;
    if (IsWindowActiveAndVisible(window) &&
        GetWindowDisplayLayer(window) <= GetWindowDisplayLayer(parent_window))
      bottom_most_visible_window = window;
  }
  return bottom_most_visible_window;
}

// Important: AddWindowToDrawData() has not been called yet, meaning
// DockNodeHost windows needs a DrawList->ChannelsMerge() before usage. We call
// ChannelsMerge() lazily here at it is faster that doing a full iteration of
// g.Windows[] prior to calling RenderDimmedBackgrounds().
static void Gui::RenderDimmedBackgrounds() {
  Context &g = *GGui;
  Window *modal_window = GetTopMostAndVisiblePopupModal();
  if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
    return;
  const bool dim_bg_for_modal = (modal_window != NULL);
  const bool dim_bg_for_window_list =
      (g.NavWindowingTargetAnim != NULL && g.NavWindowingTargetAnim->Active);
  if (!dim_bg_for_modal && !dim_bg_for_window_list)
    return;

  Viewport *viewports_already_dimmed[2] = {NULL, NULL};
  if (dim_bg_for_modal) {
    // Draw dimming behind modal or a begin stack child, whichever comes first
    // in draw order.
    Window *dim_behind_window =
        FindBottomMostVisibleWindowWithinBeginStack(modal_window);
    RenderDimmedBackgroundBehindWindow(
        dim_behind_window, GetColorU32(Col_ModalWindowDimBg, g.DimBgRatio));
    viewports_already_dimmed[0] = modal_window->Viewport;
  } else if (dim_bg_for_window_list) {
    // Draw dimming behind CTRL+Tab target window and behind CTRL+Tab UI window
    RenderDimmedBackgroundBehindWindow(
        g.NavWindowingTargetAnim,
        GetColorU32(Col_NavWindowingDimBg, g.DimBgRatio));
    if (g.NavWindowingListWindow != NULL &&
        g.NavWindowingListWindow->Viewport &&
        g.NavWindowingListWindow->Viewport !=
            g.NavWindowingTargetAnim->Viewport)
      RenderDimmedBackgroundBehindWindow(
          g.NavWindowingListWindow,
          GetColorU32(Col_NavWindowingDimBg, g.DimBgRatio));
    viewports_already_dimmed[0] = g.NavWindowingTargetAnim->Viewport;
    viewports_already_dimmed[1] =
        g.NavWindowingListWindow ? g.NavWindowingListWindow->Viewport : NULL;

    // Draw border around CTRL+Tab target window
    Window *window = g.NavWindowingTargetAnim;
    Viewport *viewport = window->Viewport;
    float distance = g.FontSize;
    Rect bb = window->Rect();
    bb.Expand(distance);
    if (bb.GetWidth() >= viewport->Size.x && bb.GetHeight() >= viewport->Size.y)
      bb.Expand(-distance - 1.0f); // If a window fits the entire viewport,
                                   // adjust its highlight inward
    window->DrawList->ChannelsMerge();
    if (window->DrawList->CmdBuffer.Size == 0)
      window->DrawList->AddDrawCmd();
    window->DrawList->PushClipRect(viewport->Pos,
                                   viewport->Pos + viewport->Size);
    window->DrawList->AddRect(
        bb.Min, bb.Max,
        GetColorU32(Col_NavWindowingHighlight, g.NavWindowingHighlightAlpha),
        window->WindowRounding, 0, 3.0f);
    window->DrawList->PopClipRect();
  }

  // Draw dimming background on _other_ viewports than the ones our windows are
  // in
  for (ViewportP *viewport : g.Viewports) {
    if (viewport == viewports_already_dimmed[0] ||
        viewport == viewports_already_dimmed[1])
      continue;
    if (modal_window && viewport->Window &&
        IsWindowAbove(viewport->Window, modal_window))
      continue;
    DrawList *draw_list = GetForegroundDrawList(viewport);
    const U32 dim_bg_col = GetColorU32(dim_bg_for_modal ? Col_ModalWindowDimBg
                                                        : Col_NavWindowingDimBg,
                                       g.DimBgRatio);
    draw_list->AddRectFilled(viewport->Pos, viewport->Pos + viewport->Size,
                             dim_bg_col);
  }
}

// This is normally called by Render(). You may want to call it directly if you
// want to avoid calling Render() but the gain will be very minimal.
void Gui::EndFrame() {
  Context &g = *GGui;
  ASSERT(g.Initialized);

  // Don't process EndFrame() multiple times.
  if (g.FrameCountEnded == g.FrameCount)
    return;
  ASSERT(g.WithinFrameScope && "Forgot to call Gui::NewFrame()?");

  CallContextHooks(&g, ContextHookType_EndFramePre);

  ErrorCheckEndFrameSanityChecks();

  // Notify Platform/OS when our Input Method Editor cursor has moved (e.g. CJK
  // inputs using Microsoft IME)
  PlatformImeData *ime_data = &g.PlatformImeData;
  if (g.IO.SetPlatformImeDataFn &&
      memcmp(ime_data, &g.PlatformImeDataPrev, sizeof(PlatformImeData)) != 0) {
    Viewport *viewport = FindViewportByID(g.PlatformImeViewport);
    DEBUG_LOG_IO("[io] Calling io.SetPlatformImeDataFn(): WantVisible: %d, "
                 "InputPos (%.2f,%.2f)\n",
                 ime_data->WantVisible, ime_data->InputPos.x,
                 ime_data->InputPos.y);
    if (viewport == NULL)
      viewport = GetMainViewport();
#ifndef DISABLE_OBSOLETE_FUNCTIONS
    if (viewport->PlatformHandleRaw == NULL && g.IO.ImeWindowHandle != NULL) {
      viewport->PlatformHandleRaw = g.IO.ImeWindowHandle;
      g.IO.SetPlatformImeDataFn(viewport, ime_data);
      viewport->PlatformHandleRaw = NULL;
    } else
#endif
    {
      g.IO.SetPlatformImeDataFn(viewport, ime_data);
    }
  }

  // Hide implicit/fallback "Debug" window if it hasn't been used
  g.WithinFrameScopeWithImplicitWindow = false;
  if (g.CurrentWindow && !g.CurrentWindow->WriteAccessed)
    g.CurrentWindow->Active = false;
  End();

  // Update navigation: CTRL+Tab, wrap-around requests
  NavEndFrame();

  // Update docking
  DockContextEndFrame(&g);

  SetCurrentViewport(NULL, NULL);

  // Drag and Drop: Elapse payload (if delivered, or if source stops being
  // submitted)
  if (g.DragDropActive) {
    bool is_delivered = g.DragDropPayload.Delivery;
    bool is_elapsed =
        (g.DragDropPayload.DataFrameCount + 1 < g.FrameCount) &&
        ((g.DragDropSourceFlags & DragDropFlags_SourceAutoExpirePayload) ||
         !IsMouseDown(g.DragDropMouseButton));
    if (is_delivered || is_elapsed)
      ClearDragDrop();
  }

  // Drag and Drop: Fallback for source tooltip. This is not ideal but better
  // than nothing.
  if (g.DragDropActive && g.DragDropSourceFrameCount < g.FrameCount &&
      !(g.DragDropSourceFlags & DragDropFlags_SourceNoPreviewTooltip)) {
    g.DragDropWithinSource = true;
    SetTooltip("...");
    g.DragDropWithinSource = false;
  }

  // End frame
  g.WithinFrameScope = false;
  g.FrameCountEnded = g.FrameCount;

  // Initiate moving window + handle left-click and right-click focus
  UpdateMouseMovingWindowEndFrame();

  // Update user-facing viewport list (g.Viewports -> g.PlatformIO.Viewports
  // after filtering out some)
  UpdateViewportsEndFrame();

  // Sort the window list so that all child windows are after their parent
  // We cannot do that on FocusWindow() because children may not exist yet
  g.WindowsTempSortBuffer.resize(0);
  g.WindowsTempSortBuffer.reserve(g.Windows.Size);
  for (Window *window : g.Windows) {
    if (window->Active &&
        (window->Flags & WindowFlags_ChildWindow)) // if a child is active its
                                                   // parent will add it
      continue;
    AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
  }

  // This usually assert if there is a mismatch between the
  // WindowFlags_ChildWindow / ParentWindow values and DC.ChildWindows[] in
  // parents, aka we've done something wrong.
  ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size);
  g.Windows.swap(g.WindowsTempSortBuffer);
  g.IO.MetricsActiveWindows = g.WindowsActiveCount;

  // Unlock font atlas
  g.IO.Fonts->Locked = false;

  // Clear Input data for next frame
  g.IO.MousePosPrev = g.IO.MousePos;
  g.IO.AppFocusLost = false;
  g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
  g.IO.InputQueueCharacters.resize(0);

  CallContextHooks(&g, ContextHookType_EndFramePost);
}

// Prepare the data for rendering so you can call GetDrawData()
// (As with anything within the Gui:: namspace this doesn't touch your GPU or
// graphics API at all: it is the role of the XXXX_RenderDrawData()
// function provided by the renderer backend)
void Gui::Render() {
  Context &g = *GGui;
  ASSERT(g.Initialized);

  if (g.FrameCountEnded != g.FrameCount)
    EndFrame();
  if (g.FrameCountRendered == g.FrameCount)
    return;
  g.FrameCountRendered = g.FrameCount;

  g.IO.MetricsRenderWindows = 0;
  CallContextHooks(&g, ContextHookType_RenderPre);

  // Add background DrawList (for each active viewport)
  for (ViewportP *viewport : g.Viewports) {
    InitViewportDrawData(viewport);
    if (viewport->BgFgDrawLists[0] != NULL)
      AddDrawListToDrawDataEx(&viewport->DrawDataP,
                              viewport->DrawDataBuilder.Layers[0],
                              GetBackgroundDrawList(viewport));
  }

  // Draw modal/window whitening backgrounds
  RenderDimmedBackgrounds();

  // Add DrawList to render
  Window *windows_to_render_top_most[2];
  windows_to_render_top_most[0] =
      (g.NavWindowingTarget &&
       !(g.NavWindowingTarget->Flags & WindowFlags_NoBringToFrontOnFocus))
          ? g.NavWindowingTarget->RootWindowDockTree
          : NULL;
  windows_to_render_top_most[1] =
      (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
  for (Window *window : g.Windows) {
    MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning
                                 // C6011: Dereferencing NULL pointer 'window'"
    if (IsWindowActiveAndVisible(window) &&
        (window->Flags & WindowFlags_ChildWindow) == 0 &&
        window != windows_to_render_top_most[0] &&
        window != windows_to_render_top_most[1])
      AddRootWindowToDrawData(window);
  }
  for (int n = 0; n < ARRAYSIZE(windows_to_render_top_most); n++)
    if (windows_to_render_top_most[n] &&
        IsWindowActiveAndVisible(
            windows_to_render_top_most[n])) // NavWindowingTarget is always
                                            // temporarily displayed as the
                                            // top-most window
      AddRootWindowToDrawData(windows_to_render_top_most[n]);

  // Draw software mouse cursor if requested by io.MouseDrawCursor flag
  if (g.IO.MouseDrawCursor && g.MouseCursor != MouseCursor_None)
    RenderMouseCursor(g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor,
                      COL32_WHITE, COL32_BLACK, COL32(0, 0, 0, 48));

  // Setup DrawData structures for end-user
  g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
  for (ViewportP *viewport : g.Viewports) {
    FlattenDrawDataIntoSingleLayer(&viewport->DrawDataBuilder);

    // Add foreground DrawList (for each active viewport)
    if (viewport->BgFgDrawLists[1] != NULL)
      AddDrawListToDrawDataEx(&viewport->DrawDataP,
                              viewport->DrawDataBuilder.Layers[0],
                              GetForegroundDrawList(viewport));

    // We call _PopUnusedDrawCmd() last thing, as RenderDimmedBackgrounds() rely
    // on a valid command being there (especially in docking branch).
    DrawData *draw_data = &viewport->DrawDataP;
    ASSERT(draw_data->CmdLists.Size == draw_data->CmdListsCount);
    for (DrawList *draw_list : draw_data->CmdLists)
      draw_list->_PopUnusedDrawCmd();

    g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
    g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
  }

  CallContextHooks(&g, ContextHookType_RenderPost);
}

// Calculate text size. Text can be multi-line. Optionally ignore text after a
// ## marker. CalcTextSize("") should return Vec2(0.0f, g.FontSize)
Vec2 Gui::CalcTextSize(const char *text, const char *text_end,
                       bool hide_text_after_double_hash, float wrap_width) {
  Context &g = *GGui;

  const char *text_display_end;
  if (hide_text_after_double_hash)
    text_display_end = FindRenderedTextEnd(
        text, text_end); // Hide anything after a '##' string
  else
    text_display_end = text_end;

  Font *font = g.Font;
  const float font_size = g.FontSize;
  if (text == text_display_end)
    return Vec2(0.0f, font_size);
  Vec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text,
                                       text_display_end, NULL);

  // Round
  // FIXME: This has been here since Dec 2015 (7b0bf230) but down the line we
  // want this out.
  // FIXME: Investigate using ceilf or e.g.
  // - https://git.musl-libc.org/cgit/musl/tree/src/math/ceilf.c
  // - https://embarkstudios.github.io/rust-gpu/api/src/libm/math/ceilf.rs.html
  text_size.x = TRUNC(text_size.x + 0.99999f);

  return text_size;
}

// Find window given position, search front-to-back
// FIXME: Note that we have an inconsequential lag here: OuterRectClipped is
// updated in Begin(), so windows moved programmatically with SetWindowPos() and
// not SetNextWindowPos() will have that rectangle lagging by a frame at the
// time FindHoveredWindow() is called, aka before the next Begin(). Moving
// window isn't affected.
static void FindHoveredWindow() {
  Context &g = *GGui;

  // Special handling for the window being moved: Ignore the mouse viewport
  // check (because it may reset/lose its viewport during the undocking frame)
  ViewportP *moving_window_viewport =
      g.MovingWindow ? g.MovingWindow->Viewport : NULL;
  if (g.MovingWindow)
    g.MovingWindow->Viewport = g.MouseViewport;

  Window *hovered_window = NULL;
  Window *hovered_window_ignoring_moving_window = NULL;
  if (g.MovingWindow && !(g.MovingWindow->Flags & WindowFlags_NoMouseInputs))
    hovered_window = g.MovingWindow;

  Vec2 padding_regular = g.Style.TouchExtraPadding;
  Vec2 padding_for_resize = g.IO.ConfigWindowsResizeFromEdges
                                ? g.WindowsHoverPadding
                                : padding_regular;
  for (int i = g.Windows.Size - 1; i >= 0; i--) {
    Window *window = g.Windows[i];
    MSVC_WARNING_SUPPRESS(
        28182); // [Static Analyzer] Dereferencing NULL pointer.
    if (!window->Active || window->Hidden)
      continue;
    if (window->Flags & WindowFlags_NoMouseInputs)
      continue;
    ASSERT(window->Viewport);
    if (window->Viewport != g.MouseViewport)
      continue;

    // Using the clipped AABB, a child window will typically be clipped by its
    // parent (not always)
    Vec2 hit_padding =
        (window->Flags & (WindowFlags_NoResize | WindowFlags_AlwaysAutoResize))
            ? padding_regular
            : padding_for_resize;
    if (!window->OuterRectClipped.ContainsWithPad(g.IO.MousePos, hit_padding))
      continue;

    // Support for one rectangular hole in any given window
    // FIXME: Consider generalizing hit-testing override (with more generic
    // data, callback, etc.) (#1512)
    if (window->HitTestHoleSize.x != 0) {
      Vec2 hole_pos(window->Pos.x + (float)window->HitTestHoleOffset.x,
                    window->Pos.y + (float)window->HitTestHoleOffset.y);
      Vec2 hole_size((float)window->HitTestHoleSize.x,
                     (float)window->HitTestHoleSize.y);
      if (Rect(hole_pos, hole_pos + hole_size).Contains(g.IO.MousePos))
        continue;
    }

    if (hovered_window == NULL)
      hovered_window = window;
    MSVC_WARNING_SUPPRESS(
        28182); // [Static Analyzer] Dereferencing NULL pointer.
    if (hovered_window_ignoring_moving_window == NULL &&
        (!g.MovingWindow ||
         window->RootWindowDockTree != g.MovingWindow->RootWindowDockTree))
      hovered_window_ignoring_moving_window = window;
    if (hovered_window && hovered_window_ignoring_moving_window)
      break;
  }

  g.HoveredWindow = hovered_window;
  g.HoveredWindowUnderMovingWindow = hovered_window_ignoring_moving_window;

  if (g.MovingWindow)
    g.MovingWindow->Viewport = moving_window_viewport;
}

bool Gui::IsItemActive() {
  Context &g = *GGui;
  if (g.ActiveId)
    return g.ActiveId == g.LastItemData.ID;
  return false;
}

bool Gui::IsItemActivated() {
  Context &g = *GGui;
  if (g.ActiveId)
    if (g.ActiveId == g.LastItemData.ID &&
        g.ActiveIdPreviousFrame != g.LastItemData.ID)
      return true;
  return false;
}

bool Gui::IsItemDeactivated() {
  Context &g = *GGui;
  if (g.LastItemData.StatusFlags & ItemStatusFlags_HasDeactivated)
    return (g.LastItemData.StatusFlags & ItemStatusFlags_Deactivated) != 0;
  return (g.ActiveIdPreviousFrame == g.LastItemData.ID &&
          g.ActiveIdPreviousFrame != 0 && g.ActiveId != g.LastItemData.ID);
}

bool Gui::IsItemDeactivatedAfterEdit() {
  Context &g = *GGui;
  return IsItemDeactivated() &&
         (g.ActiveIdPreviousFrameHasBeenEditedBefore ||
          (g.ActiveId == 0 && g.ActiveIdHasBeenEditedBefore));
}

// == GetItemID() == GetFocusID()
bool Gui::IsItemFocused() {
  Context &g = *GGui;
  if (g.NavId != g.LastItemData.ID || g.NavId == 0)
    return false;

  // Special handling for the dummy item after Begin() which represent the title
  // bar or tab. When the window is collapsed (SkipItems==true) that last item
  // will never be overwritten so we need to detect the case.
  Window *window = g.CurrentWindow;
  if (g.LastItemData.ID == window->ID && window->WriteAccessed)
    return false;

  return true;
}

// Important: this can be useful but it is NOT equivalent to the behavior of
// e.g.Button()! Most widgets have specific reactions based on mouse-up/down
// state, mouse position etc.
bool Gui::IsItemClicked(MouseButton mouse_button) {
  return IsMouseClicked(mouse_button) && IsItemHovered(HoveredFlags_None);
}

bool Gui::IsItemToggledOpen() {
  Context &g = *GGui;
  return (g.LastItemData.StatusFlags & ItemStatusFlags_ToggledOpen) ? true
                                                                    : false;
}

bool Gui::IsItemToggledSelection() {
  Context &g = *GGui;
  return (g.LastItemData.StatusFlags & ItemStatusFlags_ToggledSelection)
             ? true
             : false;
}

// IMPORTANT: If you are trying to check whether your mouse should be dispatched
// to Gui or to your underlying app, you should not use this function!
// Use the 'io.WantCaptureMouse' boolean for that! Refer to FAQ entry "How can I
// tell whether to dispatch mouse/keyboard to Gui or my application?" for
// details.
bool Gui::IsAnyItemHovered() {
  Context &g = *GGui;
  return g.HoveredId != 0 || g.HoveredIdPreviousFrame != 0;
}

bool Gui::IsAnyItemActive() {
  Context &g = *GGui;
  return g.ActiveId != 0;
}

bool Gui::IsAnyItemFocused() {
  Context &g = *GGui;
  return g.NavId != 0 && !g.NavDisableHighlight;
}

bool Gui::IsItemVisible() {
  Context &g = *GGui;
  return (g.LastItemData.StatusFlags & ItemStatusFlags_Visible) != 0;
}

bool Gui::IsItemEdited() {
  Context &g = *GGui;
  return (g.LastItemData.StatusFlags & ItemStatusFlags_Edited) != 0;
}

// Allow next item to be overlapped by subsequent items.
// This works by requiring HoveredId to match for two subsequent frames,
// so if a following items overwrite it our interactions will naturally be
// disabled.
void Gui::SetNextItemAllowOverlap() {
  Context &g = *GGui;
  g.NextItemData.ItemFlags |= ItemFlags_AllowOverlap;
}

#ifndef DISABLE_OBSOLETE_FUNCTIONS
// Allow last item to be overlapped by a subsequent item. Both may be activated
// during the same frame before the later one takes priority.
// FIXME-LEGACY: Use SetNextItemAllowOverlap() *before* your item instead.
void Gui::SetItemAllowOverlap() {
  Context &g = *GGui;
  ID id = g.LastItemData.ID;
  if (g.HoveredId == id)
    g.HoveredIdAllowOverlap = true;
  if (g.ActiveId ==
      id) // Before we made this obsolete, most calls to SetItemAllowOverlap()
          // used to avoid this path by testing g.ActiveId != id.
    g.ActiveIdAllowOverlap = true;
}
#endif

// FIXME: It might be undesirable that this will likely disable KeyOwner-aware
// shortcuts systems. Consider a more fine-tuned version for the two users of
// this function.
void Gui::SetActiveIdUsingAllKeyboardKeys() {
  Context &g = *GGui;
  ASSERT(g.ActiveId != 0);
  g.ActiveIdUsingNavDirMask = (1 << Dir_COUNT) - 1;
  g.ActiveIdUsingAllKeyboardKeys = true;
  NavMoveRequestCancel();
}

ID Gui::GetItemID() {
  Context &g = *GGui;
  return g.LastItemData.ID;
}

Vec2 Gui::GetItemRectMin() {
  Context &g = *GGui;
  return g.LastItemData.Rect.Min;
}

Vec2 Gui::GetItemRectMax() {
  Context &g = *GGui;
  return g.LastItemData.Rect.Max;
}

Vec2 Gui::GetItemRectSize() {
  Context &g = *GGui;
  return g.LastItemData.Rect.GetSize();
}

// Prior to v1.90 2023/10/16, the BeginChild() function took a 'bool border =
// false' parameter instead of 'ChildFlags child_flags = 0'.
// ChildFlags_Border is defined as always == 1 in order to allow old code
// passing 'true'.
bool Gui::BeginChild(const char *str_id, const Vec2 &size_arg,
                     ChildFlags child_flags, WindowFlags window_flags) {
  ID id = GetCurrentWindow()->GetID(str_id);
  return BeginChildEx(str_id, id, size_arg, child_flags, window_flags);
}

bool Gui::BeginChild(ID id, const Vec2 &size_arg, ChildFlags child_flags,
                     WindowFlags window_flags) {
  return BeginChildEx(NULL, id, size_arg, child_flags, window_flags);
}

bool Gui::BeginChildEx(const char *name, ID id, const Vec2 &size_arg,
                       ChildFlags child_flags, WindowFlags window_flags) {
  Context &g = *GGui;
  Window *parent_window = g.CurrentWindow;
  ASSERT(id != 0);

  // Sanity check as it is likely that some user will accidentally pass
  // WindowFlags into the ChildFlags argument.
  const ChildFlags ChildFlags_SupportedMask_ =
      ChildFlags_Border | ChildFlags_AlwaysUseWindowPadding |
      ChildFlags_ResizeX | ChildFlags_ResizeY | ChildFlags_AutoResizeX |
      ChildFlags_AutoResizeY | ChildFlags_AlwaysAutoResize |
      ChildFlags_FrameStyle;
  UNUSED(ChildFlags_SupportedMask_);
  ASSERT((child_flags & ~ChildFlags_SupportedMask_) == 0 &&
         "Illegal ChildFlags value. Did you pass WindowFlags "
         "values instead of ChildFlags?");
  ASSERT((window_flags & WindowFlags_AlwaysAutoResize) == 0 &&
         "Cannot specify WindowFlags_AlwaysAutoResize for "
         "BeginChild(). Use ChildFlags_AlwaysAutoResize!");
  if (child_flags & ChildFlags_AlwaysAutoResize) {
    ASSERT((child_flags & (ChildFlags_ResizeX | ChildFlags_ResizeY)) == 0 &&
           "Cannot use ChildFlags_ResizeX or ChildFlags_ResizeY "
           "with ChildFlags_AlwaysAutoResize!");
    ASSERT((child_flags & (ChildFlags_AutoResizeX | ChildFlags_AutoResizeY)) !=
               0 &&
           "Must use ChildFlags_AutoResizeX or ChildFlags_AutoResizeY "
           "with ChildFlags_AlwaysAutoResize!");
  }
#ifndef DISABLE_OBSOLETE_FUNCTIONS
  if (window_flags & WindowFlags_AlwaysUseWindowPadding)
    child_flags |= ChildFlags_AlwaysUseWindowPadding;
#endif
  if (child_flags & ChildFlags_AutoResizeX)
    child_flags &= ~ChildFlags_ResizeX;
  if (child_flags & ChildFlags_AutoResizeY)
    child_flags &= ~ChildFlags_ResizeY;

  // Set window flags
  window_flags |=
      WindowFlags_ChildWindow | WindowFlags_NoTitleBar | WindowFlags_NoDocking;
  window_flags |=
      (parent_window->Flags & WindowFlags_NoMove); // Inherit the NoMove flag
  if (child_flags & (ChildFlags_AutoResizeX | ChildFlags_AutoResizeY |
                     ChildFlags_AlwaysAutoResize))
    window_flags |= WindowFlags_AlwaysAutoResize;
  if ((child_flags & (ChildFlags_ResizeX | ChildFlags_ResizeY)) == 0)
    window_flags |= WindowFlags_NoResize | WindowFlags_NoSavedSettings;

  // Special framed style
  if (child_flags & ChildFlags_FrameStyle) {
    PushStyleColor(Col_ChildBg, g.Style.Colors[Col_FrameBg]);
    PushStyleVar(StyleVar_ChildRounding, g.Style.FrameRounding);
    PushStyleVar(StyleVar_ChildBorderSize, g.Style.FrameBorderSize);
    PushStyleVar(StyleVar_WindowPadding, g.Style.FramePadding);
    child_flags |= ChildFlags_Border | ChildFlags_AlwaysUseWindowPadding;
    window_flags |= WindowFlags_NoMove;
  }

  // Forward child flags
  g.NextWindowData.Flags |= NextWindowDataFlags_HasChildFlags;
  g.NextWindowData.ChildFlags = child_flags;

  // Forward size
  // Important: Begin() has special processing to switch condition to
  // Cond_FirstUseEver for a given axis when ChildFlags_ResizeXXX is
  // set. (the alternative would to store conditional flags per axis, which is
  // possible but more code)
  const Vec2 size_avail = GetContentRegionAvail();
  const Vec2 size_default(
      (child_flags & ChildFlags_AutoResizeX) ? 0.0f : size_avail.x,
      (child_flags & ChildFlags_AutoResizeY) ? 0.0f : size_avail.y);
  const Vec2 size = CalcItemSize(size_arg, size_default.x, size_default.y);
  SetNextWindowSize(size);

  // Build up name. If you need to append to a same child from multiple location
  // in the ID stack, use BeginChild(ID id) with a stable value.
  // FIXME: 2023/11/14: commented out shorted version. We had an issue with
  // multiple ### in child window path names, which the trailing hash helped
  // workaround. e.g.
  // "ParentName###ParentIdentifier/ChildName###ChildIdentifier" would get
  // hashed incorrectly by HashStr(), trailing _%08X somehow fixes it.
  const char *temp_window_name;
  /*if (name && parent_window->IDStack.back() == parent_window->ID)
      FormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s",
  parent_window->Name, name); // May omit ID if in root of ID stack else*/
  if (name)
    FormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X",
                             parent_window->Name, name, id);
  else
    FormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X",
                             parent_window->Name, id);

  // Set style
  const float backup_border_size = g.Style.ChildBorderSize;
  if ((child_flags & ChildFlags_Border) == 0)
    g.Style.ChildBorderSize = 0.0f;

  // Begin into window
  const bool ret = Begin(temp_window_name, NULL, window_flags);

  // Restore style
  g.Style.ChildBorderSize = backup_border_size;
  if (child_flags & ChildFlags_FrameStyle) {
    PopStyleVar(3);
    PopStyleColor();
  }

  Window *child_window = g.CurrentWindow;
  child_window->ChildId = id;

  // Set the cursor to handle case where the user called
  // SetNextWindowPos()+BeginChild() manually. While this is not really
  // documented/defined, it seems that the expected thing to do.
  if (child_window->BeginCount == 1)
    parent_window->DC.CursorPos = child_window->Pos;

  // Process navigation-in immediately so NavInit can run on first frame
  // Can enter a child if (A) it has navigable items or (B) it can be scrolled.
  const ID temp_id_for_activation = HashStr("##Child", 0, id);
  if (g.ActiveId == temp_id_for_activation)
    ClearActiveID();
  if (g.NavActivateId == id && !(window_flags & WindowFlags_NavFlattened) &&
      (child_window->DC.NavLayersActiveMask != 0 ||
       child_window->DC.NavWindowHasScrollY)) {
    FocusWindow(child_window);
    NavInitWindow(child_window, false);
    SetActiveID(temp_id_for_activation,
                child_window); // Steal ActiveId with another arbitrary id so
                               // that key-press won't activate child item
    g.ActiveIdSource = g.NavInputSource;
  }
  return ret;
}

void Gui::EndChild() {
  Context &g = *GGui;
  Window *child_window = g.CurrentWindow;

  ASSERT(g.WithinEndChild == false);
  ASSERT(child_window->Flags &
         WindowFlags_ChildWindow); // Mismatched BeginChild()/EndChild() calls

  g.WithinEndChild = true;
  Vec2 child_size = child_window->Size;
  End();
  if (child_window->BeginCount == 1) {
    Window *parent_window = g.CurrentWindow;
    Rect bb(parent_window->DC.CursorPos,
            parent_window->DC.CursorPos + child_size);
    ItemSize(child_size);
    if ((child_window->DC.NavLayersActiveMask != 0 ||
         child_window->DC.NavWindowHasScrollY) &&
        !(child_window->Flags & WindowFlags_NavFlattened)) {
      ItemAdd(bb, child_window->ChildId);
      RenderNavHighlight(bb, child_window->ChildId);

      // When browsing a window that has no activable items (scroll only) we
      // keep a highlight on the child (pass g.NavId to trick into always
      // displaying)
      if (child_window->DC.NavLayersActiveMask == 0 &&
          child_window == g.NavWindow)
        RenderNavHighlight(Rect(bb.Min - Vec2(2, 2), bb.Max + Vec2(2, 2)),
                           g.NavId, NavHighlightFlags_TypeThin);
    } else {
      // Not navigable into
      ItemAdd(bb, 0);

      // But when flattened we directly reach items, adjust active layer mask
      // accordingly
      if (child_window->Flags & WindowFlags_NavFlattened)
        parent_window->DC.NavLayersActiveMaskNext |=
            child_window->DC.NavLayersActiveMaskNext;
    }
    if (g.HoveredWindow == child_window)
      g.LastItemData.StatusFlags |= ItemStatusFlags_HoveredWindow;
  }
  g.WithinEndChild = false;
  g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

static void SetWindowConditionAllowFlags(Window *window, Cond flags,
                                         bool enabled) {
  window->SetWindowPosAllowFlags =
      enabled ? (window->SetWindowPosAllowFlags | flags)
              : (window->SetWindowPosAllowFlags & ~flags);
  window->SetWindowSizeAllowFlags =
      enabled ? (window->SetWindowSizeAllowFlags | flags)
              : (window->SetWindowSizeAllowFlags & ~flags);
  window->SetWindowCollapsedAllowFlags =
      enabled ? (window->SetWindowCollapsedAllowFlags | flags)
              : (window->SetWindowCollapsedAllowFlags & ~flags);
  window->SetWindowDockAllowFlags =
      enabled ? (window->SetWindowDockAllowFlags | flags)
              : (window->SetWindowDockAllowFlags & ~flags);
}

Window *Gui::FindWindowByID(ID id) {
  Context &g = *GGui;
  return (Window *)g.WindowsById.GetVoidPtr(id);
}

Window *Gui::FindWindowByName(const char *name) {
  ID id = HashStr(name);
  return FindWindowByID(id);
}

static void ApplyWindowSettings(Window *window, WindowSettings *settings) {
  const Viewport *main_viewport = Gui::GetMainViewport();
  window->ViewportPos = main_viewport->Pos;
  if (settings->ViewportId) {
    window->ViewportId = settings->ViewportId;
    window->ViewportPos =
        Vec2(settings->ViewportPos.x, settings->ViewportPos.y);
  }
  window->Pos = Trunc(Vec2(settings->Pos.x + window->ViewportPos.x,
                           settings->Pos.y + window->ViewportPos.y));
  if (settings->Size.x > 0 && settings->Size.y > 0)
    window->Size = window->SizeFull =
        Trunc(Vec2(settings->Size.x, settings->Size.y));
  window->Collapsed = settings->Collapsed;
  window->DockId = settings->DockId;
  window->DockOrder = settings->DockOrder;
}

static void UpdateWindowInFocusOrderList(Window *window, bool just_created,
                                         WindowFlags new_flags) {
  Context &g = *GGui;

  const bool new_is_explicit_child =
      (new_flags & WindowFlags_ChildWindow) != 0 &&
      ((new_flags & WindowFlags_Popup) == 0 ||
       (new_flags & WindowFlags_ChildMenu) != 0);
  const bool child_flag_changed =
      new_is_explicit_child != window->IsExplicitChild;
  if ((just_created || child_flag_changed) && !new_is_explicit_child) {
    ASSERT(!g.WindowsFocusOrder.contains(window));
    g.WindowsFocusOrder.push_back(window);
    window->FocusOrder = (short)(g.WindowsFocusOrder.Size - 1);
  } else if (!just_created && child_flag_changed && new_is_explicit_child) {
    ASSERT(g.WindowsFocusOrder[window->FocusOrder] == window);
    for (int n = window->FocusOrder + 1; n < g.WindowsFocusOrder.Size; n++)
      g.WindowsFocusOrder[n]->FocusOrder--;
    g.WindowsFocusOrder.erase(g.WindowsFocusOrder.Data + window->FocusOrder);
    window->FocusOrder = -1;
  }
  window->IsExplicitChild = new_is_explicit_child;
}

static void InitOrLoadWindowSettings(Window *window, WindowSettings *settings) {
  // Initial window state with e.g. default/arbitrary window position
  // Use SetNextWindowPos() with the appropriate condition flag to change the
  // initial position of a window.
  const Viewport *main_viewport = Gui::GetMainViewport();
  window->Pos = main_viewport->Pos + Vec2(60, 60);
  window->Size = window->SizeFull = Vec2(0, 0);
  window->ViewportPos = main_viewport->Pos;
  window->SetWindowPosAllowFlags = window->SetWindowSizeAllowFlags =
      window->SetWindowCollapsedAllowFlags = window->SetWindowDockAllowFlags =
          Cond_Always | Cond_Once | Cond_FirstUseEver | Cond_Appearing;

  if (settings != NULL) {
    SetWindowConditionAllowFlags(window, Cond_FirstUseEver, false);
    ApplyWindowSettings(window, settings);
  }
  window->DC.CursorStartPos = window->DC.CursorMaxPos = window->DC.IdealMaxPos =
      window->Pos; // So first call to CalcWindowContentSizes() doesn't return
                   // crazy values

  if ((window->Flags & WindowFlags_AlwaysAutoResize) != 0) {
    window->AutoFitFramesX = window->AutoFitFramesY = 2;
    window->AutoFitOnlyGrows = false;
  } else {
    if (window->Size.x <= 0.0f)
      window->AutoFitFramesX = 2;
    if (window->Size.y <= 0.0f)
      window->AutoFitFramesY = 2;
    window->AutoFitOnlyGrows =
        (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
  }
}

static Window *CreateNewWindow(const char *name, WindowFlags flags) {
  // Create window the first time
  // DEBUG_LOG("CreateNewWindow '%s', flags = 0x%08X\n", name, flags);
  Context &g = *GGui;
  Window *window = NEW(Window)(&g, name);
  window->Flags = flags;
  g.WindowsById.SetVoidPtr(window->ID, window);

  WindowSettings *settings = NULL;
  if (!(flags & WindowFlags_NoSavedSettings))
    if ((settings = Gui::FindWindowSettingsByWindow(window)) != 0)
      window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);

  InitOrLoadWindowSettings(window, settings);

  if (flags & WindowFlags_NoBringToFrontOnFocus)
    g.Windows.push_front(window); // Quite slow but rare and only once
  else
    g.Windows.push_back(window);

  return window;
}

static Window *GetWindowForTitleDisplay(Window *window) {
  return window->DockNodeAsHost ? window->DockNodeAsHost->VisibleWindow
                                : window;
}

static Window *GetWindowForTitleAndMenuHeight(Window *window) {
  return (window->DockNodeAsHost && window->DockNodeAsHost->VisibleWindow)
             ? window->DockNodeAsHost->VisibleWindow
             : window;
}

static inline Vec2 CalcWindowMinSize(Window *window) {
  // Popups, menus and childs bypass style.WindowMinSize by default, but we give
  // then a non-zero minimum size to facilitate understanding problematic cases
  // (e.g. empty popups)
  // FIXME: the if/else could probably be removed, "reduce artifacts" section
  // for all windows.
  Context &g = *GGui;
  Vec2 size_min;
  if (window->Flags &
      (WindowFlags_Popup | WindowFlags_ChildMenu | WindowFlags_ChildWindow)) {
    size_min.x = (window->ChildFlags & ChildFlags_ResizeX)
                     ? g.Style.WindowMinSize.x
                     : 4.0f;
    size_min.y = (window->ChildFlags & ChildFlags_ResizeY)
                     ? g.Style.WindowMinSize.y
                     : 4.0f;
  } else {
    Window *window_for_height = GetWindowForTitleAndMenuHeight(window);
    size_min.x = ((window->Flags & WindowFlags_AlwaysAutoResize) == 0)
                     ? g.Style.WindowMinSize.x
                     : 4.0f;
    size_min.y = ((window->Flags & WindowFlags_AlwaysAutoResize) == 0)
                     ? g.Style.WindowMinSize.y
                     : 4.0f;
    size_min.y = Max(
        size_min.y,
        window_for_height->TitleBarHeight() +
            window_for_height->MenuBarHeight() +
            Max(0.0f, g.Style.WindowRounding -
                          1.0f)); // Reduce artifacts with very small windows
  }
  return size_min;
}

static Vec2 CalcWindowSizeAfterConstraint(Window *window,
                                          const Vec2 &size_desired) {
  Context &g = *GGui;
  Vec2 new_size = size_desired;
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasSizeConstraint) {
    // See comments in SetNextWindowSizeConstraints() for details about setting
    // size_min an size_max.
    Rect cr = g.NextWindowData.SizeConstraintRect;
    new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0)
                     ? Clamp(new_size.x, cr.Min.x, cr.Max.x)
                     : window->SizeFull.x;
    new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0)
                     ? Clamp(new_size.y, cr.Min.y, cr.Max.y)
                     : window->SizeFull.y;
    if (g.NextWindowData.SizeCallback) {
      SizeCallbackData data;
      data.UserData = g.NextWindowData.SizeCallbackUserData;
      data.Pos = window->Pos;
      data.CurrentSize = window->SizeFull;
      data.DesiredSize = new_size;
      g.NextWindowData.SizeCallback(&data);
      new_size = data.DesiredSize;
    }
    new_size.x = TRUNC(new_size.x);
    new_size.y = TRUNC(new_size.y);
  }

  // Minimum size
  Vec2 size_min = CalcWindowMinSize(window);
  return Max(new_size, size_min);
}

static void CalcWindowContentSizes(Window *window, Vec2 *content_size_current,
                                   Vec2 *content_size_ideal) {
  bool preserve_old_content_sizes = false;
  if (window->Collapsed && window->AutoFitFramesX <= 0 &&
      window->AutoFitFramesY <= 0)
    preserve_old_content_sizes = true;
  else if (window->Hidden && window->HiddenFramesCannotSkipItems == 0 &&
           window->HiddenFramesCanSkipItems > 0)
    preserve_old_content_sizes = true;
  if (preserve_old_content_sizes) {
    *content_size_current = window->ContentSize;
    *content_size_ideal = window->ContentSizeIdeal;
    return;
  }

  content_size_current->x =
      (window->ContentSizeExplicit.x != 0.0f)
          ? window->ContentSizeExplicit.x
          : TRUNC(window->DC.CursorMaxPos.x - window->DC.CursorStartPos.x);
  content_size_current->y =
      (window->ContentSizeExplicit.y != 0.0f)
          ? window->ContentSizeExplicit.y
          : TRUNC(window->DC.CursorMaxPos.y - window->DC.CursorStartPos.y);
  content_size_ideal->x =
      (window->ContentSizeExplicit.x != 0.0f)
          ? window->ContentSizeExplicit.x
          : TRUNC(Max(window->DC.CursorMaxPos.x, window->DC.IdealMaxPos.x) -
                  window->DC.CursorStartPos.x);
  content_size_ideal->y =
      (window->ContentSizeExplicit.y != 0.0f)
          ? window->ContentSizeExplicit.y
          : TRUNC(Max(window->DC.CursorMaxPos.y, window->DC.IdealMaxPos.y) -
                  window->DC.CursorStartPos.y);
}

static Vec2 CalcWindowAutoFitSize(Window *window, const Vec2 &size_contents) {
  Context &g = *GGui;
  Style &style = g.Style;
  const float decoration_w_without_scrollbars = window->DecoOuterSizeX1 +
                                                window->DecoOuterSizeX2 -
                                                window->ScrollbarSizes.x;
  const float decoration_h_without_scrollbars = window->DecoOuterSizeY1 +
                                                window->DecoOuterSizeY2 -
                                                window->ScrollbarSizes.y;
  Vec2 size_pad = window->WindowPadding * 2.0f;
  Vec2 size_desired =
      size_contents + size_pad +
      Vec2(decoration_w_without_scrollbars, decoration_h_without_scrollbars);
  if (window->Flags & WindowFlags_Tooltip) {
    // Tooltip always resize
    return size_desired;
  } else {
    // Maximum window size is determined by the viewport size or monitor size
    Vec2 size_min = CalcWindowMinSize(window);
    Vec2 size_max =
        (window->ViewportOwned || (window->Flags & WindowFlags_ChildWindow))
            ? Vec2(FLT_MAX, FLT_MAX)
            : window->Viewport->WorkSize - style.DisplaySafeAreaPadding * 2.0f;
    const int monitor_idx = window->ViewportAllowPlatformMonitorExtend;
    if (monitor_idx >= 0 && monitor_idx < g.PlatformIO.Monitors.Size &&
        (window->Flags & WindowFlags_ChildWindow) == 0)
      size_max = g.PlatformIO.Monitors[monitor_idx].WorkSize -
                 style.DisplaySafeAreaPadding * 2.0f;
    Vec2 size_auto_fit = Clamp(size_desired, size_min, Max(size_min, size_max));

    // When the window cannot fit all contents (either because of constraints,
    // either because screen is too small), we are growing the size on the other
    // axis to compensate for expected scrollbar. FIXME: Might turn bigger than
    // ViewportSize-WindowPadding.
    Vec2 size_auto_fit_after_constraint =
        CalcWindowSizeAfterConstraint(window, size_auto_fit);
    bool will_have_scrollbar_x =
        (size_auto_fit_after_constraint.x - size_pad.x -
                 decoration_w_without_scrollbars <
             size_contents.x &&
         !(window->Flags & WindowFlags_NoScrollbar) &&
         (window->Flags & WindowFlags_HorizontalScrollbar)) ||
        (window->Flags & WindowFlags_AlwaysHorizontalScrollbar);
    bool will_have_scrollbar_y =
        (size_auto_fit_after_constraint.y - size_pad.y -
                 decoration_h_without_scrollbars <
             size_contents.y &&
         !(window->Flags & WindowFlags_NoScrollbar)) ||
        (window->Flags & WindowFlags_AlwaysVerticalScrollbar);
    if (will_have_scrollbar_x)
      size_auto_fit.y += style.ScrollbarSize;
    if (will_have_scrollbar_y)
      size_auto_fit.x += style.ScrollbarSize;
    return size_auto_fit;
  }
}

Vec2 Gui::CalcWindowNextAutoFitSize(Window *window) {
  Vec2 size_contents_current;
  Vec2 size_contents_ideal;
  CalcWindowContentSizes(window, &size_contents_current, &size_contents_ideal);
  Vec2 size_auto_fit = CalcWindowAutoFitSize(window, size_contents_ideal);
  Vec2 size_final = CalcWindowSizeAfterConstraint(window, size_auto_fit);
  return size_final;
}

static Col GetWindowBgColorIdx(Window *window) {
  if (window->Flags & (WindowFlags_Tooltip | WindowFlags_Popup))
    return Col_PopupBg;
  if ((window->Flags & WindowFlags_ChildWindow) && !window->DockIsActive)
    return Col_ChildBg;
  return Col_WindowBg;
}

static void CalcResizePosSizeFromAnyCorner(Window *window,
                                           const Vec2 &corner_target,
                                           const Vec2 &corner_norm,
                                           Vec2 *out_pos, Vec2 *out_size) {
  Vec2 pos_min = Lerp(corner_target, window->Pos,
                      corner_norm); // Expected window upper-left
  Vec2 pos_max = Lerp(window->Pos + window->Size, corner_target,
                      corner_norm); // Expected window lower-right
  Vec2 size_expected = pos_max - pos_min;
  Vec2 size_constrained = CalcWindowSizeAfterConstraint(window, size_expected);
  *out_pos = pos_min;
  if (corner_norm.x == 0.0f)
    out_pos->x -= (size_constrained.x - size_expected.x);
  if (corner_norm.y == 0.0f)
    out_pos->y -= (size_constrained.y - size_expected.y);
  *out_size = size_constrained;
}

// Data for resizing from resize grip / corner
struct ResizeGripDef {
  Vec2 CornerPosN;
  Vec2 InnerDir;
  int AngleMin12, AngleMax12;
};
static const ResizeGripDef resize_grip_def[4] = {
    {Vec2(1, 1), Vec2(-1, -1), 0, 3}, // Lower-right
    {Vec2(0, 1), Vec2(+1, -1), 3, 6}, // Lower-left
    {Vec2(0, 0), Vec2(+1, +1), 6, 9}, // Upper-left (Unused)
    {Vec2(1, 0), Vec2(-1, +1), 9, 12} // Upper-right (Unused)
};

// Data for resizing from borders
struct ResizeBorderDef {
  Vec2 InnerDir;             // Normal toward inside
  Vec2 SegmentN1, SegmentN2; // End positions, normalized (0,0: upper left)
  float OuterAngle;          // Angle toward outside
};
static const ResizeBorderDef resize_border_def[4] = {
    {Vec2(+1, 0), Vec2(0, 1), Vec2(0, 0), PI * 1.00f}, // Left
    {Vec2(-1, 0), Vec2(1, 0), Vec2(1, 1), PI * 0.00f}, // Right
    {Vec2(0, +1), Vec2(0, 0), Vec2(1, 0), PI * 1.50f}, // Up
    {Vec2(0, -1), Vec2(1, 1), Vec2(0, 1), PI * 0.50f}  // Down
};

static Rect GetResizeBorderRect(Window *window, int border_n,
                                float perp_padding, float thickness) {
  Rect rect = window->Rect();
  if (thickness == 0.0f)
    rect.Max -= Vec2(1, 1);
  if (border_n == Dir_Left) {
    return Rect(rect.Min.x - thickness, rect.Min.y + perp_padding,
                rect.Min.x + thickness, rect.Max.y - perp_padding);
  }
  if (border_n == Dir_Right) {
    return Rect(rect.Max.x - thickness, rect.Min.y + perp_padding,
                rect.Max.x + thickness, rect.Max.y - perp_padding);
  }
  if (border_n == Dir_Up) {
    return Rect(rect.Min.x + perp_padding, rect.Min.y - thickness,
                rect.Max.x - perp_padding, rect.Min.y + thickness);
  }
  if (border_n == Dir_Down) {
    return Rect(rect.Min.x + perp_padding, rect.Max.y - thickness,
                rect.Max.x - perp_padding, rect.Max.y + thickness);
  }
  ASSERT(0);
  return Rect();
}

// 0..3: corners (Lower-right, Lower-left, Unused, Unused)
ID Gui::GetWindowResizeCornerID(Window *window, int n) {
  ASSERT(n >= 0 && n < 4);
  ID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
  id = HashStr("#RESIZE", 0, id);
  id = HashData(&n, sizeof(int), id);
  return id;
}

// Borders (Left, Right, Up, Down)
ID Gui::GetWindowResizeBorderID(Window *window, Dir dir) {
  ASSERT(dir >= 0 && dir < 4);
  int n = (int)dir + 4;
  ID id = window->DockIsActive ? window->DockNode->HostWindow->ID : window->ID;
  id = HashStr("#RESIZE", 0, id);
  id = HashData(&n, sizeof(int), id);
  return id;
}

// Handle resize for: Resize Grips, Borders, Gamepad
// Return true when using auto-fit (double-click on resize grip)
static int Gui::UpdateWindowManualResize(Window *window,
                                         const Vec2 &size_auto_fit,
                                         int *border_hovered, int *border_held,
                                         int resize_grip_count,
                                         U32 resize_grip_col[4],
                                         const Rect &visibility_rect) {
  Context &g = *GGui;
  WindowFlags flags = window->Flags;

  if ((flags & WindowFlags_NoResize) ||
      (flags & WindowFlags_AlwaysAutoResize) || window->AutoFitFramesX > 0 ||
      window->AutoFitFramesY > 0)
    return false;
  if (window->WasActive ==
      false) // Early out to avoid running this code for e.g. a hidden
             // implicit/fallback Debug window.
    return false;

  int ret_auto_fit_mask = 0x00;
  const float grip_draw_size = TRUNC(Max(
      g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
  const float grip_hover_inner_size = TRUNC(grip_draw_size * 0.75f);
  const float grip_hover_outer_size =
      g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_HOVER_PADDING : 0.0f;

  Rect clamp_rect = visibility_rect;
  const bool window_move_from_title_bar =
      g.IO.ConfigWindowsMoveFromTitleBarOnly &&
      !(window->Flags & WindowFlags_NoTitleBar);
  if (window_move_from_title_bar)
    clamp_rect.Min.y -= window->TitleBarHeight();

  Vec2 pos_target(FLT_MAX, FLT_MAX);
  Vec2 size_target(FLT_MAX, FLT_MAX);

  // Clip mouse interaction rectangles within the viewport rectangle (in
  // practice the narrowing is going to happen most of the time).
  // - Not narrowing would mostly benefit the situation where OS windows
  // _without_ decoration have a threshold for hovering when outside their
  // limits.
  //   This is however not the case with current backends under Win32, but a
  //   custom borderless window implementation would benefit from it.
  // - When decoration are enabled we typically benefit from that distance, but
  // then our resize elements would be conflicting with OS resize elements, so
  // we also narrow.
  // - Note that we are unable to tell if the platform setup allows hovering
  // with a distance threshold (on Win32, decorated window have such threshold).
  // We only clip interaction so we overwrite window->ClipRect, cannot call
  // PushClipRect() yet as DrawList is not yet setup.
  const bool clip_with_viewport_rect =
      !(g.IO.BackendFlags & BackendFlags_HasMouseHoveredViewport) ||
      (g.IO.MouseHoveredViewport != window->ViewportId) ||
      !(window->Viewport->Flags & ViewportFlags_NoDecoration);
  if (clip_with_viewport_rect)
    window->ClipRect = window->Viewport->GetMainRect();

  // Resize grips and borders are on layer 1
  window->DC.NavLayerCurrent = NavLayer_Menu;

  // Manual resize grips
  PushID("#RESIZE");
  for (int resize_grip_n = 0; resize_grip_n < resize_grip_count;
       resize_grip_n++) {
    const ResizeGripDef &def = resize_grip_def[resize_grip_n];
    const Vec2 corner =
        Lerp(window->Pos, window->Pos + window->Size, def.CornerPosN);

    // Using the FlattenChilds button flag we make the resize button accessible
    // even if we are hovering over a child window
    bool hovered, held;
    Rect resize_rect(corner - def.InnerDir * grip_hover_outer_size,
                     corner + def.InnerDir * grip_hover_inner_size);
    if (resize_rect.Min.x > resize_rect.Max.x)
      Swap(resize_rect.Min.x, resize_rect.Max.x);
    if (resize_rect.Min.y > resize_rect.Max.y)
      Swap(resize_rect.Min.y, resize_rect.Max.y);
    ID resize_grip_id =
        window->GetID(resize_grip_n); // == GetWindowResizeCornerID()
    ItemAdd(resize_rect, resize_grip_id, NULL, ItemFlags_NoNav);
    ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held,
                   ButtonFlags_FlattenChildren | ButtonFlags_NoNavFocus);
    // GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max,
    // COL32(255, 255, 0, 255));
    if (hovered || held)
      g.MouseCursor =
          (resize_grip_n & 1) ? MouseCursor_ResizeNESW : MouseCursor_ResizeNWSE;

    if (held && g.IO.MouseDoubleClicked[0]) {
      // Auto-fit when double-clicking
      size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
      ret_auto_fit_mask = 0x03; // Both axises
      ClearActiveID();
    } else if (held) {
      // Resize from any of the four corners
      // We don't use an incremental MouseDelta but rather compute an absolute
      // target size based on mouse position
      Vec2 clamp_min =
          Vec2(def.CornerPosN.x == 1.0f ? clamp_rect.Min.x : -FLT_MAX,
               (def.CornerPosN.y == 1.0f ||
                (def.CornerPosN.y == 0.0f && window_move_from_title_bar))
                   ? clamp_rect.Min.y
                   : -FLT_MAX);
      Vec2 clamp_max =
          Vec2(def.CornerPosN.x == 0.0f ? clamp_rect.Max.x : +FLT_MAX,
               def.CornerPosN.y == 0.0f ? clamp_rect.Max.y : +FLT_MAX);
      Vec2 corner_target =
          g.IO.MousePos - g.ActiveIdClickOffset +
          Lerp(def.InnerDir * grip_hover_outer_size,
               def.InnerDir * -grip_hover_inner_size,
               def.CornerPosN); // Corner of the window corresponding to our
                                // corner grip
      corner_target = Clamp(corner_target, clamp_min, clamp_max);
      CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN,
                                     &pos_target, &size_target);
    }

    // Only lower-left grip is visible before hovering/activating
    if (resize_grip_n == 0 || held || hovered)
      resize_grip_col[resize_grip_n] =
          GetColorU32(held      ? Col_ResizeGripActive
                      : hovered ? Col_ResizeGripHovered
                                : Col_ResizeGrip);
  }

  int resize_border_mask = 0x00;
  if (window->Flags & WindowFlags_ChildWindow)
    resize_border_mask |=
        ((window->ChildFlags & ChildFlags_ResizeX) ? 0x02 : 0) |
        ((window->ChildFlags & ChildFlags_ResizeY) ? 0x08 : 0);
  else
    resize_border_mask = g.IO.ConfigWindowsResizeFromEdges ? 0x0F : 0x00;
  for (int border_n = 0; border_n < 4; border_n++) {
    if ((resize_border_mask & (1 << border_n)) == 0)
      continue;
    const ResizeBorderDef &def = resize_border_def[border_n];
    const Axis axis =
        (border_n == Dir_Left || border_n == Dir_Right) ? Axis_X : Axis_Y;

    bool hovered, held;
    Rect border_rect = GetResizeBorderRect(
        window, border_n, grip_hover_inner_size, WINDOWS_HOVER_PADDING);
    ID border_id = window->GetID(border_n + 4); // == GetWindowResizeBorderID()
    ItemAdd(border_rect, border_id, NULL, ItemFlags_NoNav);
    ButtonBehavior(border_rect, border_id, &hovered, &held,
                   ButtonFlags_FlattenChildren | ButtonFlags_NoNavFocus);
    // GetForegroundDrawList(window)->AddRect(border_rect.Min, border_rect.Max,
    // COL32(255, 255, 0, 255));
    if (hovered && g.HoveredIdTimer <= WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER)
      hovered = false;
    if (hovered || held)
      g.MouseCursor =
          (axis == Axis_X) ? MouseCursor_ResizeEW : MouseCursor_ResizeNS;
    if (held && g.IO.MouseDoubleClicked[0]) {
      // Double-clicking bottom or right border auto-fit on this axis
      // FIXME: Support top and right borders: rework
      // CalcResizePosSizeFromAnyCorner() to be reusable in both cases.
      if (border_n == 1 || border_n == 3) // Right and bottom border
      {
        size_target[axis] =
            CalcWindowSizeAfterConstraint(window, size_auto_fit)[axis];
        ret_auto_fit_mask |= (1 << axis);
        hovered = held =
            false; // So border doesn't show highlighted at new position
      }
      ClearActiveID();
    } else if (held) {
      // Switch to relative resizing mode when border geometry moved (e.g.
      // resizing a child altering parent scroll), in order to avoid resizing
      // feedback loop. Currently only using relative mode on resizable child
      // windows, as the problem to solve is more likely noticeable for them,
      // but could apply for all windows eventually.
      // FIXME: May want to generalize this idiom at lower-level, so more
      // widgets can use it!
      const bool just_scrolled_manually_while_resizing =
          (g.WheelingWindow != NULL &&
           g.WheelingWindowScrolledFrame == g.FrameCount &&
           IsWindowChildOf(window, g.WheelingWindow, false, true));
      if (g.ActiveIdIsJustActivated || just_scrolled_manually_while_resizing) {
        g.WindowResizeBorderExpectedRect = border_rect;
        g.WindowResizeRelativeMode = false;
      }
      if ((window->Flags & WindowFlags_ChildWindow) &&
          memcmp(&g.WindowResizeBorderExpectedRect, &border_rect,
                 sizeof(Rect)) != 0)
        g.WindowResizeRelativeMode = true;

      const Vec2 border_curr =
          (window->Pos + Min(def.SegmentN1, def.SegmentN2) * window->Size);
      const float border_target_rel_mode_for_axis =
          border_curr[axis] + g.IO.MouseDelta[axis];
      const float border_target_abs_mode_for_axis =
          g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] +
          WINDOWS_HOVER_PADDING; // Match ButtonBehavior() padding above.

      // Use absolute mode position
      Vec2 border_target = window->Pos;
      border_target[axis] = border_target_abs_mode_for_axis;

      // Use relative mode target for child window, ignore resize when moving
      // back toward the ideal absolute position.
      bool ignore_resize = false;
      if (g.WindowResizeRelativeMode) {
        // GetForegroundDrawList()->AddText(GetMainViewport()->WorkPos,
        // COL32_WHITE, "Relative Mode");
        border_target[axis] = border_target_rel_mode_for_axis;
        if (g.IO.MouseDelta[axis] == 0.0f ||
            (g.IO.MouseDelta[axis] > 0.0f) == (border_target_rel_mode_for_axis >
                                               border_target_abs_mode_for_axis))
          ignore_resize = true;
      }

      // Clamp, apply
      Vec2 clamp_min(border_n == Dir_Right ? clamp_rect.Min.x : -FLT_MAX,
                     border_n == Dir_Down ||
                             (border_n == Dir_Up && window_move_from_title_bar)
                         ? clamp_rect.Min.y
                         : -FLT_MAX);
      Vec2 clamp_max(border_n == Dir_Left ? clamp_rect.Max.x : +FLT_MAX,
                     border_n == Dir_Up ? clamp_rect.Max.y : +FLT_MAX);
      border_target = Clamp(border_target, clamp_min, clamp_max);
      if (flags & WindowFlags_ChildWindow) // Clamp resizing of childs
                                           // within parent
      {
        if ((flags & (WindowFlags_HorizontalScrollbar |
                      WindowFlags_AlwaysHorizontalScrollbar)) == 0 ||
            (flags & WindowFlags_NoScrollbar))
          border_target.x =
              Clamp(border_target.x, window->ParentWindow->InnerClipRect.Min.x,
                    window->ParentWindow->InnerClipRect.Max.x);
        if (flags & WindowFlags_NoScrollbar)
          border_target.y =
              Clamp(border_target.y, window->ParentWindow->InnerClipRect.Min.y,
                    window->ParentWindow->InnerClipRect.Max.y);
      }
      if (!ignore_resize)
        CalcResizePosSizeFromAnyCorner(window, border_target,
                                       Min(def.SegmentN1, def.SegmentN2),
                                       &pos_target, &size_target);
    }
    if (hovered)
      *border_hovered = border_n;
    if (held)
      *border_held = border_n;
  }
  PopID();

  // Restore nav layer
  window->DC.NavLayerCurrent = NavLayer_Main;

  // Navigation resize (keyboard/gamepad)
  // FIXME: This cannot be moved to NavUpdateWindowing() because
  // CalcWindowSizeAfterConstraint() need to callback into user. Not even sure
  // the callback works here.
  if (g.NavWindowingTarget &&
      g.NavWindowingTarget->RootWindowDockTree == window) {
    Vec2 nav_resize_dir;
    if (g.NavInputSource == InputSource_Keyboard && g.IO.KeyShift)
      nav_resize_dir = GetKeyMagnitude2d(Key_LeftArrow, Key_RightArrow,
                                         Key_UpArrow, Key_DownArrow);
    if (g.NavInputSource == InputSource_Gamepad)
      nav_resize_dir =
          GetKeyMagnitude2d(Key_GamepadDpadLeft, Key_GamepadDpadRight,
                            Key_GamepadDpadUp, Key_GamepadDpadDown);
    if (nav_resize_dir.x != 0.0f || nav_resize_dir.y != 0.0f) {
      const float NAV_RESIZE_SPEED = 600.0f;
      const float resize_step =
          NAV_RESIZE_SPEED * g.IO.DeltaTime *
          Min(g.IO.DisplayFramebufferScale.x, g.IO.DisplayFramebufferScale.y);
      g.NavWindowingAccumDeltaSize += nav_resize_dir * resize_step;
      g.NavWindowingAccumDeltaSize =
          Max(g.NavWindowingAccumDeltaSize,
              clamp_rect.Min - window->Pos -
                  window->Size); // We need Pos+Size >= clmap_rect.Min, so Size
                                 // >= clmap_rect.Min - Pos, so size_delta >=
                                 // clmap_rect.Min - window->Pos - window->Size
      g.NavWindowingToggleLayer = false;
      g.NavDisableMouseHover = true;
      resize_grip_col[0] = GetColorU32(Col_ResizeGripActive);
      Vec2 accum_floored = Trunc(g.NavWindowingAccumDeltaSize);
      if (accum_floored.x != 0.0f || accum_floored.y != 0.0f) {
        // FIXME-NAV: Should store and accumulate into a separate size buffer to
        // handle sizing constraints properly, right now a constraint will make
        // us stuck.
        size_target = CalcWindowSizeAfterConstraint(window, window->SizeFull +
                                                                accum_floored);
        g.NavWindowingAccumDeltaSize -= accum_floored;
      }
    }
  }

  // Apply back modified position/size to window
  const Vec2 curr_pos = window->Pos;
  const Vec2 curr_size = window->SizeFull;
  if (size_target.x != FLT_MAX &&
      (window->Size.x != size_target.x || window->SizeFull.x != size_target.x))
    window->Size.x = window->SizeFull.x = size_target.x;
  if (size_target.y != FLT_MAX &&
      (window->Size.y != size_target.y || window->SizeFull.y != size_target.y))
    window->Size.y = window->SizeFull.y = size_target.y;
  if (pos_target.x != FLT_MAX && window->Pos.x != Trunc(pos_target.x))
    window->Pos.x = Trunc(pos_target.x);
  if (pos_target.y != FLT_MAX && window->Pos.y != Trunc(pos_target.y))
    window->Pos.y = Trunc(pos_target.y);
  if (curr_pos.x != window->Pos.x || curr_pos.y != window->Pos.y ||
      curr_size.x != window->SizeFull.x || curr_size.y != window->SizeFull.y)
    MarkIniSettingsDirty(window);

  // Recalculate next expected border expected coordinates
  if (*border_held != -1)
    g.WindowResizeBorderExpectedRect = GetResizeBorderRect(
        window, *border_held, grip_hover_inner_size, WINDOWS_HOVER_PADDING);

  return ret_auto_fit_mask;
}

static inline void ClampWindowPos(Window *window, const Rect &visibility_rect) {
  Context &g = *GGui;
  Vec2 size_for_clamping = window->Size;
  if (g.IO.ConfigWindowsMoveFromTitleBarOnly &&
      (!(window->Flags & WindowFlags_NoTitleBar) || window->DockNodeAsHost))
    size_for_clamping.y =
        Gui::GetFrameHeight(); // Not using window->TitleBarHeight() as
                               // DockNodeAsHost will report 0.0f here.
  window->Pos = Clamp(window->Pos, visibility_rect.Min - size_for_clamping,
                      visibility_rect.Max);
}

static void Gui::RenderWindowOuterBorders(Window *window) {
  Context &g = *GGui;
  float rounding = window->WindowRounding;
  float border_size = window->WindowBorderSize;
  if (border_size > 0.0f && !(window->Flags & WindowFlags_NoBackground))
    window->DrawList->AddRect(window->Pos, window->Pos + window->Size,
                              GetColorU32(Col_Border), rounding, 0,
                              border_size);

  if (window->ResizeBorderHovered != -1 || window->ResizeBorderHeld != -1) {
    const int border_n = (window->ResizeBorderHeld != -1)
                             ? window->ResizeBorderHeld
                             : window->ResizeBorderHovered;
    const ResizeBorderDef &def = resize_border_def[border_n];
    const Rect border_r = GetResizeBorderRect(window, border_n, rounding, 0.0f);
    const U32 border_col =
        GetColorU32((window->ResizeBorderHeld != -1) ? Col_SeparatorActive
                                                     : Col_SeparatorHovered);
    window->DrawList->PathArcTo(
        Lerp(border_r.Min, border_r.Max, def.SegmentN1) + Vec2(0.5f, 0.5f) +
            def.InnerDir * rounding,
        rounding, def.OuterAngle - PI * 0.25f, def.OuterAngle);
    window->DrawList->PathArcTo(
        Lerp(border_r.Min, border_r.Max, def.SegmentN2) + Vec2(0.5f, 0.5f) +
            def.InnerDir * rounding,
        rounding, def.OuterAngle, def.OuterAngle + PI * 0.25f);
    window->DrawList->PathStroke(border_col, 0,
                                 Max(2.0f, border_size)); // Thicker than usual
  }
  if (g.Style.FrameBorderSize > 0 &&
      !(window->Flags & WindowFlags_NoTitleBar) && !window->DockIsActive) {
    float y = window->Pos.y + window->TitleBarHeight() - 1;
    window->DrawList->AddLine(
        Vec2(window->Pos.x + border_size, y),
        Vec2(window->Pos.x + window->Size.x - border_size, y),
        GetColorU32(Col_Border), g.Style.FrameBorderSize);
  }
}

// Draw background and borders
// Draw and handle scrollbars
void Gui::RenderWindowDecorations(Window *window, const Rect &title_bar_rect,
                                  bool title_bar_is_highlight,
                                  bool handle_borders_and_resize_grips,
                                  int resize_grip_count,
                                  const U32 resize_grip_col[4],
                                  float resize_grip_draw_size) {
  Context &g = *GGui;
  Style &style = g.Style;
  WindowFlags flags = window->Flags;

  // Ensure that ScrollBar doesn't read last frame's SkipItems
  ASSERT(window->BeginCount == 0);
  window->SkipItems = false;

  // Draw window + handle manual resize
  // As we highlight the title bar when want_focus is set, multiple reappearing
  // windows will have their title bar highlighted on their reappearing frame.
  const float window_rounding = window->WindowRounding;
  const float window_border_size = window->WindowBorderSize;
  if (window->Collapsed) {
    // Title bar only
    const float backup_border_size = style.FrameBorderSize;
    g.Style.FrameBorderSize = window->WindowBorderSize;
    U32 title_bar_col =
        GetColorU32((title_bar_is_highlight && !g.NavDisableHighlight)
                        ? Col_TitleBgActive
                        : Col_TitleBgCollapsed);
    if (window->ViewportOwned)
      title_bar_col |= COL32_A_MASK; // No alpha (we don't support
                                     // is_docking_transparent_payload here
                                     // because simpler and less meaningful, but
                                     // could with a bit of code shuffle/reuse)
    RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true,
                window_rounding);
    g.Style.FrameBorderSize = backup_border_size;
  } else {
    // Window background
    if (!(flags & WindowFlags_NoBackground)) {
      bool is_docking_transparent_payload = false;
      if (g.DragDropActive &&
          (g.FrameCount - g.DragDropAcceptFrameCount) <= 1 &&
          g.IO.ConfigDockingTransparentPayload)
        if (g.DragDropPayload.IsDataType(PAYLOAD_TYPE_WINDOW) &&
            *(Window **)g.DragDropPayload.Data == window)
          is_docking_transparent_payload = true;

      U32 bg_col = GetColorU32(GetWindowBgColorIdx(window));
      if (window->ViewportOwned) {
        bg_col |= COL32_A_MASK; // No alpha
        if (is_docking_transparent_payload)
          window->Viewport->Alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA;
      } else {
        // Adjust alpha. For docking
        bool override_alpha = false;
        float alpha = 1.0f;
        if (g.NextWindowData.Flags & NextWindowDataFlags_HasBgAlpha) {
          alpha = g.NextWindowData.BgAlphaVal;
          override_alpha = true;
        }
        if (is_docking_transparent_payload) {
          alpha *= DOCKING_TRANSPARENT_PAYLOAD_ALPHA; // FIXME-DOCK: Should that
                                                      // be an override?
          override_alpha = true;
        }
        if (override_alpha)
          bg_col = (bg_col & ~COL32_A_MASK) |
                   (F32_TO_INT8_SAT(alpha) << COL32_A_SHIFT);
      }

      // Render, for docked windows and host windows we ensure bg goes before
      // decorations
      if (window->DockIsActive)
        window->DockNode->LastBgColor = bg_col;
      DrawList *bg_draw_list = window->DockIsActive
                                   ? window->DockNode->HostWindow->DrawList
                                   : window->DrawList;
      if (window->DockIsActive || (flags & WindowFlags_DockNodeHost))
        bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
      bg_draw_list->AddRectFilled(
          window->Pos + Vec2(0, window->TitleBarHeight()),
          window->Pos + window->Size, bg_col, window_rounding,
          (flags & WindowFlags_NoTitleBar) ? 0 : DrawFlags_RoundCornersBottom);
      if (window->DockIsActive || (flags & WindowFlags_DockNodeHost))
        bg_draw_list->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG);
    }
    if (window->DockIsActive)
      window->DockNode->IsBgDrawnThisFrame = true;

    // Title bar
    // (when docked, DockNode are drawing their own title bar. Individual
    // windows however do NOT set the _NoTitleBar flag, in order for their
    // pos/size to be matching their undocking state.)
    if (!(flags & WindowFlags_NoTitleBar) && !window->DockIsActive) {
      U32 title_bar_col =
          GetColorU32(title_bar_is_highlight ? Col_TitleBgActive : Col_TitleBg);
      if (window->ViewportOwned)
        title_bar_col |= COL32_A_MASK; // No alpha
      window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max,
                                      title_bar_col, window_rounding,
                                      DrawFlags_RoundCornersTop);
    }

    // Menu bar
    if (flags & WindowFlags_MenuBar) {
      Rect menu_bar_rect = window->MenuBarRect();
      menu_bar_rect.ClipWith(
          window->Rect()); // Soft clipping, in particular child window don't
                           // have minimum size covering the menu bar so this is
                           // useful for them.
      window->DrawList->AddRectFilled(
          menu_bar_rect.Min + Vec2(window_border_size, 0),
          menu_bar_rect.Max - Vec2(window_border_size, 0),
          GetColorU32(Col_MenuBarBg),
          (flags & WindowFlags_NoTitleBar) ? window_rounding : 0.0f,
          DrawFlags_RoundCornersTop);
      if (style.FrameBorderSize > 0.0f &&
          menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
        window->DrawList->AddLine(menu_bar_rect.GetBL(), menu_bar_rect.GetBR(),
                                  GetColorU32(Col_Border),
                                  style.FrameBorderSize);
    }

    // Docking: Unhide tab bar (small triangle in the corner), drag from small
    // triangle to quickly undock
    DockNode *node = window->DockNode;
    if (window->DockIsActive && node->IsHiddenTabBar() && !node->IsNoTabBar()) {
      float unhide_sz_draw = Trunc(g.FontSize * 0.70f);
      float unhide_sz_hit = Trunc(g.FontSize * 0.55f);
      Vec2 p = node->Pos;
      Rect r(p, p + Vec2(unhide_sz_hit, unhide_sz_hit));
      ID unhide_id = window->GetID("#UNHIDE");
      KeepAliveID(unhide_id);
      bool hovered, held;
      if (ButtonBehavior(r, unhide_id, &hovered, &held,
                         ButtonFlags_FlattenChildren))
        node->WantHiddenTabBarToggle = true;
      else if (held && IsMouseDragging(0))
        StartMouseMovingWindowOrNode(
            window, node, true); // Undock from tab-bar triangle = same as
                                 // window/collapse menu button

      // FIXME-DOCK: Ideally we'd use Col_TitleBgActive/Col_TitleBg
      // here, but neither is guaranteed to be visible enough at this sort of
      // size..
      U32 col = GetColorU32(((held && hovered) || (node->IsFocused && !hovered))
                                ? Col_ButtonActive
                            : hovered ? Col_ButtonHovered
                                      : Col_Button);
      window->DrawList->AddTriangleFilled(p, p + Vec2(unhide_sz_draw, 0.0f),
                                          p + Vec2(0.0f, unhide_sz_draw), col);
    }

    // Scrollbars
    if (window->ScrollbarX)
      Scrollbar(Axis_X);
    if (window->ScrollbarY)
      Scrollbar(Axis_Y);

    // Render resize grips (after their input handling so we don't have a frame
    // of latency)
    if (handle_borders_and_resize_grips && !(flags & WindowFlags_NoResize)) {
      for (int resize_grip_n = 0; resize_grip_n < resize_grip_count;
           resize_grip_n++) {
        const U32 col = resize_grip_col[resize_grip_n];
        if ((col & COL32_A_MASK) == 0)
          continue;
        const ResizeGripDef &grip = resize_grip_def[resize_grip_n];
        const Vec2 corner =
            Lerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
        window->DrawList->PathLineTo(
            corner +
            grip.InnerDir *
                ((resize_grip_n & 1)
                     ? Vec2(window_border_size, resize_grip_draw_size)
                     : Vec2(resize_grip_draw_size, window_border_size)));
        window->DrawList->PathLineTo(
            corner +
            grip.InnerDir *
                ((resize_grip_n & 1)
                     ? Vec2(resize_grip_draw_size, window_border_size)
                     : Vec2(window_border_size, resize_grip_draw_size)));
        window->DrawList->PathArcToFast(
            Vec2(corner.x +
                     grip.InnerDir.x * (window_rounding + window_border_size),
                 corner.y +
                     grip.InnerDir.y * (window_rounding + window_border_size)),
            window_rounding, grip.AngleMin12, grip.AngleMax12);
        window->DrawList->PathFillConvex(col);
      }
    }

    // Borders (for dock node host they will be rendered over after the tab bar)
    if (handle_borders_and_resize_grips && !window->DockNodeAsHost)
      RenderWindowOuterBorders(window);
  }
}

// When inside a dock node, this is handled in DockNodeCalcTabBarLayout()
// instead. Render title text, collapse button, close button
void Gui::RenderWindowTitleBarContents(Window *window,
                                       const Rect &title_bar_rect,
                                       const char *name, bool *p_open) {
  Context &g = *GGui;
  Style &style = g.Style;
  WindowFlags flags = window->Flags;

  const bool has_close_button = (p_open != NULL);
  const bool has_collapse_button = !(flags & WindowFlags_NoCollapse) &&
                                   (style.WindowMenuButtonPosition != Dir_None);

  // Close & Collapse button are on the Menu NavLayer and don't default focus
  // (unless there's nothing else on that layer)
  // FIXME-NAV: Might want (or not?) to set the equivalent of
  // ButtonFlags_NoNavFocus so that mouse clicks on standard title bar
  // items don't necessarily set nav/keyboard ref?
  const ItemFlags item_flags_backup = g.CurrentItemFlags;
  g.CurrentItemFlags |= ItemFlags_NoNavDefaultFocus;
  window->DC.NavLayerCurrent = NavLayer_Menu;

  // Layout buttons
  // FIXME: Would be nice to generalize the subtleties expressed here into
  // reusable code.
  float pad_l = style.FramePadding.x;
  float pad_r = style.FramePadding.x;
  float button_sz = g.FontSize;
  Vec2 close_button_pos;
  Vec2 collapse_button_pos;
  if (has_close_button) {
    close_button_pos = Vec2(title_bar_rect.Max.x - pad_r - button_sz,
                            title_bar_rect.Min.y + style.FramePadding.y);
    pad_r += button_sz + style.ItemInnerSpacing.x;
  }
  if (has_collapse_button && style.WindowMenuButtonPosition == Dir_Right) {
    collapse_button_pos = Vec2(title_bar_rect.Max.x - pad_r - button_sz,
                               title_bar_rect.Min.y + style.FramePadding.y);
    pad_r += button_sz + style.ItemInnerSpacing.x;
  }
  if (has_collapse_button && style.WindowMenuButtonPosition == Dir_Left) {
    collapse_button_pos = Vec2(title_bar_rect.Min.x + pad_l,
                               title_bar_rect.Min.y + style.FramePadding.y);
    pad_l += button_sz + style.ItemInnerSpacing.x;
  }

  // Collapse button (submitting first so it gets priority when choosing a
  // navigation init fallback)
  if (has_collapse_button)
    if (CollapseButton(window->GetID("#COLLAPSE"), collapse_button_pos, NULL))
      window->WantCollapseToggle =
          true; // Defer actual collapsing to next frame as we are too far in
                // the Begin() function

  // Close button
  if (has_close_button)
    if (CloseButton(window->GetID("#CLOSE"), close_button_pos))
      *p_open = false;

  window->DC.NavLayerCurrent = NavLayer_Main;
  g.CurrentItemFlags = item_flags_backup;

  // Title bar text (with: horizontal alignment, avoiding collapse/close button,
  // optional "unsaved document" marker)
  // FIXME: Refactor text alignment facilities along with RenderText helpers,
  // this is WAY too much messy code..
  const float marker_size_x =
      (flags & WindowFlags_UnsavedDocument) ? button_sz * 0.80f : 0.0f;
  const Vec2 text_size =
      CalcTextSize(name, NULL, true) + Vec2(marker_size_x, 0.0f);

  // As a nice touch we try to ensure that centered title text doesn't get
  // affected by visibility of Close/Collapse button, while uncentered title
  // text will still reach edges correctly.
  if (pad_l > style.FramePadding.x)
    pad_l += g.Style.ItemInnerSpacing.x;
  if (pad_r > style.FramePadding.x)
    pad_r += g.Style.ItemInnerSpacing.x;
  if (style.WindowTitleAlign.x > 0.0f && style.WindowTitleAlign.x < 1.0f) {
    float centerness =
        Saturate(1.0f - Fabs(style.WindowTitleAlign.x - 0.5f) *
                            2.0f); // 0.0f on either edges, 1.0f on center
    float pad_extend = Min(Max(pad_l, pad_r), title_bar_rect.GetWidth() -
                                                  pad_l - pad_r - text_size.x);
    pad_l = Max(pad_l, pad_extend * centerness);
    pad_r = Max(pad_r, pad_extend * centerness);
  }

  Rect layout_r(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y,
                title_bar_rect.Max.x - pad_r, title_bar_rect.Max.y);
  Rect clip_r(
      layout_r.Min.x, layout_r.Min.y,
      Min(layout_r.Max.x + g.Style.ItemInnerSpacing.x, title_bar_rect.Max.x),
      layout_r.Max.y);
  if (flags & WindowFlags_UnsavedDocument) {
    Vec2 marker_pos;
    marker_pos.x = Clamp(layout_r.Min.x +
                             (layout_r.GetWidth() - text_size.x) *
                                 style.WindowTitleAlign.x +
                             text_size.x,
                         layout_r.Min.x, layout_r.Max.x);
    marker_pos.y = (layout_r.Min.y + layout_r.Max.y) * 0.5f;
    if (marker_pos.x > layout_r.Min.x) {
      RenderBullet(window->DrawList, marker_pos, GetColorU32(Col_Text));
      clip_r.Max.x =
          Min(clip_r.Max.x, marker_pos.x - (int)(marker_size_x * 0.5f));
    }
  }
  // if (g.IO.KeyShift) window->DrawList->AddRect(layout_r.Min, layout_r.Max,
  // COL32(255, 128, 0, 255)); // [DEBUG] if (g.IO.KeyCtrl)
  // window->DrawList->AddRect(clip_r.Min, clip_r.Max, COL32(255, 128, 0,
  // 255)); // [DEBUG]
  RenderTextClipped(layout_r.Min, layout_r.Max, name, NULL, &text_size,
                    style.WindowTitleAlign, &clip_r);
}

void Gui::UpdateWindowParentAndRootLinks(Window *window, WindowFlags flags,
                                         Window *parent_window) {
  window->ParentWindow = parent_window;
  window->RootWindow = window->RootWindowPopupTree =
      window->RootWindowDockTree = window->RootWindowForTitleBarHighlight =
          window->RootWindowForNav = window;
  if (parent_window && (flags & WindowFlags_ChildWindow) &&
      !(flags & WindowFlags_Tooltip)) {
    window->RootWindowDockTree = parent_window->RootWindowDockTree;
    if (!window->DockIsActive &&
        !(parent_window->Flags & WindowFlags_DockNodeHost))
      window->RootWindow = parent_window->RootWindow;
  }
  if (parent_window && (flags & WindowFlags_Popup))
    window->RootWindowPopupTree = parent_window->RootWindowPopupTree;
  if (parent_window && !(flags & WindowFlags_Modal) &&
      (flags & (WindowFlags_ChildWindow |
                WindowFlags_Popup))) // FIXME: simply use _NoTitleBar ?
    window->RootWindowForTitleBarHighlight =
        parent_window->RootWindowForTitleBarHighlight;
  while (window->RootWindowForNav->Flags & WindowFlags_NavFlattened) {
    ASSERT(window->RootWindowForNav->ParentWindow != NULL);
    window->RootWindowForNav = window->RootWindowForNav->ParentWindow;
  }
}

// When a modal popup is open, newly created windows that want focus (i.e. are
// not popups and do not specify WindowFlags_NoFocusOnAppearing) should be
// positioned behind that modal window, unless the window was created inside the
// modal begin-stack. In case of multiple stacked modals newly created window
// honors begin stack order and does not go below its own modal parent.
// - WindowA            // FindBlockingModal() returns Modal1
//   - WindowB          //                  .. returns Modal1
//   - Modal1           //                  .. returns Modal2
//      - WindowC       //                  .. returns Modal2
//          - WindowD   //                  .. returns Modal2
//          - Modal2    //                  .. returns Modal2
//            - WindowE //                  .. returns NULL
// Notes:
// - FindBlockingModal(NULL) == NULL is generally equivalent to
// GetTopMostPopupModal() == NULL.
//   Only difference is here we check for ->Active/WasActive but it may be
//   unecessary.
Window *Gui::FindBlockingModal(Window *window) {
  Context &g = *GGui;
  if (g.OpenPopupStack.Size <= 0)
    return NULL;

  // Find a modal that has common parent with specified window. Specified window
  // should be positioned behind that modal.
  for (PopupData &popup_data : g.OpenPopupStack) {
    Window *popup_window = popup_data.Window;
    if (popup_window == NULL || !(popup_window->Flags & WindowFlags_Modal))
      continue;
    if (!popup_window->Active &&
        !popup_window
             ->WasActive) // Check WasActive, because this code may run before
                          // popup renders on current frame, also check Active
                          // to handle newly created windows.
      continue;
    if (window == NULL) // FindBlockingModal(NULL) test for if FocusWindow(NULL)
                        // is naturally possible via a mouse click.
      return popup_window;
    if (IsWindowWithinBeginStackOf(window,
                                   popup_window)) // Window may be over modal
      continue;
    return popup_window; // Place window right below first block modal
  }
  return NULL;
}

// Push a new Gui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning
// of every frame so you can use widgets without explicitly calling a Begin/End
// pair.
// - Begin/End can be called multiple times during the frame with the same
// window name to append content.
// - The window name is used as a unique identifier to preserve window
// information across frames (and save rudimentary information to the .ini
// file).
//   You can use the "##" or "###" markers to use the same label with different
//   id, or same id with different label. See documentation at the top of this
//   file.
// - Return false when window is collapsed, so you can early out in your code.
// You always need to call Gui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of
// the window, the pointed value will be set to false when the button is
// pressed.
bool Gui::Begin(const char *name, bool *p_open, WindowFlags flags) {
  Context &g = *GGui;
  const Style &style = g.Style;
  ASSERT(name != NULL && name[0] != '\0'); // Window name required
  ASSERT(g.WithinFrameScope);              // Forgot to call Gui::NewFrame()
  ASSERT(g.FrameCountEnded !=
         g.FrameCount); // Called Gui::Render() or Gui::EndFrame() and
                        // haven't called Gui::NewFrame() again yet

  // Find or create
  Window *window = FindWindowByName(name);
  const bool window_just_created = (window == NULL);
  if (window_just_created)
    window = CreateNewWindow(name, flags);

  // Automatically disable manual moving/resizing when NoInputs is set
  if ((flags & WindowFlags_NoInputs) == WindowFlags_NoInputs)
    flags |= WindowFlags_NoMove | WindowFlags_NoResize;

  if (flags & WindowFlags_NavFlattened)
    ASSERT(flags & WindowFlags_ChildWindow);

  const int current_frame = g.FrameCount;
  const bool first_begin_of_the_frame =
      (window->LastFrameActive != current_frame);
  window->IsFallbackWindow =
      (g.CurrentWindowStack.Size == 0 && g.WithinFrameScopeWithImplicitWindow);

  // Update the Appearing flag (note: the BeginDocked() path may also set this
  // to true later)
  bool window_just_activated_by_user =
      (window->LastFrameActive <
       current_frame - 1); // Not using !WasActive because the implicit "Debug"
                           // window would always toggle off->on
  if (flags & WindowFlags_Popup) {
    PopupData &popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
    window_just_activated_by_user |=
        (window->PopupId !=
         popup_ref.PopupId); // We recycle popups so treat window as activated
                             // if popup id changed
    window_just_activated_by_user |= (window != popup_ref.Window);
  }

  // Update Flags, LastFrameActive, BeginOrderXXX fields
  const bool window_was_appearing = window->Appearing;
  if (first_begin_of_the_frame) {
    UpdateWindowInFocusOrderList(window, window_just_created, flags);
    window->Appearing = window_just_activated_by_user;
    if (window->Appearing)
      SetWindowConditionAllowFlags(window, Cond_Appearing, true);
    window->FlagsPreviousFrame = window->Flags;
    window->Flags = (WindowFlags)flags;
    window->ChildFlags =
        (g.NextWindowData.Flags & NextWindowDataFlags_HasChildFlags)
            ? g.NextWindowData.ChildFlags
            : 0;
    window->LastFrameActive = current_frame;
    window->LastTimeActive = (float)g.Time;
    window->BeginOrderWithinParent = 0;
    window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
  } else {
    flags = window->Flags;
  }

  // Docking
  // (NB: during the frame dock nodes are created, it is possible that
  // (window->DockIsActive == false) even though (window->DockNode->Windows.Size
  // > 1)
  ASSERT(window->DockNode == NULL ||
         window->DockNodeAsHost == NULL); // Cannot be both
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasDock)
    SetWindowDock(window, g.NextWindowData.DockId, g.NextWindowData.DockCond);
  if (first_begin_of_the_frame) {
    bool has_dock_node = (window->DockId != 0 || window->DockNode != NULL);
    bool new_auto_dock_node =
        !has_dock_node && GetWindowAlwaysWantOwnTabBar(window);
    bool dock_node_was_visible = window->DockNodeIsVisible;
    bool dock_tab_was_visible = window->DockTabIsVisible;
    if (has_dock_node || new_auto_dock_node) {
      BeginDocked(window, p_open);
      flags = window->Flags;
      if (window->DockIsActive) {
        ASSERT(window->DockNode != NULL);
        g.NextWindowData.Flags &=
            ~NextWindowDataFlags_HasSizeConstraint; // Docking currently
                                                    // override constraints
      }

      // Amend the Appearing flag
      if (window->DockTabIsVisible && !dock_tab_was_visible &&
          dock_node_was_visible && !window->Appearing &&
          !window_was_appearing) {
        window->Appearing = true;
        SetWindowConditionAllowFlags(window, Cond_Appearing, true);
      }
    } else {
      window->DockIsActive = window->DockNodeIsVisible =
          window->DockTabIsVisible = false;
    }
  }

  // Parent window is latched only on the first call to Begin() of the frame, so
  // further append-calls can be done from a different window stack
  Window *parent_window_in_stack =
      (window->DockIsActive && window->DockNode->HostWindow)
          ? window->DockNode->HostWindow
      : g.CurrentWindowStack.empty() ? NULL
                                     : g.CurrentWindowStack.back().Window;
  Window *parent_window =
      first_begin_of_the_frame
          ? ((flags & (WindowFlags_ChildWindow | WindowFlags_Popup))
                 ? parent_window_in_stack
                 : NULL)
          : window->ParentWindow;
  ASSERT(parent_window != NULL || !(flags & WindowFlags_ChildWindow));

  // We allow window memory to be compacted so recreate the base stack when
  // needed.
  if (window->IDStack.Size == 0)
    window->IDStack.push_back(window->ID);

  // Add to stack
  g.CurrentWindow = window;
  WindowStackData window_stack_data;
  window_stack_data.Window = window;
  window_stack_data.ParentLastItemDataBackup = g.LastItemData;
  window_stack_data.StackSizesOnBegin.SetToContextState(&g);
  g.CurrentWindowStack.push_back(window_stack_data);
  if (flags & WindowFlags_ChildMenu)
    g.BeginMenuCount++;

  // Update ->RootWindow and others pointers (before any possible call to
  // FocusWindow)
  if (first_begin_of_the_frame) {
    UpdateWindowParentAndRootLinks(window, flags, parent_window);
    window->ParentWindowInBeginStack = parent_window_in_stack;
  }

  // Add to focus scope stack
  // We intentionally set g.CurrentWindow to NULL to prevent usage until when
  // the viewport is set, then will call SetCurrentWindow()
  if ((flags & WindowFlags_NavFlattened) == 0)
    PushFocusScope(window->ID);
  window->NavRootFocusScopeId = g.CurrentFocusScopeId;
  g.CurrentWindow = NULL;

  // Add to popup stack
  if (flags & WindowFlags_Popup) {
    PopupData &popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
    popup_ref.Window = window;
    popup_ref.ParentNavLayer = parent_window_in_stack->DC.NavLayerCurrent;
    g.BeginPopupStack.push_back(popup_ref);
    window->PopupId = popup_ref.PopupId;
  }

  // Process SetNextWindow***() calls
  // (FIXME: Consider splitting the HasXXX flags into X/Y components
  bool window_pos_set_by_api = false;
  bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasPos) {
    window_pos_set_by_api =
        (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
    if (window_pos_set_by_api &&
        LengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f) {
      // May be processed on the next frame if this is our first frame and we
      // are measuring size
      // FIXME: Look into removing the branch so everything can go through this
      // same code path for consistency.
      window->SetWindowPosVal = g.NextWindowData.PosVal;
      window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
      window->SetWindowPosAllowFlags &=
          ~(Cond_Once | Cond_FirstUseEver | Cond_Appearing);
    } else {
      SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
    }
  }
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasSize) {
    window_size_x_set_by_api =
        (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 &&
        (g.NextWindowData.SizeVal.x > 0.0f);
    window_size_y_set_by_api =
        (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 &&
        (g.NextWindowData.SizeVal.y > 0.0f);
    if ((window->ChildFlags & ChildFlags_ResizeX) &&
        (window->SetWindowSizeAllowFlags & Cond_FirstUseEver) ==
            0) // Axis-specific conditions for BeginChild()
      g.NextWindowData.SizeVal.x = window->SizeFull.x;
    if ((window->ChildFlags & ChildFlags_ResizeY) &&
        (window->SetWindowSizeAllowFlags & Cond_FirstUseEver) == 0)
      g.NextWindowData.SizeVal.y = window->SizeFull.y;
    SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
  }
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasScroll) {
    if (g.NextWindowData.ScrollVal.x >= 0.0f) {
      window->ScrollTarget.x = g.NextWindowData.ScrollVal.x;
      window->ScrollTargetCenterRatio.x = 0.0f;
    }
    if (g.NextWindowData.ScrollVal.y >= 0.0f) {
      window->ScrollTarget.y = g.NextWindowData.ScrollVal.y;
      window->ScrollTargetCenterRatio.y = 0.0f;
    }
  }
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasContentSize)
    window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
  else if (first_begin_of_the_frame)
    window->ContentSizeExplicit = Vec2(0.0f, 0.0f);
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasWindowClass)
    window->WindowClass = g.NextWindowData.WindowClass;
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasCollapsed)
    SetWindowCollapsed(window, g.NextWindowData.CollapsedVal,
                       g.NextWindowData.CollapsedCond);
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasFocus)
    FocusWindow(window);
  if (window->Appearing)
    SetWindowConditionAllowFlags(window, Cond_Appearing, false);

  // When reusing window again multiple times a frame, just append content
  // (don't need to setup again)
  if (first_begin_of_the_frame) {
    // Initialize
    const bool window_is_child_tooltip =
        (flags & WindowFlags_ChildWindow) &&
        (flags &
         WindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of
                               // Child+Tooltip for pinned tooltip (#1345)
    const bool window_just_appearing_after_hidden_for_resize =
        (window->HiddenFramesCannotSkipItems > 0);
    window->Active = true;
    window->HasCloseButton = (p_open != NULL);
    window->ClipRect = Vec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
    window->IDStack.resize(1);
    window->DrawList->_ResetForNewFrame();
    window->DC.CurrentTableIdx = -1;
    if (flags & WindowFlags_DockNodeHost) {
      window->DrawList->ChannelsSplit(2);
      window->DrawList->ChannelsSetCurrent(
          DOCKING_HOST_DRAW_CHANNEL_FG); // Render decorations on channel 1 as
                                         // we will render the backgrounds
                                         // manually later
    }

    // Restore buffer capacity when woken from a compacted state, to avoid
    if (window->MemoryCompacted)
      GcAwakeTransientWindowBuffers(window);

    // Update stored window name when it changes (which can _only_ happen with
    // the "###" operator, so the ID would stay unchanged). The title bar always
    // display the 'name' parameter, so we only update the string storage if it
    // needs to be visible to the end-user elsewhere.
    bool window_title_visible_elsewhere = false;
    if ((window->Viewport && window->Viewport->Window == window) ||
        (window->DockIsActive))
      window_title_visible_elsewhere = true;
    else if (g.NavWindowingListWindow != NULL &&
             (window->Flags & WindowFlags_NoNavFocus) ==
                 0) // Window titles visible when using CTRL+TAB
      window_title_visible_elsewhere = true;
    if (window_title_visible_elsewhere && !window_just_created &&
        strcmp(name, window->Name) != 0) {
      size_t buf_len = (size_t)window->NameBufLen;
      window->Name = Strdupcpy(window->Name, &buf_len, name);
      window->NameBufLen = (int)buf_len;
    }

    // UPDATE CONTENTS SIZE, UPDATE HIDDEN STATUS

    // Update contents size from last frame for auto-fitting (or use explicit
    // size)
    CalcWindowContentSizes(window, &window->ContentSize,
                           &window->ContentSizeIdeal);

    // FIXME: These flags are decremented before they are used. This means that
    // in order to have these fields produce their intended behaviors for one
    // frame we must set them to at least 2, which is counter-intuitive.
    // HiddenFramesCannotSkipItems is a more complicated case because it has a
    // single usage before this code block and may be set below before it is
    // finally checked.
    if (window->HiddenFramesCanSkipItems > 0)
      window->HiddenFramesCanSkipItems--;
    if (window->HiddenFramesCannotSkipItems > 0)
      window->HiddenFramesCannotSkipItems--;
    if (window->HiddenFramesForRenderOnly > 0)
      window->HiddenFramesForRenderOnly--;

    // Hide new windows for one frame until they calculate their size
    if (window_just_created &&
        (!window_size_x_set_by_api || !window_size_y_set_by_api))
      window->HiddenFramesCannotSkipItems = 1;

    // Hide popup/tooltip window when re-opening while we measure size (because
    // we recycle the windows) We reset Size/ContentSize for reappearing
    // popups/tooltips early in this function, so further code won't be tempted
    // to use the old size.
    if (window_just_activated_by_user &&
        (flags & (WindowFlags_Popup | WindowFlags_Tooltip)) != 0) {
      window->HiddenFramesCannotSkipItems = 1;
      if (flags & WindowFlags_AlwaysAutoResize) {
        if (!window_size_x_set_by_api)
          window->Size.x = window->SizeFull.x = 0.f;
        if (!window_size_y_set_by_api)
          window->Size.y = window->SizeFull.y = 0.f;
        window->ContentSize = window->ContentSizeIdeal = Vec2(0.f, 0.f);
      }
    }

    // SELECT VIEWPORT
    // We need to do this before using any style/font sizes, as viewport with a
    // different DPI may affect font sizes.

    WindowSelectViewport(window);
    SetCurrentViewport(window, window->Viewport);
    window->FontDpiScale = (g.IO.ConfigFlags & ConfigFlags_DpiEnableScaleFonts)
                               ? window->Viewport->DpiScale
                               : 1.0f;
    SetCurrentWindow(window);
    flags = window->Flags;

    // LOCK BORDER SIZE AND PADDING FOR THE FRAME (so that altering them doesn't
    // cause inconsistencies) We read Style data after the call to
    // UpdateSelectWindowViewport() which might be swapping the style.

    if (!window->DockIsActive && (flags & WindowFlags_ChildWindow))
      window->WindowBorderSize = style.ChildBorderSize;
    else
      window->WindowBorderSize =
          ((flags & (WindowFlags_Popup | WindowFlags_Tooltip)) &&
           !(flags & WindowFlags_Modal))
              ? style.PopupBorderSize
              : style.WindowBorderSize;
    window->WindowPadding = style.WindowPadding;
    if (!window->DockIsActive && (flags & WindowFlags_ChildWindow) &&
        !(flags & WindowFlags_Popup) &&
        !(window->ChildFlags & ChildFlags_AlwaysUseWindowPadding) &&
        window->WindowBorderSize == 0.0f)
      window->WindowPadding = Vec2(
          0.0f, (flags & WindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);

    // Lock menu offset so size calculation can use it as menu-bar windows need
    // a minimum size.
    window->DC.MenuBarOffset.x =
        Max(Max(window->WindowPadding.x, style.ItemSpacing.x),
            g.NextWindowData.MenuBarOffsetMinVal.x);
    window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;

    bool use_current_size_for_scrollbar_x = window_just_created;
    bool use_current_size_for_scrollbar_y = window_just_created;

    // Collapse window by double-clicking on title bar
    // At this point we don't have a clipping rectangle setup yet, so we can use
    // the title bar area for hit detection and drawing
    if (!(flags & WindowFlags_NoTitleBar) &&
        !(flags & WindowFlags_NoCollapse) && !window->DockIsActive) {
      // We don't use a regular button+id to test for double-click on title bar
      // (mostly due to legacy reason, could be fixed), so verify that we don't
      // have items over the title bar.
      Rect title_bar_rect = window->TitleBarRect();
      if (g.HoveredWindow == window && g.HoveredId == 0 &&
          g.HoveredIdPreviousFrame == 0 &&
          IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max) &&
          g.IO.MouseClickedCount[0] == 2)
        window->WantCollapseToggle = true;
      if (window->WantCollapseToggle) {
        window->Collapsed = !window->Collapsed;
        if (!window->Collapsed)
          use_current_size_for_scrollbar_y = true;
        MarkIniSettingsDirty(window);
      }
    } else {
      window->Collapsed = false;
    }
    window->WantCollapseToggle = false;

    // SIZE

    // Outer Decoration Sizes
    // (we need to clear ScrollbarSize immediatly as CalcWindowAutoFitSize()
    // needs it and can be called from other locations).
    const Vec2 scrollbar_sizes_from_last_frame = window->ScrollbarSizes;
    window->DecoOuterSizeX1 = 0.0f;
    window->DecoOuterSizeX2 = 0.0f;
    window->DecoOuterSizeY1 =
        window->TitleBarHeight() + window->MenuBarHeight();
    window->DecoOuterSizeY2 = 0.0f;
    window->ScrollbarSizes = Vec2(0.0f, 0.0f);

    // Calculate auto-fit size, handle automatic resize
    const Vec2 size_auto_fit =
        CalcWindowAutoFitSize(window, window->ContentSizeIdeal);
    if ((flags & WindowFlags_AlwaysAutoResize) && !window->Collapsed) {
      // Using SetNextWindowSize() overrides WindowFlags_AlwaysAutoResize,
      // so it can be used on tooltips/popups, etc.
      if (!window_size_x_set_by_api) {
        window->SizeFull.x = size_auto_fit.x;
        use_current_size_for_scrollbar_x = true;
      }
      if (!window_size_y_set_by_api) {
        window->SizeFull.y = size_auto_fit.y;
        use_current_size_for_scrollbar_y = true;
      }
    } else if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0) {
      // Auto-fit may only grow window during the first few frames
      // We still process initial auto-fit on collapsed windows to get a window
      // width, but otherwise don't honor WindowFlags_AlwaysAutoResize when
      // collapsed.
      if (!window_size_x_set_by_api && window->AutoFitFramesX > 0) {
        window->SizeFull.x = window->AutoFitOnlyGrows
                                 ? Max(window->SizeFull.x, size_auto_fit.x)
                                 : size_auto_fit.x;
        use_current_size_for_scrollbar_x = true;
      }
      if (!window_size_y_set_by_api && window->AutoFitFramesY > 0) {
        window->SizeFull.y = window->AutoFitOnlyGrows
                                 ? Max(window->SizeFull.y, size_auto_fit.y)
                                 : size_auto_fit.y;
        use_current_size_for_scrollbar_y = true;
      }
      if (!window->Collapsed)
        MarkIniSettingsDirty(window);
    }

    // Apply minimum/maximum window size constraints and final size
    window->SizeFull = CalcWindowSizeAfterConstraint(window, window->SizeFull);
    window->Size = window->Collapsed && !(flags & WindowFlags_ChildWindow)
                       ? window->TitleBarRect().GetSize()
                       : window->SizeFull;

    // POSITION

    // Popup latch its initial position, will position itself when it appears
    // next frame
    if (window_just_activated_by_user) {
      window->AutoPosLastDirection = Dir_None;
      if ((flags & WindowFlags_Popup) != 0 && !(flags & WindowFlags_Modal) &&
          !window_pos_set_by_api) // FIXME: BeginPopup() could use
                                  // SetNextWindowPos()
        window->Pos = g.BeginPopupStack.back().OpenPopupPos;
    }

    // Position child window
    if (flags & WindowFlags_ChildWindow) {
      ASSERT(parent_window && parent_window->Active);
      window->BeginOrderWithinParent =
          (short)parent_window->DC.ChildWindows.Size;
      parent_window->DC.ChildWindows.push_back(window);
      if (!(flags & WindowFlags_Popup) && !window_pos_set_by_api &&
          !window_is_child_tooltip)
        window->Pos = parent_window->DC.CursorPos;
    }

    const bool window_pos_with_pivot =
        (window->SetWindowPosVal.x != FLT_MAX &&
         window->HiddenFramesCannotSkipItems == 0);
    if (window_pos_with_pivot)
      SetWindowPos(window,
                   window->SetWindowPosVal -
                       window->Size * window->SetWindowPosPivot,
                   0); // Position given a pivot (e.g. for centering)
    else if ((flags & WindowFlags_ChildMenu) != 0)
      window->Pos = FindBestWindowPosForPopup(window);
    else if ((flags & WindowFlags_Popup) != 0 && !window_pos_set_by_api &&
             window_just_appearing_after_hidden_for_resize)
      window->Pos = FindBestWindowPosForPopup(window);
    else if ((flags & WindowFlags_Tooltip) != 0 && !window_pos_set_by_api &&
             !window_is_child_tooltip)
      window->Pos = FindBestWindowPosForPopup(window);

    // Late create viewport if we don't fit within our current host viewport.
    if (window->ViewportAllowPlatformMonitorExtend >= 0 &&
        !window->ViewportOwned &&
        !(window->Viewport->Flags & ViewportFlags_IsMinimized))
      if (!window->Viewport->GetMainRect().Contains(window->Rect())) {
        // This is based on the assumption that the DPI will be known ahead
        // (same as the DPI of the selection done in UpdateSelectWindowViewport)
        // Viewport* old_viewport = window->Viewport;
        window->Viewport =
            AddUpdateViewport(window, window->ID, window->Pos, window->Size,
                              ViewportFlags_NoFocusOnAppearing);

        // FIXME-DPI
        // ASSERT(old_viewport->DpiScale == window->Viewport->DpiScale); //
        // FIXME-DPI: Something went wrong
        SetCurrentViewport(window, window->Viewport);
        window->FontDpiScale =
            (g.IO.ConfigFlags & ConfigFlags_DpiEnableScaleFonts)
                ? window->Viewport->DpiScale
                : 1.0f;
        SetCurrentWindow(window);
      }

    if (window->ViewportOwned)
      WindowSyncOwnedViewport(window, parent_window_in_stack);

    // Calculate the range of allowed position for that window (to be movable
    // and visible past safe area padding) When clamping to stay visible, we
    // will enforce that window->Pos stays inside of visibility_rect.
    Rect viewport_rect(window->Viewport->GetMainRect());
    Rect viewport_work_rect(window->Viewport->GetWorkRect());
    Vec2 visibility_padding =
        Max(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
    Rect visibility_rect(viewport_work_rect.Min + visibility_padding,
                         viewport_work_rect.Max - visibility_padding);

    // Clamp position/size so window stays visible within its viewport or
    // monitor Ignore zero-sized display explicitly to avoid losing positions if
    // a window manager reports zero-sized window when initializing or
    // minimizing.
    // FIXME: Similar to code in GetWindowAllowedExtentRect()
    if (!window_pos_set_by_api && !(flags & WindowFlags_ChildWindow)) {
      if (!window->ViewportOwned && viewport_rect.GetWidth() > 0 &&
          viewport_rect.GetHeight() > 0.0f) {
        ClampWindowPos(window, visibility_rect);
      } else if (window->ViewportOwned && g.PlatformIO.Monitors.Size > 0) {
        // Lost windows (e.g. a monitor disconnected) will naturally moved to
        // the fallback/dummy monitor aka the main viewport.
        const PlatformMonitor *monitor =
            GetViewportPlatformMonitor(window->Viewport);
        visibility_rect.Min = monitor->WorkPos + visibility_padding;
        visibility_rect.Max =
            monitor->WorkPos + monitor->WorkSize - visibility_padding;
        ClampWindowPos(window, visibility_rect);
      }
    }
    window->Pos = Trunc(window->Pos);

    // Lock window rounding for the frame (so that altering them doesn't cause
    // inconsistencies) Large values tend to lead to variety of artifacts and
    // are not recommended.
    if (window->ViewportOwned || window->DockIsActive)
      window->WindowRounding = 0.0f;
    else
      window->WindowRounding =
          (flags & WindowFlags_ChildWindow) ? style.ChildRounding
          : ((flags & WindowFlags_Popup) && !(flags & WindowFlags_Modal))
              ? style.PopupRounding
              : style.WindowRounding;

    // For windows with title bar or menu bar, we clamp to FrameHeight(FontSize
    // + FramePadding.y * 2.0f) to completely hide artifacts.
    // if ((window->Flags & WindowFlags_MenuBar) || !(window->Flags &
    // WindowFlags_NoTitleBar))
    //    window->WindowRounding = Min(window->WindowRounding, g.FontSize +
    //    style.FramePadding.y * 2.0f);

    // Apply window focus (new and reactivated windows are moved to front)
    bool want_focus = false;
    if (window_just_activated_by_user &&
        !(flags & WindowFlags_NoFocusOnAppearing)) {
      if (flags & WindowFlags_Popup)
        want_focus = true;
      else if ((window->DockIsActive ||
                (flags & WindowFlags_ChildWindow) == 0) &&
               !(flags & WindowFlags_Tooltip))
        want_focus = true;
    }

    // [Test Engine] Register whole window in the item system (before submitting
    // further decorations)
#ifdef ENABLE_TEST_ENGINE
    if (g.TestEngineHookItems) {
      ASSERT(window->IDStack.Size == 1);
      window->IDStack.Size =
          0; // As window->IDStack[0] == window->ID here, make sure TestEngine
             // doesn't erroneously see window as parent of itself.
      TEST_ENGINE_ITEM_ADD(window->ID, window->Rect(), NULL);
      TEST_ENGINE_ITEM_INFO(
          window->ID, window->Name,
          (g.HoveredWindow == window) ? ItemStatusFlags_HoveredRect : 0);
      window->IDStack.Size = 1;
    }
#endif

    // Decide if we are going to handle borders and resize grips
    const bool handle_borders_and_resize_grips =
        (window->DockNodeAsHost || !window->DockIsActive);

    // Handle manual resize: Resize Grips, Borders, Gamepad
    int border_hovered = -1, border_held = -1;
    U32 resize_grip_col[4] = {};
    const int resize_grip_count =
        (window->Flags & WindowFlags_ChildWindow) ? 0
        : g.IO.ConfigWindowsResizeFromEdges
            ? 2
            : 1; // Allow resize from lower-left if we have the mouse cursor
                 // feedback for it.
    const float resize_grip_draw_size = TRUNC(Max(
        g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
    if (handle_borders_and_resize_grips && !window->Collapsed)
      if (int auto_fit_mask = UpdateWindowManualResize(
              window, size_auto_fit, &border_hovered, &border_held,
              resize_grip_count, &resize_grip_col[0], visibility_rect)) {
        if (auto_fit_mask & (1 << Axis_X))
          use_current_size_for_scrollbar_x = true;
        if (auto_fit_mask & (1 << Axis_Y))
          use_current_size_for_scrollbar_y = true;
      }
    window->ResizeBorderHovered = (signed char)border_hovered;
    window->ResizeBorderHeld = (signed char)border_held;

    // Synchronize window --> viewport again and one last time (clamping and
    // manual resize may have affected either)
    if (window->ViewportOwned) {
      if (!window->Viewport->PlatformRequestMove)
        window->Viewport->Pos = window->Pos;
      if (!window->Viewport->PlatformRequestResize)
        window->Viewport->Size = window->Size;
      window->Viewport->UpdateWorkRect();
      viewport_rect = window->Viewport->GetMainRect();
    }

    // Save last known viewport position within the window itself (so it can be
    // saved in .ini file and restored)
    window->ViewportPos = window->Viewport->Pos;

    // SCROLLBAR VISIBILITY

    // Update scrollbar visibility (based on the Size that was effective during
    // last frame or the auto-resized Size).
    if (!window->Collapsed) {
      // When reading the current size we need to read it after size constraints
      // have been applied. Intentionally use previous frame values for
      // InnerRect and ScrollbarSizes. And when we use window->DecorationUp here
      // it doesn't have ScrollbarSizes.y applied yet.
      Vec2 avail_size_from_current_frame =
          Vec2(window->SizeFull.x,
               window->SizeFull.y -
                   (window->DecoOuterSizeY1 + window->DecoOuterSizeY2));
      Vec2 avail_size_from_last_frame =
          window->InnerRect.GetSize() + scrollbar_sizes_from_last_frame;
      Vec2 needed_size_from_last_frame =
          window_just_created
              ? Vec2(0, 0)
              : window->ContentSize + window->WindowPadding * 2.0f;
      float size_x_for_scrollbars = use_current_size_for_scrollbar_x
                                        ? avail_size_from_current_frame.x
                                        : avail_size_from_last_frame.x;
      float size_y_for_scrollbars = use_current_size_for_scrollbar_y
                                        ? avail_size_from_current_frame.y
                                        : avail_size_from_last_frame.y;
      // bool scrollbar_y_from_last_frame = window->ScrollbarY; // FIXME: May
      // want to use that in the ScrollbarX expression? How many pros vs cons?
      window->ScrollbarY =
          (flags & WindowFlags_AlwaysVerticalScrollbar) ||
          ((needed_size_from_last_frame.y > size_y_for_scrollbars) &&
           !(flags & WindowFlags_NoScrollbar));
      window->ScrollbarX =
          (flags & WindowFlags_AlwaysHorizontalScrollbar) ||
          ((needed_size_from_last_frame.x >
            size_x_for_scrollbars -
                (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) &&
           !(flags & WindowFlags_NoScrollbar) &&
           (flags & WindowFlags_HorizontalScrollbar));
      if (window->ScrollbarX && !window->ScrollbarY)
        window->ScrollbarY =
            (needed_size_from_last_frame.y > size_y_for_scrollbars) &&
            !(flags & WindowFlags_NoScrollbar);
      window->ScrollbarSizes =
          Vec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f,
               window->ScrollbarX ? style.ScrollbarSize : 0.0f);

      // Amend the partially filled window->DecorationXXX values.
      window->DecoOuterSizeX2 += window->ScrollbarSizes.x;
      window->DecoOuterSizeY2 += window->ScrollbarSizes.y;
    }

    // UPDATE RECTANGLES (1- THOSE NOT AFFECTED BY SCROLLING)
    // Update various regions. Variables they depend on should be set above in
    // this function. We set this up after processing the resize grip so that
    // our rectangles doesn't lag by a frame.

    // Outer rectangle
    // Not affected by window border size. Used by:
    // - FindHoveredWindow() (w/ extra padding when border resize is enabled)
    // - Begin() initial clipping rect for drawing window background and
    // borders.
    // - Begin() clipping whole child
    const Rect host_rect =
        ((flags & WindowFlags_ChildWindow) && !(flags & WindowFlags_Popup) &&
         !window_is_child_tooltip)
            ? parent_window->ClipRect
            : viewport_rect;
    const Rect outer_rect = window->Rect();
    const Rect title_bar_rect = window->TitleBarRect();
    window->OuterRectClipped = outer_rect;
    if (window->DockIsActive)
      window->OuterRectClipped.Min.y += window->TitleBarHeight();
    window->OuterRectClipped.ClipWith(host_rect);

    // Inner rectangle
    // Not affected by window border size. Used by:
    // - InnerClipRect
    // - ScrollToRectEx()
    // - NavUpdatePageUpPageDown()
    // - Scrollbar()
    window->InnerRect.Min.x = window->Pos.x + window->DecoOuterSizeX1;
    window->InnerRect.Min.y = window->Pos.y + window->DecoOuterSizeY1;
    window->InnerRect.Max.x =
        window->Pos.x + window->Size.x - window->DecoOuterSizeX2;
    window->InnerRect.Max.y =
        window->Pos.y + window->Size.y - window->DecoOuterSizeY2;

    // Inner clipping rectangle.
    // Will extend a little bit outside the normal work region.
    // This is to allow e.g. Selectable or CollapsingHeader or some separators
    // to cover that space. Force round operator last to ensure that e.g.
    // (int)(max.x-min.x) in user's render code produce correct result. Note
    // that if our window is collapsed we will end up with an inverted (~null)
    // clipping rectangle which is the correct behavior. Affected by
    // window/frame border size. Used by:
    // - Begin() initial clip rect
    float top_border_size =
        (((flags & WindowFlags_MenuBar) || !(flags & WindowFlags_NoTitleBar))
             ? style.FrameBorderSize
             : window->WindowBorderSize);
    window->InnerClipRect.Min.x = Floor(
        0.5f + window->InnerRect.Min.x +
        Max(Trunc(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
    window->InnerClipRect.Min.y =
        Floor(0.5f + window->InnerRect.Min.y + top_border_size);
    window->InnerClipRect.Max.x = Floor(
        0.5f + window->InnerRect.Max.x -
        Max(Trunc(window->WindowPadding.x * 0.5f), window->WindowBorderSize));
    window->InnerClipRect.Max.y =
        Floor(0.5f + window->InnerRect.Max.y - window->WindowBorderSize);
    window->InnerClipRect.ClipWithFull(host_rect);

    // Default item width. Make it proportional to window size if window
    // manually resizes
    if (window->Size.x > 0.0f && !(flags & WindowFlags_Tooltip) &&
        !(flags & WindowFlags_AlwaysAutoResize))
      window->ItemWidthDefault = Trunc(window->Size.x * 0.65f);
    else
      window->ItemWidthDefault = Trunc(g.FontSize * 16.0f);

    // SCROLLING

    // Lock down maximum scrolling
    // The value of ScrollMax are ahead from ScrollbarX/ScrollbarY which is
    // intentionally using InnerRect from previous rect in order to accommodate
    // for right/bottom aligned items without creating a scrollbar.
    window->ScrollMax.x =
        Max(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f -
                      window->InnerRect.GetWidth());
    window->ScrollMax.y =
        Max(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f -
                      window->InnerRect.GetHeight());

    // Apply scrolling
    window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
    window->ScrollTarget = Vec2(FLT_MAX, FLT_MAX);
    window->DecoInnerSizeX1 = window->DecoInnerSizeY1 = 0.0f;

    // DRAWING

    // Setup draw list and outer clipping rectangle
    ASSERT(window->DrawList->CmdBuffer.Size == 1 &&
           window->DrawList->CmdBuffer[0].ElemCount == 0);
    window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
    PushClipRect(host_rect.Min, host_rect.Max, false);

    // Child windows can render their decoration (bg color, border, scrollbars,
    // etc.) within their parent to save a draw call (since 1.71) When using
    // overlapping child windows, this will break the assumption that child
    // z-order is mapped to submission order.
    // FIXME: User code may rely on explicit sorting of overlapping child window
    // and would need to disable this somehow. Please get in contact if you are
    // affected (github #4493)
    const bool is_undocked_or_docked_visible =
        !window->DockIsActive || window->DockTabIsVisible;
    if (is_undocked_or_docked_visible) {
      bool render_decorations_in_parent = false;
      if ((flags & WindowFlags_ChildWindow) && !(flags & WindowFlags_Popup) &&
          !window_is_child_tooltip) {
        // - We test overlap with the previous child window only (testing all
        // would end up being O(log N) not a good investment here)
        // - We disable this when the parent window has zero vertices, which is
        // a common pattern leading to laying out multiple overlapping childs
        Window *previous_child =
            parent_window->DC.ChildWindows.Size >= 2
                ? parent_window->DC
                      .ChildWindows[parent_window->DC.ChildWindows.Size - 2]
                : NULL;
        bool previous_child_overlapping =
            previous_child ? previous_child->Rect().Overlaps(window->Rect())
                           : false;
        bool parent_is_empty = (parent_window->DrawList->VtxBuffer.Size == 0);
        if (window->DrawList->CmdBuffer.back().ElemCount == 0 &&
            !parent_is_empty && !previous_child_overlapping)
          render_decorations_in_parent = true;
      }
      if (render_decorations_in_parent)
        window->DrawList = parent_window->DrawList;

      // Handle title bar, scrollbar, resize grips and resize borders
      const Window *window_to_highlight =
          g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
      const bool title_bar_is_highlight =
          want_focus ||
          (window_to_highlight &&
           (window->RootWindowForTitleBarHighlight ==
                window_to_highlight->RootWindowForTitleBarHighlight ||
            (window->DockNode &&
             window->DockNode == window_to_highlight->DockNode)));
      RenderWindowDecorations(window, title_bar_rect, title_bar_is_highlight,
                              handle_borders_and_resize_grips,
                              resize_grip_count, resize_grip_col,
                              resize_grip_draw_size);

      if (render_decorations_in_parent)
        window->DrawList = &window->DrawListInst;
    }

    // UPDATE RECTANGLES (2- THOSE AFFECTED BY SCROLLING)

    // Work rectangle.
    // Affected by window padding and border size. Used by:
    // - Columns() for right-most edge
    // - TreeNode(), CollapsingHeader() for right-most edge
    // - BeginTabBar() for right-most edge
    const bool allow_scrollbar_x = !(flags & WindowFlags_NoScrollbar) &&
                                   (flags & WindowFlags_HorizontalScrollbar);
    const bool allow_scrollbar_y = !(flags & WindowFlags_NoScrollbar);
    const float work_rect_size_x =
        (window->ContentSizeExplicit.x != 0.0f
             ? window->ContentSizeExplicit.x
             : Max(allow_scrollbar_x ? window->ContentSize.x : 0.0f,
                   window->Size.x - window->WindowPadding.x * 2.0f -
                       (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
    const float work_rect_size_y =
        (window->ContentSizeExplicit.y != 0.0f
             ? window->ContentSizeExplicit.y
             : Max(allow_scrollbar_y ? window->ContentSize.y : 0.0f,
                   window->Size.y - window->WindowPadding.y * 2.0f -
                       (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));
    window->WorkRect.Min.x =
        Trunc(window->InnerRect.Min.x - window->Scroll.x +
              Max(window->WindowPadding.x, window->WindowBorderSize));
    window->WorkRect.Min.y =
        Trunc(window->InnerRect.Min.y - window->Scroll.y +
              Max(window->WindowPadding.y, window->WindowBorderSize));
    window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
    window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;
    window->ParentWorkRect = window->WorkRect;

    // [LEGACY] Content Region
    // FIXME-OBSOLETE: window->ContentRegionRect.Max is currently very
    // misleading / partly faulty, but some BeginChild() patterns relies on it.
    // Unless explicit content size is specified by user, this currently
    // represent the region leading to no scrolling. Used by:
    // - Mouse wheel scrolling + many other things
    window->ContentRegionRect.Min.x = window->Pos.x - window->Scroll.x +
                                      window->WindowPadding.x +
                                      window->DecoOuterSizeX1;
    window->ContentRegionRect.Min.y = window->Pos.y - window->Scroll.y +
                                      window->WindowPadding.y +
                                      window->DecoOuterSizeY1;
    window->ContentRegionRect.Max.x =
        window->ContentRegionRect.Min.x +
        (window->ContentSizeExplicit.x != 0.0f
             ? window->ContentSizeExplicit.x
             : (window->Size.x - window->WindowPadding.x * 2.0f -
                (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
    window->ContentRegionRect.Max.y =
        window->ContentRegionRect.Min.y +
        (window->ContentSizeExplicit.y != 0.0f
             ? window->ContentSizeExplicit.y
             : (window->Size.y - window->WindowPadding.y * 2.0f -
                (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));

    // Setup drawing context
    // (NB: That term "drawing context / DC" lost its meaning a long time ago.
    // Initially was meant to hold transient data only. Nowadays difference
    // between window-> and window->DC-> is dubious.)
    window->DC.Indent.x =
        window->DecoOuterSizeX1 + window->WindowPadding.x - window->Scroll.x;
    window->DC.GroupOffset.x = 0.0f;
    window->DC.ColumnsOffset.x = 0.0f;

    // Record the loss of precision of CursorStartPos which can happen due to
    // really large scrolling amount. This is used by clipper to compensate and
    // fix the most common use case of large scroll area. Easy and cheap, next
    // best thing compared to switching everything to double or U64.
    double start_pos_highp_x = (double)window->Pos.x + window->WindowPadding.x -
                               (double)window->Scroll.x +
                               window->DecoOuterSizeX1 +
                               window->DC.ColumnsOffset.x;
    double start_pos_highp_y = (double)window->Pos.y + window->WindowPadding.y -
                               (double)window->Scroll.y +
                               window->DecoOuterSizeY1;
    window->DC.CursorStartPos =
        Vec2((float)start_pos_highp_x, (float)start_pos_highp_y);
    window->DC.CursorStartPosLossyness =
        Vec2((float)(start_pos_highp_x - window->DC.CursorStartPos.x),
             (float)(start_pos_highp_y - window->DC.CursorStartPos.y));
    window->DC.CursorPos = window->DC.CursorStartPos;
    window->DC.CursorPosPrevLine = window->DC.CursorPos;
    window->DC.CursorMaxPos = window->DC.CursorStartPos;
    window->DC.IdealMaxPos = window->DC.CursorStartPos;
    window->DC.CurrLineSize = window->DC.PrevLineSize = Vec2(0.0f, 0.0f);
    window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset =
        0.0f;
    window->DC.IsSameLine = window->DC.IsSetPos = false;

    window->DC.NavLayerCurrent = NavLayer_Main;
    window->DC.NavLayersActiveMask = window->DC.NavLayersActiveMaskNext;
    window->DC.NavLayersActiveMaskNext = 0x00;
    window->DC.NavIsScrollPushableX = true;
    window->DC.NavHideHighlightOneFrame = false;
    window->DC.NavWindowHasScrollY = (window->ScrollMax.y > 0.0f);

    window->DC.MenuBarAppending = false;
    window->DC.MenuColumns.Update(style.ItemSpacing.x,
                                  window_just_activated_by_user);
    window->DC.TreeDepth = 0;
    window->DC.TreeJumpToParentOnPopMask = 0x00;
    window->DC.ChildWindows.resize(0);
    window->DC.StateStorage = &window->StateStorage;
    window->DC.CurrentColumns = NULL;
    window->DC.LayoutType = LayoutType_Vertical;
    window->DC.ParentLayoutType =
        parent_window ? parent_window->DC.LayoutType : LayoutType_Vertical;

    window->DC.ItemWidth = window->ItemWidthDefault;
    window->DC.TextWrapPos = -1.0f; // disabled
    window->DC.ItemWidthStack.resize(0);
    window->DC.TextWrapPosStack.resize(0);

    if (window->AutoFitFramesX > 0)
      window->AutoFitFramesX--;
    if (window->AutoFitFramesY > 0)
      window->AutoFitFramesY--;

    // Apply focus (we need to call FocusWindow() AFTER setting
    // DC.CursorStartPos so our initial navigation reference rectangle can start
    // around there) We FocusRequestFlags_UnlessBelowModal to:
    // - Avoid focusing a window that is created outside of a modal. This will
    // prevent active modal from being closed.
    // - Position window behind the modal that is not a begin-parent of this
    // window.
    if (want_focus)
      FocusWindow(window, FocusRequestFlags_UnlessBelowModal);
    if (want_focus && window == g.NavWindow)
      NavInitWindow(window,
                    false); // <-- this is in the way for us to be able to defer
                            // and sort reappearing FocusWindow() calls

    // Close requested by platform window (apply to all windows in this
    // viewport)
    if (p_open != NULL && window->Viewport->PlatformRequestClose &&
        window->Viewport != GetMainViewport()) {
      DEBUG_LOG_VIEWPORT(
          "[viewport] Window '%s' closed by PlatformRequestClose\n",
          window->Name);
      *p_open = false;
      g.NavWindowingToggleLayer =
          false; // Assume user mapped PlatformRequestClose on ALT-F4 so we
                 // disable ALT for menu toggle. False positive not an issue. //
                 // FIXME-NAV: Try removing.
    }

    // Title bar
    if (!(flags & WindowFlags_NoTitleBar) && !window->DockIsActive)
      RenderWindowTitleBarContents(
          window,
          Rect(title_bar_rect.Min.x + window->WindowBorderSize,
               title_bar_rect.Min.y,
               title_bar_rect.Max.x - window->WindowBorderSize,
               title_bar_rect.Max.y),
          name, p_open);

    // Clear hit test shape every frame
    window->HitTestHoleSize.x = window->HitTestHoleSize.y = 0;

    // Pressing CTRL+C while holding on a window copy its content to the
    // clipboard This works but 1. doesn't handle multiple Begin/End pairs, 2.
    // recursing into another Begin/End pair - so we need to work that out and
    // add better logging scope. Maybe we can support CTRL+C on every element?
    /*
    //if (g.NavWindow == window && g.ActiveId == 0)
    if (g.ActiveId == window->MoveId)
        if (g.IO.KeyCtrl && IsKeyPressed(Key_C))
            LogToClipboard();
    */

    if (g.IO.ConfigFlags & ConfigFlags_DockingEnable) {
      // Docking: Dragging a dockable window (or any of its child) turns it into
      // a drag and drop source. We need to do this _before_ we overwrite
      // window->DC.LastItemId below because BeginDockableDragDropSource() also
      // overwrites it.
      if (g.MovingWindow == window &&
          (window->RootWindowDockTree->Flags & WindowFlags_NoDocking) == 0)
        BeginDockableDragDropSource(window);

      // Docking: Any dockable window can act as a target. For dock node hosts
      // we call BeginDockableDragDropTarget() in DockNodeUpdate() instead.
      if (g.DragDropActive && !(flags & WindowFlags_NoDocking))
        if (g.MovingWindow == NULL ||
            g.MovingWindow->RootWindowDockTree != window)
          if ((window == window->RootWindowDockTree) &&
              !(window->Flags & WindowFlags_DockNodeHost))
            BeginDockableDragDropTarget(window);
    }

    // We fill last item data based on Title Bar/Tab, in order for
    // IsItemHovered() and IsItemActive() to be usable after Begin(). This is
    // useful to allow creating context menus on title bar only, etc.
    if (window->DockIsActive)
      SetLastItemData(window->MoveId, g.CurrentItemFlags,
                      window->DockTabItemStatusFlags, window->DockTabItemRect);
    else
      SetLastItemData(
          window->MoveId, g.CurrentItemFlags,
          IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false)
              ? ItemStatusFlags_HoveredRect
              : 0,
          title_bar_rect);

      // [DEBUG]
#ifndef DISABLE_DEBUG_TOOLS
    if (g.DebugLocateId != 0 &&
        (window->ID == g.DebugLocateId || window->MoveId == g.DebugLocateId))
      DebugLocateItemResolveWithLastItem();
#endif

      // [Test Engine] Register title bar / tab with MoveId.
#ifdef ENABLE_TEST_ENGINE
    if (!(window->Flags & WindowFlags_NoTitleBar))
      TEST_ENGINE_ITEM_ADD(g.LastItemData.ID, g.LastItemData.Rect,
                           &g.LastItemData);
#endif
  } else {
    // Append
    SetCurrentViewport(window, window->Viewport);
    SetCurrentWindow(window);
  }

  if (!(flags & WindowFlags_DockNodeHost))
    PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

  // Clear 'accessed' flag last thing (After PushClipRect which will set the
  // flag. We want the flag to stay false when the default "Debug" window is
  // unused)
  window->WriteAccessed = false;
  window->BeginCount++;
  g.NextWindowData.ClearFlags();

  // Update visibility
  if (first_begin_of_the_frame) {
    // When we are about to select this tab (which will only be visible on the
    // _next frame_), flag it with a non-zero HiddenFramesCannotSkipItems. This
    // will have the important effect of actually returning true in Begin() and
    // not setting SkipItems, allowing an earlier submission of the window
    // contents. This is analogous to regular windows being hidden from one
    // frame. It is especially important as e.g. nested TabBars would otherwise
    // generate flicker in the form of one empty frame, or focus requests won't
    // be processed.
    if (window->DockIsActive && !window->DockTabIsVisible) {
      if (window->LastFrameJustFocused == g.FrameCount)
        window->HiddenFramesCannotSkipItems = 1;
      else
        window->HiddenFramesCanSkipItems = 1;
    }

    if ((flags & WindowFlags_ChildWindow) && !(flags & WindowFlags_ChildMenu)) {
      // Child window can be out of sight and have "negative" clip windows.
      // Mark them as collapsed so commands are skipped earlier (we can't
      // manually collapse them because they have no title bar).
      ASSERT((flags & WindowFlags_NoTitleBar) != 0 || window->DockIsActive);
      const bool nav_request =
          (flags & WindowFlags_NavFlattened) &&
          (g.NavAnyRequest && g.NavWindow &&
           g.NavWindow->RootWindowForNav == window->RootWindowForNav);
      if (!g.LogEnabled && !nav_request)
        if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x ||
            window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y) {
          if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
            window->HiddenFramesCannotSkipItems = 1;
          else
            window->HiddenFramesCanSkipItems = 1;
        }

      // Hide along with parent or if parent is collapsed
      if (parent_window && (parent_window->Collapsed ||
                            parent_window->HiddenFramesCanSkipItems > 0))
        window->HiddenFramesCanSkipItems = 1;
      if (parent_window && (parent_window->Collapsed ||
                            parent_window->HiddenFramesCannotSkipItems > 0))
        window->HiddenFramesCannotSkipItems = 1;
    }

    // Don't render if style alpha is 0.0 at the time of Begin(). This is
    // arbitrary and inconsistent but has been there for a long while (may
    // remove at some point)
    if (style.Alpha <= 0.0f)
      window->HiddenFramesCanSkipItems = 1;

    // Update the Hidden flag
    bool hidden_regular = (window->HiddenFramesCanSkipItems > 0) ||
                          (window->HiddenFramesCannotSkipItems > 0);
    window->Hidden = hidden_regular || (window->HiddenFramesForRenderOnly > 0);

    // Disable inputs for requested number of frames
    if (window->DisableInputsFrames > 0) {
      window->DisableInputsFrames--;
      window->Flags |= WindowFlags_NoInputs;
    }

    // Update the SkipItems flag, used to early out of all items functions (no
    // layout required)
    bool skip_items = false;
    if (window->Collapsed || !window->Active || hidden_regular)
      if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 &&
          window->HiddenFramesCannotSkipItems <= 0)
        skip_items = true;
    window->SkipItems = skip_items;

    // Restore NavLayersActiveMaskNext to previous value when not visible, so a
    // CTRL+Tab back can use a safe value.
    if (window->SkipItems)
      window->DC.NavLayersActiveMaskNext = window->DC.NavLayersActiveMask;

    // Sanity check: there are two spots which can set Appearing = true
    // - when 'window_just_activated_by_user' is set ->
    // HiddenFramesCannotSkipItems is set -> SkipItems always false
    // - in BeginDocked() path when DockNodeIsVisible == DockTabIsVisible ==
    // true -> hidden _should_ be all zero // FIXME: Not formally proven, hence
    // the assert.
    if (window->SkipItems && !window->Appearing)
      ASSERT(window->Appearing == false);
  }

  // [DEBUG] io.ConfigDebugBeginReturnValue override return value to test
  // Begin/End and BeginChild/EndChild behaviors. (The implicit fallback window
  // is NOT automatically ended allowing it to always be able to receive
  // commands without crashing)
  if (!window->IsFallbackWindow &&
      ((g.IO.ConfigDebugBeginReturnValueOnce && window_just_created) ||
       (g.IO.ConfigDebugBeginReturnValueLoop &&
        g.DebugBeginReturnValueCullDepth == g.CurrentWindowStack.Size))) {
    if (window->AutoFitFramesX > 0) {
      window->AutoFitFramesX++;
    }
    if (window->AutoFitFramesY > 0) {
      window->AutoFitFramesY++;
    }
    return false;
  }

  return !window->SkipItems;
}

void Gui::End() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  // Error checking: verify that user hasn't called End() too many times!
  if (g.CurrentWindowStack.Size <= 1 && g.WithinFrameScopeWithImplicitWindow) {
    ASSERT_USER_ERROR(g.CurrentWindowStack.Size > 1,
                      "Calling End() too many times!");
    return;
  }
  ASSERT(g.CurrentWindowStack.Size > 0);

  // Error checking: verify that user doesn't directly call End() on a child
  // window.
  if ((window->Flags & WindowFlags_ChildWindow) &&
      !(window->Flags & WindowFlags_DockNodeHost) && !window->DockIsActive)
    ASSERT_USER_ERROR(g.WithinEndChild, "Must call EndChild() and not End()!");

  // Close anything that is open
  if (window->DC.CurrentColumns)
    EndColumns();
  if (!(window->Flags &
        WindowFlags_DockNodeHost)) // Pop inner window clip rectangle
    PopClipRect();
  if ((window->Flags & WindowFlags_NavFlattened) == 0)
    PopFocusScope();

  // Stop logging
  if (!(window->Flags & WindowFlags_ChildWindow)) // FIXME: add more options for
                                                  // scope of logging
    LogFinish();

  if (window->DC.IsSetPos)
    ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

  // Docking: report contents sizes to parent to allow for auto-resize
  if (window->DockNode && window->DockTabIsVisible)
    if (Window *host_window = window->DockNode->HostWindow) // FIXME-DOCK
      host_window->DC.CursorMaxPos = window->DC.CursorMaxPos +
                                     window->WindowPadding -
                                     host_window->WindowPadding;

  // Pop from window stack
  g.LastItemData = g.CurrentWindowStack.back().ParentLastItemDataBackup;
  if (window->Flags & WindowFlags_ChildMenu)
    g.BeginMenuCount--;
  if (window->Flags & WindowFlags_Popup)
    g.BeginPopupStack.pop_back();
  g.CurrentWindowStack.back().StackSizesOnBegin.CompareWithContextState(&g);
  g.CurrentWindowStack.pop_back();
  SetCurrentWindow(g.CurrentWindowStack.Size == 0
                       ? NULL
                       : g.CurrentWindowStack.back().Window);
  if (g.CurrentWindow)
    SetCurrentViewport(g.CurrentWindow, g.CurrentWindow->Viewport);
}

void Gui::BringWindowToFocusFront(Window *window) {
  Context &g = *GGui;
  ASSERT(window == window->RootWindow);

  const int cur_order = window->FocusOrder;
  ASSERT(g.WindowsFocusOrder[cur_order] == window);
  if (g.WindowsFocusOrder.back() == window)
    return;

  const int new_order = g.WindowsFocusOrder.Size - 1;
  for (int n = cur_order; n < new_order; n++) {
    g.WindowsFocusOrder[n] = g.WindowsFocusOrder[n + 1];
    g.WindowsFocusOrder[n]->FocusOrder--;
    ASSERT(g.WindowsFocusOrder[n]->FocusOrder == n);
  }
  g.WindowsFocusOrder[new_order] = window;
  window->FocusOrder = (short)new_order;
}

void Gui::BringWindowToDisplayFront(Window *window) {
  Context &g = *GGui;
  Window *current_front_window = g.Windows.back();
  if (current_front_window == window ||
      current_front_window->RootWindowDockTree ==
          window) // Cheap early out (could be better)
    return;
  for (int i = g.Windows.Size - 2; i >= 0;
       i--) // We can ignore the top-most window
    if (g.Windows[i] == window) {
      memmove(&g.Windows[i], &g.Windows[i + 1],
              (size_t)(g.Windows.Size - i - 1) * sizeof(Window *));
      g.Windows[g.Windows.Size - 1] = window;
      break;
    }
}

void Gui::BringWindowToDisplayBack(Window *window) {
  Context &g = *GGui;
  if (g.Windows[0] == window)
    return;
  for (int i = 0; i < g.Windows.Size; i++)
    if (g.Windows[i] == window) {
      memmove(&g.Windows[1], &g.Windows[0], (size_t)i * sizeof(Window *));
      g.Windows[0] = window;
      break;
    }
}

void Gui::BringWindowToDisplayBehind(Window *window, Window *behind_window) {
  ASSERT(window != NULL && behind_window != NULL);
  Context &g = *GGui;
  window = window->RootWindow;
  behind_window = behind_window->RootWindow;
  int pos_wnd = FindWindowDisplayIndex(window);
  int pos_beh = FindWindowDisplayIndex(behind_window);
  if (pos_wnd < pos_beh) {
    size_t copy_bytes = (pos_beh - pos_wnd - 1) * sizeof(Window *);
    memmove(&g.Windows.Data[pos_wnd], &g.Windows.Data[pos_wnd + 1], copy_bytes);
    g.Windows[pos_beh - 1] = window;
  } else {
    size_t copy_bytes = (pos_wnd - pos_beh) * sizeof(Window *);
    memmove(&g.Windows.Data[pos_beh + 1], &g.Windows.Data[pos_beh], copy_bytes);
    g.Windows[pos_beh] = window;
  }
}

int Gui::FindWindowDisplayIndex(Window *window) {
  Context &g = *GGui;
  return g.Windows.index_from_ptr(g.Windows.find(window));
}

// Moving window to front of display and set focus (which happens to be back of
// our sorted list)
void Gui::FocusWindow(Window *window, FocusRequestFlags flags) {
  Context &g = *GGui;

  // Modal check?
  if ((flags & FocusRequestFlags_UnlessBelowModal) &&
      (g.NavWindow != window)) // Early out in common case.
    if (Window *blocking_modal = FindBlockingModal(window)) {
      DEBUG_LOG_FOCUS("[focus] FocusWindow(\"%s\", UnlessBelowModal): "
                      "prevented by \"%s\".\n",
                      window ? window->Name : "<NULL>", blocking_modal->Name);
      if (window && window == window->RootWindow &&
          (window->Flags & WindowFlags_NoBringToFrontOnFocus) == 0)
        BringWindowToDisplayBehind(
            window, blocking_modal); // Still bring to right below modal.
      return;
    }

  // Find last focused child (if any) and focus it instead.
  if ((flags & FocusRequestFlags_RestoreFocusedChild) && window != NULL)
    window = NavRestoreLastChildNavWindow(window);

  // Apply focus
  if (g.NavWindow != window) {
    SetNavWindow(window);
    if (window && g.NavDisableMouseHover)
      g.NavMousePosDirty = true;
    g.NavId = window ? window->NavLastIds[0] : 0; // Restore NavId
    g.NavLayer = NavLayer_Main;
    g.NavFocusScopeId = window ? window->NavRootFocusScopeId : 0;
    g.NavIdIsAlive = false;
    g.NavLastValidSelectionUserData = SelectionUserData_Invalid;

    // Close popups if any
    ClosePopupsOverWindow(window, false);
  }

  // Move the root window to the top of the pile
  ASSERT(window == NULL || window->RootWindowDockTree != NULL);
  Window *focus_front_window = window ? window->RootWindow : NULL;
  Window *display_front_window = window ? window->RootWindowDockTree : NULL;
  DockNode *dock_node = window ? window->DockNode : NULL;
  bool active_id_window_is_dock_node_host =
      (g.ActiveIdWindow && dock_node &&
       dock_node->HostWindow == g.ActiveIdWindow);

  // Steal active widgets. Some of the cases it triggers includes:
  // - Focus a window while an InputText in another window is active, if focus
  // happens before the old InputText can run.
  // - When using Nav to activate menu items (due to timing of activating on
  // press->new window appears->losing ActiveId)
  // - Using dock host items (tab, collapse button) can trigger this before we
  // redirect the ActiveIdWindow toward the child window.
  if (g.ActiveId != 0 && g.ActiveIdWindow &&
      g.ActiveIdWindow->RootWindow != focus_front_window)
    if (!g.ActiveIdNoClearOnFocusLoss && !active_id_window_is_dock_node_host)
      ClearActiveID();

  // Passing NULL allow to disable keyboard focus
  if (!window)
    return;
  window->LastFrameJustFocused = g.FrameCount;

  // Select in dock node
  // For #2304 we avoid applying focus immediately before the tabbar is visible.
  // if (dock_node && dock_node->TabBar)
  //    dock_node->TabBar->SelectedTabId = dock_node->TabBar->NextSelectedTabId
  //    = window->TabId;

  // Bring to front
  BringWindowToFocusFront(focus_front_window);
  if (((window->Flags | focus_front_window->Flags |
        display_front_window->Flags) &
       WindowFlags_NoBringToFrontOnFocus) == 0)
    BringWindowToDisplayFront(display_front_window);
}

void Gui::FocusTopMostWindowUnderOne(Window *under_this_window,
                                     Window *ignore_window,
                                     Viewport *filter_viewport,
                                     FocusRequestFlags flags) {
  Context &g = *GGui;
  int start_idx = g.WindowsFocusOrder.Size - 1;
  if (under_this_window != NULL) {
    // Aim at root window behind us, if we are in a child window that's our own
    // root (see #4640)
    int offset = -1;
    while (under_this_window->Flags & WindowFlags_ChildWindow) {
      under_this_window = under_this_window->ParentWindow;
      offset = 0;
    }
    start_idx = FindWindowFocusIndex(under_this_window) + offset;
  }
  for (int i = start_idx; i >= 0; i--) {
    // We may later decide to test for different NoXXXInputs based on the active
    // navigation input (mouse vs nav) but that may feel more confusing to the
    // user.
    Window *window = g.WindowsFocusOrder[i];
    if (window == ignore_window || !window->WasActive)
      continue;
    if (filter_viewport != NULL && window->Viewport != filter_viewport)
      continue;
    if ((window->Flags &
         (WindowFlags_NoMouseInputs | WindowFlags_NoNavInputs)) !=
        (WindowFlags_NoMouseInputs | WindowFlags_NoNavInputs)) {
      // FIXME-DOCK: When FocusRequestFlags_RestoreFocusedChild is set...
      // This is failing (lagging by one frame) for docked windows.
      // If A and B are docked into window and B disappear, at the NewFrame()
      // call site window->NavLastChildNavWindow will still point to B. We might
      // leverage the tab order implicitly stored in
      // window->DockNodeAsHost->TabBar (essentially the
      // 'most_recently_selected_tab' code in tab bar will do that but on next
      // update) to tell which is the "previous" window. Or we may leverage
      // 'LastFrameFocused/LastFrameJustFocused' and have this function handle
      // child window itself?
      FocusWindow(window, flags);
      return;
    }
  }
  FocusWindow(NULL, flags);
}

// Important: this alone doesn't alter current DrawList state. This is called
// by PushFont/PopFont only.
void Gui::SetCurrentFont(Font *font) {
  Context &g = *GGui;
  ASSERT(
      font &&
      font->IsLoaded()); // Font Atlas not created. Did you call
                         // io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
  ASSERT(font->Scale > 0.0f);
  g.Font = font;
  g.FontBaseSize =
      Max(1.0f, g.IO.FontGlobalScale * g.Font->FontSize * g.Font->Scale);
  g.FontSize = g.CurrentWindow ? g.CurrentWindow->CalcFontSize() : 0.0f;

  FontAtlas *atlas = g.Font->ContainerAtlas;
  g.DrawListSharedData.TexUvWhitePixel = atlas->TexUvWhitePixel;
  g.DrawListSharedData.TexUvLines = atlas->TexUvLines;
  g.DrawListSharedData.Font = g.Font;
  g.DrawListSharedData.FontSize = g.FontSize;
}

void Gui::PushFont(Font *font) {
  Context &g = *GGui;
  if (!font)
    font = GetDefaultFont();
  SetCurrentFont(font);
  g.FontStack.push_back(font);
  g.CurrentWindow->DrawList->PushTextureID(font->ContainerAtlas->TexID);
}

void Gui::PopFont() {
  Context &g = *GGui;
  g.CurrentWindow->DrawList->PopTextureID();
  g.FontStack.pop_back();
  SetCurrentFont(g.FontStack.empty() ? GetDefaultFont() : g.FontStack.back());
}

void Gui::PushItemFlag(ItemFlags option, bool enabled) {
  Context &g = *GGui;
  ItemFlags item_flags = g.CurrentItemFlags;
  ASSERT(item_flags == g.ItemFlagsStack.back());
  if (enabled)
    item_flags |= option;
  else
    item_flags &= ~option;
  g.CurrentItemFlags = item_flags;
  g.ItemFlagsStack.push_back(item_flags);
}

void Gui::PopItemFlag() {
  Context &g = *GGui;
  ASSERT(g.ItemFlagsStack.Size >
         1); // Too many calls to PopItemFlag() - we always leave a 0 at the
             // bottom of the stack.
  g.ItemFlagsStack.pop_back();
  g.CurrentItemFlags = g.ItemFlagsStack.back();
}

// BeginDisabled()/EndDisabled()
// - Those can be nested but it cannot be used to enable an already disabled
// section (a single BeginDisabled(true) in the stack is enough to keep
// everything disabled)
// - Visually this is currently altering alpha, but it is expected that in a
// future styling system this would work differently.
// - BeginDisabled(false) essentially does nothing useful but is provided to
// facilitate use of boolean expressions. If you can avoid calling
// BeginDisabled(False)/EndDisabled() best to avoid it.
// - Optimized shortcuts instead of PushStyleVar() + PushItemFlag()
void Gui::BeginDisabled(bool disabled) {
  Context &g = *GGui;
  bool was_disabled = (g.CurrentItemFlags & ItemFlags_Disabled) != 0;
  if (!was_disabled && disabled) {
    g.DisabledAlphaBackup = g.Style.Alpha;
    g.Style.Alpha *=
        g.Style.DisabledAlpha; // PushStyleVar(StyleVar_Alpha,
                               // g.Style.Alpha * g.Style.DisabledAlpha);
  }
  if (was_disabled || disabled)
    g.CurrentItemFlags |= ItemFlags_Disabled;
  g.ItemFlagsStack.push_back(g.CurrentItemFlags);
  g.DisabledStackSize++;
}

void Gui::EndDisabled() {
  Context &g = *GGui;
  ASSERT(g.DisabledStackSize > 0);
  g.DisabledStackSize--;
  bool was_disabled = (g.CurrentItemFlags & ItemFlags_Disabled) != 0;
  // PopItemFlag();
  g.ItemFlagsStack.pop_back();
  g.CurrentItemFlags = g.ItemFlagsStack.back();
  if (was_disabled && (g.CurrentItemFlags & ItemFlags_Disabled) == 0)
    g.Style.Alpha = g.DisabledAlphaBackup; // PopStyleVar();
}

void Gui::PushTabStop(bool tab_stop) {
  PushItemFlag(ItemFlags_NoTabStop, !tab_stop);
}

void Gui::PopTabStop() { PopItemFlag(); }

void Gui::PushButtonRepeat(bool repeat) {
  PushItemFlag(ItemFlags_ButtonRepeat, repeat);
}

void Gui::PopButtonRepeat() { PopItemFlag(); }

void Gui::PushTextWrapPos(float wrap_pos_x) {
  Window *window = GetCurrentWindow();
  window->DC.TextWrapPosStack.push_back(window->DC.TextWrapPos);
  window->DC.TextWrapPos = wrap_pos_x;
}

void Gui::PopTextWrapPos() {
  Window *window = GetCurrentWindow();
  window->DC.TextWrapPos = window->DC.TextWrapPosStack.back();
  window->DC.TextWrapPosStack.pop_back();
}

static Window *GetCombinedRootWindow(Window *window, bool popup_hierarchy,
                                     bool dock_hierarchy) {
  Window *last_window = NULL;
  while (last_window != window) {
    last_window = window;
    window = window->RootWindow;
    if (popup_hierarchy)
      window = window->RootWindowPopupTree;
    if (dock_hierarchy)
      window = window->RootWindowDockTree;
  }
  return window;
}

bool Gui::IsWindowChildOf(Window *window, Window *potential_parent,
                          bool popup_hierarchy, bool dock_hierarchy) {
  Window *window_root =
      GetCombinedRootWindow(window, popup_hierarchy, dock_hierarchy);
  if (window_root == potential_parent)
    return true;
  while (window != NULL) {
    if (window == potential_parent)
      return true;
    if (window == window_root) // end of chain
      return false;
    window = window->ParentWindow;
  }
  return false;
}

bool Gui::IsWindowWithinBeginStackOf(Window *window, Window *potential_parent) {
  if (window->RootWindow == potential_parent)
    return true;
  while (window != NULL) {
    if (window == potential_parent)
      return true;
    window = window->ParentWindowInBeginStack;
  }
  return false;
}

bool Gui::IsWindowAbove(Window *potential_above, Window *potential_below) {
  Context &g = *GGui;

  // It would be saner to ensure that display layer is always reflected in the
  // g.Windows[] order, which would likely requires altering all manipulations
  // of that array
  const int display_layer_delta = GetWindowDisplayLayer(potential_above) -
                                  GetWindowDisplayLayer(potential_below);
  if (display_layer_delta != 0)
    return display_layer_delta > 0;

  for (int i = g.Windows.Size - 1; i >= 0; i--) {
    Window *candidate_window = g.Windows[i];
    if (candidate_window == potential_above)
      return true;
    if (candidate_window == potential_below)
      return false;
  }
  return false;
}

// Is current window hovered and hoverable (e.g. not blocked by a popup/modal)?
// See HoveredFlags_ for options. IMPORTANT: If you are trying to check
// whether your mouse should be dispatched to Gui or to your underlying
// app, you should not use this function! Use the 'io.WantCaptureMouse' boolean
// for that! Refer to FAQ entry "How can I tell whether to dispatch
// mouse/keyboard to Gui or my application?" for details.
bool Gui::IsWindowHovered(HoveredFlags flags) {
  ASSERT((flags & ~HoveredFlags_AllowedMaskForIsWindowHovered) == 0 &&
         "Invalid flags for IsWindowHovered()!");

  Context &g = *GGui;
  Window *ref_window = g.HoveredWindow;
  Window *cur_window = g.CurrentWindow;
  if (ref_window == NULL)
    return false;

  if ((flags & HoveredFlags_AnyWindow) == 0) {
    ASSERT(cur_window); // Not inside a Begin()/End()
    const bool popup_hierarchy = (flags & HoveredFlags_NoPopupHierarchy) == 0;
    const bool dock_hierarchy = (flags & HoveredFlags_DockHierarchy) != 0;
    if (flags & HoveredFlags_RootWindow)
      cur_window =
          GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

    bool result;
    if (flags & HoveredFlags_ChildWindows)
      result = IsWindowChildOf(ref_window, cur_window, popup_hierarchy,
                               dock_hierarchy);
    else
      result = (ref_window == cur_window);
    if (!result)
      return false;
  }

  if (!IsWindowContentHoverable(ref_window, flags))
    return false;
  if (!(flags & HoveredFlags_AllowWhenBlockedByActiveItem))
    if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap &&
        g.ActiveId != ref_window->MoveId)
      return false;

  // When changing hovered window we requires a bit of stationary delay before
  // activating hover timer.
  // FIXME: We don't support delay other than stationary one for now, other
  // delay would need a way to fullfill the possibility that multiple
  // IsWindowHovered() with varying flag could return true for different windows
  // of the hierarchy. Possibly need a Hash(Current+Flags) ==> (Timer) cache. We
  // can implement this for _Stationary because the data is linked to
  // HoveredWindow rather than CurrentWindow.
  if (flags & HoveredFlags_ForTooltip)
    flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);
  if ((flags & HoveredFlags_Stationary) != 0 &&
      g.HoverWindowUnlockedStationaryId != ref_window->ID)
    return false;

  return true;
}

bool Gui::IsWindowFocused(FocusedFlags flags) {
  Context &g = *GGui;
  Window *ref_window = g.NavWindow;
  Window *cur_window = g.CurrentWindow;

  if (ref_window == NULL)
    return false;
  if (flags & FocusedFlags_AnyWindow)
    return true;

  ASSERT(cur_window); // Not inside a Begin()/End()
  const bool popup_hierarchy = (flags & FocusedFlags_NoPopupHierarchy) == 0;
  const bool dock_hierarchy = (flags & FocusedFlags_DockHierarchy) != 0;
  if (flags & HoveredFlags_RootWindow)
    cur_window =
        GetCombinedRootWindow(cur_window, popup_hierarchy, dock_hierarchy);

  if (flags & HoveredFlags_ChildWindows)
    return IsWindowChildOf(ref_window, cur_window, popup_hierarchy,
                           dock_hierarchy);
  else
    return (ref_window == cur_window);
}

ID Gui::GetWindowDockID() {
  Context &g = *GGui;
  return g.CurrentWindow->DockId;
}

bool Gui::IsWindowDocked() {
  Context &g = *GGui;
  return g.CurrentWindow->DockIsActive;
}

// Can we focus this window with CTRL+TAB (or PadMenu +
// PadFocusPrev/PadFocusNext) Note that NoNavFocus makes the window not
// reachable with CTRL+TAB but it can still be focused with mouse or
// programmatically. If you want a window to never be focused, you may use the
// e.g. NoInputs flag.
bool Gui::IsWindowNavFocusable(Window *window) {
  return window->WasActive && window == window->RootWindow &&
         !(window->Flags & WindowFlags_NoNavFocus);
}

float Gui::GetWindowWidth() {
  Window *window = GGui->CurrentWindow;
  return window->Size.x;
}

float Gui::GetWindowHeight() {
  Window *window = GGui->CurrentWindow;
  return window->Size.y;
}

Vec2 Gui::GetWindowPos() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  return window->Pos;
}

void Gui::SetWindowPos(Window *window, const Vec2 &pos, Cond cond) {
  // Test condition (NB: bit 0 is always true) and clear flags for next time
  if (cond && (window->SetWindowPosAllowFlags & cond) == 0)
    return;

  ASSERT(cond == 0 ||
         IsPowerOfTwo(cond)); // Make sure the user doesn't attempt to
                              // combine multiple condition flags.
  window->SetWindowPosAllowFlags &=
      ~(Cond_Once | Cond_FirstUseEver | Cond_Appearing);
  window->SetWindowPosVal = Vec2(FLT_MAX, FLT_MAX);

  // Set
  const Vec2 old_pos = window->Pos;
  window->Pos = Trunc(pos);
  Vec2 offset = window->Pos - old_pos;
  if (offset.x == 0.0f && offset.y == 0.0f)
    return;
  MarkIniSettingsDirty(window);
  // FIXME: share code with TranslateWindow(), need to confirm whether the 3
  // rect modified by TranslateWindow() are desirable here.
  window->DC.CursorPos +=
      offset; // As we happen to move the window while it is being appended to
              // (which is a bad idea - will smear) let's at least offset the
              // cursor
  window->DC.CursorMaxPos +=
      offset; // And more importantly we need to offset
              // CursorMaxPos/CursorStartPos this so ContentSize calculation
              // doesn't get affected.
  window->DC.IdealMaxPos += offset;
  window->DC.CursorStartPos += offset;
}

void Gui::SetWindowPos(const Vec2 &pos, Cond cond) {
  Window *window = GetCurrentWindowRead();
  SetWindowPos(window, pos, cond);
}

void Gui::SetWindowPos(const char *name, const Vec2 &pos, Cond cond) {
  if (Window *window = FindWindowByName(name))
    SetWindowPos(window, pos, cond);
}

Vec2 Gui::GetWindowSize() {
  Window *window = GetCurrentWindowRead();
  return window->Size;
}

void Gui::SetWindowSize(Window *window, const Vec2 &size, Cond cond) {
  // Test condition (NB: bit 0 is always true) and clear flags for next time
  if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
    return;

  ASSERT(cond == 0 ||
         IsPowerOfTwo(cond)); // Make sure the user doesn't attempt to
                              // combine multiple condition flags.
  window->SetWindowSizeAllowFlags &=
      ~(Cond_Once | Cond_FirstUseEver | Cond_Appearing);

  // Enable auto-fit (not done in BeginChild() path unless appearing or combined
  // with ChildFlags_AlwaysAutoResize)
  if ((window->Flags & WindowFlags_ChildWindow) == 0 || window->Appearing ||
      (window->ChildFlags & ChildFlags_AlwaysAutoResize) != 0)
    window->AutoFitFramesX = (size.x <= 0.0f) ? 2 : 0;
  if ((window->Flags & WindowFlags_ChildWindow) == 0 || window->Appearing ||
      (window->ChildFlags & ChildFlags_AlwaysAutoResize) != 0)
    window->AutoFitFramesY = (size.y <= 0.0f) ? 2 : 0;

  // Set
  Vec2 old_size = window->SizeFull;
  if (size.x <= 0.0f)
    window->AutoFitOnlyGrows = false;
  else
    window->SizeFull.x = TRUNC(size.x);
  if (size.y <= 0.0f)
    window->AutoFitOnlyGrows = false;
  else
    window->SizeFull.y = TRUNC(size.y);
  if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
    MarkIniSettingsDirty(window);
}

void Gui::SetWindowSize(const Vec2 &size, Cond cond) {
  SetWindowSize(GGui->CurrentWindow, size, cond);
}

void Gui::SetWindowSize(const char *name, const Vec2 &size, Cond cond) {
  if (Window *window = FindWindowByName(name))
    SetWindowSize(window, size, cond);
}

void Gui::SetWindowCollapsed(Window *window, bool collapsed, Cond cond) {
  // Test condition (NB: bit 0 is always true) and clear flags for next time
  if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
    return;
  window->SetWindowCollapsedAllowFlags &=
      ~(Cond_Once | Cond_FirstUseEver | Cond_Appearing);

  // Set
  window->Collapsed = collapsed;
}

void Gui::SetWindowHitTestHole(Window *window, const Vec2 &pos,
                               const Vec2 &size) {
  ASSERT(window->HitTestHoleSize.x ==
         0); // We don't support multiple holes/hit test filters
  window->HitTestHoleSize = Vec2ih(size);
  window->HitTestHoleOffset = Vec2ih(pos - window->Pos);
}

void Gui::SetWindowHiddenAndSkipItemsForCurrentFrame(Window *window) {
  window->Hidden = window->SkipItems = true;
  window->HiddenFramesCanSkipItems = 1;
}

void Gui::SetWindowCollapsed(bool collapsed, Cond cond) {
  SetWindowCollapsed(GGui->CurrentWindow, collapsed, cond);
}

bool Gui::IsWindowCollapsed() {
  Window *window = GetCurrentWindowRead();
  return window->Collapsed;
}

bool Gui::IsWindowAppearing() {
  Window *window = GetCurrentWindowRead();
  return window->Appearing;
}

void Gui::SetWindowCollapsed(const char *name, bool collapsed, Cond cond) {
  if (Window *window = FindWindowByName(name))
    SetWindowCollapsed(window, collapsed, cond);
}

void Gui::SetWindowFocus() { FocusWindow(GGui->CurrentWindow); }

void Gui::SetWindowFocus(const char *name) {
  if (name) {
    if (Window *window = FindWindowByName(name))
      FocusWindow(window);
  } else {
    FocusWindow(NULL);
  }
}

void Gui::SetNextWindowPos(const Vec2 &pos, Cond cond, const Vec2 &pivot) {
  Context &g = *GGui;
  ASSERT(cond == 0 ||
         IsPowerOfTwo(cond)); // Make sure the user doesn't attempt to
                              // combine multiple condition flags.
  g.NextWindowData.Flags |= NextWindowDataFlags_HasPos;
  g.NextWindowData.PosVal = pos;
  g.NextWindowData.PosPivotVal = pivot;
  g.NextWindowData.PosCond = cond ? cond : Cond_Always;
  g.NextWindowData.PosUndock = true;
}

void Gui::SetNextWindowSize(const Vec2 &size, Cond cond) {
  Context &g = *GGui;
  ASSERT(cond == 0 ||
         IsPowerOfTwo(cond)); // Make sure the user doesn't attempt to
                              // combine multiple condition flags.
  g.NextWindowData.Flags |= NextWindowDataFlags_HasSize;
  g.NextWindowData.SizeVal = size;
  g.NextWindowData.SizeCond = cond ? cond : Cond_Always;
}

// For each axis:
// - Use 0.0f as min or FLT_MAX as max if you don't want limits, e.g. size_min =
// (500.0f, 0.0f), size_max = (FLT_MAX, FLT_MAX) sets a minimum width.
// - Use -1 for both min and max of same axis to preserve current size which
// itself is a constraint.
// - See "Demo->Examples->Constrained-resizing window" for examples.
void Gui::SetNextWindowSizeConstraints(const Vec2 &size_min,
                                       const Vec2 &size_max,
                                       SizeCallback custom_callback,
                                       void *custom_callback_user_data) {
  Context &g = *GGui;
  g.NextWindowData.Flags |= NextWindowDataFlags_HasSizeConstraint;
  g.NextWindowData.SizeConstraintRect = Rect(size_min, size_max);
  g.NextWindowData.SizeCallback = custom_callback;
  g.NextWindowData.SizeCallbackUserData = custom_callback_user_data;
}

// Content size = inner scrollable rectangle, padded with WindowPadding.
// SetNextWindowContentSize(Vec2(100,100) + WindowFlags_AlwaysAutoResize
// will always allow submitting a 100x100 item.
void Gui::SetNextWindowContentSize(const Vec2 &size) {
  Context &g = *GGui;
  g.NextWindowData.Flags |= NextWindowDataFlags_HasContentSize;
  g.NextWindowData.ContentSizeVal = Trunc(size);
}

void Gui::SetNextWindowScroll(const Vec2 &scroll) {
  Context &g = *GGui;
  g.NextWindowData.Flags |= NextWindowDataFlags_HasScroll;
  g.NextWindowData.ScrollVal = scroll;
}

void Gui::SetNextWindowCollapsed(bool collapsed, Cond cond) {
  Context &g = *GGui;
  ASSERT(cond == 0 ||
         IsPowerOfTwo(cond)); // Make sure the user doesn't attempt to
                              // combine multiple condition flags.
  g.NextWindowData.Flags |= NextWindowDataFlags_HasCollapsed;
  g.NextWindowData.CollapsedVal = collapsed;
  g.NextWindowData.CollapsedCond = cond ? cond : Cond_Always;
}

void Gui::SetNextWindowFocus() {
  Context &g = *GGui;
  g.NextWindowData.Flags |= NextWindowDataFlags_HasFocus;
}

void Gui::SetNextWindowBgAlpha(float alpha) {
  Context &g = *GGui;
  g.NextWindowData.Flags |= NextWindowDataFlags_HasBgAlpha;
  g.NextWindowData.BgAlphaVal = alpha;
}

void Gui::SetNextWindowViewport(ID id) {
  Context &g = *GGui;
  g.NextWindowData.Flags |= NextWindowDataFlags_HasViewport;
  g.NextWindowData.ViewportId = id;
}

void Gui::SetNextWindowDockID(ID id, Cond cond) {
  Context &g = *GGui;
  g.NextWindowData.Flags |= NextWindowDataFlags_HasDock;
  g.NextWindowData.DockCond = cond ? cond : Cond_Always;
  g.NextWindowData.DockId = id;
}

void Gui::SetNextWindowClass(const WindowClass *window_class) {
  Context &g = *GGui;
  ASSERT((window_class->ViewportFlagsOverrideSet &
          window_class->ViewportFlagsOverrideClear) ==
         0); // Cannot set both set and clear for the same bit
  g.NextWindowData.Flags |= NextWindowDataFlags_HasWindowClass;
  g.NextWindowData.WindowClass = *window_class;
}

DrawList *Gui::GetWindowDrawList() {
  Window *window = GetCurrentWindow();
  return window->DrawList;
}

float Gui::GetWindowDpiScale() {
  Context &g = *GGui;
  return g.CurrentDpiScale;
}

Viewport *Gui::GetWindowViewport() {
  Context &g = *GGui;
  ASSERT(g.CurrentViewport != NULL &&
         g.CurrentViewport == g.CurrentWindow->Viewport);
  return g.CurrentViewport;
}

Font *Gui::GetFont() { return GGui->Font; }

float Gui::GetFontSize() { return GGui->FontSize; }

Vec2 Gui::GetFontTexUvWhitePixel() {
  return GGui->DrawListSharedData.TexUvWhitePixel;
}

void Gui::SetWindowFontScale(float scale) {
  ASSERT(scale > 0.0f);
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  window->FontWindowScale = scale;
  g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
}

void Gui::PushFocusScope(ID id) {
  Context &g = *GGui;
  g.FocusScopeStack.push_back(id);
  g.CurrentFocusScopeId = id;
}

void Gui::PopFocusScope() {
  Context &g = *GGui;
  ASSERT(g.FocusScopeStack.Size > 0); // Too many PopFocusScope() ?
  g.FocusScopeStack.pop_back();
  g.CurrentFocusScopeId = g.FocusScopeStack.Size ? g.FocusScopeStack.back() : 0;
}

// Focus = move navigation cursor, set scrolling, set focus window.
void Gui::FocusItem() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  DEBUG_LOG_FOCUS("FocusItem(0x%08x) in window \"%s\"\n", g.LastItemData.ID,
                  window->Name);
  if (g.DragDropActive ||
      g.MovingWindow != NULL) // FIXME: Opt-in flags for this?
  {
    DEBUG_LOG_FOCUS("FocusItem() ignored while DragDropActive!\n");
    return;
  }

  NavMoveFlags move_flags = NavMoveFlags_IsTabbing | NavMoveFlags_FocusApi |
                            NavMoveFlags_NoSetNavHighlight |
                            NavMoveFlags_NoSelect;
  ScrollFlags scroll_flags =
      window->Appearing
          ? ScrollFlags_KeepVisibleEdgeX | ScrollFlags_AlwaysCenterY
          : ScrollFlags_KeepVisibleEdgeX | ScrollFlags_KeepVisibleEdgeY;
  SetNavWindow(window);
  NavMoveRequestSubmit(Dir_None, Dir_Up, move_flags, scroll_flags);
  NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
}

void Gui::ActivateItemByID(ID id) {
  Context &g = *GGui;
  g.NavNextActivateId = id;
  g.NavNextActivateFlags = ActivateFlags_None;
}

// Note: this will likely be called ActivateItem() once we rework our
// Focus/Activation system! But ActivateItem() should function without altering
// scroll/focus?
void Gui::SetKeyboardFocusHere(int offset) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT(offset >= -1); // -1 is allowed but not below
  DEBUG_LOG_FOCUS("SetKeyboardFocusHere(%d) in window \"%s\"\n", offset,
                  window->Name);

  // It makes sense in the vast majority of cases to never interrupt a drag and
  // drop. When we refactor this function into ActivateItem() we may want to
  // make this an option. MovingWindow is protected from most user inputs using
  // SetActiveIdUsingNavAndKeys(), but is also automatically dropped in the
  // event g.ActiveId is stolen.
  if (g.DragDropActive || g.MovingWindow != NULL) {
    DEBUG_LOG_FOCUS("SetKeyboardFocusHere() ignored while DragDropActive!\n");
    return;
  }

  SetNavWindow(window);

  NavMoveFlags move_flags = NavMoveFlags_IsTabbing | NavMoveFlags_Activate |
                            NavMoveFlags_FocusApi |
                            NavMoveFlags_NoSetNavHighlight;
  ScrollFlags scroll_flags =
      window->Appearing
          ? ScrollFlags_KeepVisibleEdgeX | ScrollFlags_AlwaysCenterY
          : ScrollFlags_KeepVisibleEdgeX | ScrollFlags_KeepVisibleEdgeY;
  NavMoveRequestSubmit(
      Dir_None, offset < 0 ? Dir_Up : Dir_Down, move_flags,
      scroll_flags); // FIXME-NAV: Once we refactor tabbing, add LegacyApi flag
                     // to not activate non-inputable.
  if (offset == -1) {
    NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
  } else {
    g.NavTabbingDir = 1;
    g.NavTabbingCounter = offset + 1;
  }
}

void Gui::SetItemDefaultFocus() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (!window->Appearing)
    return;
  if (g.NavWindow != window->RootWindowForNav ||
      (!g.NavInitRequest && g.NavInitResult.ID == 0) ||
      g.NavLayer != window->DC.NavLayerCurrent)
    return;

  g.NavInitRequest = false;
  NavApplyItemToResult(&g.NavInitResult);
  NavUpdateAnyRequestFlag();

  // Scroll could be done in NavInitRequestApplyResult() via an opt-in flag (we
  // however don't want regular init requests to scroll)
  if (!window->ClipRect.Contains(g.LastItemData.Rect))
    ScrollToRectEx(window, g.LastItemData.Rect, ScrollFlags_None);
}

void Gui::SetStateStorage(Storage *tree) {
  Window *window = GGui->CurrentWindow;
  window->DC.StateStorage = tree ? tree : &window->StateStorage;
}

Storage *Gui::GetStateStorage() {
  Window *window = GGui->CurrentWindow;
  return window->DC.StateStorage;
}

void Gui::PushID(const char *str_id) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ID id = window->GetID(str_id);
  window->IDStack.push_back(id);
}

void Gui::PushID(const char *str_id_begin, const char *str_id_end) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ID id = window->GetID(str_id_begin, str_id_end);
  window->IDStack.push_back(id);
}

void Gui::PushID(const void *ptr_id) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ID id = window->GetID(ptr_id);
  window->IDStack.push_back(id);
}

void Gui::PushID(int int_id) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ID id = window->GetID(int_id);
  window->IDStack.push_back(id);
}

// Push a given id value ignoring the ID stack as a seed.
void Gui::PushOverrideID(ID id) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (g.DebugHookIdInfo == id)
    DebugHookIdInfo(id, DataType_ID, NULL, NULL);
  window->IDStack.push_back(id);
}

// Helper to avoid a common series of PushOverrideID -> GetID() -> PopID() call
// (note that when using this pattern, ID Stack Tool will tend to not display
// the intermediate stack level.
//  for that to work we would need to do PushOverrideID() -> ItemAdd() ->
//  PopID() which would alter widget code a little more)
ID Gui::GetIDWithSeed(const char *str, const char *str_end, ID seed) {
  ID id = HashStr(str, str_end ? (str_end - str) : 0, seed);
  Context &g = *GGui;
  if (g.DebugHookIdInfo == id)
    DebugHookIdInfo(id, DataType_String, str, str_end);
  return id;
}

ID Gui::GetIDWithSeed(int n, ID seed) {
  ID id = HashData(&n, sizeof(n), seed);
  Context &g = *GGui;
  if (g.DebugHookIdInfo == id)
    DebugHookIdInfo(id, DataType_S32, (void *)(intptr_t)n, NULL);
  return id;
}

void Gui::PopID() {
  Window *window = GGui->CurrentWindow;
  ASSERT(
      window->IDStack.Size >
      1); // Too many PopID(), or could be popping in a wrong/different window?
  window->IDStack.pop_back();
}

ID Gui::GetID(const char *str_id) {
  Window *window = GGui->CurrentWindow;
  return window->GetID(str_id);
}

ID Gui::GetID(const char *str_id_begin, const char *str_id_end) {
  Window *window = GGui->CurrentWindow;
  return window->GetID(str_id_begin, str_id_end);
}

ID Gui::GetID(const void *ptr_id) {
  Window *window = GGui->CurrentWindow;
  return window->GetID(ptr_id);
}

bool Gui::IsRectVisible(const Vec2 &size) {
  Window *window = GGui->CurrentWindow;
  return window->ClipRect.Overlaps(
      Rect(window->DC.CursorPos, window->DC.CursorPos + size));
}

bool Gui::IsRectVisible(const Vec2 &rect_min, const Vec2 &rect_max) {
  Window *window = GGui->CurrentWindow;
  return window->ClipRect.Overlaps(Rect(rect_min, rect_max));
}

//-----------------------------------------------------------------------------
// [SECTION] INPUTS
//-----------------------------------------------------------------------------
// - GetKeyData() [Internal]
// - GetKeyIndex() [Internal]
// - GetKeyName()
// - GetKeyChordName() [Internal]
// - CalcTypematicRepeatAmount() [Internal]
// - GetTypematicRepeatRate() [Internal]
// - GetKeyPressedAmount() [Internal]
// - GetKeyMagnitude2d() [Internal]
//-----------------------------------------------------------------------------
// - UpdateKeyRoutingTable() [Internal]
// - GetRoutingIdFromOwnerId() [Internal]
// - GetShortcutRoutingData() [Internal]
// - CalcRoutingScore() [Internal]
// - SetShortcutRouting() [Internal]
// - TestShortcutRouting() [Internal]
//-----------------------------------------------------------------------------
// - IsKeyDown()
// - IsKeyPressed()
// - IsKeyReleased()
//-----------------------------------------------------------------------------
// - IsMouseDown()
// - IsMouseClicked()
// - IsMouseReleased()
// - IsMouseDoubleClicked()
// - GetMouseClickedCount()
// - IsMouseHoveringRect() [Internal]
// - IsMouseDragPastThreshold() [Internal]
// - IsMouseDragging()
// - GetMousePos()
// - SetMousePos() [Internal]
// - GetMousePosOnOpeningCurrentPopup()
// - IsMousePosValid()
// - IsAnyMouseDown()
// - GetMouseDragDelta()
// - ResetMouseDragDelta()
// - GetMouseCursor()
// - SetMouseCursor()
//-----------------------------------------------------------------------------
// - UpdateAliasKey()
// - GetMergedModsFromKeys()
// - UpdateKeyboardInputs()
// - UpdateMouseInputs()
//-----------------------------------------------------------------------------
// - LockWheelingWindow [Internal]
// - FindBestWheelingWindow [Internal]
// - UpdateMouseWheel() [Internal]
//-----------------------------------------------------------------------------
// - SetNextFrameWantCaptureKeyboard()
// - SetNextFrameWantCaptureMouse()
//-----------------------------------------------------------------------------
// - GetInputSourceName() [Internal]
// - DebugPrintInputEvent() [Internal]
// - UpdateInputEvents() [Internal]
//-----------------------------------------------------------------------------
// - GetKeyOwner() [Internal]
// - TestKeyOwner() [Internal]
// - SetKeyOwner() [Internal]
// - SetItemKeyOwner() [Internal]
// - Shortcut() [Internal]
//-----------------------------------------------------------------------------

KeyData *Gui::GetKeyData(Context *ctx, Key key) {
  Context &g = *ctx;

  // Special storage location for mods
  if (key & Mod_Mask_)
    key = ConvertSingleModFlagToKey(ctx, key);

#ifndef DISABLE_OBSOLETE_KEYIO
  ASSERT(key >= Key_LegacyNativeKey_BEGIN && key < Key_NamedKey_END);
  if (IsLegacyKey(key) && g.IO.KeyMap[key] != -1)
    key = (Key)g.IO.KeyMap[key]; // Remap native->imgui or imgui->native
#else
  ASSERT(IsNamedKey(key) &&
         "Support for user key indices was dropped in favor of Key. "
         "Please update backend & user code.");
#endif
  return &g.IO.KeysData[key - Key_KeysData_OFFSET];
}

#ifndef DISABLE_OBSOLETE_KEYIO
Key Gui::GetKeyIndex(Key key) {
  Context &g = *GGui;
  ASSERT(IsNamedKey(key));
  const KeyData *key_data = GetKeyData(key);
  return (Key)(key_data - g.IO.KeysData);
}
#endif

// Those names a provided for debugging purpose and are not meant to be saved
// persistently not compared.
static const char *const GKeyNames[] = {
    "Tab",
    "LeftArrow",
    "RightArrow",
    "UpArrow",
    "DownArrow",
    "PageUp",
    "PageDown",
    "Home",
    "End",
    "Insert",
    "Delete",
    "Backspace",
    "Space",
    "Enter",
    "Escape",
    "LeftCtrl",
    "LeftShift",
    "LeftAlt",
    "LeftSuper",
    "RightCtrl",
    "RightShift",
    "RightAlt",
    "RightSuper",
    "Menu",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Apostrophe",
    "Comma",
    "Minus",
    "Period",
    "Slash",
    "Semicolon",
    "Equal",
    "LeftBracket",
    "Backslash",
    "RightBracket",
    "GraveAccent",
    "CapsLock",
    "ScrollLock",
    "NumLock",
    "PrintScreen",
    "Pause",
    "Keypad0",
    "Keypad1",
    "Keypad2",
    "Keypad3",
    "Keypad4",
    "Keypad5",
    "Keypad6",
    "Keypad7",
    "Keypad8",
    "Keypad9",
    "KeypadDecimal",
    "KeypadDivide",
    "KeypadMultiply",
    "KeypadSubtract",
    "KeypadAdd",
    "KeypadEnter",
    "KeypadEqual",
    "AppBack",
    "AppForward",
    "GamepadStart",
    "GamepadBack",
    "GamepadFaceLeft",
    "GamepadFaceRight",
    "GamepadFaceUp",
    "GamepadFaceDown",
    "GamepadDpadLeft",
    "GamepadDpadRight",
    "GamepadDpadUp",
    "GamepadDpadDown",
    "GamepadL1",
    "GamepadR1",
    "GamepadL2",
    "GamepadR2",
    "GamepadL3",
    "GamepadR3",
    "GamepadLStickLeft",
    "GamepadLStickRight",
    "GamepadLStickUp",
    "GamepadLStickDown",
    "GamepadRStickLeft",
    "GamepadRStickRight",
    "GamepadRStickUp",
    "GamepadRStickDown",
    "MouseLeft",
    "MouseRight",
    "MouseMiddle",
    "MouseX1",
    "MouseX2",
    "MouseWheelX",
    "MouseWheelY",
    "ModCtrl",
    "ModShift",
    "ModAlt",
    "ModSuper", // ReservedForModXXX are showing the ModXXX names.
};
STATIC_ASSERT(Key_NamedKey_COUNT == ARRAYSIZE(GKeyNames));

const char *Gui::GetKeyName(Key key) {
  Context &g = *GGui;
#ifdef DISABLE_OBSOLETE_KEYIO
  ASSERT((IsNamedKeyOrModKey(key) || key == Key_None) &&
         "Support for user key indices was dropped in favor of Key. "
         "Please update backend and user code.");
#else
  if (IsLegacyKey(key)) {
    if (g.IO.KeyMap[key] == -1)
      return "N/A";
    ASSERT(IsNamedKey((Key)g.IO.KeyMap[key]));
    key = (Key)g.IO.KeyMap[key];
  }
#endif
  if (key == Key_None)
    return "None";
  if (key & Mod_Mask_)
    key = ConvertSingleModFlagToKey(&g, key);
  if (!IsNamedKey(key))
    return "Unknown";

  return GKeyNames[key - Key_NamedKey_BEGIN];
}

// Mod_Shortcut is translated to either Ctrl or Super.
void Gui::GetKeyChordName(KeyChord key_chord, char *out_buf, int out_buf_size) {
  Context &g = *GGui;
  if (key_chord & Mod_Shortcut)
    key_chord = ConvertShortcutMod(key_chord);
  FormatString(out_buf, (size_t)out_buf_size, "%s%s%s%s%s",
               (key_chord & Mod_Ctrl) ? "Ctrl+" : "",
               (key_chord & Mod_Shift) ? "Shift+" : "",
               (key_chord & Mod_Alt) ? "Alt+" : "",
               (key_chord & Mod_Super)
                   ? (g.IO.ConfigMacOSXBehaviors ? "Cmd+" : "Super+")
                   : "",
               GetKeyName((Key)(key_chord & ~Mod_Mask_)));
}

// t0 = previous time (e.g.: g.Time - g.IO.DeltaTime)
// t1 = current time (e.g.: g.Time)
// An event is triggered at:
//  t = 0.0f     t = repeat_delay,    t = repeat_delay + repeat_rate*N
int Gui::CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay,
                                   float repeat_rate) {
  if (t1 == 0.0f)
    return 1;
  if (t0 >= t1)
    return 0;
  if (repeat_rate <= 0.0f)
    return (t0 < repeat_delay) && (t1 >= repeat_delay);
  const int count_t0 =
      (t0 < repeat_delay) ? -1 : (int)((t0 - repeat_delay) / repeat_rate);
  const int count_t1 =
      (t1 < repeat_delay) ? -1 : (int)((t1 - repeat_delay) / repeat_rate);
  const int count = count_t1 - count_t0;
  return count;
}

void Gui::GetTypematicRepeatRate(InputFlags flags, float *repeat_delay,
                                 float *repeat_rate) {
  Context &g = *GGui;
  switch (flags & InputFlags_RepeatRateMask_) {
  case InputFlags_RepeatRateNavMove:
    *repeat_delay = g.IO.KeyRepeatDelay * 0.72f;
    *repeat_rate = g.IO.KeyRepeatRate * 0.80f;
    return;
  case InputFlags_RepeatRateNavTweak:
    *repeat_delay = g.IO.KeyRepeatDelay * 0.72f;
    *repeat_rate = g.IO.KeyRepeatRate * 0.30f;
    return;
  case InputFlags_RepeatRateDefault:
  default:
    *repeat_delay = g.IO.KeyRepeatDelay * 1.00f;
    *repeat_rate = g.IO.KeyRepeatRate * 1.00f;
    return;
  }
}

// Return value representing the number of presses in the last time period, for
// the given repeat rate (most often returns 0 or 1. The result is generally
// only >1 when RepeatRate is smaller than DeltaTime, aka large DeltaTime or
// fast RepeatRate)
int Gui::GetKeyPressedAmount(Key key, float repeat_delay, float repeat_rate) {
  Context &g = *GGui;
  const KeyData *key_data = GetKeyData(key);
  if (!key_data
           ->Down) // In theory this should already be encoded as (DownDuration
                   // < 0.0f), but testing this facilitates eating mechanism
                   // (until we finish work on key ownership)
    return 0;
  const float t = key_data->DownDuration;
  return CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, repeat_delay,
                                   repeat_rate);
}

// Return 2D vector representing the combination of four cardinal direction,
// with analog value support (for e.g. Key_GamepadLStick* values).
Vec2 Gui::GetKeyMagnitude2d(Key key_left, Key key_right, Key key_up,
                            Key key_down) {
  return Vec2(
      GetKeyData(key_right)->AnalogValue - GetKeyData(key_left)->AnalogValue,
      GetKeyData(key_down)->AnalogValue - GetKeyData(key_up)->AnalogValue);
}

// Rewrite routing data buffers to strip old entries + sort by key to make
// queries not touch scattered data.
//   Entries   D,A,B,B,A,C,B     --> A,A,B,B,B,C,D
//   Index     A:1 B:2 C:5 D:0   --> A:0 B:2 C:5 D:6
// See 'Metrics->Key Owners & Shortcut Routing' to visualize the result of that
// operation.
static void Gui::UpdateKeyRoutingTable(KeyRoutingTable *rt) {
  Context &g = *GGui;
  rt->EntriesNext.resize(0);
  for (Key key = Key_NamedKey_BEGIN; key < Key_NamedKey_END;
       key = (Key)(key + 1)) {
    const int new_routing_start_idx = rt->EntriesNext.Size;
    KeyRoutingData *routing_entry;
    for (int old_routing_idx = rt->Index[key - Key_NamedKey_BEGIN];
         old_routing_idx != -1;
         old_routing_idx = routing_entry->NextEntryIndex) {
      routing_entry = &rt->Entries[old_routing_idx];
      routing_entry->RoutingCurr = routing_entry->RoutingNext; // Update entry
      routing_entry->RoutingNext = KeyOwner_None;
      routing_entry->RoutingNextScore = 255;
      if (routing_entry->RoutingCurr == KeyOwner_None)
        continue;
      rt->EntriesNext.push_back(
          *routing_entry); // Write alive ones into new buffer

      // Apply routing to owner if there's no owner already (RoutingCurr == None
      // at this point)
      if (routing_entry->Mods == g.IO.KeyMods) {
        KeyOwnerData *owner_data = GetKeyOwnerData(&g, key);
        if (owner_data->OwnerCurr == KeyOwner_None)
          owner_data->OwnerCurr = routing_entry->RoutingCurr;
      }
    }

    // Rewrite linked-list
    rt->Index[key - Key_NamedKey_BEGIN] =
        (KeyRoutingIndex)(new_routing_start_idx < rt->EntriesNext.Size
                              ? new_routing_start_idx
                              : -1);
    for (int n = new_routing_start_idx; n < rt->EntriesNext.Size; n++)
      rt->EntriesNext[n].NextEntryIndex =
          (KeyRoutingIndex)((n + 1 < rt->EntriesNext.Size) ? n + 1 : -1);
  }
  rt->Entries.swap(rt->EntriesNext); // Swap new and old indexes
}

// owner_id may be None/Any, but routing_id needs to be always be set, so we
// default to GetCurrentFocusScope().
static inline ID GetRoutingIdFromOwnerId(ID owner_id) {
  Context &g = *GGui;
  return (owner_id != KeyOwner_None && owner_id != KeyOwner_Any)
             ? owner_id
             : g.CurrentFocusScopeId;
}

KeyRoutingData *Gui::GetShortcutRoutingData(KeyChord key_chord) {
  // Majority of shortcuts will be Key + any number of Mods
  // We accept _Single_ mod with Key_None.
  //  - Shortcut(Key_S | Mod_Ctrl);                    // Legal
  //  - Shortcut(Key_S | Mod_Ctrl | Mod_Shift);   // Legal
  //  - Shortcut(Mod_Ctrl);                                 // Legal
  //  - Shortcut(Mod_Ctrl | Mod_Shift);                // Not legal
  Context &g = *GGui;
  KeyRoutingTable *rt = &g.KeysRoutingTable;
  KeyRoutingData *routing_data;
  if (key_chord & Mod_Shortcut)
    key_chord = ConvertShortcutMod(key_chord);
  Key key = (Key)(key_chord & ~Mod_Mask_);
  Key mods = (Key)(key_chord & Mod_Mask_);
  if (key == Key_None)
    key = ConvertSingleModFlagToKey(&g, mods);
  ASSERT(IsNamedKey(key));

  // Get (in the majority of case, the linked list will have one element so this
  // should be 2 reads. Subsequent elements will be contiguous in memory as list
  // is sorted/rebuilt in NewFrame).
  for (KeyRoutingIndex idx = rt->Index[key - Key_NamedKey_BEGIN]; idx != -1;
       idx = routing_data->NextEntryIndex) {
    routing_data = &rt->Entries[idx];
    if (routing_data->Mods == mods)
      return routing_data;
  }

  // Add to linked-list
  KeyRoutingIndex routing_data_idx = (KeyRoutingIndex)rt->Entries.Size;
  rt->Entries.push_back(KeyRoutingData());
  routing_data = &rt->Entries[routing_data_idx];
  routing_data->Mods = (U16)mods;
  routing_data->NextEntryIndex =
      rt->Index[key - Key_NamedKey_BEGIN]; // Setup linked list
  rt->Index[key - Key_NamedKey_BEGIN] = routing_data_idx;
  return routing_data;
}

// Current score encoding (lower is highest priority):
//  -   0: InputFlags_RouteGlobalHigh
//  -   1: InputFlags_RouteFocused (if item active)
//  -   2: InputFlags_RouteGlobal
//  -  3+: InputFlags_RouteFocused (if window in focus-stack)
//  - 254: InputFlags_RouteGlobalLow
//  - 255: never route
// 'flags' should include an explicit routing policy
static int CalcRoutingScore(Window *location, ID owner_id, InputFlags flags) {
  if (flags & InputFlags_RouteFocused) {
    Context &g = *GGui;
    Window *focused = g.NavWindow;

    // ActiveID gets top priority
    // (we don't check g.ActiveIdUsingAllKeys here. Routing is applied but if
    // input ownership is tested later it may discard it)
    if (owner_id != 0 && g.ActiveId == owner_id)
      return 1;

    // Early out when not in focus stack
    if (focused == NULL || focused->RootWindow != location->RootWindow)
      return 255;

    // Score based on distance to focused window (lower is better)
    // Assuming both windows are submitting a routing request,
    // - When Window....... is focused -> Window scores 3 (best), Window/ChildB
    // scores 255 (no match)
    // - When Window/ChildB is focused -> Window scores 4,        Window/ChildB
    // scores 3 (best) Assuming only WindowA is submitting a routing request,
    // - When Window/ChildB is focused -> Window scores 4 (best), Window/ChildB
    // doesn't have a score.
    for (int next_score = 3; focused != NULL; next_score++) {
      if (focused == location) {
        ASSERT(next_score < 255);
        return next_score;
      }
      focused =
          (focused->RootWindow != focused)
              ? focused->ParentWindow
              : NULL; // FIXME: This could be later abstracted as a focus path
    }
    return 255;
  }

  // InputFlags_RouteGlobalHigh is default, so calls without flags are not
  // conditional
  if (flags & InputFlags_RouteGlobal)
    return 2;
  if (flags & InputFlags_RouteGlobalLow)
    return 254;
  return 0;
}

// Request a desired route for an input chord (key + mods).
// Return true if the route is available this frame.
// - Routes and key ownership are attributed at the beginning of next frame
// based on best score and mod state.
//   (Conceptually this does a "Submit for next frame" + "Test for current
//   frame". As such, it could be called TrySetXXX or SubmitXXX, or the Submit
//   and Test operations should be separate.)
// - Using 'owner_id == KeyOwner_Any/0': auto-assign an owner based on
// current focus scope (each window has its focus scope by default)
// - Using 'owner_id == KeyOwner_None': allows disabling/locking a
// shortcut.
bool Gui::SetShortcutRouting(KeyChord key_chord, ID owner_id,
                             InputFlags flags) {
  Context &g = *GGui;
  if ((flags & InputFlags_RouteMask_) == 0)
    flags |= InputFlags_RouteGlobalHigh; // IMPORTANT: This is the default
                                         // for SetShortcutRouting() but
                                         // NOT Shortcut()
  else
    ASSERT(IsPowerOfTwo(
        flags &
        InputFlags_RouteMask_)); // Check that only 1 routing flag is used

  if (flags & InputFlags_RouteUnlessBgFocused)
    if (g.NavWindow == NULL)
      return false;
  if (flags & InputFlags_RouteAlways)
    return true;

  const int score = CalcRoutingScore(g.CurrentWindow, owner_id, flags);
  if (score == 255)
    return false;

  // Submit routing for NEXT frame (assuming score is sufficient)
  // FIXME: Could expose a way to use a "serve last" policy for same score
  // resolution (using <= instead of <).
  KeyRoutingData *routing_data = GetShortcutRoutingData(key_chord);
  const ID routing_id = GetRoutingIdFromOwnerId(owner_id);
  // const bool set_route = (flags & InputFlags_ServeLast) ? (score <=
  // routing_data->RoutingNextScore) : (score < routing_data->RoutingNextScore);
  if (score < routing_data->RoutingNextScore) {
    routing_data->RoutingNext = routing_id;
    routing_data->RoutingNextScore = (U8)score;
  }

  // Return routing state for CURRENT frame
  return routing_data->RoutingCurr == routing_id;
}

// Currently unused by core (but used by tests)
// Note: this cannot be turned into GetShortcutRouting() because we do the
// owner_id->routing_id translation, name would be more misleading.
bool Gui::TestShortcutRouting(KeyChord key_chord, ID owner_id) {
  const ID routing_id = GetRoutingIdFromOwnerId(owner_id);
  KeyRoutingData *routing_data =
      GetShortcutRoutingData(key_chord); // FIXME: Could avoid creating entry.
  return routing_data->RoutingCurr == routing_id;
}

// Note that Gui doesn't know the meaning/semantic of Key from
// 0..511: they are legacy native keycodes. Consider transitioning from
// 'IsKeyDown(MY_ENGINE_KEY_A)' (<1.87) to IsKeyDown(Key_A) (>= 1.87)
bool Gui::IsKeyDown(Key key) { return IsKeyDown(key, KeyOwner_Any); }

bool Gui::IsKeyDown(Key key, ID owner_id) {
  const KeyData *key_data = GetKeyData(key);
  if (!key_data->Down)
    return false;
  if (!TestKeyOwner(key, owner_id))
    return false;
  return true;
}

bool Gui::IsKeyPressed(Key key, bool repeat) {
  return IsKeyPressed(key, KeyOwner_Any,
                      repeat ? InputFlags_Repeat : InputFlags_None);
}

// Important: unless legacy IsKeyPressed(Key, bool repeat=true) which
// DEFAULT to repeat, this requires EXPLICIT repeat.
bool Gui::IsKeyPressed(Key key, ID owner_id, InputFlags flags) {
  const KeyData *key_data = GetKeyData(key);
  if (!key_data
           ->Down) // In theory this should already be encoded as (DownDuration
                   // < 0.0f), but testing this facilitates eating mechanism
                   // (until we finish work on key ownership)
    return false;
  const float t = key_data->DownDuration;
  if (t < 0.0f)
    return false;
  ASSERT((flags & ~InputFlags_SupportedByIsKeyPressed) ==
         0); // Passing flags not supported by this function!

  bool pressed = (t == 0.0f);
  if (!pressed && ((flags & InputFlags_Repeat) != 0)) {
    float repeat_delay, repeat_rate;
    GetTypematicRepeatRate(flags, &repeat_delay, &repeat_rate);
    pressed = (t > repeat_delay) &&
              GetKeyPressedAmount(key, repeat_delay, repeat_rate) > 0;
  }
  if (!pressed)
    return false;
  if (!TestKeyOwner(key, owner_id))
    return false;
  return true;
}

bool Gui::IsKeyReleased(Key key) { return IsKeyReleased(key, KeyOwner_Any); }

bool Gui::IsKeyReleased(Key key, ID owner_id) {
  const KeyData *key_data = GetKeyData(key);
  if (key_data->DownDurationPrev < 0.0f || key_data->Down)
    return false;
  if (!TestKeyOwner(key, owner_id))
    return false;
  return true;
}

bool Gui::IsMouseDown(MouseButton button) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  return g.IO.MouseDown[button] &&
         TestKeyOwner(MouseButtonToKey(button),
                      KeyOwner_Any); // should be same as
                                     // IsKeyDown(MouseButtonToKey(button),
                                     // KeyOwner_Any), but this allows legacy
                                     // code hijacking the io.Mousedown[] array.
}

bool Gui::IsMouseDown(MouseButton button, ID owner_id) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  return g.IO.MouseDown[button] &&
         TestKeyOwner(
             MouseButtonToKey(button),
             owner_id); // Should be same as IsKeyDown(MouseButtonToKey(button),
                        // owner_id), but this allows legacy code hijacking the
                        // io.Mousedown[] array.
}

bool Gui::IsMouseClicked(MouseButton button, bool repeat) {
  return IsMouseClicked(button, KeyOwner_Any,
                        repeat ? InputFlags_Repeat : InputFlags_None);
}

bool Gui::IsMouseClicked(MouseButton button, ID owner_id, InputFlags flags) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  if (!g.IO.MouseDown[button]) // In theory this should already be encoded as
                               // (DownDuration < 0.0f), but testing this
                               // facilitates eating mechanism (until we finish
                               // work on key ownership)
    return false;
  const float t = g.IO.MouseDownDuration[button];
  if (t < 0.0f)
    return false;
  ASSERT((flags & ~InputFlags_SupportedByIsKeyPressed) ==
         0); // Passing flags not supported by this function!

  const bool repeat = (flags & InputFlags_Repeat) != 0;
  const bool pressed =
      (t == 0.0f) ||
      (repeat && t > g.IO.KeyRepeatDelay &&
       CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, g.IO.KeyRepeatDelay,
                                 g.IO.KeyRepeatRate) > 0);
  if (!pressed)
    return false;

  if (!TestKeyOwner(MouseButtonToKey(button), owner_id))
    return false;

  return true;
}

bool Gui::IsMouseReleased(MouseButton button) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  return g.IO.MouseReleased[button] &&
         TestKeyOwner(MouseButtonToKey(button),
                      KeyOwner_Any); // Should be same as
                                     // IsKeyReleased(MouseButtonToKey(button),
                                     // KeyOwner_Any)
}

bool Gui::IsMouseReleased(MouseButton button, ID owner_id) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  return g.IO.MouseReleased[button] &&
         TestKeyOwner(
             MouseButtonToKey(button),
             owner_id); // Should be same as
                        // IsKeyReleased(MouseButtonToKey(button), owner_id)
}

bool Gui::IsMouseDoubleClicked(MouseButton button) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  return g.IO.MouseClickedCount[button] == 2 &&
         TestKeyOwner(MouseButtonToKey(button), KeyOwner_Any);
}

bool Gui::IsMouseDoubleClicked(MouseButton button, ID owner_id) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  return g.IO.MouseClickedCount[button] == 2 &&
         TestKeyOwner(MouseButtonToKey(button), owner_id);
}

int Gui::GetMouseClickedCount(MouseButton button) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  return g.IO.MouseClickedCount[button];
}

// Test if mouse cursor is hovering given rectangle
// NB- Rectangle is clipped by our current clip setting
// NB- Expand the rectangle to be generous on imprecise inputs systems
// (g.Style.TouchExtraPadding)
bool Gui::IsMouseHoveringRect(const Vec2 &r_min, const Vec2 &r_max, bool clip) {
  Context &g = *GGui;

  // Clip
  Rect rect_clipped(r_min, r_max);
  if (clip)
    rect_clipped.ClipWith(g.CurrentWindow->ClipRect);

  // Hit testing, expanded for touch input
  if (!rect_clipped.ContainsWithPad(g.IO.MousePos, g.Style.TouchExtraPadding))
    return false;
  if (!g.MouseViewport->GetMainRect().Overlaps(rect_clipped))
    return false;
  return true;
}

// Return if a mouse click/drag went past the given threshold. Valid to call
// during the MouseReleased frame. [Internal] This doesn't test if the button is
// pressed
bool Gui::IsMouseDragPastThreshold(MouseButton button, float lock_threshold) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  if (lock_threshold < 0.0f)
    lock_threshold = g.IO.MouseDragThreshold;
  return g.IO.MouseDragMaxDistanceSqr[button] >=
         lock_threshold * lock_threshold;
}

bool Gui::IsMouseDragging(MouseButton button, float lock_threshold) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  if (!g.IO.MouseDown[button])
    return false;
  return IsMouseDragPastThreshold(button, lock_threshold);
}

Vec2 Gui::GetMousePos() {
  Context &g = *GGui;
  return g.IO.MousePos;
}

// This is called TeleportMousePos() and not SetMousePos() to emphasis that
// setting MousePosPrev will effectively clear mouse delta as well. It is
// expected you only call this if (io.BackendFlags &
// BackendFlags_HasSetMousePos) is set and supported by backend.
void Gui::TeleportMousePos(const Vec2 &pos) {
  Context &g = *GGui;
  g.IO.MousePos = g.IO.MousePosPrev = pos;
  g.IO.MouseDelta = Vec2(0.0f, 0.0f);
  g.IO.WantSetMousePos = true;
  // DEBUG_LOG_IO("TeleportMousePos: (%.1f,%.1f)\n", io.MousePos.x,
  // io.MousePos.y);
}

// NB: prefer to call right after BeginPopup(). At the time Selectable/MenuItem
// is activated, the popup is already closed!
Vec2 Gui::GetMousePosOnOpeningCurrentPopup() {
  Context &g = *GGui;
  if (g.BeginPopupStack.Size > 0)
    return g.OpenPopupStack[g.BeginPopupStack.Size - 1].OpenMousePos;
  return g.IO.MousePos;
}

// We typically use Vec2(-FLT_MAX,-FLT_MAX) to denote an invalid mouse
// position.
bool Gui::IsMousePosValid(const Vec2 *mouse_pos) {
  // The assert is only to silence a false-positive in XCode Static Analysis.
  // Because GGui is not dereferenced in every code path, the static analyzer
  // assume that it may be NULL (which it doesn't for other functions).
  ASSERT(GGui != NULL);
  const float MOUSE_INVALID = -256000.0f;
  Vec2 p = mouse_pos ? *mouse_pos : GGui->IO.MousePos;
  return p.x >= MOUSE_INVALID && p.y >= MOUSE_INVALID;
}

// [WILL OBSOLETE] This was designed for backends, but prefer having backend
// maintain a mask of held mouse buttons, because upcoming input queue system
// will make this invalid.
bool Gui::IsAnyMouseDown() {
  Context &g = *GGui;
  for (int n = 0; n < ARRAYSIZE(g.IO.MouseDown); n++)
    if (g.IO.MouseDown[n])
      return true;
  return false;
}

// Return the delta from the initial clicking position while the mouse button is
// clicked or was just released. This is locked and return 0.0f until the mouse
// moves past a distance threshold at least once. NB: This is only valid if
// IsMousePosValid(). backends in theory should always keep mouse position valid
// when dragging even outside the client window.
Vec2 Gui::GetMouseDragDelta(MouseButton button, float lock_threshold) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  if (lock_threshold < 0.0f)
    lock_threshold = g.IO.MouseDragThreshold;
  if (g.IO.MouseDown[button] || g.IO.MouseReleased[button])
    if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
      if (IsMousePosValid(&g.IO.MousePos) &&
          IsMousePosValid(&g.IO.MouseClickedPos[button]))
        return g.IO.MousePos - g.IO.MouseClickedPos[button];
  return Vec2(0.0f, 0.0f);
}

void Gui::ResetMouseDragDelta(MouseButton button) {
  Context &g = *GGui;
  ASSERT(button >= 0 && button < ARRAYSIZE(g.IO.MouseDown));
  // NB: We don't need to reset g.IO.MouseDragMaxDistanceSqr
  g.IO.MouseClickedPos[button] = g.IO.MousePos;
}

// Get desired mouse cursor shape.
// Important: this is meant to be used by a platform backend, it is reset in
// Gui::NewFrame(), updated during the frame, and locked in
// EndFrame()/Render(). If you use software rendering by setting
// io.MouseDrawCursor then Gui will render those for you
MouseCursor Gui::GetMouseCursor() {
  Context &g = *GGui;
  return g.MouseCursor;
}

void Gui::SetMouseCursor(MouseCursor cursor_type) {
  Context &g = *GGui;
  g.MouseCursor = cursor_type;
}

static void UpdateAliasKey(Key key, bool v, float analog_value) {
  ASSERT(Gui::IsAliasKey(key));
  KeyData *key_data = Gui::GetKeyData(key);
  key_data->Down = v;
  key_data->AnalogValue = analog_value;
}

// [Internal] Do not use directly
static KeyChord GetMergedModsFromKeys() {
  KeyChord mods = 0;
  if (Gui::IsKeyDown(Mod_Ctrl)) {
    mods |= Mod_Ctrl;
  }
  if (Gui::IsKeyDown(Mod_Shift)) {
    mods |= Mod_Shift;
  }
  if (Gui::IsKeyDown(Mod_Alt)) {
    mods |= Mod_Alt;
  }
  if (Gui::IsKeyDown(Mod_Super)) {
    mods |= Mod_Super;
  }
  return mods;
}

static void Gui::UpdateKeyboardInputs() {
  Context &g = *GGui;
  IO &io = g.IO;

  // Import legacy keys or verify they are not used
#ifndef DISABLE_OBSOLETE_KEYIO
  if (io.BackendUsingLegacyKeyArrays == 0) {
    // Backend used new io.AddKeyEvent() API: Good! Verify that old arrays are
    // never written to externally.
    for (int n = 0; n < Key_LegacyNativeKey_END; n++)
      ASSERT((io.KeysDown[n] == false || IsKeyDown((Key)n)) &&
             "Backend needs to either only use io.AddKeyEvent(), either "
             "only fill legacy io.KeysDown[] + io.KeyMap[]. Not both!");
  } else {
    if (g.FrameCount == 0)
      for (int n = Key_LegacyNativeKey_BEGIN; n < Key_LegacyNativeKey_END; n++)
        ASSERT(g.IO.KeyMap[n] == -1 &&
               "Backend is not allowed to write to io.KeyMap[0..511]!");

    // Build reverse KeyMap (Named -> Legacy)
    for (int n = Key_NamedKey_BEGIN; n < Key_NamedKey_END; n++)
      if (io.KeyMap[n] != -1) {
        ASSERT(IsLegacyKey((Key)io.KeyMap[n]));
        io.KeyMap[io.KeyMap[n]] = n;
      }

    // Import legacy keys into new ones
    for (int n = Key_LegacyNativeKey_BEGIN; n < Key_LegacyNativeKey_END; n++)
      if (io.KeysDown[n] || io.BackendUsingLegacyKeyArrays == 1) {
        const Key key = (Key)(io.KeyMap[n] != -1 ? io.KeyMap[n] : n);
        ASSERT(io.KeyMap[n] == -1 || IsNamedKey(key));
        io.KeysData[key].Down = io.KeysDown[n];
        if (key != n)
          io.KeysDown[key] =
              io.KeysDown[n]; // Allow legacy code using
                              // io.KeysDown[GetKeyIndex()] with old backends
        io.BackendUsingLegacyKeyArrays = 1;
      }
    if (io.BackendUsingLegacyKeyArrays == 1) {
      GetKeyData(Mod_Ctrl)->Down = io.KeyCtrl;
      GetKeyData(Mod_Shift)->Down = io.KeyShift;
      GetKeyData(Mod_Alt)->Down = io.KeyAlt;
      GetKeyData(Mod_Super)->Down = io.KeySuper;
    }
  }

#ifndef DISABLE_OBSOLETE_KEYIO
  const bool nav_gamepad_active =
      (io.ConfigFlags & ConfigFlags_NavEnableGamepad) != 0 &&
      (io.BackendFlags & BackendFlags_HasGamepad) != 0;
  if (io.BackendUsingLegacyNavInputArray && nav_gamepad_active) {
#define MAP_LEGACY_NAV_INPUT_TO_KEY1(_KEY, _NAV1)                              \
  do {                                                                         \
    io.KeysData[_KEY].Down = (io.NavInputs[_NAV1] > 0.0f);                     \
    io.KeysData[_KEY].AnalogValue = io.NavInputs[_NAV1];                       \
  } while (0)
#define MAP_LEGACY_NAV_INPUT_TO_KEY2(_KEY, _NAV1, _NAV2)                       \
  do {                                                                         \
    io.KeysData[_KEY].Down =                                                   \
        (io.NavInputs[_NAV1] > 0.0f) || (io.NavInputs[_NAV2] > 0.0f);          \
    io.KeysData[_KEY].AnalogValue =                                            \
        Max(io.NavInputs[_NAV1], io.NavInputs[_NAV2]);                         \
  } while (0)
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadFaceDown, NavInput_Activate);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadFaceRight, NavInput_Cancel);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadFaceLeft, NavInput_Menu);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadFaceUp, NavInput_Input);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadDpadLeft, NavInput_DpadLeft);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadDpadRight, NavInput_DpadRight);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadDpadUp, NavInput_DpadUp);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadDpadDown, NavInput_DpadDown);
    MAP_LEGACY_NAV_INPUT_TO_KEY2(Key_GamepadL1, NavInput_FocusPrev,
                                 NavInput_TweakSlow);
    MAP_LEGACY_NAV_INPUT_TO_KEY2(Key_GamepadR1, NavInput_FocusNext,
                                 NavInput_TweakFast);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadLStickLeft, NavInput_LStickLeft);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadLStickRight, NavInput_LStickRight);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadLStickUp, NavInput_LStickUp);
    MAP_LEGACY_NAV_INPUT_TO_KEY1(Key_GamepadLStickDown, NavInput_LStickDown);
#undef NAV_MAP_KEY
  }
#endif
#endif

  // Update aliases
  for (int n = 0; n < MouseButton_COUNT; n++)
    UpdateAliasKey(MouseButtonToKey(n), io.MouseDown[n],
                   io.MouseDown[n] ? 1.0f : 0.0f);
  UpdateAliasKey(Key_MouseWheelX, io.MouseWheelH != 0.0f, io.MouseWheelH);
  UpdateAliasKey(Key_MouseWheelY, io.MouseWheel != 0.0f, io.MouseWheel);

  // Synchronize io.KeyMods and io.KeyXXX values.
  // - New backends (1.87+): send io.AddKeyEvent(Mod_XXX) -> -> (here)
  // deriving io.KeyMods + io.KeyXXX from key array.
  // - Legacy backends:      set io.KeyXXX bools               -> (above) set
  // key array from io.KeyXXX -> (here) deriving io.KeyMods + io.KeyXXX from key
  // array. So with legacy backends the 4 values will do a unnecessary
  // back-and-forth but it makes the code simpler and future facing.
  io.KeyMods = GetMergedModsFromKeys();
  io.KeyCtrl = (io.KeyMods & Mod_Ctrl) != 0;
  io.KeyShift = (io.KeyMods & Mod_Shift) != 0;
  io.KeyAlt = (io.KeyMods & Mod_Alt) != 0;
  io.KeySuper = (io.KeyMods & Mod_Super) != 0;

  // Clear gamepad data if disabled
  if ((io.BackendFlags & BackendFlags_HasGamepad) == 0)
    for (int i = Key_Gamepad_BEGIN; i < Key_Gamepad_END; i++) {
      io.KeysData[i - Key_KeysData_OFFSET].Down = false;
      io.KeysData[i - Key_KeysData_OFFSET].AnalogValue = 0.0f;
    }

  // Update keys
  for (int i = 0; i < Key_KeysData_SIZE; i++) {
    KeyData *key_data = &io.KeysData[i];
    key_data->DownDurationPrev = key_data->DownDuration;
    key_data->DownDuration = key_data->Down
                                 ? (key_data->DownDuration < 0.0f
                                        ? 0.0f
                                        : key_data->DownDuration + io.DeltaTime)
                                 : -1.0f;
  }

  // Update keys/input owner (named keys only): one entry per key
  for (Key key = Key_NamedKey_BEGIN; key < Key_NamedKey_END;
       key = (Key)(key + 1)) {
    KeyData *key_data = &io.KeysData[key - Key_KeysData_OFFSET];
    KeyOwnerData *owner_data = &g.KeysOwnerData[key - Key_NamedKey_BEGIN];
    owner_data->OwnerCurr = owner_data->OwnerNext;
    if (!key_data
             ->Down) // Important: ownership is released on the frame after a
                     // release. Ensure a 'MouseDown -> CloseWindow -> MouseUp'
                     // chain doesn't lead to someone else seeing the MouseUp.
      owner_data->OwnerNext = KeyOwner_None;
    owner_data->LockThisFrame = owner_data->LockUntilRelease =
        owner_data->LockUntilRelease &&
        key_data->Down; // Clear LockUntilRelease when key is not Down anymore
  }

  UpdateKeyRoutingTable(&g.KeysRoutingTable);
}

static void Gui::UpdateMouseInputs() {
  Context &g = *GGui;
  IO &io = g.IO;

  // Mouse Wheel swapping flag
  // As a standard behavior holding SHIFT while using Vertical Mouse Wheel
  // triggers Horizontal scroll instead
  // - We avoid doing it on OSX as it the OS input layer handles this already.
  // - FIXME: However this means when running on OSX over Emscripten,
  // Shift+WheelY will incur two swapping (1 in OS, 1 here), canceling the
  // feature.
  // - FIXME: When we can distinguish e.g. touchpad scroll events from mouse
  // ones, we'll set this accordingly based on input source.
  io.MouseWheelRequestAxisSwap = io.KeyShift && !io.ConfigMacOSXBehaviors;

  // Round mouse position to avoid spreading non-rounded position (e.g.
  // UpdateManualResize doesn't support them well)
  if (IsMousePosValid(&io.MousePos))
    io.MousePos = g.MouseLastValidPos = Floor(io.MousePos);

  // If mouse just appeared or disappeared (usually denoted by -FLT_MAX
  // components) we cancel out movement in MouseDelta
  if (IsMousePosValid(&io.MousePos) && IsMousePosValid(&io.MousePosPrev))
    io.MouseDelta = io.MousePos - io.MousePosPrev;
  else
    io.MouseDelta = Vec2(0.0f, 0.0f);

  // Update stationary timer.
  // FIXME: May need to rework again to have some tolerance for occasional small
  // movement, while being functional on high-framerates.
  const float mouse_stationary_threshold =
      (io.MouseSource == MouseSource_Mouse)
          ? 2.0f
          : 3.0f; // Slightly higher threshold for
                  // MouseSource_TouchScreen/MouseSource_Pen, may need
                  // rework.
  const bool mouse_stationary =
      (LengthSqr(io.MouseDelta) <=
       mouse_stationary_threshold * mouse_stationary_threshold);
  g.MouseStationaryTimer =
      mouse_stationary ? (g.MouseStationaryTimer + io.DeltaTime) : 0.0f;
  // DEBUG_LOG("%.4f\n", g.MouseStationaryTimer);

  // If mouse moved we re-enable mouse hovering in case it was disabled by
  // gamepad/keyboard. In theory should use a >0.0f threshold but would need to
  // reset in everywhere we set this to true.
  if (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)
    g.NavDisableMouseHover = false;

  for (int i = 0; i < ARRAYSIZE(io.MouseDown); i++) {
    io.MouseClicked[i] = io.MouseDown[i] && io.MouseDownDuration[i] < 0.0f;
    io.MouseClickedCount[i] = 0; // Will be filled below
    io.MouseReleased[i] = !io.MouseDown[i] && io.MouseDownDuration[i] >= 0.0f;
    io.MouseDownDurationPrev[i] = io.MouseDownDuration[i];
    io.MouseDownDuration[i] =
        io.MouseDown[i] ? (io.MouseDownDuration[i] < 0.0f
                               ? 0.0f
                               : io.MouseDownDuration[i] + io.DeltaTime)
                        : -1.0f;
    if (io.MouseClicked[i]) {
      bool is_repeated_click = false;
      if ((float)(g.Time - io.MouseClickedTime[i]) < io.MouseDoubleClickTime) {
        Vec2 delta_from_click_pos = IsMousePosValid(&io.MousePos)
                                        ? (io.MousePos - io.MouseClickedPos[i])
                                        : Vec2(0.0f, 0.0f);
        if (LengthSqr(delta_from_click_pos) <
            io.MouseDoubleClickMaxDist * io.MouseDoubleClickMaxDist)
          is_repeated_click = true;
      }
      if (is_repeated_click)
        io.MouseClickedLastCount[i]++;
      else
        io.MouseClickedLastCount[i] = 1;
      io.MouseClickedTime[i] = g.Time;
      io.MouseClickedPos[i] = io.MousePos;
      io.MouseClickedCount[i] = io.MouseClickedLastCount[i];
      io.MouseDragMaxDistanceAbs[i] = Vec2(0.0f, 0.0f);
      io.MouseDragMaxDistanceSqr[i] = 0.0f;
    } else if (io.MouseDown[i]) {
      // Maintain the maximum distance we reaching from the initial click
      // position, which is used with dragging threshold
      Vec2 delta_from_click_pos = IsMousePosValid(&io.MousePos)
                                      ? (io.MousePos - io.MouseClickedPos[i])
                                      : Vec2(0.0f, 0.0f);
      io.MouseDragMaxDistanceSqr[i] =
          Max(io.MouseDragMaxDistanceSqr[i], LengthSqr(delta_from_click_pos));
      io.MouseDragMaxDistanceAbs[i].x =
          Max(io.MouseDragMaxDistanceAbs[i].x, delta_from_click_pos.x < 0.0f
                                                   ? -delta_from_click_pos.x
                                                   : delta_from_click_pos.x);
      io.MouseDragMaxDistanceAbs[i].y =
          Max(io.MouseDragMaxDistanceAbs[i].y, delta_from_click_pos.y < 0.0f
                                                   ? -delta_from_click_pos.y
                                                   : delta_from_click_pos.y);
    }

    // We provide io.MouseDoubleClicked[] as a legacy service
    io.MouseDoubleClicked[i] = (io.MouseClickedCount[i] == 2);

    // Clicking any mouse button reactivate mouse hovering which may have been
    // deactivated by gamepad/keyboard navigation
    if (io.MouseClicked[i])
      g.NavDisableMouseHover = false;
  }
}

static void LockWheelingWindow(Window *window, float wheel_amount) {
  Context &g = *GGui;
  if (window)
    g.WheelingWindowReleaseTimer =
        Min(g.WheelingWindowReleaseTimer +
                Abs(wheel_amount) * WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER,
            WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER);
  else
    g.WheelingWindowReleaseTimer = 0.0f;
  if (g.WheelingWindow == window)
    return;
  DEBUG_LOG_IO("[io] LockWheelingWindow() \"%s\"\n",
               window ? window->Name : "NULL");
  g.WheelingWindow = window;
  g.WheelingWindowRefMousePos = g.IO.MousePos;
  if (window == NULL) {
    g.WheelingWindowStartFrame = -1;
    g.WheelingAxisAvg = Vec2(0.0f, 0.0f);
  }
}

static Window *FindBestWheelingWindow(const Vec2 &wheel) {
  // For each axis, find window in the hierarchy that may want to use scrolling
  Context &g = *GGui;
  Window *windows[2] = {NULL, NULL};
  for (int axis = 0; axis < 2; axis++)
    if (wheel[axis] != 0.0f)
      for (Window *window = windows[axis] = g.HoveredWindow;
           window->Flags & WindowFlags_ChildWindow;
           window = windows[axis] = window->ParentWindow) {
        // Bubble up into parent window if:
        // - a child window doesn't allow any scrolling.
        // - a child window has the WindowFlags_NoScrollWithMouse flag.
        //// - a child window doesn't need scrolling because it is already at
        /// the edge for the direction we are going in (FIXME-WIP)
        const bool has_scrolling = (window->ScrollMax[axis] != 0.0f);
        const bool inputs_disabled =
            (window->Flags & WindowFlags_NoScrollWithMouse) &&
            !(window->Flags & WindowFlags_NoMouseInputs);
        // const bool scrolling_past_limits = (wheel_v < 0.0f) ?
        // (window->Scroll[axis] <= 0.0f) : (window->Scroll[axis] >=
        // window->ScrollMax[axis]);
        if (has_scrolling && !inputs_disabled) // && !scrolling_past_limits)
          break;                               // select this window
      }
  if (windows[0] == NULL && windows[1] == NULL)
    return NULL;

  // If there's only one window or only one axis then there's no ambiguity
  if (windows[0] == windows[1] || windows[0] == NULL || windows[1] == NULL)
    return windows[1] ? windows[1] : windows[0];

  // If candidate are different windows we need to decide which one to
  // prioritize
  // - First frame: only find a winner if one axis is zero.
  // - Subsequent frames: only find a winner when one is more than the other.
  if (g.WheelingWindowStartFrame == -1)
    g.WheelingWindowStartFrame = g.FrameCount;
  if ((g.WheelingWindowStartFrame == g.FrameCount && wheel.x != 0.0f &&
       wheel.y != 0.0f) ||
      (g.WheelingAxisAvg.x == g.WheelingAxisAvg.y)) {
    g.WheelingWindowWheelRemainder = wheel;
    return NULL;
  }
  return (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? windows[0] : windows[1];
}

// Called by NewFrame()
void Gui::UpdateMouseWheel() {
  // Reset the locked window if we move the mouse or after the timer elapses.
  // FIXME: Ideally we could refactor to have one timer for "changing window w/
  // same axis" and a shorter timer for "changing window or axis w/ other axis"
  // (#3795)
  Context &g = *GGui;
  if (g.WheelingWindow != NULL) {
    g.WheelingWindowReleaseTimer -= g.IO.DeltaTime;
    if (IsMousePosValid() &&
        LengthSqr(g.IO.MousePos - g.WheelingWindowRefMousePos) >
            g.IO.MouseDragThreshold * g.IO.MouseDragThreshold)
      g.WheelingWindowReleaseTimer = 0.0f;
    if (g.WheelingWindowReleaseTimer <= 0.0f)
      LockWheelingWindow(NULL, 0.0f);
  }

  Vec2 wheel;
  wheel.x =
      TestKeyOwner(Key_MouseWheelX, KeyOwner_None) ? g.IO.MouseWheelH : 0.0f;
  wheel.y =
      TestKeyOwner(Key_MouseWheelY, KeyOwner_None) ? g.IO.MouseWheel : 0.0f;

  // DEBUG_LOG("MouseWheel X:%.3f Y:%.3f\n", wheel_x, wheel_y);
  Window *mouse_window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;
  if (!mouse_window || mouse_window->Collapsed)
    return;

  // Zoom / Scale window
  // FIXME-OBSOLETE: This is an old feature, it still works but pretty much
  // nobody is using it and may be best redesigned.
  if (wheel.y != 0.0f && g.IO.KeyCtrl && g.IO.FontAllowUserScaling) {
    LockWheelingWindow(mouse_window, wheel.y);
    Window *window = mouse_window;
    const float new_font_scale =
        Clamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
    const float scale = new_font_scale / window->FontWindowScale;
    window->FontWindowScale = new_font_scale;
    if (window == window->RootWindow) {
      const Vec2 offset = window->Size * (1.0f - scale) *
                          (g.IO.MousePos - window->Pos) / window->Size;
      SetWindowPos(window, window->Pos + offset, 0);
      window->Size = Trunc(window->Size * scale);
      window->SizeFull = Trunc(window->SizeFull * scale);
    }
    return;
  }
  if (g.IO.KeyCtrl)
    return;

  // Mouse wheel scrolling
  // Read about io.MouseWheelRequestAxisSwap and its issue on Mac+Emscripten in
  // UpdateMouseInputs()
  if (g.IO.MouseWheelRequestAxisSwap)
    wheel = Vec2(wheel.y, 0.0f);

  // Maintain a rough average of moving magnitude on both axises
  // FIXME: should by based on wall clock time rather than frame-counter
  g.WheelingAxisAvg.x =
      ExponentialMovingAverage(g.WheelingAxisAvg.x, Abs(wheel.x), 30);
  g.WheelingAxisAvg.y =
      ExponentialMovingAverage(g.WheelingAxisAvg.y, Abs(wheel.y), 30);

  // In the rare situation where FindBestWheelingWindow() had to defer first
  // frame of wheeling due to ambiguous main axis, reinject it now.
  wheel += g.WheelingWindowWheelRemainder;
  g.WheelingWindowWheelRemainder = Vec2(0.0f, 0.0f);
  if (wheel.x == 0.0f && wheel.y == 0.0f)
    return;

  // Mouse wheel scrolling: find target and apply
  // - don't renew lock if axis doesn't apply on the window.
  // - select a main axis when both axises are being moved.
  if (Window *window =
          (g.WheelingWindow ? g.WheelingWindow : FindBestWheelingWindow(wheel)))
    if (!(window->Flags & WindowFlags_NoScrollWithMouse) &&
        !(window->Flags & WindowFlags_NoMouseInputs)) {
      bool do_scroll[2] = {wheel.x != 0.0f && window->ScrollMax.x != 0.0f,
                           wheel.y != 0.0f && window->ScrollMax.y != 0.0f};
      if (do_scroll[Axis_X] && do_scroll[Axis_Y])
        do_scroll[(g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? Axis_Y
                                                              : Axis_X] = false;
      if (do_scroll[Axis_X]) {
        LockWheelingWindow(window, wheel.x);
        float max_step = window->InnerRect.GetWidth() * 0.67f;
        float scroll_step = Trunc(Min(2 * window->CalcFontSize(), max_step));
        SetScrollX(window, window->Scroll.x - wheel.x * scroll_step);
        g.WheelingWindowScrolledFrame = g.FrameCount;
      }
      if (do_scroll[Axis_Y]) {
        LockWheelingWindow(window, wheel.y);
        float max_step = window->InnerRect.GetHeight() * 0.67f;
        float scroll_step = Trunc(Min(5 * window->CalcFontSize(), max_step));
        SetScrollY(window, window->Scroll.y - wheel.y * scroll_step);
        g.WheelingWindowScrolledFrame = g.FrameCount;
      }
    }
}

void Gui::SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard) {
  Context &g = *GGui;
  g.WantCaptureKeyboardNextFrame = want_capture_keyboard ? 1 : 0;
}

void Gui::SetNextFrameWantCaptureMouse(bool want_capture_mouse) {
  Context &g = *GGui;
  g.WantCaptureMouseNextFrame = want_capture_mouse ? 1 : 0;
}

#ifndef DISABLE_DEBUG_TOOLS
static const char *GetInputSourceName(InputSource source) {
  const char *input_source_names[] = {"None", "Mouse", "Keyboard", "Gamepad",
                                      "Clipboard"};
  ASSERT(ARRAYSIZE(input_source_names) == InputSource_COUNT && source >= 0 &&
         source < InputSource_COUNT);
  return input_source_names[source];
}
static const char *GetMouseSourceName(MouseSource source) {
  const char *mouse_source_names[] = {"Mouse", "TouchScreen", "Pen"};
  ASSERT(ARRAYSIZE(mouse_source_names) == MouseSource_COUNT && source >= 0 &&
         source < MouseSource_COUNT);
  return mouse_source_names[source];
}
static void DebugPrintInputEvent(const char *prefix, const InputEvent *e) {
  Context &g = *GGui;
  if (e->Type == InputEventType_MousePos) {
    if (e->MousePos.PosX == -FLT_MAX && e->MousePos.PosY == -FLT_MAX)
      DEBUG_LOG_IO("[io] %s: MousePos (-FLT_MAX, -FLT_MAX)\n", prefix);
    else
      DEBUG_LOG_IO("[io] %s: MousePos (%.1f, %.1f) (%s)\n", prefix,
                   e->MousePos.PosX, e->MousePos.PosY,
                   GetMouseSourceName(e->MousePos.MouseSource));
    return;
  }
  if (e->Type == InputEventType_MouseButton) {
    DEBUG_LOG_IO("[io] %s: MouseButton %d %s (%s)\n", prefix,
                 e->MouseButton.Button, e->MouseButton.Down ? "Down" : "Up",
                 GetMouseSourceName(e->MouseButton.MouseSource));
    return;
  }
  if (e->Type == InputEventType_MouseWheel) {
    DEBUG_LOG_IO("[io] %s: MouseWheel (%.3f, %.3f) (%s)\n", prefix,
                 e->MouseWheel.WheelX, e->MouseWheel.WheelY,
                 GetMouseSourceName(e->MouseWheel.MouseSource));
    return;
  }
  if (e->Type == InputEventType_MouseViewport) {
    DEBUG_LOG_IO("[io] %s: MouseViewport (0x%08X)\n", prefix,
                 e->MouseViewport.HoveredViewportID);
    return;
  }
  if (e->Type == InputEventType_Key) {
    DEBUG_LOG_IO("[io] %s: Key \"%s\" %s\n", prefix,
                 Gui::GetKeyName(e->Key.Key), e->Key.Down ? "Down" : "Up");
    return;
  }
  if (e->Type == InputEventType_Text) {
    DEBUG_LOG_IO("[io] %s: Text: %c (U+%08X)\n", prefix, e->Text.Char,
                 e->Text.Char);
    return;
  }
  if (e->Type == InputEventType_Focus) {
    DEBUG_LOG_IO("[io] %s: AppFocused %d\n", prefix, e->AppFocused.Focused);
    return;
  }
}
#endif

// Process input queue
// We always call this with the value of 'bool
// g.IO.ConfigInputTrickleEventQueue'.
// - trickle_fast_inputs = false : process all events, turn into flattened input
// state (e.g. successive down/up/down/up will be lost)
// - trickle_fast_inputs = true  : process as many events as possible
// (successive down/up/down/up will be trickled over several frames so nothing
// is lost) (new feature in 1.87)
void Gui::UpdateInputEvents(bool trickle_fast_inputs) {
  Context &g = *GGui;
  IO &io = g.IO;

  // Only trickle chars<>key when working with InputText()
  // FIXME: InputText() could parse event trail?
  // FIXME: Could specialize chars<>keys trickling rules for control keys (those
  // not typically associated to characters)
  const bool trickle_interleaved_keys_and_text =
      (trickle_fast_inputs && g.WantTextInputNextFrame == 1);

  bool mouse_moved = false, mouse_wheeled = false, key_changed = false,
       text_inputted = false;
  int mouse_button_changed = 0x00;
  BitArray<Key_KeysData_SIZE> key_changed_mask;

  int event_n = 0;
  for (; event_n < g.InputEventsQueue.Size; event_n++) {
    InputEvent *e = &g.InputEventsQueue[event_n];
    if (e->Type == InputEventType_MousePos) {
      if (g.IO.WantSetMousePos)
        continue;
      // Trickling Rule: Stop processing queued events if we already handled a
      // mouse button change
      Vec2 event_pos(e->MousePos.PosX, e->MousePos.PosY);
      if (trickle_fast_inputs && (mouse_button_changed != 0 || mouse_wheeled ||
                                  key_changed || text_inputted))
        break;
      io.MousePos = event_pos;
      io.MouseSource = e->MousePos.MouseSource;
      mouse_moved = true;
    } else if (e->Type == InputEventType_MouseButton) {
      // Trickling Rule: Stop processing queued events if we got multiple action
      // on the same button
      const MouseButton button = e->MouseButton.Button;
      ASSERT(button >= 0 && button < MouseButton_COUNT);
      if (trickle_fast_inputs &&
          ((mouse_button_changed & (1 << button)) || mouse_wheeled))
        break;
      if (trickle_fast_inputs &&
          e->MouseButton.MouseSource == MouseSource_TouchScreen &&
          mouse_moved) // #2702: TouchScreen have no initial hover.
        break;
      io.MouseDown[button] = e->MouseButton.Down;
      io.MouseSource = e->MouseButton.MouseSource;
      mouse_button_changed |= (1 << button);
    } else if (e->Type == InputEventType_MouseWheel) {
      // Trickling Rule: Stop processing queued events if we got multiple action
      // on the event
      if (trickle_fast_inputs && (mouse_moved || mouse_button_changed != 0))
        break;
      io.MouseWheelH += e->MouseWheel.WheelX;
      io.MouseWheel += e->MouseWheel.WheelY;
      io.MouseSource = e->MouseWheel.MouseSource;
      mouse_wheeled = true;
    } else if (e->Type == InputEventType_MouseViewport) {
      io.MouseHoveredViewport = e->MouseViewport.HoveredViewportID;
    } else if (e->Type == InputEventType_Key) {
      // Trickling Rule: Stop processing queued events if we got multiple action
      // on the same button
      Key key = e->Key.Key;
      ASSERT(key != Key_None);
      KeyData *key_data = GetKeyData(key);
      const int key_data_index = (int)(key_data - g.IO.KeysData);
      if (trickle_fast_inputs && key_data->Down != e->Key.Down &&
          (key_changed_mask.TestBit(key_data_index) || text_inputted ||
           mouse_button_changed != 0))
        break;
      key_data->Down = e->Key.Down;
      key_data->AnalogValue = e->Key.AnalogValue;
      key_changed = true;
      key_changed_mask.SetBit(key_data_index);

      // Allow legacy code using io.KeysDown[GetKeyIndex()] with new backends
#ifndef DISABLE_OBSOLETE_KEYIO
      io.KeysDown[key_data_index] = key_data->Down;
      if (io.KeyMap[key_data_index] != -1)
        io.KeysDown[io.KeyMap[key_data_index]] = key_data->Down;
#endif
    } else if (e->Type == InputEventType_Text) {
      // Trickling Rule: Stop processing queued events if keys/mouse have been
      // interacted with
      if (trickle_fast_inputs &&
          ((key_changed && trickle_interleaved_keys_and_text) ||
           mouse_button_changed != 0 || mouse_moved || mouse_wheeled))
        break;
      unsigned int c = e->Text.Char;
      io.InputQueueCharacters.push_back(
          c <= UNICODE_CODEPOINT_MAX ? (Wchar)c : UNICODE_CODEPOINT_INVALID);
      if (trickle_interleaved_keys_and_text)
        text_inputted = true;
    } else if (e->Type == InputEventType_Focus) {
      // We intentionally overwrite this and process in NewFrame(), in order to
      // give a chance to multi-viewports backends to queue AddFocusEvent(false)
      // + AddFocusEvent(true) in same frame.
      const bool focus_lost = !e->AppFocused.Focused;
      io.AppFocusLost = focus_lost;
    } else {
      ASSERT(0 && "Unknown event!");
    }
  }

  // Record trail (for domain-specific applications wanting to access a precise
  // trail)
  // if (event_n != 0) DEBUG_LOG_IO("Processed: %d / Remaining: %d\n", event_n,
  // g.InputEventsQueue.Size - event_n);
  for (int n = 0; n < event_n; n++)
    g.InputEventsTrail.push_back(g.InputEventsQueue[n]);

    // [DEBUG]
#ifndef DISABLE_DEBUG_TOOLS
  if (event_n != 0 && (g.DebugLogFlags & DebugLogFlags_EventIO))
    for (int n = 0; n < g.InputEventsQueue.Size; n++)
      DebugPrintInputEvent(n < event_n ? "Processed" : "Remaining",
                           &g.InputEventsQueue[n]);
#endif

  // Remaining events will be processed on the next frame
  if (event_n == g.InputEventsQueue.Size)
    g.InputEventsQueue.resize(0);
  else
    g.InputEventsQueue.erase(g.InputEventsQueue.Data,
                             g.InputEventsQueue.Data + event_n);

  // Clear buttons state when focus is lost
  // - this is useful so e.g. releasing Alt after focus loss on Alt-Tab doesn't
  // trigger the Alt menu toggle.
  // - we clear in EndFrame() and not now in order allow application/user code
  // polling this flag
  //   (e.g. custom backend may want to clear additional data, custom widgets
  //   may want to react with a "canceling" event).
  if (g.IO.AppFocusLost)
    g.IO.ClearInputKeys();
}

ID Gui::GetKeyOwner(Key key) {
  if (!IsNamedKeyOrModKey(key))
    return KeyOwner_None;

  Context &g = *GGui;
  KeyOwnerData *owner_data = GetKeyOwnerData(&g, key);
  ID owner_id = owner_data->OwnerCurr;

  if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId &&
      owner_id != KeyOwner_Any)
    if (key >= Key_Keyboard_BEGIN && key < Key_Keyboard_END)
      return KeyOwner_None;

  return owner_id;
}

// TestKeyOwner(..., ID)   : (owner == None || owner == ID)
// TestKeyOwner(..., None) : (owner == None)
// TestKeyOwner(..., Any)  : no owner test
// All paths are also testing for key not being locked, for the rare cases that
// key have been locked with using InputFlags_LockXXX flags.
bool Gui::TestKeyOwner(Key key, ID owner_id) {
  if (!IsNamedKeyOrModKey(key))
    return true;

  Context &g = *GGui;
  if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId &&
      owner_id != KeyOwner_Any)
    if (key >= Key_Keyboard_BEGIN && key < Key_Keyboard_END)
      return false;

  KeyOwnerData *owner_data = GetKeyOwnerData(&g, key);
  if (owner_id == KeyOwner_Any)
    return (owner_data->LockThisFrame == false);

  // Note: SetKeyOwner() sets OwnerCurr. It is not strictly required for most
  // mouse routing overlap (because of ActiveId/HoveredId are acting as filter
  // before this has a chance to filter), but sane as soon as user tries to look
  // into things. Setting OwnerCurr in SetKeyOwner() is more consistent than
  // testing OwnerNext here: would be inconsistent with getter and other
  // functions.
  if (owner_data->OwnerCurr != owner_id) {
    if (owner_data->LockThisFrame)
      return false;
    if (owner_data->OwnerCurr != KeyOwner_None)
      return false;
  }

  return true;
}

// _LockXXX flags are useful to lock keys away from code which is not
// input-owner aware. When using _LockXXX flags, you can use KeyOwner_Any
// to lock keys from everyone.
// - SetKeyOwner(..., None)              : clears owner
// - SetKeyOwner(..., Any, !Lock)        : illegal (assert)
// - SetKeyOwner(..., Any or None, Lock) : set lock
void Gui::SetKeyOwner(Key key, ID owner_id, InputFlags flags) {
  ASSERT(IsNamedKeyOrModKey(key) &&
         (owner_id != KeyOwner_Any ||
          (flags & (InputFlags_LockThisFrame |
                    InputFlags_LockUntilRelease)))); // Can only use _Any with
                                                     // _LockXXX flags (to eat
                                                     // a key away without an
                                                     // ID to retrieve it)
  ASSERT((flags & ~InputFlags_SupportedBySetKeyOwner) ==
         0); // Passing flags not supported by this function!

  Context &g = *GGui;
  KeyOwnerData *owner_data = GetKeyOwnerData(&g, key);
  owner_data->OwnerCurr = owner_data->OwnerNext = owner_id;

  // We cannot lock by default as it would likely break lots of legacy code.
  // In the case of using LockUntilRelease while key is not down we still lock
  // during the frame (no key_data->Down test)
  owner_data->LockUntilRelease = (flags & InputFlags_LockUntilRelease) != 0;
  owner_data->LockThisFrame =
      (flags & InputFlags_LockThisFrame) != 0 || (owner_data->LockUntilRelease);
}

// Rarely used helper
void Gui::SetKeyOwnersForKeyChord(KeyChord key_chord, ID owner_id,
                                  InputFlags flags) {
  if (key_chord & Mod_Ctrl) {
    SetKeyOwner(Mod_Ctrl, owner_id, flags);
  }
  if (key_chord & Mod_Shift) {
    SetKeyOwner(Mod_Shift, owner_id, flags);
  }
  if (key_chord & Mod_Alt) {
    SetKeyOwner(Mod_Alt, owner_id, flags);
  }
  if (key_chord & Mod_Super) {
    SetKeyOwner(Mod_Super, owner_id, flags);
  }
  if (key_chord & Mod_Shortcut) {
    SetKeyOwner(Mod_Shortcut, owner_id, flags);
  }
  if (key_chord & ~Mod_Mask_) {
    SetKeyOwner((Key)(key_chord & ~Mod_Mask_), owner_id, flags);
  }
}

// This is more or less equivalent to:
//   if (IsItemHovered() || IsItemActive())
//       SetKeyOwner(key, GetItemID());
// Extensive uses of that (e.g. many calls for a single item) may want to
// manually perform the tests once and then call SetKeyOwner() multiple times.
// More advanced usage scenarios may want to call SetKeyOwner() manually based
// on different condition. Worth noting is that only one item can be hovered and
// only one item can be active, therefore this usage pattern doesn't need to
// bother with routing and priority.
void Gui::SetItemKeyOwner(Key key, InputFlags flags) {
  Context &g = *GGui;
  ID id = g.LastItemData.ID;
  if (id == 0 || (g.HoveredId != id && g.ActiveId != id))
    return;
  if ((flags & InputFlags_CondMask_) == 0)
    flags |= InputFlags_CondDefault_;
  if ((g.HoveredId == id && (flags & InputFlags_CondHovered)) ||
      (g.ActiveId == id && (flags & InputFlags_CondActive))) {
    ASSERT((flags & ~InputFlags_SupportedBySetItemKeyOwner) ==
           0); // Passing flags not supported by this function!
    SetKeyOwner(key, id, flags & ~InputFlags_CondMask_);
  }
}

// This is the only public API until we expose owner_id versions of the API as
// replacements.
bool Gui::IsKeyChordPressed(KeyChord key_chord) {
  return IsKeyChordPressed(key_chord, 0, InputFlags_None);
}

// This is equivalent to comparing KeyMods + doing a IsKeyPressed()
bool Gui::IsKeyChordPressed(KeyChord key_chord, ID owner_id, InputFlags flags) {
  Context &g = *GGui;
  if (key_chord & Mod_Shortcut)
    key_chord = ConvertShortcutMod(key_chord);
  Key mods = (Key)(key_chord & Mod_Mask_);
  if (g.IO.KeyMods != mods)
    return false;

  // Special storage location for mods
  Key key = (Key)(key_chord & ~Mod_Mask_);
  if (key == Key_None)
    key = ConvertSingleModFlagToKey(&g, mods);
  if (!IsKeyPressed(key, owner_id,
                    (flags & (InputFlags_Repeat |
                              (InputFlags)InputFlags_RepeatRateMask_))))
    return false;
  return true;
}

bool Gui::Shortcut(KeyChord key_chord, ID owner_id, InputFlags flags) {
  // When using (owner_id == 0/Any): SetShortcutRouting() will use
  // CurrentFocusScopeId and filter with this, so IsKeyPressed() is fine with he
  // 0/Any.
  if ((flags & InputFlags_RouteMask_) == 0)
    flags |= InputFlags_RouteFocused;
  if (!SetShortcutRouting(key_chord, owner_id, flags))
    return false;

  if (!IsKeyChordPressed(key_chord, owner_id, flags))
    return false;
  ASSERT((flags & ~InputFlags_SupportedByShortcut) ==
         0); // Passing flags not supported by this function!
  return true;
}

//-----------------------------------------------------------------------------
// [SECTION] ERROR CHECKING
//-----------------------------------------------------------------------------

// Helper function to verify ABI compatibility between caller code and compiled
// version of Gui. Verify that the type sizes are matching between the
// calling file's compilation unit and gui.cpp's compilation unit If this
// triggers you have an issue:
// - Most commonly: mismatched headers and compiled code version.
// - Or: mismatched configuration #define, compilation settings, packing pragma
// etc.
//   The configuration settings mentioned in config.hpp must be set for all
//   compilation units involved with Gui, which is way it is required you
//   put them in your imconfig file (and not just before including gui.hpp).
//   Otherwise it is possible that different compilation units would see
//   different structure layout
bool Gui::DebugCheckVersionAndDataLayout(const char *version, size_t sz_io,
                                         size_t sz_style, size_t sz_vec2,
                                         size_t sz_vec4, size_t sz_vert,
                                         size_t sz_idx) {
  bool error = false;
  if (strcmp(version, VERSION) != 0) {
    error = true;
    ASSERT(strcmp(version, VERSION) == 0 && "Mismatched version string!");
  }
  if (sz_io != sizeof(IO)) {
    error = true;
    ASSERT(sz_io == sizeof(IO) && "Mismatched struct layout!");
  }
  if (sz_style != sizeof(Style)) {
    error = true;
    ASSERT(sz_style == sizeof(Style) && "Mismatched struct layout!");
  }
  if (sz_vec2 != sizeof(Vec2)) {
    error = true;
    ASSERT(sz_vec2 == sizeof(Vec2) && "Mismatched struct layout!");
  }
  if (sz_vec4 != sizeof(Vec4)) {
    error = true;
    ASSERT(sz_vec4 == sizeof(Vec4) && "Mismatched struct layout!");
  }
  if (sz_vert != sizeof(DrawVert)) {
    error = true;
    ASSERT(sz_vert == sizeof(DrawVert) && "Mismatched struct layout!");
  }
  if (sz_idx != sizeof(DrawIdx)) {
    error = true;
    ASSERT(sz_idx == sizeof(DrawIdx) && "Mismatched struct layout!");
  }
  return !error;
}

// Until 1.89 (VERSION_NUM < 18814) it was legal to use SetCursorPos() to extend
// the boundary of a parent (e.g. window or table cell) This is causing issues
// and ambiguity and we need to retire that.
//  Previously this would make the window content size ~200x200:
//    Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + Vec2(200,200)) +
//    End();  // NOT OK
//  Instead, please submit an item:
//    Begin(...) + SetCursorScreenPos(GetCursorScreenPos() + Vec2(200,200)) +
//    Dummy(Vec2(0,0)) + End(); // OK
//  Alternative:
//    Begin(...) + Dummy(Vec2(200,200)) + End(); // OK
// [Scenario 2]
//  For reference this is one of the issue what we aim to fix with this change:
//    BeginGroup() + SomeItem("foobar") +
//    SetCursorScreenPos(GetCursorScreenPos()) + EndGroup()
//  The previous logic made SetCursorScreenPos(GetCursorScreenPos()) have a
//  side-effect! It would erroneously incorporate ItemSpacing.y after the item
//  into content size, making the group taller! While this code is a little
//  twisted, no-one would expect SetXXX(GetXXX()) to have a side-effect. Using
//  vertical alignment patterns could trigger this issue.
void Gui::ErrorCheckUsingSetCursorPosToExtendParentBoundaries() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT(window->DC.IsSetPos);
  window->DC.IsSetPos = false;
#ifdef DISABLE_OBSOLETE_FUNCTIONS
  if (window->DC.CursorPos.x <= window->DC.CursorMaxPos.x &&
      window->DC.CursorPos.y <= window->DC.CursorMaxPos.y)
    return;
  if (window->SkipItems)
    return;
  ASSERT(
      0 &&
      "Code uses SetCursorPos()/SetCursorScreenPos() to extend window/parent "
      "boundaries. Please submit an item e.g. Dummy() to validate extent.");
#else
  window->DC.CursorMaxPos = Max(window->DC.CursorMaxPos, window->DC.CursorPos);
#endif
}

static void Gui::ErrorCheckNewFrameSanityChecks() {
  Context &g = *GGui;

  // Check user ASSERT macro
  // (IF YOU GET A WARNING OR COMPILE ERROR HERE: it means your assert macro is
  // incorrectly defined!
  //  If your macro uses multiple statements, it NEEDS to be surrounded by a 'do
  //  { ... } while (0)' block. This is a common C/C++ idiom to allow multiple
  //  statements macros to be used in control flow blocks.)
  // #define ASSERT(EXPR)   if (SomeCode(EXPR)) SomeMoreCode(); // Wrong!
  // #define ASSERT(EXPR)   do { if (SomeCode(EXPR)) SomeMoreCode(); } while
  // (0)   // Correct!
  if (true)
    ASSERT(1);
  else
    ASSERT(0);

    // Emscripten backends are often imprecise in their submission of DeltaTime.
    // (#6114, #3644) Ideally the Emscripten app/backend should aim to fix or
    // smooth this value and avoid feeding zero, but we tolerate it.
#ifdef __EMSCRIPTEN__
  if (g.IO.DeltaTime <= 0.0f && g.FrameCount > 0)
    g.IO.DeltaTime = 0.00001f;
#endif

  // Check user data
  // (We pass an error message in the assert expression to make it visible to
  // programmers who are not using a debugger, as most assert handlers display
  // their argument)
  ASSERT(g.Initialized);
  ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0) &&
         "Need a positive DeltaTime!");
  ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount) &&
         "Forgot to call Render() or EndFrame() at the end of the previous "
         "frame?");
  ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f &&
         "Invalid DisplaySize value!");
  ASSERT(g.IO.Fonts->IsBuilt() &&
         "Font Atlas not built! Make sure you called XXXX_NewFrame() "
         "function for renderer backend, which should call "
         "io.Fonts->GetTexDataAsRGBA32() / GetTexDataAsAlpha8()");
  ASSERT(g.Style.CurveTessellationTol > 0.0f && "Invalid style setting!");
  ASSERT(g.Style.CircleTessellationMaxError > 0.0f && "Invalid style setting!");
  ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f &&
         "Invalid style setting!"); // Allows us to avoid a few clamps in
                                    // color computations
  ASSERT(g.Style.WindowMinSize.x >= 1.0f && g.Style.WindowMinSize.y >= 1.0f &&
         "Invalid style setting.");
  ASSERT(g.Style.WindowMenuButtonPosition == Dir_None ||
         g.Style.WindowMenuButtonPosition == Dir_Left ||
         g.Style.WindowMenuButtonPosition == Dir_Right);
  ASSERT(g.Style.ColorButtonPosition == Dir_Left ||
         g.Style.ColorButtonPosition == Dir_Right);
#ifndef DISABLE_OBSOLETE_KEYIO
  for (int n = Key_NamedKey_BEGIN; n < Key_COUNT; n++)
    ASSERT(g.IO.KeyMap[n] >= -1 && g.IO.KeyMap[n] < Key_LegacyNativeKey_END &&
           "io.KeyMap[] contains an out of bound value (need to be 0..511, "
           "or -1 for unmapped key)");

  // Check: required key mapping (we intentionally do NOT check all keys to not
  // pressure user into setting up everything, but Space is required and was
  // only added in 1.60 WIP)
  if ((g.IO.ConfigFlags & ConfigFlags_NavEnableKeyboard) &&
      g.IO.BackendUsingLegacyKeyArrays == 1)
    ASSERT(g.IO.KeyMap[Key_Space] != -1 &&
           "Key_Space is not mapped, required for keyboard navigation.");
#endif

  // Check: the io.ConfigWindowsResizeFromEdges option requires backend to honor
  // mouse cursor changes and set the BackendFlags_HasMouseCursors flag
  // accordingly.
  if (g.IO.ConfigWindowsResizeFromEdges &&
      !(g.IO.BackendFlags & BackendFlags_HasMouseCursors))
    g.IO.ConfigWindowsResizeFromEdges = false;

  // Perform simple check: error if Docking or Viewport are enabled _exactly_ on
  // frame 1 (instead of frame 0 or later), which is a common error leading to
  // loss of .ini data.
  if (g.FrameCount == 1 && (g.IO.ConfigFlags & ConfigFlags_DockingEnable) &&
      (g.ConfigFlagsLastFrame & ConfigFlags_DockingEnable) == 0)
    ASSERT(0 && "Please set DockingEnable before the first call to "
                "NewFrame()! Otherwise you will lose your .ini settings!");
  if (g.FrameCount == 1 && (g.IO.ConfigFlags & ConfigFlags_ViewportsEnable) &&
      (g.ConfigFlagsLastFrame & ConfigFlags_ViewportsEnable) == 0)
    ASSERT(0 && "Please set ViewportsEnable before the first call to "
                "NewFrame()! Otherwise you will lose your .ini settings!");

  // Perform simple checks: multi-viewport and platform windows support
  if (g.IO.ConfigFlags & ConfigFlags_ViewportsEnable) {
    if ((g.IO.BackendFlags & BackendFlags_PlatformHasViewports) &&
        (g.IO.BackendFlags & BackendFlags_RendererHasViewports)) {
      ASSERT((g.FrameCount == 0 || g.FrameCount == g.FrameCountPlatformEnded) &&
             "Forgot to call UpdatePlatformWindows() in main loop after "
             "EndFrame()? Check examples/ applications for reference.");
      ASSERT(g.PlatformIO.Platform_CreateWindow != NULL &&
             "Platform init didn't install handlers?");
      ASSERT(g.PlatformIO.Platform_DestroyWindow != NULL &&
             "Platform init didn't install handlers?");
      ASSERT(g.PlatformIO.Platform_GetWindowPos != NULL &&
             "Platform init didn't install handlers?");
      ASSERT(g.PlatformIO.Platform_SetWindowPos != NULL &&
             "Platform init didn't install handlers?");
      ASSERT(g.PlatformIO.Platform_GetWindowSize != NULL &&
             "Platform init didn't install handlers?");
      ASSERT(g.PlatformIO.Platform_SetWindowSize != NULL &&
             "Platform init didn't install handlers?");
      ASSERT(g.PlatformIO.Monitors.Size > 0 &&
             "Platform init didn't setup Monitors list?");
      ASSERT((g.Viewports[0]->PlatformUserData != NULL ||
              g.Viewports[0]->PlatformHandle != NULL) &&
             "Platform init didn't setup main viewport.");
      if (g.IO.ConfigDockingTransparentPayload &&
          (g.IO.ConfigFlags & ConfigFlags_DockingEnable))
        ASSERT(g.PlatformIO.Platform_SetWindowAlpha != NULL &&
               "Platform_SetWindowAlpha handler is required to use "
               "io.ConfigDockingTransparent!");
    } else {
      // Disable feature, our backends do not support it
      g.IO.ConfigFlags &= ~ConfigFlags_ViewportsEnable;
    }

    // Perform simple checks on platform monitor data + compute a total bounding
    // box for quick early outs
    for (PlatformMonitor &mon : g.PlatformIO.Monitors) {
      UNUSED(mon);
      ASSERT(mon.MainSize.x > 0.0f && mon.MainSize.y > 0.0f &&
             "Monitor main bounds not setup properly.");
      ASSERT(
          Rect(mon.MainPos, mon.MainPos + mon.MainSize)
              .Contains(Rect(mon.WorkPos, mon.WorkPos + mon.WorkSize)) &&
          "Monitor work bounds not setup properly. If you don't have work area "
          "information, just copy MainPos/MainSize into them.");
      ASSERT(mon.DpiScale != 0.0f);
    }
  }
}

static void Gui::ErrorCheckEndFrameSanityChecks() {
  Context &g = *GGui;

  // Verify that io.KeyXXX fields haven't been tampered with. Key mods should
  // not be modified between NewFrame() and EndFrame() One possible reason
  // leading to this assert is that your backends update inputs _AFTER_
  // NewFrame(). It is known that when some modal native windows called
  // mid-frame takes focus away, some backends such as GLFW will send key
  // release events mid-frame. This would normally trigger this assertion and
  // lead to sheared inputs. We silently accommodate for this case by ignoring
  // the case where all io.KeyXXX modifiers were released (aka key_mod_flags ==
  // 0), while still correctly asserting on mid-frame key press events.
  const KeyChord key_mods = GetMergedModsFromKeys();
  ASSERT(
      (key_mods == 0 || g.IO.KeyMods == key_mods) &&
      "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");
  UNUSED(key_mods);

  // [EXPERIMENTAL] Recover from errors: You may call this yourself before
  // EndFrame().
  // ErrorCheckEndFrameRecover();

  // Report when there is a mismatch of Begin/BeginChild vs End/EndChild calls.
  // Important: Remember that the Begin/BeginChild API requires you to always
  // call End/EndChild even if Begin/BeginChild returns false! (this is
  // unfortunately inconsistent with most other Begin* API).
  if (g.CurrentWindowStack.Size != 1) {
    if (g.CurrentWindowStack.Size > 1) {
      Window *window =
          g.CurrentWindowStack.back().Window; // <-- This window was not Ended!
      ASSERT_USER_ERROR(g.CurrentWindowStack.Size == 1,
                        "Mismatched Begin/BeginChild vs End/EndChild calls: "
                        "did you forget to call End/EndChild?");
      UNUSED(window);
      while (g.CurrentWindowStack.Size > 1)
        End();
    } else {
      ASSERT_USER_ERROR(g.CurrentWindowStack.Size == 1,
                        "Mismatched Begin/BeginChild vs End/EndChild calls: "
                        "did you call End/EndChild too much?");
    }
  }

  ASSERT_USER_ERROR(g.GroupStack.Size == 0, "Missing EndGroup call!");
}

// Experimental recovery from incorrect usage of BeginXXX/EndXXX/PushXXX/PopXXX
// calls. Must be called during or before EndFrame(). This is generally flawed
// as we are not necessarily End/Popping things in the right order.
// FIXME: Can't recover from inside BeginTabItem/EndTabItem yet.
// FIXME: Can't recover from interleaved BeginTabBar/Begin
void Gui::ErrorCheckEndFrameRecover(ErrorLogCallback log_callback,
                                    void *user_data) {
  // PVS-Studio V1044 is "Loop break conditions do not depend on the number of
  // iterations"
  Context &g = *GGui;
  while (g.CurrentWindowStack.Size > 0) //-V1044
  {
    ErrorCheckEndWindowRecover(log_callback, user_data);
    Window *window = g.CurrentWindow;
    if (g.CurrentWindowStack.Size == 1) {
      ASSERT(window->IsFallbackWindow);
      break;
    }
    if (window->Flags & WindowFlags_ChildWindow) {
      if (log_callback)
        log_callback(user_data, "Recovered from missing EndChild() for '%s'",
                     window->Name);
      EndChild();
    } else {
      if (log_callback)
        log_callback(user_data, "Recovered from missing End() for '%s'",
                     window->Name);
      End();
    }
  }
}

// Must be called before End()/EndChild()
void Gui::ErrorCheckEndWindowRecover(ErrorLogCallback log_callback,
                                     void *user_data) {
  Context &g = *GGui;
  while (g.CurrentTable && (g.CurrentTable->OuterWindow == g.CurrentWindow ||
                            g.CurrentTable->InnerWindow == g.CurrentWindow)) {
    if (log_callback)
      log_callback(user_data, "Recovered from missing EndTable() in '%s'",
                   g.CurrentTable->OuterWindow->Name);
    EndTable();
  }

  Window *window = g.CurrentWindow;
  StackSizes *stack_sizes = &g.CurrentWindowStack.back().StackSizesOnBegin;
  ASSERT(window != NULL);
  while (g.CurrentTabBar != NULL) //-V1044
  {
    if (log_callback)
      log_callback(user_data, "Recovered from missing EndTabBar() in '%s'",
                   window->Name);
    EndTabBar();
  }
  while (window->DC.TreeDepth > 0) {
    if (log_callback)
      log_callback(user_data, "Recovered from missing TreePop() in '%s'",
                   window->Name);
    TreePop();
  }
  while (g.GroupStack.Size > stack_sizes->SizeOfGroupStack) //-V1044
  {
    if (log_callback)
      log_callback(user_data, "Recovered from missing EndGroup() in '%s'",
                   window->Name);
    EndGroup();
  }
  while (window->IDStack.Size > 1) {
    if (log_callback)
      log_callback(user_data, "Recovered from missing PopID() in '%s'",
                   window->Name);
    PopID();
  }
  while (g.DisabledStackSize > stack_sizes->SizeOfDisabledStack) //-V1044
  {
    if (log_callback)
      log_callback(user_data, "Recovered from missing EndDisabled() in '%s'",
                   window->Name);
    EndDisabled();
  }
  while (g.ColorStack.Size > stack_sizes->SizeOfColorStack) {
    if (log_callback)
      log_callback(user_data,
                   "Recovered from missing PopStyleColor() in '%s' for Col_%s",
                   window->Name, GetStyleColorName(g.ColorStack.back().Col));
    PopStyleColor();
  }
  while (g.ItemFlagsStack.Size > stack_sizes->SizeOfItemFlagsStack) //-V1044
  {
    if (log_callback)
      log_callback(user_data, "Recovered from missing PopItemFlag() in '%s'",
                   window->Name);
    PopItemFlag();
  }
  while (g.StyleVarStack.Size > stack_sizes->SizeOfStyleVarStack) //-V1044
  {
    if (log_callback)
      log_callback(user_data, "Recovered from missing PopStyleVar() in '%s'",
                   window->Name);
    PopStyleVar();
  }
  while (g.FontStack.Size > stack_sizes->SizeOfFontStack) //-V1044
  {
    if (log_callback)
      log_callback(user_data, "Recovered from missing PopFont() in '%s'",
                   window->Name);
    PopFont();
  }
  while (g.FocusScopeStack.Size >
         stack_sizes->SizeOfFocusScopeStack + 1) //-V1044
  {
    if (log_callback)
      log_callback(user_data, "Recovered from missing PopFocusScope() in '%s'",
                   window->Name);
    PopFocusScope();
  }
}

// Save current stack sizes for later compare
void StackSizes::SetToContextState(Context *ctx) {
  Context &g = *ctx;
  Window *window = g.CurrentWindow;
  SizeOfIDStack = (short)window->IDStack.Size;
  SizeOfColorStack = (short)g.ColorStack.Size;
  SizeOfStyleVarStack = (short)g.StyleVarStack.Size;
  SizeOfFontStack = (short)g.FontStack.Size;
  SizeOfFocusScopeStack = (short)g.FocusScopeStack.Size;
  SizeOfGroupStack = (short)g.GroupStack.Size;
  SizeOfItemFlagsStack = (short)g.ItemFlagsStack.Size;
  SizeOfBeginPopupStack = (short)g.BeginPopupStack.Size;
  SizeOfDisabledStack = (short)g.DisabledStackSize;
}

// Compare to detect usage errors
void StackSizes::CompareWithContextState(Context *ctx) {
  Context &g = *ctx;
  Window *window = g.CurrentWindow;
  UNUSED(window);

  // Window stacks
  // NOT checking: DC.ItemWidth, DC.TextWrapPos (per window) to allow user to
  // conveniently push once and not pop (they are cleared on Begin)
  ASSERT(SizeOfIDStack == window->IDStack.Size &&
         "PushID/PopID or TreeNode/TreePop Mismatch!");

  // Global stacks
  // For color, style and font stacks there is an incentive to use
  // Push/Begin/Pop/.../End patterns, so we relax our checks a little to allow
  // them.
  ASSERT(SizeOfGroupStack == g.GroupStack.Size &&
         "BeginGroup/EndGroup Mismatch!");
  ASSERT(SizeOfBeginPopupStack == g.BeginPopupStack.Size &&
         "BeginPopup/EndPopup or BeginMenu/EndMenu Mismatch!");
  ASSERT(SizeOfDisabledStack == g.DisabledStackSize &&
         "BeginDisabled/EndDisabled Mismatch!");
  ASSERT(SizeOfItemFlagsStack >= g.ItemFlagsStack.Size &&
         "PushItemFlag/PopItemFlag Mismatch!");
  ASSERT(SizeOfColorStack >= g.ColorStack.Size &&
         "PushStyleColor/PopStyleColor Mismatch!");
  ASSERT(SizeOfStyleVarStack >= g.StyleVarStack.Size &&
         "PushStyleVar/PopStyleVar Mismatch!");
  ASSERT(SizeOfFontStack >= g.FontStack.Size && "PushFont/PopFont Mismatch!");
  ASSERT(SizeOfFocusScopeStack == g.FocusScopeStack.Size &&
         "PushFocusScope/PopFocusScope Mismatch!");
}

//-----------------------------------------------------------------------------
// [SECTION] LAYOUT
//-----------------------------------------------------------------------------
// - ItemSize()
// - ItemAdd()
// - SameLine()
// - GetCursorScreenPos()
// - SetCursorScreenPos()
// - GetCursorPos(), GetCursorPosX(), GetCursorPosY()
// - SetCursorPos(), SetCursorPosX(), SetCursorPosY()
// - GetCursorStartPos()
// - Indent()
// - Unindent()
// - SetNextItemWidth()
// - PushItemWidth()
// - PushMultiItemsWidths()
// - PopItemWidth()
// - CalcItemWidth()
// - CalcItemSize()
// - GetTextLineHeight()
// - GetTextLineHeightWithSpacing()
// - GetFrameHeight()
// - GetFrameHeightWithSpacing()
// - GetContentRegionMax()
// - GetContentRegionMaxAbs() [Internal]
// - GetContentRegionAvail(),
// - GetWindowContentRegionMin(), GetWindowContentRegionMax()
// - BeginGroup()
// - EndGroup()
// Also see in widgets: tab bars, and in tables: tables, columns.
//-----------------------------------------------------------------------------

// Advance cursor given item size for layout.
// Register minimum needed size so it can extend the bounding box used for
// auto-fit calculation. See comments in ItemAdd() about how/why the size
// provided to ItemSize() vs ItemAdd() may often different. THIS IS IN THE
// PERFORMANCE CRITICAL PATH.
void Gui::ItemSize(const Vec2 &size, float text_baseline_y) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  // We increase the height in this function to accommodate for baseline offset.
  // In theory we should be offsetting the starting position
  // (window->DC.CursorPos), that will be the topic of a larger refactor, but
  // since ItemSize() is not yet an API that moves the cursor (to handle e.g.
  // wrapping) enlarging the height has the same effect.
  const float offset_to_match_baseline_y =
      (text_baseline_y >= 0)
          ? Max(0.0f, window->DC.CurrLineTextBaseOffset - text_baseline_y)
          : 0.0f;

  const float line_y1 = window->DC.IsSameLine ? window->DC.CursorPosPrevLine.y
                                              : window->DC.CursorPos.y;
  const float line_height =
      Max(window->DC.CurrLineSize.y, /*Max(*/ window->DC.CursorPos.y -
                                         line_y1 /*, 0.0f)*/ + size.y +
                                         offset_to_match_baseline_y);

  // Always align ourselves on pixel boundaries
  // if (g.IO.KeyAlt) window->DrawList->AddRect(window->DC.CursorPos,
  // window->DC.CursorPos + Vec2(size.x, line_height), COL32(255,0,0,200));
  // // [DEBUG]
  window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x + size.x;
  window->DC.CursorPosPrevLine.y = line_y1;
  window->DC.CursorPos.x = TRUNC(window->Pos.x + window->DC.Indent.x +
                                 window->DC.ColumnsOffset.x); // Next line
  window->DC.CursorPos.y =
      TRUNC(line_y1 + line_height + g.Style.ItemSpacing.y); // Next line
  window->DC.CursorMaxPos.x =
      Max(window->DC.CursorMaxPos.x, window->DC.CursorPosPrevLine.x);
  window->DC.CursorMaxPos.y =
      Max(window->DC.CursorMaxPos.y,
          window->DC.CursorPos.y - g.Style.ItemSpacing.y);
  // if (g.IO.KeyAlt) window->DrawList->AddCircle(window->DC.CursorMaxPos, 3.0f,
  // COL32(255,0,0,255), 4); // [DEBUG]

  window->DC.PrevLineSize.y = line_height;
  window->DC.CurrLineSize.y = 0.0f;
  window->DC.PrevLineTextBaseOffset =
      Max(window->DC.CurrLineTextBaseOffset, text_baseline_y);
  window->DC.CurrLineTextBaseOffset = 0.0f;
  window->DC.IsSameLine = window->DC.IsSetPos = false;

  // Horizontal layout mode
  if (window->DC.LayoutType == LayoutType_Horizontal)
    SameLine();
}

// Declare item bounding box for clipping and interaction.
// Note that the size can be different than the one provided to ItemSize().
// Typically, widgets that spread over available surface declare their minimum
// size requirement to ItemSize() and provide a larger region to ItemAdd() which
// is used drawing/interaction. THIS IS IN THE PERFORMANCE CRITICAL PATH (UNTIL
// THE CLIPPING TEST AND EARLY-RETURN)
bool Gui::ItemAdd(const Rect &bb, ID id, const Rect *nav_bb_arg,
                  ItemFlags extra_flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  // Set item data
  // (DisplayRect is left untouched, made valid when
  // ItemStatusFlags_HasDisplayRect is set)
  g.LastItemData.ID = id;
  g.LastItemData.Rect = bb;
  g.LastItemData.NavRect = nav_bb_arg ? *nav_bb_arg : bb;
  g.LastItemData.InFlags =
      g.CurrentItemFlags | g.NextItemData.ItemFlags | extra_flags;
  g.LastItemData.StatusFlags = ItemStatusFlags_None;
  // Note: we don't copy 'g.NextItemData.SelectionUserData' to an hypothetical
  // g.LastItemData.SelectionUserData: since the former is not cleared.

  if (id != 0) {
    KeepAliveID(id);

    // Directional navigation processing
    // Runs prior to clipping early-out
    //  (a) So that NavInitRequest can be honored, for newly opened windows to
    //  select a default widget (b) So that we can scroll up/down past clipped
    //  items. This adds a small O(N) cost to regular navigation requests
    //      unfortunately, but it is still limited to one window. It may not
    //      scale very well for windows with ten of thousands of item, but at
    //      least NavMoveRequest is only set on user interaction, aka maximum
    //      once a frame. We could early out with "if (is_clipped &&
    //      !g.NavInitRequest) return false;" but when we wouldn't be able to
    //      reach unclipped widgets. This would work if user had explicit
    //      scrolling control (e.g. mapped on a stick).
    // We intentionally don't check if g.NavWindow != NULL because
    // g.NavAnyRequest should only be set when it is non null. If we crash on a
    // NULL g.NavWindow we need to fix the bug elsewhere.
    if (!(g.LastItemData.InFlags & ItemFlags_NoNav)) {
      // FIMXE-NAV: investigate changing the window tests into a simple 'if
      // (g.NavFocusScopeId == g.CurrentFocusScopeId)' test.
      window->DC.NavLayersActiveMaskNext |= (1 << window->DC.NavLayerCurrent);
      if (g.NavId == id || g.NavAnyRequest)
        if (g.NavWindow->RootWindowForNav == window->RootWindowForNav)
          if (window == g.NavWindow ||
              ((window->Flags | g.NavWindow->Flags) & WindowFlags_NavFlattened))
            NavProcessItem();
    }
  }

  // Lightweight clear of SetNextItemXXX data.
  g.NextItemData.Flags = NextItemDataFlags_None;
  g.NextItemData.ItemFlags = ItemFlags_None;

#ifdef ENABLE_TEST_ENGINE
  if (id != 0)
    TEST_ENGINE_ITEM_ADD(id, g.LastItemData.NavRect, &g.LastItemData);
#endif

  // Clipping test
  // (this is a modified copy of IsClippedEx() so we can reuse the
  // is_rect_visible value)
  // const bool is_clipped = IsClippedEx(bb, id);
  // if (is_clipped)
  //    return false;
  const bool is_rect_visible = bb.Overlaps(window->ClipRect);
  if (!is_rect_visible)
    if (id == 0 ||
        (id != g.ActiveId && id != g.ActiveIdPreviousFrame && id != g.NavId))
      if (!g.LogEnabled)
        return false;

        // [DEBUG]
#ifndef DISABLE_DEBUG_TOOLS
  if (id != 0) {
    if (id == g.DebugLocateId)
      DebugLocateItemResolveWithLastItem();

    // [DEBUG] People keep stumbling on this problem and using "" as identifier
    // in the root of a window instead of "##something". Empty identifier are
    // valid and useful in a small amount of cases, but 99.9% of the time you
    // want to use "##something".
    ASSERT(
        id != window->ID &&
        "Cannot have an empty ID at the root of a window. If you need an empty "
        "label, use ## and read the FAQ about how the ID Stack works!");
  }
  // if (g.IO.KeyAlt) window->DrawList->AddRect(bb.Min, bb.Max,
  // COL32(255,255,0,120)); // [DEBUG] if ((g.LastItemData.InFlags &
  // ItemFlags_NoNav) == 0)
  //     window->DrawList->AddRect(g.LastItemData.NavRect.Min,
  //     g.LastItemData.NavRect.Max, COL32(255,255,0,255)); // [DEBUG]
#endif

  // We need to calculate this now to take account of the current clipping
  // rectangle (as items like Selectable may change them)
  if (is_rect_visible)
    g.LastItemData.StatusFlags |= ItemStatusFlags_Visible;
  if (IsMouseHoveringRect(bb.Min, bb.Max))
    g.LastItemData.StatusFlags |= ItemStatusFlags_HoveredRect;
  return true;
}

// Gets back to previous line and continue with horizontal layout
//      offset_from_start_x == 0 : follow right after previous item
//      offset_from_start_x != 0 : align to specified x position (relative to
//      window/group left) spacing_w < 0            : use default spacing if
//      offset_from_start_x == 0, no spacing if offset_from_start_x != 0
//      spacing_w >= 0           : enforce spacing amount
void Gui::SameLine(float offset_from_start_x, float spacing_w) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  if (offset_from_start_x != 0.0f) {
    if (spacing_w < 0.0f)
      spacing_w = 0.0f;
    window->DC.CursorPos.x =
        window->Pos.x - window->Scroll.x + offset_from_start_x + spacing_w +
        window->DC.GroupOffset.x + window->DC.ColumnsOffset.x;
    window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
  } else {
    if (spacing_w < 0.0f)
      spacing_w = g.Style.ItemSpacing.x;
    window->DC.CursorPos.x = window->DC.CursorPosPrevLine.x + spacing_w;
    window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
  }
  window->DC.CurrLineSize = window->DC.PrevLineSize;
  window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
  window->DC.IsSameLine = true;
}

Vec2 Gui::GetCursorScreenPos() {
  Window *window = GetCurrentWindowRead();
  return window->DC.CursorPos;
}

void Gui::SetCursorScreenPos(const Vec2 &pos) {
  Window *window = GetCurrentWindow();
  window->DC.CursorPos = pos;
  // window->DC.CursorMaxPos = Max(window->DC.CursorMaxPos,
  // window->DC.CursorPos);
  window->DC.IsSetPos = true;
}

// User generally sees positions in window coordinates. Internally we store
// CursorPos in absolute screen coordinates because it is more convenient.
// Conversion happens as we pass the value to user, but it makes our naming
// convention confusing because GetCursorPos() == (DC.CursorPos - window.Pos).
// May want to rename 'DC.CursorPos'.
Vec2 Gui::GetCursorPos() {
  Window *window = GetCurrentWindowRead();
  return window->DC.CursorPos - window->Pos + window->Scroll;
}

float Gui::GetCursorPosX() {
  Window *window = GetCurrentWindowRead();
  return window->DC.CursorPos.x - window->Pos.x + window->Scroll.x;
}

float Gui::GetCursorPosY() {
  Window *window = GetCurrentWindowRead();
  return window->DC.CursorPos.y - window->Pos.y + window->Scroll.y;
}

void Gui::SetCursorPos(const Vec2 &local_pos) {
  Window *window = GetCurrentWindow();
  window->DC.CursorPos = window->Pos - window->Scroll + local_pos;
  // window->DC.CursorMaxPos = Max(window->DC.CursorMaxPos,
  // window->DC.CursorPos);
  window->DC.IsSetPos = true;
}

void Gui::SetCursorPosX(float x) {
  Window *window = GetCurrentWindow();
  window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + x;
  // window->DC.CursorMaxPos.x = Max(window->DC.CursorMaxPos.x,
  // window->DC.CursorPos.x);
  window->DC.IsSetPos = true;
}

void Gui::SetCursorPosY(float y) {
  Window *window = GetCurrentWindow();
  window->DC.CursorPos.y = window->Pos.y - window->Scroll.y + y;
  // window->DC.CursorMaxPos.y = Max(window->DC.CursorMaxPos.y,
  // window->DC.CursorPos.y);
  window->DC.IsSetPos = true;
}

Vec2 Gui::GetCursorStartPos() {
  Window *window = GetCurrentWindowRead();
  return window->DC.CursorStartPos - window->Pos;
}

void Gui::Indent(float indent_w) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  window->DC.Indent.x += (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
  window->DC.CursorPos.x =
      window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

void Gui::Unindent(float indent_w) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  window->DC.Indent.x -= (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
  window->DC.CursorPos.x =
      window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

// Affect large frame+labels widgets only.
void Gui::SetNextItemWidth(float item_width) {
  Context &g = *GGui;
  g.NextItemData.Flags |= NextItemDataFlags_HasWidth;
  g.NextItemData.Width = item_width;
}

// FIXME: Remove the == 0.0f behavior?
void Gui::PushItemWidth(float item_width) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  window->DC.ItemWidthStack.push_back(
      window->DC.ItemWidth); // Backup current width
  window->DC.ItemWidth =
      (item_width == 0.0f ? window->ItemWidthDefault : item_width);
  g.NextItemData.Flags &= ~NextItemDataFlags_HasWidth;
}

void Gui::PushMultiItemsWidths(int components, float w_full) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT(components > 0);
  const Style &style = g.Style;
  window->DC.ItemWidthStack.push_back(
      window->DC.ItemWidth); // Backup current width
  float w_items = w_full - style.ItemInnerSpacing.x * (components - 1);
  float prev_split = w_items;
  for (int i = components - 1; i > 0; i--) {
    float next_split = TRUNC(w_items * i / components);
    window->DC.ItemWidthStack.push_back(Max(prev_split - next_split, 1.0f));
    prev_split = next_split;
  }
  window->DC.ItemWidth = Max(prev_split, 1.0f);
  g.NextItemData.Flags &= ~NextItemDataFlags_HasWidth;
}

void Gui::PopItemWidth() {
  Window *window = GetCurrentWindow();
  window->DC.ItemWidth = window->DC.ItemWidthStack.back();
  window->DC.ItemWidthStack.pop_back();
}

// Calculate default item width given value passed to PushItemWidth() or
// SetNextItemWidth(). The SetNextItemWidth() data is generally cleared/consumed
// by ItemAdd() or NextItemData.ClearFlags()
float Gui::CalcItemWidth() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  float w;
  if (g.NextItemData.Flags & NextItemDataFlags_HasWidth)
    w = g.NextItemData.Width;
  else
    w = window->DC.ItemWidth;
  if (w < 0.0f) {
    float region_max_x = GetContentRegionMaxAbs().x;
    w = Max(1.0f, region_max_x - window->DC.CursorPos.x + w);
  }
  w = TRUNC(w);
  return w;
}

// [Internal] Calculate full item size given user provided 'size' parameter and
// default width/height. Default width is often == CalcItemWidth(). Those two
// functions CalcItemWidth vs CalcItemSize are awkwardly named because they are
// not fully symmetrical. Note that only CalcItemWidth() is publicly exposed.
// The 4.0f here may be changed to match CalcItemWidth() and/or BeginChild()
// (right now we have a mismatch which is harmless but undesirable)
Vec2 Gui::CalcItemSize(Vec2 size, float default_w, float default_h) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  Vec2 region_max;
  if (size.x < 0.0f || size.y < 0.0f)
    region_max = GetContentRegionMaxAbs();

  if (size.x == 0.0f)
    size.x = default_w;
  else if (size.x < 0.0f)
    size.x = Max(4.0f, region_max.x - window->DC.CursorPos.x + size.x);

  if (size.y == 0.0f)
    size.y = default_h;
  else if (size.y < 0.0f)
    size.y = Max(4.0f, region_max.y - window->DC.CursorPos.y + size.y);

  return size;
}

float Gui::GetTextLineHeight() {
  Context &g = *GGui;
  return g.FontSize;
}

float Gui::GetTextLineHeightWithSpacing() {
  Context &g = *GGui;
  return g.FontSize + g.Style.ItemSpacing.y;
}

float Gui::GetFrameHeight() {
  Context &g = *GGui;
  return g.FontSize + g.Style.FramePadding.y * 2.0f;
}

float Gui::GetFrameHeightWithSpacing() {
  Context &g = *GGui;
  return g.FontSize + g.Style.FramePadding.y * 2.0f + g.Style.ItemSpacing.y;
}

// FIXME: All the Contents Region function are messy or misleading. WE WILL AIM
// TO OBSOLETE ALL OF THEM WITH A NEW "WORK RECT" API. Thanks for your patience!

// FIXME: This is in window space (not screen space!).
Vec2 Gui::GetContentRegionMax() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Vec2 mx = (window->DC.CurrentColumns || g.CurrentTable)
                ? window->WorkRect.Max
                : window->ContentRegionRect.Max;
  return mx - window->Pos;
}

// [Internal] Absolute coordinate. Saner. This is not exposed until we finishing
// refactoring work rect features.
Vec2 Gui::GetContentRegionMaxAbs() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Vec2 mx = (window->DC.CurrentColumns || g.CurrentTable)
                ? window->WorkRect.Max
                : window->ContentRegionRect.Max;
  return mx;
}

Vec2 Gui::GetContentRegionAvail() {
  Window *window = GGui->CurrentWindow;
  return GetContentRegionMaxAbs() - window->DC.CursorPos;
}

// In window space (not screen space!)
Vec2 Gui::GetWindowContentRegionMin() {
  Window *window = GGui->CurrentWindow;
  return window->ContentRegionRect.Min - window->Pos;
}

Vec2 Gui::GetWindowContentRegionMax() {
  Window *window = GGui->CurrentWindow;
  return window->ContentRegionRect.Max - window->Pos;
}

// Lock horizontal starting position + capture group bounding box into one
// "item" (so you can use IsItemHovered() or layout primitives such as
// SameLine() on whole group, etc.) Groups are currently a mishmash of
// functionalities which should perhaps be clarified and separated.
// FIXME-OPT: Could we safely early out on ->SkipItems?
void Gui::BeginGroup() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  g.GroupStack.resize(g.GroupStack.Size + 1);
  GroupData &group_data = g.GroupStack.back();
  group_data.WindowID = window->ID;
  group_data.BackupCursorPos = window->DC.CursorPos;
  group_data.BackupCursorPosPrevLine = window->DC.CursorPosPrevLine;
  group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
  group_data.BackupIndent = window->DC.Indent;
  group_data.BackupGroupOffset = window->DC.GroupOffset;
  group_data.BackupCurrLineSize = window->DC.CurrLineSize;
  group_data.BackupCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
  group_data.BackupActiveIdIsAlive = g.ActiveIdIsAlive;
  group_data.BackupHoveredIdIsAlive = g.HoveredId != 0;
  group_data.BackupIsSameLine = window->DC.IsSameLine;
  group_data.BackupActiveIdPreviousFrameIsAlive =
      g.ActiveIdPreviousFrameIsAlive;
  group_data.EmitItem = true;

  window->DC.GroupOffset.x =
      window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffset.x;
  window->DC.Indent = window->DC.GroupOffset;
  window->DC.CursorMaxPos = window->DC.CursorPos;
  window->DC.CurrLineSize = Vec2(0.0f, 0.0f);
  if (g.LogEnabled)
    g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}

void Gui::EndGroup() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT(g.GroupStack.Size > 0); // Mismatched BeginGroup()/EndGroup() calls

  GroupData &group_data = g.GroupStack.back();
  ASSERT(group_data.WindowID == window->ID); // EndGroup() in wrong window?

  if (window->DC.IsSetPos)
    ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

  Rect group_bb(group_data.BackupCursorPos,
                Max(window->DC.CursorMaxPos, group_data.BackupCursorPos));

  window->DC.CursorPos = group_data.BackupCursorPos;
  window->DC.CursorPosPrevLine = group_data.BackupCursorPosPrevLine;
  window->DC.CursorMaxPos =
      Max(group_data.BackupCursorMaxPos, window->DC.CursorMaxPos);
  window->DC.Indent = group_data.BackupIndent;
  window->DC.GroupOffset = group_data.BackupGroupOffset;
  window->DC.CurrLineSize = group_data.BackupCurrLineSize;
  window->DC.CurrLineTextBaseOffset = group_data.BackupCurrLineTextBaseOffset;
  window->DC.IsSameLine = group_data.BackupIsSameLine;
  if (g.LogEnabled)
    g.LogLinePosY = -FLT_MAX; // To enforce a carriage return

  if (!group_data.EmitItem) {
    g.GroupStack.pop_back();
    return;
  }

  window->DC.CurrLineTextBaseOffset = Max(
      window->DC.PrevLineTextBaseOffset,
      group_data.BackupCurrLineTextBaseOffset); // FIXME: Incorrect, we should
                                                // grab the base offset from the
                                                // *first line* of the group but
                                                // it is hard to obtain now.
  ItemSize(group_bb.GetSize());
  ItemAdd(group_bb, 0, NULL, ItemFlags_NoTabStop);

  // If the current ActiveId was declared within the boundary of our group, we
  // copy it to LastItemId so IsItemActive(), IsItemDeactivated() etc. will be
  // functional on the entire group. It would be neater if we replaced
  // window.DC.LastItemId by e.g. 'bool LastItemIsActive', but would put a
  // little more burden on individual widgets. Also if you grep for LastItemId
  // you'll notice it is only used in that context. (The two tests not the same
  // because ActiveIdIsAlive is an ID itself, in order to be able to handle
  // ActiveId being overwritten during the frame.)
  const bool group_contains_curr_active_id =
      (group_data.BackupActiveIdIsAlive != g.ActiveId) &&
      (g.ActiveIdIsAlive == g.ActiveId) && g.ActiveId;
  const bool group_contains_prev_active_id =
      (group_data.BackupActiveIdPreviousFrameIsAlive == false) &&
      (g.ActiveIdPreviousFrameIsAlive == true);
  if (group_contains_curr_active_id)
    g.LastItemData.ID = g.ActiveId;
  else if (group_contains_prev_active_id)
    g.LastItemData.ID = g.ActiveIdPreviousFrame;
  g.LastItemData.Rect = group_bb;

  // Forward Hovered flag
  const bool group_contains_curr_hovered_id =
      (group_data.BackupHoveredIdIsAlive == false) && g.HoveredId != 0;
  if (group_contains_curr_hovered_id)
    g.LastItemData.StatusFlags |= ItemStatusFlags_HoveredWindow;

  // Forward Edited flag
  if (group_contains_curr_active_id && g.ActiveIdHasBeenEditedThisFrame)
    g.LastItemData.StatusFlags |= ItemStatusFlags_Edited;

  // Forward Deactivated flag
  g.LastItemData.StatusFlags |= ItemStatusFlags_HasDeactivated;
  if (group_contains_prev_active_id && g.ActiveId != g.ActiveIdPreviousFrame)
    g.LastItemData.StatusFlags |= ItemStatusFlags_Deactivated;

  g.GroupStack.pop_back();
  if (g.DebugShowGroupRects)
    window->DrawList->AddRect(group_bb.Min, group_bb.Max,
                              COL32(255, 0, 255, 255)); // [Debug]
}

//-----------------------------------------------------------------------------
// [SECTION] SCROLLING
//-----------------------------------------------------------------------------

// Helper to snap on edges when aiming at an item very close to the edge,
// So the difference between WindowPadding and ItemSpacing will be in the
// visible area after scrolling. When we refactor the scrolling API this may be
// configurable with a flag? Note that the effect for this won't be visible on X
// axis with default Style settings as WindowPadding.x == ItemSpacing.x by
// default.
static float CalcScrollEdgeSnap(float target, float snap_min, float snap_max,
                                float snap_threshold, float center_ratio) {
  if (target <= snap_min + snap_threshold)
    return Lerp(snap_min, target, center_ratio);
  if (target >= snap_max - snap_threshold)
    return Lerp(target, snap_max, center_ratio);
  return target;
}

static Vec2 CalcNextScrollFromScrollTargetAndClamp(Window *window) {
  Vec2 scroll = window->Scroll;
  Vec2 decoration_size(window->DecoOuterSizeX1 + window->DecoInnerSizeX1 +
                           window->DecoOuterSizeX2,
                       window->DecoOuterSizeY1 + window->DecoInnerSizeY1 +
                           window->DecoOuterSizeY2);
  for (int axis = 0; axis < 2; axis++) {
    if (window->ScrollTarget[axis] < FLT_MAX) {
      float center_ratio = window->ScrollTargetCenterRatio[axis];
      float scroll_target = window->ScrollTarget[axis];
      if (window->ScrollTargetEdgeSnapDist[axis] > 0.0f) {
        float snap_min = 0.0f;
        float snap_max = window->ScrollMax[axis] + window->SizeFull[axis] -
                         decoration_size[axis];
        scroll_target = CalcScrollEdgeSnap(
            scroll_target, snap_min, snap_max,
            window->ScrollTargetEdgeSnapDist[axis], center_ratio);
      }
      scroll[axis] = scroll_target - center_ratio * (window->SizeFull[axis] -
                                                     decoration_size[axis]);
    }
    scroll[axis] = ROUND(Max(scroll[axis], 0.0f));
    if (!window->Collapsed && !window->SkipItems)
      scroll[axis] = Min(scroll[axis], window->ScrollMax[axis]);
  }
  return scroll;
}

void Gui::ScrollToItem(ScrollFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ScrollToRectEx(window, g.LastItemData.NavRect, flags);
}

void Gui::ScrollToRect(Window *window, const Rect &item_rect,
                       ScrollFlags flags) {
  ScrollToRectEx(window, item_rect, flags);
}

// Scroll to keep newly navigated item fully into view
Vec2 Gui::ScrollToRectEx(Window *window, const Rect &item_rect,
                         ScrollFlags flags) {
  Context &g = *GGui;
  Rect scroll_rect(window->InnerRect.Min - Vec2(1, 1),
                   window->InnerRect.Max + Vec2(1, 1));
  scroll_rect.Min.x =
      Min(scroll_rect.Min.x + window->DecoInnerSizeX1, scroll_rect.Max.x);
  scroll_rect.Min.y =
      Min(scroll_rect.Min.y + window->DecoInnerSizeY1, scroll_rect.Max.y);
  // GetForegroundDrawList(window)->AddRect(item_rect.Min, item_rect.Max,
  // COL32(255,0,0,255), 0.0f, 0, 5.0f); // [DEBUG]
  // GetForegroundDrawList(window)->AddRect(scroll_rect.Min, scroll_rect.Max,
  // COL32_WHITE); // [DEBUG]

  // Check that only one behavior is selected per axis
  ASSERT((flags & ScrollFlags_MaskX_) == 0 ||
         IsPowerOfTwo(flags & ScrollFlags_MaskX_));
  ASSERT((flags & ScrollFlags_MaskY_) == 0 ||
         IsPowerOfTwo(flags & ScrollFlags_MaskY_));

  // Defaults
  ScrollFlags in_flags = flags;
  if ((flags & ScrollFlags_MaskX_) == 0 && window->ScrollbarX)
    flags |= ScrollFlags_KeepVisibleEdgeX;
  if ((flags & ScrollFlags_MaskY_) == 0)
    flags |= window->Appearing ? ScrollFlags_AlwaysCenterY
                               : ScrollFlags_KeepVisibleEdgeY;

  const bool fully_visible_x = item_rect.Min.x >= scroll_rect.Min.x &&
                               item_rect.Max.x <= scroll_rect.Max.x;
  const bool fully_visible_y = item_rect.Min.y >= scroll_rect.Min.y &&
                               item_rect.Max.y <= scroll_rect.Max.y;
  const bool can_be_fully_visible_x =
      (item_rect.GetWidth() + g.Style.ItemSpacing.x * 2.0f) <=
          scroll_rect.GetWidth() ||
      (window->AutoFitFramesX > 0) ||
      (window->Flags & WindowFlags_AlwaysAutoResize) != 0;
  const bool can_be_fully_visible_y =
      (item_rect.GetHeight() + g.Style.ItemSpacing.y * 2.0f) <=
          scroll_rect.GetHeight() ||
      (window->AutoFitFramesY > 0) ||
      (window->Flags & WindowFlags_AlwaysAutoResize) != 0;

  if ((flags & ScrollFlags_KeepVisibleEdgeX) && !fully_visible_x) {
    if (item_rect.Min.x < scroll_rect.Min.x || !can_be_fully_visible_x)
      SetScrollFromPosX(window,
                        item_rect.Min.x - g.Style.ItemSpacing.x - window->Pos.x,
                        0.0f);
    else if (item_rect.Max.x >= scroll_rect.Max.x)
      SetScrollFromPosX(window,
                        item_rect.Max.x + g.Style.ItemSpacing.x - window->Pos.x,
                        1.0f);
  } else if (((flags & ScrollFlags_KeepVisibleCenterX) && !fully_visible_x) ||
             (flags & ScrollFlags_AlwaysCenterX)) {
    if (can_be_fully_visible_x)
      SetScrollFromPosX(window,
                        Trunc((item_rect.Min.x + item_rect.Max.x) * 0.5f) -
                            window->Pos.x,
                        0.5f);
    else
      SetScrollFromPosX(window, item_rect.Min.x - window->Pos.x, 0.0f);
  }

  if ((flags & ScrollFlags_KeepVisibleEdgeY) && !fully_visible_y) {
    if (item_rect.Min.y < scroll_rect.Min.y || !can_be_fully_visible_y)
      SetScrollFromPosY(window,
                        item_rect.Min.y - g.Style.ItemSpacing.y - window->Pos.y,
                        0.0f);
    else if (item_rect.Max.y >= scroll_rect.Max.y)
      SetScrollFromPosY(window,
                        item_rect.Max.y + g.Style.ItemSpacing.y - window->Pos.y,
                        1.0f);
  } else if (((flags & ScrollFlags_KeepVisibleCenterY) && !fully_visible_y) ||
             (flags & ScrollFlags_AlwaysCenterY)) {
    if (can_be_fully_visible_y)
      SetScrollFromPosY(window,
                        Trunc((item_rect.Min.y + item_rect.Max.y) * 0.5f) -
                            window->Pos.y,
                        0.5f);
    else
      SetScrollFromPosY(window, item_rect.Min.y - window->Pos.y, 0.0f);
  }

  Vec2 next_scroll = CalcNextScrollFromScrollTargetAndClamp(window);
  Vec2 delta_scroll = next_scroll - window->Scroll;

  // Also scroll parent window to keep us into view if necessary
  if (!(flags & ScrollFlags_NoScrollParent) &&
      (window->Flags & WindowFlags_ChildWindow)) {
    // FIXME-SCROLL: May be an option?
    if ((in_flags &
         (ScrollFlags_AlwaysCenterX | ScrollFlags_KeepVisibleCenterX)) != 0)
      in_flags =
          (in_flags & ~ScrollFlags_MaskX_) | ScrollFlags_KeepVisibleEdgeX;
    if ((in_flags &
         (ScrollFlags_AlwaysCenterY | ScrollFlags_KeepVisibleCenterY)) != 0)
      in_flags =
          (in_flags & ~ScrollFlags_MaskY_) | ScrollFlags_KeepVisibleEdgeY;
    delta_scroll += ScrollToRectEx(
        window->ParentWindow,
        Rect(item_rect.Min - delta_scroll, item_rect.Max - delta_scroll),
        in_flags);
  }

  return delta_scroll;
}

float Gui::GetScrollX() {
  Window *window = GGui->CurrentWindow;
  return window->Scroll.x;
}

float Gui::GetScrollY() {
  Window *window = GGui->CurrentWindow;
  return window->Scroll.y;
}

float Gui::GetScrollMaxX() {
  Window *window = GGui->CurrentWindow;
  return window->ScrollMax.x;
}

float Gui::GetScrollMaxY() {
  Window *window = GGui->CurrentWindow;
  return window->ScrollMax.y;
}

void Gui::SetScrollX(Window *window, float scroll_x) {
  window->ScrollTarget.x = scroll_x;
  window->ScrollTargetCenterRatio.x = 0.0f;
  window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void Gui::SetScrollY(Window *window, float scroll_y) {
  window->ScrollTarget.y = scroll_y;
  window->ScrollTargetCenterRatio.y = 0.0f;
  window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void Gui::SetScrollX(float scroll_x) {
  Context &g = *GGui;
  SetScrollX(g.CurrentWindow, scroll_x);
}

void Gui::SetScrollY(float scroll_y) {
  Context &g = *GGui;
  SetScrollY(g.CurrentWindow, scroll_y);
}

// Note that a local position will vary depending on initial scroll value,
// This is a little bit confusing so bear with us:
//  - local_pos = (absolution_pos - window->Pos)
//  - So local_x/local_y are 0.0f for a position at the upper-left corner of a
//  window,
//    and generally local_x/local_y are >(padding+decoration) &&
//    <(size-padding-decoration) when in the visible area.
//  - They mostly exist because of legacy API.
// Following the rules above, when trying to work with scrolling code, consider
// that:
//  - SetScrollFromPosY(0.0f) == SetScrollY(0.0f + scroll.y) == has no effect!
//  - SetScrollFromPosY(-scroll.y) == SetScrollY(-scroll.y + scroll.y) ==
//  SetScrollY(0.0f) == reset scroll. Of course writing SetScrollY(0.0f)
//  directly then makes more sense
// We store a target position so centering and clamping can occur on the next
// frame when we are guaranteed to have a known window size
void Gui::SetScrollFromPosX(Window *window, float local_x,
                            float center_x_ratio) {
  ASSERT(center_x_ratio >= 0.0f && center_x_ratio <= 1.0f);
  window->ScrollTarget.x =
      TRUNC(local_x - window->DecoOuterSizeX1 - window->DecoInnerSizeX1 +
            window->Scroll.x); // Convert local position to scroll offset
  window->ScrollTargetCenterRatio.x = center_x_ratio;
  window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void Gui::SetScrollFromPosY(Window *window, float local_y,
                            float center_y_ratio) {
  ASSERT(center_y_ratio >= 0.0f && center_y_ratio <= 1.0f);
  window->ScrollTarget.y =
      TRUNC(local_y - window->DecoOuterSizeY1 - window->DecoInnerSizeY1 +
            window->Scroll.y); // Convert local position to scroll offset
  window->ScrollTargetCenterRatio.y = center_y_ratio;
  window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void Gui::SetScrollFromPosX(float local_x, float center_x_ratio) {
  Context &g = *GGui;
  SetScrollFromPosX(g.CurrentWindow, local_x, center_x_ratio);
}

void Gui::SetScrollFromPosY(float local_y, float center_y_ratio) {
  Context &g = *GGui;
  SetScrollFromPosY(g.CurrentWindow, local_y, center_y_ratio);
}

// center_x_ratio: 0.0f left of last item, 0.5f horizontal center of last
// item, 1.0f right of last item.
void Gui::SetScrollHereX(float center_x_ratio) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  float spacing_x = Max(window->WindowPadding.x, g.Style.ItemSpacing.x);
  float target_pos_x =
      Lerp(g.LastItemData.Rect.Min.x - spacing_x,
           g.LastItemData.Rect.Max.x + spacing_x, center_x_ratio);
  SetScrollFromPosX(window, target_pos_x - window->Pos.x,
                    center_x_ratio); // Convert from absolute to local pos

  // Tweak: snap on edges when aiming at an item very close to the edge
  window->ScrollTargetEdgeSnapDist.x =
      Max(0.0f, window->WindowPadding.x - spacing_x);
}

// center_y_ratio: 0.0f top of last item, 0.5f vertical center of last
// item, 1.0f bottom of last item.
void Gui::SetScrollHereY(float center_y_ratio) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  float spacing_y = Max(window->WindowPadding.y, g.Style.ItemSpacing.y);
  float target_pos_y = Lerp(window->DC.CursorPosPrevLine.y - spacing_y,
                            window->DC.CursorPosPrevLine.y +
                                window->DC.PrevLineSize.y + spacing_y,
                            center_y_ratio);
  SetScrollFromPosY(window, target_pos_y - window->Pos.y,
                    center_y_ratio); // Convert from absolute to local pos

  // Tweak: snap on edges when aiming at an item very close to the edge
  window->ScrollTargetEdgeSnapDist.y =
      Max(0.0f, window->WindowPadding.y - spacing_y);
}

//-----------------------------------------------------------------------------
// [SECTION] TOOLTIPS
//-----------------------------------------------------------------------------

bool Gui::BeginTooltip() {
  return BeginTooltipEx(TooltipFlags_None, WindowFlags_None);
}

bool Gui::BeginItemTooltip() {
  if (!IsItemHovered(HoveredFlags_ForTooltip))
    return false;
  return BeginTooltipEx(TooltipFlags_None, WindowFlags_None);
}

bool Gui::BeginTooltipEx(TooltipFlags tooltip_flags,
                         WindowFlags extra_window_flags) {
  Context &g = *GGui;

  if (g.DragDropWithinSource || g.DragDropWithinTarget) {
    // Drag and Drop tooltips are positioning differently than other tooltips:
    // - offset visibility to increase visibility around mouse.
    // - never clamp within outer viewport boundary.
    // We call SetNextWindowPos() to enforce position and disable clamping.
    // See FindBestWindowPosForPopup() for positionning logic of other tooltips
    // (not drag and drop ones).
    // Vec2 tooltip_pos = g.IO.MousePos - g.ActiveIdClickOffset -
    // g.Style.WindowPadding;
    Vec2 tooltip_pos =
        g.IO.MousePos + TOOLTIP_DEFAULT_OFFSET * g.Style.MouseCursorScale;
    SetNextWindowPos(tooltip_pos);
    SetNextWindowBgAlpha(g.Style.Colors[Col_PopupBg].w * 0.60f);
    // PushStyleVar(StyleVar_Alpha, g.Style.Alpha * 0.60f); // This would
    // be nice but e.g ColorButton with checkboard has issue with transparent
    // colors :(
    tooltip_flags |= TooltipFlags_OverridePrevious;
  }

  char window_name[16];
  FormatString(window_name, ARRAYSIZE(window_name), "##Tooltip_%02d",
               g.TooltipOverrideCount);
  if (tooltip_flags & TooltipFlags_OverridePrevious)
    if (Window *window = FindWindowByName(window_name))
      if (window->Active) {
        // Hide previous tooltip from being displayed. We can't easily "reset"
        // the content of a window so we create a new one.
        SetWindowHiddenAndSkipItemsForCurrentFrame(window);
        FormatString(window_name, ARRAYSIZE(window_name), "##Tooltip_%02d",
                     ++g.TooltipOverrideCount);
      }
  WindowFlags flags = WindowFlags_Tooltip | WindowFlags_NoInputs |
                      WindowFlags_NoTitleBar | WindowFlags_NoMove |
                      WindowFlags_NoResize | WindowFlags_NoSavedSettings |
                      WindowFlags_AlwaysAutoResize | WindowFlags_NoDocking;
  Begin(window_name, NULL, flags | extra_window_flags);
  // 2023-03-09: Added bool return value to the API, but currently always
  // returning true. If this ever returns false we need to update
  // BeginDragDropSource() accordingly.
  // if (!ret)
  //    End();
  // return ret;
  return true;
}

void Gui::EndTooltip() {
  ASSERT(GetCurrentWindowRead()->Flags &
         WindowFlags_Tooltip); // Mismatched BeginTooltip()/EndTooltip() calls
  End();
}

void Gui::SetTooltip(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  SetTooltipV(fmt, args);
  va_end(args);
}

void Gui::SetTooltipV(const char *fmt, va_list args) {
  if (!BeginTooltipEx(TooltipFlags_OverridePrevious, WindowFlags_None))
    return;
  TextV(fmt, args);
  EndTooltip();
}

// Shortcut to use 'style.HoverFlagsForTooltipMouse' or
// 'style.HoverFlagsForTooltipNav'. Defaults to == HoveredFlags_Stationary
// | HoveredFlags_DelayShort when using the mouse.
void Gui::SetItemTooltip(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (IsItemHovered(HoveredFlags_ForTooltip))
    SetTooltipV(fmt, args);
  va_end(args);
}

void Gui::SetItemTooltipV(const char *fmt, va_list args) {
  if (IsItemHovered(HoveredFlags_ForTooltip))
    SetTooltipV(fmt, args);
}

//-----------------------------------------------------------------------------
// [SECTION] POPUPS
//-----------------------------------------------------------------------------

// Supported flags: PopupFlags_AnyPopupId, PopupFlags_AnyPopupLevel
bool Gui::IsPopupOpen(ID id, PopupFlags popup_flags) {
  Context &g = *GGui;
  if (popup_flags & PopupFlags_AnyPopupId) {
    // Return true if any popup is open at the current BeginPopup() level of the
    // popup stack This may be used to e.g. test for another popups already
    // opened to handle popups priorities at the same level.
    ASSERT(id == 0);
    if (popup_flags & PopupFlags_AnyPopupLevel)
      return g.OpenPopupStack.Size > 0;
    else
      return g.OpenPopupStack.Size > g.BeginPopupStack.Size;
  } else {
    if (popup_flags & PopupFlags_AnyPopupLevel) {
      // Return true if the popup is open anywhere in the popup stack
      for (PopupData &popup_data : g.OpenPopupStack)
        if (popup_data.PopupId == id)
          return true;
      return false;
    } else {
      // Return true if the popup is open at the current BeginPopup() level of
      // the popup stack (this is the most-common query)
      return g.OpenPopupStack.Size > g.BeginPopupStack.Size &&
             g.OpenPopupStack[g.BeginPopupStack.Size].PopupId == id;
    }
  }
}

bool Gui::IsPopupOpen(const char *str_id, PopupFlags popup_flags) {
  Context &g = *GGui;
  ID id = (popup_flags & PopupFlags_AnyPopupId)
              ? 0
              : g.CurrentWindow->GetID(str_id);
  if ((popup_flags & PopupFlags_AnyPopupLevel) && id != 0)
    ASSERT(0 && "Cannot use IsPopupOpen() with a string id and "
                "PopupFlags_AnyPopupLevel."); // But non-string version is
                                              // legal and used internally
  return IsPopupOpen(id, popup_flags);
}

// Also see FindBlockingModal(NULL)
Window *Gui::GetTopMostPopupModal() {
  Context &g = *GGui;
  for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
    if (Window *popup = g.OpenPopupStack.Data[n].Window)
      if (popup->Flags & WindowFlags_Modal)
        return popup;
  return NULL;
}

// See Demo->Stacked Modal to confirm what this is for.
Window *Gui::GetTopMostAndVisiblePopupModal() {
  Context &g = *GGui;
  for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
    if (Window *popup = g.OpenPopupStack.Data[n].Window)
      if ((popup->Flags & WindowFlags_Modal) && IsWindowActiveAndVisible(popup))
        return popup;
  return NULL;
}

void Gui::OpenPopup(const char *str_id, PopupFlags popup_flags) {
  Context &g = *GGui;
  ID id = g.CurrentWindow->GetID(str_id);
  DEBUG_LOG_POPUP("[popup] OpenPopup(\"%s\" -> 0x%08X)\n", str_id, id);
  OpenPopupEx(id, popup_flags);
}

void Gui::OpenPopup(ID id, PopupFlags popup_flags) {
  OpenPopupEx(id, popup_flags);
}

// Mark popup as open (toggle toward open state).
// Popups are closed when user click outside, or activate a pressable item, or
// CloseCurrentPopup() is called within a BeginPopup()/EndPopup() block. Popup
// identifiers are relative to the current ID-stack (so OpenPopup and BeginPopup
// needs to be at the same level). One open popup per level of the popup
// hierarchy (NB: when assigning we reset the Window member of PopupRef to
// NULL)
void Gui::OpenPopupEx(ID id, PopupFlags popup_flags) {
  Context &g = *GGui;
  Window *parent_window = g.CurrentWindow;
  const int current_stack_size = g.BeginPopupStack.Size;

  if (popup_flags & PopupFlags_NoOpenOverExistingPopup)
    if (IsPopupOpen((ID)0, PopupFlags_AnyPopupId))
      return;

  PopupData popup_ref; // Tagged as new ref as Window will be set back to
                       // NULL if we write this into OpenPopupStack.
  popup_ref.PopupId = id;
  popup_ref.Window = NULL;
  popup_ref.BackupNavWindow =
      g.NavWindow; // When popup closes focus may be restored to NavWindow
                   // (depend on window type).
  popup_ref.OpenFrameCount = g.FrameCount;
  popup_ref.OpenParentId = parent_window->IDStack.back();
  popup_ref.OpenPopupPos = NavCalcPreferredRefPos();
  popup_ref.OpenMousePos =
      IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : popup_ref.OpenPopupPos;

  DEBUG_LOG_POPUP("[popup] OpenPopupEx(0x%08X)\n", id);
  if (g.OpenPopupStack.Size < current_stack_size + 1) {
    g.OpenPopupStack.push_back(popup_ref);
  } else {
    // Gently handle the user mistakenly calling OpenPopup() every frame. It is
    // a programming mistake! However, if we were to run the regular code path,
    // the ui would become completely unusable because the popup will always be
    // in hidden-while-calculating-size state _while_ claiming focus. Which
    // would be a very confusing situation for the programmer. Instead, we
    // silently allow the popup to proceed, it will keep reappearing and the
    // programming error will be more obvious to understand.
    if (g.OpenPopupStack[current_stack_size].PopupId == id &&
        g.OpenPopupStack[current_stack_size].OpenFrameCount ==
            g.FrameCount - 1) {
      g.OpenPopupStack[current_stack_size].OpenFrameCount =
          popup_ref.OpenFrameCount;
    } else {
      // Close child popups if any, then flag popup for open/reopen
      ClosePopupToLevel(current_stack_size, false);
      g.OpenPopupStack.push_back(popup_ref);
    }

    // When reopening a popup we first refocus its parent, otherwise if its
    // parent is itself a popup it would get closed by ClosePopupsOverWindow().
    // This is equivalent to what ClosePopupToLevel() does.
    // if (g.OpenPopupStack[current_stack_size].PopupId == id)
    //    FocusWindow(parent_window);
  }
}

// When popups are stacked, clicking on a lower level popups puts focus back to
// it and close popups above it. This function closes any popups that are over
// 'ref_window'.
void Gui::ClosePopupsOverWindow(Window *ref_window,
                                bool restore_focus_to_window_under_popup) {
  Context &g = *GGui;
  if (g.OpenPopupStack.Size == 0)
    return;

  // Don't close our own child popup windows.
  int popup_count_to_keep = 0;
  if (ref_window) {
    // Find the highest popup which is a descendant of the reference window
    // (generally reference window = NavWindow)
    for (; popup_count_to_keep < g.OpenPopupStack.Size; popup_count_to_keep++) {
      PopupData &popup = g.OpenPopupStack[popup_count_to_keep];
      if (!popup.Window)
        continue;
      ASSERT((popup.Window->Flags & WindowFlags_Popup) != 0);
      if (popup.Window->Flags & WindowFlags_ChildWindow)
        continue;

      // Trim the stack unless the popup is a direct parent of the reference
      // window (the reference window is often the NavWindow)
      // - With this stack of window, clicking/focusing Popup1 will close Popup2
      // and Popup3:
      //     Window -> Popup1 -> Popup2 -> Popup3
      // - Each popups may contain child windows, which is why we compare
      // ->RootWindowDockTree!
      //     Window -> Popup1 -> Popup1_Child -> Popup2 -> Popup2_Child
      bool ref_window_is_descendent_of_popup = false;
      for (int n = popup_count_to_keep; n < g.OpenPopupStack.Size; n++)
        if (Window *popup_window = g.OpenPopupStack[n].Window)
          // if (popup_window->RootWindowDockTree ==
          // ref_window->RootWindowDockTree) // FIXME-MERGE
          if (IsWindowWithinBeginStackOf(ref_window, popup_window)) {
            ref_window_is_descendent_of_popup = true;
            break;
          }
      if (!ref_window_is_descendent_of_popup)
        break;
    }
  }
  if (popup_count_to_keep <
      g.OpenPopupStack.Size) // This test is not required but it allows to set a
                             // convenient breakpoint on the statement below
  {
    DEBUG_LOG_POPUP("[popup] ClosePopupsOverWindow(\"%s\")\n",
                    ref_window ? ref_window->Name : "<NULL>");
    ClosePopupToLevel(popup_count_to_keep, restore_focus_to_window_under_popup);
  }
}

void Gui::ClosePopupsExceptModals() {
  Context &g = *GGui;

  int popup_count_to_keep;
  for (popup_count_to_keep = g.OpenPopupStack.Size; popup_count_to_keep > 0;
       popup_count_to_keep--) {
    Window *window = g.OpenPopupStack[popup_count_to_keep - 1].Window;
    if (!window || (window->Flags & WindowFlags_Modal))
      break;
  }
  if (popup_count_to_keep <
      g.OpenPopupStack.Size) // This test is not required but it allows to set a
                             // convenient breakpoint on the statement below
    ClosePopupToLevel(popup_count_to_keep, true);
}

void Gui::ClosePopupToLevel(int remaining,
                            bool restore_focus_to_window_under_popup) {
  Context &g = *GGui;
  DEBUG_LOG_POPUP(
      "[popup] ClosePopupToLevel(%d), restore_focus_to_window_under_popup=%d\n",
      remaining, restore_focus_to_window_under_popup);
  ASSERT(remaining >= 0 && remaining < g.OpenPopupStack.Size);

  // Trim open popup stack
  Window *popup_window = g.OpenPopupStack[remaining].Window;
  Window *popup_backup_nav_window = g.OpenPopupStack[remaining].BackupNavWindow;
  g.OpenPopupStack.resize(remaining);

  if (restore_focus_to_window_under_popup) {
    Window *focus_window =
        (popup_window && popup_window->Flags & WindowFlags_ChildMenu)
            ? popup_window->ParentWindow
            : popup_backup_nav_window;
    if (focus_window && !focus_window->WasActive && popup_window)
      FocusTopMostWindowUnderOne(
          popup_window, NULL, NULL,
          FocusRequestFlags_RestoreFocusedChild); // Fallback
    else
      FocusWindow(focus_window, (g.NavLayer == NavLayer_Main)
                                    ? FocusRequestFlags_RestoreFocusedChild
                                    : FocusRequestFlags_None);
  }
}

// Close the popup we have begin-ed into.
void Gui::CloseCurrentPopup() {
  Context &g = *GGui;
  int popup_idx = g.BeginPopupStack.Size - 1;
  if (popup_idx < 0 || popup_idx >= g.OpenPopupStack.Size ||
      g.BeginPopupStack[popup_idx].PopupId !=
          g.OpenPopupStack[popup_idx].PopupId)
    return;

  // Closing a menu closes its top-most parent popup (unless a modal)
  while (popup_idx > 0) {
    Window *popup_window = g.OpenPopupStack[popup_idx].Window;
    Window *parent_popup_window = g.OpenPopupStack[popup_idx - 1].Window;
    bool close_parent = false;
    if (popup_window && (popup_window->Flags & WindowFlags_ChildMenu))
      if (parent_popup_window &&
          !(parent_popup_window->Flags & WindowFlags_MenuBar))
        close_parent = true;
    if (!close_parent)
      break;
    popup_idx--;
  }
  DEBUG_LOG_POPUP("[popup] CloseCurrentPopup %d -> %d\n",
                  g.BeginPopupStack.Size - 1, popup_idx);
  ClosePopupToLevel(popup_idx, true);

  // A common pattern is to close a popup when selecting a menu item/selectable
  // that will open another window. To improve this usage pattern, we avoid nav
  // highlight for a single frame in the parent window. Similarly, we could
  // avoid mouse hover highlight in this window but it is less visually
  // problematic.
  if (Window *window = g.NavWindow)
    window->DC.NavHideHighlightOneFrame = true;
}

// Attention! BeginPopup() adds default flags which BeginPopupEx()!
bool Gui::BeginPopupEx(ID id, WindowFlags flags) {
  Context &g = *GGui;
  if (!IsPopupOpen(id, PopupFlags_None)) {
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume
                                   // those values
    return false;
  }

  char name[20];
  if (flags & WindowFlags_ChildMenu)
    FormatString(name, ARRAYSIZE(name), "##Menu_%02d",
                 g.BeginMenuCount); // Recycle windows based on depth
  else
    FormatString(
        name, ARRAYSIZE(name), "##Popup_%08x",
        id); // Not recycling, so we can close/open during the same frame

  flags |= WindowFlags_Popup | WindowFlags_NoDocking;
  bool is_open = Begin(name, NULL, flags);
  if (!is_open) // NB: Begin can return false when the popup is completely
                // clipped (e.g. zero size display)
    EndPopup();

  return is_open;
}

bool Gui::BeginPopup(const char *str_id, WindowFlags flags) {
  Context &g = *GGui;
  if (g.OpenPopupStack.Size <=
      g.BeginPopupStack.Size) // Early out for performance
  {
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume
                                   // those values
    return false;
  }
  flags |= WindowFlags_AlwaysAutoResize | WindowFlags_NoTitleBar |
           WindowFlags_NoSavedSettings;
  ID id = g.CurrentWindow->GetID(str_id);
  return BeginPopupEx(id, flags);
}

// If 'p_open' is specified for a modal popup window, the popup will have a
// regular close button which will close the popup. Note that popup visibility
// status is owned by Gui (and manipulated with e.g. OpenPopup).
// - *p_open set back to false in BeginPopupModal() when popup is not open.
// - if you set *p_open to false before calling BeginPopupModal(), it will close
// the popup.
bool Gui::BeginPopupModal(const char *name, bool *p_open, WindowFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  const ID id = window->GetID(name);
  if (!IsPopupOpen(id, PopupFlags_None)) {
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume
                                   // those values
    if (p_open && *p_open)
      *p_open = false;
    return false;
  }

  // Center modal windows by default for increased visibility
  // (this won't really last as settings will kick in, and is mostly for
  // backward compatibility. user may do the same themselves)
  // FIXME: Should test for (PosCond & window->SetWindowPosAllowFlags) with the
  // upcoming window.
  if ((g.NextWindowData.Flags & NextWindowDataFlags_HasPos) == 0) {
    const Viewport *viewport =
        window->WasActive ? window->Viewport
                          : GetMainViewport(); // FIXME-VIEWPORT: What may be
                                               // our reference viewport?
    SetNextWindowPos(viewport->GetCenter(), Cond_FirstUseEver,
                     Vec2(0.5f, 0.5f));
  }

  flags |= WindowFlags_Popup | WindowFlags_Modal | WindowFlags_NoCollapse |
           WindowFlags_NoDocking;
  const bool is_open = Begin(name, p_open, flags);
  if (!is_open ||
      (p_open && !*p_open)) // NB: is_open can be 'false' when the popup is
                            // completely clipped (e.g. zero size display)
  {
    EndPopup();
    if (is_open)
      ClosePopupToLevel(g.BeginPopupStack.Size, true);
    return false;
  }
  return is_open;
}

void Gui::EndPopup() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT(window->Flags &
         WindowFlags_Popup); // Mismatched BeginPopup()/EndPopup() calls
  ASSERT(g.BeginPopupStack.Size > 0);

  // Make all menus and popups wrap around for now, may need to expose that
  // policy (e.g. focus scope could include wrap/loop policy flags used by new
  // move requests)
  if (g.NavWindow == window)
    NavMoveRequestTryWrapping(window, NavMoveFlags_LoopY);

  // Child-popups don't need to be laid out
  ASSERT(g.WithinEndChild == false);
  if (window->Flags & WindowFlags_ChildWindow)
    g.WithinEndChild = true;
  End();
  g.WithinEndChild = false;
}

// Helper to open a popup if mouse button is released over the item
// - This is essentially the same as BeginPopupContextItem() but without the
// trailing BeginPopup()
void Gui::OpenPopupOnItemClick(const char *str_id, PopupFlags popup_flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  int mouse_button = (popup_flags & PopupFlags_MouseButtonMask_);
  if (IsMouseReleased(mouse_button) &&
      IsItemHovered(HoveredFlags_AllowWhenBlockedByPopup)) {
    ID id = str_id ? window->GetID(str_id)
                   : g.LastItemData.ID; // If user hasn't passed an ID, we can
                                        // use the LastItemID. Using LastItemID
                                        // as a Popup ID won't conflict!
    ASSERT(id != 0); // You cannot pass a NULL str_id if the last item has no
                     // identifier (e.g. a Text() item)
    OpenPopupEx(id, popup_flags);
  }
}

// This is a helper to handle the simplest case of associating one named popup
// to one given widget.
// - To create a popup associated to the last item, you generally want to pass a
// NULL value to str_id.
// - To create a popup with a specific identifier, pass it in str_id.
//    - This is useful when using using BeginPopupContextItem() on an item which
//    doesn't have an identifier, e.g. a Text() call.
//    - This is useful when multiple code locations may want to manipulate/open
//    the same popup, given an explicit id.
// - You may want to handle the whole on user side if you have specific needs
// (e.g. tweaking IsItemHovered() parameters).
//   This is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       OpenPopupOnItemClick(str_id, PopupFlags_MouseButtonRight);
//       return BeginPopup(id);
//   Which is essentially the same as:
//       id = str_id ? GetID(str_id) : GetItemID();
//       if (IsItemHovered() && IsMouseReleased(MouseButton_Right))
//           OpenPopup(id);
//       return BeginPopup(id);
//   The main difference being that this is tweaked to avoid computing the ID
//   twice.
bool Gui::BeginPopupContextItem(const char *str_id, PopupFlags popup_flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;
  ID id =
      str_id
          ? window->GetID(str_id)
          : g.LastItemData
                .ID; // If user hasn't passed an ID, we can use the LastItemID.
                     // Using LastItemID as a Popup ID won't conflict!
  ASSERT(id != 0);   // You cannot pass a NULL str_id if the last item has no
                     // identifier (e.g. a Text() item)
  int mouse_button = (popup_flags & PopupFlags_MouseButtonMask_);
  if (IsMouseReleased(mouse_button) &&
      IsItemHovered(HoveredFlags_AllowWhenBlockedByPopup))
    OpenPopupEx(id, popup_flags);
  return BeginPopupEx(id, WindowFlags_AlwaysAutoResize |
                              WindowFlags_NoTitleBar |
                              WindowFlags_NoSavedSettings);
}

bool Gui::BeginPopupContextWindow(const char *str_id, PopupFlags popup_flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (!str_id)
    str_id = "window_context";
  ID id = window->GetID(str_id);
  int mouse_button = (popup_flags & PopupFlags_MouseButtonMask_);
  if (IsMouseReleased(mouse_button) &&
      IsWindowHovered(HoveredFlags_AllowWhenBlockedByPopup))
    if (!(popup_flags & PopupFlags_NoOpenOverItems) || !IsAnyItemHovered())
      OpenPopupEx(id, popup_flags);
  return BeginPopupEx(id, WindowFlags_AlwaysAutoResize |
                              WindowFlags_NoTitleBar |
                              WindowFlags_NoSavedSettings);
}

bool Gui::BeginPopupContextVoid(const char *str_id, PopupFlags popup_flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (!str_id)
    str_id = "void_context";
  ID id = window->GetID(str_id);
  int mouse_button = (popup_flags & PopupFlags_MouseButtonMask_);
  if (IsMouseReleased(mouse_button) && !IsWindowHovered(HoveredFlags_AnyWindow))
    if (GetTopMostPopupModal() == NULL)
      OpenPopupEx(id, popup_flags);
  return BeginPopupEx(id, WindowFlags_AlwaysAutoResize |
                              WindowFlags_NoTitleBar |
                              WindowFlags_NoSavedSettings);
}

// r_avoid = the rectangle to avoid (e.g. for tooltip it is a rectangle around
// the mouse cursor which we want to avoid. for popups it's a small point around
// the cursor.) r_outer = the visible area rectangle, minus safe area padding.
// If our popup size won't fit because of safe area padding we ignore it.
// (r_outer is usually equivalent to the viewport rectangle minus padding, but
// when multi-viewports are enabled and monitor
//  information are available, it may represent the entire platform monitor from
//  the frame of reference of the current viewport. this allows us to have
//  tooltips/popups displayed out of the parent viewport.)
Vec2 Gui::FindBestWindowPosForPopupEx(const Vec2 &ref_pos, const Vec2 &size,
                                      Dir *last_dir, const Rect &r_outer,
                                      const Rect &r_avoid,
                                      PopupPositionPolicy policy) {
  Vec2 base_pos_clamped = Clamp(ref_pos, r_outer.Min, r_outer.Max - size);
  // GetForegroundDrawList()->AddRect(r_avoid.Min, r_avoid.Max,
  // COL32(255,0,0,255)); GetForegroundDrawList()->AddRect(r_outer.Min,
  // r_outer.Max, COL32(0,255,0,255));

  // Combo Box policy (we want a connecting edge)
  if (policy == PopupPositionPolicy_ComboBox) {
    const Dir dir_prefered_order[Dir_COUNT] = {Dir_Down, Dir_Right, Dir_Left,
                                               Dir_Up};
    for (int n = (*last_dir != Dir_None) ? -1 : 0; n < Dir_COUNT; n++) {
      const Dir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
      if (n != -1 && dir == *last_dir) // Already tried this direction?
        continue;
      Vec2 pos;
      if (dir == Dir_Down)
        pos = Vec2(r_avoid.Min.x,
                   r_avoid.Max.y); // Below, Toward Right (default)
      if (dir == Dir_Right)
        pos = Vec2(r_avoid.Min.x,
                   r_avoid.Min.y - size.y); // Above, Toward Right
      if (dir == Dir_Left)
        pos = Vec2(r_avoid.Max.x - size.x, r_avoid.Max.y); // Below, Toward Left
      if (dir == Dir_Up)
        pos = Vec2(r_avoid.Max.x - size.x,
                   r_avoid.Min.y - size.y); // Above, Toward Left
      if (!r_outer.Contains(Rect(pos, pos + size)))
        continue;
      *last_dir = dir;
      return pos;
    }
  }

  // Tooltip and Default popup policy
  // (Always first try the direction we used on the last frame, if any)
  if (policy == PopupPositionPolicy_Tooltip ||
      policy == PopupPositionPolicy_Default) {
    const Dir dir_prefered_order[Dir_COUNT] = {Dir_Right, Dir_Down, Dir_Up,
                                               Dir_Left};
    for (int n = (*last_dir != Dir_None) ? -1 : 0; n < Dir_COUNT; n++) {
      const Dir dir = (n == -1) ? *last_dir : dir_prefered_order[n];
      if (n != -1 && dir == *last_dir) // Already tried this direction?
        continue;

      const float avail_w = (dir == Dir_Left ? r_avoid.Min.x : r_outer.Max.x) -
                            (dir == Dir_Right ? r_avoid.Max.x : r_outer.Min.x);
      const float avail_h = (dir == Dir_Up ? r_avoid.Min.y : r_outer.Max.y) -
                            (dir == Dir_Down ? r_avoid.Max.y : r_outer.Min.y);

      // If there's not enough room on one axis, there's no point in positioning
      // on a side on this axis (e.g. when not enough width, use a top/bottom
      // position to maximize available width)
      if (avail_w < size.x && (dir == Dir_Left || dir == Dir_Right))
        continue;
      if (avail_h < size.y && (dir == Dir_Up || dir == Dir_Down))
        continue;

      Vec2 pos;
      pos.x = (dir == Dir_Left)    ? r_avoid.Min.x - size.x
              : (dir == Dir_Right) ? r_avoid.Max.x
                                   : base_pos_clamped.x;
      pos.y = (dir == Dir_Up)     ? r_avoid.Min.y - size.y
              : (dir == Dir_Down) ? r_avoid.Max.y
                                  : base_pos_clamped.y;

      // Clamp top-left corner of popup
      pos.x = Max(pos.x, r_outer.Min.x);
      pos.y = Max(pos.y, r_outer.Min.y);

      *last_dir = dir;
      return pos;
    }
  }

  // Fallback when not enough room:
  *last_dir = Dir_None;

  // For tooltip we prefer avoiding the cursor at all cost even if it means that
  // part of the tooltip won't be visible.
  if (policy == PopupPositionPolicy_Tooltip)
    return ref_pos + Vec2(2, 2);

  // Otherwise try to keep within display
  Vec2 pos = ref_pos;
  pos.x = Max(Min(pos.x + size.x, r_outer.Max.x) - size.x, r_outer.Min.x);
  pos.y = Max(Min(pos.y + size.y, r_outer.Max.y) - size.y, r_outer.Min.y);
  return pos;
}

// Note that this is used for popups, which can overlap the non work-area of
// individual viewports.
Rect Gui::GetPopupAllowedExtentRect(Window *window) {
  Context &g = *GGui;
  Rect r_screen;
  if (window->ViewportAllowPlatformMonitorExtend >= 0) {
    // Extent with be in the frame of reference of the given viewport (so Min is
    // likely to be negative here)
    const PlatformMonitor &monitor =
        g.PlatformIO.Monitors[window->ViewportAllowPlatformMonitorExtend];
    r_screen.Min = monitor.WorkPos;
    r_screen.Max = monitor.WorkPos + monitor.WorkSize;
  } else {
    // Use the full viewport area (not work area) for popups
    r_screen = window->Viewport->GetMainRect();
  }
  Vec2 padding = g.Style.DisplaySafeAreaPadding;
  r_screen.Expand(
      Vec2((r_screen.GetWidth() > padding.x * 2) ? -padding.x : 0.0f,
           (r_screen.GetHeight() > padding.y * 2) ? -padding.y : 0.0f));
  return r_screen;
}

Vec2 Gui::FindBestWindowPosForPopup(Window *window) {
  Context &g = *GGui;

  Rect r_outer = GetPopupAllowedExtentRect(window);
  if (window->Flags & WindowFlags_ChildMenu) {
    // Child menus typically request _any_ position within the parent menu item,
    // and then we move the new menu outside the parent bounds. This is how we
    // end up with child menus appearing (most-commonly) on the right of the
    // parent menu.
    Window *parent_window = window->ParentWindow;
    float horizontal_overlap =
        g.Style.ItemInnerSpacing
            .x; // We want some overlap to convey the relative depth of each
                // menu (currently the amount of overlap is hard-coded to
                // style.ItemSpacing.x).
    Rect r_avoid;
    if (parent_window->DC.MenuBarAppending)
      r_avoid = Rect(
          -FLT_MAX, parent_window->ClipRect.Min.y, FLT_MAX,
          parent_window->ClipRect.Max
              .y); // Avoid parent menu-bar. If we wanted multi-line menu-bar,
                   // we may instead want to have the calling window setup e.g.
                   // a NextWindowData.PosConstraintAvoidRect field
    else
      r_avoid = Rect(parent_window->Pos.x + horizontal_overlap, -FLT_MAX,
                     parent_window->Pos.x + parent_window->Size.x -
                         horizontal_overlap - parent_window->ScrollbarSizes.x,
                     FLT_MAX);
    return FindBestWindowPosForPopupEx(window->Pos, window->Size,
                                       &window->AutoPosLastDirection, r_outer,
                                       r_avoid, PopupPositionPolicy_Default);
  }
  if (window->Flags & WindowFlags_Popup) {
    return FindBestWindowPosForPopupEx(
        window->Pos, window->Size, &window->AutoPosLastDirection, r_outer,
        Rect(window->Pos, window->Pos),
        PopupPositionPolicy_Default); // Ideally we'd disable r_avoid here
  }
  if (window->Flags & WindowFlags_Tooltip) {
    // Position tooltip (always follows mouse + clamp within outer boundaries)
    // Note that drag and drop tooltips are NOT using this path:
    // BeginTooltipEx() manually sets their position. In theory we could handle
    // both cases in same location, but requires a bit of shuffling as drag and
    // drop tooltips are calling SetWindowPos() leading to
    // 'window_pos_set_by_api' being set in Begin()
    ASSERT(g.CurrentWindow == window);
    const float scale = g.Style.MouseCursorScale;
    const Vec2 ref_pos = NavCalcPreferredRefPos();
    const Vec2 tooltip_pos = ref_pos + TOOLTIP_DEFAULT_OFFSET * scale;
    Rect r_avoid;
    if (!g.NavDisableHighlight && g.NavDisableMouseHover &&
        !(g.IO.ConfigFlags & ConfigFlags_NavEnableSetMousePos))
      r_avoid =
          Rect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 16, ref_pos.y + 8);
    else
      r_avoid = Rect(
          ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 24 * scale,
          ref_pos.y +
              24 * scale); // FIXME: Hard-coded based on mouse cursor shape
                           // expectation. Exact dimension not very important.
    // GetForegroundDrawList()->AddRect(r_avoid.Min, r_avoid.Max, COL32(255,
    // 0, 255, 255));
    return FindBestWindowPosForPopupEx(tooltip_pos, window->Size,
                                       &window->AutoPosLastDirection, r_outer,
                                       r_avoid, PopupPositionPolicy_Tooltip);
  }
  ASSERT(0);
  return window->Pos;
}

//-----------------------------------------------------------------------------
// [SECTION] KEYBOARD/GAMEPAD NAVIGATION
//-----------------------------------------------------------------------------

// FIXME-NAV: The existence of SetNavID vs SetFocusID vs FocusWindow() needs to
// be clarified/reworked. In our terminology those should be interchangeable,
// yet right now this is super confusing. Those two functions are merely a
// legacy artifact, so at minimum naming should be clarified.

void Gui::SetNavWindow(Window *window) {
  Context &g = *GGui;
  if (g.NavWindow != window) {
    DEBUG_LOG_FOCUS("[focus] SetNavWindow(\"%s\")\n",
                    window ? window->Name : "<NULL>");
    g.NavWindow = window;
    g.NavLastValidSelectionUserData = SelectionUserData_Invalid;
  }
  g.NavInitRequest = g.NavMoveSubmitted = g.NavMoveScoringItems = false;
  NavUpdateAnyRequestFlag();
}

void Gui::NavClearPreferredPosForAxis(Axis axis) {
  Context &g = *GGui;
  g.NavWindow->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer][axis] =
      FLT_MAX;
}

void Gui::SetNavID(ID id, NavLayer nav_layer, ID focus_scope_id,
                   const Rect &rect_rel) {
  Context &g = *GGui;
  ASSERT(g.NavWindow != NULL);
  ASSERT(nav_layer == NavLayer_Main || nav_layer == NavLayer_Menu);
  g.NavId = id;
  g.NavLayer = nav_layer;
  g.NavFocusScopeId = focus_scope_id;
  g.NavWindow->NavLastIds[nav_layer] = id;
  g.NavWindow->NavRectRel[nav_layer] = rect_rel;

  // Clear preferred scoring position (NavMoveRequestApplyResult() will tend to
  // restore it)
  NavClearPreferredPosForAxis(Axis_X);
  NavClearPreferredPosForAxis(Axis_Y);
}

void Gui::SetFocusID(ID id, Window *window) {
  Context &g = *GGui;
  ASSERT(id != 0);

  if (g.NavWindow != window)
    SetNavWindow(window);

  // Assume that SetFocusID() is called in the context where its
  // window->DC.NavLayerCurrent and g.CurrentFocusScopeId are valid. Note that
  // window may be != g.CurrentWindow (e.g. SetFocusID call in InputTextEx for
  // multi-line text)
  const NavLayer nav_layer = window->DC.NavLayerCurrent;
  g.NavId = id;
  g.NavLayer = nav_layer;
  g.NavFocusScopeId = g.CurrentFocusScopeId;
  window->NavLastIds[nav_layer] = id;
  if (g.LastItemData.ID == id)
    window->NavRectRel[nav_layer] =
        WindowRectAbsToRel(window, g.LastItemData.NavRect);

  if (g.ActiveIdSource == InputSource_Keyboard ||
      g.ActiveIdSource == InputSource_Gamepad)
    g.NavDisableMouseHover = true;
  else
    g.NavDisableHighlight = true;

  // Clear preferred scoring position (NavMoveRequestApplyResult() will tend to
  // restore it)
  NavClearPreferredPosForAxis(Axis_X);
  NavClearPreferredPosForAxis(Axis_Y);
}

static Dir GetDirQuadrantFromDelta(float dx, float dy) {
  if (Fabs(dx) > Fabs(dy))
    return (dx > 0.0f) ? Dir_Right : Dir_Left;
  return (dy > 0.0f) ? Dir_Down : Dir_Up;
}

static float inline NavScoreItemDistInterval(float cand_min, float cand_max,
                                             float curr_min, float curr_max) {
  if (cand_max < curr_min)
    return cand_max - curr_min;
  if (curr_max < cand_min)
    return cand_min - curr_max;
  return 0.0f;
}

// Scoring function for gamepad/keyboard directional navigation. Based on
// https://gist.github.com/rygorous/6981057
static bool Gui::NavScoreItem(NavItemData *result) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (g.NavLayer != window->DC.NavLayerCurrent)
    return false;

  // FIXME: Those are not good variables names
  Rect cand = g.LastItemData.NavRect; // Current item nav rectangle
  const Rect curr =
      g.NavScoringRect; // Current modified source rect (NB: we've applied Max.x
                        // = Min.x in NavUpdate() to inhibit the effect of
                        // having varied item width)
  g.NavScoringDebugCount++;

  // When entering through a NavFlattened border, we consider child window items
  // as fully clipped for scoring
  if (window->ParentWindow == g.NavWindow) {
    ASSERT((window->Flags | g.NavWindow->Flags) & WindowFlags_NavFlattened);
    if (!window->ClipRect.Overlaps(cand))
      return false;
    cand.ClipWithFull(
        window->ClipRect); // This allows the scored item to not overlap other
                           // candidates in the parent window
  }

  // Compute distance between boxes
  // FIXME-NAV: Introducing biases for vertical navigation, needs to be removed.
  float dbx =
      NavScoreItemDistInterval(cand.Min.x, cand.Max.x, curr.Min.x, curr.Max.x);
  float dby = NavScoreItemDistInterval(
      Lerp(cand.Min.y, cand.Max.y, 0.2f), Lerp(cand.Min.y, cand.Max.y, 0.8f),
      Lerp(curr.Min.y, curr.Max.y, 0.2f),
      Lerp(curr.Min.y, curr.Max.y,
           0.8f)); // Scale down on Y to keep using box-distance for
                   // vertically touching items
  if (dby != 0.0f && dbx != 0.0f)
    dbx = (dbx / 1000.0f) + ((dbx > 0.0f) ? +1.0f : -1.0f);
  float dist_box = Fabs(dbx) + Fabs(dby);

  // Compute distance between centers (this is off by a factor of 2, but we only
  // compare center distances with each other so it doesn't matter)
  float dcx = (cand.Min.x + cand.Max.x) - (curr.Min.x + curr.Max.x);
  float dcy = (cand.Min.y + cand.Max.y) - (curr.Min.y + curr.Max.y);
  float dist_center =
      Fabs(dcx) +
      Fabs(dcy); // L1 metric (need this for our connectedness guarantee)

  // Determine which quadrant of 'curr' our candidate item 'cand' lies in based
  // on distance
  Dir quadrant;
  float dax = 0.0f, day = 0.0f, dist_axial = 0.0f;
  if (dbx != 0.0f || dby != 0.0f) {
    // For non-overlapping boxes, use distance between boxes
    dax = dbx;
    day = dby;
    dist_axial = dist_box;
    quadrant = GetDirQuadrantFromDelta(dbx, dby);
  } else if (dcx != 0.0f || dcy != 0.0f) {
    // For overlapping boxes with different centers, use distance between
    // centers
    dax = dcx;
    day = dcy;
    dist_axial = dist_center;
    quadrant = GetDirQuadrantFromDelta(dcx, dcy);
  } else {
    // Degenerate case: two overlapping buttons with same center, break ties
    // arbitrarily (note that LastItemId here is really the _previous_ item
    // order, but it doesn't matter)
    quadrant = (g.LastItemData.ID < g.NavId) ? Dir_Left : Dir_Right;
  }

  const Dir move_dir = g.NavMoveDir;
#if DEBUG_NAV_SCORING
  char buf[200];
  if (g.IO.KeyCtrl) // Hold CTRL to preview score in matching quadrant.
                    // CTRL+Arrow to rotate.
  {
    if (quadrant == move_dir) {
      FormatString(buf, ARRAYSIZE(buf), "%.0f/%.0f", dist_box, dist_center);
      DrawList *draw_list = GetForegroundDrawList(window);
      draw_list->AddRectFilled(cand.Min, cand.Max, COL32(255, 0, 0, 80));
      draw_list->AddRectFilled(cand.Min, cand.Min + CalcTextSize(buf),
                               COL32(255, 0, 0, 200));
      draw_list->AddText(cand.Min, COL32(255, 255, 255, 255), buf);
    }
  }
  const bool debug_hovering = IsMouseHoveringRect(cand.Min, cand.Max);
  const bool debug_tty = (g.IO.KeyCtrl && IsKeyPressed(Key_Space));
  if (debug_hovering || debug_tty) {
    FormatString(buf, ARRAYSIZE(buf),
                 "d-box    (%7.3f,%7.3f) -> %7.3f\nd-center (%7.3f,%7.3f) -> "
                 "%7.3f\nd-axial  (%7.3f,%7.3f) -> %7.3f\nnav %c, quadrant %c",
                 dbx, dby, dist_box, dcx, dcy, dist_center, dax, day,
                 dist_axial, "-WENS"[move_dir + 1], "-WENS"[quadrant + 1]);
    if (debug_hovering) {
      DrawList *draw_list = GetForegroundDrawList(window);
      draw_list->AddRect(curr.Min, curr.Max, COL32(255, 200, 0, 100));
      draw_list->AddRect(cand.Min, cand.Max, COL32(255, 255, 0, 200));
      draw_list->AddRectFilled(cand.Max - Vec2(4, 4),
                               cand.Max + CalcTextSize(buf) + Vec2(4, 4),
                               COL32(40, 0, 0, 200));
      draw_list->AddText(cand.Max, ~0U, buf);
    }
    if (debug_tty) {
      DEBUG_LOG_NAV("id 0x%08X\n%s\n", g.LastItemData.ID, buf);
    }
  }
#endif

  // Is it in the quadrant we're interested in moving to?
  bool new_best = false;
  if (quadrant == move_dir) {
    // Does it beat the current best candidate?
    if (dist_box < result->DistBox) {
      result->DistBox = dist_box;
      result->DistCenter = dist_center;
      return true;
    }
    if (dist_box == result->DistBox) {
      // Try using distance between center points to break ties
      if (dist_center < result->DistCenter) {
        result->DistCenter = dist_center;
        new_best = true;
      } else if (dist_center == result->DistCenter) {
        // Still tied! we need to be extra-careful to make sure everything gets
        // linked properly. We consistently break ties by symbolically moving
        // "later" items (with higher index) to the right/downwards by an
        // infinitesimal amount since we the current "best" button already (so
        // it must have a lower index), this is fairly easy. This rule ensures
        // that all buttons with dx==dy==0 will end up being linked in order of
        // appearance along the x axis.
        if (((move_dir == Dir_Up || move_dir == Dir_Down) ? dby : dbx) <
            0.0f) // moving bj to the right/down decreases distance
          new_best = true;
      }
    }
  }

  // Axial check: if 'curr' has no link at all in some direction and 'cand' lies
  // roughly in that direction, add a tentative link. This will only be kept if
  // no "real" matches are found, so it only augments the graph produced by the
  // above method using extra links. (important, since it doesn't guarantee
  // strong connectedness) This is just to avoid buttons having no links in a
  // particular direction when there's a suitable neighbor. you get good graphs
  // without this too. 2017/09/29: FIXME: This now currently only enabled inside
  // menu bars, ideally we'd disable it everywhere. Menus in particular need to
  // catch failure. For general navigation it feels awkward. Disabling it may
  // lead to disconnected graphs when nodes are very spaced out on different
  // axis. Perhaps consider offering this as an option?
  if (result->DistBox == FLT_MAX &&
      dist_axial < result->DistAxial) // Check axial match
    if (g.NavLayer == NavLayer_Menu &&
        !(g.NavWindow->Flags & WindowFlags_ChildMenu))
      if ((move_dir == Dir_Left && dax < 0.0f) ||
          (move_dir == Dir_Right && dax > 0.0f) ||
          (move_dir == Dir_Up && day < 0.0f) ||
          (move_dir == Dir_Down && day > 0.0f)) {
        result->DistAxial = dist_axial;
        new_best = true;
      }

  return new_best;
}

static void Gui::NavApplyItemToResult(NavItemData *result) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  result->Window = window;
  result->ID = g.LastItemData.ID;
  result->FocusScopeId = g.CurrentFocusScopeId;
  result->InFlags = g.LastItemData.InFlags;
  result->RectRel = WindowRectAbsToRel(window, g.LastItemData.NavRect);
  if (result->InFlags & ItemFlags_HasSelectionUserData) {
    ASSERT(g.NextItemData.SelectionUserData != SelectionUserData_Invalid);
    result->SelectionUserData =
        g.NextItemData
            .SelectionUserData; // INTENTIONAL: At this point this field is not
                                // cleared in NextItemData. Avoid unnecessary
                                // copy to LastItemData.
  }
}

// True when current work location may be scrolled horizontally when moving left
// / right. This is generally always true UNLESS within a column. We don't have
// a vertical equivalent.
void Gui::NavUpdateCurrentWindowIsScrollPushableX() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  window->DC.NavIsScrollPushableX =
      (g.CurrentTable == NULL && window->DC.CurrentColumns == NULL);
}

// We get there when either NavId == id, or when g.NavAnyRequest is set (which
// is updated by NavUpdateAnyRequestFlag above) This is called after
// LastItemData is set, but NextItemData is also still valid.
static void Gui::NavProcessItem() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  const ID id = g.LastItemData.ID;
  const ItemFlags item_flags = g.LastItemData.InFlags;

  // When inside a container that isn't scrollable with Left<>Right, clip
  // NavRect accordingly (#2221)
  if (window->DC.NavIsScrollPushableX == false) {
    g.LastItemData.NavRect.Min.x =
        Clamp(g.LastItemData.NavRect.Min.x, window->ClipRect.Min.x,
              window->ClipRect.Max.x);
    g.LastItemData.NavRect.Max.x =
        Clamp(g.LastItemData.NavRect.Max.x, window->ClipRect.Min.x,
              window->ClipRect.Max.x);
  }
  const Rect nav_bb = g.LastItemData.NavRect;

  // Process Init Request
  if (g.NavInitRequest && g.NavLayer == window->DC.NavLayerCurrent &&
      (item_flags & ItemFlags_Disabled) == 0) {
    // Even if 'ItemFlags_NoNavDefaultFocus' is on (typically
    // collapse/close button) we record the first ResultId so they can be used
    // as a fallback
    const bool candidate_for_nav_default_focus =
        (item_flags & ItemFlags_NoNavDefaultFocus) == 0;
    if (candidate_for_nav_default_focus || g.NavInitResult.ID == 0) {
      NavApplyItemToResult(&g.NavInitResult);
    }
    if (candidate_for_nav_default_focus) {
      g.NavInitRequest = false; // Found a match, clear request
      NavUpdateAnyRequestFlag();
    }
  }

  // Process Move Request (scoring for navigation)
  // FIXME-NAV: Consider policy for double scoring (scoring from NavScoringRect
  // + scoring from a rect wrapped according to current wrapping policy)
  if (g.NavMoveScoringItems && (item_flags & ItemFlags_Disabled) == 0) {
    const bool is_tabbing = (g.NavMoveFlags & NavMoveFlags_IsTabbing) != 0;
    if (is_tabbing) {
      NavProcessItemForTabbingRequest(id, item_flags, g.NavMoveFlags);
    } else if (g.NavId != id ||
               (g.NavMoveFlags & NavMoveFlags_AllowCurrentNavId)) {
      NavItemData *result = (window == g.NavWindow) ? &g.NavMoveResultLocal
                                                    : &g.NavMoveResultOther;
      if (NavScoreItem(result))
        NavApplyItemToResult(result);

      // Features like PageUp/PageDown need to maintain a separate score for the
      // visible set of items.
      const float VISIBLE_RATIO = 0.70f;
      if ((g.NavMoveFlags & NavMoveFlags_AlsoScoreVisibleSet) &&
          window->ClipRect.Overlaps(nav_bb))
        if (Clamp(nav_bb.Max.y, window->ClipRect.Min.y,
                  window->ClipRect.Max.y) -
                Clamp(nav_bb.Min.y, window->ClipRect.Min.y,
                      window->ClipRect.Max.y) >=
            (nav_bb.Max.y - nav_bb.Min.y) * VISIBLE_RATIO)
          if (NavScoreItem(&g.NavMoveResultLocalVisible))
            NavApplyItemToResult(&g.NavMoveResultLocalVisible);
    }
  }

  // Update information for currently focused/navigated item
  if (g.NavId == id) {
    if (g.NavWindow != window)
      SetNavWindow(
          window); // Always refresh g.NavWindow, because some operations such
                   // as FocusItem() may not have a window.
    g.NavLayer = window->DC.NavLayerCurrent;
    g.NavFocusScopeId = g.CurrentFocusScopeId;
    g.NavIdIsAlive = true;
    if (g.LastItemData.InFlags & ItemFlags_HasSelectionUserData) {
      ASSERT(g.NextItemData.SelectionUserData != SelectionUserData_Invalid);
      g.NavLastValidSelectionUserData =
          g.NextItemData
              .SelectionUserData; // INTENTIONAL: At this point this field is
                                  // not cleared in NextItemData. Avoid
                                  // unnecessary copy to LastItemData.
    }
    window->NavRectRel[window->DC.NavLayerCurrent] = WindowRectAbsToRel(
        window,
        nav_bb); // Store item bounding box (relative to window position)
  }
}

// Handle "scoring" of an item for a tabbing/focusing request initiated by
// NavUpdateCreateTabbingRequest(). Note that SetKeyboardFocusHere() API calls
// are considered tabbing requests!
// - Case 1: no nav/active id:    set result to first eligible item, stop
// storing.
// - Case 2: tab forward:         on ref id set counter, on counter elapse store
// result
// - Case 3: tab forward wrap:    set result to first eligible item
// (preemptively), on ref id set counter, on next frame if counter hasn't
// elapsed store result. // FIXME-TABBING: Could be done as a next-frame
// forwarded request
// - Case 4: tab backward:        store all results, on ref id pick prev, stop
// storing
// - Case 5: tab backward wrap:   store all results, on ref id if no result keep
// storing until last // FIXME-TABBING: Could be done as next-frame forwarded
// requested
void Gui::NavProcessItemForTabbingRequest(ID id, ItemFlags item_flags,
                                          NavMoveFlags move_flags) {
  Context &g = *GGui;

  if ((move_flags & NavMoveFlags_FocusApi) == 0)
    if (g.NavLayer != g.CurrentWindow->DC.NavLayerCurrent)
      return;
  if (g.NavFocusScopeId != g.CurrentFocusScopeId)
    return;

  // - Can always land on an item when using API call.
  // - Tabbing with _NavEnableKeyboard (space/enter/arrows): goes through every
  // item.
  // - Tabbing without _NavEnableKeyboard: goes through inputable items only.
  bool can_stop;
  if (move_flags & NavMoveFlags_FocusApi)
    can_stop = true;
  else
    can_stop = (item_flags & ItemFlags_NoTabStop) == 0 &&
               ((g.IO.ConfigFlags & ConfigFlags_NavEnableKeyboard) ||
                (item_flags & ItemFlags_Inputable));

  // Always store in NavMoveResultLocal (unlike directional request which uses
  // NavMoveResultOther on sibling/flattened windows)
  NavItemData *result = &g.NavMoveResultLocal;
  if (g.NavTabbingDir == +1) {
    // Tab Forward or SetKeyboardFocusHere() with >= 0
    if (can_stop && g.NavTabbingResultFirst.ID == 0)
      NavApplyItemToResult(&g.NavTabbingResultFirst);
    if (can_stop && g.NavTabbingCounter > 0 && --g.NavTabbingCounter == 0)
      NavMoveRequestResolveWithLastItem(result);
    else if (g.NavId == id)
      g.NavTabbingCounter = 1;
  } else if (g.NavTabbingDir == -1) {
    // Tab Backward
    if (g.NavId == id) {
      if (result->ID) {
        g.NavMoveScoringItems = false;
        NavUpdateAnyRequestFlag();
      }
    } else if (can_stop) {
      // Keep applying until reaching NavId
      NavApplyItemToResult(result);
    }
  } else if (g.NavTabbingDir == 0) {
    if (can_stop && g.NavId == id)
      NavMoveRequestResolveWithLastItem(result);
    if (can_stop && g.NavTabbingResultFirst.ID == 0) // Tab init
      NavApplyItemToResult(&g.NavTabbingResultFirst);
  }
}

bool Gui::NavMoveRequestButNoResultYet() {
  Context &g = *GGui;
  return g.NavMoveScoringItems && g.NavMoveResultLocal.ID == 0 &&
         g.NavMoveResultOther.ID == 0;
}

// FIXME: ScoringRect is not set
void Gui::NavMoveRequestSubmit(Dir move_dir, Dir clip_dir,
                               NavMoveFlags move_flags,
                               ScrollFlags scroll_flags) {
  Context &g = *GGui;
  ASSERT(g.NavWindow != NULL);

  if (move_flags & NavMoveFlags_IsTabbing)
    move_flags |= NavMoveFlags_AllowCurrentNavId;

  g.NavMoveSubmitted = g.NavMoveScoringItems = true;
  g.NavMoveDir = move_dir;
  g.NavMoveDirForDebug = move_dir;
  g.NavMoveClipDir = clip_dir;
  g.NavMoveFlags = move_flags;
  g.NavMoveScrollFlags = scroll_flags;
  g.NavMoveForwardToNextFrame = false;
  g.NavMoveKeyMods = (move_flags & NavMoveFlags_FocusApi) ? 0 : g.IO.KeyMods;
  g.NavMoveResultLocal.Clear();
  g.NavMoveResultLocalVisible.Clear();
  g.NavMoveResultOther.Clear();
  g.NavTabbingCounter = 0;
  g.NavTabbingResultFirst.Clear();
  NavUpdateAnyRequestFlag();
}

void Gui::NavMoveRequestResolveWithLastItem(NavItemData *result) {
  Context &g = *GGui;
  g.NavMoveScoringItems = false; // Ensure request doesn't need more processing
  NavApplyItemToResult(result);
  NavUpdateAnyRequestFlag();
}

// Called by TreePop() to implement TreeNodeFlags_NavLeftJumpsBackHere
void Gui::NavMoveRequestResolveWithPastTreeNode(
    NavItemData *result, NavTreeNodeData *tree_node_data) {
  Context &g = *GGui;
  g.NavMoveScoringItems = false;
  g.LastItemData.ID = tree_node_data->ID;
  g.LastItemData.InFlags =
      tree_node_data->InFlags &
      ~ItemFlags_HasSelectionUserData; // Losing SelectionUserData,
                                       // recovered next-frame (cheaper).
  g.LastItemData.NavRect = tree_node_data->NavRect;
  NavApplyItemToResult(result); // Result this instead of implementing a
                                // NavApplyPastTreeNodeToResult()
  NavClearPreferredPosForAxis(Axis_Y);
  NavUpdateAnyRequestFlag();
}

void Gui::NavMoveRequestCancel() {
  Context &g = *GGui;
  g.NavMoveSubmitted = g.NavMoveScoringItems = false;
  NavUpdateAnyRequestFlag();
}

// Forward will reuse the move request again on the next frame (generally with
// modifications done to it)
void Gui::NavMoveRequestForward(Dir move_dir, Dir clip_dir,
                                NavMoveFlags move_flags,
                                ScrollFlags scroll_flags) {
  Context &g = *GGui;
  ASSERT(g.NavMoveForwardToNextFrame == false);
  NavMoveRequestCancel();
  g.NavMoveForwardToNextFrame = true;
  g.NavMoveDir = move_dir;
  g.NavMoveClipDir = clip_dir;
  g.NavMoveFlags = move_flags | NavMoveFlags_Forwarded;
  g.NavMoveScrollFlags = scroll_flags;
}

// Navigation wrap-around logic is delayed to the end of the frame because this
// operation is only valid after entire popup is assembled and in case of
// appended popups it is not clear which EndPopup() call is final.
void Gui::NavMoveRequestTryWrapping(Window *window, NavMoveFlags wrap_flags) {
  Context &g = *GGui;
  ASSERT((wrap_flags & NavMoveFlags_WrapMask_) != 0 &&
         (wrap_flags & ~NavMoveFlags_WrapMask_) ==
             0); // Call with _WrapX, _WrapY, _LoopX, _LoopY

  // In theory we should test for NavMoveRequestButNoResultYet() but there's no
  // point doing it: as NavEndFrame() will do the same test. It will end up
  // calling NavUpdateCreateWrappingRequest().
  if (g.NavWindow == window && g.NavMoveScoringItems &&
      g.NavLayer == NavLayer_Main)
    g.NavMoveFlags = (g.NavMoveFlags & ~NavMoveFlags_WrapMask_) | wrap_flags;
}

// FIXME: This could be replaced by updating a frame number in each window when
// (window == NavWindow) and (NavLayer == 0). This way we could find the last
// focused window among our children. It would be much less confusing this way?
static void Gui::NavSaveLastChildNavWindowIntoParent(Window *nav_window) {
  Window *parent = nav_window;
  while (parent && parent->RootWindow != parent &&
         (parent->Flags & (WindowFlags_Popup | WindowFlags_ChildMenu)) == 0)
    parent = parent->ParentWindow;
  if (parent && parent != nav_window)
    parent->NavLastChildNavWindow = nav_window;
}

// Restore the last focused child.
// Call when we are expected to land on the Main Layer (0) after FocusWindow()
static Window *Gui::NavRestoreLastChildNavWindow(Window *window) {
  if (window->NavLastChildNavWindow && window->NavLastChildNavWindow->WasActive)
    return window->NavLastChildNavWindow;
  if (window->DockNodeAsHost && window->DockNodeAsHost->TabBar)
    if (TabItem *tab = TabBarFindMostRecentlySelectedTabForActiveWindow(
            window->DockNodeAsHost->TabBar))
      return tab->Window;
  return window;
}

void Gui::NavRestoreLayer(NavLayer layer) {
  Context &g = *GGui;
  if (layer == NavLayer_Main) {
    Window *prev_nav_window = g.NavWindow;
    g.NavWindow = NavRestoreLastChildNavWindow(
        g.NavWindow); // FIXME-NAV: Should clear ongoing nav requests?
    g.NavLastValidSelectionUserData = SelectionUserData_Invalid;
    if (prev_nav_window)
      DEBUG_LOG_FOCUS(
          "[focus] NavRestoreLayer: from \"%s\" to SetNavWindow(\"%s\")\n",
          prev_nav_window->Name, g.NavWindow->Name);
  }
  Window *window = g.NavWindow;
  if (window->NavLastIds[layer] != 0) {
    SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
  } else {
    g.NavLayer = layer;
    NavInitWindow(window, true);
  }
}

void Gui::NavRestoreHighlightAfterMove() {
  Context &g = *GGui;
  g.NavDisableHighlight = false;
  g.NavDisableMouseHover = g.NavMousePosDirty = true;
}

static inline void Gui::NavUpdateAnyRequestFlag() {
  Context &g = *GGui;
  g.NavAnyRequest = g.NavMoveScoringItems || g.NavInitRequest ||
                    (DEBUG_NAV_SCORING && g.NavWindow != NULL);
  if (g.NavAnyRequest)
    ASSERT(g.NavWindow != NULL);
}

// This needs to be called before we submit any widget (aka in or before Begin)
void Gui::NavInitWindow(Window *window, bool force_reinit) {
  // FIXME: ChildWindow test here is wrong for docking
  Context &g = *GGui;
  ASSERT(window == g.NavWindow);

  if (window->Flags & WindowFlags_NoNavInputs) {
    g.NavId = 0;
    g.NavFocusScopeId = window->NavRootFocusScopeId;
    return;
  }

  bool init_for_nav = false;
  if (window == window->RootWindow || (window->Flags & WindowFlags_Popup) ||
      (window->NavLastIds[0] == 0) || force_reinit)
    init_for_nav = true;
  DEBUG_LOG_NAV("[nav] NavInitRequest: from NavInitWindow(), init_for_nav=%d, "
                "window=\"%s\", layer=%d\n",
                init_for_nav, window->Name, g.NavLayer);
  if (init_for_nav) {
    SetNavID(0, g.NavLayer, window->NavRootFocusScopeId, Rect());
    g.NavInitRequest = true;
    g.NavInitRequestFromMove = false;
    g.NavInitResult.ID = 0;
    NavUpdateAnyRequestFlag();
  } else {
    g.NavId = window->NavLastIds[0];
    g.NavFocusScopeId = window->NavRootFocusScopeId;
  }
}

static Vec2 Gui::NavCalcPreferredRefPos() {
  Context &g = *GGui;
  Window *window = g.NavWindow;
  if (g.NavDisableHighlight || !g.NavDisableMouseHover || !window) {
    // Mouse (we need a fallback in case the mouse becomes invalid after being
    // used) The +1.0f offset when stored by OpenPopupEx() allows reopening this
    // or another popup (same or another mouse button) while not moving the
    // mouse, it is pretty standard. In theory we could move that +1.0f offset
    // in OpenPopupEx()
    Vec2 p =
        IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : g.MouseLastValidPos;
    return Vec2(p.x + 1.0f, p.y);
  } else {
    // When navigation is active and mouse is disabled, pick a position around
    // the bottom left of the currently navigated item Take account of upcoming
    // scrolling (maybe set mouse pos should be done in EndFrame?)
    Rect rect_rel = WindowRectRelToAbs(window, window->NavRectRel[g.NavLayer]);
    if (window->LastFrameActive != g.FrameCount &&
        (window->ScrollTarget.x != FLT_MAX ||
         window->ScrollTarget.y != FLT_MAX)) {
      Vec2 next_scroll = CalcNextScrollFromScrollTargetAndClamp(window);
      rect_rel.Translate(window->Scroll - next_scroll);
    }
    Vec2 pos = Vec2(
        rect_rel.Min.x + Min(g.Style.FramePadding.x * 4, rect_rel.GetWidth()),
        rect_rel.Max.y - Min(g.Style.FramePadding.y, rect_rel.GetHeight()));
    Viewport *viewport = window->Viewport;
    return Trunc(Clamp(
        pos, viewport->Pos,
        viewport->Pos +
            viewport->Size)); // Trunc() is important because non-integer
                              // mouse position application in backend might be
                              // lossy and result in undesirable non-zero delta.
  }
}

float Gui::GetNavTweakPressedAmount(Axis axis) {
  Context &g = *GGui;
  float repeat_delay, repeat_rate;
  GetTypematicRepeatRate(InputFlags_RepeatRateNavTweak, &repeat_delay,
                         &repeat_rate);

  Key key_less, key_more;
  if (g.NavInputSource == InputSource_Gamepad) {
    key_less = (axis == Axis_X) ? Key_GamepadDpadLeft : Key_GamepadDpadUp;
    key_more = (axis == Axis_X) ? Key_GamepadDpadRight : Key_GamepadDpadDown;
  } else {
    key_less = (axis == Axis_X) ? Key_LeftArrow : Key_UpArrow;
    key_more = (axis == Axis_X) ? Key_RightArrow : Key_DownArrow;
  }
  float amount =
      (float)GetKeyPressedAmount(key_more, repeat_delay, repeat_rate) -
      (float)GetKeyPressedAmount(key_less, repeat_delay, repeat_rate);
  if (amount != 0.0f && IsKeyDown(key_less) &&
      IsKeyDown(key_more)) // Cancel when opposite directions are held,
                           // regardless of repeat phase
    amount = 0.0f;
  return amount;
}

static void Gui::NavUpdate() {
  Context &g = *GGui;
  IO &io = g.IO;

  io.WantSetMousePos = false;
  // if (g.NavScoringDebugCount > 0) DEBUG_LOG_NAV("[nav] NavScoringDebugCount
  // %d for '%s' layer %d (Init:%d, Move:%d)\n", g.NavScoringDebugCount,
  // g.NavWindow ? g.NavWindow->Name : "NULL", g.NavLayer, g.NavInitRequest ||
  // g.NavInitResultId != 0, g.NavMoveRequest);

  // Set input source based on which keys are last pressed (as some features
  // differs when used with Gamepad vs Keyboard)
  // FIXME-NAV: Now that keys are separated maybe we can get rid of
  // NavInputSource?
  const bool nav_gamepad_active =
      (io.ConfigFlags & ConfigFlags_NavEnableGamepad) != 0 &&
      (io.BackendFlags & BackendFlags_HasGamepad) != 0;
  const Key nav_gamepad_keys_to_change_source[] = {
      Key_GamepadFaceRight, Key_GamepadFaceLeft,  Key_GamepadFaceUp,
      Key_GamepadFaceDown,  Key_GamepadDpadRight, Key_GamepadDpadLeft,
      Key_GamepadDpadUp,    Key_GamepadDpadDown};
  if (nav_gamepad_active)
    for (Key key : nav_gamepad_keys_to_change_source)
      if (IsKeyDown(key))
        g.NavInputSource = InputSource_Gamepad;
  const bool nav_keyboard_active =
      (io.ConfigFlags & ConfigFlags_NavEnableKeyboard) != 0;
  const Key nav_keyboard_keys_to_change_source[] = {
      Key_Space,     Key_Enter,   Key_Escape,   Key_RightArrow,
      Key_LeftArrow, Key_UpArrow, Key_DownArrow};
  if (nav_keyboard_active)
    for (Key key : nav_keyboard_keys_to_change_source)
      if (IsKeyDown(key))
        g.NavInputSource = InputSource_Keyboard;

  // Process navigation init request (select first/default focus)
  g.NavJustMovedToId = 0;
  if (g.NavInitResult.ID != 0)
    NavInitRequestApplyResult();
  g.NavInitRequest = false;
  g.NavInitRequestFromMove = false;
  g.NavInitResult.ID = 0;

  // Process navigation move request
  if (g.NavMoveSubmitted)
    NavMoveRequestApplyResult();
  g.NavTabbingCounter = 0;
  g.NavMoveSubmitted = g.NavMoveScoringItems = false;

  // Schedule mouse position update (will be done at the bottom of this
  // function, after 1) processing all move requests and 2) updating scrolling)
  bool set_mouse_pos = false;
  if (g.NavMousePosDirty && g.NavIdIsAlive)
    if (!g.NavDisableHighlight && g.NavDisableMouseHover && g.NavWindow)
      set_mouse_pos = true;
  g.NavMousePosDirty = false;
  ASSERT(g.NavLayer == NavLayer_Main || g.NavLayer == NavLayer_Menu);

  // Store our return window (for returning from Menu Layer to Main Layer) and
  // clear it as soon as we step back in our own Layer 0
  if (g.NavWindow)
    NavSaveLastChildNavWindowIntoParent(g.NavWindow);
  if (g.NavWindow && g.NavWindow->NavLastChildNavWindow != NULL &&
      g.NavLayer == NavLayer_Main)
    g.NavWindow->NavLastChildNavWindow = NULL;

  // Update CTRL+TAB and Windowing features (hold Square to move/resize/etc.)
  NavUpdateWindowing();

  // Set output flags for user application
  io.NavActive = (nav_keyboard_active || nav_gamepad_active) && g.NavWindow &&
                 !(g.NavWindow->Flags & WindowFlags_NoNavInputs);
  io.NavVisible = (io.NavActive && g.NavId != 0 && !g.NavDisableHighlight) ||
                  (g.NavWindowingTarget != NULL);

  // Process NavCancel input (to close a popup, get back to parent, clear focus)
  NavUpdateCancelRequest();

  // Process manual activation request
  g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = 0;
  g.NavActivateFlags = ActivateFlags_None;
  if (g.NavId != 0 && !g.NavDisableHighlight && !g.NavWindowingTarget &&
      g.NavWindow && !(g.NavWindow->Flags & WindowFlags_NoNavInputs)) {
    const bool activate_down =
        (nav_keyboard_active && IsKeyDown(Key_Space)) ||
        (nav_gamepad_active && IsKeyDown(Key_NavGamepadActivate));
    const bool activate_pressed =
        activate_down &&
        ((nav_keyboard_active && IsKeyPressed(Key_Space, false)) ||
         (nav_gamepad_active && IsKeyPressed(Key_NavGamepadActivate, false)));
    const bool input_down =
        (nav_keyboard_active &&
         (IsKeyDown(Key_Enter) || IsKeyDown(Key_KeypadEnter))) ||
        (nav_gamepad_active && IsKeyDown(Key_NavGamepadInput));
    const bool input_pressed =
        input_down &&
        ((nav_keyboard_active && (IsKeyPressed(Key_Enter, false) ||
                                  IsKeyPressed(Key_KeypadEnter, false))) ||
         (nav_gamepad_active && IsKeyPressed(Key_NavGamepadInput, false)));
    if (g.ActiveId == 0 && activate_pressed) {
      g.NavActivateId = g.NavId;
      g.NavActivateFlags = ActivateFlags_PreferTweak;
    }
    if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && input_pressed) {
      g.NavActivateId = g.NavId;
      g.NavActivateFlags = ActivateFlags_PreferInput;
    }
    if ((g.ActiveId == 0 || g.ActiveId == g.NavId) &&
        (activate_down || input_down))
      g.NavActivateDownId = g.NavId;
    if ((g.ActiveId == 0 || g.ActiveId == g.NavId) &&
        (activate_pressed || input_pressed))
      g.NavActivatePressedId = g.NavId;
  }
  if (g.NavWindow && (g.NavWindow->Flags & WindowFlags_NoNavInputs))
    g.NavDisableHighlight = true;
  if (g.NavActivateId != 0)
    ASSERT(g.NavActivateDownId == g.NavActivateId);

  // Process programmatic activation request
  // FIXME-NAV: Those should eventually be queued (unlike focus they don't
  // cancel each others)
  if (g.NavNextActivateId != 0) {
    g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId =
        g.NavNextActivateId;
    g.NavActivateFlags = g.NavNextActivateFlags;
  }
  g.NavNextActivateId = 0;

  // Process move requests
  NavUpdateCreateMoveRequest();
  if (g.NavMoveDir == Dir_None)
    NavUpdateCreateTabbingRequest();
  NavUpdateAnyRequestFlag();
  g.NavIdIsAlive = false;

  // Scrolling
  if (g.NavWindow && !(g.NavWindow->Flags & WindowFlags_NoNavInputs) &&
      !g.NavWindowingTarget) {
    // *Fallback* manual-scroll with Nav directional keys when window has no
    // navigable item
    Window *window = g.NavWindow;
    const float scroll_speed =
        ROUND(window->CalcFontSize() * 100 *
              io.DeltaTime); // We need round the scrolling speed because
                             // sub-pixel scroll isn't reliably supported.
    const Dir move_dir = g.NavMoveDir;
    if (window->DC.NavLayersActiveMask == 0x00 &&
        window->DC.NavWindowHasScrollY && move_dir != Dir_None) {
      if (move_dir == Dir_Left || move_dir == Dir_Right)
        SetScrollX(window, Trunc(window->Scroll.x +
                                 ((move_dir == Dir_Left) ? -1.0f : +1.0f) *
                                     scroll_speed));
      if (move_dir == Dir_Up || move_dir == Dir_Down)
        SetScrollY(window, Trunc(window->Scroll.y +
                                 ((move_dir == Dir_Up) ? -1.0f : +1.0f) *
                                     scroll_speed));
    }

    // *Normal* Manual scroll with LStick
    // Next movement request will clamp the NavId reference rectangle to the
    // visible area, so navigation will resume within those bounds.
    if (nav_gamepad_active) {
      const Vec2 scroll_dir =
          GetKeyMagnitude2d(Key_GamepadLStickLeft, Key_GamepadLStickRight,
                            Key_GamepadLStickUp, Key_GamepadLStickDown);
      const float tweak_factor = IsKeyDown(Key_NavGamepadTweakSlow)
                                     ? 1.0f / 10.0f
                                 : IsKeyDown(Key_NavGamepadTweakFast) ? 10.0f
                                                                      : 1.0f;
      if (scroll_dir.x != 0.0f && window->ScrollbarX)
        SetScrollX(window, Trunc(window->Scroll.x +
                                 scroll_dir.x * scroll_speed * tweak_factor));
      if (scroll_dir.y != 0.0f)
        SetScrollY(window, Trunc(window->Scroll.y +
                                 scroll_dir.y * scroll_speed * tweak_factor));
    }
  }

  // Always prioritize mouse highlight if navigation is disabled
  if (!nav_keyboard_active && !nav_gamepad_active) {
    g.NavDisableHighlight = true;
    g.NavDisableMouseHover = set_mouse_pos = false;
  }

  // Update mouse position if requested
  // (This will take into account the possibility that a Scroll was queued in
  // the window to offset our absolute mouse position before scroll has been
  // applied)
  if (set_mouse_pos && (io.ConfigFlags & ConfigFlags_NavEnableSetMousePos) &&
      (io.BackendFlags & BackendFlags_HasSetMousePos))
    TeleportMousePos(NavCalcPreferredRefPos());

  // [DEBUG]
  g.NavScoringDebugCount = 0;
#if DEBUG_NAV_RECTS
  if (Window *debug_window = g.NavWindow) {
    DrawList *draw_list = GetForegroundDrawList(debug_window);
    int layer = g.NavLayer; /* for (int layer = 0; layer < 2; layer++)*/
    {
      Rect r =
          WindowRectRelToAbs(debug_window, debug_window->NavRectRel[layer]);
      draw_list->AddRect(r.Min, r.Max, COL32(255, 200, 0, 255));
    }
    // if (1) { U32 col = (!debug_window->Hidden) ? COL32(255,0,255,255) :
    // COL32(255,0,0,255); Vec2 p = NavCalcPreferredRefPos(); char buf[32];
    // FormatString(buf, 32, "%d", g.NavLayer);
    // draw_list->AddCircleFilled(p, 3.0f, col); draw_list->AddText(NULL, 13.0f,
    // p + Vec2(8,-4), col, buf); }
  }
#endif
}

void Gui::NavInitRequestApplyResult() {
  // In very rare cases g.NavWindow may be null (e.g. clearing focus after
  // requesting an init request, which does happen when releasing Alt while
  // clicking on void)
  Context &g = *GGui;
  if (!g.NavWindow)
    return;

  NavItemData *result = &g.NavInitResult;
  if (g.NavId != result->ID) {
    g.NavJustMovedToId = result->ID;
    g.NavJustMovedToFocusScopeId = result->FocusScopeId;
    g.NavJustMovedToKeyMods = 0;
  }

  // Apply result from previous navigation init request (will typically select
  // the first item, unless SetItemDefaultFocus() has been called)
  // FIXME-NAV: On _NavFlattened windows, g.NavWindow will only be updated
  // during subsequent frame. Not a problem currently.
  DEBUG_LOG_NAV("[nav] NavInitRequest: ApplyResult: NavID 0x%08X in Layer %d "
                "Window \"%s\"\n",
                result->ID, g.NavLayer, g.NavWindow->Name);
  SetNavID(result->ID, g.NavLayer, result->FocusScopeId, result->RectRel);
  g.NavIdIsAlive = true; // Mark as alive from previous frame as we got a result
  if (result->SelectionUserData != SelectionUserData_Invalid)
    g.NavLastValidSelectionUserData = result->SelectionUserData;
  if (g.NavInitRequestFromMove)
    NavRestoreHighlightAfterMove();
}

// Bias scoring rect ahead of scoring + update preferred pos (if missing) using
// source position
static void NavBiasScoringRect(Rect &r, Vec2 &preferred_pos_rel, Dir move_dir,
                               NavMoveFlags move_flags) {
  // Bias initial rect
  Context &g = *GGui;
  const Vec2 rel_to_abs_offset = g.NavWindow->DC.CursorStartPos;

  // Initialize bias on departure if we don't have any. So mouse-click + arrow
  // will record bias.
  // - We default to L/U bias, so moving down from a large source item into
  // several columns will land on left-most column.
  // - But each successful move sets new bias on one axis, only cleared when
  // using mouse.
  if ((move_flags & NavMoveFlags_Forwarded) == 0) {
    if (preferred_pos_rel.x == FLT_MAX)
      preferred_pos_rel.x = Min(r.Min.x + 1.0f, r.Max.x) - rel_to_abs_offset.x;
    if (preferred_pos_rel.y == FLT_MAX)
      preferred_pos_rel.y = r.GetCenter().y - rel_to_abs_offset.y;
  }

  // Apply general bias on the other axis
  if ((move_dir == Dir_Up || move_dir == Dir_Down) &&
      preferred_pos_rel.x != FLT_MAX)
    r.Min.x = r.Max.x = preferred_pos_rel.x + rel_to_abs_offset.x;
  else if ((move_dir == Dir_Left || move_dir == Dir_Right) &&
           preferred_pos_rel.y != FLT_MAX)
    r.Min.y = r.Max.y = preferred_pos_rel.y + rel_to_abs_offset.y;
}

void Gui::NavUpdateCreateMoveRequest() {
  Context &g = *GGui;
  IO &io = g.IO;
  Window *window = g.NavWindow;
  const bool nav_gamepad_active =
      (io.ConfigFlags & ConfigFlags_NavEnableGamepad) != 0 &&
      (io.BackendFlags & BackendFlags_HasGamepad) != 0;
  const bool nav_keyboard_active =
      (io.ConfigFlags & ConfigFlags_NavEnableKeyboard) != 0;

  if (g.NavMoveForwardToNextFrame && window != NULL) {
    // Forwarding previous request (which has been modified, e.g. wrap around
    // menus rewrite the requests with a starting rectangle at the other side of
    // the window) (preserve most state, which were already set by the
    // NavMoveRequestForward() function)
    ASSERT(g.NavMoveDir != Dir_None && g.NavMoveClipDir != Dir_None);
    ASSERT(g.NavMoveFlags & NavMoveFlags_Forwarded);
    DEBUG_LOG_NAV("[nav] NavMoveRequestForward %d\n", g.NavMoveDir);
  } else {
    // Initiate directional inputs request
    g.NavMoveDir = Dir_None;
    g.NavMoveFlags = NavMoveFlags_None;
    g.NavMoveScrollFlags = ScrollFlags_None;
    if (window && !g.NavWindowingTarget &&
        !(window->Flags & WindowFlags_NoNavInputs)) {
      const InputFlags repeat_mode =
          InputFlags_Repeat | (InputFlags)InputFlags_RepeatRateNavMove;
      if (!IsActiveIdUsingNavDir(Dir_Left) &&
          ((nav_gamepad_active &&
            IsKeyPressed(Key_GamepadDpadLeft, KeyOwner_None, repeat_mode)) ||
           (nav_keyboard_active &&
            IsKeyPressed(Key_LeftArrow, KeyOwner_None, repeat_mode)))) {
        g.NavMoveDir = Dir_Left;
      }
      if (!IsActiveIdUsingNavDir(Dir_Right) &&
          ((nav_gamepad_active &&
            IsKeyPressed(Key_GamepadDpadRight, KeyOwner_None, repeat_mode)) ||
           (nav_keyboard_active &&
            IsKeyPressed(Key_RightArrow, KeyOwner_None, repeat_mode)))) {
        g.NavMoveDir = Dir_Right;
      }
      if (!IsActiveIdUsingNavDir(Dir_Up) &&
          ((nav_gamepad_active &&
            IsKeyPressed(Key_GamepadDpadUp, KeyOwner_None, repeat_mode)) ||
           (nav_keyboard_active &&
            IsKeyPressed(Key_UpArrow, KeyOwner_None, repeat_mode)))) {
        g.NavMoveDir = Dir_Up;
      }
      if (!IsActiveIdUsingNavDir(Dir_Down) &&
          ((nav_gamepad_active &&
            IsKeyPressed(Key_GamepadDpadDown, KeyOwner_None, repeat_mode)) ||
           (nav_keyboard_active &&
            IsKeyPressed(Key_DownArrow, KeyOwner_None, repeat_mode)))) {
        g.NavMoveDir = Dir_Down;
      }
    }
    g.NavMoveClipDir = g.NavMoveDir;
    g.NavScoringNoClipRect = Rect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
  }

  // Update PageUp/PageDown/Home/End scroll
  // FIXME-NAV: Consider enabling those keys even without the master
  // ConfigFlags_NavEnableKeyboard flag?
  float scoring_rect_offset_y = 0.0f;
  if (window && g.NavMoveDir == Dir_None && nav_keyboard_active)
    scoring_rect_offset_y = NavUpdatePageUpPageDown();
  if (scoring_rect_offset_y != 0.0f) {
    g.NavScoringNoClipRect = window->InnerRect;
    g.NavScoringNoClipRect.TranslateY(scoring_rect_offset_y);
  }

  // [DEBUG] Always send a request when holding CTRL. Hold CTRL + Arrow change
  // the direction.
#if DEBUG_NAV_SCORING
  // if (io.KeyCtrl && IsKeyPressed(Key_C))
  //     g.NavMoveDirForDebug = (Dir)((g.NavMoveDirForDebug + 1) & 3);
  if (io.KeyCtrl) {
    if (g.NavMoveDir == Dir_None)
      g.NavMoveDir = g.NavMoveDirForDebug;
    g.NavMoveClipDir = g.NavMoveDir;
    g.NavMoveFlags |= NavMoveFlags_DebugNoResult;
  }
#endif

  // Submit
  g.NavMoveForwardToNextFrame = false;
  if (g.NavMoveDir != Dir_None)
    NavMoveRequestSubmit(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags,
                         g.NavMoveScrollFlags);

  // Moving with no reference triggers an init request (will be used as a
  // fallback if the direction fails to find a match)
  if (g.NavMoveSubmitted && g.NavId == 0) {
    DEBUG_LOG_NAV("[nav] NavInitRequest: from move, window \"%s\", layer=%d\n",
                  window ? window->Name : "<NULL>", g.NavLayer);
    g.NavInitRequest = g.NavInitRequestFromMove = true;
    g.NavInitResult.ID = 0;
    g.NavDisableHighlight = false;
  }

  // When using gamepad, we project the reference nav bounding box into window
  // visible area. This is to allow resuming navigation inside the visible area
  // after doing a large amount of scrolling, since with gamepad all movements
  // are relative (can't focus a visible object like we can with the mouse).
  if (g.NavMoveSubmitted && g.NavInputSource == InputSource_Gamepad &&
      g.NavLayer == NavLayer_Main &&
      window != NULL) // && (g.NavMoveFlags & NavMoveFlags_Forwarded))
  {
    bool clamp_x =
        (g.NavMoveFlags & (NavMoveFlags_LoopX | NavMoveFlags_WrapX)) == 0;
    bool clamp_y =
        (g.NavMoveFlags & (NavMoveFlags_LoopY | NavMoveFlags_WrapY)) == 0;
    Rect inner_rect_rel =
        WindowRectAbsToRel(window, Rect(window->InnerRect.Min - Vec2(1, 1),
                                        window->InnerRect.Max + Vec2(1, 1)));

    // Take account of changing scroll to handle triggering a new move request
    // on a scrolling frame. (#6171) Otherwise 'inner_rect_rel' would be off on
    // the move result frame.
    inner_rect_rel.Translate(CalcNextScrollFromScrollTargetAndClamp(window) -
                             window->Scroll);

    if ((clamp_x || clamp_y) &&
        !inner_rect_rel.Contains(window->NavRectRel[g.NavLayer])) {
      DEBUG_LOG_NAV(
          "[nav] NavMoveRequest: clamp NavRectRel for gamepad move\n");
      float pad_x =
          Min(inner_rect_rel.GetWidth(), window->CalcFontSize() * 0.5f);
      float pad_y =
          Min(inner_rect_rel.GetHeight(),
              window->CalcFontSize() *
                  0.5f); // Terrible approximation for the intent of starting
                         // navigation from first fully visible item
      inner_rect_rel.Min.x =
          clamp_x ? (inner_rect_rel.Min.x + pad_x) : -FLT_MAX;
      inner_rect_rel.Max.x =
          clamp_x ? (inner_rect_rel.Max.x - pad_x) : +FLT_MAX;
      inner_rect_rel.Min.y =
          clamp_y ? (inner_rect_rel.Min.y + pad_y) : -FLT_MAX;
      inner_rect_rel.Max.y =
          clamp_y ? (inner_rect_rel.Max.y - pad_y) : +FLT_MAX;
      window->NavRectRel[g.NavLayer].ClipWithFull(inner_rect_rel);
      g.NavId = 0;
    }
  }

  // For scoring we use a single segment on the left side our current item
  // bounding box (not touching the edge to avoid box overlap with zero-spaced
  // items)
  Rect scoring_rect;
  if (window != NULL) {
    Rect nav_rect_rel = !window->NavRectRel[g.NavLayer].IsInverted()
                            ? window->NavRectRel[g.NavLayer]
                            : Rect(0, 0, 0, 0);
    scoring_rect = WindowRectRelToAbs(window, nav_rect_rel);
    scoring_rect.TranslateY(scoring_rect_offset_y);
    if (g.NavMoveSubmitted)
      NavBiasScoringRect(
          scoring_rect,
          window->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer],
          g.NavMoveDir, g.NavMoveFlags);
    ASSERT(!scoring_rect
                .IsInverted()); // Ensure we have a non-inverted bounding box
                                // here will allow us to remove extraneous
                                // Fabs() calls in NavScoreItem().
    // GetForegroundDrawList()->AddRect(scoring_rect.Min, scoring_rect.Max,
    // COL32(255,200,0,255)); // [DEBUG] if
    // (!g.NavScoringNoClipRect.IsInverted()) {
    // GetForegroundDrawList()->AddRect(g.NavScoringNoClipRect.Min,
    // g.NavScoringNoClipRect.Max, COL32(255, 200, 0, 255)); } // [DEBUG]
  }
  g.NavScoringRect = scoring_rect;
  g.NavScoringNoClipRect.Add(scoring_rect);
}

void Gui::NavUpdateCreateTabbingRequest() {
  Context &g = *GGui;
  Window *window = g.NavWindow;
  ASSERT(g.NavMoveDir == Dir_None);
  if (window == NULL || g.NavWindowingTarget != NULL ||
      (window->Flags & WindowFlags_NoNavInputs))
    return;

  const bool tab_pressed =
      IsKeyPressed(Key_Tab, KeyOwner_None, InputFlags_Repeat) &&
      !g.IO.KeyCtrl && !g.IO.KeyAlt;
  if (!tab_pressed)
    return;

  // Initiate tabbing request
  // (this is ALWAYS ENABLED, regardless of ConfigFlags_NavEnableKeyboard
  // flag!) See NavProcessItemForTabbingRequest() for a description of the
  // various forward/backward tabbing cases with and without wrapping.
  const bool nav_keyboard_active =
      (g.IO.ConfigFlags & ConfigFlags_NavEnableKeyboard) != 0;
  if (nav_keyboard_active)
    g.NavTabbingDir = g.IO.KeyShift                                        ? -1
                      : (g.NavDisableHighlight == true && g.ActiveId == 0) ? 0
                                                                           : +1;
  else
    g.NavTabbingDir = g.IO.KeyShift ? -1 : (g.ActiveId == 0) ? 0 : +1;
  NavMoveFlags move_flags = NavMoveFlags_IsTabbing | NavMoveFlags_Activate;
  ScrollFlags scroll_flags =
      window->Appearing
          ? ScrollFlags_KeepVisibleEdgeX | ScrollFlags_AlwaysCenterY
          : ScrollFlags_KeepVisibleEdgeX | ScrollFlags_KeepVisibleEdgeY;
  Dir clip_dir = (g.NavTabbingDir < 0) ? Dir_Up : Dir_Down;
  NavMoveRequestSubmit(
      Dir_None, clip_dir, move_flags,
      scroll_flags); // FIXME-NAV: Once we refactor tabbing, add LegacyApi flag
                     // to not activate non-inputable.
  g.NavTabbingCounter = -1;
}

// Apply result from previous frame navigation directional move request. Always
// called from NavUpdate()
void Gui::NavMoveRequestApplyResult() {
  Context &g = *GGui;
#if DEBUG_NAV_SCORING
  if (g.NavMoveFlags & NavMoveFlags_DebugNoResult) // [DEBUG] Scoring all items
                                                   // in NavWindow at all times
    return;
#endif

  // Select which result to use
  NavItemData *result = (g.NavMoveResultLocal.ID != 0)   ? &g.NavMoveResultLocal
                        : (g.NavMoveResultOther.ID != 0) ? &g.NavMoveResultOther
                                                         : NULL;

  // Tabbing forward wrap
  if ((g.NavMoveFlags & NavMoveFlags_IsTabbing) && result == NULL)
    if ((g.NavTabbingCounter == 1 || g.NavTabbingDir == 0) &&
        g.NavTabbingResultFirst.ID)
      result = &g.NavTabbingResultFirst;

  // In a situation when there are no results but NavId != 0, re-enable the
  // Navigation highlight (because g.NavId is not considered as a possible
  // result)
  const Axis axis =
      (g.NavMoveDir == Dir_Up || g.NavMoveDir == Dir_Down) ? Axis_Y : Axis_X;
  if (result == NULL) {
    if (g.NavMoveFlags & NavMoveFlags_IsTabbing)
      g.NavMoveFlags |= NavMoveFlags_NoSetNavHighlight;
    if (g.NavId != 0 && (g.NavMoveFlags & NavMoveFlags_NoSetNavHighlight) == 0)
      NavRestoreHighlightAfterMove();
    NavClearPreferredPosForAxis(
        axis); // On a failed move, clear preferred pos for this axis.
    DEBUG_LOG_NAV("[nav] NavMoveSubmitted but not led to a result!\n");
    return;
  }

  // PageUp/PageDown behavior first jumps to the bottom/top mostly visible item,
  // _otherwise_ use the result from the previous/next page.
  if (g.NavMoveFlags & NavMoveFlags_AlsoScoreVisibleSet)
    if (g.NavMoveResultLocalVisible.ID != 0 &&
        g.NavMoveResultLocalVisible.ID != g.NavId)
      result = &g.NavMoveResultLocalVisible;

  // Maybe entering a flattened child from the outside? In this case solve the
  // tie using the regular scoring rules.
  if (result != &g.NavMoveResultOther && g.NavMoveResultOther.ID != 0 &&
      g.NavMoveResultOther.Window->ParentWindow == g.NavWindow)
    if ((g.NavMoveResultOther.DistBox < result->DistBox) ||
        (g.NavMoveResultOther.DistBox == result->DistBox &&
         g.NavMoveResultOther.DistCenter < result->DistCenter))
      result = &g.NavMoveResultOther;
  ASSERT(g.NavWindow && result->Window);

  // Scroll to keep newly navigated item fully into view.
  if (g.NavLayer == NavLayer_Main) {
    Rect rect_abs = WindowRectRelToAbs(result->Window, result->RectRel);
    ScrollToRectEx(result->Window, rect_abs, g.NavMoveScrollFlags);

    if (g.NavMoveFlags & NavMoveFlags_ScrollToEdgeY) {
      // FIXME: Should remove this? Or make more precise: use ScrollToRectEx()
      // with edge?
      float scroll_target =
          (g.NavMoveDir == Dir_Up) ? result->Window->ScrollMax.y : 0.0f;
      SetScrollY(result->Window, scroll_target);
    }
  }

  if (g.NavWindow != result->Window) {
    DEBUG_LOG_FOCUS("[focus] NavMoveRequest: SetNavWindow(\"%s\")\n",
                    result->Window->Name);
    g.NavWindow = result->Window;
    g.NavLastValidSelectionUserData = SelectionUserData_Invalid;
  }
  if (g.ActiveId != result->ID)
    ClearActiveID();

  // Don't set NavJustMovedToId if just landed on the same spot (which may
  // happen with NavMoveFlags_AllowCurrentNavId) PageUp/PageDown however
  // sets always set NavJustMovedTo (vs Home/End which doesn't) mimicking
  // Windows behavior.
  if ((g.NavId != result->ID || (g.NavMoveFlags & NavMoveFlags_IsPageMove)) &&
      (g.NavMoveFlags & NavMoveFlags_NoSelect) == 0) {
    g.NavJustMovedToId = result->ID;
    g.NavJustMovedToFocusScopeId = result->FocusScopeId;
    g.NavJustMovedToKeyMods = g.NavMoveKeyMods;
  }

  // Apply new NavID/Focus
  DEBUG_LOG_NAV(
      "[nav] NavMoveRequest: result NavID 0x%08X in Layer %d Window \"%s\"\n",
      result->ID, g.NavLayer, g.NavWindow->Name);
  Vec2 preferred_scoring_pos_rel =
      g.NavWindow->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer];
  SetNavID(result->ID, g.NavLayer, result->FocusScopeId, result->RectRel);
  if (result->SelectionUserData != SelectionUserData_Invalid)
    g.NavLastValidSelectionUserData = result->SelectionUserData;

  // Restore last preferred position for current axis
  // (storing in RootWindowForNav-> as the info is desirable at the beginning of
  // a Move Request. In theory all storage should use RootWindowForNav..)
  if ((g.NavMoveFlags & NavMoveFlags_IsTabbing) == 0) {
    preferred_scoring_pos_rel[axis] = result->RectRel.GetCenter()[axis];
    g.NavWindow->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer] =
        preferred_scoring_pos_rel;
  }

  // Tabbing: Activates Inputable, otherwise only Focus
  if ((g.NavMoveFlags & NavMoveFlags_IsTabbing) &&
      (result->InFlags & ItemFlags_Inputable) == 0)
    g.NavMoveFlags &= ~NavMoveFlags_Activate;

  // Activate
  if (g.NavMoveFlags & NavMoveFlags_Activate) {
    g.NavNextActivateId = result->ID;
    g.NavNextActivateFlags = ActivateFlags_None;
    if (g.NavMoveFlags & NavMoveFlags_IsTabbing)
      g.NavNextActivateFlags |= ActivateFlags_PreferInput |
                                ActivateFlags_TryToPreserveState |
                                ActivateFlags_FromTabbing;
  }

  // Enable nav highlight
  if ((g.NavMoveFlags & NavMoveFlags_NoSetNavHighlight) == 0)
    NavRestoreHighlightAfterMove();
}

// Process NavCancel input (to close a popup, get back to parent, clear focus)
// FIXME: In order to support e.g. Escape to clear a selection we'll need:
// - either to store the equivalent of ActiveIdUsingKeyInputMask for a
// FocusScope and test for it.
// - either to move most/all of those tests to the epilogue/end functions of the
// scope they are dealing with (e.g. exit child window in EndChild()) or in
// EndFrame(), to allow an earlier intercept
static void Gui::NavUpdateCancelRequest() {
  Context &g = *GGui;
  const bool nav_gamepad_active =
      (g.IO.ConfigFlags & ConfigFlags_NavEnableGamepad) != 0 &&
      (g.IO.BackendFlags & BackendFlags_HasGamepad) != 0;
  const bool nav_keyboard_active =
      (g.IO.ConfigFlags & ConfigFlags_NavEnableKeyboard) != 0;
  if (!(nav_keyboard_active && IsKeyPressed(Key_Escape, KeyOwner_None)) &&
      !(nav_gamepad_active &&
        IsKeyPressed(Key_NavGamepadCancel, KeyOwner_None)))
    return;

  DEBUG_LOG_NAV("[nav] NavUpdateCancelRequest()\n");
  if (g.ActiveId != 0) {
    ClearActiveID();
  } else if (g.NavLayer != NavLayer_Main) {
    // Leave the "menu" layer
    NavRestoreLayer(NavLayer_Main);
    NavRestoreHighlightAfterMove();
  } else if (g.NavWindow && g.NavWindow != g.NavWindow->RootWindow &&
             !(g.NavWindow->Flags & WindowFlags_Popup) &&
             g.NavWindow->ParentWindow) {
    // Exit child window
    Window *child_window = g.NavWindow;
    Window *parent_window = g.NavWindow->ParentWindow;
    ASSERT(child_window->ChildId != 0);
    Rect child_rect = child_window->Rect();
    FocusWindow(parent_window);
    SetNavID(child_window->ChildId, NavLayer_Main, 0,
             WindowRectAbsToRel(parent_window, child_rect));
    NavRestoreHighlightAfterMove();
  } else if (g.OpenPopupStack.Size > 0 &&
             g.OpenPopupStack.back().Window != NULL &&
             !(g.OpenPopupStack.back().Window->Flags & WindowFlags_Modal)) {
    // Close open popup/menu
    ClosePopupToLevel(g.OpenPopupStack.Size - 1, true);
  } else {
    // Clear NavLastId for popups but keep it for regular child window so we can
    // leave one and come back where we were
    if (g.NavWindow && ((g.NavWindow->Flags & WindowFlags_Popup) ||
                        !(g.NavWindow->Flags & WindowFlags_ChildWindow)))
      g.NavWindow->NavLastIds[0] = 0;
    g.NavId = 0;
  }
}

// Handle PageUp/PageDown/Home/End keys
// Called from NavUpdateCreateMoveRequest() which will use our output to create
// a move request
// FIXME-NAV: This doesn't work properly with NavFlattened siblings as we use
// NavWindow rectangle for reference
// FIXME-NAV: how to get Home/End to aim at the beginning/end of a 2D grid?
static float Gui::NavUpdatePageUpPageDown() {
  Context &g = *GGui;
  Window *window = g.NavWindow;
  if ((window->Flags & WindowFlags_NoNavInputs) || g.NavWindowingTarget != NULL)
    return 0.0f;

  const bool page_up_held = IsKeyDown(Key_PageUp, KeyOwner_None);
  const bool page_down_held = IsKeyDown(Key_PageDown, KeyOwner_None);
  const bool home_pressed =
      IsKeyPressed(Key_Home, KeyOwner_None, InputFlags_Repeat);
  const bool end_pressed =
      IsKeyPressed(Key_End, KeyOwner_None, InputFlags_Repeat);
  if (page_up_held == page_down_held &&
      home_pressed == end_pressed) // Proceed if either (not both) are pressed,
                                   // otherwise early out
    return 0.0f;

  if (g.NavLayer != NavLayer_Main)
    NavRestoreLayer(NavLayer_Main);

  if (window->DC.NavLayersActiveMask == 0x00 &&
      window->DC.NavWindowHasScrollY) {
    // Fallback manual-scroll when window has no navigable item
    if (IsKeyPressed(Key_PageUp, KeyOwner_None, InputFlags_Repeat))
      SetScrollY(window, window->Scroll.y - window->InnerRect.GetHeight());
    else if (IsKeyPressed(Key_PageDown, KeyOwner_None, InputFlags_Repeat))
      SetScrollY(window, window->Scroll.y + window->InnerRect.GetHeight());
    else if (home_pressed)
      SetScrollY(window, 0.0f);
    else if (end_pressed)
      SetScrollY(window, window->ScrollMax.y);
  } else {
    Rect &nav_rect_rel = window->NavRectRel[g.NavLayer];
    const float page_offset_y =
        Max(0.0f, window->InnerRect.GetHeight() -
                      window->CalcFontSize() * 1.0f + nav_rect_rel.GetHeight());
    float nav_scoring_rect_offset_y = 0.0f;
    if (IsKeyPressed(Key_PageUp, true)) {
      nav_scoring_rect_offset_y = -page_offset_y;
      g.NavMoveDir = Dir_Down; // Because our scoring rect is offset up, we
                               // request the down direction (so we can
                               // always land on the last item)
      g.NavMoveClipDir = Dir_Up;
      g.NavMoveFlags = NavMoveFlags_AllowCurrentNavId |
                       NavMoveFlags_AlsoScoreVisibleSet |
                       NavMoveFlags_IsPageMove;
    } else if (IsKeyPressed(Key_PageDown, true)) {
      nav_scoring_rect_offset_y = +page_offset_y;
      g.NavMoveDir = Dir_Up; // Because our scoring rect is offset down, we
                             // request the up direction (so we can always
                             // land on the last item)
      g.NavMoveClipDir = Dir_Down;
      g.NavMoveFlags = NavMoveFlags_AllowCurrentNavId |
                       NavMoveFlags_AlsoScoreVisibleSet |
                       NavMoveFlags_IsPageMove;
    } else if (home_pressed) {
      // FIXME-NAV: handling of Home/End is assuming that the top/bottom most
      // item will be visible with Scroll.y == 0/ScrollMax.y Scrolling will be
      // handled via the NavMoveFlags_ScrollToEdgeY flag, we don't scroll
      // immediately to avoid scrolling happening before nav result. Preserve
      // current horizontal position if we have any.
      nav_rect_rel.Min.y = nav_rect_rel.Max.y = 0.0f;
      if (nav_rect_rel.IsInverted())
        nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
      g.NavMoveDir = Dir_Down;
      g.NavMoveFlags =
          NavMoveFlags_AllowCurrentNavId | NavMoveFlags_ScrollToEdgeY;
      // FIXME-NAV: MoveClipDir left to _None, intentional?
    } else if (end_pressed) {
      nav_rect_rel.Min.y = nav_rect_rel.Max.y = window->ContentSize.y;
      if (nav_rect_rel.IsInverted())
        nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
      g.NavMoveDir = Dir_Up;
      g.NavMoveFlags =
          NavMoveFlags_AllowCurrentNavId | NavMoveFlags_ScrollToEdgeY;
      // FIXME-NAV: MoveClipDir left to _None, intentional?
    }
    return nav_scoring_rect_offset_y;
  }
  return 0.0f;
}

static void Gui::NavEndFrame() {
  Context &g = *GGui;

  // Show CTRL+TAB list window
  if (g.NavWindowingTarget != NULL)
    NavUpdateWindowingOverlay();

  // Perform wrap-around in menus
  // FIXME-NAV: Wrap may need to apply a weight bias on the other axis. e.g. 4x4
  // grid with 2 last items missing on last item won't handle LoopY/WrapY
  // correctly.
  // FIXME-NAV: Wrap (not Loop) support could be handled by the scoring function
  // and then WrapX would function without an extra frame.
  if (g.NavWindow && NavMoveRequestButNoResultYet() &&
      (g.NavMoveFlags & NavMoveFlags_WrapMask_) &&
      (g.NavMoveFlags & NavMoveFlags_Forwarded) == 0)
    NavUpdateCreateWrappingRequest();
}

static void Gui::NavUpdateCreateWrappingRequest() {
  Context &g = *GGui;
  Window *window = g.NavWindow;

  bool do_forward = false;
  Rect bb_rel = window->NavRectRel[g.NavLayer];
  Dir clip_dir = g.NavMoveDir;

  const NavMoveFlags move_flags = g.NavMoveFlags;
  // const Axis move_axis = (g.NavMoveDir == Dir_Up || g.NavMoveDir ==
  // Dir_Down) ? Axis_Y : Axis_X;
  if (g.NavMoveDir == Dir_Left &&
      (move_flags & (NavMoveFlags_WrapX | NavMoveFlags_LoopX))) {
    bb_rel.Min.x = bb_rel.Max.x =
        window->ContentSize.x + window->WindowPadding.x;
    if (move_flags & NavMoveFlags_WrapX) {
      bb_rel.TranslateY(-bb_rel.GetHeight()); // Previous row
      clip_dir = Dir_Up;
    }
    do_forward = true;
  }
  if (g.NavMoveDir == Dir_Right &&
      (move_flags & (NavMoveFlags_WrapX | NavMoveFlags_LoopX))) {
    bb_rel.Min.x = bb_rel.Max.x = -window->WindowPadding.x;
    if (move_flags & NavMoveFlags_WrapX) {
      bb_rel.TranslateY(+bb_rel.GetHeight()); // Next row
      clip_dir = Dir_Down;
    }
    do_forward = true;
  }
  if (g.NavMoveDir == Dir_Up &&
      (move_flags & (NavMoveFlags_WrapY | NavMoveFlags_LoopY))) {
    bb_rel.Min.y = bb_rel.Max.y =
        window->ContentSize.y + window->WindowPadding.y;
    if (move_flags & NavMoveFlags_WrapY) {
      bb_rel.TranslateX(-bb_rel.GetWidth()); // Previous column
      clip_dir = Dir_Left;
    }
    do_forward = true;
  }
  if (g.NavMoveDir == Dir_Down &&
      (move_flags & (NavMoveFlags_WrapY | NavMoveFlags_LoopY))) {
    bb_rel.Min.y = bb_rel.Max.y = -window->WindowPadding.y;
    if (move_flags & NavMoveFlags_WrapY) {
      bb_rel.TranslateX(+bb_rel.GetWidth()); // Next column
      clip_dir = Dir_Right;
    }
    do_forward = true;
  }
  if (!do_forward)
    return;
  window->NavRectRel[g.NavLayer] = bb_rel;
  NavClearPreferredPosForAxis(Axis_X);
  NavClearPreferredPosForAxis(Axis_Y);
  NavMoveRequestForward(g.NavMoveDir, clip_dir, move_flags,
                        g.NavMoveScrollFlags);
}

static int Gui::FindWindowFocusIndex(Window *window) {
  Context &g = *GGui;
  UNUSED(g);
  int order = window->FocusOrder;
  ASSERT(
      window->RootWindow ==
      window); // No child window (not testing _ChildWindow because of docking)
  ASSERT(g.WindowsFocusOrder[order] == window);
  return order;
}

static Window *FindWindowNavFocusable(int i_start, int i_stop,
                                      int dir) // FIXME-OPT O(N)
{
  Context &g = *GGui;
  for (int i = i_start; i >= 0 && i < g.WindowsFocusOrder.Size && i != i_stop;
       i += dir)
    if (Gui::IsWindowNavFocusable(g.WindowsFocusOrder[i]))
      return g.WindowsFocusOrder[i];
  return NULL;
}

static void NavUpdateWindowingHighlightWindow(int focus_change_dir) {
  Context &g = *GGui;
  ASSERT(g.NavWindowingTarget);
  if (g.NavWindowingTarget->Flags & WindowFlags_Modal)
    return;

  const int i_current = Gui::FindWindowFocusIndex(g.NavWindowingTarget);
  Window *window_target = FindWindowNavFocusable(i_current + focus_change_dir,
                                                 -INT_MAX, focus_change_dir);
  if (!window_target)
    window_target = FindWindowNavFocusable(
        (focus_change_dir < 0) ? (g.WindowsFocusOrder.Size - 1) : 0, i_current,
        focus_change_dir);
  if (window_target) // Don't reset windowing target if there's a single window
                     // in the list
  {
    g.NavWindowingTarget = g.NavWindowingTargetAnim = window_target;
    g.NavWindowingAccumDeltaPos = g.NavWindowingAccumDeltaSize =
        Vec2(0.0f, 0.0f);
  }
  g.NavWindowingToggleLayer = false;
}

// Windowing management mode
// Keyboard: CTRL+Tab (change focus/move/resize), Alt (toggle menu layer)
// Gamepad:  Hold Menu/Square (change focus/move/resize), Tap Menu/Square
// (toggle menu layer)
static void Gui::NavUpdateWindowing() {
  Context &g = *GGui;
  IO &io = g.IO;

  Window *apply_focus_window = NULL;
  bool apply_toggle_layer = false;

  Window *modal_window = GetTopMostPopupModal();
  bool allow_windowing =
      (modal_window ==
       NULL); // FIXME: This prevent CTRL+TAB from being usable with windows
              // that are inside the Begin-stack of that modal.
  if (!allow_windowing)
    g.NavWindowingTarget = NULL;

  // Fade out
  if (g.NavWindowingTargetAnim && g.NavWindowingTarget == NULL) {
    g.NavWindowingHighlightAlpha =
        Max(g.NavWindowingHighlightAlpha - io.DeltaTime * 10.0f, 0.0f);
    if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
      g.NavWindowingTargetAnim = NULL;
  }

  // Start CTRL+Tab or Square+L/R window selection
  const ID owner_id = HashStr("###NavUpdateWindowing");
  const bool nav_gamepad_active =
      (io.ConfigFlags & ConfigFlags_NavEnableGamepad) != 0 &&
      (io.BackendFlags & BackendFlags_HasGamepad) != 0;
  const bool nav_keyboard_active =
      (io.ConfigFlags & ConfigFlags_NavEnableKeyboard) != 0;
  const bool keyboard_next_window =
      allow_windowing && g.ConfigNavWindowingKeyNext &&
      Shortcut(g.ConfigNavWindowingKeyNext, owner_id,
               InputFlags_Repeat | InputFlags_RouteAlways);
  const bool keyboard_prev_window =
      allow_windowing && g.ConfigNavWindowingKeyPrev &&
      Shortcut(g.ConfigNavWindowingKeyPrev, owner_id,
               InputFlags_Repeat | InputFlags_RouteAlways);
  const bool start_windowing_with_gamepad =
      allow_windowing && nav_gamepad_active && !g.NavWindowingTarget &&
      IsKeyPressed(Key_NavGamepadMenu, 0, InputFlags_None);
  const bool start_windowing_with_keyboard =
      allow_windowing && !g.NavWindowingTarget &&
      (keyboard_next_window ||
       keyboard_prev_window); // Note: enabled even without NavEnableKeyboard!
  if (start_windowing_with_gamepad || start_windowing_with_keyboard)
    if (Window *window =
            g.NavWindow ? g.NavWindow
                        : FindWindowNavFocusable(g.WindowsFocusOrder.Size - 1,
                                                 -INT_MAX, -1)) {
      g.NavWindowingTarget = g.NavWindowingTargetAnim = window->RootWindow;
      g.NavWindowingTimer = g.NavWindowingHighlightAlpha = 0.0f;
      g.NavWindowingAccumDeltaPos = g.NavWindowingAccumDeltaSize =
          Vec2(0.0f, 0.0f);
      g.NavWindowingToggleLayer = start_windowing_with_gamepad
                                      ? true
                                      : false; // Gamepad starts toggling layer
      g.NavInputSource = start_windowing_with_keyboard ? InputSource_Keyboard
                                                       : InputSource_Gamepad;

      // Register ownership of our mods. Using InputFlags_RouteGlobalHigh
      // in the Shortcut() calls instead would probably be correct but may have
      // more side-effects.
      if (keyboard_next_window || keyboard_prev_window)
        SetKeyOwnersForKeyChord(
            (g.ConfigNavWindowingKeyNext | g.ConfigNavWindowingKeyPrev) &
                Mod_Mask_,
            owner_id);
    }

  // Gamepad update
  g.NavWindowingTimer += io.DeltaTime;
  if (g.NavWindowingTarget && g.NavInputSource == InputSource_Gamepad) {
    // Highlight only appears after a brief time holding the button, so that a
    // fast tap on PadMenu (to toggle NavLayer) doesn't add visual noise
    g.NavWindowingHighlightAlpha =
        Max(g.NavWindowingHighlightAlpha,
            Saturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) /
                     0.05f));

    // Select window to focus
    const int focus_change_dir =
        (int)IsKeyPressed(Key_GamepadL1) - (int)IsKeyPressed(Key_GamepadR1);
    if (focus_change_dir != 0) {
      NavUpdateWindowingHighlightWindow(focus_change_dir);
      g.NavWindowingHighlightAlpha = 1.0f;
    }

    // Single press toggles NavLayer, long press with L/R apply actual focus on
    // release (until then the window was merely rendered top-most)
    if (!IsKeyDown(Key_NavGamepadMenu)) {
      g.NavWindowingToggleLayer &=
          (g.NavWindowingHighlightAlpha <
           1.0f); // Once button was held long enough we don't consider it a
                  // tap-to-toggle-layer press anymore.
      if (g.NavWindowingToggleLayer && g.NavWindow)
        apply_toggle_layer = true;
      else if (!g.NavWindowingToggleLayer)
        apply_focus_window = g.NavWindowingTarget;
      g.NavWindowingTarget = NULL;
    }
  }

  // Keyboard: Focus
  if (g.NavWindowingTarget && g.NavInputSource == InputSource_Keyboard) {
    // Visuals only appears after a brief time after pressing TAB the first
    // time, so that a fast CTRL+TAB doesn't add visual noise
    KeyChord shared_mods =
        ((g.ConfigNavWindowingKeyNext ? g.ConfigNavWindowingKeyNext
                                      : Mod_Mask_) &
         (g.ConfigNavWindowingKeyPrev ? g.ConfigNavWindowingKeyPrev
                                      : Mod_Mask_)) &
        Mod_Mask_;
    ASSERT(
        shared_mods !=
        0); // Next/Prev shortcut currently needs a shared modifier to "hold",
            // otherwise Prev actions would keep cycling between two windows.
    g.NavWindowingHighlightAlpha =
        Max(g.NavWindowingHighlightAlpha,
            Saturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) /
                     0.05f)); // 1.0f
    if (keyboard_next_window || keyboard_prev_window)
      NavUpdateWindowingHighlightWindow(keyboard_next_window ? -1 : +1);
    else if ((io.KeyMods & shared_mods) != shared_mods)
      apply_focus_window = g.NavWindowingTarget;
  }

  // Keyboard: Press and Release ALT to toggle menu layer
  // - Testing that only Alt is tested prevents Alt+Shift or AltGR from toggling
  // menu layer.
  // - AltGR is normally Alt+Ctrl but we can't reliably detect it (not all
  // backends/systems/layout emit it as Alt+Ctrl). But even on keyboards without
  // AltGR we don't want Alt+Ctrl to open menu anyway.
  if (nav_keyboard_active && IsKeyPressed(Mod_Alt, KeyOwner_None)) {
    g.NavWindowingToggleLayer = true;
    g.NavInputSource = InputSource_Keyboard;
  }
  if (g.NavWindowingToggleLayer && g.NavInputSource == InputSource_Keyboard) {
    // We cancel toggling nav layer when any text has been typed (generally
    // while holding Alt). (See #370) We cancel toggling nav layer when other
    // modifiers are pressed. (See #4439) We cancel toggling nav layer if an
    // owner has claimed the key.
    if (io.InputQueueCharacters.Size > 0 || io.KeyCtrl || io.KeyShift ||
        io.KeySuper || TestKeyOwner(Mod_Alt, KeyOwner_None) == false)
      g.NavWindowingToggleLayer = false;

    // Apply layer toggle on release
    // Important: as before version <18314 we lacked an explicit IO event for
    // focus gain/loss, we also compare mouse validity to detect old backends
    // clearing mouse pos on focus loss.
    if (IsKeyReleased(Mod_Alt) && g.NavWindowingToggleLayer)
      if (g.ActiveId == 0 || g.ActiveIdAllowOverlap)
        if (IsMousePosValid(&io.MousePos) == IsMousePosValid(&io.MousePosPrev))
          apply_toggle_layer = true;
    if (!IsKeyDown(Mod_Alt))
      g.NavWindowingToggleLayer = false;
  }

  // Move window
  if (g.NavWindowingTarget &&
      !(g.NavWindowingTarget->Flags & WindowFlags_NoMove)) {
    Vec2 nav_move_dir;
    if (g.NavInputSource == InputSource_Keyboard && !io.KeyShift)
      nav_move_dir = GetKeyMagnitude2d(Key_LeftArrow, Key_RightArrow,
                                       Key_UpArrow, Key_DownArrow);
    if (g.NavInputSource == InputSource_Gamepad)
      nav_move_dir =
          GetKeyMagnitude2d(Key_GamepadLStickLeft, Key_GamepadLStickRight,
                            Key_GamepadLStickUp, Key_GamepadLStickDown);
    if (nav_move_dir.x != 0.0f || nav_move_dir.y != 0.0f) {
      const float NAV_MOVE_SPEED = 800.0f;
      const float move_step =
          NAV_MOVE_SPEED * io.DeltaTime *
          Min(io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
      g.NavWindowingAccumDeltaPos += nav_move_dir * move_step;
      g.NavDisableMouseHover = true;
      Vec2 accum_floored = Trunc(g.NavWindowingAccumDeltaPos);
      if (accum_floored.x != 0.0f || accum_floored.y != 0.0f) {
        Window *moving_window = g.NavWindowingTarget->RootWindowDockTree;
        SetWindowPos(moving_window, moving_window->Pos + accum_floored,
                     Cond_Always);
        g.NavWindowingAccumDeltaPos -= accum_floored;
      }
    }
  }

  // Apply final focus
  if (apply_focus_window &&
      (g.NavWindow == NULL || apply_focus_window != g.NavWindow->RootWindow)) {
    // FIXME: Many actions here could be part of a higher-level/reused function.
    // Why aren't they in FocusWindow() Investigate for each of them:
    // ClearActiveID(), NavRestoreHighlightAfterMove(),
    // NavRestoreLastChildNavWindow(), ClosePopupsOverWindow(), NavInitWindow()
    Viewport *previous_viewport = g.NavWindow ? g.NavWindow->Viewport : NULL;
    ClearActiveID();
    NavRestoreHighlightAfterMove();
    ClosePopupsOverWindow(apply_focus_window, false);
    FocusWindow(apply_focus_window, FocusRequestFlags_RestoreFocusedChild);
    apply_focus_window = g.NavWindow;
    if (apply_focus_window->NavLastIds[0] == 0)
      NavInitWindow(apply_focus_window, false);

    // If the window has ONLY a menu layer (no main layer), select it directly
    // Use NavLayersActiveMaskNext since windows didn't have a chance to be
    // Begin()-ed on this frame, so CTRL+Tab where the keys are only held for 1
    // frame will be able to use correct layers mask since the target window as
    // already been previewed once.
    // FIXME-NAV: This should be done in NavInit.. or in FocusWindow... However
    // in both of those cases, we won't have a guarantee that windows has been
    // visible before and therefore NavLayersActiveMask* won't be valid.
    if (apply_focus_window->DC.NavLayersActiveMaskNext == (1 << NavLayer_Menu))
      g.NavLayer = NavLayer_Menu;

    // Request OS level focus
    if (apply_focus_window->Viewport != previous_viewport &&
        g.PlatformIO.Platform_SetWindowFocus)
      g.PlatformIO.Platform_SetWindowFocus(apply_focus_window->Viewport);
  }
  if (apply_focus_window)
    g.NavWindowingTarget = NULL;

  // Apply menu/layer toggle
  if (apply_toggle_layer && g.NavWindow) {
    ClearActiveID();

    // Move to parent menu if necessary
    Window *new_nav_window = g.NavWindow;
    while (
        new_nav_window->ParentWindow &&
        (new_nav_window->DC.NavLayersActiveMask & (1 << NavLayer_Menu)) == 0 &&
        (new_nav_window->Flags & WindowFlags_ChildWindow) != 0 &&
        (new_nav_window->Flags & (WindowFlags_Popup | WindowFlags_ChildMenu)) ==
            0)
      new_nav_window = new_nav_window->ParentWindow;
    if (new_nav_window != g.NavWindow) {
      Window *old_nav_window = g.NavWindow;
      FocusWindow(new_nav_window);
      new_nav_window->NavLastChildNavWindow = old_nav_window;
    }

    // Toggle layer
    const NavLayer new_nav_layer =
        (g.NavWindow->DC.NavLayersActiveMask & (1 << NavLayer_Menu))
            ? (NavLayer)((int)g.NavLayer ^ 1)
            : NavLayer_Main;
    if (new_nav_layer != g.NavLayer) {
      // Reinitialize navigation when entering menu bar with the Alt key (FIXME:
      // could be a properly of the layer?)
      const bool preserve_layer_1_nav_id =
          (new_nav_window->DockNodeAsHost != NULL);
      if (new_nav_layer == NavLayer_Menu && !preserve_layer_1_nav_id)
        g.NavWindow->NavLastIds[new_nav_layer] = 0;
      NavRestoreLayer(new_nav_layer);
      NavRestoreHighlightAfterMove();
    }
  }
}

// Window has already passed the IsWindowNavFocusable()
static const char *GetFallbackWindowNameForWindowingList(Window *window) {
  if (window->Flags & WindowFlags_Popup)
    return Gui::LocalizeGetMsg(LocKey_WindowingPopup);
  if ((window->Flags & WindowFlags_MenuBar) &&
      strcmp(window->Name, "##MainMenuBar") == 0)
    return Gui::LocalizeGetMsg(LocKey_WindowingMainMenuBar);
  if (window->DockNodeAsHost)
    return "(Dock node)"; // Not normally shown to user.
  return Gui::LocalizeGetMsg(LocKey_WindowingUntitled);
}

// Overlay displayed when using CTRL+TAB. Called by EndFrame().
void Gui::NavUpdateWindowingOverlay() {
  Context &g = *GGui;
  ASSERT(g.NavWindowingTarget != NULL);

  if (g.NavWindowingTimer < NAV_WINDOWING_LIST_APPEAR_DELAY)
    return;

  if (g.NavWindowingListWindow == NULL)
    g.NavWindowingListWindow = FindWindowByName("###NavWindowingList");
  const Viewport *viewport =
      /*g.NavWindow ? g.NavWindow->Viewport :*/ GetMainViewport();
  SetNextWindowSizeConstraints(
      Vec2(viewport->Size.x * 0.20f, viewport->Size.y * 0.20f),
      Vec2(FLT_MAX, FLT_MAX));
  SetNextWindowPos(viewport->GetCenter(), Cond_Always, Vec2(0.5f, 0.5f));
  PushStyleVar(StyleVar_WindowPadding, g.Style.WindowPadding * 2.0f);
  Begin("###NavWindowingList", NULL,
        WindowFlags_NoTitleBar | WindowFlags_NoFocusOnAppearing |
            WindowFlags_NoResize | WindowFlags_NoMove | WindowFlags_NoInputs |
            WindowFlags_AlwaysAutoResize | WindowFlags_NoSavedSettings);
  for (int n = g.WindowsFocusOrder.Size - 1; n >= 0; n--) {
    Window *window = g.WindowsFocusOrder[n];
    ASSERT(window != NULL); // Fix static analyzers
    if (!IsWindowNavFocusable(window))
      continue;
    const char *label = window->Name;
    if (label == FindRenderedTextEnd(label))
      label = GetFallbackWindowNameForWindowingList(window);
    Selectable(label, g.NavWindowingTarget == window);
  }
  End();
  PopStyleVar();
}

//-----------------------------------------------------------------------------
// [SECTION] DRAG AND DROP
//-----------------------------------------------------------------------------

bool Gui::IsDragDropActive() {
  Context &g = *GGui;
  return g.DragDropActive;
}

void Gui::ClearDragDrop() {
  Context &g = *GGui;
  g.DragDropActive = false;
  g.DragDropPayload.Clear();
  g.DragDropAcceptFlags = DragDropFlags_None;
  g.DragDropAcceptIdCurr = g.DragDropAcceptIdPrev = 0;
  g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
  g.DragDropAcceptFrameCount = -1;

  g.DragDropPayloadBufHeap.clear();
  memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
}

bool Gui::BeginTooltipHidden() {
  Context &g = *GGui;
  bool ret = Begin("##Tooltip_Hidden", NULL,
                   WindowFlags_Tooltip | WindowFlags_NoInputs |
                       WindowFlags_NoTitleBar | WindowFlags_NoMove |
                       WindowFlags_NoResize | WindowFlags_NoSavedSettings |
                       WindowFlags_AlwaysAutoResize);
  SetWindowHiddenAndSkipItemsForCurrentFrame(g.CurrentWindow);
  return ret;
}

// When this returns true you need to: a) call SetDragDropPayload() exactly
// once, b) you may render the payload visual/description, c) call
// EndDragDropSource() If the item has an identifier:
// - This assume/require the item to be activated (typically via
// ButtonBehavior).
// - Therefore if you want to use this with a mouse button other than left mouse
// button, it is up to the item itself to activate with another button.
// - We then pull and use the mouse button that was used to activate the item
// and use it to carry on the drag. If the item has no identifier:
// - Currently always assume left mouse button.
bool Gui::BeginDragDropSource(DragDropFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  // FIXME-DRAGDROP: While in the common-most "drag from non-zero active id"
  // case we can tell the mouse button, in both SourceExtern and id==0 cases we
  // may requires something else (explicit flags or some heuristic).
  MouseButton mouse_button = MouseButton_Left;

  bool source_drag_active = false;
  ID source_id = 0;
  ID source_parent_id = 0;
  if (!(flags & DragDropFlags_SourceExtern)) {
    source_id = g.LastItemData.ID;
    if (source_id != 0) {
      // Common path: items with ID
      if (g.ActiveId != source_id)
        return false;
      if (g.ActiveIdMouseButton != -1)
        mouse_button = g.ActiveIdMouseButton;
      if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
        return false;
      g.ActiveIdAllowOverlap = false;
    } else {
      // Uncommon path: items without ID
      if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
        return false;
      if ((g.LastItemData.StatusFlags & ItemStatusFlags_HoveredRect) == 0 &&
          (g.ActiveId == 0 || g.ActiveIdWindow != window))
        return false;

      // If you want to use BeginDragDropSource() on an item with no unique
      // identifier for interaction, such as Text() or Image(), you need to: A)
      // Read the explanation below, B) Use the
      // DragDropFlags_SourceAllowNullID flag.
      if (!(flags & DragDropFlags_SourceAllowNullID)) {
        ASSERT(0);
        return false;
      }

      // Magic fallback to handle items with no assigned ID, e.g. Text(),
      // Image() We build a throwaway ID based on current ID stack + relative
      // AABB of items in window. THE IDENTIFIER WON'T SURVIVE ANY
      // REPOSITIONING/RESIZINGG OF THE WIDGET, so if your widget moves your
      // dragging operation will be canceled. We don't need to maintain/call
      // ClearActiveID() as releasing the button will early out this function
      // and trigger !ActiveIdIsAlive. Rely on keeping other window->LastItemXXX
      // fields intact.
      source_id = g.LastItemData.ID =
          window->GetIDFromRectangle(g.LastItemData.Rect);
      KeepAliveID(source_id);
      bool is_hovered =
          ItemHoverable(g.LastItemData.Rect, source_id, g.LastItemData.InFlags);
      if (is_hovered && g.IO.MouseClicked[mouse_button]) {
        SetActiveID(source_id, window);
        FocusWindow(window);
      }
      if (g.ActiveId ==
          source_id) // Allow the underlying widget to display/return hovered
                     // during the mouse release frame, else we would get a
                     // flicker.
        g.ActiveIdAllowOverlap = is_hovered;
    }
    if (g.ActiveId != source_id)
      return false;
    source_parent_id = window->IDStack.back();
    source_drag_active = IsMouseDragging(mouse_button);

    // Disable navigation and key inputs while dragging + cancel existing
    // request if any
    SetActiveIdUsingAllKeyboardKeys();
  } else {
    window = NULL;
    source_id = HashStr("#SourceExtern");
    source_drag_active = true;
  }

  if (source_drag_active) {
    if (!g.DragDropActive) {
      ASSERT(source_id != 0);
      ClearDragDrop();
      Payload &payload = g.DragDropPayload;
      payload.SourceId = source_id;
      payload.SourceParentId = source_parent_id;
      g.DragDropActive = true;
      g.DragDropSourceFlags = flags;
      g.DragDropMouseButton = mouse_button;
      if (payload.SourceId == g.ActiveId)
        g.ActiveIdNoClearOnFocusLoss = true;
    }
    g.DragDropSourceFrameCount = g.FrameCount;
    g.DragDropWithinSource = true;

    if (!(flags & DragDropFlags_SourceNoPreviewTooltip)) {
      // Target can request the Source to not display its tooltip (we use a
      // dedicated flag to make this request explicit) We unfortunately can't
      // just modify the source flags and skip the call to BeginTooltip, as
      // caller may be emitting contents.
      bool ret;
      if (g.DragDropAcceptIdPrev &&
          (g.DragDropAcceptFlags & DragDropFlags_AcceptNoPreviewTooltip))
        ret = BeginTooltipHidden();
      else
        ret = BeginTooltip();
      ASSERT(ret); // FIXME-NEWBEGIN: If this ever becomes false, we need to
                   // Begin("##Hidden", NULL, WindowFlags_NoSavedSettings)
                   // + SetWindowHiddendAndSkipItemsForCurrentFrame().
      UNUSED(ret);
    }

    if (!(flags & DragDropFlags_SourceNoDisableHover) &&
        !(flags & DragDropFlags_SourceExtern))
      g.LastItemData.StatusFlags &= ~ItemStatusFlags_HoveredRect;

    return true;
  }
  return false;
}

void Gui::EndDragDropSource() {
  Context &g = *GGui;
  ASSERT(g.DragDropActive);
  ASSERT(g.DragDropWithinSource && "Not after a BeginDragDropSource()?");

  if (!(g.DragDropSourceFlags & DragDropFlags_SourceNoPreviewTooltip))
    EndTooltip();

  // Discard the drag if have not called SetDragDropPayload()
  if (g.DragDropPayload.DataFrameCount == -1)
    ClearDragDrop();
  g.DragDropWithinSource = false;
}

// Use 'cond' to choose to submit payload on drag start or every frame
bool Gui::SetDragDropPayload(const char *type, const void *data,
                             size_t data_size, Cond cond) {
  Context &g = *GGui;
  Payload &payload = g.DragDropPayload;
  if (cond == 0)
    cond = Cond_Always;

  ASSERT(type != NULL);
  ASSERT(strlen(type) < ARRAYSIZE(payload.DataType) &&
         "Payload type can be at most 32 characters long");
  ASSERT((data != NULL && data_size > 0) || (data == NULL && data_size == 0));
  ASSERT(cond == Cond_Always || cond == Cond_Once);
  ASSERT(payload.SourceId !=
         0); // Not called between BeginDragDropSource() and EndDragDropSource()

  if (cond == Cond_Always || payload.DataFrameCount == -1) {
    // Copy payload
    Strncpy(payload.DataType, type, ARRAYSIZE(payload.DataType));
    g.DragDropPayloadBufHeap.resize(0);
    if (data_size > sizeof(g.DragDropPayloadBufLocal)) {
      // Store in heap
      g.DragDropPayloadBufHeap.resize((int)data_size);
      payload.Data = g.DragDropPayloadBufHeap.Data;
      memcpy(payload.Data, data, data_size);
    } else if (data_size > 0) {
      // Store locally
      memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
      payload.Data = g.DragDropPayloadBufLocal;
      memcpy(payload.Data, data, data_size);
    } else {
      payload.Data = NULL;
    }
    payload.DataSize = (int)data_size;
  }
  payload.DataFrameCount = g.FrameCount;

  // Return whether the payload has been accepted
  return (g.DragDropAcceptFrameCount == g.FrameCount) ||
         (g.DragDropAcceptFrameCount == g.FrameCount - 1);
}

bool Gui::BeginDragDropTargetCustom(const Rect &bb, ID id) {
  Context &g = *GGui;
  if (!g.DragDropActive)
    return false;

  Window *window = g.CurrentWindow;
  Window *hovered_window = g.HoveredWindowUnderMovingWindow;
  if (hovered_window == NULL ||
      window->RootWindowDockTree != hovered_window->RootWindowDockTree)
    return false;
  ASSERT(id != 0);
  if (!IsMouseHoveringRect(bb.Min, bb.Max) ||
      (id == g.DragDropPayload.SourceId))
    return false;
  if (window->SkipItems)
    return false;

  ASSERT(g.DragDropWithinTarget == false);
  g.DragDropTargetRect = bb;
  g.DragDropTargetClipRect =
      window
          ->ClipRect; // May want to be overriden by user depending on use case?
  g.DragDropTargetId = id;
  g.DragDropWithinTarget = true;
  return true;
}

// We don't use BeginDragDropTargetCustom() and duplicate its code because:
// 1) we use LastItemData's ItemStatusFlags_HoveredRect which handles items
// that push a temporarily clip rectangle in their code. Calling
// BeginDragDropTargetCustom(LastItemRect) would not handle them. 2) and it's
// faster. as this code may be very frequently called, we want to early out as
// fast as we can. Also note how the HoveredWindow test is positioned
// differently in both functions (in both functions we optimize for the cheapest
// early out case)
bool Gui::BeginDragDropTarget() {
  Context &g = *GGui;
  if (!g.DragDropActive)
    return false;

  Window *window = g.CurrentWindow;
  if (!(g.LastItemData.StatusFlags & ItemStatusFlags_HoveredRect))
    return false;
  Window *hovered_window = g.HoveredWindowUnderMovingWindow;
  if (hovered_window == NULL ||
      window->RootWindowDockTree != hovered_window->RootWindowDockTree ||
      window->SkipItems)
    return false;

  const Rect &display_rect =
      (g.LastItemData.StatusFlags & ItemStatusFlags_HasDisplayRect)
          ? g.LastItemData.DisplayRect
          : g.LastItemData.Rect;
  ID id = g.LastItemData.ID;
  if (id == 0) {
    id = window->GetIDFromRectangle(display_rect);
    KeepAliveID(id);
  }
  if (g.DragDropPayload.SourceId == id)
    return false;

  ASSERT(g.DragDropWithinTarget == false);
  g.DragDropTargetRect = display_rect;
  g.DragDropTargetClipRect =
      (g.LastItemData.StatusFlags & ItemStatusFlags_HasClipRect)
          ? g.LastItemData.ClipRect
          : window->ClipRect;
  g.DragDropTargetId = id;
  g.DragDropWithinTarget = true;
  return true;
}

bool Gui::IsDragDropPayloadBeingAccepted() {
  Context &g = *GGui;
  return g.DragDropActive && g.DragDropAcceptIdPrev != 0;
}

const Payload *Gui::AcceptDragDropPayload(const char *type,
                                          DragDropFlags flags) {
  Context &g = *GGui;
  Payload &payload = g.DragDropPayload;
  ASSERT(g.DragDropActive); // Not called between BeginDragDropTarget() and
                            // EndDragDropTarget() ?
  ASSERT(payload.DataFrameCount != -1); // Forgot to call EndDragDropTarget() ?
  if (type != NULL && !payload.IsDataType(type))
    return NULL;

  // Accept smallest drag target bounding box, this allows us to nest drag
  // targets conveniently without ordering constraints. NB: We currently accept
  // NULL id as target. However, overlapping targets requires a unique ID to
  // function!
  const bool was_accepted_previously =
      (g.DragDropAcceptIdPrev == g.DragDropTargetId);
  Rect r = g.DragDropTargetRect;
  float r_surface = r.GetWidth() * r.GetHeight();
  if (r_surface > g.DragDropAcceptIdCurrRectSurface)
    return NULL;

  g.DragDropAcceptFlags = flags;
  g.DragDropAcceptIdCurr = g.DragDropTargetId;
  g.DragDropAcceptIdCurrRectSurface = r_surface;
  // DEBUG_LOG("AcceptDragDropPayload(): %08X: accept\n", g.DragDropTargetId);

  // Render default drop visuals
  payload.Preview = was_accepted_previously;
  flags |= (g.DragDropSourceFlags &
            DragDropFlags_AcceptNoDrawDefaultRect); // Source can also inhibit
                                                    // the preview (useful for
                                                    // external sources that
                                                    // live for 1 frame)
  if (!(flags & DragDropFlags_AcceptNoDrawDefaultRect) && payload.Preview)
    RenderDragDropTargetRect(r, g.DragDropTargetClipRect);

  g.DragDropAcceptFrameCount = g.FrameCount;
  payload.Delivery =
      was_accepted_previously &&
      !IsMouseDown(
          g.DragDropMouseButton); // For extern drag sources affecting OS window
                                  // focus, it's easier to just test
                                  // !IsMouseDown() instead of IsMouseReleased()
  if (!payload.Delivery && !(flags & DragDropFlags_AcceptBeforeDelivery))
    return NULL;

  // DEBUG_LOG("AcceptDragDropPayload(): %08X: return payload\n",
  // g.DragDropTargetId);
  return &payload;
}

// FIXME-STYLE FIXME-DRAGDROP: Settle on a proper default visuals for drop
// target.
void Gui::RenderDragDropTargetRect(const Rect &bb, const Rect &item_clip_rect) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Rect bb_display = bb;
  bb_display.ClipWith(
      item_clip_rect); // Clip THEN expand so we have a way to visualize that
                       // target is not entirely visible.
  bb_display.Expand(3.5f);
  bool push_clip_rect = !window->ClipRect.Contains(bb_display);
  if (push_clip_rect)
    window->DrawList->PushClipRectFullScreen();
  window->DrawList->AddRect(bb_display.Min, bb_display.Max,
                            GetColorU32(Col_DragDropTarget), 0.0f, 0, 2.0f);
  if (push_clip_rect)
    window->DrawList->PopClipRect();
}

const Payload *Gui::GetDragDropPayload() {
  Context &g = *GGui;
  return (g.DragDropActive && g.DragDropPayload.DataFrameCount != -1)
             ? &g.DragDropPayload
             : NULL;
}

void Gui::EndDragDropTarget() {
  Context &g = *GGui;
  ASSERT(g.DragDropActive);
  ASSERT(g.DragDropWithinTarget);
  g.DragDropWithinTarget = false;

  // Clear drag and drop state payload right after delivery
  if (g.DragDropPayload.Delivery)
    ClearDragDrop();
}

//-----------------------------------------------------------------------------
// [SECTION] LOGGING/CAPTURING
//-----------------------------------------------------------------------------
// All text output from the interface can be captured into tty/file/clipboard.
// By default, tree nodes are automatically opened during logging.
//-----------------------------------------------------------------------------

// Pass text data straight to log (without being displayed)
static inline void LogTextV(Context &g, const char *fmt, va_list args) {
  if (g.LogFile) {
    g.LogBuffer.Buf.resize(0);
    g.LogBuffer.appendfv(fmt, args);
    FileWrite(g.LogBuffer.c_str(), sizeof(char), (U64)g.LogBuffer.size(),
              g.LogFile);
  } else {
    g.LogBuffer.appendfv(fmt, args);
  }
}

void Gui::LogText(const char *fmt, ...) {
  Context &g = *GGui;
  if (!g.LogEnabled)
    return;

  va_list args;
  va_start(args, fmt);
  LogTextV(g, fmt, args);
  va_end(args);
}

void Gui::LogTextV(const char *fmt, va_list args) {
  Context &g = *GGui;
  if (!g.LogEnabled)
    return;

  LogTextV(g, fmt, args);
}

// Internal version that takes a position to decide on newline placement and pad
// items according to their depth. We split text into individual lines to add
// current tree level padding
// FIXME: This code is a little complicated perhaps, considering simplifying the
// whole system.
void Gui::LogRenderedText(const Vec2 *ref_pos, const char *text,
                          const char *text_end) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  const char *prefix = g.LogNextPrefix;
  const char *suffix = g.LogNextSuffix;
  g.LogNextPrefix = g.LogNextSuffix = NULL;

  if (!text_end)
    text_end = FindRenderedTextEnd(text, text_end);

  const bool log_new_line =
      ref_pos && (ref_pos->y > g.LogLinePosY + g.Style.FramePadding.y + 1);
  if (ref_pos)
    g.LogLinePosY = ref_pos->y;
  if (log_new_line) {
    LogText(NEWLINE);
    g.LogLineFirstItem = true;
  }

  if (prefix)
    LogRenderedText(ref_pos, prefix,
                    prefix + strlen(prefix)); // Calculate end ourself to ensure
                                              // "##" are included here.

  // Re-adjust padding if we have popped out of our starting depth
  if (g.LogDepthRef > window->DC.TreeDepth)
    g.LogDepthRef = window->DC.TreeDepth;
  const int tree_depth = (window->DC.TreeDepth - g.LogDepthRef);

  const char *text_remaining = text;
  for (;;) {
    // Split the string. Each new line (after a '\n') is followed by indentation
    // corresponding to the current depth of our log entry. We don't add a
    // trailing \n yet to allow a subsequent item on the same line to be
    // captured.
    const char *line_start = text_remaining;
    const char *line_end = StreolRange(line_start, text_end);
    const bool is_last_line = (line_end == text_end);
    if (line_start != line_end || !is_last_line) {
      const int line_length = (int)(line_end - line_start);
      const int indentation = g.LogLineFirstItem ? tree_depth * 4 : 1;
      LogText("%*s%.*s", indentation, "", line_length, line_start);
      g.LogLineFirstItem = false;
      if (*line_end == '\n') {
        LogText(NEWLINE);
        g.LogLineFirstItem = true;
      }
    }
    if (is_last_line)
      break;
    text_remaining = line_end + 1;
  }

  if (suffix)
    LogRenderedText(ref_pos, suffix, suffix + strlen(suffix));
}

// Start logging/capturing text output
void Gui::LogBegin(LogType type, int auto_open_depth) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT(g.LogEnabled == false);
  ASSERT(g.LogFile == NULL);
  ASSERT(g.LogBuffer.empty());
  g.LogEnabled = true;
  g.LogType = type;
  g.LogNextPrefix = g.LogNextSuffix = NULL;
  g.LogDepthRef = window->DC.TreeDepth;
  g.LogDepthToExpand =
      ((auto_open_depth >= 0) ? auto_open_depth : g.LogDepthToExpandDefault);
  g.LogLinePosY = FLT_MAX;
  g.LogLineFirstItem = true;
}

// Important: doesn't copy underlying data, use carefully (prefix/suffix must be
// in scope at the time of the next LogRenderedText)
void Gui::LogSetNextTextDecoration(const char *prefix, const char *suffix) {
  Context &g = *GGui;
  g.LogNextPrefix = prefix;
  g.LogNextSuffix = suffix;
}

void Gui::LogToTTY(int auto_open_depth) {
  Context &g = *GGui;
  if (g.LogEnabled)
    return;
  UNUSED(auto_open_depth);
#ifndef DISABLE_TTY_FUNCTIONS
  LogBegin(LogType_TTY, auto_open_depth);
  g.LogFile = stdout;
#endif
}

// Start logging/capturing text output to given file
void Gui::LogToFile(int auto_open_depth, const char *filename) {
  Context &g = *GGui;
  if (g.LogEnabled)
    return;

  // FIXME: We could probably open the file in text mode "at", however note that
  // clipboard/buffer logging will still be subject to outputting
  // OS-incompatible carriage return if within strings the user doesn't use
  // NEWLINE. By opening the file in binary mode "ab" we have consistent
  // output everywhere.
  if (!filename)
    filename = g.IO.LogFilename;
  if (!filename || !filename[0])
    return;
  FileHandle f = FileOpen(filename, "ab");
  if (!f) {
    ASSERT(0);
    return;
  }

  LogBegin(LogType_File, auto_open_depth);
  g.LogFile = f;
}

// Start logging/capturing text output to clipboard
void Gui::LogToClipboard(int auto_open_depth) {
  Context &g = *GGui;
  if (g.LogEnabled)
    return;
  LogBegin(LogType_Clipboard, auto_open_depth);
}

void Gui::LogToBuffer(int auto_open_depth) {
  Context &g = *GGui;
  if (g.LogEnabled)
    return;
  LogBegin(LogType_Buffer, auto_open_depth);
}

void Gui::LogFinish() {
  Context &g = *GGui;
  if (!g.LogEnabled)
    return;

  LogText(NEWLINE);
  switch (g.LogType) {
  case LogType_TTY:
#ifndef DISABLE_TTY_FUNCTIONS
    fflush(g.LogFile);
#endif
    break;
  case LogType_File:
    FileClose(g.LogFile);
    break;
  case LogType_Buffer:
    break;
  case LogType_Clipboard:
    if (!g.LogBuffer.empty())
      SetClipboardText(g.LogBuffer.begin());
    break;
  case LogType_None:
    ASSERT(0);
    break;
  }

  g.LogEnabled = false;
  g.LogType = LogType_None;
  g.LogFile = NULL;
  g.LogBuffer.clear();
}

// Helper to display logging buttons
// FIXME-OBSOLETE: We should probably obsolete this and let the user have their
// own helper (this is one of the oldest function alive!)
void Gui::LogButtons() {
  Context &g = *GGui;

  PushID("LogButtons");
#ifndef DISABLE_TTY_FUNCTIONS
  const bool log_to_tty = Button("Log To TTY");
  SameLine();
#else
  const bool log_to_tty = false;
#endif
  const bool log_to_file = Button("Log To File");
  SameLine();
  const bool log_to_clipboard = Button("Log To Clipboard");
  SameLine();
  PushTabStop(false);
  SetNextItemWidth(80.0f);
  SliderInt("Default Depth", &g.LogDepthToExpandDefault, 0, 9, NULL);
  PopTabStop();
  PopID();

  // Start logging at the end of the function so that the buttons don't appear
  // in the log
  if (log_to_tty)
    LogToTTY();
  if (log_to_file)
    LogToFile();
  if (log_to_clipboard)
    LogToClipboard();
}

//-----------------------------------------------------------------------------
// [SECTION] SETTINGS
//-----------------------------------------------------------------------------
// - UpdateSettings() [Internal]
// - MarkIniSettingsDirty() [Internal]
// - FindSettingsHandler() [Internal]
// - ClearIniSettings() [Internal]
// - LoadIniSettingsFromDisk()
// - LoadIniSettingsFromMemory()
// - SaveIniSettingsToDisk()
// - SaveIniSettingsToMemory()
//-----------------------------------------------------------------------------
// - CreateNewWindowSettings() [Internal]
// - FindWindowSettingsByID() [Internal]
// - FindWindowSettingsByWindow() [Internal]
// - ClearWindowSettings() [Internal]
// - WindowSettingsHandler_***() [Internal]
//-----------------------------------------------------------------------------

// Called by NewFrame()
void Gui::UpdateSettings() {
  // Load settings on first frame (if not explicitly loaded manually before)
  Context &g = *GGui;
  if (!g.SettingsLoaded) {
    ASSERT(g.SettingsWindows.empty());
    if (g.IO.IniFilename)
      LoadIniSettingsFromDisk(g.IO.IniFilename);
    g.SettingsLoaded = true;
  }

  // Save settings (with a delay after the last modification, so we don't spam
  // disk too much)
  if (g.SettingsDirtyTimer > 0.0f) {
    g.SettingsDirtyTimer -= g.IO.DeltaTime;
    if (g.SettingsDirtyTimer <= 0.0f) {
      if (g.IO.IniFilename != NULL)
        SaveIniSettingsToDisk(g.IO.IniFilename);
      else
        g.IO.WantSaveIniSettings =
            true; // Let user know they can call SaveIniSettingsToMemory(). user
                  // will need to clear io.WantSaveIniSettings themselves.
      g.SettingsDirtyTimer = 0.0f;
    }
  }
}

void Gui::MarkIniSettingsDirty() {
  Context &g = *GGui;
  if (g.SettingsDirtyTimer <= 0.0f)
    g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void Gui::MarkIniSettingsDirty(Window *window) {
  Context &g = *GGui;
  if (!(window->Flags & WindowFlags_NoSavedSettings))
    if (g.SettingsDirtyTimer <= 0.0f)
      g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void Gui::AddSettingsHandler(const SettingsHandler *handler) {
  Context &g = *GGui;
  ASSERT(FindSettingsHandler(handler->TypeName) == NULL);
  g.SettingsHandlers.push_back(*handler);
}

void Gui::RemoveSettingsHandler(const char *type_name) {
  Context &g = *GGui;
  if (SettingsHandler *handler = FindSettingsHandler(type_name))
    g.SettingsHandlers.erase(handler);
}

SettingsHandler *Gui::FindSettingsHandler(const char *type_name) {
  Context &g = *GGui;
  const ID type_hash = HashStr(type_name);
  for (SettingsHandler &handler : g.SettingsHandlers)
    if (handler.TypeHash == type_hash)
      return &handler;
  return NULL;
}

// Clear all settings (windows, tables, docking etc.)
void Gui::ClearIniSettings() {
  Context &g = *GGui;
  g.SettingsIniData.clear();
  for (SettingsHandler &handler : g.SettingsHandlers)
    if (handler.ClearAllFn != NULL)
      handler.ClearAllFn(&g, &handler);
}

void Gui::LoadIniSettingsFromDisk(const char *ini_filename) {
  size_t file_data_size = 0;
  char *file_data =
      (char *)FileLoadToMemory(ini_filename, "rb", &file_data_size);
  if (!file_data)
    return;
  if (file_data_size > 0)
    LoadIniSettingsFromMemory(file_data, (size_t)file_data_size);
  FREE(file_data);
}

// Zero-tolerance, no error reporting, cheap .ini parsing
// Set ini_size==0 to let us use strlen(ini_data). Do not call this function
// with a 0 if your buffer is actually empty!
void Gui::LoadIniSettingsFromMemory(const char *ini_data, size_t ini_size) {
  Context &g = *GGui;
  ASSERT(g.Initialized);
  // ASSERT(!g.WithinFrameScope && "Cannot be called between NewFrame() and
  // EndFrame()"); ASSERT(g.SettingsLoaded == false && g.FrameCount == 0);

  // For user convenience, we allow passing a non zero-terminated string (hence
  // the ini_size parameter). For our convenience and to make the code simpler,
  // we'll also write zero-terminators within the buffer. So let's create a
  // writable copy..
  if (ini_size == 0)
    ini_size = strlen(ini_data);
  g.SettingsIniData.Buf.resize((int)ini_size + 1);
  char *const buf = g.SettingsIniData.Buf.Data;
  char *const buf_end = buf + ini_size;
  memcpy(buf, ini_data, ini_size);
  buf_end[0] = 0;

  // Call pre-read handlers
  // Some types will clear their data (e.g. dock information) some types will
  // allow merge/override (window)
  for (SettingsHandler &handler : g.SettingsHandlers)
    if (handler.ReadInitFn != NULL)
      handler.ReadInitFn(&g, &handler);

  void *entry_data = NULL;
  SettingsHandler *entry_handler = NULL;

  char *line_end = NULL;
  for (char *line = buf; line < buf_end; line = line_end + 1) {
    // Skip new lines markers, then find end of the line
    while (*line == '\n' || *line == '\r')
      line++;
    line_end = line;
    while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
      line_end++;
    line_end[0] = 0;
    if (line[0] == ';')
      continue;
    if (line[0] == '[' && line_end > line && line_end[-1] == ']') {
      // Parse "[Type][Name]". Note that 'Name' can itself contains []
      // characters, which is acceptable with the current format and parsing
      // code.
      line_end[-1] = 0;
      const char *name_end = line_end - 1;
      const char *type_start = line + 1;
      char *type_end = (char *)(void *)StrchrRange(type_start, name_end, ']');
      const char *name_start =
          type_end ? StrchrRange(type_end + 1, name_end, '[') : NULL;
      if (!type_end || !name_start)
        continue;
      *type_end = 0; // Overwrite first ']'
      name_start++;  // Skip second '['
      entry_handler = FindSettingsHandler(type_start);
      entry_data = entry_handler ? entry_handler->ReadOpenFn(&g, entry_handler,
                                                             name_start)
                                 : NULL;
    } else if (entry_handler != NULL && entry_data != NULL) {
      // Let type handler parse the line
      entry_handler->ReadLineFn(&g, entry_handler, entry_data, line);
    }
  }
  g.SettingsLoaded = true;

  // [DEBUG] Restore untouched copy so it can be browsed in Metrics (not
  // strictly necessary)
  memcpy(buf, ini_data, ini_size);

  // Call post-read handlers
  for (SettingsHandler &handler : g.SettingsHandlers)
    if (handler.ApplyAllFn != NULL)
      handler.ApplyAllFn(&g, &handler);
}

void Gui::SaveIniSettingsToDisk(const char *ini_filename) {
  Context &g = *GGui;
  g.SettingsDirtyTimer = 0.0f;
  if (!ini_filename)
    return;

  size_t ini_data_size = 0;
  const char *ini_data = SaveIniSettingsToMemory(&ini_data_size);
  FileHandle f = FileOpen(ini_filename, "wt");
  if (!f)
    return;
  FileWrite(ini_data, sizeof(char), ini_data_size, f);
  FileClose(f);
}

// Call registered handlers (e.g. SettingsHandlerWindow_WriteAll() + custom
// handlers) to write their stuff into a text buffer
const char *Gui::SaveIniSettingsToMemory(size_t *out_size) {
  Context &g = *GGui;
  g.SettingsDirtyTimer = 0.0f;
  g.SettingsIniData.Buf.resize(0);
  g.SettingsIniData.Buf.push_back(0);
  for (SettingsHandler &handler : g.SettingsHandlers)
    handler.WriteAllFn(&g, &handler, &g.SettingsIniData);
  if (out_size)
    *out_size = (size_t)g.SettingsIniData.size();
  return g.SettingsIniData.c_str();
}

WindowSettings *Gui::CreateNewWindowSettings(const char *name) {
  Context &g = *GGui;

  if (g.IO.ConfigDebugIniSettings == false) {
    // Skip to the "###" marker if any. We don't skip past to match the behavior
    // of GetID() Preserve the full string when ConfigDebugVerboseIniSettings is
    // set to make .ini inspection easier.
    if (const char *p = strstr(name, "###"))
      name = p;
  }
  const size_t name_len = strlen(name);

  // Allocate chunk
  const size_t chunk_size = sizeof(WindowSettings) + name_len + 1;
  WindowSettings *settings = g.SettingsWindows.alloc_chunk(chunk_size);
  PLACEMENT_NEW(settings) WindowSettings();
  settings->ID = HashStr(name, name_len);
  memcpy(settings->GetName(), name, name_len + 1); // Store with zero terminator

  return settings;
}

// We don't provide a FindWindowSettingsByName() because Docking system doesn't
// always hold on names. This is called once per window .ini entry + once per
// newly instantiated window.
WindowSettings *Gui::FindWindowSettingsByID(ID id) {
  Context &g = *GGui;
  for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
       settings = g.SettingsWindows.next_chunk(settings))
    if (settings->ID == id && !settings->WantDelete)
      return settings;
  return NULL;
}

// This is faster if you are holding on a Window already as we don't need to
// perform a search.
WindowSettings *Gui::FindWindowSettingsByWindow(Window *window) {
  Context &g = *GGui;
  if (window->SettingsOffset != -1)
    return g.SettingsWindows.ptr_from_offset(window->SettingsOffset);
  return FindWindowSettingsByID(window->ID);
}

// This will revert window to its initial state, including enabling the
// Cond_FirstUseEver/Cond_Once conditions once more.
void Gui::ClearWindowSettings(const char *name) {
  // DEBUG_LOG("ClearWindowSettings('%s')\n", name);
  Context &g = *GGui;
  Window *window = FindWindowByName(name);
  if (window != NULL) {
    window->Flags |= WindowFlags_NoSavedSettings;
    InitOrLoadWindowSettings(window, NULL);
    if (window->DockId != 0)
      DockContextProcessUndockWindow(&g, window, true);
  }
  if (WindowSettings *settings = window ? FindWindowSettingsByWindow(window)
                                        : FindWindowSettingsByID(HashStr(name)))
    settings->WantDelete = true;
}

static void WindowSettingsHandler_ClearAll(Context *ctx, SettingsHandler *) {
  Context &g = *ctx;
  for (Window *window : g.Windows)
    window->SettingsOffset = -1;
  g.SettingsWindows.clear();
}

static void *WindowSettingsHandler_ReadOpen(Context *, SettingsHandler *,
                                            const char *name) {
  ID id = HashStr(name);
  WindowSettings *settings = Gui::FindWindowSettingsByID(id);
  if (settings)
    *settings = WindowSettings(); // Clear existing if recycling previous entry
  else
    settings = Gui::CreateNewWindowSettings(name);
  settings->ID = id;
  settings->WantApply = true;
  return (void *)settings;
}

static void WindowSettingsHandler_ReadLine(Context *, SettingsHandler *,
                                           void *entry, const char *line) {
  WindowSettings *settings = (WindowSettings *)entry;
  int x, y;
  int i;
  U32 u1;
  if (sscanf(line, "Pos=%i,%i", &x, &y) == 2) {
    settings->Pos = Vec2ih((short)x, (short)y);
  } else if (sscanf(line, "Size=%i,%i", &x, &y) == 2) {
    settings->Size = Vec2ih((short)x, (short)y);
  } else if (sscanf(line, "ViewportId=0x%08X", &u1) == 1) {
    settings->ViewportId = u1;
  } else if (sscanf(line, "ViewportPos=%i,%i", &x, &y) == 2) {
    settings->ViewportPos = Vec2ih((short)x, (short)y);
  } else if (sscanf(line, "Collapsed=%d", &i) == 1) {
    settings->Collapsed = (i != 0);
  } else if (sscanf(line, "IsChild=%d", &i) == 1) {
    settings->IsChild = (i != 0);
  } else if (sscanf(line, "DockId=0x%X,%d", &u1, &i) == 2) {
    settings->DockId = u1;
    settings->DockOrder = (short)i;
  } else if (sscanf(line, "DockId=0x%X", &u1) == 1) {
    settings->DockId = u1;
    settings->DockOrder = -1;
  } else if (sscanf(line, "ClassId=0x%X", &u1) == 1) {
    settings->ClassId = u1;
  }
}

// Apply to existing windows (if any)
static void WindowSettingsHandler_ApplyAll(Context *ctx, SettingsHandler *) {
  Context &g = *ctx;
  for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
       settings = g.SettingsWindows.next_chunk(settings))
    if (settings->WantApply) {
      if (Window *window = Gui::FindWindowByID(settings->ID))
        ApplyWindowSettings(window, settings);
      settings->WantApply = false;
    }
}

static void WindowSettingsHandler_WriteAll(Context *ctx,
                                           SettingsHandler *handler,
                                           TextBuffer *buf) {
  // Gather data from windows that were active during this session
  // (if a window wasn't opened in this session we preserve its settings)
  Context &g = *ctx;
  for (Window *window : g.Windows) {
    if (window->Flags & WindowFlags_NoSavedSettings)
      continue;

    WindowSettings *settings = Gui::FindWindowSettingsByWindow(window);
    if (!settings) {
      settings = Gui::CreateNewWindowSettings(window->Name);
      window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);
    }
    ASSERT(settings->ID == window->ID);
    settings->Pos = Vec2ih(window->Pos - window->ViewportPos);
    settings->Size = Vec2ih(window->SizeFull);
    settings->ViewportId = window->ViewportId;
    settings->ViewportPos = Vec2ih(window->ViewportPos);
    ASSERT(window->DockNode == NULL || window->DockNode->ID == window->DockId);
    settings->DockId = window->DockId;
    settings->ClassId = window->WindowClass.ClassId;
    settings->DockOrder = window->DockOrder;
    settings->Collapsed = window->Collapsed;
    settings->IsChild = (window->RootWindow !=
                         window); // Cannot rely on WindowFlags_ChildWindow
                                  // here as docked windows have this set.
    settings->WantDelete = false;
  }

  // Write to text buffer
  buf->reserve(buf->size() + g.SettingsWindows.size() * 6); // ballpark reserve
  for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
       settings = g.SettingsWindows.next_chunk(settings)) {
    if (settings->WantDelete)
      continue;
    const char *settings_name = settings->GetName();
    buf->appendf("[%s][%s]\n", handler->TypeName, settings_name);
    if (settings->IsChild) {
      buf->appendf("IsChild=1\n");
      buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
    } else {
      if (settings->ViewportId != 0 &&
          settings->ViewportId != Gui::VIEWPORT_DEFAULT_ID) {
        buf->appendf("ViewportPos=%d,%d\n", settings->ViewportPos.x,
                     settings->ViewportPos.y);
        buf->appendf("ViewportId=0x%08X\n", settings->ViewportId);
      }
      if (settings->Pos.x != 0 || settings->Pos.y != 0 ||
          settings->ViewportId == Gui::VIEWPORT_DEFAULT_ID)
        buf->appendf("Pos=%d,%d\n", settings->Pos.x, settings->Pos.y);
      if (settings->Size.x != 0 || settings->Size.y != 0)
        buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
      buf->appendf("Collapsed=%d\n", settings->Collapsed);
      if (settings->DockId != 0) {
        // buf->appendf("TabId=0x%08X\n", HashStr("#TAB", 4, settings->ID));
        // // window->TabId: this is not read back but writing it makes
        // "debugging" the .ini data easier.
        if (settings->DockOrder == -1)
          buf->appendf("DockId=0x%08X\n", settings->DockId);
        else
          buf->appendf("DockId=0x%08X,%d\n", settings->DockId,
                       settings->DockOrder);
        if (settings->ClassId != 0)
          buf->appendf("ClassId=0x%08X\n", settings->ClassId);
      }
    }
    buf->append("\n");
  }
}

//-----------------------------------------------------------------------------
// [SECTION] LOCALIZATION
//-----------------------------------------------------------------------------

void Gui::LocalizeRegisterEntries(const LocEntry *entries, int count) {
  Context &g = *GGui;
  for (int n = 0; n < count; n++)
    g.LocalizationTable[entries[n].Key] = entries[n].Text;
}

//-----------------------------------------------------------------------------
// [SECTION] VIEWPORTS, PLATFORM WINDOWS
//-----------------------------------------------------------------------------
// - GetMainViewport()
// - FindViewportByID()
// - FindViewportByPlatformHandle()
// - SetCurrentViewport() [Internal]
// - SetWindowViewport() [Internal]
// - GetWindowAlwaysWantOwnViewport() [Internal]
// - UpdateTryMergeWindowIntoHostViewport() [Internal]
// - UpdateTryMergeWindowIntoHostViewports() [Internal]
// - TranslateWindowsInViewport() [Internal]
// - ScaleWindowsInViewport() [Internal]
// - FindHoveredViewportFromPlatformWindowStack() [Internal]
// - UpdateViewportsNewFrame() [Internal]
// - UpdateViewportsEndFrame() [Internal]
// - AddUpdateViewport() [Internal]
// - WindowSelectViewport() [Internal]
// - WindowSyncOwnedViewport() [Internal]
// - UpdatePlatformWindows()
// - RenderPlatformWindowsDefault()
// - FindPlatformMonitorForPos() [Internal]
// - FindPlatformMonitorForRect() [Internal]
// - UpdateViewportPlatformMonitor() [Internal]
// - DestroyPlatformWindow() [Internal]
// - DestroyPlatformWindows()
//-----------------------------------------------------------------------------

Viewport *Gui::GetMainViewport() {
  Context &g = *GGui;
  return g.Viewports[0];
}

// FIXME: This leaks access to viewports not listed in PlatformIO.Viewports[].
// Problematic? (#4236)
Viewport *Gui::FindViewportByID(ID id) {
  Context &g = *GGui;
  for (ViewportP *viewport : g.Viewports)
    if (viewport->ID == id)
      return viewport;
  return NULL;
}

Viewport *Gui::FindViewportByPlatformHandle(void *platform_handle) {
  Context &g = *GGui;
  for (ViewportP *viewport : g.Viewports)
    if (viewport->PlatformHandle == platform_handle)
      return viewport;
  return NULL;
}

void Gui::SetCurrentViewport(Window *current_window, ViewportP *viewport) {
  Context &g = *GGui;
  (void)current_window;

  if (viewport)
    viewport->LastFrameActive = g.FrameCount;
  if (g.CurrentViewport == viewport)
    return;
  g.CurrentDpiScale = viewport ? viewport->DpiScale : 1.0f;
  g.CurrentViewport = viewport;
  // DEBUG_LOG_VIEWPORT("[viewport] SetCurrentViewport changed '%s' 0x%08X\n",
  // current_window ? current_window->Name : NULL, viewport ? viewport->ID : 0);

  // Notify platform layer of viewport changes
  // FIXME-DPI: This is only currently used for experimenting with handling of
  // multiple DPI
  if (g.CurrentViewport && g.PlatformIO.Platform_OnChangedViewport)
    g.PlatformIO.Platform_OnChangedViewport(g.CurrentViewport);
}

void Gui::SetWindowViewport(Window *window, ViewportP *viewport) {
  // Abandon viewport
  if (window->ViewportOwned && window->Viewport->Window == window)
    window->Viewport->Size = Vec2(0.0f, 0.0f);

  window->Viewport = viewport;
  window->ViewportId = viewport->ID;
  window->ViewportOwned = (viewport->Window == window);
}

static bool Gui::GetWindowAlwaysWantOwnViewport(Window *window) {
  // Tooltips and menus are not automatically forced into their own viewport
  // when the NoMerge flag is set, however the multiplication of viewports makes
  // them more likely to protrude and create their own.
  Context &g = *GGui;
  if (g.IO.ConfigViewportsNoAutoMerge ||
      (window->WindowClass.ViewportFlagsOverrideSet &
       ViewportFlags_NoAutoMerge))
    if (g.ConfigFlagsCurrFrame & ConfigFlags_ViewportsEnable)
      if (!window->DockIsActive)
        if ((window->Flags & (WindowFlags_ChildWindow | WindowFlags_ChildMenu |
                              WindowFlags_Tooltip)) == 0)
          if ((window->Flags & WindowFlags_Popup) == 0 ||
              (window->Flags & WindowFlags_Modal) != 0)
            return true;
  return false;
}

static bool Gui::UpdateTryMergeWindowIntoHostViewport(Window *window,
                                                      ViewportP *viewport) {
  Context &g = *GGui;
  if (window->Viewport == viewport)
    return false;
  if ((viewport->Flags & ViewportFlags_CanHostOtherWindows) == 0)
    return false;
  if ((viewport->Flags & ViewportFlags_IsMinimized) != 0)
    return false;
  if (!viewport->GetMainRect().Contains(window->Rect()))
    return false;
  if (GetWindowAlwaysWantOwnViewport(window))
    return false;

  // FIXME: Can't use g.WindowsFocusOrder[] for root windows only as we care
  // about Z order. If we maintained a DisplayOrder along with FocusOrder we
  // could..
  for (Window *window_behind : g.Windows) {
    if (window_behind == window)
      break;
    if (window_behind->WasActive && window_behind->ViewportOwned &&
        !(window_behind->Flags & WindowFlags_ChildWindow))
      if (window_behind->Viewport->GetMainRect().Overlaps(window->Rect()))
        return false;
  }

  // Move to the existing viewport, Move child/hosted windows as well
  // (FIXME-OPT: iterate child)
  ViewportP *old_viewport = window->Viewport;
  if (window->ViewportOwned)
    for (int n = 0; n < g.Windows.Size; n++)
      if (g.Windows[n]->Viewport == old_viewport)
        SetWindowViewport(g.Windows[n], viewport);
  SetWindowViewport(window, viewport);
  BringWindowToDisplayFront(window);

  return true;
}

// FIXME: handle 0 to N host viewports
static bool Gui::UpdateTryMergeWindowIntoHostViewports(Window *window) {
  Context &g = *GGui;
  return UpdateTryMergeWindowIntoHostViewport(window, g.Viewports[0]);
}

// Translate Gui windows when a Host Viewport has been moved
// (This additionally keeps windows at the same place when
// ConfigFlags_ViewportsEnable is toggled!)
void Gui::TranslateWindowsInViewport(ViewportP *viewport, const Vec2 &old_pos,
                                     const Vec2 &new_pos) {
  Context &g = *GGui;
  ASSERT(viewport->Window == NULL &&
         (viewport->Flags & ViewportFlags_CanHostOtherWindows));

  // 1) We test if ConfigFlags_ViewportsEnable was just toggled, which
  // allows us to conveniently translate imgui windows from OS-window-local to
  // absolute coordinates or vice-versa. 2) If it's not going to fit into the
  // new size, keep it at same absolute position. One problem with this is that
  // most Win32 applications doesn't update their render while dragging, and so
  // the window will appear to teleport when releasing the mouse.
  const bool translate_all_windows =
      (g.ConfigFlagsCurrFrame & ConfigFlags_ViewportsEnable) !=
      (g.ConfigFlagsLastFrame & ConfigFlags_ViewportsEnable);
  Rect test_still_fit_rect(old_pos, old_pos + viewport->Size);
  Vec2 delta_pos = new_pos - old_pos;
  for (Window *window : g.Windows) // FIXME-OPT
    if (translate_all_windows || (window->Viewport == viewport &&
                                  test_still_fit_rect.Contains(window->Rect())))
      TranslateWindow(window, delta_pos);
}

// Scale all windows (position, size). Use when e.g. changing DPI. (This is a
// lossy operation!)
void Gui::ScaleWindowsInViewport(ViewportP *viewport, float scale) {
  Context &g = *GGui;
  if (viewport->Window) {
    ScaleWindow(viewport->Window, scale);
  } else {
    for (Window *window : g.Windows)
      if (window->Viewport == viewport)
        ScaleWindow(window, scale);
  }
}

// If the backend doesn't set MouseLastHoveredViewport or doesn't honor
// ViewportFlags_NoInputs, we do a search ourselves. A) It won't take
// account of the possibility that non-imgui windows may be in-between our
// dragged window and our target window. B) It requires Platform_GetWindowFocus
// to be implemented by backend.
ViewportP *Gui::FindHoveredViewportFromPlatformWindowStack(
    const Vec2 &mouse_platform_pos) {
  Context &g = *GGui;
  ViewportP *best_candidate = NULL;
  for (ViewportP *viewport : g.Viewports)
    if (!(viewport->Flags &
          (ViewportFlags_NoInputs | ViewportFlags_IsMinimized)) &&
        viewport->GetMainRect().Contains(mouse_platform_pos))
      if (best_candidate == NULL || best_candidate->LastFocusedStampCount <
                                        viewport->LastFocusedStampCount)
        best_candidate = viewport;
  return best_candidate;
}

// Update viewports and monitor infos
// Note that this is running even if 'ConfigFlags_ViewportsEnable' is not
// set, in order to clear unused viewports (if any) and update monitor info.
static void Gui::UpdateViewportsNewFrame() {
  Context &g = *GGui;
  ASSERT(g.PlatformIO.Viewports.Size <= g.Viewports.Size);

  // Update Minimized status (we need it first in order to decide if we'll apply
  // Pos/Size of the main viewport) Update Focused status
  const bool viewports_enabled =
      (g.ConfigFlagsCurrFrame & ConfigFlags_ViewportsEnable) != 0;
  if (viewports_enabled) {
    ViewportP *focused_viewport = NULL;
    for (ViewportP *viewport : g.Viewports) {
      const bool platform_funcs_available = viewport->PlatformWindowCreated;
      if (g.PlatformIO.Platform_GetWindowMinimized &&
          platform_funcs_available) {
        bool is_minimized = g.PlatformIO.Platform_GetWindowMinimized(viewport);
        if (is_minimized)
          viewport->Flags |= ViewportFlags_IsMinimized;
        else
          viewport->Flags &= ~ViewportFlags_IsMinimized;
      }

      // Update our implicit z-order knowledge of platform windows, which is
      // used when the backend cannot provide io.MouseHoveredViewport. When
      // setting Platform_GetWindowFocus, it is expected that the platform
      // backend can handle calls without crashing if it doesn't have data
      // stored.
      if (g.PlatformIO.Platform_GetWindowFocus && platform_funcs_available) {
        bool is_focused = g.PlatformIO.Platform_GetWindowFocus(viewport);
        if (is_focused)
          viewport->Flags |= ViewportFlags_IsFocused;
        else
          viewport->Flags &= ~ViewportFlags_IsFocused;
        if (is_focused)
          focused_viewport = viewport;
      }
    }

    // Focused viewport has changed?
    if (focused_viewport &&
        g.PlatformLastFocusedViewportId != focused_viewport->ID) {
      DEBUG_LOG_VIEWPORT("[viewport] Focused viewport changed %08X -> %08X, "
                         "attempting to apply our focus.\n",
                         g.PlatformLastFocusedViewportId, focused_viewport->ID);
      const Viewport *prev_focused_viewport =
          FindViewportByID(g.PlatformLastFocusedViewportId);
      const bool prev_focused_has_been_destroyed =
          (prev_focused_viewport == NULL) ||
          (prev_focused_viewport->PlatformWindowCreated == false);

      // Store a tag so we can infer z-order easily from all our windows
      // We compare PlatformLastFocusedViewportId so newly created viewports
      // with _NoFocusOnAppearing flag will keep the front most stamp instead of
      // losing it back to their parent viewport.
      if (focused_viewport->LastFocusedStampCount !=
          g.ViewportFocusedStampCount)
        focused_viewport->LastFocusedStampCount = ++g.ViewportFocusedStampCount;
      g.PlatformLastFocusedViewportId = focused_viewport->ID;

      // Focus associated gui window
      // - if focus didn't happen with a click within imgui boundaries, e.g.
      // Clicking platform title bar. (#6299)
      // - if focus didn't happen because we destroyed another window (#6462)
      // FIXME: perhaps 'FocusTopMostWindowUnderOne()' can handle the
      // 'focused_window->Window != NULL' case as well.
      const bool apply_focus_on_focused_viewport =
          !IsAnyMouseDown() && !prev_focused_has_been_destroyed;
      if (apply_focus_on_focused_viewport) {
        focused_viewport->LastFocusedHadNavWindow |=
            (g.NavWindow != NULL) &&
            (g.NavWindow->Viewport ==
             focused_viewport); // Update so a window changing viewport won't
                                // lose focus.
        FocusRequestFlags focus_request_flags =
            FocusRequestFlags_UnlessBelowModal |
            FocusRequestFlags_RestoreFocusedChild;
        if (focused_viewport->Window != NULL)
          FocusWindow(focused_viewport->Window, focus_request_flags);
        else if (focused_viewport->LastFocusedHadNavWindow)
          FocusTopMostWindowUnderOne(
              NULL, NULL, focused_viewport,
              focus_request_flags); // Focus top most in viewport
        else
          FocusWindow(NULL, focus_request_flags); // No window had focus last
                                                  // time viewport was focused
      }
    }
    if (focused_viewport)
      focused_viewport->LastFocusedHadNavWindow =
          (g.NavWindow != NULL) && (g.NavWindow->Viewport == focused_viewport);
  }

  // Create/update main viewport with current platform position.
  // FIXME-VIEWPORT: Size is driven by backend/user code for
  // backward-compatibility but we should aim to make this more consistent.
  ViewportP *main_viewport = g.Viewports[0];
  ASSERT(main_viewport->ID == VIEWPORT_DEFAULT_ID);
  ASSERT(main_viewport->Window == NULL);
  Vec2 main_viewport_pos =
      viewports_enabled ? g.PlatformIO.Platform_GetWindowPos(main_viewport)
                        : Vec2(0.0f, 0.0f);
  Vec2 main_viewport_size = g.IO.DisplaySize;
  if (viewports_enabled && (main_viewport->Flags & ViewportFlags_IsMinimized)) {
    main_viewport_pos =
        main_viewport
            ->Pos; // Preserve last pos/size when minimized (FIXME: We don't do
                   // the same for Size outside of the viewport path)
    main_viewport_size = main_viewport->Size;
  }
  AddUpdateViewport(
      NULL, VIEWPORT_DEFAULT_ID, main_viewport_pos, main_viewport_size,
      ViewportFlags_OwnedByApp | ViewportFlags_CanHostOtherWindows);

  g.CurrentDpiScale = 0.0f;
  g.CurrentViewport = NULL;
  g.MouseViewport = NULL;
  for (int n = 0; n < g.Viewports.Size; n++) {
    ViewportP *viewport = g.Viewports[n];
    viewport->Idx = n;

    // Erase unused viewports
    if (n > 0 && viewport->LastFrameActive < g.FrameCount - 2) {
      DestroyViewport(viewport);
      n--;
      continue;
    }

    const bool platform_funcs_available = viewport->PlatformWindowCreated;
    if (viewports_enabled) {
      // Update Position and Size (from Platform Window to Gui) if requested.
      // We do it early in the frame instead of waiting for
      // UpdatePlatformWindows() to avoid a frame of lag when moving/resizing
      // using OS facilities.
      if (!(viewport->Flags & ViewportFlags_IsMinimized) &&
          platform_funcs_available) {
        // Viewport->WorkPos and WorkSize will be updated below
        if (viewport->PlatformRequestMove)
          viewport->Pos = viewport->LastPlatformPos =
              g.PlatformIO.Platform_GetWindowPos(viewport);
        if (viewport->PlatformRequestResize)
          viewport->Size = viewport->LastPlatformSize =
              g.PlatformIO.Platform_GetWindowSize(viewport);
      }
    }

    // Update/copy monitor info
    UpdateViewportPlatformMonitor(viewport);

    // Lock down space taken by menu bars and status bars, reset the offset for
    // functions like BeginMainMenuBar() to alter them again.
    viewport->WorkOffsetMin = viewport->BuildWorkOffsetMin;
    viewport->WorkOffsetMax = viewport->BuildWorkOffsetMax;
    viewport->BuildWorkOffsetMin = viewport->BuildWorkOffsetMax =
        Vec2(0.0f, 0.0f);
    viewport->UpdateWorkRect();

    // Reset alpha every frame. Users of transparency (docking) needs to request
    // a lower alpha back.
    viewport->Alpha = 1.0f;

    // Translate Gui windows when a Host Viewport has been moved
    // (This additionally keeps windows at the same place when
    // ConfigFlags_ViewportsEnable is toggled!)
    const Vec2 viewport_delta_pos = viewport->Pos - viewport->LastPos;
    if ((viewport->Flags & ViewportFlags_CanHostOtherWindows) &&
        (viewport_delta_pos.x != 0.0f || viewport_delta_pos.y != 0.0f))
      TranslateWindowsInViewport(viewport, viewport->LastPos, viewport->Pos);

    // Update DPI scale
    float new_dpi_scale;
    if (g.PlatformIO.Platform_GetWindowDpiScale && platform_funcs_available)
      new_dpi_scale = g.PlatformIO.Platform_GetWindowDpiScale(viewport);
    else if (viewport->PlatformMonitor != -1)
      new_dpi_scale = g.PlatformIO.Monitors[viewport->PlatformMonitor].DpiScale;
    else
      new_dpi_scale = (viewport->DpiScale != 0.0f) ? viewport->DpiScale : 1.0f;
    if (viewport->DpiScale != 0.0f && new_dpi_scale != viewport->DpiScale) {
      float scale_factor = new_dpi_scale / viewport->DpiScale;
      if (g.IO.ConfigFlags & ConfigFlags_DpiEnableScaleViewports)
        ScaleWindowsInViewport(viewport, scale_factor);
      // if (viewport == GetMainViewport())
      //     g.PlatformInterface.SetWindowSize(viewport, viewport->Size *
      //     scale_factor);

      // Scale our window moving pivot so that the window will rescale roughly
      // around the mouse position.
      // FIXME-VIEWPORT: This currently creates a resizing feedback loop when a
      // window is straddling a DPI transition border. (Minor: since our sizes
      // do not perfectly linearly scale, deferring the click offset scale until
      // we know the actual window scale ratio may get us slightly more precise
      // mouse positioning.)
      // if (g.MovingWindow != NULL && g.MovingWindow->Viewport == viewport)
      //    g.ActiveIdClickOffset = Trunc(g.ActiveIdClickOffset *
      //    scale_factor);
    }
    viewport->DpiScale = new_dpi_scale;
  }

  // Update fallback monitor
  if (g.PlatformIO.Monitors.Size == 0) {
    PlatformMonitor *monitor = &g.FallbackMonitor;
    monitor->MainPos = main_viewport->Pos;
    monitor->MainSize = main_viewport->Size;
    monitor->WorkPos = main_viewport->WorkPos;
    monitor->WorkSize = main_viewport->WorkSize;
    monitor->DpiScale = main_viewport->DpiScale;
  }

  if (!viewports_enabled) {
    g.MouseViewport = main_viewport;
    return;
  }

  // Mouse handling: decide on the actual mouse viewport for this frame between
  // the active/focused viewport and the hovered viewport. Note that
  // 'viewport_hovered' should skip over any viewport that has the
  // ViewportFlags_NoInputs flags set.
  ViewportP *viewport_hovered = NULL;
  if (g.IO.BackendFlags & BackendFlags_HasMouseHoveredViewport) {
    viewport_hovered =
        g.IO.MouseHoveredViewport
            ? (ViewportP *)FindViewportByID(g.IO.MouseHoveredViewport)
            : NULL;
    if (viewport_hovered && (viewport_hovered->Flags & ViewportFlags_NoInputs))
      viewport_hovered = FindHoveredViewportFromPlatformWindowStack(
          g.IO.MousePos); // Backend failed to handle _NoInputs viewport: revert
                          // to our fallback.
  } else {
    // If the backend doesn't know how to honor ViewportFlags_NoInputs, we
    // do a search ourselves. Note that this search: A) won't take account of
    // the possibility that non-imgui windows may be in-between our dragged
    // window and our target window. B) won't take account of how the backend
    // apply parent<>child relationship to secondary viewports, which affects
    // their Z order. C) uses LastFrameAsRefViewport as a flawed replacement for
    // the last time a window was focused (we could/should fix that by
    // introducing Focus functions in PlatformIO)
    viewport_hovered =
        FindHoveredViewportFromPlatformWindowStack(g.IO.MousePos);
  }
  if (viewport_hovered != NULL)
    g.MouseLastHoveredViewport = viewport_hovered;
  else if (g.MouseLastHoveredViewport == NULL)
    g.MouseLastHoveredViewport = g.Viewports[0];

  // Update mouse reference viewport
  // (when moving a window we aim at its viewport, but this will be overwritten
  // below if we go in drag and drop mode) (MovingViewport->Viewport will be
  // NULL in the rare situation where the window disappared while moving, set
  // UpdateMouseMovingWindowNewFrame() for details)
  if (g.MovingWindow && g.MovingWindow->Viewport)
    g.MouseViewport = g.MovingWindow->Viewport;
  else
    g.MouseViewport = g.MouseLastHoveredViewport;

  // When dragging something, always refer to the last hovered viewport.
  // - when releasing a moving window we will revert to aiming behind (at
  // viewport_hovered)
  // - when we are between viewports, our dragged preview will tend to show in
  // the last viewport _even_ if we don't have tooltips in their viewports (when
  // lacking monitor info)
  // - consider the case of holding on a menu item to browse child menus: even
  // thou a mouse button is held, there's no active id because menu items only
  // react on mouse release.
  // FIXME-VIEWPORT: This is essentially broken, when
  // BackendFlags_HasMouseHoveredViewport is set we want to trust when
  // viewport_hovered==NULL and use that.
  const bool is_mouse_dragging_with_an_expected_destination = g.DragDropActive;
  if (is_mouse_dragging_with_an_expected_destination &&
      viewport_hovered == NULL)
    viewport_hovered = g.MouseLastHoveredViewport;
  if (is_mouse_dragging_with_an_expected_destination || g.ActiveId == 0 ||
      !IsAnyMouseDown())
    if (viewport_hovered != NULL && viewport_hovered != g.MouseViewport &&
        !(viewport_hovered->Flags & ViewportFlags_NoInputs))
      g.MouseViewport = viewport_hovered;

  ASSERT(g.MouseViewport != NULL);
}

// Update user-facing viewport list (g.Viewports -> g.PlatformIO.Viewports after
// filtering out some)
static void Gui::UpdateViewportsEndFrame() {
  Context &g = *GGui;
  g.PlatformIO.Viewports.resize(0);
  for (int i = 0; i < g.Viewports.Size; i++) {
    ViewportP *viewport = g.Viewports[i];
    viewport->LastPos = viewport->Pos;
    if (viewport->LastFrameActive < g.FrameCount || viewport->Size.x <= 0.0f ||
        viewport->Size.y <= 0.0f)
      if (i > 0) // Always include main viewport in the list
        continue;
    if (viewport->Window && !IsWindowActiveAndVisible(viewport->Window))
      continue;
    if (i > 0)
      ASSERT(viewport->Window != NULL);
    g.PlatformIO.Viewports.push_back(viewport);
  }
  g.Viewports[0]->ClearRequestFlags(); // Clear main viewport flags because
                                       // UpdatePlatformWindows() won't do it
                                       // and may not even be called
}

// FIXME: We should ideally refactor the system to call this every frame (we
// currently don't)
ViewportP *Gui::AddUpdateViewport(Window *window, ID id, const Vec2 &pos,
                                  const Vec2 &size, ViewportFlags flags) {
  Context &g = *GGui;
  ASSERT(id != 0);

  flags |= ViewportFlags_IsPlatformWindow;
  if (window != NULL) {
    if (g.MovingWindow && g.MovingWindow->RootWindowDockTree == window)
      flags |= ViewportFlags_NoInputs | ViewportFlags_NoFocusOnAppearing;
    if ((window->Flags & WindowFlags_NoMouseInputs) &&
        (window->Flags & WindowFlags_NoNavInputs))
      flags |= ViewportFlags_NoInputs;
    if (window->Flags & WindowFlags_NoFocusOnAppearing)
      flags |= ViewportFlags_NoFocusOnAppearing;
  }

  ViewportP *viewport = (ViewportP *)FindViewportByID(id);
  if (viewport) {
    // Always update for main viewport as we are already pulling correct
    // platform pos/size (see #4900)
    if (!viewport->PlatformRequestMove || viewport->ID == VIEWPORT_DEFAULT_ID)
      viewport->Pos = pos;
    if (!viewport->PlatformRequestResize || viewport->ID == VIEWPORT_DEFAULT_ID)
      viewport->Size = size;
    viewport->Flags =
        flags | (viewport->Flags &
                 (ViewportFlags_IsMinimized |
                  ViewportFlags_IsFocused)); // Preserve existing flags
  } else {
    // New viewport
    viewport = NEW(ViewportP)();
    viewport->ID = id;
    viewport->Idx = g.Viewports.Size;
    viewport->Pos = viewport->LastPos = pos;
    viewport->Size = size;
    viewport->Flags = flags;
    UpdateViewportPlatformMonitor(viewport);
    g.Viewports.push_back(viewport);
    g.ViewportCreatedCount++;
    DEBUG_LOG_VIEWPORT("[viewport] Add Viewport %08X '%s'\n", id,
                       window ? window->Name : "<NULL>");

    // We normally setup for all viewports in NewFrame() but here need to handle
    // the mid-frame creation of a new viewport. We need to extend the
    // fullscreen clip rect so the OverlayDrawList clip is correct for that the
    // first frame
    g.DrawListSharedData.ClipRectFullscreen.x =
        Min(g.DrawListSharedData.ClipRectFullscreen.x, viewport->Pos.x);
    g.DrawListSharedData.ClipRectFullscreen.y =
        Min(g.DrawListSharedData.ClipRectFullscreen.y, viewport->Pos.y);
    g.DrawListSharedData.ClipRectFullscreen.z =
        Max(g.DrawListSharedData.ClipRectFullscreen.z,
            viewport->Pos.x + viewport->Size.x);
    g.DrawListSharedData.ClipRectFullscreen.w =
        Max(g.DrawListSharedData.ClipRectFullscreen.w,
            viewport->Pos.y + viewport->Size.y);

    // Store initial DpiScale before the OS platform window creation, based on
    // expected monitor data. This is so we can select an appropriate font size
    // on the first frame of our window lifetime
    if (viewport->PlatformMonitor != -1)
      viewport->DpiScale =
          g.PlatformIO.Monitors[viewport->PlatformMonitor].DpiScale;
  }

  viewport->Window = window;
  viewport->LastFrameActive = g.FrameCount;
  viewport->UpdateWorkRect();
  ASSERT(window == NULL || viewport->ID == window->ID);

  if (window != NULL)
    window->ViewportOwned = true;

  return viewport;
}

static void Gui::DestroyViewport(ViewportP *viewport) {
  // Clear references to this viewport in windows (window->ViewportId becomes
  // the master data)
  Context &g = *GGui;
  for (Window *window : g.Windows) {
    if (window->Viewport != viewport)
      continue;
    window->Viewport = NULL;
    window->ViewportOwned = false;
  }
  if (viewport == g.MouseLastHoveredViewport)
    g.MouseLastHoveredViewport = NULL;

  // Destroy
  DEBUG_LOG_VIEWPORT("[viewport] Delete Viewport %08X '%s'\n", viewport->ID,
                     viewport->Window ? viewport->Window->Name : "n/a");
  DestroyPlatformWindow(viewport); // In most circumstances the platform window
                                   // will already be destroyed here.
  ASSERT(g.PlatformIO.Viewports.contains(viewport) == false);
  ASSERT(g.Viewports[viewport->Idx] == viewport);
  g.Viewports.erase(g.Viewports.Data + viewport->Idx);
  DELETE(viewport);
}

// FIXME-VIEWPORT: This is all super messy and ought to be clarified or
// rewritten.
static void Gui::WindowSelectViewport(Window *window) {
  Context &g = *GGui;
  WindowFlags flags = window->Flags;
  window->ViewportAllowPlatformMonitorExtend = -1;

  // Restore main viewport if multi-viewport is not supported by the backend
  ViewportP *main_viewport = (ViewportP *)(void *)GetMainViewport();
  if (!(g.ConfigFlagsCurrFrame & ConfigFlags_ViewportsEnable)) {
    SetWindowViewport(window, main_viewport);
    return;
  }
  window->ViewportOwned = false;

  // Appearing popups reset their viewport so they can inherit again
  if ((flags & (WindowFlags_Popup | WindowFlags_Tooltip)) &&
      window->Appearing) {
    window->Viewport = NULL;
    window->ViewportId = 0;
  }

  if ((g.NextWindowData.Flags & NextWindowDataFlags_HasViewport) == 0) {
    // By default inherit from parent window
    if (window->Viewport == NULL && window->ParentWindow &&
        (!window->ParentWindow->IsFallbackWindow ||
         window->ParentWindow->WasActive))
      window->Viewport = window->ParentWindow->Viewport;

    // Attempt to restore saved viewport id (= window that hasn't been activated
    // yet), try to restore the viewport based on saved 'window->ViewportPos'
    // restored from .ini file
    if (window->Viewport == NULL && window->ViewportId != 0) {
      window->Viewport = (ViewportP *)FindViewportByID(window->ViewportId);
      if (window->Viewport == NULL && window->ViewportPos.x != FLT_MAX &&
          window->ViewportPos.y != FLT_MAX)
        window->Viewport =
            AddUpdateViewport(window, window->ID, window->ViewportPos,
                              window->Size, ViewportFlags_None);
    }
  }

  bool lock_viewport = false;
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasViewport) {
    // Code explicitly request a viewport
    window->Viewport =
        (ViewportP *)FindViewportByID(g.NextWindowData.ViewportId);
    window->ViewportId =
        g.NextWindowData
            .ViewportId; // Store ID even if Viewport isn't resolved yet.
    if (window->Viewport && (window->Flags & WindowFlags_DockNodeHost) != 0 &&
        window->Viewport->Window != NULL) {
      window->Viewport->Window = window;
      window->Viewport->ID = window->ViewportId =
          window->ID; // Overwrite ID (always owned by node)
    }
    lock_viewport = true;
  } else if ((flags & WindowFlags_ChildWindow) ||
             (flags & WindowFlags_ChildMenu)) {
    // Always inherit viewport from parent window
    if (window->DockNode && window->DockNode->HostWindow)
      ASSERT(window->DockNode->HostWindow->Viewport ==
             window->ParentWindow->Viewport);
    window->Viewport = window->ParentWindow->Viewport;
  } else if (window->DockNode && window->DockNode->HostWindow) {
    // This covers the "always inherit viewport from parent window" case for
    // when a window reattach to a node that was just created mid-frame
    window->Viewport = window->DockNode->HostWindow->Viewport;
  } else if (flags & WindowFlags_Tooltip) {
    window->Viewport = g.MouseViewport;
  } else if (GetWindowAlwaysWantOwnViewport(window)) {
    window->Viewport = AddUpdateViewport(window, window->ID, window->Pos,
                                         window->Size, ViewportFlags_None);
  } else if (g.MovingWindow && g.MovingWindow->RootWindowDockTree == window &&
             IsMousePosValid()) {
    if (window->Viewport != NULL && window->Viewport->Window == window)
      window->Viewport = AddUpdateViewport(window, window->ID, window->Pos,
                                           window->Size, ViewportFlags_None);
  } else {
    // Merge into host viewport?
    // We cannot test window->ViewportOwned as it set lower in the function.
    // Testing (g.ActiveId == 0 || g.ActiveIdAllowOverlap) to avoid merging
    // during a short-term widget interaction. Main intent was to avoid during
    // resize (see #4212)
    bool try_to_merge_into_host_viewport =
        (window->Viewport && window == window->Viewport->Window &&
         (g.ActiveId == 0 || g.ActiveIdAllowOverlap));
    if (try_to_merge_into_host_viewport)
      UpdateTryMergeWindowIntoHostViewports(window);
  }

  // Fallback: merge in default viewport if z-order matches, otherwise create a
  // new viewport
  if (window->Viewport == NULL)
    if (!UpdateTryMergeWindowIntoHostViewport(window, main_viewport))
      window->Viewport = AddUpdateViewport(window, window->ID, window->Pos,
                                           window->Size, ViewportFlags_None);

  // Mark window as allowed to protrude outside of its viewport and into the
  // current monitor
  if (!lock_viewport) {
    if (flags & (WindowFlags_Tooltip | WindowFlags_Popup)) {
      // We need to take account of the possibility that mouse may become
      // invalid. Popups/Tooltip always set ViewportAllowPlatformMonitorExtend
      // so GetWindowAllowedExtentRect() will return full monitor bounds.
      Vec2 mouse_ref = (flags & WindowFlags_Tooltip)
                           ? g.IO.MousePos
                           : g.BeginPopupStack.back().OpenMousePos;
      bool use_mouse_ref =
          (g.NavDisableHighlight || !g.NavDisableMouseHover || !g.NavWindow);
      bool mouse_valid = IsMousePosValid(&mouse_ref);
      if ((window->Appearing ||
           (flags & (WindowFlags_Tooltip | WindowFlags_ChildMenu))) &&
          (!use_mouse_ref || mouse_valid))
        window->ViewportAllowPlatformMonitorExtend = FindPlatformMonitorForPos(
            (use_mouse_ref && mouse_valid) ? mouse_ref
                                           : NavCalcPreferredRefPos());
      else
        window->ViewportAllowPlatformMonitorExtend =
            window->Viewport->PlatformMonitor;
    } else if (window->Viewport && window != window->Viewport->Window &&
               window->Viewport->Window && !(flags & WindowFlags_ChildWindow) &&
               window->DockNode == NULL) {
      // When called from Begin() we don't have access to a proper version of
      // the Hidden flag yet, so we replicate this code.
      const bool will_be_visible =
          (window->DockIsActive && !window->DockTabIsVisible) ? false : true;
      if ((window->Flags & WindowFlags_DockNodeHost) &&
          window->Viewport->LastFrameActive < g.FrameCount && will_be_visible) {
        // Steal/transfer ownership
        DEBUG_LOG_VIEWPORT(
            "[viewport] Window '%s' steal Viewport %08X from Window '%s'\n",
            window->Name, window->Viewport->ID, window->Viewport->Window->Name);
        window->Viewport->Window = window;
        window->Viewport->ID = window->ID;
        window->Viewport->LastNameHash = 0;
      } else if (!UpdateTryMergeWindowIntoHostViewports(window)) // Merge?
      {
        // New viewport
        window->Viewport =
            AddUpdateViewport(window, window->ID, window->Pos, window->Size,
                              ViewportFlags_NoFocusOnAppearing);
      }
    } else if (window->ViewportAllowPlatformMonitorExtend < 0 &&
               (flags & WindowFlags_ChildWindow) == 0) {
      // Regular (non-child, non-popup) windows by default are also allowed to
      // protrude Child windows are kept contained within their parent.
      window->ViewportAllowPlatformMonitorExtend =
          window->Viewport->PlatformMonitor;
    }
  }

  // Update flags
  window->ViewportOwned = (window == window->Viewport->Window);
  window->ViewportId = window->Viewport->ID;

  // If the OS window has a title bar, hide our imgui title bar
  // if (window->ViewportOwned && !(window->Viewport->Flags &
  // ViewportFlags_NoDecoration))
  //    window->Flags |= WindowFlags_NoTitleBar;
}

void Gui::WindowSyncOwnedViewport(Window *window,
                                  Window *parent_window_in_stack) {
  Context &g = *GGui;

  bool viewport_rect_changed = false;

  // Synchronize window --> viewport in most situations
  // Synchronize viewport -> window in case the platform window has been moved
  // or resized from the OS/WM
  if (window->Viewport->PlatformRequestMove) {
    window->Pos = window->Viewport->Pos;
    MarkIniSettingsDirty(window);
  } else if (memcmp(&window->Viewport->Pos, &window->Pos,
                    sizeof(window->Pos)) != 0) {
    viewport_rect_changed = true;
    window->Viewport->Pos = window->Pos;
  }

  if (window->Viewport->PlatformRequestResize) {
    window->Size = window->SizeFull = window->Viewport->Size;
    MarkIniSettingsDirty(window);
  } else if (memcmp(&window->Viewport->Size, &window->Size,
                    sizeof(window->Size)) != 0) {
    viewport_rect_changed = true;
    window->Viewport->Size = window->Size;
  }
  window->Viewport->UpdateWorkRect();

  // The viewport may have changed monitor since the global update in
  // UpdateViewportsNewFrame() Either a SetNextWindowPos() call in the current
  // frame or a SetWindowPos() call in the previous frame may have this effect.
  if (viewport_rect_changed)
    UpdateViewportPlatformMonitor(window->Viewport);

  // Update common viewport flags
  const ViewportFlags viewport_flags_to_clear =
      ViewportFlags_TopMost | ViewportFlags_NoTaskBarIcon |
      ViewportFlags_NoDecoration | ViewportFlags_NoRendererClear;
  ViewportFlags viewport_flags =
      window->Viewport->Flags & ~viewport_flags_to_clear;
  WindowFlags window_flags = window->Flags;
  const bool is_modal = (window_flags & WindowFlags_Modal) != 0;
  const bool is_short_lived_floating_window =
      (window_flags &
       (WindowFlags_ChildMenu | WindowFlags_Tooltip | WindowFlags_Popup)) != 0;
  if (window_flags & WindowFlags_Tooltip)
    viewport_flags |= ViewportFlags_TopMost;
  if ((g.IO.ConfigViewportsNoTaskBarIcon || is_short_lived_floating_window) &&
      !is_modal)
    viewport_flags |= ViewportFlags_NoTaskBarIcon;
  if (g.IO.ConfigViewportsNoDecoration || is_short_lived_floating_window)
    viewport_flags |= ViewportFlags_NoDecoration;

  // Not correct to set modal as topmost because:
  // - Because other popups can be stacked above a modal (e.g. combo box in a
  // modal)
  // - ViewportFlags_TopMost is currently handled different in backends: in
  // Win32 it is "appear top most" whereas in GLFW and SDL it is "stay topmost"
  // if (flags & WindowFlags_Modal)
  //    viewport_flags |= ViewportFlags_TopMost;

  // For popups and menus that may be protruding out of their parent viewport,
  // we enable _NoFocusOnClick so that clicking on them won't steal the OS focus
  // away from their parent window (which may be reflected in OS the title bar
  // decoration). Setting _NoFocusOnClick would technically prevent us from
  // bringing back to front in case they are being covered by an OS window from
  // a different app, but it shouldn't be much of a problem considering those
  // are already popups that are closed when clicking elsewhere.
  if (is_short_lived_floating_window && !is_modal)
    viewport_flags |=
        ViewportFlags_NoFocusOnAppearing | ViewportFlags_NoFocusOnClick;

  // We can overwrite viewport flags using WindowClass (advanced users)
  if (window->WindowClass.ViewportFlagsOverrideSet)
    viewport_flags |= window->WindowClass.ViewportFlagsOverrideSet;
  if (window->WindowClass.ViewportFlagsOverrideClear)
    viewport_flags &= ~window->WindowClass.ViewportFlagsOverrideClear;

  // We can also tell the backend that clearing the platform window won't be
  // necessary, as our window background is filling the viewport and we have
  // disabled BgAlpha.
  // FIXME: Work on support for per-viewport transparency (#2766)
  if (!(window_flags & WindowFlags_NoBackground))
    viewport_flags |= ViewportFlags_NoRendererClear;

  window->Viewport->Flags = viewport_flags;

  // Update parent viewport ID
  // (the !IsFallbackWindow test mimic the one done in WindowSelectViewport())
  if (window->WindowClass.ParentViewportId != (ID)-1)
    window->Viewport->ParentViewportId = window->WindowClass.ParentViewportId;
  else if ((window_flags & (WindowFlags_Popup | WindowFlags_Tooltip)) &&
           parent_window_in_stack &&
           (!parent_window_in_stack->IsFallbackWindow ||
            parent_window_in_stack->WasActive))
    window->Viewport->ParentViewportId = parent_window_in_stack->Viewport->ID;
  else
    window->Viewport->ParentViewportId =
        g.IO.ConfigViewportsNoDefaultParent ? 0 : VIEWPORT_DEFAULT_ID;
}

// Called by user at the end of the main loop, after EndFrame()
// This will handle the creation/update of all OS windows via function defined
// in the PlatformIO api.
void Gui::UpdatePlatformWindows() {
  Context &g = *GGui;
  ASSERT(
      g.FrameCountEnded == g.FrameCount &&
      "Forgot to call Render() or EndFrame() before UpdatePlatformWindows()?");
  ASSERT(g.FrameCountPlatformEnded < g.FrameCount);
  g.FrameCountPlatformEnded = g.FrameCount;
  if (!(g.ConfigFlagsCurrFrame & ConfigFlags_ViewportsEnable))
    return;

  // Create/resize/destroy platform windows to match each active viewport.
  // Skip the main viewport (index 0), which is always fully handled by the
  // application!
  for (int i = 1; i < g.Viewports.Size; i++) {
    ViewportP *viewport = g.Viewports[i];

    // Destroy platform window if the viewport hasn't been submitted or if it is
    // hosting a hidden window (the implicit/fallback Debug##Default window will
    // be registering its viewport then be disabled, causing a dummy
    // DestroyPlatformWindow to be made each frame)
    bool destroy_platform_window = false;
    destroy_platform_window |= (viewport->LastFrameActive < g.FrameCount - 1);
    destroy_platform_window |=
        (viewport->Window && !IsWindowActiveAndVisible(viewport->Window));
    if (destroy_platform_window) {
      DestroyPlatformWindow(viewport);
      continue;
    }

    // New windows that appears directly in a new viewport won't always have a
    // size on their first frame
    if (viewport->LastFrameActive < g.FrameCount || viewport->Size.x <= 0 ||
        viewport->Size.y <= 0)
      continue;

    // Create window
    const bool is_new_platform_window =
        (viewport->PlatformWindowCreated == false);
    if (is_new_platform_window) {
      DEBUG_LOG_VIEWPORT("[viewport] Create Platform Window %08X '%s'\n",
                         viewport->ID,
                         viewport->Window ? viewport->Window->Name : "n/a");
      g.PlatformIO.Platform_CreateWindow(viewport);
      if (g.PlatformIO.Renderer_CreateWindow != NULL)
        g.PlatformIO.Renderer_CreateWindow(viewport);
      g.PlatformWindowsCreatedCount++;
      viewport->LastNameHash = 0;
      viewport->LastPlatformPos = viewport->LastPlatformSize = Vec2(
          FLT_MAX, FLT_MAX); // By clearing those we'll enforce a call to
                             // Platform_SetWindowPos/Size below, before
                             // Platform_ShowWindow (FIXME: Is that necessary?)
      viewport->LastRendererSize =
          viewport
              ->Size; // We don't need to call Renderer_SetWindowSize() as it is
                      // expected Renderer_CreateWindow() already did it.
      viewport->PlatformWindowCreated = true;
    }

    // Apply Position and Size (from Gui to Platform/Renderer backends)
    if ((viewport->LastPlatformPos.x != viewport->Pos.x ||
         viewport->LastPlatformPos.y != viewport->Pos.y) &&
        !viewport->PlatformRequestMove)
      g.PlatformIO.Platform_SetWindowPos(viewport, viewport->Pos);
    if ((viewport->LastPlatformSize.x != viewport->Size.x ||
         viewport->LastPlatformSize.y != viewport->Size.y) &&
        !viewport->PlatformRequestResize)
      g.PlatformIO.Platform_SetWindowSize(viewport, viewport->Size);
    if ((viewport->LastRendererSize.x != viewport->Size.x ||
         viewport->LastRendererSize.y != viewport->Size.y) &&
        g.PlatformIO.Renderer_SetWindowSize)
      g.PlatformIO.Renderer_SetWindowSize(viewport, viewport->Size);
    viewport->LastPlatformPos = viewport->Pos;
    viewport->LastPlatformSize = viewport->LastRendererSize = viewport->Size;

    // Update title bar (if it changed)
    if (Window *window_for_title = GetWindowForTitleDisplay(viewport->Window)) {
      const char *title_begin = window_for_title->Name;
      char *title_end = (char *)(intptr_t)FindRenderedTextEnd(title_begin);
      const ID title_hash = HashStr(title_begin, title_end - title_begin);
      if (viewport->LastNameHash != title_hash) {
        char title_end_backup_c = *title_end;
        *title_end = 0; // Cut existing buffer short instead of doing an
                        // alloc/free, no small gain.
        g.PlatformIO.Platform_SetWindowTitle(viewport, title_begin);
        *title_end = title_end_backup_c;
        viewport->LastNameHash = title_hash;
      }
    }

    // Update alpha (if it changed)
    if (viewport->LastAlpha != viewport->Alpha &&
        g.PlatformIO.Platform_SetWindowAlpha)
      g.PlatformIO.Platform_SetWindowAlpha(viewport, viewport->Alpha);
    viewport->LastAlpha = viewport->Alpha;

    // Optional, general purpose call to allow the backend to perform general
    // book-keeping even if things haven't changed.
    if (g.PlatformIO.Platform_UpdateWindow)
      g.PlatformIO.Platform_UpdateWindow(viewport);

    if (is_new_platform_window) {
      // On startup ensure new platform window don't steal focus (give it a few
      // frames, as nested contents may lead to viewport being created a few
      // frames late)
      if (g.FrameCount < 3)
        viewport->Flags |= ViewportFlags_NoFocusOnAppearing;

      // Show window
      g.PlatformIO.Platform_ShowWindow(viewport);

      // Even without focus, we assume the window becomes front-most.
      // This is useful for our platform z-order heuristic when
      // io.MouseHoveredViewport is not available.
      if (viewport->LastFocusedStampCount != g.ViewportFocusedStampCount)
        viewport->LastFocusedStampCount = ++g.ViewportFocusedStampCount;
    }

    // Clear request flags
    viewport->ClearRequestFlags();
  }
}

// This is a default/basic function for performing the rendering/swap of
// multiple Platform Windows. Custom renderers may prefer to not call this
// function at all, and instead iterate the publicly exposed platform data and
// handle rendering/sync themselves. The Render/Swap functions stored in
// PlatformIO are merely here to allow for this helper to exist, but you
// can do it yourself:
//
//    PlatformIO& platform_io = Gui::GetPlatformIO();
//    for (int i = 1; i < platform_io.Viewports.Size; i++)
//        if ((platform_io.Viewports[i]->Flags & ViewportFlags_Minimized)
//        == 0)
//            MyRenderFunction(platform_io.Viewports[i], my_args);
//    for (int i = 1; i < platform_io.Viewports.Size; i++)
//        if ((platform_io.Viewports[i]->Flags & ViewportFlags_Minimized)
//        == 0)
//            MySwapBufferFunction(platform_io.Viewports[i], my_args);
//
void Gui::RenderPlatformWindowsDefault(void *platform_render_arg,
                                       void *renderer_render_arg) {
  // Skip the main viewport (index 0), which is always fully handled by the
  // application!
  PlatformIO &platform_io = Gui::GetPlatformIO();
  for (int i = 1; i < platform_io.Viewports.Size; i++) {
    Viewport *viewport = platform_io.Viewports[i];
    if (viewport->Flags & ViewportFlags_IsMinimized)
      continue;
    if (platform_io.Platform_RenderWindow)
      platform_io.Platform_RenderWindow(viewport, platform_render_arg);
    if (platform_io.Renderer_RenderWindow)
      platform_io.Renderer_RenderWindow(viewport, renderer_render_arg);
  }
  for (int i = 1; i < platform_io.Viewports.Size; i++) {
    Viewport *viewport = platform_io.Viewports[i];
    if (viewport->Flags & ViewportFlags_IsMinimized)
      continue;
    if (platform_io.Platform_SwapBuffers)
      platform_io.Platform_SwapBuffers(viewport, platform_render_arg);
    if (platform_io.Renderer_SwapBuffers)
      platform_io.Renderer_SwapBuffers(viewport, renderer_render_arg);
  }
}

static int Gui::FindPlatformMonitorForPos(const Vec2 &pos) {
  Context &g = *GGui;
  for (int monitor_n = 0; monitor_n < g.PlatformIO.Monitors.Size; monitor_n++) {
    const PlatformMonitor &monitor = g.PlatformIO.Monitors[monitor_n];
    if (Rect(monitor.MainPos, monitor.MainPos + monitor.MainSize).Contains(pos))
      return monitor_n;
  }
  return -1;
}

// Search for the monitor with the largest intersection area with the given
// rectangle We generally try to avoid searching loops but the monitor count
// should be very small here
// FIXME-OPT: We could test the last monitor used for that viewport first, and
// early
static int Gui::FindPlatformMonitorForRect(const Rect &rect) {
  Context &g = *GGui;

  const int monitor_count = g.PlatformIO.Monitors.Size;
  if (monitor_count <= 1)
    return monitor_count - 1;

  // Use a minimum threshold of 1.0f so a zero-sized rect won't false positive,
  // and will still find the correct monitor given its position. This is
  // necessary for tooltips which always resize down to zero at first.
  const float surface_threshold =
      Max(rect.GetWidth() * rect.GetHeight() * 0.5f, 1.0f);
  int best_monitor_n = -1;
  float best_monitor_surface = 0.001f;

  for (int monitor_n = 0; monitor_n < g.PlatformIO.Monitors.Size &&
                          best_monitor_surface < surface_threshold;
       monitor_n++) {
    const PlatformMonitor &monitor = g.PlatformIO.Monitors[monitor_n];
    const Rect monitor_rect =
        Rect(monitor.MainPos, monitor.MainPos + monitor.MainSize);
    if (monitor_rect.Contains(rect))
      return monitor_n;
    Rect overlapping_rect = rect;
    overlapping_rect.ClipWithFull(monitor_rect);
    float overlapping_surface =
        overlapping_rect.GetWidth() * overlapping_rect.GetHeight();
    if (overlapping_surface < best_monitor_surface)
      continue;
    best_monitor_surface = overlapping_surface;
    best_monitor_n = monitor_n;
  }
  return best_monitor_n;
}

// Update monitor from viewport rectangle (we'll use this info to clamp windows
// and save windows lost in a removed monitor)
static void Gui::UpdateViewportPlatformMonitor(ViewportP *viewport) {
  viewport->PlatformMonitor =
      (short)FindPlatformMonitorForRect(viewport->GetMainRect());
}

// Return value is always != NULL, but don't hold on it across frames.
const PlatformMonitor *Gui::GetViewportPlatformMonitor(Viewport *viewport_p) {
  Context &g = *GGui;
  ViewportP *viewport = (ViewportP *)(void *)viewport_p;
  int monitor_idx = viewport->PlatformMonitor;
  if (monitor_idx >= 0 && monitor_idx < g.PlatformIO.Monitors.Size)
    return &g.PlatformIO.Monitors[monitor_idx];
  return &g.FallbackMonitor;
}

void Gui::DestroyPlatformWindow(ViewportP *viewport) {
  Context &g = *GGui;
  if (viewport->PlatformWindowCreated) {
    DEBUG_LOG_VIEWPORT("[viewport] Destroy Platform Window %08X '%s'\n",
                       viewport->ID,
                       viewport->Window ? viewport->Window->Name : "n/a");
    if (g.PlatformIO.Renderer_DestroyWindow)
      g.PlatformIO.Renderer_DestroyWindow(viewport);
    if (g.PlatformIO.Platform_DestroyWindow)
      g.PlatformIO.Platform_DestroyWindow(viewport);
    ASSERT(viewport->RendererUserData == NULL &&
           viewport->PlatformUserData == NULL);

    // Don't clear PlatformWindowCreated for the main viewport, as we initially
    // set that up to true in Initialize() The righter way may be to leave it to
    // the backend to set this flag all-together, and made the flag public.
    if (viewport->ID != VIEWPORT_DEFAULT_ID)
      viewport->PlatformWindowCreated = false;
  } else {
    ASSERT(viewport->RendererUserData == NULL &&
           viewport->PlatformUserData == NULL &&
           viewport->PlatformHandle == NULL);
  }
  viewport->RendererUserData = viewport->PlatformUserData =
      viewport->PlatformHandle = NULL;
  viewport->ClearRequestFlags();
}

void Gui::DestroyPlatformWindows() {
  // We call the destroy window on every viewport (including the main viewport,
  // index 0) to give a chance to the backend to clear any data they may have
  // stored in e.g. PlatformUserData, RendererUserData. It is convenient for the
  // platform backend code to store something in the main viewport, in order for
  // e.g. the mouse handling code to operator a consistent manner. It is
  // expected that the backend can handle calls to
  // Renderer_DestroyWindow/Platform_DestroyWindow without crashing if it
  // doesn't have data stored.
  Context &g = *GGui;
  for (ViewportP *viewport : g.Viewports)
    DestroyPlatformWindow(viewport);
}

//-----------------------------------------------------------------------------
// [SECTION] DOCKING
//-----------------------------------------------------------------------------
// Docking: Internal Types
// Docking: Forward Declarations
// Docking: DockContext
// Docking: DockContext Docking/Undocking functions
// Docking: DockNode
// Docking: DockNode Tree manipulation functions
// Docking: Public Functions (SetWindowDock, DockSpace, DockSpaceOverViewport)
// Docking: Builder Functions
// Docking: Begin/End Support Functions (called from Begin/End)
// Docking: Settings
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Typical Docking call flow: (root level is generally public API):
//-----------------------------------------------------------------------------
// - NewFrame()                               new gui frame
//    | DockContextNewFrameUpdateUndocking()  - process queued undocking
//    requests | - DockContextProcessUndockWindow()    - process one window
//    undocking request | - DockContextProcessUndockNode()      - process one
//    whole node undocking request | DockContextNewFrameUpdateUndocking()  -
//    process queue docking requests, create floating dock nodes | - update
//    g.HoveredDockNode            - [debug] update node hovered by mouse | -
//    DockContextProcessDock()            - process one docking request | -
//    DockNodeUpdate() |   - DockNodeUpdateForRootNode() |     -
//    DockNodeUpdateFlagsAndCollapse() |     - DockNodeFindInfo() |   - destroy
//    unused node or tab bar |   - create dock node host window |      - Begin()
//    etc. |   - DockNodeStartMouseMovingWindow() |   -
//    DockNodeTreeUpdatePosSize() |   - DockNodeTreeUpdateSplitter() |   - draw
//    node background |   - DockNodeUpdateTabBar()            - create/update
//    tab bar for a docking node |     - DockNodeAddTabBar() |     -
//    DockNodeWindowMenuUpdate() |     - DockNodeCalcTabBarLayout() |     -
//    BeginTabBarEx() |     - TabItemEx() calls |     - EndTabBar() |   -
//    BeginDockableDragDropTarget() |      - DockNodeUpdate()               -
//    recurse into child nodes...
//-----------------------------------------------------------------------------
// - DockSpace()                              user submit a dockspace into a
// window
//    | Begin(Child)                          - create a child window
//    | DockNodeUpdate()                      - call main dock node update
//    function | End(Child) | ItemSize()
//-----------------------------------------------------------------------------
// - Begin()
//    | BeginDocked()
//    | BeginDockableDragDropSource()
//    | BeginDockableDragDropTarget()
//    | - DockNodePreviewDockRender()
//-----------------------------------------------------------------------------
// - EndFrame()
//    | DockContextEndFrame()
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Docking: Internal Types
//-----------------------------------------------------------------------------
// - DockRequestType
// - DockRequest
// - DockPreviewData
// - DockNodeSettings
// - DockContext
//-----------------------------------------------------------------------------

enum DockRequestType {
  DockRequestType_None = 0,
  DockRequestType_Dock,
  DockRequestType_Undock,
  DockRequestType_Split // Split is the same as Dock but without a
                        // DockPayload
};

struct DockRequest {
  DockRequestType Type;
  Window *DockTargetWindow; // Destination/Target Window to dock into (may be a
                            // loose window or a DockNode, might be NULL in
                            // which case DockTargetNode cannot be NULL)
  DockNode *DockTargetNode; // Destination/Target Node to dock into
  Window *DockPayload;      // Source/Payload window to dock (may be a loose
                            // window or a DockNode), [Optional]
  Dir DockSplitDir;
  float DockSplitRatio;
  bool DockSplitOuter;
  Window *UndockTargetWindow;
  DockNode *UndockTargetNode;

  DockRequest() {
    Type = DockRequestType_None;
    DockTargetWindow = DockPayload = UndockTargetWindow = NULL;
    DockTargetNode = UndockTargetNode = NULL;
    DockSplitDir = Dir_None;
    DockSplitRatio = 0.5f;
    DockSplitOuter = false;
  }
};

struct DockPreviewData {
  DockNode FutureNode;
  bool IsDropAllowed;
  bool IsCenterAvailable;
  bool IsSidesAvailable;   // Hold your breath, grammar freaks..
  bool IsSplitDirExplicit; // Set when hovered the drop rect (vs. implicit
                           // SplitDir==None when hovered the window)
  DockNode *SplitNode;
  Dir SplitDir;
  float SplitRatio;
  Rect DropRectsDraw[Dir_COUNT +
                     1]; // May be slightly different from hit-testing drop
                         // rects used in DockNodeCalcDropRects()

  DockPreviewData() : FutureNode(0) {
    IsDropAllowed = IsCenterAvailable = IsSidesAvailable = IsSplitDirExplicit =
        false;
    SplitNode = NULL;
    SplitDir = Dir_None;
    SplitRatio = 0.f;
    for (int n = 0; n < ARRAYSIZE(DropRectsDraw); n++)
      DropRectsDraw[n] = Rect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
  }
};

// Persistent Settings data, stored contiguously in SettingsNodes (sizeof() ~32
// bytes)
struct DockNodeSettings {
  ID ID;
  ID ParentNodeId;
  ID ParentWindowId;
  ID SelectedTabId;
  signed char SplitAxis;
  char Depth;
  DockNodeFlags Flags; // NB: We save individual flags one by one in ascii
                       // format (DockNodeFlags_SavedFlagsMask_)
  Vec2ih Pos;
  Vec2ih Size;
  Vec2ih SizeRef;
  DockNodeSettings() {
    memset(this, 0, sizeof(*this));
    SplitAxis = Axis_None;
  }
};

//-----------------------------------------------------------------------------
// Docking: Forward Declarations
//-----------------------------------------------------------------------------

namespace Gui {
// DockContext
static DockNode *DockContextAddNode(Context *ctx, ID id);
static void DockContextRemoveNode(Context *ctx, DockNode *node,
                                  bool merge_sibling_into_parent_node);
static void DockContextQueueNotifyRemovedNode(Context *ctx, DockNode *node);
static void DockContextProcessDock(Context *ctx, DockRequest *req);
static void DockContextPruneUnusedSettingsNodes(Context *ctx);
static DockNode *DockContextBindNodeToWindow(Context *ctx, Window *window);
static void
DockContextBuildNodesFromSettings(Context *ctx,
                                  DockNodeSettings *node_settings_array,
                                  int node_settings_count);
static void
DockContextBuildAddWindowsToNodes(Context *ctx,
                                  ID root_id); // Use root_id==0 to add all

// DockNode
static void DockNodeAddWindow(DockNode *node, Window *window,
                              bool add_to_tab_bar);
static void DockNodeMoveWindows(DockNode *dst_node, DockNode *src_node);
static void DockNodeMoveChildNodes(DockNode *dst_node, DockNode *src_node);
static Window *DockNodeFindWindowByID(DockNode *node, ID id);
static void DockNodeApplyPosSizeToWindows(DockNode *node);
static void DockNodeRemoveWindow(DockNode *node, Window *window,
                                 ID save_dock_id);
static void DockNodeHideHostWindow(DockNode *node);
static void DockNodeUpdate(DockNode *node);
static void DockNodeUpdateForRootNode(DockNode *node);
static void DockNodeUpdateFlagsAndCollapse(DockNode *node);
static void DockNodeUpdateHasCentralNodeChild(DockNode *node);
static void DockNodeUpdateTabBar(DockNode *node, Window *host_window);
static void DockNodeAddTabBar(DockNode *node);
static void DockNodeRemoveTabBar(DockNode *node);
static void DockNodeWindowMenuUpdate(DockNode *node, TabBar *tab_bar);
static void DockNodeUpdateVisibleFlag(DockNode *node);
static void DockNodeStartMouseMovingWindow(DockNode *node, Window *window);
static bool DockNodeIsDropAllowed(Window *host_window, Window *payload_window);
static void DockNodePreviewDockSetup(Window *host_window, DockNode *host_node,
                                     Window *payload_window,
                                     DockNode *payload_node,
                                     DockPreviewData *preview_data,
                                     bool is_explicit_target,
                                     bool is_outer_docking);
static void DockNodePreviewDockRender(Window *host_window, DockNode *host_node,
                                      Window *payload_window,
                                      const DockPreviewData *preview_data);
static void DockNodeCalcTabBarLayout(const DockNode *node, Rect *out_title_rect,
                                     Rect *out_tab_bar_rect,
                                     Vec2 *out_window_menu_button_pos,
                                     Vec2 *out_close_button_pos);
static void DockNodeCalcSplitRects(Vec2 &pos_old, Vec2 &size_old, Vec2 &pos_new,
                                   Vec2 &size_new, Dir dir,
                                   Vec2 size_new_desired);
static bool DockNodeCalcDropRectsAndTestMousePos(const Rect &parent, Dir dir,
                                                 Rect &out_draw,
                                                 bool outer_docking,
                                                 Vec2 *test_mouse_pos);
static const char *DockNodeGetHostWindowTitle(DockNode *node, char *buf,
                                              int buf_size) {
  FormatString(buf, buf_size, "##DockNode_%02X", node->ID);
  return buf;
}
static int DockNodeGetTabOrder(Window *window);

// DockNode tree manipulations
static void DockNodeTreeSplit(Context *ctx, DockNode *parent_node,
                              Axis split_axis, int split_first_child,
                              float split_ratio, DockNode *new_node);
static void DockNodeTreeMerge(Context *ctx, DockNode *parent_node,
                              DockNode *merge_lead_child);
static void
DockNodeTreeUpdatePosSize(DockNode *node, Vec2 pos, Vec2 size,
                          DockNode *only_write_to_single_node = NULL);
static void DockNodeTreeUpdateSplitter(DockNode *node);
static DockNode *DockNodeTreeFindVisibleNodeByPos(DockNode *node, Vec2 pos);
static DockNode *DockNodeTreeFindFallbackLeafNode(DockNode *node);

// Settings
static void DockSettingsRenameNodeReferences(ID old_node_id, ID new_node_id);
static void DockSettingsRemoveNodeReferences(ID *node_ids, int node_ids_count);
static DockNodeSettings *DockSettingsFindNodeSettings(Context *ctx, ID node_id);
static void DockSettingsHandler_ClearAll(Context *, SettingsHandler *);
static void DockSettingsHandler_ApplyAll(Context *, SettingsHandler *);
static void *DockSettingsHandler_ReadOpen(Context *, SettingsHandler *,
                                          const char *name);
static void DockSettingsHandler_ReadLine(Context *, SettingsHandler *,
                                         void *entry, const char *line);
static void DockSettingsHandler_WriteAll(Context *ctx, SettingsHandler *handler,
                                         TextBuffer *buf);
} // namespace Gui

//-----------------------------------------------------------------------------
// Docking: DockContext
//-----------------------------------------------------------------------------
// The lifetime model is different from the one of regular windows: we always
// create a DockNode for each DockNodeSettings, or we always hold the
// entire docking node tree. Nodes are frequently hidden, e.g. if the window(s)
// or child nodes they host are not active. At boot time only, we run a simple
// GC to remove nodes that have no references. Because dock node settings (which
// are small, contiguous structures) are always mirrored by their corresponding
// dock nodes (more complete structures), we can also very easily recreate the
// nodes from scratch given the settings data (this is what DockContextRebuild()
// does). This is convenient as docking reconfiguration can be implemented by
// mostly poking at the simpler settings data.
//-----------------------------------------------------------------------------
// - DockContextInitialize()
// - DockContextShutdown()
// - DockContextClearNodes()
// - DockContextRebuildNodes()
// - DockContextNewFrameUpdateUndocking()
// - DockContextNewFrameUpdateDocking()
// - DockContextEndFrame()
// - DockContextFindNodeByID()
// - DockContextBindNodeToWindow()
// - DockContextGenNodeID()
// - DockContextAddNode()
// - DockContextRemoveNode()
// - DockContextPruneNodeData
// - DockContextPruneUnusedSettingsNodes()
// - DockContextBuildNodesFromSettings()
// - DockContextBuildAddWindowsToNodes()
//-----------------------------------------------------------------------------

void Gui::DockContextInitialize(Context *ctx) {
  Context &g = *ctx;

  // Add .ini handle for persistent docking data
  SettingsHandler ini_handler;
  ini_handler.TypeName = "Docking";
  ini_handler.TypeHash = HashStr("Docking");
  ini_handler.ClearAllFn = DockSettingsHandler_ClearAll;
  ini_handler.ReadInitFn = DockSettingsHandler_ClearAll; // Also clear on read
  ini_handler.ReadOpenFn = DockSettingsHandler_ReadOpen;
  ini_handler.ReadLineFn = DockSettingsHandler_ReadLine;
  ini_handler.ApplyAllFn = DockSettingsHandler_ApplyAll;
  ini_handler.WriteAllFn = DockSettingsHandler_WriteAll;
  g.SettingsHandlers.push_back(ini_handler);

  g.DockNodeWindowMenuHandler = &DockNodeWindowMenuHandler_Default;
}

void Gui::DockContextShutdown(Context *ctx) {
  DockContext *dc = &ctx->DockContext;
  for (int n = 0; n < dc->Nodes.Data.Size; n++)
    if (DockNode *node = (DockNode *)dc->Nodes.Data[n].val_p)
      DELETE(node);
}

void Gui::DockContextClearNodes(Context *ctx, ID root_id,
                                bool clear_settings_refs) {
  UNUSED(ctx);
  ASSERT(ctx == GGui);
  DockBuilderRemoveNodeDockedWindows(root_id, clear_settings_refs);
  DockBuilderRemoveNodeChildNodes(root_id);
}

// [DEBUG] This function also acts as a defacto test to make sure we can rebuild
// from scratch without a glitch (Different from DockSettingsHandler_ClearAll()
// + DockSettingsHandler_ApplyAll() because this reuses current settings!)
void Gui::DockContextRebuildNodes(Context *ctx) {
  Context &g = *ctx;
  DockContext *dc = &ctx->DockContext;
  DEBUG_LOG_DOCKING("[docking] DockContextRebuildNodes\n");
  SaveIniSettingsToMemory();
  ID root_id = 0; // Rebuild all
  DockContextClearNodes(ctx, root_id, false);
  DockContextBuildNodesFromSettings(ctx, dc->NodesSettings.Data,
                                    dc->NodesSettings.Size);
  DockContextBuildAddWindowsToNodes(ctx, root_id);
}

// Docking context update function, called by NewFrame()
void Gui::DockContextNewFrameUpdateUndocking(Context *ctx) {
  Context &g = *ctx;
  DockContext *dc = &ctx->DockContext;
  if (!(g.IO.ConfigFlags & ConfigFlags_DockingEnable)) {
    if (dc->Nodes.Data.Size > 0 || dc->Requests.Size > 0)
      DockContextClearNodes(ctx, 0, true);
    return;
  }

  // Setting NoSplit at runtime merges all nodes
  if (g.IO.ConfigDockingNoSplit)
    for (int n = 0; n < dc->Nodes.Data.Size; n++)
      if (DockNode *node = (DockNode *)dc->Nodes.Data[n].val_p)
        if (node->IsRootNode() && node->IsSplitNode()) {
          DockBuilderRemoveNodeChildNodes(node->ID);
          // dc->WantFullRebuild = true;
        }

        // Process full rebuild
#if 0
    if (Gui::IsKeyPressed(Gui::GetKeyIndex(Key_C)))
        dc->WantFullRebuild = true;
#endif
  if (dc->WantFullRebuild) {
    DockContextRebuildNodes(ctx);
    dc->WantFullRebuild = false;
  }

  // Process Undocking requests (we need to process them _before_ the
  // UpdateMouseMovingWindowNewFrame call in NewFrame)
  for (DockRequest &req : dc->Requests) {
    if (req.Type == DockRequestType_Undock && req.UndockTargetWindow)
      DockContextProcessUndockWindow(ctx, req.UndockTargetWindow);
    else if (req.Type == DockRequestType_Undock && req.UndockTargetNode)
      DockContextProcessUndockNode(ctx, req.UndockTargetNode);
  }
}

// Docking context update function, called by NewFrame()
void Gui::DockContextNewFrameUpdateDocking(Context *ctx) {
  Context &g = *ctx;
  DockContext *dc = &ctx->DockContext;
  if (!(g.IO.ConfigFlags & ConfigFlags_DockingEnable))
    return;

  // [DEBUG] Store hovered dock node.
  // We could in theory use DockNodeTreeFindVisibleNodeByPos() on the root host
  // dock node, but using ->DockNode is a good shortcut. Note this is mostly a
  // debug thing and isn't actually used for docking target, because docking
  // involve more detailed filtering.
  g.DebugHoveredDockNode = NULL;
  if (Window *hovered_window = g.HoveredWindowUnderMovingWindow) {
    if (hovered_window->DockNodeAsHost)
      g.DebugHoveredDockNode = DockNodeTreeFindVisibleNodeByPos(
          hovered_window->DockNodeAsHost, g.IO.MousePos);
    else if (hovered_window->RootWindow->DockNode)
      g.DebugHoveredDockNode = hovered_window->RootWindow->DockNode;
  }

  // Process Docking requests
  for (DockRequest &req : dc->Requests)
    if (req.Type == DockRequestType_Dock)
      DockContextProcessDock(ctx, &req);
  dc->Requests.resize(0);

  // Create windows for each automatic docking nodes
  // We can have NULL pointers when we delete nodes, but because ID are recycled
  // this should amortize nicely (and our node count will never be very high)
  for (int n = 0; n < dc->Nodes.Data.Size; n++)
    if (DockNode *node = (DockNode *)dc->Nodes.Data[n].val_p)
      if (node->IsFloatingNode())
        DockNodeUpdate(node);
}

void Gui::DockContextEndFrame(Context *ctx) {
  // Draw backgrounds of node missing their window
  Context &g = *ctx;
  DockContext *dc = &g.DockContext;
  for (int n = 0; n < dc->Nodes.Data.Size; n++)
    if (DockNode *node = (DockNode *)dc->Nodes.Data[n].val_p)
      if (node->LastFrameActive == g.FrameCount && node->IsVisible &&
          node->HostWindow && node->IsLeafNode() && !node->IsBgDrawnThisFrame) {
        Rect bg_rect(node->Pos + Vec2(0.0f, GetFrameHeight()),
                     node->Pos + node->Size);
        DrawFlags bg_rounding_flags = CalcRoundingFlagsForRectInRect(
            bg_rect, node->HostWindow->Rect(), g.Style.DockingSeparatorSize);
        node->HostWindow->DrawList->ChannelsSetCurrent(
            DOCKING_HOST_DRAW_CHANNEL_BG);
        node->HostWindow->DrawList->AddRectFilled(
            bg_rect.Min, bg_rect.Max, node->LastBgColor,
            node->HostWindow->WindowRounding, bg_rounding_flags);
      }
}

DockNode *Gui::DockContextFindNodeByID(Context *ctx, ID id) {
  return (DockNode *)ctx->DockContext.Nodes.GetVoidPtr(id);
}

ID Gui::DockContextGenNodeID(Context *ctx) {
  // Generate an ID for new node (the exact ID value doesn't matter as long as
  // it is not already used)
  // FIXME-OPT FIXME-DOCK: This is suboptimal, even if the node count is small
  // enough not to be a worry.0 We should poke in ctx->Nodes to find a suitable
  // ID faster. Even more so trivial that ctx->Nodes lookup is already sorted.
  ID id = 0x0001;
  while (DockContextFindNodeByID(ctx, id) != NULL)
    id++;
  return id;
}

static DockNode *Gui::DockContextAddNode(Context *ctx, ID id) {
  // Generate an ID for the new node (the exact ID value doesn't matter as long
  // as it is not already used) and add the first window.
  Context &g = *ctx;
  if (id == 0)
    id = DockContextGenNodeID(ctx);
  else
    ASSERT(DockContextFindNodeByID(ctx, id) == NULL);

  // We don't set node->LastFrameAlive on construction. Nodes are always created
  // at all time to reflect .ini settings!
  DEBUG_LOG_DOCKING("[docking] DockContextAddNode 0x%08X\n", id);
  DockNode *node = NEW(DockNode)(id);
  ctx->DockContext.Nodes.SetVoidPtr(node->ID, node);
  return node;
}

static void Gui::DockContextRemoveNode(Context *ctx, DockNode *node,
                                       bool merge_sibling_into_parent_node) {
  Context &g = *ctx;
  DockContext *dc = &ctx->DockContext;

  DEBUG_LOG_DOCKING("[docking] DockContextRemoveNode 0x%08X\n", node->ID);
  ASSERT(DockContextFindNodeByID(ctx, node->ID) == node);
  ASSERT(node->ChildNodes[0] == NULL && node->ChildNodes[1] == NULL);
  ASSERT(node->Windows.Size == 0);

  if (node->HostWindow)
    node->HostWindow->DockNodeAsHost = NULL;

  DockNode *parent_node = node->ParentNode;
  const bool merge = (merge_sibling_into_parent_node && parent_node != NULL);
  if (merge) {
    ASSERT(parent_node->ChildNodes[0] == node ||
           parent_node->ChildNodes[1] == node);
    DockNode *sibling_node =
        (parent_node->ChildNodes[0] == node ? parent_node->ChildNodes[1]
                                            : parent_node->ChildNodes[0]);
    DockNodeTreeMerge(&g, parent_node, sibling_node);
  } else {
    for (int n = 0; parent_node && n < ARRAYSIZE(parent_node->ChildNodes); n++)
      if (parent_node->ChildNodes[n] == node)
        node->ParentNode->ChildNodes[n] = NULL;
    dc->Nodes.SetVoidPtr(node->ID, NULL);
    DELETE(node);
  }
}

static int CDECL DockNodeComparerDepthMostFirst(const void *lhs,
                                                const void *rhs) {
  const DockNode *a = *(const DockNode *const *)lhs;
  const DockNode *b = *(const DockNode *const *)rhs;
  return Gui::DockNodeGetDepth(b) - Gui::DockNodeGetDepth(a);
}

// Pre C++0x doesn't allow us to use a function-local type (without linkage) as
// template parameter, so we moved this here.
struct DockContextPruneNodeData {
  int CountWindows, CountChildWindows, CountChildNodes;
  ID RootId;
  DockContextPruneNodeData() {
    CountWindows = CountChildWindows = CountChildNodes = 0;
    RootId = 0;
  }
};

// Garbage collect unused nodes (run once at init time)
static void Gui::DockContextPruneUnusedSettingsNodes(Context *ctx) {
  Context &g = *ctx;
  DockContext *dc = &ctx->DockContext;
  ASSERT(g.Windows.Size == 0);

  Pool<DockContextPruneNodeData> pool;
  pool.Reserve(dc->NodesSettings.Size);

  // Count child nodes and compute RootID
  for (int settings_n = 0; settings_n < dc->NodesSettings.Size; settings_n++) {
    DockNodeSettings *settings = &dc->NodesSettings[settings_n];
    DockContextPruneNodeData *parent_data =
        settings->ParentNodeId ? pool.GetByKey(settings->ParentNodeId) : 0;
    pool.GetOrAddByKey(settings->ID)->RootId =
        parent_data ? parent_data->RootId : settings->ID;
    if (settings->ParentNodeId)
      pool.GetOrAddByKey(settings->ParentNodeId)->CountChildNodes++;
  }

  // Count reference to dock ids from dockspaces
  // We track the 'auto-DockNode <- manual-Window <- manual-DockSpace' in order
  // to avoid 'auto-DockNode' being ditched by
  // DockContextPruneUnusedSettingsNodes()
  for (int settings_n = 0; settings_n < dc->NodesSettings.Size; settings_n++) {
    DockNodeSettings *settings = &dc->NodesSettings[settings_n];
    if (settings->ParentWindowId != 0)
      if (WindowSettings *window_settings =
              FindWindowSettingsByID(settings->ParentWindowId))
        if (window_settings->DockId)
          if (DockContextPruneNodeData *data =
                  pool.GetByKey(window_settings->DockId))
            data->CountChildNodes++;
  }

  // Count reference to dock ids from window settings
  // We guard against the possibility of an invalid .ini file (RootID may point
  // to a missing node)
  for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
       settings = g.SettingsWindows.next_chunk(settings))
    if (ID dock_id = settings->DockId)
      if (DockContextPruneNodeData *data = pool.GetByKey(dock_id)) {
        data->CountWindows++;
        if (DockContextPruneNodeData *data_root =
                (data->RootId == dock_id) ? data : pool.GetByKey(data->RootId))
          data_root->CountChildWindows++;
      }

  // Prune
  for (int settings_n = 0; settings_n < dc->NodesSettings.Size; settings_n++) {
    DockNodeSettings *settings = &dc->NodesSettings[settings_n];
    DockContextPruneNodeData *data = pool.GetByKey(settings->ID);
    if (data->CountWindows > 1)
      continue;
    DockContextPruneNodeData *data_root =
        (data->RootId == settings->ID) ? data : pool.GetByKey(data->RootId);

    bool remove = false;
    remove |=
        (data->CountWindows == 1 && settings->ParentNodeId == 0 &&
         data->CountChildNodes == 0 &&
         !(settings->Flags & DockNodeFlags_CentralNode)); // Floating root node
                                                          // with only 1 window
    remove |= (data->CountWindows == 0 && settings->ParentNodeId == 0 &&
               data->CountChildNodes == 0); // Leaf nodes with 0 window
    remove |= (data_root->CountChildWindows == 0);
    if (remove) {
      DEBUG_LOG_DOCKING(
          "[docking] DockContextPruneUnusedSettingsNodes: Prune 0x%08X\n",
          settings->ID);
      DockSettingsRemoveNodeReferences(&settings->ID, 1);
      settings->ID = 0;
    }
  }
}

static void
Gui::DockContextBuildNodesFromSettings(Context *ctx,
                                       DockNodeSettings *node_settings_array,
                                       int node_settings_count) {
  // Build nodes
  for (int node_n = 0; node_n < node_settings_count; node_n++) {
    DockNodeSettings *settings = &node_settings_array[node_n];
    if (settings->ID == 0)
      continue;
    DockNode *node = DockContextAddNode(ctx, settings->ID);
    node->ParentNode =
        settings->ParentNodeId
            ? DockContextFindNodeByID(ctx, settings->ParentNodeId)
            : NULL;
    node->Pos = Vec2(settings->Pos.x, settings->Pos.y);
    node->Size = Vec2(settings->Size.x, settings->Size.y);
    node->SizeRef = Vec2(settings->SizeRef.x, settings->SizeRef.y);
    node->AuthorityForPos = node->AuthorityForSize =
        node->AuthorityForViewport = DataAuthority_DockNode;
    if (node->ParentNode && node->ParentNode->ChildNodes[0] == NULL)
      node->ParentNode->ChildNodes[0] = node;
    else if (node->ParentNode && node->ParentNode->ChildNodes[1] == NULL)
      node->ParentNode->ChildNodes[1] = node;
    node->SelectedTabId = settings->SelectedTabId;
    node->SplitAxis = (Axis)settings->SplitAxis;
    node->SetLocalFlags(settings->Flags & DockNodeFlags_SavedFlagsMask_);

    // Bind host window immediately if it already exist (in case of a rebuild)
    // This is useful as the RootWindowForTitleBarHighlight links necessary to
    // highlight the currently focused node requires node->HostWindow to be set.
    char host_window_title[20];
    DockNode *root_node = DockNodeGetRootNode(node);
    node->HostWindow = FindWindowByName(DockNodeGetHostWindowTitle(
        root_node, host_window_title, ARRAYSIZE(host_window_title)));
  }
}

void Gui::DockContextBuildAddWindowsToNodes(Context *ctx, ID root_id) {
  // Rebind all windows to nodes (they can also lazily rebind but we'll have a
  // visible glitch during the first frame)
  Context &g = *ctx;
  for (Window *window : g.Windows) {
    if (window->DockId == 0 || window->LastFrameActive < g.FrameCount - 1)
      continue;
    if (window->DockNode != NULL)
      continue;

    DockNode *node = DockContextFindNodeByID(ctx, window->DockId);
    ASSERT(node != NULL); // This should have been called after
                          // DockContextBuildNodesFromSettings()
    if (root_id == 0 || DockNodeGetRootNode(node)->ID == root_id)
      DockNodeAddWindow(node, window, true);
  }
}

//-----------------------------------------------------------------------------
// Docking: DockContext Docking/Undocking functions
//-----------------------------------------------------------------------------
// - DockContextQueueDock()
// - DockContextQueueUndockWindow()
// - DockContextQueueUndockNode()
// - DockContextQueueNotifyRemovedNode()
// - DockContextProcessDock()
// - DockContextProcessUndockWindow()
// - DockContextProcessUndockNode()
// - DockContextCalcDropPosForDocking()
//-----------------------------------------------------------------------------

void Gui::DockContextQueueDock(Context *ctx, Window *target,
                               DockNode *target_node, Window *payload,
                               Dir split_dir, float split_ratio,
                               bool split_outer) {
  ASSERT(target != payload);
  DockRequest req;
  req.Type = DockRequestType_Dock;
  req.DockTargetWindow = target;
  req.DockTargetNode = target_node;
  req.DockPayload = payload;
  req.DockSplitDir = split_dir;
  req.DockSplitRatio = split_ratio;
  req.DockSplitOuter = split_outer;
  ctx->DockContext.Requests.push_back(req);
}

void Gui::DockContextQueueUndockWindow(Context *ctx, Window *window) {
  DockRequest req;
  req.Type = DockRequestType_Undock;
  req.UndockTargetWindow = window;
  ctx->DockContext.Requests.push_back(req);
}

void Gui::DockContextQueueUndockNode(Context *ctx, DockNode *node) {
  DockRequest req;
  req.Type = DockRequestType_Undock;
  req.UndockTargetNode = node;
  ctx->DockContext.Requests.push_back(req);
}

void Gui::DockContextQueueNotifyRemovedNode(Context *ctx, DockNode *node) {
  DockContext *dc = &ctx->DockContext;
  for (DockRequest &req : dc->Requests)
    if (req.DockTargetNode == node)
      req.Type = DockRequestType_None;
}

void Gui::DockContextProcessDock(Context *ctx, DockRequest *req) {
  ASSERT((req->Type == DockRequestType_Dock && req->DockPayload != NULL) ||
         (req->Type == DockRequestType_Split && req->DockPayload == NULL));
  ASSERT(req->DockTargetWindow != NULL || req->DockTargetNode != NULL);

  Context &g = *ctx;
  UNUSED(g);

  Window *payload_window = req->DockPayload; // Optional
  Window *target_window = req->DockTargetWindow;
  DockNode *node = req->DockTargetNode;
  if (payload_window)
    DEBUG_LOG_DOCKING("[docking] DockContextProcessDock node 0x%08X target "
                      "'%s' dock window '%s', split_dir %d\n",
                      node ? node->ID : 0,
                      target_window ? target_window->Name : "NULL",
                      payload_window->Name, req->DockSplitDir);
  else
    DEBUG_LOG_DOCKING(
        "[docking] DockContextProcessDock node 0x%08X, split_dir %d\n",
        node ? node->ID : 0, req->DockSplitDir);

  // Decide which Tab will be selected at the end of the operation
  ID next_selected_id = 0;
  DockNode *payload_node = NULL;
  if (payload_window) {
    payload_node = payload_window->DockNodeAsHost;
    payload_window->DockNodeAsHost =
        NULL; // Important to clear this as the node will have its life as a
              // child which might be merged/deleted later.
    if (payload_node && payload_node->IsLeafNode())
      next_selected_id = payload_node->TabBar->NextSelectedTabId
                             ? payload_node->TabBar->NextSelectedTabId
                             : payload_node->TabBar->SelectedTabId;
    if (payload_node == NULL)
      next_selected_id = payload_window->TabId;
  }

  // FIXME-DOCK: When we are trying to dock an existing single-window node into
  // a loose window, transfer Node ID as well When processing an interactive
  // split, usually LastFrameAlive will be < g.FrameCount. But DockBuilder
  // operations can make it ==.
  if (node)
    ASSERT(node->LastFrameAlive <= g.FrameCount);
  if (node && target_window && node == target_window->DockNodeAsHost)
    ASSERT(node->Windows.Size > 0 || node->IsSplitNode() ||
           node->IsCentralNode());

  // Create new node and add existing window to it
  if (node == NULL) {
    node = DockContextAddNode(ctx, 0);
    node->Pos = target_window->Pos;
    node->Size = target_window->Size;
    if (target_window->DockNodeAsHost == NULL) {
      DockNodeAddWindow(node, target_window, true);
      node->TabBar->Tabs[0].Flags &= ~TabItemFlags_Unsorted;
      target_window->DockIsActive = true;
    }
  }

  Dir split_dir = req->DockSplitDir;
  if (split_dir != Dir_None) {
    // Split into two, one side will be our payload node unless we are dropping
    // a loose window
    const Axis split_axis =
        (split_dir == Dir_Left || split_dir == Dir_Right) ? Axis_X : Axis_Y;
    const int split_inheritor_child_idx =
        (split_dir == Dir_Left || split_dir == Dir_Up)
            ? 1
            : 0; // Current contents will be moved to the opposite side
    const float split_ratio = req->DockSplitRatio;
    DockNodeTreeSplit(ctx, node, split_axis, split_inheritor_child_idx,
                      split_ratio,
                      payload_node); // payload_node may be NULL here!
    DockNode *new_node = node->ChildNodes[split_inheritor_child_idx ^ 1];
    new_node->HostWindow = node->HostWindow;
    node = new_node;
  }
  node->SetLocalFlags(node->LocalFlags & ~DockNodeFlags_HiddenTabBar);

  if (node != payload_node) {
    // Create tab bar before we call DockNodeMoveWindows (which would attempt to
    // move the old tab-bar, which would lead us to payload tabs wrongly
    // appearing before target tabs!)
    if (node->Windows.Size > 0 && node->TabBar == NULL) {
      DockNodeAddTabBar(node);
      for (int n = 0; n < node->Windows.Size; n++)
        TabBarAddTab(node->TabBar, TabItemFlags_None, node->Windows[n]);
    }

    if (payload_node != NULL) {
      // Transfer full payload node (with 1+ child windows or child nodes)
      if (payload_node->IsSplitNode()) {
        if (node->Windows.Size > 0) {
          // We can dock a split payload into a node that already has windows
          // _only_ if our payload is a node tree with a single visible node. In
          // this situation, we move the windows of the target node into the
          // currently visible node of the payload. This allows us to preserve
          // some of the underlying dock tree settings nicely.
          ASSERT(
              payload_node->OnlyNodeWithWindows !=
              NULL); // The docking should have been blocked by
                     // DockNodePreviewDockSetup() early on and never submitted.
          DockNode *visible_node = payload_node->OnlyNodeWithWindows;
          if (visible_node->TabBar)
            ASSERT(visible_node->TabBar->Tabs.Size > 0);
          DockNodeMoveWindows(node, visible_node);
          DockNodeMoveWindows(visible_node, node);
          DockSettingsRenameNodeReferences(node->ID, visible_node->ID);
        }
        if (node->IsCentralNode()) {
          // Central node property needs to be moved to a leaf node, pick the
          // last focused one.
          // FIXME-DOCK: If we had to transfer other flags here, what would the
          // policy be?
          DockNode *last_focused_node =
              DockContextFindNodeByID(ctx, payload_node->LastFocusedNodeId);
          ASSERT(last_focused_node != NULL);
          DockNode *last_focused_root_node =
              DockNodeGetRootNode(last_focused_node);
          ASSERT(last_focused_root_node == DockNodeGetRootNode(payload_node));
          last_focused_node->SetLocalFlags(last_focused_node->LocalFlags |
                                           DockNodeFlags_CentralNode);
          node->SetLocalFlags(node->LocalFlags & ~DockNodeFlags_CentralNode);
          last_focused_root_node->CentralNode = last_focused_node;
        }

        ASSERT(node->Windows.Size == 0);
        DockNodeMoveChildNodes(node, payload_node);
      } else {
        const ID payload_dock_id = payload_node->ID;
        DockNodeMoveWindows(node, payload_node);
        DockSettingsRenameNodeReferences(payload_dock_id, node->ID);
      }
      DockContextRemoveNode(ctx, payload_node, true);
    } else if (payload_window) {
      // Transfer single window
      const ID payload_dock_id = payload_window->DockId;
      node->VisibleWindow = payload_window;
      DockNodeAddWindow(node, payload_window, true);
      if (payload_dock_id != 0)
        DockSettingsRenameNodeReferences(payload_dock_id, node->ID);
    }
  } else {
    // When docking a floating single window node we want to reevaluate
    // auto-hiding of the tab bar
    node->WantHiddenTabBarUpdate = true;
  }

  // Update selection immediately
  if (TabBar *tab_bar = node->TabBar)
    tab_bar->NextSelectedTabId = next_selected_id;
  MarkIniSettingsDirty();
}

// Problem:
//   Undocking a large (~full screen) window would leave it so large that the
//   bottom right sizing corner would more than likely be off the screen and the
//   window would be hard to resize to fit on screen. This can be particularly
//   problematic with 'ConfigWindowsMoveFromTitleBarOnly=true' and/or with
//   'ConfigWindowsResizeFromEdges=false' as well (the later can be due to
//   missing BackendFlags_HasMouseCursors backend flag).
// Solution:
//   When undocking a window we currently force its maximum size to 90% of the
//   host viewport or monitor.
// Reevaluate this when we implement preserving docked/undocked size
// ("docking_wip/undocked_size" branch).
static Vec2 FixLargeWindowsWhenUndocking(const Vec2 &size,
                                         Viewport *ref_viewport) {
  if (ref_viewport == NULL)
    return size;

  Context &g = *GGui;
  Vec2 max_size = Trunc(ref_viewport->WorkSize * 0.90f);
  if (g.ConfigFlagsCurrFrame & ConfigFlags_ViewportsEnable) {
    const PlatformMonitor *monitor =
        Gui::GetViewportPlatformMonitor(ref_viewport);
    max_size = Trunc(monitor->WorkSize * 0.90f);
  }
  return Min(size, max_size);
}

void Gui::DockContextProcessUndockWindow(Context *ctx, Window *window,
                                         bool clear_persistent_docking_ref) {
  Context &g = *ctx;
  DEBUG_LOG_DOCKING("[docking] DockContextProcessUndockWindow window '%s', "
                    "clear_persistent_docking_ref = %d\n",
                    window->Name, clear_persistent_docking_ref);
  if (window->DockNode)
    DockNodeRemoveWindow(window->DockNode, window,
                         clear_persistent_docking_ref ? 0 : window->DockId);
  else
    window->DockId = 0;
  window->Collapsed = false;
  window->DockIsActive = false;
  window->DockNodeIsVisible = window->DockTabIsVisible = false;
  window->Size = window->SizeFull =
      FixLargeWindowsWhenUndocking(window->SizeFull, window->Viewport);

  MarkIniSettingsDirty();
}

void Gui::DockContextProcessUndockNode(Context *ctx, DockNode *node) {
  Context &g = *ctx;
  DEBUG_LOG_DOCKING("[docking] DockContextProcessUndockNode node %08X\n",
                    node->ID);
  ASSERT(node->IsLeafNode());
  ASSERT(node->Windows.Size >= 1);

  if (node->IsRootNode() || node->IsCentralNode()) {
    // In the case of a root node or central node, the node will have to stay in
    // place. Create a new node to receive the payload.
    DockNode *new_node = DockContextAddNode(ctx, 0);
    new_node->Pos = node->Pos;
    new_node->Size = node->Size;
    new_node->SizeRef = node->SizeRef;
    DockNodeMoveWindows(new_node, node);
    DockSettingsRenameNodeReferences(node->ID, new_node->ID);
    node = new_node;
  } else {
    // Otherwise extract our node and merge our sibling back into the parent
    // node.
    ASSERT(node->ParentNode->ChildNodes[0] == node ||
           node->ParentNode->ChildNodes[1] == node);
    int index_in_parent = (node->ParentNode->ChildNodes[0] == node) ? 0 : 1;
    node->ParentNode->ChildNodes[index_in_parent] = NULL;
    DockNodeTreeMerge(ctx, node->ParentNode,
                      node->ParentNode->ChildNodes[index_in_parent ^ 1]);
    node->ParentNode->AuthorityForViewport =
        DataAuthority_Window; // The node that stays in place keeps the
                              // viewport, so our newly dragged out node
                              // will create a new viewport
    node->ParentNode = NULL;
  }
  for (Window *window : node->Windows) {
    window->Flags &= ~WindowFlags_ChildWindow;
    if (window->ParentWindow)
      window->ParentWindow->DC.ChildWindows.find_erase(window);
    UpdateWindowParentAndRootLinks(window, window->Flags, NULL);
  }
  node->AuthorityForPos = node->AuthorityForSize = DataAuthority_DockNode;
  node->Size =
      FixLargeWindowsWhenUndocking(node->Size, node->Windows[0]->Viewport);
  node->WantMouseMove = true;
  MarkIniSettingsDirty();
}

// This is mostly used for automation.
bool Gui::DockContextCalcDropPosForDocking(
    Window *target, DockNode *target_node, Window *payload_window,
    DockNode *payload_node, Dir split_dir, bool split_outer, Vec2 *out_pos) {
  if (target != NULL && target_node == NULL)
    target_node = target->DockNode;

  // In DockNodePreviewDockSetup() for a root central node instead of showing
  // both "inner" and "outer" drop rects (which would be functionally identical)
  // we only show the outer one. Reflect this here.
  if (target_node && target_node->ParentNode == NULL &&
      target_node->IsCentralNode() && split_dir != Dir_None)
    split_outer = true;
  DockPreviewData split_data;
  DockNodePreviewDockSetup(target, target_node, payload_window, payload_node,
                           &split_data, false, split_outer);
  if (split_data.DropRectsDraw[split_dir + 1].IsInverted())
    return false;
  *out_pos = split_data.DropRectsDraw[split_dir + 1].GetCenter();
  return true;
}

//-----------------------------------------------------------------------------
// Docking: DockNode
//-----------------------------------------------------------------------------
// - DockNodeGetTabOrder()
// - DockNodeAddWindow()
// - DockNodeRemoveWindow()
// - DockNodeMoveChildNodes()
// - DockNodeMoveWindows()
// - DockNodeApplyPosSizeToWindows()
// - DockNodeHideHostWindow()
// - DockNodeFindInfoResults
// - DockNodeFindInfo()
// - DockNodeFindWindowByID()
// - DockNodeUpdateFlagsAndCollapse()
// - DockNodeUpdateHasCentralNodeFlag()
// - DockNodeUpdateVisibleFlag()
// - DockNodeStartMouseMovingWindow()
// - DockNodeUpdate()
// - DockNodeUpdateWindowMenu()
// - DockNodeBeginAmendTabBar()
// - DockNodeEndAmendTabBar()
// - DockNodeUpdateTabBar()
// - DockNodeAddTabBar()
// - DockNodeRemoveTabBar()
// - DockNodeIsDropAllowedOne()
// - DockNodeIsDropAllowed()
// - DockNodeCalcTabBarLayout()
// - DockNodeCalcSplitRects()
// - DockNodeCalcDropRectsAndTestMousePos()
// - DockNodePreviewDockSetup()
// - DockNodePreviewDockRender()
//-----------------------------------------------------------------------------

DockNode::DockNode(ID id) {
  ID = id;
  SharedFlags = LocalFlags = LocalFlagsInWindows = MergedFlags =
      DockNodeFlags_None;
  ParentNode = ChildNodes[0] = ChildNodes[1] = NULL;
  TabBar = NULL;
  SplitAxis = Axis_None;

  State = DockNodeState_Unknown;
  LastBgColor = COL32_WHITE;
  HostWindow = VisibleWindow = NULL;
  CentralNode = OnlyNodeWithWindows = NULL;
  CountNodeWithWindows = 0;
  LastFrameAlive = LastFrameActive = LastFrameFocused = -1;
  LastFocusedNodeId = 0;
  SelectedTabId = 0;
  WantCloseTabId = 0;
  RefViewportId = 0;
  AuthorityForPos = AuthorityForSize = DataAuthority_DockNode;
  AuthorityForViewport = DataAuthority_Auto;
  IsVisible = true;
  IsFocused = HasCloseButton = HasWindowMenuButton = HasCentralNodeChild =
      false;
  IsBgDrawnThisFrame = false;
  WantCloseAll = WantLockSizeOnce = WantMouseMove = WantHiddenTabBarUpdate =
      WantHiddenTabBarToggle = false;
}

DockNode::~DockNode() {
  DELETE(TabBar);
  TabBar = NULL;
  ChildNodes[0] = ChildNodes[1] = NULL;
}

int Gui::DockNodeGetTabOrder(Window *window) {
  TabBar *tab_bar = window->DockNode->TabBar;
  if (tab_bar == NULL)
    return -1;
  TabItem *tab = TabBarFindTabByID(tab_bar, window->TabId);
  return tab ? TabBarGetTabOrder(tab_bar, tab) : -1;
}

static void DockNodeHideWindowDuringHostWindowCreation(Window *window) {
  window->Hidden = true;
  window->HiddenFramesCanSkipItems = window->Active ? 1 : 2;
}

static void Gui::DockNodeAddWindow(DockNode *node, Window *window,
                                   bool add_to_tab_bar) {
  Context &g = *GGui;
  (void)g;
  if (window->DockNode) {
    // Can overwrite an existing window->DockNode (e.g. pointing to a disabled
    // DockSpace node)
    ASSERT(window->DockNode->ID != node->ID);
    DockNodeRemoveWindow(window->DockNode, window, 0);
  }
  ASSERT(window->DockNode == NULL || window->DockNodeAsHost == NULL);
  DEBUG_LOG_DOCKING("[docking] DockNodeAddWindow node 0x%08X window '%s'\n",
                    node->ID, window->Name);

  // If more than 2 windows appeared on the same frame leading to the creation
  // of a new hosting window, we'll hide windows until the host window is ready.
  // Hide the 1st window after its been output (so it is not visible for one
  // frame). We will call DockNodeHideWindowDuringHostWindowCreation() on
  // ourselves in Begin()
  if (node->HostWindow == NULL && node->Windows.Size == 1 &&
      node->Windows[0]->WasActive == false)
    DockNodeHideWindowDuringHostWindowCreation(node->Windows[0]);

  node->Windows.push_back(window);
  node->WantHiddenTabBarUpdate = true;
  window->DockNode = node;
  window->DockId = node->ID;
  window->DockIsActive = (node->Windows.Size > 1);
  window->DockTabWantClose = false;

  // When reactivating a node with one or two loose window, the window
  // pos/size/viewport are authoritative over the node storage. In particular it
  // is important we init the viewport from the first window so we don't create
  // two viewports and drop one.
  if (node->HostWindow == NULL && node->IsFloatingNode()) {
    if (node->AuthorityForPos == DataAuthority_Auto)
      node->AuthorityForPos = DataAuthority_Window;
    if (node->AuthorityForSize == DataAuthority_Auto)
      node->AuthorityForSize = DataAuthority_Window;
    if (node->AuthorityForViewport == DataAuthority_Auto)
      node->AuthorityForViewport = DataAuthority_Window;
  }

  // Add to tab bar if requested
  if (add_to_tab_bar) {
    if (node->TabBar == NULL) {
      DockNodeAddTabBar(node);
      node->TabBar->SelectedTabId = node->TabBar->NextSelectedTabId =
          node->SelectedTabId;

      // Add existing windows
      for (int n = 0; n < node->Windows.Size - 1; n++)
        TabBarAddTab(node->TabBar, TabItemFlags_None, node->Windows[n]);
    }
    TabBarAddTab(node->TabBar, TabItemFlags_Unsorted, window);
  }

  DockNodeUpdateVisibleFlag(node);

  // Update this without waiting for the next time we Begin() in the window, so
  // our host window will have the proper title bar color on its first frame.
  if (node->HostWindow)
    UpdateWindowParentAndRootLinks(
        window, window->Flags | WindowFlags_ChildWindow, node->HostWindow);
}

static void Gui::DockNodeRemoveWindow(DockNode *node, Window *window,
                                      ID save_dock_id) {
  Context &g = *GGui;
  ASSERT(window->DockNode == node);
  // ASSERT(window->RootWindowDockTree == node->HostWindow);
  // ASSERT(window->LastFrameActive < g.FrameCount);    // We may call this
  // from Begin()
  ASSERT(save_dock_id == 0 || save_dock_id == node->ID);
  DEBUG_LOG_DOCKING("[docking] DockNodeRemoveWindow node 0x%08X window '%s'\n",
                    node->ID, window->Name);

  window->DockNode = NULL;
  window->DockIsActive = window->DockTabWantClose = false;
  window->DockId = save_dock_id;
  window->Flags &= ~WindowFlags_ChildWindow;
  if (window->ParentWindow)
    window->ParentWindow->DC.ChildWindows.find_erase(window);
  UpdateWindowParentAndRootLinks(window, window->Flags,
                                 NULL); // Update immediately

  if (node->HostWindow && node->HostWindow->ViewportOwned) {
    // When undocking from a user interaction this will always run in NewFrame()
    // and have not much effect. But mid-frame, if we clear viewport we need to
    // mark window as hidden as well.
    window->Viewport = NULL;
    window->ViewportId = 0;
    window->ViewportOwned = false;
    window->Hidden = true;
  }

  // Remove window
  bool erased = false;
  for (int n = 0; n < node->Windows.Size; n++)
    if (node->Windows[n] == window) {
      node->Windows.erase(node->Windows.Data + n);
      erased = true;
      break;
    }
  if (!erased)
    ASSERT(erased);
  if (node->VisibleWindow == window)
    node->VisibleWindow = NULL;

  // Remove tab and possibly tab bar
  node->WantHiddenTabBarUpdate = true;
  if (node->TabBar) {
    TabBarRemoveTab(node->TabBar, window->TabId);
    const int tab_count_threshold_for_tab_bar = node->IsCentralNode() ? 1 : 2;
    if (node->Windows.Size < tab_count_threshold_for_tab_bar)
      DockNodeRemoveTabBar(node);
  }

  if (node->Windows.Size == 0 && !node->IsCentralNode() &&
      !node->IsDockSpace() && window->DockId != node->ID) {
    // Automatic dock node delete themselves if they are not holding at least
    // one tab
    DockContextRemoveNode(&g, node, true);
    return;
  }

  if (node->Windows.Size == 1 && !node->IsCentralNode() && node->HostWindow) {
    Window *remaining_window = node->Windows[0];
    // Note: we used to transport viewport ownership here.
    remaining_window->Collapsed = node->HostWindow->Collapsed;
  }

  // Update visibility immediately is required so the
  // DockNodeUpdateRemoveInactiveChilds() processing can reflect changes up the
  // tree
  DockNodeUpdateVisibleFlag(node);
}

static void Gui::DockNodeMoveChildNodes(DockNode *dst_node,
                                        DockNode *src_node) {
  ASSERT(dst_node->Windows.Size == 0);
  dst_node->ChildNodes[0] = src_node->ChildNodes[0];
  dst_node->ChildNodes[1] = src_node->ChildNodes[1];
  if (dst_node->ChildNodes[0])
    dst_node->ChildNodes[0]->ParentNode = dst_node;
  if (dst_node->ChildNodes[1])
    dst_node->ChildNodes[1]->ParentNode = dst_node;
  dst_node->SplitAxis = src_node->SplitAxis;
  dst_node->SizeRef = src_node->SizeRef;
  src_node->ChildNodes[0] = src_node->ChildNodes[1] = NULL;
}

static void Gui::DockNodeMoveWindows(DockNode *dst_node, DockNode *src_node) {
  // Insert tabs in the same orders as currently ordered (node->Windows isn't
  // ordered)
  ASSERT(src_node && dst_node && dst_node != src_node);
  TabBar *src_tab_bar = src_node->TabBar;
  if (src_tab_bar != NULL)
    ASSERT(src_node->Windows.Size <= src_node->TabBar->Tabs.Size);

  // If the dst_node is empty we can just move the entire tab bar (to preserve
  // selection, scrolling, etc.)
  bool move_tab_bar = (src_tab_bar != NULL) && (dst_node->TabBar == NULL);
  if (move_tab_bar) {
    dst_node->TabBar = src_node->TabBar;
    src_node->TabBar = NULL;
  }

  // Tab order is not important here, it is preserved by sorting in
  // DockNodeUpdateTabBar().
  for (Window *window : src_node->Windows) {
    window->DockNode = NULL;
    window->DockIsActive = false;
    DockNodeAddWindow(dst_node, window, !move_tab_bar);
  }
  src_node->Windows.clear();

  if (!move_tab_bar && src_node->TabBar) {
    if (dst_node->TabBar)
      dst_node->TabBar->SelectedTabId = src_node->TabBar->SelectedTabId;
    DockNodeRemoveTabBar(src_node);
  }
}

static void Gui::DockNodeApplyPosSizeToWindows(DockNode *node) {
  for (Window *window : node->Windows) {
    SetWindowPos(window, node->Pos,
                 Cond_Always); // We don't assign directly to Pos because
                               // it can break the calculation of
                               // SizeContents on next frame
    SetWindowSize(window, node->Size, Cond_Always);
  }
}

static void Gui::DockNodeHideHostWindow(DockNode *node) {
  if (node->HostWindow) {
    if (node->HostWindow->DockNodeAsHost == node)
      node->HostWindow->DockNodeAsHost = NULL;
    node->HostWindow = NULL;
  }

  if (node->Windows.Size == 1) {
    node->VisibleWindow = node->Windows[0];
    node->Windows[0]->DockIsActive = false;
  }

  if (node->TabBar)
    DockNodeRemoveTabBar(node);
}

// Search function called once by root node in DockNodeUpdate()
struct DockNodeTreeInfo {
  DockNode *CentralNode;
  DockNode *FirstNodeWithWindows;
  int CountNodesWithWindows;
  // WindowClass  WindowClassForMerges;

  DockNodeTreeInfo() { memset(this, 0, sizeof(*this)); }
};

static void DockNodeFindInfo(DockNode *node, DockNodeTreeInfo *info) {
  if (node->Windows.Size > 0) {
    if (info->FirstNodeWithWindows == NULL)
      info->FirstNodeWithWindows = node;
    info->CountNodesWithWindows++;
  }
  if (node->IsCentralNode()) {
    ASSERT(info->CentralNode == NULL); // Should be only one
    ASSERT(node->IsLeafNode() &&
           "If you get this assert: please submit .ini file + repro of "
           "actions leading to this.");
    info->CentralNode = node;
  }
  if (info->CountNodesWithWindows > 1 && info->CentralNode != NULL)
    return;
  if (node->ChildNodes[0])
    DockNodeFindInfo(node->ChildNodes[0], info);
  if (node->ChildNodes[1])
    DockNodeFindInfo(node->ChildNodes[1], info);
}

static Window *Gui::DockNodeFindWindowByID(DockNode *node, ID id) {
  ASSERT(id != 0);
  for (Window *window : node->Windows)
    if (window->ID == id)
      return window;
  return NULL;
}

// - Remove inactive windows/nodes.
// - Update visibility flag.
static void Gui::DockNodeUpdateFlagsAndCollapse(DockNode *node) {
  Context &g = *GGui;
  ASSERT(node->ParentNode == NULL || node->ParentNode->ChildNodes[0] == node ||
         node->ParentNode->ChildNodes[1] == node);

  // Inherit most flags
  if (node->ParentNode)
    node->SharedFlags =
        node->ParentNode->SharedFlags & DockNodeFlags_SharedFlagsInheritMask_;

  // Recurse into children
  // There is the possibility that one of our child becoming empty will delete
  // itself and moving its sibling contents into 'node'. If 'node->ChildNode[0]'
  // delete itself, then 'node->ChildNode[1]->Windows' will be moved into 'node'
  // If 'node->ChildNode[1]' delete itself, then 'node->ChildNode[0]->Windows'
  // will be moved into 'node' and the "remove inactive windows" loop will have
  // run twice on those windows (harmless)
  node->HasCentralNodeChild = false;
  if (node->ChildNodes[0])
    DockNodeUpdateFlagsAndCollapse(node->ChildNodes[0]);
  if (node->ChildNodes[1])
    DockNodeUpdateFlagsAndCollapse(node->ChildNodes[1]);

  // Remove inactive windows, collapse nodes
  // Merge node flags overrides stored in windows
  node->LocalFlagsInWindows = DockNodeFlags_None;
  for (int window_n = 0; window_n < node->Windows.Size; window_n++) {
    Window *window = node->Windows[window_n];
    ASSERT(window->DockNode == node);

    bool node_was_active = (node->LastFrameActive + 1 == g.FrameCount);
    bool remove = false;
    remove |= node_was_active && (window->LastFrameActive + 1 < g.FrameCount);
    remove |= node_was_active &&
              (node->WantCloseAll || node->WantCloseTabId == window->TabId) &&
              window->HasCloseButton &&
              !(window->Flags &
                WindowFlags_UnsavedDocument); // Submit all _expected_
                                              // closure from last frame
    remove |= (window->DockTabWantClose);
    if (remove) {
      window->DockTabWantClose = false;
      if (node->Windows.Size == 1 && !node->IsCentralNode()) {
        DockNodeHideHostWindow(node);
        node->State = DockNodeState_HostWindowHiddenBecauseSingleWindow;
        DockNodeRemoveWindow(
            node, window,
            node->ID); // Will delete the node so it'll be invalid on return
        return;
      }
      DockNodeRemoveWindow(node, window, node->ID);
      window_n--;
      continue;
    }

    // FIXME-DOCKING: Missing policies for conflict resolution, hence the
    // "Experimental" tag on this.
    // node->LocalFlagsInWindow &=
    // ~window->WindowClass.DockNodeFlagsOverrideClear;
    node->LocalFlagsInWindows |= window->WindowClass.DockNodeFlagsOverrideSet;
  }
  node->UpdateMergedFlags();

  // Auto-hide tab bar option
  DockNodeFlags node_flags = node->MergedFlags;
  if (node->WantHiddenTabBarUpdate && node->Windows.Size == 1 &&
      (node_flags & DockNodeFlags_AutoHideTabBar) && !node->IsHiddenTabBar())
    node->WantHiddenTabBarToggle = true;
  node->WantHiddenTabBarUpdate = false;

  // Cancel toggling if we know our tab bar is enforced to be hidden at all
  // times
  if (node->WantHiddenTabBarToggle && node->VisibleWindow &&
      (node->VisibleWindow->WindowClass.DockNodeFlagsOverrideSet &
       DockNodeFlags_HiddenTabBar))
    node->WantHiddenTabBarToggle = false;

  // Apply toggles at a single point of the frame (here!)
  if (node->Windows.Size > 1)
    node->SetLocalFlags(node->LocalFlags & ~DockNodeFlags_HiddenTabBar);
  else if (node->WantHiddenTabBarToggle)
    node->SetLocalFlags(node->LocalFlags ^ DockNodeFlags_HiddenTabBar);
  node->WantHiddenTabBarToggle = false;

  DockNodeUpdateVisibleFlag(node);
}

// This is rarely called as DockNodeUpdateForRootNode() generally does it most
// frames.
static void Gui::DockNodeUpdateHasCentralNodeChild(DockNode *node) {
  node->HasCentralNodeChild = false;
  if (node->ChildNodes[0])
    DockNodeUpdateHasCentralNodeChild(node->ChildNodes[0]);
  if (node->ChildNodes[1])
    DockNodeUpdateHasCentralNodeChild(node->ChildNodes[1]);
  if (node->IsRootNode()) {
    DockNode *mark_node = node->CentralNode;
    while (mark_node) {
      mark_node->HasCentralNodeChild = true;
      mark_node = mark_node->ParentNode;
    }
  }
}

static void Gui::DockNodeUpdateVisibleFlag(DockNode *node) {
  // Update visibility flag
  bool is_visible =
      (node->ParentNode == NULL) ? node->IsDockSpace() : node->IsCentralNode();
  is_visible |= (node->Windows.Size > 0);
  is_visible |= (node->ChildNodes[0] && node->ChildNodes[0]->IsVisible);
  is_visible |= (node->ChildNodes[1] && node->ChildNodes[1]->IsVisible);
  node->IsVisible = is_visible;
}

static void Gui::DockNodeStartMouseMovingWindow(DockNode *node,
                                                Window *window) {
  Context &g = *GGui;
  ASSERT(node->WantMouseMove == true);
  StartMouseMovingWindow(window);
  g.ActiveIdClickOffset = g.IO.MouseClickedPos[0] - node->Pos;
  g.MovingWindow = window; // If we are docked into a non moveable root window,
                           // StartMouseMovingWindow() won't set g.MovingWindow.
                           // Override that decision.
  node->WantMouseMove = false;
}

// Update CentralNode, OnlyNodeWithWindows, LastFocusedNodeID. Copy window
// class.
static void Gui::DockNodeUpdateForRootNode(DockNode *node) {
  DockNodeUpdateFlagsAndCollapse(node);

  // - Setup central node pointers
  // - Find if there's only a single visible window in the hierarchy (in which
  // case we need to display a regular title bar -> FIXME-DOCK: that last part
  // is not done yet!) Cannot merge this with DockNodeUpdateFlagsAndCollapse()
  // because FirstNodeWithWindows is found after window removal and child
  // collapsing
  DockNodeTreeInfo info;
  DockNodeFindInfo(node, &info);
  node->CentralNode = info.CentralNode;
  node->OnlyNodeWithWindows =
      (info.CountNodesWithWindows == 1) ? info.FirstNodeWithWindows : NULL;
  node->CountNodeWithWindows = info.CountNodesWithWindows;
  if (node->LastFocusedNodeId == 0 && info.FirstNodeWithWindows != NULL)
    node->LastFocusedNodeId = info.FirstNodeWithWindows->ID;

  // Copy the window class from of our first window so it can be used for proper
  // dock filtering. When node has mixed windows, prioritize the class with the
  // most constraint (DockingAllowUnclassed = false) as the reference to copy.
  // FIXME-DOCK: We don't recurse properly, this code could be reworked to work
  // from DockNodeUpdateScanRec.
  if (DockNode *first_node_with_windows = info.FirstNodeWithWindows) {
    node->WindowClass = first_node_with_windows->Windows[0]->WindowClass;
    for (int n = 1; n < first_node_with_windows->Windows.Size; n++)
      if (first_node_with_windows->Windows[n]
              ->WindowClass.DockingAllowUnclassed == false) {
        node->WindowClass = first_node_with_windows->Windows[n]->WindowClass;
        break;
      }
  }

  DockNode *mark_node = node->CentralNode;
  while (mark_node) {
    mark_node->HasCentralNodeChild = true;
    mark_node = mark_node->ParentNode;
  }
}

static void DockNodeSetupHostWindow(DockNode *node, Window *host_window) {
  // Remove ourselves from any previous different host window
  // This can happen if a user mistakenly does (see #4295 for details):
  //  - N+0: DockBuilderAddNode(id, 0)    // missing
  //  DockNodeFlags_DockSpace
  //  - N+1: NewFrame()                   // will create floating host window
  //  for that node
  //  - N+1: DockSpace(id)                // requalify node as dockspace, moving
  //  host window
  if (node->HostWindow && node->HostWindow != host_window &&
      node->HostWindow->DockNodeAsHost == node)
    node->HostWindow->DockNodeAsHost = NULL;

  host_window->DockNodeAsHost = node;
  node->HostWindow = host_window;
}

static void Gui::DockNodeUpdate(DockNode *node) {
  Context &g = *GGui;
  ASSERT(node->LastFrameActive != g.FrameCount);
  node->LastFrameAlive = g.FrameCount;
  node->IsBgDrawnThisFrame = false;

  node->CentralNode = node->OnlyNodeWithWindows = NULL;
  if (node->IsRootNode())
    DockNodeUpdateForRootNode(node);

  // Remove tab bar if not needed
  if (node->TabBar && node->IsNoTabBar())
    DockNodeRemoveTabBar(node);

  // Early out for hidden root dock nodes (when all DockId references are in
  // inactive windows, or there is only 1 floating window holding on the DockId)
  bool want_to_hide_host_window = false;
  if (node->IsFloatingNode()) {
    if (node->Windows.Size <= 1 && node->IsLeafNode())
      if (!g.IO.ConfigDockingAlwaysTabBar &&
          (node->Windows.Size == 0 ||
           !node->Windows[0]->WindowClass.DockingAlwaysTabBar))
        want_to_hide_host_window = true;
    if (node->CountNodeWithWindows == 0)
      want_to_hide_host_window = true;
  }
  if (want_to_hide_host_window) {
    if (node->Windows.Size == 1) {
      // Floating window pos/size is authoritative
      Window *single_window = node->Windows[0];
      node->Pos = single_window->Pos;
      node->Size = single_window->SizeFull;
      node->AuthorityForPos = node->AuthorityForSize =
          node->AuthorityForViewport = DataAuthority_Window;

      // Transfer focus immediately so when we revert to a regular window it is
      // immediately selected
      if (node->HostWindow && g.NavWindow == node->HostWindow)
        FocusWindow(single_window);
      if (node->HostWindow) {
        DEBUG_LOG_VIEWPORT("[viewport] Node %08X transfer Viewport %08X->%08X "
                           "to Window '%s'\n",
                           node->ID, node->HostWindow->Viewport->ID,
                           single_window->ID, single_window->Name);
        single_window->Viewport = node->HostWindow->Viewport;
        single_window->ViewportId = node->HostWindow->ViewportId;
        if (node->HostWindow->ViewportOwned) {
          single_window->Viewport->ID = single_window->ID;
          single_window->Viewport->Window = single_window;
          single_window->ViewportOwned = true;
        }
      }
      node->RefViewportId = single_window->ViewportId;
    }

    DockNodeHideHostWindow(node);
    node->State = DockNodeState_HostWindowHiddenBecauseSingleWindow;
    node->WantCloseAll = false;
    node->WantCloseTabId = 0;
    node->HasCloseButton = node->HasWindowMenuButton = false;
    node->LastFrameActive = g.FrameCount;

    if (node->WantMouseMove && node->Windows.Size == 1)
      DockNodeStartMouseMovingWindow(node, node->Windows[0]);
    return;
  }

  // In some circumstance we will defer creating the host window (so everything
  // will be kept hidden), while the expected visible window is resizing itself.
  // This is important for first-time (no ini settings restored) single window
  // when io.ConfigDockingAlwaysTabBar is enabled, otherwise the node ends up
  // using the minimum window size. Effectively those windows will take an extra
  // frame to show up:
  //   N+0: Begin(): window created (with no known size), node is created
  //   N+1: DockNodeUpdate(): node skip creating host window / Begin(): window
  //   size applied, not visible N+2: DockNodeUpdate(): node can create host
  //   window / Begin(): window becomes visible
  // We could remove this frame if we could reliably calculate the expected
  // window size during node update, before the Begin() code. It would require a
  // generalization of CalcWindowExpectedSize(), probably extracting code away
  // from Begin(). In reality it isn't very important as user quickly ends up
  // with size data in .ini file.
  if (node->IsVisible && node->HostWindow == NULL && node->IsFloatingNode() &&
      node->IsLeafNode()) {
    ASSERT(node->Windows.Size > 0);
    Window *ref_window = NULL;
    if (node->SelectedTabId !=
        0) // Note that we prune single-window-node settings on .ini loading, so
           // this is generally 0 for them!
      ref_window = DockNodeFindWindowByID(node, node->SelectedTabId);
    if (ref_window == NULL)
      ref_window = node->Windows[0];
    if (ref_window->AutoFitFramesX > 0 || ref_window->AutoFitFramesY > 0) {
      node->State = DockNodeState_HostWindowHiddenBecauseWindowsAreResizing;
      return;
    }
  }

  const DockNodeFlags node_flags = node->MergedFlags;

  // Decide if the node will have a close button and a window menu button
  node->HasWindowMenuButton =
      (node->Windows.Size > 0) &&
      (node_flags & DockNodeFlags_NoWindowMenuButton) == 0;
  node->HasCloseButton = false;
  for (Window *window : node->Windows) {
    // FIXME-DOCK: Setting DockIsActive here means that for single active window
    // in a leaf node, DockIsActive will be cleared until the next Begin() call.
    node->HasCloseButton |= window->HasCloseButton;
    window->DockIsActive = (node->Windows.Size > 1);
  }
  if (node_flags & DockNodeFlags_NoCloseButton)
    node->HasCloseButton = false;

  // Bind or create host window
  Window *host_window = NULL;
  bool beginned_into_host_window = false;
  if (node->IsDockSpace()) {
    // [Explicit root dockspace node]
    ASSERT(node->HostWindow);
    host_window = node->HostWindow;
  } else {
    // [Automatic root or child nodes]
    if (node->IsRootNode() && node->IsVisible) {
      Window *ref_window = (node->Windows.Size > 0) ? node->Windows[0] : NULL;

      // Sync Pos
      if (node->AuthorityForPos == DataAuthority_Window && ref_window)
        SetNextWindowPos(ref_window->Pos);
      else if (node->AuthorityForPos == DataAuthority_DockNode)
        SetNextWindowPos(node->Pos);

      // Sync Size
      if (node->AuthorityForSize == DataAuthority_Window && ref_window)
        SetNextWindowSize(ref_window->SizeFull);
      else if (node->AuthorityForSize == DataAuthority_DockNode)
        SetNextWindowSize(node->Size);

      // Sync Collapsed
      if (node->AuthorityForSize == DataAuthority_Window && ref_window)
        SetNextWindowCollapsed(ref_window->Collapsed);

      // Sync Viewport
      if (node->AuthorityForViewport == DataAuthority_Window && ref_window)
        SetNextWindowViewport(ref_window->ViewportId);
      else if (node->AuthorityForViewport == DataAuthority_Window &&
               node->RefViewportId != 0)
        SetNextWindowViewport(node->RefViewportId);

      SetNextWindowClass(&node->WindowClass);

      // Begin into the host window
      char window_label[20];
      DockNodeGetHostWindowTitle(node, window_label, ARRAYSIZE(window_label));
      WindowFlags window_flags = WindowFlags_NoScrollbar |
                                 WindowFlags_NoScrollWithMouse |
                                 WindowFlags_DockNodeHost;
      window_flags |= WindowFlags_NoFocusOnAppearing;
      window_flags |= WindowFlags_NoSavedSettings | WindowFlags_NoNavFocus |
                      WindowFlags_NoCollapse;
      window_flags |= WindowFlags_NoTitleBar;

      SetNextWindowBgAlpha(0.0f); // Don't set WindowFlags_NoBackground
                                  // because it disables borders
      PushStyleVar(StyleVar_WindowPadding, Vec2(0, 0));
      Begin(window_label, NULL, window_flags);
      PopStyleVar();
      beginned_into_host_window = true;

      host_window = g.CurrentWindow;
      DockNodeSetupHostWindow(node, host_window);
      host_window->DC.CursorPos = host_window->Pos;
      node->Pos = host_window->Pos;
      node->Size = host_window->Size;

      // We set WindowFlags_NoFocusOnAppearing because we don't want the
      // host window to take full focus (e.g. steal NavWindow) But we still it
      // bring it to the front of display. There's no way to choose this precise
      // behavior via window flags. One simple case to ponder if: window A has a
      // toggle to create windows B/C/D. Dock B/C/D together, clear the toggle
      // and enable it again. When reappearing B/C/D will request focus and be
      // moved to the top of the display pile, but they are not linked to the
      // dock host window during the frame they appear. The dock host window
      // would keep its old display order, and the sorting in EndFrame would
      // move B/C/D back after the dock host window, losing their top-most
      // status.
      if (node->HostWindow->Appearing)
        BringWindowToDisplayFront(node->HostWindow);

      node->AuthorityForPos = node->AuthorityForSize =
          node->AuthorityForViewport = DataAuthority_Auto;
    } else if (node->ParentNode) {
      node->HostWindow = host_window = node->ParentNode->HostWindow;
      node->AuthorityForPos = node->AuthorityForSize =
          node->AuthorityForViewport = DataAuthority_Auto;
    }
    if (node->WantMouseMove && node->HostWindow)
      DockNodeStartMouseMovingWindow(node, node->HostWindow);
  }
  node->RefViewportId = 0; // Clear when we have a host window

  // Update focused node (the one whose title bar is highlight) within a node
  // tree
  if (node->IsSplitNode())
    ASSERT(node->TabBar == NULL);
  if (node->IsRootNode())
    if (Window *p_window = g.NavWindow ? g.NavWindow->RootWindow : NULL)
      while (p_window != NULL && p_window->DockNode != NULL) {
        DockNode *p_node = DockNodeGetRootNode(p_window->DockNode);
        if (p_node == node) {
          node->LastFocusedNodeId =
              p_window->DockNode->ID; // Note: not using root node ID!
          break;
        }
        p_window = p_node->HostWindow ? p_node->HostWindow->RootWindow : NULL;
      }

  // Register a hit-test hole in the window unless we are currently dragging a
  // window that is compatible with our dockspace
  DockNode *central_node = node->CentralNode;
  const bool central_node_hole =
      node->IsRootNode() && host_window &&
      (node_flags & DockNodeFlags_PassthruCentralNode) != 0 &&
      central_node != NULL && central_node->IsEmpty();
  bool central_node_hole_register_hit_test_hole = central_node_hole;
  if (central_node_hole)
    if (const Payload *payload = Gui::GetDragDropPayload())
      if (payload->IsDataType(PAYLOAD_TYPE_WINDOW) &&
          DockNodeIsDropAllowed(host_window, *(Window **)payload->Data))
        central_node_hole_register_hit_test_hole = false;
  if (central_node_hole_register_hit_test_hole) {
    // We add a little padding to match the "resize from edges" behavior and
    // allow grabbing the splitter easily. (But we only add it if there's
    // something else on the other side of the hole, otherwise for e.g.
    // fullscreen covering passthru node we'd have a gap on the edge not covered
    // by the hole)
    ASSERT(
        node->IsDockSpace()); // We cannot pass this flag without the
                              // DockSpace() api. Testing this because we also
                              // setup the hole in host_window->ParentNode
    DockNode *root_node = DockNodeGetRootNode(central_node);
    Rect root_rect(root_node->Pos, root_node->Pos + root_node->Size);
    Rect hole_rect(central_node->Pos, central_node->Pos + central_node->Size);
    if (hole_rect.Min.x > root_rect.Min.x) {
      hole_rect.Min.x += WINDOWS_HOVER_PADDING;
    }
    if (hole_rect.Max.x < root_rect.Max.x) {
      hole_rect.Max.x -= WINDOWS_HOVER_PADDING;
    }
    if (hole_rect.Min.y > root_rect.Min.y) {
      hole_rect.Min.y += WINDOWS_HOVER_PADDING;
    }
    if (hole_rect.Max.y < root_rect.Max.y) {
      hole_rect.Max.y -= WINDOWS_HOVER_PADDING;
    }
    // GetForegroundDrawList()->AddRect(hole_rect.Min, hole_rect.Max,
    // COL32(255, 0, 0, 255));
    if (central_node_hole && !hole_rect.IsInverted()) {
      SetWindowHitTestHole(host_window, hole_rect.Min,
                           hole_rect.Max - hole_rect.Min);
      if (host_window->ParentWindow)
        SetWindowHitTestHole(host_window->ParentWindow, hole_rect.Min,
                             hole_rect.Max - hole_rect.Min);
    }
  }

  // Update position/size, process and draw resizing splitters
  if (node->IsRootNode() && host_window) {
    DockNodeTreeUpdatePosSize(node, host_window->Pos, host_window->Size);
    PushStyleColor(Col_Separator, g.Style.Colors[Col_Border]);
    PushStyleColor(Col_SeparatorActive, g.Style.Colors[Col_ResizeGripActive]);
    PushStyleColor(Col_SeparatorHovered, g.Style.Colors[Col_ResizeGripHovered]);
    DockNodeTreeUpdateSplitter(node);
    PopStyleColor(3);
  }

  // Draw empty node background (currently can only be the Central Node)
  if (host_window && node->IsEmpty() && node->IsVisible) {
    host_window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
    node->LastBgColor = (node_flags & DockNodeFlags_PassthruCentralNode)
                            ? 0
                            : GetColorU32(Col_DockingEmptyBg);
    if (node->LastBgColor != 0)
      host_window->DrawList->AddRectFilled(node->Pos, node->Pos + node->Size,
                                           node->LastBgColor);
    node->IsBgDrawnThisFrame = true;
  }

  // Draw whole dockspace background if DockNodeFlags_PassthruCentralNode
  // if set. We need to draw a background at the root level if requested by
  // DockNodeFlags_PassthruCentralNode, but we will only know the correct
  // pos/size _after_ processing the resizing splitters. So we are using the
  // DrawList channel splitting facility to submit drawing primitives out of
  // order!
  const bool render_dockspace_bg =
      node->IsRootNode() && host_window &&
      (node_flags & DockNodeFlags_PassthruCentralNode) != 0;
  if (render_dockspace_bg && node->IsVisible) {
    host_window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_BG);
    if (central_node_hole)
      RenderRectFilledWithHole(host_window->DrawList, node->Rect(),
                               central_node->Rect(), GetColorU32(Col_WindowBg),
                               0.0f);
    else
      host_window->DrawList->AddRectFilled(node->Pos, node->Pos + node->Size,
                                           GetColorU32(Col_WindowBg), 0.0f);
  }

  // Draw and populate Tab Bar
  if (host_window)
    host_window->DrawList->ChannelsSetCurrent(DOCKING_HOST_DRAW_CHANNEL_FG);
  if (host_window && node->Windows.Size > 0) {
    DockNodeUpdateTabBar(node, host_window);
  } else {
    node->WantCloseAll = false;
    node->WantCloseTabId = 0;
    node->IsFocused = false;
  }
  if (node->TabBar && node->TabBar->SelectedTabId)
    node->SelectedTabId = node->TabBar->SelectedTabId;
  else if (node->Windows.Size > 0)
    node->SelectedTabId = node->Windows[0]->TabId;

  // Draw payload drop target
  if (host_window && node->IsVisible)
    if (node->IsRootNode() &&
        (g.MovingWindow == NULL ||
         g.MovingWindow->RootWindowDockTree != host_window))
      BeginDockableDragDropTarget(host_window);

  // We update this after DockNodeUpdateTabBar()
  node->LastFrameActive = g.FrameCount;

  // Recurse into children
  // FIXME-DOCK FIXME-OPT: Should not need to recurse into children
  if (host_window) {
    if (node->ChildNodes[0])
      DockNodeUpdate(node->ChildNodes[0]);
    if (node->ChildNodes[1])
      DockNodeUpdate(node->ChildNodes[1]);

    // Render outer borders last (after the tab bar)
    if (node->IsRootNode())
      RenderWindowOuterBorders(host_window);
  }

  // End host window
  if (beginned_into_host_window) //-V1020
    End();
}

// Compare TabItem nodes given the last known DockOrder (will persist in .ini
// file as hint), used to sort tabs when multiple tabs are added on the same
// frame.
static int CDECL TabItemComparerByDockOrder(const void *lhs, const void *rhs) {
  Window *a = ((const TabItem *)lhs)->Window;
  Window *b = ((const TabItem *)rhs)->Window;
  if (int d = ((a->DockOrder == -1) ? INT_MAX : a->DockOrder) -
              ((b->DockOrder == -1) ? INT_MAX : b->DockOrder))
    return d;
  return (a->BeginOrderWithinContext - b->BeginOrderWithinContext);
}

// Default handler for g.DockNodeWindowMenuHandler(): display the list of
// windows for a given dock-node. This is exceptionally stored in a function
// pointer to also user applications to tweak this menu (undocumented) Custom
// overrides may want to decorate, group, sort entries. Please note those are
// internal structures: if you copy this expect occasional breakage. (if you
// don't need to modify the "Tabs.Size == 1" behavior/path it is recommend you
// call this function in your handler)
void Gui::DockNodeWindowMenuHandler_Default(Context *ctx, DockNode *node,
                                            TabBar *tab_bar) {
  UNUSED(ctx);
  if (tab_bar->Tabs.Size == 1) {
    // "Hide tab bar" option. Being one of our rare user-facing string we pull
    // it from a table.
    if (MenuItem(LocalizeGetMsg(LocKey_DockingHideTabBar), NULL,
                 node->IsHiddenTabBar()))
      node->WantHiddenTabBarToggle = true;
  } else {
    // Display a selectable list of windows in this docking node
    for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++) {
      TabItem *tab = &tab_bar->Tabs[tab_n];
      if (tab->Flags & TabItemFlags_Button)
        continue;
      if (Selectable(TabBarGetTabName(tab_bar, tab),
                     tab->ID == tab_bar->SelectedTabId))
        TabBarQueueFocus(tab_bar, tab);
      SameLine();
      Text("   ");
    }
  }
}

static void Gui::DockNodeWindowMenuUpdate(DockNode *node, TabBar *tab_bar) {
  // Try to position the menu so it is more likely to stays within the same
  // viewport
  Context &g = *GGui;
  if (g.Style.WindowMenuButtonPosition == Dir_Left)
    SetNextWindowPos(Vec2(node->Pos.x, node->Pos.y + GetFrameHeight()),
                     Cond_Always, Vec2(0.0f, 0.0f));
  else
    SetNextWindowPos(
        Vec2(node->Pos.x + node->Size.x, node->Pos.y + GetFrameHeight()),
        Cond_Always, Vec2(1.0f, 0.0f));
  if (BeginPopup("#WindowMenu")) {
    node->IsFocused = true;
    g.DockNodeWindowMenuHandler(&g, node, tab_bar);
    EndPopup();
  }
}

// User helper to append/amend into a dock node tab bar. Most commonly used to
// add e.g. a "+" button.
bool Gui::DockNodeBeginAmendTabBar(DockNode *node) {
  if (node->TabBar == NULL || node->HostWindow == NULL)
    return false;
  if (node->MergedFlags & DockNodeFlags_KeepAliveOnly)
    return false;
  if (node->TabBar->ID == 0)
    return false;
  Begin(node->HostWindow->Name);
  PushOverrideID(node->ID);
  bool ret =
      BeginTabBarEx(node->TabBar, node->TabBar->BarRect, node->TabBar->Flags);
  UNUSED(ret);
  ASSERT(ret);
  return true;
}

void Gui::DockNodeEndAmendTabBar() {
  EndTabBar();
  PopID();
  End();
}

static bool IsDockNodeTitleBarHighlighted(DockNode *node, DockNode *root_node) {
  // CTRL+Tab highlight (only highlighting leaf node, not whole hierarchy)
  Context &g = *GGui;
  if (g.NavWindowingTarget)
    return (g.NavWindowingTarget->DockNode == node);

  // FIXME-DOCKING: May want alternative to treat central node void differently?
  // e.g. if (g.NavWindow == host_window)
  if (g.NavWindow && root_node->LastFocusedNodeId == node->ID) {
    // FIXME: This could all be backed in RootWindowForTitleBarHighlight?
    // Probably need to reorganize for both dock nodes + other
    // RootWindowForTitleBarHighlight users (not-node)
    Window *parent_window = g.NavWindow->RootWindow;
    while (parent_window->Flags & WindowFlags_ChildMenu)
      parent_window = parent_window->ParentWindow->RootWindow;
    DockNode *start_parent_node = parent_window->DockNodeAsHost
                                      ? parent_window->DockNodeAsHost
                                      : parent_window->DockNode;
    for (DockNode *parent_node = start_parent_node; parent_node != NULL;
         parent_node = parent_node->HostWindow
                           ? parent_node->HostWindow->RootWindow->DockNode
                           : NULL)
      if ((parent_node = Gui::DockNodeGetRootNode(parent_node)) == root_node)
        return true;
  }
  return false;
}

// Submit the tab bar corresponding to a dock node and various housekeeping
// details.
static void Gui::DockNodeUpdateTabBar(DockNode *node, Window *host_window) {
  Context &g = *GGui;
  Style &style = g.Style;

  const bool node_was_active = (node->LastFrameActive + 1 == g.FrameCount);
  const bool closed_all = node->WantCloseAll && node_was_active;
  const ID closed_one = node->WantCloseTabId && node_was_active;
  node->WantCloseAll = false;
  node->WantCloseTabId = 0;

  // Decide if we should use a focused title bar color
  bool is_focused = false;
  DockNode *root_node = DockNodeGetRootNode(node);
  if (IsDockNodeTitleBarHighlighted(node, root_node))
    is_focused = true;

  // Hidden tab bar will show a triangle on the upper-left (in Begin)
  if (node->IsHiddenTabBar() || node->IsNoTabBar()) {
    node->VisibleWindow = (node->Windows.Size > 0) ? node->Windows[0] : NULL;
    node->IsFocused = is_focused;
    if (is_focused)
      node->LastFrameFocused = g.FrameCount;
    if (node->VisibleWindow) {
      // Notify root of visible window (used to display title in OS task bar)
      if (is_focused || root_node->VisibleWindow == NULL)
        root_node->VisibleWindow = node->VisibleWindow;
      if (node->TabBar)
        node->TabBar->VisibleTabId = node->VisibleWindow->TabId;
    }
    return;
  }

  // Move ourselves to the Menu layer (so we can be accessed by tapping Alt) +
  // undo SkipItems flag in order to draw over the title bar even if the window
  // is collapsed
  bool backup_skip_item = host_window->SkipItems;
  if (!node->IsDockSpace()) {
    host_window->SkipItems = false;
    host_window->DC.NavLayerCurrent = NavLayer_Menu;
  }

  // Use PushOverrideID() instead of PushID() to use the node id _without_ the
  // host window ID. This is to facilitate computing those ID from the outside,
  // and will affect more or less only the ID of the collapse button, popup and
  // tabs, as docked windows themselves will override the stack with their own
  // root ID.
  PushOverrideID(node->ID);
  TabBar *tab_bar = node->TabBar;
  bool tab_bar_is_recreated =
      (tab_bar ==
       NULL); // Tab bar are automatically destroyed when a node gets hidden
  if (tab_bar == NULL) {
    DockNodeAddTabBar(node);
    tab_bar = node->TabBar;
  }

  ID focus_tab_id = 0;
  node->IsFocused = is_focused;

  const DockNodeFlags node_flags = node->MergedFlags;
  const bool has_window_menu_button =
      (node_flags & DockNodeFlags_NoWindowMenuButton) == 0 &&
      (style.WindowMenuButtonPosition != Dir_None);

  // In a dock node, the Collapse Button turns into the Window Menu button.
  // FIXME-DOCK FIXME-OPT: Could we recycle popups id across multiple dock
  // nodes?
  if (has_window_menu_button && IsPopupOpen("#WindowMenu")) {
    ID next_selected_tab_id = tab_bar->NextSelectedTabId;
    DockNodeWindowMenuUpdate(node, tab_bar);
    if (tab_bar->NextSelectedTabId != 0 &&
        tab_bar->NextSelectedTabId != next_selected_tab_id)
      focus_tab_id = tab_bar->NextSelectedTabId;
    is_focused |= node->IsFocused;
  }

  // Layout
  Rect title_bar_rect, tab_bar_rect;
  Vec2 window_menu_button_pos;
  Vec2 close_button_pos;
  DockNodeCalcTabBarLayout(node, &title_bar_rect, &tab_bar_rect,
                           &window_menu_button_pos, &close_button_pos);

  // Submit new tabs, they will be added as Unsorted and sorted below based on
  // relative DockOrder value.
  const int tabs_count_old = tab_bar->Tabs.Size;
  for (int window_n = 0; window_n < node->Windows.Size; window_n++) {
    Window *window = node->Windows[window_n];
    if (TabBarFindTabByID(tab_bar, window->TabId) == NULL)
      TabBarAddTab(tab_bar, TabItemFlags_Unsorted, window);
  }

  // Title bar
  if (is_focused)
    node->LastFrameFocused = g.FrameCount;
  U32 title_bar_col = GetColorU32(host_window->Collapsed ? Col_TitleBgCollapsed
                                  : is_focused           ? Col_TitleBgActive
                                                         : Col_TitleBg);
  DrawFlags rounding_flags = CalcRoundingFlagsForRectInRect(
      title_bar_rect, host_window->Rect(), g.Style.DockingSeparatorSize);
  host_window->DrawList->AddRectFilled(
      title_bar_rect.Min, title_bar_rect.Max, title_bar_col,
      host_window->WindowRounding, rounding_flags);

  // Docking/Collapse button
  if (has_window_menu_button) {
    if (CollapseButton(host_window->GetID("#COLLAPSE"), window_menu_button_pos,
                       node)) // == DockNodeGetWindowMenuButtonId(node)
      OpenPopup("#WindowMenu");
    if (IsItemActive())
      focus_tab_id = tab_bar->SelectedTabId;
    if (IsItemHovered(HoveredFlags_ForTooltip | HoveredFlags_DelayNormal) &&
        g.HoveredIdTimer > 0.5f)
      SetTooltip("%s", LocalizeGetMsg(LocKey_DockingDragToUndockOrMoveNode));
  }

  // If multiple tabs are appearing on the same frame, sort them based on their
  // persistent DockOrder value
  int tabs_unsorted_start = tab_bar->Tabs.Size;
  for (int tab_n = tab_bar->Tabs.Size - 1;
       tab_n >= 0 && (tab_bar->Tabs[tab_n].Flags & TabItemFlags_Unsorted);
       tab_n--) {
    // FIXME-DOCK: Consider only clearing the flag after the tab has been alive
    // for a few consecutive frames, allowing late comers to not break sorting?
    tab_bar->Tabs[tab_n].Flags &= ~TabItemFlags_Unsorted;
    tabs_unsorted_start = tab_n;
  }
  if (tab_bar->Tabs.Size > tabs_unsorted_start) {
    DEBUG_LOG_DOCKING(
        "[docking] In node 0x%08X: %d new appearing tabs:%s\n", node->ID,
        tab_bar->Tabs.Size - tabs_unsorted_start,
        (tab_bar->Tabs.Size > tabs_unsorted_start + 1) ? " (will sort)" : "");
    for (int tab_n = tabs_unsorted_start; tab_n < tab_bar->Tabs.Size; tab_n++) {
      TabItem *tab = &tab_bar->Tabs[tab_n];
      DEBUG_LOG_DOCKING("[docking] - Tab 0x%08X '%s' Order %d\n", tab->ID,
                        TabBarGetTabName(tab_bar, tab),
                        tab->Window ? tab->Window->DockOrder : -1);
    }
    DEBUG_LOG_DOCKING(
        "[docking] SelectedTabId = 0x%08X, NavWindow->TabId = 0x%08X\n",
        node->SelectedTabId, g.NavWindow ? g.NavWindow->TabId : -1);
    if (tab_bar->Tabs.Size > tabs_unsorted_start + 1)
      Qsort(tab_bar->Tabs.Data + tabs_unsorted_start,
            tab_bar->Tabs.Size - tabs_unsorted_start, sizeof(TabItem),
            TabItemComparerByDockOrder);
  }

  // Apply NavWindow focus back to the tab bar
  if (g.NavWindow && g.NavWindow->RootWindow->DockNode == node)
    tab_bar->SelectedTabId = g.NavWindow->RootWindow->TabId;

  // Selected newly added tabs, or persistent tab ID if the tab bar was just
  // recreated
  if (tab_bar_is_recreated &&
      TabBarFindTabByID(tab_bar, node->SelectedTabId) != NULL)
    tab_bar->SelectedTabId = tab_bar->NextSelectedTabId = node->SelectedTabId;
  else if (tab_bar->Tabs.Size > tabs_count_old)
    tab_bar->SelectedTabId = tab_bar->NextSelectedTabId =
        tab_bar->Tabs.back().Window->TabId;

  // Begin tab bar
  TabBarFlags tab_bar_flags =
      TabBarFlags_Reorderable |
      TabBarFlags_AutoSelectNewTabs; // |
                                     // TabBarFlags_NoTabListScrollingButtons);
  tab_bar_flags |= TabBarFlags_SaveSettings |
                   TabBarFlags_DockNode; // | TabBarFlags_FittingPolicyScroll;
  if (!host_window->Collapsed && is_focused)
    tab_bar_flags |= TabBarFlags_IsFocused;
  tab_bar->ID = GetID("#TabBar");
  tab_bar->SeparatorMinX =
      node->Pos.x +
      host_window->WindowBorderSize; // Separator cover the whole node width
  tab_bar->SeparatorMaxX =
      node->Pos.x + node->Size.x - host_window->WindowBorderSize;
  BeginTabBarEx(tab_bar, tab_bar_rect, tab_bar_flags);
  // host_window->DrawList->AddRect(tab_bar_rect.Min, tab_bar_rect.Max,
  // COL32(255,0,255,255));

  // Backup style colors
  Vec4 backup_style_cols[WindowDockStyleCol_COUNT];
  for (int color_n = 0; color_n < WindowDockStyleCol_COUNT; color_n++)
    backup_style_cols[color_n] =
        g.Style.Colors[GWindowDockStyleColors[color_n]];

  // Submit actual tabs
  node->VisibleWindow = NULL;
  for (int window_n = 0; window_n < node->Windows.Size; window_n++) {
    Window *window = node->Windows[window_n];
    if ((closed_all || closed_one == window->TabId) && window->HasCloseButton &&
        !(window->Flags & WindowFlags_UnsavedDocument))
      continue;
    if (window->LastFrameActive + 1 >= g.FrameCount || !node_was_active) {
      TabItemFlags tab_item_flags = 0;
      tab_item_flags |= window->WindowClass.TabItemFlagsOverrideSet;
      if (window->Flags & WindowFlags_UnsavedDocument)
        tab_item_flags |= TabItemFlags_UnsavedDocument;
      if (tab_bar->Flags & TabBarFlags_NoCloseWithMiddleMouseButton)
        tab_item_flags |= TabItemFlags_NoCloseWithMiddleMouseButton;

      // Apply stored style overrides for the window
      for (int color_n = 0; color_n < WindowDockStyleCol_COUNT; color_n++)
        g.Style.Colors[GWindowDockStyleColors[color_n]] =
            ColorConvertU32ToFloat4(window->DockStyle.Colors[color_n]);

      // Note that TabItemEx() calls TabBarCalcTabID() so our tab item ID will
      // ignore the current ID stack (rightly so)
      bool tab_open = true;
      TabItemEx(tab_bar, window->Name,
                window->HasCloseButton ? &tab_open : NULL, tab_item_flags,
                window);
      if (!tab_open)
        node->WantCloseTabId = window->TabId;
      if (tab_bar->VisibleTabId == window->TabId)
        node->VisibleWindow = window;

      // Store last item data so it can be queried with IsItemXXX functions
      // after the user Begin() call
      window->DockTabItemStatusFlags = g.LastItemData.StatusFlags;
      window->DockTabItemRect = g.LastItemData.Rect;

      // Update navigation ID on menu layer
      if (g.NavWindow && g.NavWindow->RootWindow == window &&
          (window->DC.NavLayersActiveMask & (1 << NavLayer_Menu)) == 0)
        host_window->NavLastIds[1] = window->TabId;
    }
  }

  // Restore style colors
  for (int color_n = 0; color_n < WindowDockStyleCol_COUNT; color_n++)
    g.Style.Colors[GWindowDockStyleColors[color_n]] =
        backup_style_cols[color_n];

  // Notify root of visible window (used to display title in OS task bar)
  if (node->VisibleWindow)
    if (is_focused || root_node->VisibleWindow == NULL)
      root_node->VisibleWindow = node->VisibleWindow;

  // Close button (after VisibleWindow was updated)
  // Note that VisibleWindow may have been overrided by CTRL+Tabbing, so
  // VisibleWindow->TabId may be != from tab_bar->SelectedTabId
  const bool close_button_is_enabled = node->HasCloseButton &&
                                       node->VisibleWindow &&
                                       node->VisibleWindow->HasCloseButton;
  const bool close_button_is_visible = node->HasCloseButton;
  // const bool close_button_is_visible = close_button_is_enabled; // Most
  // people would expect this behavior of not even showing the button (leaving a
  // hole since we can't claim that space as other windows in the tba bar have
  // one)
  if (close_button_is_visible) {
    if (!close_button_is_enabled) {
      PushItemFlag(ItemFlags_Disabled, true);
      PushStyleColor(Col_Text,
                     style.Colors[Col_Text] * Vec4(1.0f, 1.0f, 1.0f, 0.4f));
    }
    if (CloseButton(host_window->GetID("#CLOSE"), close_button_pos)) {
      node->WantCloseAll = true;
      for (int n = 0; n < tab_bar->Tabs.Size; n++)
        TabBarCloseTab(tab_bar, &tab_bar->Tabs[n]);
    }
    // if (IsItemActive())
    //     focus_tab_id = tab_bar->SelectedTabId;
    if (!close_button_is_enabled) {
      PopStyleColor();
      PopItemFlag();
    }
  }

  // When clicking on the title bar outside of tabs, we still focus the selected
  // tab for that node
  // FIXME: TabItems submitted earlier use AllowItemOverlap so we manually
  // perform a more specific test for now (hovered || held) in order to not
  // cover them.
  ID title_bar_id = host_window->GetID("#TITLEBAR");
  if (g.HoveredId == 0 || g.HoveredId == title_bar_id ||
      g.ActiveId == title_bar_id) {
    // AllowOverlap mode required for appending into dock node tab bar,
    // otherwise dragging window will steal HoveredId and amended tabs cannot
    // get them.
    bool held;
    KeepAliveID(title_bar_id);
    ButtonBehavior(title_bar_rect, title_bar_id, NULL, &held,
                   ButtonFlags_AllowOverlap);
    if (g.HoveredId == title_bar_id) {
      g.LastItemData.ID = title_bar_id;
    }
    if (held) {
      if (IsMouseClicked(0))
        focus_tab_id = tab_bar->SelectedTabId;

      // Forward moving request to selected window
      if (TabItem *tab = TabBarFindTabByID(tab_bar, tab_bar->SelectedTabId))
        StartMouseMovingWindowOrNode(
            tab->Window ? tab->Window : node->HostWindow, node,
            false); // Undock from tab bar empty space
    }
  }

  // Forward focus from host node to selected window
  // if (is_focused && g.NavWindow == host_window && !g.NavWindowingTarget)
  //    focus_tab_id = tab_bar->SelectedTabId;

  // When clicked on a tab we requested focus to the docked child
  // This overrides the value set by "forward focus from host node to selected
  // window".
  if (tab_bar->NextSelectedTabId)
    focus_tab_id = tab_bar->NextSelectedTabId;

  // Apply navigation focus
  if (focus_tab_id != 0)
    if (TabItem *tab = TabBarFindTabByID(tab_bar, focus_tab_id))
      if (tab->Window) {
        FocusWindow(tab->Window);
        NavInitWindow(tab->Window, false);
      }

  EndTabBar();
  PopID();

  // Restore SkipItems flag
  if (!node->IsDockSpace()) {
    host_window->DC.NavLayerCurrent = NavLayer_Main;
    host_window->SkipItems = backup_skip_item;
  }
}

static void Gui::DockNodeAddTabBar(DockNode *node) {
  ASSERT(node->TabBar == NULL);
  node->TabBar = NEW(TabBar);
}

static void Gui::DockNodeRemoveTabBar(DockNode *node) {
  if (node->TabBar == NULL)
    return;
  DELETE(node->TabBar);
  node->TabBar = NULL;
}

static bool DockNodeIsDropAllowedOne(Window *payload, Window *host_window) {
  if (host_window->DockNodeAsHost &&
      host_window->DockNodeAsHost->IsDockSpace() &&
      payload->BeginOrderWithinContext < host_window->BeginOrderWithinContext)
    return false;

  WindowClass *host_class = host_window->DockNodeAsHost
                                ? &host_window->DockNodeAsHost->WindowClass
                                : &host_window->WindowClass;
  WindowClass *payload_class = &payload->WindowClass;
  if (host_class->ClassId != payload_class->ClassId) {
    bool pass = false;
    if (host_class->ClassId != 0 && host_class->DockingAllowUnclassed &&
        payload_class->ClassId == 0)
      pass = true;
    if (payload_class->ClassId != 0 && payload_class->DockingAllowUnclassed &&
        host_class->ClassId == 0)
      pass = true;
    if (!pass)
      return false;
  }

  // Prevent docking any window created above a popup
  // Technically we should support it (e.g. in the case of a long-lived modal
  // window that had fancy docking features), by e.g. adding a 'if
  // (!Gui::IsWindowWithinBeginStackOf(host_window, popup_window))' test. But
  // it would requires more work on our end because the dock host windows is
  // technically created in NewFrame() and our ->ParentXXX and ->RootXXX
  // pointers inside windows are currently mislading or lacking.
  Context &g = *GGui;
  for (int i = g.OpenPopupStack.Size - 1; i >= 0; i--)
    if (Window *popup_window = g.OpenPopupStack[i].Window)
      if (Gui::IsWindowWithinBeginStackOf(
              payload, popup_window)) // Payload is created from within a popup
                                      // begin stack.
        return false;

  return true;
}

static bool Gui::DockNodeIsDropAllowed(Window *host_window,
                                       Window *root_payload) {
  if (root_payload->DockNodeAsHost &&
      root_payload->DockNodeAsHost
          ->IsSplitNode()) // FIXME-DOCK: Missing filtering
    return true;

  const int payload_count = root_payload->DockNodeAsHost
                                ? root_payload->DockNodeAsHost->Windows.Size
                                : 1;
  for (int payload_n = 0; payload_n < payload_count; payload_n++) {
    Window *payload = root_payload->DockNodeAsHost
                          ? root_payload->DockNodeAsHost->Windows[payload_n]
                          : root_payload;
    if (DockNodeIsDropAllowedOne(payload, host_window))
      return true;
  }
  return false;
}

// window menu button == collapse button when not in a dock node.
// FIXME: This is similar to RenderWindowTitleBarContents(), may want to share
// code.
static void Gui::DockNodeCalcTabBarLayout(const DockNode *node,
                                          Rect *out_title_rect,
                                          Rect *out_tab_bar_rect,
                                          Vec2 *out_window_menu_button_pos,
                                          Vec2 *out_close_button_pos) {
  Context &g = *GGui;
  Style &style = g.Style;

  Rect r = Rect(node->Pos.x, node->Pos.y, node->Pos.x + node->Size.x,
                node->Pos.y + g.FontSize + g.Style.FramePadding.y * 2.0f);
  if (out_title_rect) {
    *out_title_rect = r;
  }

  r.Min.x += style.WindowBorderSize;
  r.Max.x -= style.WindowBorderSize;

  float button_sz = g.FontSize;
  r.Min.x += style.FramePadding.x;
  r.Max.x -= style.FramePadding.x;
  Vec2 window_menu_button_pos = Vec2(r.Min.x, r.Min.y + style.FramePadding.y);
  if (node->HasCloseButton) {
    if (out_close_button_pos)
      *out_close_button_pos =
          Vec2(r.Max.x - button_sz, r.Min.y + style.FramePadding.y);
    r.Max.x -= button_sz + style.ItemInnerSpacing.x;
  }
  if (node->HasWindowMenuButton && style.WindowMenuButtonPosition == Dir_Left) {
    r.Min.x += button_sz + style.ItemInnerSpacing.x;
  } else if (node->HasWindowMenuButton &&
             style.WindowMenuButtonPosition == Dir_Right) {
    window_menu_button_pos =
        Vec2(r.Max.x - button_sz, r.Min.y + style.FramePadding.y);
    r.Max.x -= button_sz + style.ItemInnerSpacing.x;
  }
  if (out_tab_bar_rect) {
    *out_tab_bar_rect = r;
  }
  if (out_window_menu_button_pos) {
    *out_window_menu_button_pos = window_menu_button_pos;
  }
}

void Gui::DockNodeCalcSplitRects(Vec2 &pos_old, Vec2 &size_old, Vec2 &pos_new,
                                 Vec2 &size_new, Dir dir,
                                 Vec2 size_new_desired) {
  Context &g = *GGui;
  const float dock_spacing = g.Style.ItemInnerSpacing.x;
  const Axis axis = (dir == Dir_Left || dir == Dir_Right) ? Axis_X : Axis_Y;
  pos_new[axis ^ 1] = pos_old[axis ^ 1];
  size_new[axis ^ 1] = size_old[axis ^ 1];

  // Distribute size on given axis (with a desired size or equally)
  const float w_avail = size_old[axis] - dock_spacing;
  if (size_new_desired[axis] > 0.0f &&
      size_new_desired[axis] <= w_avail * 0.5f) {
    size_new[axis] = size_new_desired[axis];
    size_old[axis] = TRUNC(w_avail - size_new[axis]);
  } else {
    size_new[axis] = TRUNC(w_avail * 0.5f);
    size_old[axis] = TRUNC(w_avail - size_new[axis]);
  }

  // Position each node
  if (dir == Dir_Right || dir == Dir_Down) {
    pos_new[axis] = pos_old[axis] + size_old[axis] + dock_spacing;
  } else if (dir == Dir_Left || dir == Dir_Up) {
    pos_new[axis] = pos_old[axis];
    pos_old[axis] = pos_new[axis] + size_new[axis] + dock_spacing;
  }
}

// Retrieve the drop rectangles for a given direction or for the center +
// perform hit testing.
bool Gui::DockNodeCalcDropRectsAndTestMousePos(const Rect &parent, Dir dir,
                                               Rect &out_r, bool outer_docking,
                                               Vec2 *test_mouse_pos) {
  Context &g = *GGui;

  const float parent_smaller_axis = Min(parent.GetWidth(), parent.GetHeight());
  const float hs_for_central_nodes = Min(
      g.FontSize * 1.5f, Max(g.FontSize * 0.5f, parent_smaller_axis / 8.0f));
  float hs_w; // Half-size, longer axis
  float hs_h; // Half-size, smaller axis
  Vec2 off;   // Distance from edge or center
  if (outer_docking) {
    // hs_w = Trunc(Clamp(parent_smaller_axis - hs_for_central_nodes * 4.0f,
    // g.FontSize * 0.5f, g.FontSize * 8.0f)); hs_h = Trunc(hs_w * 0.15f); off
    // = Vec2(Trunc(parent.GetWidth() * 0.5f - GetFrameHeightWithSpacing()
    // * 1.4f - hs_h), Trunc(parent.GetHeight() * 0.5f -
    // GetFrameHeightWithSpacing() * 1.4f - hs_h));
    hs_w = Trunc(hs_for_central_nodes * 1.50f);
    hs_h = Trunc(hs_for_central_nodes * 0.80f);
    off = Trunc(Vec2(parent.GetWidth() * 0.5f - hs_h,
                     parent.GetHeight() * 0.5f - hs_h));
  } else {
    hs_w = Trunc(hs_for_central_nodes);
    hs_h = Trunc(hs_for_central_nodes * 0.90f);
    off = Trunc(Vec2(hs_w * 2.40f, hs_w * 2.40f));
  }

  Vec2 c = Trunc(parent.GetCenter());
  if (dir == Dir_None) {
    out_r = Rect(c.x - hs_w, c.y - hs_w, c.x + hs_w, c.y + hs_w);
  } else if (dir == Dir_Up) {
    out_r =
        Rect(c.x - hs_w, c.y - off.y - hs_h, c.x + hs_w, c.y - off.y + hs_h);
  } else if (dir == Dir_Down) {
    out_r =
        Rect(c.x - hs_w, c.y + off.y - hs_h, c.x + hs_w, c.y + off.y + hs_h);
  } else if (dir == Dir_Left) {
    out_r =
        Rect(c.x - off.x - hs_h, c.y - hs_w, c.x - off.x + hs_h, c.y + hs_w);
  } else if (dir == Dir_Right) {
    out_r =
        Rect(c.x + off.x - hs_h, c.y - hs_w, c.x + off.x + hs_h, c.y + hs_w);
  }

  if (test_mouse_pos == NULL)
    return false;

  Rect hit_r = out_r;
  if (!outer_docking) {
    // Custom hit testing for the 5-way selection, designed to reduce flickering
    // when moving diagonally between sides
    hit_r.Expand(Trunc(hs_w * 0.30f));
    Vec2 mouse_delta = (*test_mouse_pos - c);
    float mouse_delta_len2 = LengthSqr(mouse_delta);
    float r_threshold_center = hs_w * 1.4f;
    float r_threshold_sides = hs_w * (1.4f + 1.2f);
    if (mouse_delta_len2 < r_threshold_center * r_threshold_center)
      return (dir == Dir_None);
    if (mouse_delta_len2 < r_threshold_sides * r_threshold_sides)
      return (dir == GetDirQuadrantFromDelta(mouse_delta.x, mouse_delta.y));
  }
  return hit_r.Contains(*test_mouse_pos);
}

// host_node may be NULL if the window doesn't have a DockNode already.
// FIXME-DOCK: This is misnamed since it's also doing the filtering.
static void
Gui::DockNodePreviewDockSetup(Window *host_window, DockNode *host_node,
                              Window *payload_window, DockNode *payload_node,
                              DockPreviewData *data, bool is_explicit_target,
                              bool is_outer_docking) {
  Context &g = *GGui;

  // There is an edge case when docking into a dockspace which only has inactive
  // nodes. In this case DockNodeTreeFindNodeByPos() will have selected a leaf
  // node which is inactive. Because the inactive leaf node doesn't have proper
  // pos/size yet, we'll use the root node as reference.
  if (payload_node == NULL)
    payload_node = payload_window->DockNodeAsHost;
  DockNode *ref_node_for_rect = (host_node && !host_node->IsVisible)
                                    ? DockNodeGetRootNode(host_node)
                                    : host_node;
  if (ref_node_for_rect)
    ASSERT(ref_node_for_rect->IsVisible == true);

  // Filter, figure out where we are allowed to dock
  DockNodeFlags src_node_flags =
      payload_node ? payload_node->MergedFlags
                   : payload_window->WindowClass.DockNodeFlagsOverrideSet;
  DockNodeFlags dst_node_flags =
      host_node ? host_node->MergedFlags
                : host_window->WindowClass.DockNodeFlagsOverrideSet;
  data->IsCenterAvailable = true;
  if (is_outer_docking)
    data->IsCenterAvailable = false;
  else if (dst_node_flags & DockNodeFlags_NoDockingOverMe)
    data->IsCenterAvailable = false;
  else if (host_node &&
           (dst_node_flags & DockNodeFlags_NoDockingOverCentralNode) &&
           host_node->IsCentralNode())
    data->IsCenterAvailable = false;
  else if ((!host_node || !host_node->IsEmpty()) && payload_node &&
           payload_node->IsSplitNode() &&
           (payload_node->OnlyNodeWithWindows == NULL)) // Is _visibly_ split?
    data->IsCenterAvailable = false;
  else if ((src_node_flags & DockNodeFlags_NoDockingOverOther) &&
           (!host_node || !host_node->IsEmpty()))
    data->IsCenterAvailable = false;
  else if ((src_node_flags & DockNodeFlags_NoDockingOverEmpty) && host_node &&
           host_node->IsEmpty())
    data->IsCenterAvailable = false;

  data->IsSidesAvailable = true;
  if ((dst_node_flags & DockNodeFlags_NoDockingSplit) ||
      g.IO.ConfigDockingNoSplit)
    data->IsSidesAvailable = false;
  else if (!is_outer_docking && host_node && host_node->ParentNode == NULL &&
           host_node->IsCentralNode())
    data->IsSidesAvailable = false;
  else if (src_node_flags & DockNodeFlags_NoDockingSplitOther)
    data->IsSidesAvailable = false;

  // Build a tentative future node (reuse same structure because it is
  // practical. Shape will be readjusted when previewing a split)
  data->FutureNode.HasCloseButton =
      (host_node ? host_node->HasCloseButton : host_window->HasCloseButton) ||
      (payload_window->HasCloseButton);
  data->FutureNode.HasWindowMenuButton =
      host_node ? true : ((host_window->Flags & WindowFlags_NoCollapse) == 0);
  data->FutureNode.Pos =
      ref_node_for_rect ? ref_node_for_rect->Pos : host_window->Pos;
  data->FutureNode.Size =
      ref_node_for_rect ? ref_node_for_rect->Size : host_window->Size;

  // Calculate drop shapes geometry for allowed splitting directions
  ASSERT(Dir_None == -1);
  data->SplitNode = host_node;
  data->SplitDir = Dir_None;
  data->IsSplitDirExplicit = false;
  if (!host_window->Collapsed)
    for (int dir = Dir_None; dir < Dir_COUNT; dir++) {
      if (dir == Dir_None && !data->IsCenterAvailable)
        continue;
      if (dir != Dir_None && !data->IsSidesAvailable)
        continue;
      if (DockNodeCalcDropRectsAndTestMousePos(
              data->FutureNode.Rect(), (Dir)dir, data->DropRectsDraw[dir + 1],
              is_outer_docking, &g.IO.MousePos)) {
        data->SplitDir = (Dir)dir;
        data->IsSplitDirExplicit = true;
      }
    }

  // When docking without holding Shift, we only allow and preview docking when
  // hovering over a drop rect or over the title bar
  data->IsDropAllowed =
      (data->SplitDir != Dir_None) || (data->IsCenterAvailable);
  if (!is_explicit_target && !data->IsSplitDirExplicit &&
      !g.IO.ConfigDockingWithShift)
    data->IsDropAllowed = false;

  // Calculate split area
  data->SplitRatio = 0.0f;
  if (data->SplitDir != Dir_None) {
    Dir split_dir = data->SplitDir;
    Axis split_axis =
        (split_dir == Dir_Left || split_dir == Dir_Right) ? Axis_X : Axis_Y;
    Vec2 pos_new, pos_old = data->FutureNode.Pos;
    Vec2 size_new, size_old = data->FutureNode.Size;
    DockNodeCalcSplitRects(pos_old, size_old, pos_new, size_new, split_dir,
                           payload_window->Size);

    // Calculate split ratio so we can pass it down the docking request
    float split_ratio =
        Saturate(size_new[split_axis] / data->FutureNode.Size[split_axis]);
    data->FutureNode.Pos = pos_new;
    data->FutureNode.Size = size_new;
    data->SplitRatio = (split_dir == Dir_Right || split_dir == Dir_Down)
                           ? (1.0f - split_ratio)
                           : (split_ratio);
  }
}

static void Gui::DockNodePreviewDockRender(Window *host_window,
                                           DockNode *host_node,
                                           Window *root_payload,
                                           const DockPreviewData *data) {
  Context &g = *GGui;
  ASSERT(g.CurrentWindow ==
         host_window); // Because we rely on font size to calculate tab sizes

  // With this option, we only display the preview on the target viewport, and
  // the payload viewport is made transparent. To compensate for the single
  // layer obstructed by the payload, we'll increase the alpha of the preview
  // nodes.
  const bool is_transparent_payload = g.IO.ConfigDockingTransparentPayload;

  // In case the two windows involved are on different viewports, we will draw
  // the overlay on each of them.
  int overlay_draw_lists_count = 0;
  DrawList *overlay_draw_lists[2];
  overlay_draw_lists[overlay_draw_lists_count++] =
      GetForegroundDrawList(host_window->Viewport);
  if (host_window->Viewport != root_payload->Viewport &&
      !is_transparent_payload)
    overlay_draw_lists[overlay_draw_lists_count++] =
        GetForegroundDrawList(root_payload->Viewport);

  // Draw main preview rectangle
  const U32 overlay_col_main =
      GetColorU32(Col_DockingPreview, is_transparent_payload ? 0.60f : 0.40f);
  const U32 overlay_col_drop =
      GetColorU32(Col_DockingPreview, is_transparent_payload ? 0.90f : 0.70f);
  const U32 overlay_col_drop_hovered =
      GetColorU32(Col_DockingPreview, is_transparent_payload ? 1.20f : 1.00f);
  const U32 overlay_col_lines = GetColorU32(
      Col_NavWindowingHighlight, is_transparent_payload ? 0.80f : 0.60f);

  // Display area preview
  const bool can_preview_tabs =
      (root_payload->DockNodeAsHost == NULL ||
       root_payload->DockNodeAsHost->Windows.Size > 0);
  if (data->IsDropAllowed) {
    Rect overlay_rect = data->FutureNode.Rect();
    if (data->SplitDir == Dir_None && can_preview_tabs)
      overlay_rect.Min.y += GetFrameHeight();
    if (data->SplitDir != Dir_None || data->IsCenterAvailable)
      for (int overlay_n = 0; overlay_n < overlay_draw_lists_count; overlay_n++)
        overlay_draw_lists[overlay_n]->AddRectFilled(
            overlay_rect.Min, overlay_rect.Max, overlay_col_main,
            host_window->WindowRounding,
            CalcRoundingFlagsForRectInRect(overlay_rect, host_window->Rect(),
                                           g.Style.DockingSeparatorSize));
  }

  // Display tab shape/label preview unless we are splitting node (it generally
  // makes the situation harder to read)
  if (data->IsDropAllowed && can_preview_tabs && data->SplitDir == Dir_None &&
      data->IsCenterAvailable) {
    // Compute target tab bar geometry so we can locate our preview tabs
    Rect tab_bar_rect;
    DockNodeCalcTabBarLayout(&data->FutureNode, NULL, &tab_bar_rect, NULL,
                             NULL);
    Vec2 tab_pos = tab_bar_rect.Min;
    if (host_node && host_node->TabBar) {
      if (!host_node->IsHiddenTabBar() && !host_node->IsNoTabBar())
        tab_pos.x += host_node->TabBar->WidthAllTabs +
                     g.Style.ItemInnerSpacing
                         .x; // We don't use OffsetNewTab because when using
                             // non-persistent-order tab bar it is incremented
                             // with each Tab submission.
      else
        tab_pos.x += g.Style.ItemInnerSpacing.x +
                     TabItemCalcSize(host_node->Windows[0]).x;
    } else if (!(host_window->Flags & WindowFlags_DockNodeHost)) {
      tab_pos.x += g.Style.ItemInnerSpacing.x +
                   TabItemCalcSize(host_window)
                       .x; // Account for slight offset which will be added when
                           // changing from title bar to tab bar
    }

    // Draw tab shape/label preview (payload may be a loose window or a host
    // window carrying multiple tabbed windows)
    if (root_payload->DockNodeAsHost)
      ASSERT(root_payload->DockNodeAsHost->Windows.Size <=
             root_payload->DockNodeAsHost->TabBar->Tabs.Size);
    TabBar *tab_bar_with_payload = root_payload->DockNodeAsHost
                                       ? root_payload->DockNodeAsHost->TabBar
                                       : NULL;
    const int payload_count =
        tab_bar_with_payload ? tab_bar_with_payload->Tabs.Size : 1;
    for (int payload_n = 0; payload_n < payload_count; payload_n++) {
      // DockNode's TabBar may have non-window Tabs manually appended by user
      Window *payload_window =
          tab_bar_with_payload ? tab_bar_with_payload->Tabs[payload_n].Window
                               : root_payload;
      if (tab_bar_with_payload && payload_window == NULL)
        continue;
      if (!DockNodeIsDropAllowedOne(payload_window, host_window))
        continue;

      // Calculate the tab bounding box for each payload window
      Vec2 tab_size = TabItemCalcSize(payload_window);
      Rect tab_bb(tab_pos.x, tab_pos.y, tab_pos.x + tab_size.x,
                  tab_pos.y + tab_size.y);
      tab_pos.x += tab_size.x + g.Style.ItemInnerSpacing.x;
      const U32 overlay_col_text = GetColorU32(
          payload_window->DockStyle.Colors[WindowDockStyleCol_Text]);
      const U32 overlay_col_tabs = GetColorU32(
          payload_window->DockStyle.Colors[WindowDockStyleCol_TabActive]);
      PushStyleColor(Col_Text, overlay_col_text);
      for (int overlay_n = 0; overlay_n < overlay_draw_lists_count;
           overlay_n++) {
        TabItemFlags tab_flags =
            (payload_window->Flags & WindowFlags_UnsavedDocument)
                ? TabItemFlags_UnsavedDocument
                : 0;
        if (!tab_bar_rect.Contains(tab_bb))
          overlay_draw_lists[overlay_n]->PushClipRect(tab_bar_rect.Min,
                                                      tab_bar_rect.Max);
        TabItemBackground(overlay_draw_lists[overlay_n], tab_bb, tab_flags,
                          overlay_col_tabs);
        TabItemLabelAndCloseButton(overlay_draw_lists[overlay_n], tab_bb,
                                   tab_flags, g.Style.FramePadding,
                                   payload_window->Name, 0, 0, false, NULL,
                                   NULL);
        if (!tab_bar_rect.Contains(tab_bb))
          overlay_draw_lists[overlay_n]->PopClipRect();
      }
      PopStyleColor();
    }
  }

  // Display drop boxes
  const float overlay_rounding = Max(3.0f, g.Style.FrameRounding);
  for (int dir = Dir_None; dir < Dir_COUNT; dir++) {
    if (!data->DropRectsDraw[dir + 1].IsInverted()) {
      Rect draw_r = data->DropRectsDraw[dir + 1];
      Rect draw_r_in = draw_r;
      draw_r_in.Expand(-2.0f);
      U32 overlay_col = (data->SplitDir == (Dir)dir && data->IsSplitDirExplicit)
                            ? overlay_col_drop_hovered
                            : overlay_col_drop;
      for (int overlay_n = 0; overlay_n < overlay_draw_lists_count;
           overlay_n++) {
        Vec2 center = Floor(draw_r_in.GetCenter());
        overlay_draw_lists[overlay_n]->AddRectFilled(
            draw_r.Min, draw_r.Max, overlay_col, overlay_rounding);
        overlay_draw_lists[overlay_n]->AddRect(
            draw_r_in.Min, draw_r_in.Max, overlay_col_lines, overlay_rounding);
        if (dir == Dir_Left || dir == Dir_Right)
          overlay_draw_lists[overlay_n]->AddLine(
              Vec2(center.x, draw_r_in.Min.y), Vec2(center.x, draw_r_in.Max.y),
              overlay_col_lines);
        if (dir == Dir_Up || dir == Dir_Down)
          overlay_draw_lists[overlay_n]->AddLine(
              Vec2(draw_r_in.Min.x, center.y), Vec2(draw_r_in.Max.x, center.y),
              overlay_col_lines);
      }
    }

    // Stop after Dir_None
    if ((host_node &&
         (host_node->MergedFlags & DockNodeFlags_NoDockingSplit)) ||
        g.IO.ConfigDockingNoSplit)
      return;
  }
}

//-----------------------------------------------------------------------------
// Docking: DockNode Tree manipulation functions
//-----------------------------------------------------------------------------
// - DockNodeTreeSplit()
// - DockNodeTreeMerge()
// - DockNodeTreeUpdatePosSize()
// - DockNodeTreeUpdateSplitterFindTouchingNode()
// - DockNodeTreeUpdateSplitter()
// - DockNodeTreeFindFallbackLeafNode()
// - DockNodeTreeFindNodeByPos()
//-----------------------------------------------------------------------------

void Gui::DockNodeTreeSplit(Context *ctx, DockNode *parent_node,
                            Axis split_axis, int split_inheritor_child_idx,
                            float split_ratio, DockNode *new_node) {
  Context &g = *GGui;
  ASSERT(split_axis != Axis_None);

  DockNode *child_0 = (new_node && split_inheritor_child_idx != 0)
                          ? new_node
                          : DockContextAddNode(ctx, 0);
  child_0->ParentNode = parent_node;

  DockNode *child_1 = (new_node && split_inheritor_child_idx != 1)
                          ? new_node
                          : DockContextAddNode(ctx, 0);
  child_1->ParentNode = parent_node;

  DockNode *child_inheritor =
      (split_inheritor_child_idx == 0) ? child_0 : child_1;
  DockNodeMoveChildNodes(child_inheritor, parent_node);
  parent_node->ChildNodes[0] = child_0;
  parent_node->ChildNodes[1] = child_1;
  parent_node->ChildNodes[split_inheritor_child_idx]->VisibleWindow =
      parent_node->VisibleWindow;
  parent_node->SplitAxis = split_axis;
  parent_node->VisibleWindow = NULL;
  parent_node->AuthorityForPos = parent_node->AuthorityForSize =
      DataAuthority_DockNode;

  float size_avail =
      (parent_node->Size[split_axis] - g.Style.DockingSeparatorSize);
  size_avail = Max(size_avail, g.Style.WindowMinSize[split_axis] * 2.0f);
  ASSERT(size_avail >
         0.0f); // If you created a node manually with DockBuilderAddNode(), you
                // need to also call DockBuilderSetNodeSize() before splitting.
  child_0->SizeRef = child_1->SizeRef = parent_node->Size;
  child_0->SizeRef[split_axis] = Trunc(size_avail * split_ratio);
  child_1->SizeRef[split_axis] =
      Trunc(size_avail - child_0->SizeRef[split_axis]);

  DockNodeMoveWindows(parent_node->ChildNodes[split_inheritor_child_idx],
                      parent_node);
  DockSettingsRenameNodeReferences(
      parent_node->ID, parent_node->ChildNodes[split_inheritor_child_idx]->ID);
  DockNodeUpdateHasCentralNodeChild(DockNodeGetRootNode(parent_node));
  DockNodeTreeUpdatePosSize(parent_node, parent_node->Pos, parent_node->Size);

  // Flags transfer (e.g. this is where we transfer the
  // DockNodeFlags_CentralNode property)
  child_0->SharedFlags =
      parent_node->SharedFlags & DockNodeFlags_SharedFlagsInheritMask_;
  child_1->SharedFlags =
      parent_node->SharedFlags & DockNodeFlags_SharedFlagsInheritMask_;
  child_inheritor->LocalFlags =
      parent_node->LocalFlags & DockNodeFlags_LocalFlagsTransferMask_;
  parent_node->LocalFlags &= ~DockNodeFlags_LocalFlagsTransferMask_;
  child_0->UpdateMergedFlags();
  child_1->UpdateMergedFlags();
  parent_node->UpdateMergedFlags();
  if (child_inheritor->IsCentralNode())
    DockNodeGetRootNode(parent_node)->CentralNode = child_inheritor;
}

void Gui::DockNodeTreeMerge(Context *ctx, DockNode *parent_node,
                            DockNode *merge_lead_child) {
  // When called from DockContextProcessUndockNode() it is possible that one of
  // the child is NULL.
  Context &g = *GGui;
  DockNode *child_0 = parent_node->ChildNodes[0];
  DockNode *child_1 = parent_node->ChildNodes[1];
  ASSERT(child_0 || child_1);
  ASSERT(merge_lead_child == child_0 || merge_lead_child == child_1);
  if ((child_0 && child_0->Windows.Size > 0) ||
      (child_1 && child_1->Windows.Size > 0)) {
    ASSERT(parent_node->TabBar == NULL);
    ASSERT(parent_node->Windows.Size == 0);
  }
  DEBUG_LOG_DOCKING(
      "[docking] DockNodeTreeMerge: 0x%08X + 0x%08X back into parent 0x%08X\n",
      child_0 ? child_0->ID : 0, child_1 ? child_1->ID : 0, parent_node->ID);

  Vec2 backup_last_explicit_size = parent_node->SizeRef;
  DockNodeMoveChildNodes(parent_node, merge_lead_child);
  if (child_0) {
    DockNodeMoveWindows(
        parent_node,
        child_0); // Generally only 1 of the 2 child node will have windows
    DockSettingsRenameNodeReferences(child_0->ID, parent_node->ID);
  }
  if (child_1) {
    DockNodeMoveWindows(parent_node, child_1);
    DockSettingsRenameNodeReferences(child_1->ID, parent_node->ID);
  }
  DockNodeApplyPosSizeToWindows(parent_node);
  parent_node->AuthorityForPos = parent_node->AuthorityForSize =
      parent_node->AuthorityForViewport = DataAuthority_Auto;
  parent_node->VisibleWindow = merge_lead_child->VisibleWindow;
  parent_node->SizeRef = backup_last_explicit_size;

  // Flags transfer
  parent_node->LocalFlags &=
      ~DockNodeFlags_LocalFlagsTransferMask_; // Preserve Dockspace flag
  parent_node->LocalFlags |= (child_0 ? child_0->LocalFlags : 0) &
                             DockNodeFlags_LocalFlagsTransferMask_;
  parent_node->LocalFlags |= (child_1 ? child_1->LocalFlags : 0) &
                             DockNodeFlags_LocalFlagsTransferMask_;
  parent_node->LocalFlagsInWindows =
      (child_0 ? child_0->LocalFlagsInWindows : 0) |
      (child_1 ? child_1->LocalFlagsInWindows
               : 0); // FIXME: Would be more consistent to update from actual
                     // windows
  parent_node->UpdateMergedFlags();

  if (child_0) {
    ctx->DockContext.Nodes.SetVoidPtr(child_0->ID, NULL);
    DELETE(child_0);
  }
  if (child_1) {
    ctx->DockContext.Nodes.SetVoidPtr(child_1->ID, NULL);
    DELETE(child_1);
  }
}

// Update Pos/Size for a node hierarchy (don't affect child Windows yet)
// (Depth-first, Pre-Order)
void Gui::DockNodeTreeUpdatePosSize(DockNode *node, Vec2 pos, Vec2 size,
                                    DockNode *only_write_to_single_node) {
  // During the regular dock node update we write to all nodes.
  // 'only_write_to_single_node' is only set when turning a node visible
  // mid-frame and we need its size right-away.
  Context &g = *GGui;
  const bool write_to_node =
      only_write_to_single_node == NULL || only_write_to_single_node == node;
  if (write_to_node) {
    node->Pos = pos;
    node->Size = size;
  }

  if (node->IsLeafNode())
    return;

  DockNode *child_0 = node->ChildNodes[0];
  DockNode *child_1 = node->ChildNodes[1];
  Vec2 child_0_pos = pos, child_1_pos = pos;
  Vec2 child_0_size = size, child_1_size = size;

  const bool child_0_is_toward_single_node =
      (only_write_to_single_node != NULL &&
       DockNodeIsInHierarchyOf(only_write_to_single_node, child_0));
  const bool child_1_is_toward_single_node =
      (only_write_to_single_node != NULL &&
       DockNodeIsInHierarchyOf(only_write_to_single_node, child_1));
  const bool child_0_is_or_will_be_visible =
      child_0->IsVisible || child_0_is_toward_single_node;
  const bool child_1_is_or_will_be_visible =
      child_1->IsVisible || child_1_is_toward_single_node;

  if (child_0_is_or_will_be_visible && child_1_is_or_will_be_visible) {
    const float spacing = g.Style.DockingSeparatorSize;
    const Axis axis = (Axis)node->SplitAxis;
    const float size_avail = Max(size[axis] - spacing, 0.0f);

    // Size allocation policy
    // 1) The first 0..WindowMinSize[axis]*2 are allocated evenly to both
    // windows.
    const float size_min_each =
        Trunc(Min(size_avail, g.Style.WindowMinSize[axis] * 2.0f) * 0.5f);

    // FIXME: Blocks 2) and 3) are essentially doing nearly the same thing.
    // Difference are: write-back to SizeRef; application of a minimum size;
    // rounding before Trunc() Clarify and rework differences between Size &
    // SizeRef and purpose of WantLockSizeOnce

    // 2) Process locked absolute size (during a splitter resize we preserve the
    // child of nodes not touching the splitter edge)
    if (child_0->WantLockSizeOnce && !child_1->WantLockSizeOnce) {
      child_0_size[axis] = child_0->SizeRef[axis] =
          Min(size_avail - 1.0f, child_0->Size[axis]);
      child_1_size[axis] = child_1->SizeRef[axis] =
          (size_avail - child_0_size[axis]);
      ASSERT(child_0->SizeRef[axis] > 0.0f && child_1->SizeRef[axis] > 0.0f);
    } else if (child_1->WantLockSizeOnce && !child_0->WantLockSizeOnce) {
      child_1_size[axis] = child_1->SizeRef[axis] =
          Min(size_avail - 1.0f, child_1->Size[axis]);
      child_0_size[axis] = child_0->SizeRef[axis] =
          (size_avail - child_1_size[axis]);
      ASSERT(child_0->SizeRef[axis] > 0.0f && child_1->SizeRef[axis] > 0.0f);
    } else if (child_0->WantLockSizeOnce && child_1->WantLockSizeOnce) {
      // FIXME-DOCK: We cannot honor the requested size, so apply ratio.
      // Currently this path will only be taken if code programmatically sets
      // WantLockSizeOnce
      float split_ratio =
          child_0_size[axis] / (child_0_size[axis] + child_1_size[axis]);
      child_0_size[axis] = child_0->SizeRef[axis] =
          Trunc(size_avail * split_ratio);
      child_1_size[axis] = child_1->SizeRef[axis] =
          (size_avail - child_0_size[axis]);
      ASSERT(child_0->SizeRef[axis] > 0.0f && child_1->SizeRef[axis] > 0.0f);
    }

    // 3) If one window is the central node (~ use remaining space, should be
    // made explicit!), use explicit size from the other, and remainder for the
    // central node
    else if (child_0->SizeRef[axis] != 0.0f && child_1->HasCentralNodeChild) {
      child_0_size[axis] =
          Min(size_avail - size_min_each, child_0->SizeRef[axis]);
      child_1_size[axis] = (size_avail - child_0_size[axis]);
    } else if (child_1->SizeRef[axis] != 0.0f && child_0->HasCentralNodeChild) {
      child_1_size[axis] =
          Min(size_avail - size_min_each, child_1->SizeRef[axis]);
      child_0_size[axis] = (size_avail - child_1_size[axis]);
    } else {
      // 4) Otherwise distribute according to the relative ratio of each SizeRef
      // value
      float split_ratio = child_0->SizeRef[axis] /
                          (child_0->SizeRef[axis] + child_1->SizeRef[axis]);
      child_0_size[axis] =
          Max(size_min_each, Trunc(size_avail * split_ratio + 0.5f));
      child_1_size[axis] = (size_avail - child_0_size[axis]);
    }

    child_1_pos[axis] += spacing + child_0_size[axis];
  }

  if (only_write_to_single_node == NULL)
    child_0->WantLockSizeOnce = child_1->WantLockSizeOnce = false;

  const bool child_0_recurse = only_write_to_single_node
                                   ? child_0_is_toward_single_node
                                   : child_0->IsVisible;
  const bool child_1_recurse = only_write_to_single_node
                                   ? child_1_is_toward_single_node
                                   : child_1->IsVisible;
  if (child_0_recurse)
    DockNodeTreeUpdatePosSize(child_0, child_0_pos, child_0_size);
  if (child_1_recurse)
    DockNodeTreeUpdatePosSize(child_1, child_1_pos, child_1_size);
}

static void
DockNodeTreeUpdateSplitterFindTouchingNode(DockNode *node, Axis axis, int side,
                                           Vector<DockNode *> *touching_nodes) {
  if (node->IsLeafNode()) {
    touching_nodes->push_back(node);
    return;
  }
  if (node->ChildNodes[0]->IsVisible)
    if (node->SplitAxis != axis || side == 0 || !node->ChildNodes[1]->IsVisible)
      DockNodeTreeUpdateSplitterFindTouchingNode(node->ChildNodes[0], axis,
                                                 side, touching_nodes);
  if (node->ChildNodes[1]->IsVisible)
    if (node->SplitAxis != axis || side == 1 || !node->ChildNodes[0]->IsVisible)
      DockNodeTreeUpdateSplitterFindTouchingNode(node->ChildNodes[1], axis,
                                                 side, touching_nodes);
}

// (Depth-First, Pre-Order)
void Gui::DockNodeTreeUpdateSplitter(DockNode *node) {
  if (node->IsLeafNode())
    return;

  Context &g = *GGui;

  DockNode *child_0 = node->ChildNodes[0];
  DockNode *child_1 = node->ChildNodes[1];
  if (child_0->IsVisible && child_1->IsVisible) {
    // Bounding box of the splitter cover the space between both nodes (w =
    // Spacing, h = Size[xy^1] for when splitting horizontally)
    const Axis axis = (Axis)node->SplitAxis;
    ASSERT(axis != Axis_None);
    Rect bb;
    bb.Min = child_0->Pos;
    bb.Max = child_1->Pos;
    bb.Min[axis] += child_0->Size[axis];
    bb.Max[axis ^ 1] += child_1->Size[axis ^ 1];
    // if (g.IO.KeyCtrl)
    // GetForegroundDrawList(g.CurrentWindow->Viewport)->AddRect(bb.Min, bb.Max,
    // COL32(255,0,255,255));

    const DockNodeFlags merged_flags =
        child_0->MergedFlags |
        child_1->MergedFlags; // Merged flags for BOTH childs
    const DockNodeFlags no_resize_axis_flag =
        (axis == Axis_X) ? DockNodeFlags_NoResizeX : DockNodeFlags_NoResizeY;
    if ((merged_flags & DockNodeFlags_NoResize) ||
        (merged_flags & no_resize_axis_flag)) {
      Window *window = g.CurrentWindow;
      window->DrawList->AddRectFilled(
          bb.Min, bb.Max, GetColorU32(Col_Separator), g.Style.FrameRounding);
    } else {
      // bb.Min[axis] += 1; // Display a little inward so highlight doesn't
      // connect with nearby tabs on the neighbor node. bb.Max[axis] -= 1;
      PushID(node->ID);

      // Find resizing limits by gathering list of nodes that are touching the
      // splitter line.
      Vector<DockNode *> touching_nodes[2];
      float min_size = g.Style.WindowMinSize[axis];
      float resize_limits[2];
      resize_limits[0] = node->ChildNodes[0]->Pos[axis] + min_size;
      resize_limits[1] = node->ChildNodes[1]->Pos[axis] +
                         node->ChildNodes[1]->Size[axis] - min_size;

      ID splitter_id = GetID("##Splitter");
      if (g.ActiveId == splitter_id) // Only process when splitter is active
      {
        DockNodeTreeUpdateSplitterFindTouchingNode(child_0, axis, 1,
                                                   &touching_nodes[0]);
        DockNodeTreeUpdateSplitterFindTouchingNode(child_1, axis, 0,
                                                   &touching_nodes[1]);
        for (int touching_node_n = 0; touching_node_n < touching_nodes[0].Size;
             touching_node_n++)
          resize_limits[0] = Max(
              resize_limits[0],
              touching_nodes[0][touching_node_n]->Rect().Min[axis] + min_size);
        for (int touching_node_n = 0; touching_node_n < touching_nodes[1].Size;
             touching_node_n++)
          resize_limits[1] = Min(
              resize_limits[1],
              touching_nodes[1][touching_node_n]->Rect().Max[axis] - min_size);

        // [DEBUG] Render touching nodes & limits
        /*
        DrawList* draw_list = node->HostWindow ?
        GetForegroundDrawList(node->HostWindow) :
        GetForegroundDrawList(GetMainViewport()); for (int n = 0; n < 2; n++)
        {
            for (int touching_node_n = 0; touching_node_n <
        touching_nodes[n].Size; touching_node_n++)
                draw_list->AddRect(touching_nodes[n][touching_node_n]->Pos,
        touching_nodes[n][touching_node_n]->Pos +
        touching_nodes[n][touching_node_n]->Size, COL32(0, 255, 0, 255)); if
        (axis == Axis_X) draw_list->AddLine(Vec2(resize_limits[n],
        node->ChildNodes[n]->Pos.y), Vec2(resize_limits[n],
        node->ChildNodes[n]->Pos.y + node->ChildNodes[n]->Size.y), COL32(255,
        0, 255, 255), 3.0f); else
                draw_list->AddLine(Vec2(node->ChildNodes[n]->Pos.x,
        resize_limits[n]), Vec2(node->ChildNodes[n]->Pos.x +
        node->ChildNodes[n]->Size.x, resize_limits[n]), COL32(255, 0, 255,
        255), 3.0f);
        }
        */
      }

      // Use a short delay before highlighting the splitter (and changing the
      // mouse cursor) in order for regular mouse movement to not highlight many
      // splitters
      float cur_size_0 = child_0->Size[axis];
      float cur_size_1 = child_1->Size[axis];
      float min_size_0 = resize_limits[0] - child_0->Pos[axis];
      float min_size_1 =
          child_1->Pos[axis] + child_1->Size[axis] - resize_limits[1];
      U32 bg_col = GetColorU32(Col_WindowBg);
      if (SplitterBehavior(bb, GetID("##Splitter"), axis, &cur_size_0,
                           &cur_size_1, min_size_0, min_size_1,
                           WINDOWS_HOVER_PADDING,
                           WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER, bg_col)) {
        if (touching_nodes[0].Size > 0 && touching_nodes[1].Size > 0) {
          child_0->Size[axis] = child_0->SizeRef[axis] = cur_size_0;
          child_1->Pos[axis] -= cur_size_1 - child_1->Size[axis];
          child_1->Size[axis] = child_1->SizeRef[axis] = cur_size_1;

          // Lock the size of every node that is a sibling of the node we are
          // touching This might be less desirable if we can merge sibling of a
          // same axis into the same parental level.
          for (int side_n = 0; side_n < 2; side_n++)
            for (int touching_node_n = 0;
                 touching_node_n < touching_nodes[side_n].Size;
                 touching_node_n++) {
              DockNode *touching_node = touching_nodes[side_n][touching_node_n];
              // DrawList* draw_list = node->HostWindow ?
              // GetForegroundDrawList(node->HostWindow) :
              // GetForegroundDrawList(GetMainViewport());
              // draw_list->AddRect(touching_node->Pos, touching_node->Pos +
              // touching_node->Size, COL32(255, 128, 0, 255));
              while (touching_node->ParentNode != node) {
                if (touching_node->ParentNode->SplitAxis == axis) {
                  // Mark other node so its size will be preserved during the
                  // upcoming call to DockNodeTreeUpdatePosSize().
                  DockNode *node_to_preserve =
                      touching_node->ParentNode->ChildNodes[side_n];
                  node_to_preserve->WantLockSizeOnce = true;
                  // draw_list->AddRect(touching_node->Pos,
                  // touching_node->Rect().Max, COL32(255, 0, 0, 255));
                  // draw_list->AddRectFilled(node_to_preserve->Pos,
                  // node_to_preserve->Rect().Max, COL32(0, 255, 0, 100));
                }
                touching_node = touching_node->ParentNode;
              }
            }

          DockNodeTreeUpdatePosSize(child_0, child_0->Pos, child_0->Size);
          DockNodeTreeUpdatePosSize(child_1, child_1->Pos, child_1->Size);
          MarkIniSettingsDirty();
        }
      }
      PopID();
    }
  }

  if (child_0->IsVisible)
    DockNodeTreeUpdateSplitter(child_0);
  if (child_1->IsVisible)
    DockNodeTreeUpdateSplitter(child_1);
}

DockNode *Gui::DockNodeTreeFindFallbackLeafNode(DockNode *node) {
  if (node->IsLeafNode())
    return node;
  if (DockNode *leaf_node =
          DockNodeTreeFindFallbackLeafNode(node->ChildNodes[0]))
    return leaf_node;
  if (DockNode *leaf_node =
          DockNodeTreeFindFallbackLeafNode(node->ChildNodes[1]))
    return leaf_node;
  return NULL;
}

DockNode *Gui::DockNodeTreeFindVisibleNodeByPos(DockNode *node, Vec2 pos) {
  if (!node->IsVisible)
    return NULL;

  const float dock_spacing = 0.0f; // g.Style.ItemInnerSpacing.x; // FIXME:
                                   // Relation to DOCKING_SPLITTER_SIZE?
  Rect r(node->Pos, node->Pos + node->Size);
  r.Expand(dock_spacing * 0.5f);
  bool inside = r.Contains(pos);
  if (!inside)
    return NULL;

  if (node->IsLeafNode())
    return node;
  if (DockNode *hovered_node =
          DockNodeTreeFindVisibleNodeByPos(node->ChildNodes[0], pos))
    return hovered_node;
  if (DockNode *hovered_node =
          DockNodeTreeFindVisibleNodeByPos(node->ChildNodes[1], pos))
    return hovered_node;

  // This means we are hovering over the splitter/spacing of a parent node
  return node;
}

//-----------------------------------------------------------------------------
// Docking: Public Functions (SetWindowDock, DockSpace, DockSpaceOverViewport)
//-----------------------------------------------------------------------------
// - SetWindowDock() [Internal]
// - DockSpace()
// - DockSpaceOverViewport()
//-----------------------------------------------------------------------------

// [Internal] Called via SetNextWindowDockID()
void Gui::SetWindowDock(Window *window, ID dock_id, Cond cond) {
  // Test condition (NB: bit 0 is always true) and clear flags for next time
  if (cond && (window->SetWindowDockAllowFlags & cond) == 0)
    return;
  window->SetWindowDockAllowFlags &=
      ~(Cond_Once | Cond_FirstUseEver | Cond_Appearing);

  if (window->DockId == dock_id)
    return;

  // If the user attempt to set a dock id that is a split node, we'll dig within
  // to find a suitable docking spot
  Context &g = *GGui;
  if (DockNode *new_node = DockContextFindNodeByID(&g, dock_id))
    if (new_node->IsSplitNode()) {
      // Policy: Find central node or latest focused node. We first move back to
      // our root node.
      new_node = DockNodeGetRootNode(new_node);
      if (new_node->CentralNode) {
        ASSERT(new_node->CentralNode->IsCentralNode());
        dock_id = new_node->CentralNode->ID;
      } else {
        dock_id = new_node->LastFocusedNodeId;
      }
    }

  if (window->DockId == dock_id)
    return;

  if (window->DockNode)
    DockNodeRemoveWindow(window->DockNode, window, 0);
  window->DockId = dock_id;
}

// Create an explicit dockspace node within an existing window. Also expose dock
// node flags and creates a CentralNode by default. The Central Node is always
// displayed even when empty and shrink/extend according to the requested size
// of its neighbors. DockSpace() needs to be submitted _before_ any window they
// can host. If you use a dockspace, submit it early in your app. When
// DockNodeFlags_KeepAliveOnly is set, nothing is submitted in the current
// window (function may be called from any location).
ID Gui::DockSpace(ID id, const Vec2 &size_arg, DockNodeFlags flags,
                  const WindowClass *window_class) {
  Context &g = *GGui;
  Window *window = GetCurrentWindowRead();
  if (!(g.IO.ConfigFlags & ConfigFlags_DockingEnable))
    return 0;

  // Early out if parent window is hidden/collapsed
  // This is faster but also DockNodeUpdateTabBar() relies on TabBarLayout()
  // running (which won't if SkipItems=true) to set NextSelectedTabId = 0). See
  // #2960. If for whichever reason this is causing problem we would need to
  // ensure that DockNodeUpdateTabBar() ends up clearing NextSelectedTabId even
  // if SkipItems=true.
  if (window->SkipItems)
    flags |= DockNodeFlags_KeepAliveOnly;
  if ((flags & DockNodeFlags_KeepAliveOnly) == 0)
    window = GetCurrentWindow(); // call to set window->WriteAccessed = true;

  ASSERT((flags & DockNodeFlags_DockSpace) == 0);
  ASSERT(id != 0);
  DockNode *node = DockContextFindNodeByID(&g, id);
  if (!node) {
    DEBUG_LOG_DOCKING("[docking] DockSpace: dockspace node 0x%08X created\n",
                      id);
    node = DockContextAddNode(&g, id);
    node->SetLocalFlags(DockNodeFlags_CentralNode);
  }
  if (window_class && window_class->ClassId != node->WindowClass.ClassId)
    DEBUG_LOG_DOCKING("[docking] DockSpace: dockspace node 0x%08X: setup "
                      "WindowClass 0x%08X -> 0x%08X\n",
                      id, node->WindowClass.ClassId, window_class->ClassId);
  node->SharedFlags = flags;
  node->WindowClass = window_class ? *window_class : WindowClass();

  // When a DockSpace transitioned form implicit to explicit this may be called
  // a second time It is possible that the node has already been claimed by a
  // docked window which appeared before the DockSpace() node, so we overwrite
  // IsDockSpace again.
  if (node->LastFrameActive == g.FrameCount &&
      !(flags & DockNodeFlags_KeepAliveOnly)) {
    ASSERT(node->IsDockSpace() == false &&
           "Cannot call DockSpace() twice a frame with the same ID");
    node->SetLocalFlags(node->LocalFlags | DockNodeFlags_DockSpace);
    return id;
  }
  node->SetLocalFlags(node->LocalFlags | DockNodeFlags_DockSpace);

  // Keep alive mode, this is allow windows docked into this node so stay docked
  // even if they are not visible
  if (flags & DockNodeFlags_KeepAliveOnly) {
    node->LastFrameAlive = g.FrameCount;
    return id;
  }

  const Vec2 content_avail = GetContentRegionAvail();
  Vec2 size = Trunc(size_arg);
  if (size.x <= 0.0f)
    size.x = Max(
        content_avail.x + size.x,
        4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
  if (size.y <= 0.0f)
    size.y = Max(content_avail.y + size.y, 4.0f);
  ASSERT(size.x > 0.0f && size.y > 0.0f);

  node->Pos = window->DC.CursorPos;
  node->Size = node->SizeRef = size;
  SetNextWindowPos(node->Pos);
  SetNextWindowSize(node->Size);
  g.NextWindowData.PosUndock = false;

  // FIXME-DOCK: Why do we need a child window to host a dockspace, could we
  // host it in the existing window?
  // FIXME-DOCK: What is the reason for not simply calling BeginChild()? (OK to
  // have a reason but should be commented)
  WindowFlags window_flags = WindowFlags_ChildWindow | WindowFlags_DockNodeHost;
  window_flags |= WindowFlags_NoSavedSettings | WindowFlags_NoResize |
                  WindowFlags_NoCollapse | WindowFlags_NoTitleBar;
  window_flags |= WindowFlags_NoScrollbar | WindowFlags_NoScrollWithMouse;
  window_flags |= WindowFlags_NoBackground;

  char title[256];
  FormatString(title, ARRAYSIZE(title), "%s/DockSpace_%08X", window->Name, id);

  PushStyleVar(StyleVar_ChildBorderSize, 0.0f);
  Begin(title, NULL, window_flags);
  PopStyleVar();

  Window *host_window = g.CurrentWindow;
  DockNodeSetupHostWindow(node, host_window);
  host_window->ChildId = window->GetID(title);
  node->OnlyNodeWithWindows = NULL;

  ASSERT(node->IsRootNode());

  // We need to handle the rare case were a central node is missing.
  // This can happen if the node was first created manually with
  // DockBuilderAddNode() but _without_ the DockNodeFlags_Dockspace. Doing
  // it correctly would set the _CentralNode flags, which would then propagate
  // according to subsequent split. It would also be ambiguous to attempt to
  // assign a central node while there are split nodes, so we wait until there's
  // a single node remaining. The specific sub-property of _CentralNode we are
  // interested in recovering here is the "Don't delete when empty" property, as
  // it doesn't make sense for an empty dockspace to not have this property.
  if (node->IsLeafNode() && !node->IsCentralNode())
    node->SetLocalFlags(node->LocalFlags | DockNodeFlags_CentralNode);

  // Update the node
  DockNodeUpdate(node);

  End();

  Rect bb(node->Pos, node->Pos + size);
  ItemSize(size);
  ItemAdd(bb, id, NULL,
          ItemFlags_NoNav); // Not a nav point (could be, would need to
                            // draw the nav rect and replicate/refactor
                            // activation from BeginChild(), but seems like
                            // CTRL+Tab works better here?)
  if ((g.LastItemData.StatusFlags & ItemStatusFlags_HoveredRect) &&
      IsWindowChildOf(
          g.HoveredWindow, host_window, false,
          true)) // To fullfill IsItemHovered(), similar to EndChild()
    g.LastItemData.StatusFlags |= ItemStatusFlags_HoveredWindow;

  return id;
}

// Tips: Use with DockNodeFlags_PassthruCentralNode!
// The limitation with this call is that your window won't have a menu bar.
// Even though we could pass window flags, it would also require the user to be
// able to call BeginMenuBar() somehow meaning we can't Begin/End in a single
// function. But you can also use BeginMainMenuBar(). If you really want a menu
// bar inside the same window as the one hosting the dockspace, you will need to
// copy this code somewhere and tweak it.
ID Gui::DockSpaceOverViewport(const Viewport *viewport,
                              DockNodeFlags dockspace_flags,
                              const WindowClass *window_class) {
  if (viewport == NULL)
    viewport = GetMainViewport();

  SetNextWindowPos(viewport->WorkPos);
  SetNextWindowSize(viewport->WorkSize);
  SetNextWindowViewport(viewport->ID);

  WindowFlags host_window_flags = 0;
  host_window_flags |= WindowFlags_NoTitleBar | WindowFlags_NoCollapse |
                       WindowFlags_NoResize | WindowFlags_NoMove |
                       WindowFlags_NoDocking;
  host_window_flags |=
      WindowFlags_NoBringToFrontOnFocus | WindowFlags_NoNavFocus;
  if (dockspace_flags & DockNodeFlags_PassthruCentralNode)
    host_window_flags |= WindowFlags_NoBackground;

  char label[32];
  FormatString(label, ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);

  PushStyleVar(StyleVar_WindowRounding, 0.0f);
  PushStyleVar(StyleVar_WindowBorderSize, 0.0f);
  PushStyleVar(StyleVar_WindowPadding, Vec2(0.0f, 0.0f));
  Begin(label, NULL, host_window_flags);
  PopStyleVar(3);

  ID dockspace_id = GetID("DockSpace");
  DockSpace(dockspace_id, Vec2(0.0f, 0.0f), dockspace_flags, window_class);
  End();

  return dockspace_id;
}

//-----------------------------------------------------------------------------
// Docking: Builder Functions
//-----------------------------------------------------------------------------
// Very early end-user API to manipulate dock nodes.
// Only available in internal.hpp. Expect this API to change/break!
// It is expected that those functions are all called _before_ the dockspace
// node submission.
//-----------------------------------------------------------------------------
// - DockBuilderDockWindow()
// - DockBuilderGetNode()
// - DockBuilderSetNodePos()
// - DockBuilderSetNodeSize()
// - DockBuilderAddNode()
// - DockBuilderRemoveNode()
// - DockBuilderRemoveNodeChildNodes()
// - DockBuilderRemoveNodeDockedWindows()
// - DockBuilderSplitNode()
// - DockBuilderCopyNodeRec()
// - DockBuilderCopyNode()
// - DockBuilderCopyWindowSettings()
// - DockBuilderCopyDockSpace()
// - DockBuilderFinish()
//-----------------------------------------------------------------------------

void Gui::DockBuilderDockWindow(const char *window_name, ID node_id) {
  // We don't preserve relative order of multiple docked windows (by clearing
  // DockOrder back to -1)
  Context &g = *GGui;
  UNUSED(g);
  DEBUG_LOG_DOCKING("[docking] DockBuilderDockWindow '%s' to node 0x%08X\n",
                    window_name, node_id);
  ID window_id = HashStr(window_name);
  if (Window *window = FindWindowByID(window_id)) {
    // Apply to created window
    ID prev_node_id = window->DockId;
    SetWindowDock(window, node_id, Cond_Always);
    if (window->DockId != prev_node_id)
      window->DockOrder = -1;
  } else {
    // Apply to settings
    WindowSettings *settings = FindWindowSettingsByID(window_id);
    if (settings == NULL)
      settings = CreateNewWindowSettings(window_name);
    if (settings->DockId != node_id)
      settings->DockOrder = -1;
    settings->DockId = node_id;
  }
}

DockNode *Gui::DockBuilderGetNode(ID node_id) {
  Context &g = *GGui;
  return DockContextFindNodeByID(&g, node_id);
}

void Gui::DockBuilderSetNodePos(ID node_id, Vec2 pos) {
  Context &g = *GGui;
  DockNode *node = DockContextFindNodeByID(&g, node_id);
  if (node == NULL)
    return;
  node->Pos = pos;
  node->AuthorityForPos = DataAuthority_DockNode;
}

void Gui::DockBuilderSetNodeSize(ID node_id, Vec2 size) {
  Context &g = *GGui;
  DockNode *node = DockContextFindNodeByID(&g, node_id);
  if (node == NULL)
    return;
  ASSERT(size.x > 0.0f && size.y > 0.0f);
  node->Size = node->SizeRef = size;
  node->AuthorityForSize = DataAuthority_DockNode;
}

// Make sure to use the DockNodeFlags_DockSpace flag to create a dockspace
// node! Otherwise this will create a floating node!
// - Floating node: you can then call
// DockBuilderSetNodePos()/DockBuilderSetNodeSize() to position and size the
// floating node.
// - Dockspace node: calling DockBuilderSetNodePos() is unnecessary.
// - If you intend to split a node immediately after creation using
// DockBuilderSplitNode(), make sure to call DockBuilderSetNodeSize()
// beforehand!
//   For various reason, the splitting code currently needs a base size
//   otherwise space may not be allocated as precisely as you would expect.
// - Use (id == 0) to let the system allocate a node identifier.
// - Existing node with a same id will be removed.
ID Gui::DockBuilderAddNode(ID node_id, DockNodeFlags flags) {
  Context &g = *GGui;
  UNUSED(g);
  DEBUG_LOG_DOCKING("[docking] DockBuilderAddNode 0x%08X flags=%08X\n", node_id,
                    flags);

  if (node_id != 0)
    DockBuilderRemoveNode(node_id);

  DockNode *node = NULL;
  if (flags & DockNodeFlags_DockSpace) {
    DockSpace(node_id, Vec2(0, 0),
              (flags & ~DockNodeFlags_DockSpace) | DockNodeFlags_KeepAliveOnly);
    node = DockContextFindNodeByID(&g, node_id);
  } else {
    node = DockContextAddNode(&g, node_id);
    node->SetLocalFlags(flags);
  }
  node->LastFrameAlive = g.FrameCount; // Set this otherwise BeginDocked will
                                       // undock during the same frame.
  return node->ID;
}

void Gui::DockBuilderRemoveNode(ID node_id) {
  Context &g = *GGui;
  UNUSED(g);
  DEBUG_LOG_DOCKING("[docking] DockBuilderRemoveNode 0x%08X\n", node_id);

  DockNode *node = DockContextFindNodeByID(&g, node_id);
  if (node == NULL)
    return;
  DockBuilderRemoveNodeDockedWindows(node_id, true);
  DockBuilderRemoveNodeChildNodes(node_id);
  // Node may have moved or deleted if e.g. any merge happened
  node = DockContextFindNodeByID(&g, node_id);
  if (node == NULL)
    return;
  if (node->IsCentralNode() && node->ParentNode)
    node->ParentNode->SetLocalFlags(node->ParentNode->LocalFlags |
                                    DockNodeFlags_CentralNode);
  DockContextRemoveNode(&g, node, true);
}

// root_id = 0 to remove all, root_id != 0 to remove child of given node.
void Gui::DockBuilderRemoveNodeChildNodes(ID root_id) {
  Context &g = *GGui;
  DockContext *dc = &g.DockContext;

  DockNode *root_node = root_id ? DockContextFindNodeByID(&g, root_id) : NULL;
  if (root_id && root_node == NULL)
    return;
  bool has_central_node = false;

  DataAuthority backup_root_node_authority_for_pos =
      root_node ? root_node->AuthorityForPos : DataAuthority_Auto;
  DataAuthority backup_root_node_authority_for_size =
      root_node ? root_node->AuthorityForSize : DataAuthority_Auto;

  // Process active windows
  Vector<DockNode *> nodes_to_remove;
  for (int n = 0; n < dc->Nodes.Data.Size; n++)
    if (DockNode *node = (DockNode *)dc->Nodes.Data[n].val_p) {
      bool want_removal =
          (root_id == 0) ||
          (node->ID != root_id && DockNodeGetRootNode(node)->ID == root_id);
      if (want_removal) {
        if (node->IsCentralNode())
          has_central_node = true;
        if (root_id != 0)
          DockContextQueueNotifyRemovedNode(&g, node);
        if (root_node) {
          DockNodeMoveWindows(root_node, node);
          DockSettingsRenameNodeReferences(node->ID, root_node->ID);
        }
        nodes_to_remove.push_back(node);
      }
    }

  // DockNodeMoveWindows->DockNodeAddWindow will normally set those when
  // reaching two windows (which is only adequate during interactive merge) Make
  // sure we don't lose our current pos/size. (FIXME-DOCK: Consider tidying up
  // that code in DockNodeAddWindow instead)
  if (root_node) {
    root_node->AuthorityForPos = backup_root_node_authority_for_pos;
    root_node->AuthorityForSize = backup_root_node_authority_for_size;
  }

  // Apply to settings
  for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
       settings = g.SettingsWindows.next_chunk(settings))
    if (ID window_settings_dock_id = settings->DockId)
      for (int n = 0; n < nodes_to_remove.Size; n++)
        if (nodes_to_remove[n]->ID == window_settings_dock_id) {
          settings->DockId = root_id;
          break;
        }

  // Not really efficient, but easier to destroy a whole hierarchy considering
  // DockContextRemoveNode is attempting to merge nodes
  if (nodes_to_remove.Size > 1)
    Qsort(nodes_to_remove.Data, nodes_to_remove.Size, sizeof(DockNode *),
          DockNodeComparerDepthMostFirst);
  for (int n = 0; n < nodes_to_remove.Size; n++)
    DockContextRemoveNode(&g, nodes_to_remove[n], false);

  if (root_id == 0) {
    dc->Nodes.Clear();
    dc->Requests.clear();
  } else if (has_central_node) {
    root_node->CentralNode = root_node;
    root_node->SetLocalFlags(root_node->LocalFlags | DockNodeFlags_CentralNode);
  }
}

void Gui::DockBuilderRemoveNodeDockedWindows(ID root_id,
                                             bool clear_settings_refs) {
  // Clear references in settings
  Context &g = *GGui;
  if (clear_settings_refs) {
    for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
         settings = g.SettingsWindows.next_chunk(settings)) {
      bool want_removal = (root_id == 0) || (settings->DockId == root_id);
      if (!want_removal && settings->DockId != 0)
        if (DockNode *node = DockContextFindNodeByID(&g, settings->DockId))
          if (DockNodeGetRootNode(node)->ID == root_id)
            want_removal = true;
      if (want_removal)
        settings->DockId = 0;
    }
  }

  // Clear references in windows
  for (int n = 0; n < g.Windows.Size; n++) {
    Window *window = g.Windows[n];
    bool want_removal =
        (root_id == 0) ||
        (window->DockNode &&
         DockNodeGetRootNode(window->DockNode)->ID == root_id) ||
        (window->DockNodeAsHost && window->DockNodeAsHost->ID == root_id);
    if (want_removal) {
      const ID backup_dock_id = window->DockId;
      UNUSED(backup_dock_id);
      DockContextProcessUndockWindow(&g, window, clear_settings_refs);
      if (!clear_settings_refs)
        ASSERT(window->DockId == backup_dock_id);
    }
  }
}

// If 'out_id_at_dir' or 'out_id_at_opposite_dir' are non NULL, the function
// will write out the ID of the two new nodes created. Return value is ID of the
// node at the specified direction, so same as (*out_id_at_dir) if that pointer
// is set.
// FIXME-DOCK: We are not exposing nor using split_outer.
ID Gui::DockBuilderSplitNode(ID id, Dir split_dir,
                             float size_ratio_for_node_at_dir,
                             ID *out_id_at_dir, ID *out_id_at_opposite_dir) {
  Context &g = *GGui;
  ASSERT(split_dir != Dir_None);
  DEBUG_LOG_DOCKING(
      "[docking] DockBuilderSplitNode: node 0x%08X, split_dir %d\n", id,
      split_dir);

  DockNode *node = DockContextFindNodeByID(&g, id);
  if (node == NULL) {
    ASSERT(node != NULL);
    return 0;
  }

  ASSERT(!node->IsSplitNode()); // Assert if already Split

  DockRequest req;
  req.Type = DockRequestType_Split;
  req.DockTargetWindow = NULL;
  req.DockTargetNode = node;
  req.DockPayload = NULL;
  req.DockSplitDir = split_dir;
  req.DockSplitRatio = Saturate((split_dir == Dir_Left || split_dir == Dir_Up)
                                    ? size_ratio_for_node_at_dir
                                    : 1.0f - size_ratio_for_node_at_dir);
  req.DockSplitOuter = false;
  DockContextProcessDock(&g, &req);

  ID id_at_dir =
      node->ChildNodes[(split_dir == Dir_Left || split_dir == Dir_Up) ? 0 : 1]
          ->ID;
  ID id_at_opposite_dir =
      node->ChildNodes[(split_dir == Dir_Left || split_dir == Dir_Up) ? 1 : 0]
          ->ID;
  if (out_id_at_dir)
    *out_id_at_dir = id_at_dir;
  if (out_id_at_opposite_dir)
    *out_id_at_opposite_dir = id_at_opposite_dir;
  return id_at_dir;
}

static DockNode *DockBuilderCopyNodeRec(DockNode *src_node,
                                        ID dst_node_id_if_known,
                                        Vector<ID> *out_node_remap_pairs) {
  Context &g = *GGui;
  DockNode *dst_node = Gui::DockContextAddNode(&g, dst_node_id_if_known);
  dst_node->SharedFlags = src_node->SharedFlags;
  dst_node->LocalFlags = src_node->LocalFlags;
  dst_node->LocalFlagsInWindows = DockNodeFlags_None;
  dst_node->Pos = src_node->Pos;
  dst_node->Size = src_node->Size;
  dst_node->SizeRef = src_node->SizeRef;
  dst_node->SplitAxis = src_node->SplitAxis;
  dst_node->UpdateMergedFlags();

  out_node_remap_pairs->push_back(src_node->ID);
  out_node_remap_pairs->push_back(dst_node->ID);

  for (int child_n = 0; child_n < ARRAYSIZE(src_node->ChildNodes); child_n++)
    if (src_node->ChildNodes[child_n]) {
      dst_node->ChildNodes[child_n] = DockBuilderCopyNodeRec(
          src_node->ChildNodes[child_n], 0, out_node_remap_pairs);
      dst_node->ChildNodes[child_n]->ParentNode = dst_node;
    }

  DEBUG_LOG_DOCKING("[docking] Fork node %08X -> %08X (%d childs)\n",
                    src_node->ID, dst_node->ID,
                    dst_node->IsSplitNode() ? 2 : 0);
  return dst_node;
}

void Gui::DockBuilderCopyNode(ID src_node_id, ID dst_node_id,
                              Vector<ID> *out_node_remap_pairs) {
  Context &g = *GGui;
  ASSERT(src_node_id != 0);
  ASSERT(dst_node_id != 0);
  ASSERT(out_node_remap_pairs != NULL);

  DockBuilderRemoveNode(dst_node_id);

  DockNode *src_node = DockContextFindNodeByID(&g, src_node_id);
  ASSERT(src_node != NULL);

  out_node_remap_pairs->clear();
  DockBuilderCopyNodeRec(src_node, dst_node_id, out_node_remap_pairs);

  ASSERT((out_node_remap_pairs->Size % 2) == 0);
}

void Gui::DockBuilderCopyWindowSettings(const char *src_name,
                                        const char *dst_name) {
  Window *src_window = FindWindowByName(src_name);
  if (src_window == NULL)
    return;
  if (Window *dst_window = FindWindowByName(dst_name)) {
    dst_window->Pos = src_window->Pos;
    dst_window->Size = src_window->Size;
    dst_window->SizeFull = src_window->SizeFull;
    dst_window->Collapsed = src_window->Collapsed;
  } else {
    WindowSettings *dst_settings = FindWindowSettingsByID(HashStr(dst_name));
    if (!dst_settings)
      dst_settings = CreateNewWindowSettings(dst_name);
    Vec2ih window_pos_2ih = Vec2ih(src_window->Pos);
    if (src_window->ViewportId != 0 &&
        src_window->ViewportId != VIEWPORT_DEFAULT_ID) {
      dst_settings->ViewportPos = window_pos_2ih;
      dst_settings->ViewportId = src_window->ViewportId;
      dst_settings->Pos = Vec2ih(0, 0);
    } else {
      dst_settings->Pos = window_pos_2ih;
    }
    dst_settings->Size = Vec2ih(src_window->SizeFull);
    dst_settings->Collapsed = src_window->Collapsed;
  }
}

// FIXME: Will probably want to change this signature, in particular how the
// window remapping pairs are passed.
void Gui::DockBuilderCopyDockSpace(
    ID src_dockspace_id, ID dst_dockspace_id,
    Vector<const char *> *in_window_remap_pairs) {
  Context &g = *GGui;
  ASSERT(src_dockspace_id != 0);
  ASSERT(dst_dockspace_id != 0);
  ASSERT(in_window_remap_pairs != NULL);
  ASSERT((in_window_remap_pairs->Size % 2) == 0);

  // Duplicate entire dock
  // FIXME: When overwriting dst_dockspace_id, windows that aren't part of our
  // dockspace window class but that are docked in a same node will be split
  // apart, whereas we could attempt to at least keep them together in a new,
  // same floating node.
  Vector<ID> node_remap_pairs;
  DockBuilderCopyNode(src_dockspace_id, dst_dockspace_id, &node_remap_pairs);

  // Attempt to transition all the upcoming windows associated to
  // dst_dockspace_id into the newly created hierarchy of dock nodes (The
  // windows associated to src_dockspace_id are staying in place)
  Vector<ID> src_windows;
  for (int remap_window_n = 0; remap_window_n < in_window_remap_pairs->Size;
       remap_window_n += 2) {
    const char *src_window_name = (*in_window_remap_pairs)[remap_window_n];
    const char *dst_window_name = (*in_window_remap_pairs)[remap_window_n + 1];
    ID src_window_id = HashStr(src_window_name);
    src_windows.push_back(src_window_id);

    // Search in the remapping tables
    ID src_dock_id = 0;
    if (Window *src_window = FindWindowByID(src_window_id))
      src_dock_id = src_window->DockId;
    else if (WindowSettings *src_window_settings =
                 FindWindowSettingsByID(src_window_id))
      src_dock_id = src_window_settings->DockId;
    ID dst_dock_id = 0;
    for (int dock_remap_n = 0; dock_remap_n < node_remap_pairs.Size;
         dock_remap_n += 2)
      if (node_remap_pairs[dock_remap_n] == src_dock_id) {
        dst_dock_id = node_remap_pairs[dock_remap_n + 1];
        // node_remap_pairs[dock_remap_n] = node_remap_pairs[dock_remap_n + 1] =
        // 0; // Clear
        break;
      }

    if (dst_dock_id != 0) {
      // Docked windows gets redocked into the new node hierarchy.
      DEBUG_LOG_DOCKING(
          "[docking] Remap live window '%s' 0x%08X -> '%s' 0x%08X\n",
          src_window_name, src_dock_id, dst_window_name, dst_dock_id);
      DockBuilderDockWindow(dst_window_name, dst_dock_id);
    } else {
      // Floating windows gets their settings transferred (regardless of whether
      // the new window already exist or not) When this is leading to a Copy and
      // not a Move, we would get two overlapping floating windows. Could we
      // possibly dock them together?
      DEBUG_LOG_DOCKING("[docking] Remap window settings '%s' -> '%s'\n",
                        src_window_name, dst_window_name);
      DockBuilderCopyWindowSettings(src_window_name, dst_window_name);
    }
  }

  // Anything else in the source nodes of 'node_remap_pairs' are windows that
  // are not included in the remapping list. Find those windows and move to them
  // to the cloned dock node. This may be optional? Dock those are a second step
  // as undocking would invalidate source dock nodes.
  struct DockRemainingWindowTask {
    Window *Window;
    ID DockId;
    DockRemainingWindowTask(Window *window, ID dock_id) {
      Window = window;
      DockId = dock_id;
    }
  };
  Vector<DockRemainingWindowTask> dock_remaining_windows;
  for (int dock_remap_n = 0; dock_remap_n < node_remap_pairs.Size;
       dock_remap_n += 2)
    if (ID src_dock_id = node_remap_pairs[dock_remap_n]) {
      ID dst_dock_id = node_remap_pairs[dock_remap_n + 1];
      DockNode *node = DockBuilderGetNode(src_dock_id);
      for (int window_n = 0; window_n < node->Windows.Size; window_n++) {
        Window *window = node->Windows[window_n];
        if (src_windows.contains(window->ID))
          continue;

        // Docked windows gets redocked into the new node hierarchy.
        DEBUG_LOG_DOCKING("[docking] Remap window '%s' %08X -> %08X\n",
                          window->Name, src_dock_id, dst_dock_id);
        dock_remaining_windows.push_back(
            DockRemainingWindowTask(window, dst_dock_id));
      }
    }
  for (const DockRemainingWindowTask &task : dock_remaining_windows)
    DockBuilderDockWindow(task.Window->Name, task.DockId);
}

// FIXME-DOCK: This is awkward because in series of split user is likely to
// loose access to its root node.
void Gui::DockBuilderFinish(ID root_id) {
  Context &g = *GGui;
  // DockContextRebuild(&g);
  DockContextBuildAddWindowsToNodes(&g, root_id);
}

//-----------------------------------------------------------------------------
// Docking: Begin/End Support Functions (called from Begin/End)
//-----------------------------------------------------------------------------
// - GetWindowAlwaysWantOwnTabBar()
// - DockContextBindNodeToWindow()
// - BeginDocked()
// - BeginDockableDragDropSource()
// - BeginDockableDragDropTarget()
//-----------------------------------------------------------------------------

bool Gui::GetWindowAlwaysWantOwnTabBar(Window *window) {
  Context &g = *GGui;
  if (g.IO.ConfigDockingAlwaysTabBar || window->WindowClass.DockingAlwaysTabBar)
    if ((window->Flags & (WindowFlags_ChildWindow | WindowFlags_NoTitleBar |
                          WindowFlags_NoDocking)) == 0)
      if (!window->IsFallbackWindow) // We don't support AlwaysTabBar on the
                                     // fallback/implicit window to avoid unused
                                     // dock-node overhead/noise
        return true;
  return false;
}

static DockNode *Gui::DockContextBindNodeToWindow(Context *ctx,
                                                  Window *window) {
  Context &g = *ctx;
  DockNode *node = DockContextFindNodeByID(ctx, window->DockId);
  ASSERT(window->DockNode == NULL);

  // We should not be docking into a split node (SetWindowDock should avoid
  // this)
  if (node && node->IsSplitNode()) {
    DockContextProcessUndockWindow(ctx, window);
    return NULL;
  }

  // Create node
  if (node == NULL) {
    node = DockContextAddNode(ctx, window->DockId);
    node->AuthorityForPos = node->AuthorityForSize =
        node->AuthorityForViewport = DataAuthority_Window;
    node->LastFrameAlive = g.FrameCount;
  }

  // If the node just turned visible and is part of a hierarchy, it doesn't have
  // a Size assigned by DockNodeTreeUpdatePosSize() yet, so we're forcing a
  // Pos/Size update from the first ancestor that is already visible (often it
  // will be the root node). If we don't do this, the window will be assigned a
  // zero-size on its first frame, which won't ideally warm up the layout. This
  // is a little wonky because we don't normally update the Pos/Size of visible
  // node mid-frame.
  if (!node->IsVisible) {
    DockNode *ancestor_node = node;
    while (!ancestor_node->IsVisible && ancestor_node->ParentNode)
      ancestor_node = ancestor_node->ParentNode;
    ASSERT(ancestor_node->Size.x > 0.0f && ancestor_node->Size.y > 0.0f);
    DockNodeUpdateHasCentralNodeChild(DockNodeGetRootNode(ancestor_node));
    DockNodeTreeUpdatePosSize(ancestor_node, ancestor_node->Pos,
                              ancestor_node->Size, node);
  }

  // Add window to node
  bool node_was_visible = node->IsVisible;
  DockNodeAddWindow(node, window, true);
  node->IsVisible = node_was_visible; // Don't mark visible right away (so
                                      // DockContextEndFrame() doesn't render
                                      // it, maybe other side effects? will see)
  ASSERT(node == window->DockNode);
  return node;
}

void Gui::BeginDocked(Window *window, bool *p_open) {
  Context &g = *GGui;

  // Clear fields ahead so most early-out paths don't have to do it
  window->DockIsActive = window->DockNodeIsVisible = window->DockTabIsVisible =
      false;

  const bool auto_dock_node = GetWindowAlwaysWantOwnTabBar(window);
  if (auto_dock_node) {
    if (window->DockId == 0) {
      ASSERT(window->DockNode == NULL);
      window->DockId = DockContextGenNodeID(&g);
    }
  } else {
    // Calling SetNextWindowPos() undock windows by default (by setting
    // PosUndock)
    bool want_undock = false;
    want_undock |= (window->Flags & WindowFlags_NoDocking) != 0;
    want_undock |=
        (g.NextWindowData.Flags & NextWindowDataFlags_HasPos) &&
        (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) &&
        g.NextWindowData.PosUndock;
    if (want_undock) {
      DockContextProcessUndockWindow(&g, window);
      return;
    }
  }

  // Bind to our dock node
  DockNode *node = window->DockNode;
  if (node != NULL)
    ASSERT(window->DockId == node->ID);
  if (window->DockId != 0 && node == NULL) {
    node = DockContextBindNodeToWindow(&g, window);
    if (node == NULL)
      return;
  }

#if 0
    // Undock if the DockNodeFlags_NoDockingInCentralNode got set
    if (node->IsCentralNode && (node->Flags & DockNodeFlags_NoDockingInCentralNode))
    {
        DockContextProcessUndockWindow(ctx, window);
        return;
    }
#endif

  // Undock if our dockspace node disappeared
  // Note how we are testing for LastFrameAlive and NOT LastFrameActive. A
  // DockSpace node can be maintained alive while being inactive with
  // DockNodeFlags_KeepAliveOnly.
  if (node->LastFrameAlive < g.FrameCount) {
    // If the window has been orphaned, transition the docknode to an implicit
    // node processed in DockContextNewFrameUpdateDocking()
    DockNode *root_node = DockNodeGetRootNode(node);
    if (root_node->LastFrameAlive < g.FrameCount)
      DockContextProcessUndockWindow(&g, window);
    else
      window->DockIsActive = true;
    return;
  }

  // Store style overrides
  for (int color_n = 0; color_n < WindowDockStyleCol_COUNT; color_n++)
    window->DockStyle.Colors[color_n] = ColorConvertFloat4ToU32(
        g.Style.Colors[GWindowDockStyleColors[color_n]]);

  // Fast path return. It is common for windows to hold on a persistent DockId
  // but be the only visible window, and never create neither a host window
  // neither a tab bar.
  // FIXME-DOCK: replace ->HostWindow NULL compare with something more explicit
  // (~was initially intended as a first frame test)
  if (node->HostWindow == NULL) {
    if (node->State == DockNodeState_HostWindowHiddenBecauseWindowsAreResizing)
      window->DockIsActive = true;
    if (node->Windows.Size > 1 &&
        window->Appearing) // Only hide appearing window
      DockNodeHideWindowDuringHostWindowCreation(window);
    return;
  }

  // We can have zero-sized nodes (e.g. children of a small-size dockspace)
  ASSERT(node->HostWindow);
  ASSERT(node->IsLeafNode());
  ASSERT(node->Size.x >= 0.0f && node->Size.y >= 0.0f);
  node->State = DockNodeState_HostWindowVisible;

  // Undock if we are submitted earlier than the host window
  if (!(node->MergedFlags & DockNodeFlags_KeepAliveOnly) &&
      window->BeginOrderWithinContext <
          node->HostWindow->BeginOrderWithinContext) {
    DockContextProcessUndockWindow(&g, window);
    return;
  }

  // Position/Size window
  SetNextWindowPos(node->Pos);
  SetNextWindowSize(node->Size);
  g.NextWindowData.PosUndock =
      false; // Cancel implicit undocking of SetNextWindowPos()
  window->DockIsActive = true;
  window->DockNodeIsVisible = true;
  window->DockTabIsVisible = false;
  if (node->MergedFlags & DockNodeFlags_KeepAliveOnly)
    return;

  // When the window is selected we mark it as visible.
  if (node->VisibleWindow == window)
    window->DockTabIsVisible = true;

  // Update window flag
  ASSERT((window->Flags & WindowFlags_ChildWindow) == 0);
  window->Flags |= WindowFlags_ChildWindow | WindowFlags_NoResize;
  window->ChildFlags |= ChildFlags_AlwaysUseWindowPadding;
  if (node->IsHiddenTabBar() || node->IsNoTabBar())
    window->Flags |= WindowFlags_NoTitleBar;
  else
    window->Flags &=
        ~WindowFlags_NoTitleBar; // Clear the NoTitleBar flag in case the
                                 // user set it: confusingly enough we need
                                 // a title bar height so we are correctly
                                 // offset, but it won't be displayed!

  // Save new dock order only if the window has been visible once already
  // This allows multiple windows to be created in the same frame and have their
  // respective dock orders preserved.
  if (node->TabBar && window->WasActive)
    window->DockOrder = (short)DockNodeGetTabOrder(window);

  if ((node->WantCloseAll || node->WantCloseTabId == window->TabId) &&
      p_open != NULL)
    *p_open = false;

  // Update ChildId to allow returning from Child to Parent with Escape
  Window *parent_window = window->DockNode->HostWindow;
  window->ChildId = parent_window->GetID(window->Name);
}

void Gui::BeginDockableDragDropSource(Window *window) {
  Context &g = *GGui;
  ASSERT(g.ActiveId == window->MoveId);
  ASSERT(g.MovingWindow == window);
  ASSERT(g.CurrentWindow == window);

  // 0: Hold SHIFT to disable docking, 1: Hold SHIFT to enable docking.
  if (g.IO.ConfigDockingWithShift != g.IO.KeyShift) {
    // When ConfigDockingWithShift is set, display a tooltip to increase UI
    // affordance. We cannot set for HoveredWindowUnderMovingWindow != NULL
    // here, as it is only valid/useful when drag and drop is already active
    // (because of the 'is_mouse_dragging_with_an_expected_destination' logic in
    // UpdateViewportsNewFrame() function)
    if (g.IO.ConfigDockingWithShift && g.MouseStationaryTimer >= 1.0f &&
        g.ActiveId >= 1.0f)
      SetTooltip("%s", LocalizeGetMsg(LocKey_DockingHoldShiftToDock));
    return;
  }

  g.LastItemData.ID = window->MoveId;
  window = window->RootWindowDockTree;
  ASSERT((window->Flags & WindowFlags_NoDocking) == 0);
  bool is_drag_docking =
      (g.IO.ConfigDockingWithShift) ||
      Rect(0, 0, window->SizeFull.x, GetFrameHeight())
          .Contains(g.ActiveIdClickOffset); // FIXME-DOCKING: Need to make this
                                            // stateful and explicit
  if (is_drag_docking &&
      BeginDragDropSource(DragDropFlags_SourceNoPreviewTooltip |
                          DragDropFlags_SourceNoHoldToOpenOthers |
                          DragDropFlags_SourceAutoExpirePayload)) {
    SetDragDropPayload(PAYLOAD_TYPE_WINDOW, &window, sizeof(window));
    EndDragDropSource();

    // Store style overrides
    for (int color_n = 0; color_n < WindowDockStyleCol_COUNT; color_n++)
      window->DockStyle.Colors[color_n] = ColorConvertFloat4ToU32(
          g.Style.Colors[GWindowDockStyleColors[color_n]]);
  }
}

void Gui::BeginDockableDragDropTarget(Window *window) {
  Context &g = *GGui;

  // ASSERT(window->RootWindowDockTree == window); // May also be a DockSpace
  ASSERT((window->Flags & WindowFlags_NoDocking) == 0);
  if (!g.DragDropActive)
    return;
  // GetForegroundDrawList(window)->AddRect(window->Pos, window->Pos +
  // window->Size, COL32(255, 255, 0, 255));
  if (!BeginDragDropTargetCustom(window->Rect(), window->ID))
    return;

  // Peek into the payload before calling AcceptDragDropPayload() so we can
  // handle overlapping dock nodes with filtering (this is a little unusual
  // pattern, normally most code would call AcceptDragDropPayload directly)
  const Payload *payload = &g.DragDropPayload;
  if (!payload->IsDataType(PAYLOAD_TYPE_WINDOW) ||
      !DockNodeIsDropAllowed(window, *(Window **)payload->Data)) {
    EndDragDropTarget();
    return;
  }

  Window *payload_window = *(Window **)payload->Data;
  if (AcceptDragDropPayload(PAYLOAD_TYPE_WINDOW,
                            DragDropFlags_AcceptBeforeDelivery |
                                DragDropFlags_AcceptNoDrawDefaultRect)) {
    // Select target node
    // (Important: we cannot use g.HoveredDockNode here! Because each of our
    // target node have filters based on payload, each candidate drop target
    // will do its own evaluation)
    bool dock_into_floating_window = false;
    DockNode *node = NULL;
    if (window->DockNodeAsHost) {
      // Cannot assume that node will != NULL even though we passed the
      // rectangle test: it depends on padding/spacing handled by
      // DockNodeTreeFindVisibleNodeByPos().
      node = DockNodeTreeFindVisibleNodeByPos(window->DockNodeAsHost,
                                              g.IO.MousePos);

      // There is an edge case when docking into a dockspace which only has
      // _inactive_ nodes (because none of the windows are active) In this case
      // we need to fallback into any leaf mode, possibly the central node.
      // FIXME-20181220: We should not have to test for IsLeafNode() here but we
      // have another bug to fix first.
      if (node && node->IsDockSpace() && node->IsRootNode())
        node = (node->CentralNode && node->IsLeafNode())
                   ? node->CentralNode
                   : DockNodeTreeFindFallbackLeafNode(node);
    } else {
      if (window->DockNode)
        node = window->DockNode;
      else
        dock_into_floating_window = true; // Dock into a regular window
    }

    const Rect explicit_target_rect =
        (node && node->TabBar && !node->IsHiddenTabBar() && !node->IsNoTabBar())
            ? node->TabBar->BarRect
            : Rect(window->Pos,
                   window->Pos + Vec2(window->Size.x, GetFrameHeight()));
    const bool is_explicit_target =
        g.IO.ConfigDockingWithShift ||
        IsMouseHoveringRect(explicit_target_rect.Min, explicit_target_rect.Max);

    // Preview docking request and find out split direction/ratio
    // const bool do_preview = true;     // Ignore testing for
    // payload->IsPreview() which removes one frame of delay, but breaks
    // overlapping drop targets within the same window.
    const bool do_preview = payload->IsPreview() || payload->IsDelivery();
    if (do_preview && (node != NULL || dock_into_floating_window)) {
      // If we have a non-leaf node it means we are hovering the border of a
      // parent node, in which case only outer markers will appear.
      DockPreviewData split_inner;
      DockPreviewData split_outer;
      DockPreviewData *split_data = &split_inner;
      if (node &&
          (node->ParentNode || node->IsCentralNode() || !node->IsLeafNode()))
        if (DockNode *root_node = DockNodeGetRootNode(node)) {
          DockNodePreviewDockSetup(window, root_node, payload_window, NULL,
                                   &split_outer, is_explicit_target, true);
          if (split_outer.IsSplitDirExplicit)
            split_data = &split_outer;
        }
      if (!node || node->IsLeafNode())
        DockNodePreviewDockSetup(window, node, payload_window, NULL,
                                 &split_inner, is_explicit_target, false);
      if (split_data == &split_outer)
        split_inner.IsDropAllowed = false;

      // Draw inner then outer, so that previewed tab (in inner data) will be
      // behind the outer drop boxes
      DockNodePreviewDockRender(window, node, payload_window, &split_inner);
      DockNodePreviewDockRender(window, node, payload_window, &split_outer);

      // Queue docking request
      if (split_data->IsDropAllowed && payload->IsDelivery())
        DockContextQueueDock(&g, window, split_data->SplitNode, payload_window,
                             split_data->SplitDir, split_data->SplitRatio,
                             split_data == &split_outer);
    }
  }
  EndDragDropTarget();
}

//-----------------------------------------------------------------------------
// Docking: Settings
//-----------------------------------------------------------------------------
// - DockSettingsRenameNodeReferences()
// - DockSettingsRemoveNodeReferences()
// - DockSettingsFindNodeSettings()
// - DockSettingsHandler_ApplyAll()
// - DockSettingsHandler_ReadOpen()
// - DockSettingsHandler_ReadLine()
// - DockSettingsHandler_DockNodeToSettings()
// - DockSettingsHandler_WriteAll()
//-----------------------------------------------------------------------------

static void Gui::DockSettingsRenameNodeReferences(ID old_node_id,
                                                  ID new_node_id) {
  Context &g = *GGui;
  DEBUG_LOG_DOCKING(
      "[docking] DockSettingsRenameNodeReferences: from 0x%08X -> to 0x%08X\n",
      old_node_id, new_node_id);
  for (int window_n = 0; window_n < g.Windows.Size; window_n++) {
    Window *window = g.Windows[window_n];
    if (window->DockId == old_node_id && window->DockNode == NULL)
      window->DockId = new_node_id;
  }
  //// FIXME-OPT: We could remove this loop by storing the index in the map
  for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
       settings = g.SettingsWindows.next_chunk(settings))
    if (settings->DockId == old_node_id)
      settings->DockId = new_node_id;
}

// Remove references stored in WindowSettings to the given
// DockNodeSettings
static void Gui::DockSettingsRemoveNodeReferences(ID *node_ids,
                                                  int node_ids_count) {
  Context &g = *GGui;
  int found = 0;
  //// FIXME-OPT: We could remove this loop by storing the index in the map
  for (WindowSettings *settings = g.SettingsWindows.begin(); settings != NULL;
       settings = g.SettingsWindows.next_chunk(settings))
    for (int node_n = 0; node_n < node_ids_count; node_n++)
      if (settings->DockId == node_ids[node_n]) {
        settings->DockId = 0;
        settings->DockOrder = -1;
        if (++found < node_ids_count)
          break;
        return;
      }
}

static DockNodeSettings *Gui::DockSettingsFindNodeSettings(Context *ctx,
                                                           ID id) {
  // FIXME-OPT
  DockContext *dc = &ctx->DockContext;
  for (int n = 0; n < dc->NodesSettings.Size; n++)
    if (dc->NodesSettings[n].ID == id)
      return &dc->NodesSettings[n];
  return NULL;
}

// Clear settings data
static void Gui::DockSettingsHandler_ClearAll(Context *ctx, SettingsHandler *) {
  DockContext *dc = &ctx->DockContext;
  dc->NodesSettings.clear();
  DockContextClearNodes(ctx, 0, true);
}

// Recreate nodes based on settings data
static void Gui::DockSettingsHandler_ApplyAll(Context *ctx, SettingsHandler *) {
  // Prune settings at boot time only
  DockContext *dc = &ctx->DockContext;
  if (ctx->Windows.Size == 0)
    DockContextPruneUnusedSettingsNodes(ctx);
  DockContextBuildNodesFromSettings(ctx, dc->NodesSettings.Data,
                                    dc->NodesSettings.Size);
  DockContextBuildAddWindowsToNodes(ctx, 0);
}

static void *Gui::DockSettingsHandler_ReadOpen(Context *, SettingsHandler *,
                                               const char *name) {
  if (strcmp(name, "Data") != 0)
    return NULL;
  return (void *)1;
}

static void Gui::DockSettingsHandler_ReadLine(Context *ctx, SettingsHandler *,
                                              void *, const char *line) {
  char c = 0;
  int x = 0, y = 0;
  int r = 0;

  // Parsing, e.g.
  // " DockNode   ID=0x00000001 Pos=383,193 Size=201,322 Split=Y,0.506 "
  // "   DockNode ID=0x00000002 Parent=0x00000001 "
  // Important: this code expect currently fields in a fixed order.
  DockNodeSettings node;
  line = StrSkipBlank(line);
  if (strncmp(line, "DockNode", 8) == 0) {
    line = StrSkipBlank(line + strlen("DockNode"));
  } else if (strncmp(line, "DockSpace", 9) == 0) {
    line = StrSkipBlank(line + strlen("DockSpace"));
    node.Flags |= DockNodeFlags_DockSpace;
  } else
    return;
  if (sscanf(line, "ID=0x%08X%n", &node.ID, &r) == 1) {
    line += r;
  } else
    return;
  if (sscanf(line, " Parent=0x%08X%n", &node.ParentNodeId, &r) == 1) {
    line += r;
    if (node.ParentNodeId == 0)
      return;
  }
  if (sscanf(line, " Window=0x%08X%n", &node.ParentWindowId, &r) == 1) {
    line += r;
    if (node.ParentWindowId == 0)
      return;
  }
  if (node.ParentNodeId == 0) {
    if (sscanf(line, " Pos=%i,%i%n", &x, &y, &r) == 2) {
      line += r;
      node.Pos = Vec2ih((short)x, (short)y);
    } else
      return;
    if (sscanf(line, " Size=%i,%i%n", &x, &y, &r) == 2) {
      line += r;
      node.Size = Vec2ih((short)x, (short)y);
    } else
      return;
  } else {
    if (sscanf(line, " SizeRef=%i,%i%n", &x, &y, &r) == 2) {
      line += r;
      node.SizeRef = Vec2ih((short)x, (short)y);
    }
  }
  if (sscanf(line, " Split=%c%n", &c, &r) == 1) {
    line += r;
    if (c == 'X')
      node.SplitAxis = Axis_X;
    else if (c == 'Y')
      node.SplitAxis = Axis_Y;
  }
  if (sscanf(line, " NoResize=%d%n", &x, &r) == 1) {
    line += r;
    if (x != 0)
      node.Flags |= DockNodeFlags_NoResize;
  }
  if (sscanf(line, " CentralNode=%d%n", &x, &r) == 1) {
    line += r;
    if (x != 0)
      node.Flags |= DockNodeFlags_CentralNode;
  }
  if (sscanf(line, " NoTabBar=%d%n", &x, &r) == 1) {
    line += r;
    if (x != 0)
      node.Flags |= DockNodeFlags_NoTabBar;
  }
  if (sscanf(line, " HiddenTabBar=%d%n", &x, &r) == 1) {
    line += r;
    if (x != 0)
      node.Flags |= DockNodeFlags_HiddenTabBar;
  }
  if (sscanf(line, " NoWindowMenuButton=%d%n", &x, &r) == 1) {
    line += r;
    if (x != 0)
      node.Flags |= DockNodeFlags_NoWindowMenuButton;
  }
  if (sscanf(line, " NoCloseButton=%d%n", &x, &r) == 1) {
    line += r;
    if (x != 0)
      node.Flags |= DockNodeFlags_NoCloseButton;
  }
  if (sscanf(line, " Selected=0x%08X%n", &node.SelectedTabId, &r) == 1) {
    line += r;
  }
  if (node.ParentNodeId != 0)
    if (DockNodeSettings *parent_settings =
            DockSettingsFindNodeSettings(ctx, node.ParentNodeId))
      node.Depth = parent_settings->Depth + 1;
  ctx->DockContext.NodesSettings.push_back(node);
}

static void DockSettingsHandler_DockNodeToSettings(DockContext *dc,
                                                   DockNode *node, int depth) {
  DockNodeSettings node_settings;
  ASSERT(depth < (1 << (sizeof(node_settings.Depth) << 3)));
  node_settings.ID = node->ID;
  node_settings.ParentNodeId = node->ParentNode ? node->ParentNode->ID : 0;
  node_settings.ParentWindowId = (node->IsDockSpace() && node->HostWindow &&
                                  node->HostWindow->ParentWindow)
                                     ? node->HostWindow->ParentWindow->ID
                                     : 0;
  node_settings.SelectedTabId = node->SelectedTabId;
  node_settings.SplitAxis =
      (signed char)(node->IsSplitNode() ? node->SplitAxis : Axis_None);
  node_settings.Depth = (char)depth;
  node_settings.Flags = (node->LocalFlags & DockNodeFlags_SavedFlagsMask_);
  node_settings.Pos = Vec2ih(node->Pos);
  node_settings.Size = Vec2ih(node->Size);
  node_settings.SizeRef = Vec2ih(node->SizeRef);
  dc->NodesSettings.push_back(node_settings);
  if (node->ChildNodes[0])
    DockSettingsHandler_DockNodeToSettings(dc, node->ChildNodes[0], depth + 1);
  if (node->ChildNodes[1])
    DockSettingsHandler_DockNodeToSettings(dc, node->ChildNodes[1], depth + 1);
}

static void Gui::DockSettingsHandler_WriteAll(Context *ctx,
                                              SettingsHandler *handler,
                                              TextBuffer *buf) {
  Context &g = *ctx;
  DockContext *dc = &ctx->DockContext;
  if (!(g.IO.ConfigFlags & ConfigFlags_DockingEnable))
    return;

  // Gather settings data
  // (unlike our windows settings, because nodes are always built we can do a
  // full rewrite of the SettingsNode buffer)
  dc->NodesSettings.resize(0);
  dc->NodesSettings.reserve(dc->Nodes.Data.Size);
  for (int n = 0; n < dc->Nodes.Data.Size; n++)
    if (DockNode *node = (DockNode *)dc->Nodes.Data[n].val_p)
      if (node->IsRootNode())
        DockSettingsHandler_DockNodeToSettings(dc, node, 0);

  int max_depth = 0;
  for (int node_n = 0; node_n < dc->NodesSettings.Size; node_n++)
    max_depth = Max((int)dc->NodesSettings[node_n].Depth, max_depth);

  // Write to text buffer
  buf->appendf("[%s][Data]\n", handler->TypeName);
  for (int node_n = 0; node_n < dc->NodesSettings.Size; node_n++) {
    const int line_start_pos = buf->size();
    (void)line_start_pos;
    const DockNodeSettings *node_settings = &dc->NodesSettings[node_n];
    buf->appendf("%*s%s%*s", node_settings->Depth * 2, "",
                 (node_settings->Flags & DockNodeFlags_DockSpace) ? "DockSpace"
                                                                  : "DockNode ",
                 (max_depth - node_settings->Depth) * 2,
                 ""); // Text align nodes to facilitate looking at .ini file
    buf->appendf(" ID=0x%08X", node_settings->ID);
    if (node_settings->ParentNodeId) {
      buf->appendf(" Parent=0x%08X SizeRef=%d,%d", node_settings->ParentNodeId,
                   node_settings->SizeRef.x, node_settings->SizeRef.y);
    } else {
      if (node_settings->ParentWindowId)
        buf->appendf(" Window=0x%08X", node_settings->ParentWindowId);
      buf->appendf(" Pos=%d,%d Size=%d,%d", node_settings->Pos.x,
                   node_settings->Pos.y, node_settings->Size.x,
                   node_settings->Size.y);
    }
    if (node_settings->SplitAxis != Axis_None)
      buf->appendf(" Split=%c",
                   (node_settings->SplitAxis == Axis_X) ? 'X' : 'Y');
    if (node_settings->Flags & DockNodeFlags_NoResize)
      buf->appendf(" NoResize=1");
    if (node_settings->Flags & DockNodeFlags_CentralNode)
      buf->appendf(" CentralNode=1");
    if (node_settings->Flags & DockNodeFlags_NoTabBar)
      buf->appendf(" NoTabBar=1");
    if (node_settings->Flags & DockNodeFlags_HiddenTabBar)
      buf->appendf(" HiddenTabBar=1");
    if (node_settings->Flags & DockNodeFlags_NoWindowMenuButton)
      buf->appendf(" NoWindowMenuButton=1");
    if (node_settings->Flags & DockNodeFlags_NoCloseButton)
      buf->appendf(" NoCloseButton=1");
    if (node_settings->SelectedTabId)
      buf->appendf(" Selected=0x%08X", node_settings->SelectedTabId);

    // [DEBUG] Include comments in the .ini file to ease debugging (this makes
    // saving slower!)
    if (g.IO.ConfigDebugIniSettings)
      if (DockNode *node = DockContextFindNodeByID(ctx, node_settings->ID)) {
        buf->appendf("%*s", Max(2, (line_start_pos + 92) - buf->size()),
                     ""); // Align everything
        if (node->IsDockSpace() && node->HostWindow &&
            node->HostWindow->ParentWindow)
          buf->appendf(" ; in '%s'", node->HostWindow->ParentWindow->Name);
        // Iterate settings so we can give info about windows that didn't exist
        // during the session.
        int contains_window = 0;
        for (WindowSettings *settings = g.SettingsWindows.begin();
             settings != NULL;
             settings = g.SettingsWindows.next_chunk(settings))
          if (settings->DockId == node_settings->ID) {
            if (contains_window++ == 0)
              buf->appendf(" ; contains ");
            buf->appendf("'%s' ", settings->GetName());
          }
      }

    buf->appendf("\n");
  }
  buf->appendf("\n");
}

//-----------------------------------------------------------------------------
// [SECTION] PLATFORM DEPENDENT HELPERS
//-----------------------------------------------------------------------------

#if defined(_WIN32) && !defined(DISABLE_WIN32_FUNCTIONS) &&                    \
    !defined(DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS)

#ifdef _MSC_VER
#pragma comment(lib, "user32")
#pragma comment(lib, "kernel32")
#endif

// Win32 clipboard implementation
// We use g.ClipboardHandlerData for temporary storage to ensure it is freed on
// Shutdown()
static const char *GetClipboardTextFn_DefaultImpl(void *user_data_ctx) {
  Context &g = *(Context *)user_data_ctx;
  g.ClipboardHandlerData.clear();
  if (!::OpenClipboard(NULL))
    return NULL;
  HANDLE wbuf_handle = ::GetClipboardData(CF_UNICODETEXT);
  if (wbuf_handle == NULL) {
    ::CloseClipboard();
    return NULL;
  }
  if (const WCHAR *wbuf_global = (const WCHAR *)::GlobalLock(wbuf_handle)) {
    int buf_len =
        ::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1, NULL, 0, NULL, NULL);
    g.ClipboardHandlerData.resize(buf_len);
    ::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1,
                          g.ClipboardHandlerData.Data, buf_len, NULL, NULL);
  }
  ::GlobalUnlock(wbuf_handle);
  ::CloseClipboard();
  return g.ClipboardHandlerData.Data;
}

static void SetClipboardTextFn_DefaultImpl(void *, const char *text) {
  if (!::OpenClipboard(NULL))
    return;
  const int wbuf_length = ::MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
  HGLOBAL wbuf_handle =
      ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(WCHAR));
  if (wbuf_handle == NULL) {
    ::CloseClipboard();
    return;
  }
  WCHAR *wbuf_global = (WCHAR *)::GlobalLock(wbuf_handle);
  ::MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf_global, wbuf_length);
  ::GlobalUnlock(wbuf_handle);
  ::EmptyClipboard();
  if (::SetClipboardData(CF_UNICODETEXT, wbuf_handle) == NULL)
    ::GlobalFree(wbuf_handle);
  ::CloseClipboard();
}

#elif defined(__APPLE__) && TARGET_OS_OSX &&                                   \
    defined(ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS)

#include <Carbon/Carbon.h> // Use old API to avoid need for separate .mm file
static PasteboardRef main_clipboard = 0;

// OSX clipboard implementation
// If you enable this you will need to add '-framework ApplicationServices' to
// your linker command-line!
static void SetClipboardTextFn_DefaultImpl(void *, const char *text) {
  if (!main_clipboard)
    PasteboardCreate(kPasteboardClipboard, &main_clipboard);
  PasteboardClear(main_clipboard);
  CFDataRef cf_data =
      CFDataCreate(kCFAllocatorDefault, (const UInt8 *)text, strlen(text));
  if (cf_data) {
    PasteboardPutItemFlavor(main_clipboard, (PasteboardItemID)1,
                            CFSTR("public.utf8-plain-text"), cf_data, 0);
    CFRelease(cf_data);
  }
}

static const char *GetClipboardTextFn_DefaultImpl(void *user_data_ctx) {
  Context &g = *(Context *)user_data_ctx;
  if (!main_clipboard)
    PasteboardCreate(kPasteboardClipboard, &main_clipboard);
  PasteboardSynchronize(main_clipboard);

  ItemCount item_count = 0;
  PasteboardGetItemCount(main_clipboard, &item_count);
  for (ItemCount i = 0; i < item_count; i++) {
    PasteboardItemID item_id = 0;
    PasteboardGetItemIdentifier(main_clipboard, i + 1, &item_id);
    CFArrayRef flavor_type_array = 0;
    PasteboardCopyItemFlavors(main_clipboard, item_id, &flavor_type_array);
    for (CFIndex j = 0, nj = CFArrayGetCount(flavor_type_array); j < nj; j++) {
      CFDataRef cf_data;
      if (PasteboardCopyItemFlavorData(main_clipboard, item_id,
                                       CFSTR("public.utf8-plain-text"),
                                       &cf_data) == noErr) {
        g.ClipboardHandlerData.clear();
        int length = (int)CFDataGetLength(cf_data);
        g.ClipboardHandlerData.resize(length + 1);
        CFDataGetBytes(cf_data, CFRangeMake(0, length),
                       (UInt8 *)g.ClipboardHandlerData.Data);
        g.ClipboardHandlerData[length] = 0;
        CFRelease(cf_data);
        return g.ClipboardHandlerData.Data;
      }
    }
  }
  return NULL;
}

#else

// Local Gui-only clipboard implementation, if user hasn't defined better
// clipboard handlers.
static const char *GetClipboardTextFn_DefaultImpl(void *user_data_ctx) {
  Context &g = *(Context *)user_data_ctx;
  return g.ClipboardHandlerData.empty() ? NULL : g.ClipboardHandlerData.begin();
}

static void SetClipboardTextFn_DefaultImpl(void *user_data_ctx,
                                           const char *text) {
  Context &g = *(Context *)user_data_ctx;
  g.ClipboardHandlerData.clear();
  const char *text_end = text + strlen(text);
  g.ClipboardHandlerData.resize((int)(text_end - text) + 1);
  memcpy(&g.ClipboardHandlerData[0], text, (size_t)(text_end - text));
  g.ClipboardHandlerData[(int)(text_end - text)] = 0;
}

#endif

// Win32 API IME support (for Asian languages, etc.)
#if defined(_WIN32) && !defined(DISABLE_WIN32_FUNCTIONS) &&                    \
    !defined(DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)

#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif

static void SetPlatformImeDataFn_DefaultImpl(Viewport *viewport,
                                             PlatformImeData *data) {
  // Notify OS Input Method Editor of text input position
  HWND hwnd = (HWND)viewport->PlatformHandleRaw;
  if (hwnd == 0)
    return;

  //::ImmAssociateContextEx(hwnd, NULL, data->WantVisible ? IACE_DEFAULT : 0);
  if (HIMC himc = ::ImmGetContext(hwnd)) {
    COMPOSITIONFORM composition_form = {};
    composition_form.ptCurrentPos.x =
        (LONG)(data->InputPos.x - viewport->Pos.x);
    composition_form.ptCurrentPos.y =
        (LONG)(data->InputPos.y - viewport->Pos.y);
    composition_form.dwStyle = CFS_FORCE_POSITION;
    ::ImmSetCompositionWindow(himc, &composition_form);
    CANDIDATEFORM candidate_form = {};
    candidate_form.dwStyle = CFS_CANDIDATEPOS;
    candidate_form.ptCurrentPos.x = (LONG)(data->InputPos.x - viewport->Pos.x);
    candidate_form.ptCurrentPos.y = (LONG)(data->InputPos.y - viewport->Pos.y);
    ::ImmSetCandidateWindow(himc, &candidate_form);
    ::ImmReleaseContext(hwnd, himc);
  }
}

#else

static void SetPlatformImeDataFn_DefaultImpl(Viewport *, PlatformImeData *) {}

#endif

//-----------------------------------------------------------------------------
// [SECTION] METRICS/DEBUGGER WINDOW
//-----------------------------------------------------------------------------
// - RenderViewportThumbnail() [Internal]
// - RenderViewportsThumbnails() [Internal]
// - DebugTextEncoding()
// - MetricsHelpMarker() [Internal]
// - ShowFontAtlas() [Internal]
// - ShowMetricsWindow()
// - DebugNodeColumns() [Internal]
// - DebugNodeDockNode() [Internal]
// - DebugNodeDrawList() [Internal]
// - DebugNodeDrawCmdShowMeshAndBoundingBox() [Internal]
// - DebugNodeFont() [Internal]
// - DebugNodeFontGlyph() [Internal]
// - DebugNodeStorage() [Internal]
// - DebugNodeTabBar() [Internal]
// - DebugNodeViewport() [Internal]
// - DebugNodeWindow() [Internal]
// - DebugNodeWindowSettings() [Internal]
// - DebugNodeWindowsList() [Internal]
// - DebugNodeWindowsListByBeginStackParent() [Internal]
//-----------------------------------------------------------------------------

#ifndef DISABLE_DEBUG_TOOLS

void Gui::DebugRenderViewportThumbnail(DrawList *draw_list, ViewportP *viewport,
                                       const Rect &bb) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  Vec2 scale = bb.GetSize() / viewport->Size;
  Vec2 off = bb.Min - viewport->Pos * scale;
  float alpha_mul =
      (viewport->Flags & ViewportFlags_IsMinimized) ? 0.30f : 1.00f;
  window->DrawList->AddRectFilled(bb.Min, bb.Max,
                                  GetColorU32(Col_Border, alpha_mul * 0.40f));
  for (Window *thumb_window : g.Windows) {
    if (!thumb_window->WasActive ||
        (thumb_window->Flags & WindowFlags_ChildWindow))
      continue;
    if (thumb_window->Viewport != viewport)
      continue;

    Rect thumb_r = thumb_window->Rect();
    Rect title_r = thumb_window->TitleBarRect();
    thumb_r = Rect(Trunc(off + thumb_r.Min * scale),
                   Trunc(off + thumb_r.Max * scale));
    title_r = Rect(Trunc(off + title_r.Min * scale),
                   Trunc(off + Vec2(title_r.Max.x, title_r.Min.y) * scale) +
                       Vec2(0, 5)); // Exaggerate title bar height
    thumb_r.ClipWithFull(bb);
    title_r.ClipWithFull(bb);
    const bool window_is_focused =
        (g.NavWindow && thumb_window->RootWindowForTitleBarHighlight ==
                            g.NavWindow->RootWindowForTitleBarHighlight);
    window->DrawList->AddRectFilled(thumb_r.Min, thumb_r.Max,
                                    GetColorU32(Col_WindowBg, alpha_mul));
    window->DrawList->AddRectFilled(
        title_r.Min, title_r.Max,
        GetColorU32(window_is_focused ? Col_TitleBgActive : Col_TitleBg,
                    alpha_mul));
    window->DrawList->AddRect(thumb_r.Min, thumb_r.Max,
                              GetColorU32(Col_Border, alpha_mul));
    window->DrawList->AddText(g.Font, g.FontSize * 1.0f, title_r.Min,
                              GetColorU32(Col_Text, alpha_mul),
                              thumb_window->Name,
                              FindRenderedTextEnd(thumb_window->Name));
  }
  draw_list->AddRect(bb.Min, bb.Max, GetColorU32(Col_Border, alpha_mul));
}

static void RenderViewportsThumbnails() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  // We don't display full monitor bounds (we could, but it often looks
  // awkward), instead we display just enough to cover all of our viewports.
  float SCALE = 1.0f / 8.0f;
  Rect bb_full(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (ViewportP *viewport : g.Viewports)
    bb_full.Add(viewport->GetMainRect());
  Vec2 p = window->DC.CursorPos;
  Vec2 off = p - bb_full.Min * SCALE;
  for (ViewportP *viewport : g.Viewports) {
    Rect viewport_draw_bb(off + (viewport->Pos) * SCALE,
                          off + (viewport->Pos + viewport->Size) * SCALE);
    Gui::DebugRenderViewportThumbnail(window->DrawList, viewport,
                                      viewport_draw_bb);
  }
  Gui::Dummy(bb_full.GetSize() * SCALE);
}

static int CDECL ViewportComparerByLastFocusedStampCount(const void *lhs,
                                                         const void *rhs) {
  const ViewportP *a = *(const ViewportP *const *)lhs;
  const ViewportP *b = *(const ViewportP *const *)rhs;
  return b->LastFocusedStampCount - a->LastFocusedStampCount;
}

// Draw an arbitrary US keyboard layout to visualize translated keys
void Gui::DebugRenderKeyboardPreview(DrawList *draw_list) {
  const Vec2 key_size = Vec2(35.0f, 35.0f);
  const float key_rounding = 3.0f;
  const Vec2 key_face_size = Vec2(25.0f, 25.0f);
  const Vec2 key_face_pos = Vec2(5.0f, 3.0f);
  const float key_face_rounding = 2.0f;
  const Vec2 key_label_pos = Vec2(7.0f, 4.0f);
  const Vec2 key_step = Vec2(key_size.x - 1.0f, key_size.y - 1.0f);
  const float key_row_offset = 9.0f;

  Vec2 board_min = GetCursorScreenPos();
  Vec2 board_max =
      Vec2(board_min.x + 3 * key_step.x + 2 * key_row_offset + 10.0f,
           board_min.y + 3 * key_step.y + 10.0f);
  Vec2 start_pos = Vec2(board_min.x + 5.0f - key_step.x, board_min.y);

  struct KeyLayoutData {
    int Row, Col;
    const char *Label;
    Key Key;
  };
  const KeyLayoutData keys_to_display[] = {
      {0, 0, "", Key_Tab}, {0, 1, "Q", Key_Q},        {0, 2, "W", Key_W},
      {0, 3, "E", Key_E},  {0, 4, "R", Key_R},        {1, 0, "", Key_CapsLock},
      {1, 1, "A", Key_A},  {1, 2, "S", Key_S},        {1, 3, "D", Key_D},
      {1, 4, "F", Key_F},  {2, 0, "", Key_LeftShift}, {2, 1, "Z", Key_Z},
      {2, 2, "X", Key_X},  {2, 3, "C", Key_C},        {2, 4, "V", Key_V}};

  // Elements rendered manually via DrawList API are not clipped
  // automatically. While not strictly necessary, here IsItemVisible() is used
  // to avoid rendering these shapes when they are out of view.
  Dummy(board_max - board_min);
  if (!IsItemVisible())
    return;
  draw_list->PushClipRect(board_min, board_max, true);
  for (int n = 0; n < ARRAYSIZE(keys_to_display); n++) {
    const KeyLayoutData *key_data = &keys_to_display[n];
    Vec2 key_min = Vec2(start_pos.x + key_data->Col * key_step.x +
                            key_data->Row * key_row_offset,
                        start_pos.y + key_data->Row * key_step.y);
    Vec2 key_max = key_min + key_size;
    draw_list->AddRectFilled(key_min, key_max, COL32(204, 204, 204, 255),
                             key_rounding);
    draw_list->AddRect(key_min, key_max, COL32(24, 24, 24, 255), key_rounding);
    Vec2 face_min =
        Vec2(key_min.x + key_face_pos.x, key_min.y + key_face_pos.y);
    Vec2 face_max =
        Vec2(face_min.x + key_face_size.x, face_min.y + key_face_size.y);
    draw_list->AddRect(face_min, face_max, COL32(193, 193, 193, 255),
                       key_face_rounding, DrawFlags_None, 2.0f);
    draw_list->AddRectFilled(face_min, face_max, COL32(252, 252, 252, 255),
                             key_face_rounding);
    Vec2 label_min =
        Vec2(key_min.x + key_label_pos.x, key_min.y + key_label_pos.y);
    draw_list->AddText(label_min, COL32(64, 64, 64, 255), key_data->Label);
    if (IsKeyDown(key_data->Key))
      draw_list->AddRectFilled(key_min, key_max, COL32(255, 0, 0, 128),
                               key_rounding);
  }
  draw_list->PopClipRect();
}

// Helper tool to diagnose between text encoding issues and font loading issues.
// Pass your UTF-8 string and verify that there are correct.
void Gui::DebugTextEncoding(const char *str) {
  Text("Text: \"%s\"", str);
  if (!BeginTable("##DebugTextEncoding", 4,
                  TableFlags_Borders | TableFlags_RowBg |
                      TableFlags_SizingFixedFit | TableFlags_Resizable))
    return;
  TableSetupColumn("Offset");
  TableSetupColumn("UTF-8");
  TableSetupColumn("Glyph");
  TableSetupColumn("Codepoint");
  TableHeadersRow();
  for (const char *p = str; *p != 0;) {
    unsigned int c;
    const int c_utf8_len = TextCharFromUtf8(&c, p, NULL);
    TableNextColumn();
    Text("%d", (int)(p - str));
    TableNextColumn();
    for (int byte_index = 0; byte_index < c_utf8_len; byte_index++) {
      if (byte_index > 0)
        SameLine();
      Text("0x%02X", (int)(unsigned char)p[byte_index]);
    }
    TableNextColumn();
    if (GetFont()->FindGlyphNoFallback((Wchar)c))
      TextUnformatted(p, p + c_utf8_len);
    else
      TextUnformatted((c == UNICODE_CODEPOINT_INVALID) ? "[invalid]"
                                                       : "[missing]");
    TableNextColumn();
    Text("U+%04X", (int)c);
    p += c_utf8_len;
  }
  EndTable();
}

static void DebugFlashStyleColorStop() {
  Context &g = *GGui;
  if (g.DebugFlashStyleColorIdx != Col_COUNT)
    g.Style.Colors[g.DebugFlashStyleColorIdx] = g.DebugFlashStyleColorBackup;
  g.DebugFlashStyleColorIdx = Col_COUNT;
}

// Flash a given style color for some + inhibit modifications of this color via
// PushStyleColor() calls.
void Gui::DebugFlashStyleColor(Col idx) {
  Context &g = *GGui;
  DebugFlashStyleColorStop();
  g.DebugFlashStyleColorTime = 0.5f;
  g.DebugFlashStyleColorIdx = idx;
  g.DebugFlashStyleColorBackup = g.Style.Colors[idx];
}

void Gui::UpdateDebugToolFlashStyleColor() {
  Context &g = *GGui;
  if (g.DebugFlashStyleColorTime <= 0.0f)
    return;
  ColorConvertHSVtoRGB(cosf(g.DebugFlashStyleColorTime * 6.0f) * 0.5f + 0.5f,
                       0.5f, 0.5f, g.Style.Colors[g.DebugFlashStyleColorIdx].x,
                       g.Style.Colors[g.DebugFlashStyleColorIdx].y,
                       g.Style.Colors[g.DebugFlashStyleColorIdx].z);
  g.Style.Colors[g.DebugFlashStyleColorIdx].w = 1.0f;
  if ((g.DebugFlashStyleColorTime -= g.IO.DeltaTime) <= 0.0f)
    DebugFlashStyleColorStop();
}

// Avoid naming collision with demo.cpp's HelpMarker() for unity builds.
static void MetricsHelpMarker(const char *desc) {
  Gui::TextDisabled("(?)");
  if (Gui::BeginItemTooltip()) {
    Gui::PushTextWrapPos(Gui::GetFontSize() * 35.0f);
    Gui::TextUnformatted(desc);
    Gui::PopTextWrapPos();
    Gui::EndTooltip();
  }
}

// [DEBUG] List fonts in a font atlas and display its texture
void Gui::ShowFontAtlas(FontAtlas *atlas) {
  for (Font *font : atlas->Fonts) {
    PushID(font);
    DebugNodeFont(font);
    PopID();
  }
  if (TreeNode("Font Atlas", "Font Atlas (%dx%d pixels)", atlas->TexWidth,
               atlas->TexHeight)) {
    Context &g = *GGui;
    MetricsConfig *cfg = &g.DebugMetricsConfig;
    Checkbox("Tint with Text Color",
             &cfg->ShowAtlasTintedWithTextColor); // Using text color ensure
                                                  // visibility of core atlas
                                                  // data, but will alter custom
                                                  // colored icons
    Vec4 tint_col = cfg->ShowAtlasTintedWithTextColor
                        ? GetStyleColorVec4(Col_Text)
                        : Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Vec4 border_col = GetStyleColorVec4(Col_Border);
    Image(atlas->TexID, Vec2((float)atlas->TexWidth, (float)atlas->TexHeight),
          Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), tint_col, border_col);
    TreePop();
  }
}

void Gui::ShowMetricsWindow(bool *p_open) {
  Context &g = *GGui;
  IO &io = g.IO;
  MetricsConfig *cfg = &g.DebugMetricsConfig;
  if (cfg->ShowDebugLog)
    ShowDebugLogWindow(&cfg->ShowDebugLog);
  if (cfg->ShowIDStackTool)
    ShowIDStackToolWindow(&cfg->ShowIDStackTool);

  if (!Begin("Gui Metrics/Debugger", p_open) ||
      GetCurrentWindow()->BeginCount > 1) {
    End();
    return;
  }

  // Basic info
  Text("Gui %s", GetVersion());
  Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
       io.Framerate);
  Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices,
       io.MetricsRenderIndices, io.MetricsRenderIndices / 3);
  Text("%d visible windows, %d current allocations", io.MetricsRenderWindows,
       g.DebugAllocInfo.TotalAllocCount - g.DebugAllocInfo.TotalFreeCount);
  // SameLine(); if (SmallButton("GC")) { g.GcCompactAll = true; }

  Separator();

  // Debugging enums
  enum {
    WRT_OuterRect,
    WRT_OuterRectClipped,
    WRT_InnerRect,
    WRT_InnerClipRect,
    WRT_WorkRect,
    WRT_Content,
    WRT_ContentIdeal,
    WRT_ContentRegionRect,
    WRT_Count
  }; // Windows Rect Type
  const char *wrt_rects_names[WRT_Count] = {
      "OuterRect", "OuterRectClipped", "InnerRect",    "InnerClipRect",
      "WorkRect",  "Content",          "ContentIdeal", "ContentRegionRect"};
  enum {
    TRT_OuterRect,
    TRT_InnerRect,
    TRT_WorkRect,
    TRT_HostClipRect,
    TRT_InnerClipRect,
    TRT_BackgroundClipRect,
    TRT_ColumnsRect,
    TRT_ColumnsWorkRect,
    TRT_ColumnsClipRect,
    TRT_ColumnsContentHeadersUsed,
    TRT_ColumnsContentHeadersIdeal,
    TRT_ColumnsContentFrozen,
    TRT_ColumnsContentUnfrozen,
    TRT_Count
  }; // Tables Rect Type
  const char *trt_rects_names[TRT_Count] = {"OuterRect",
                                            "InnerRect",
                                            "WorkRect",
                                            "HostClipRect",
                                            "InnerClipRect",
                                            "BackgroundClipRect",
                                            "ColumnsRect",
                                            "ColumnsWorkRect",
                                            "ColumnsClipRect",
                                            "ColumnsContentHeadersUsed",
                                            "ColumnsContentHeadersIdeal",
                                            "ColumnsContentFrozen",
                                            "ColumnsContentUnfrozen"};
  if (cfg->ShowWindowsRectsType < 0)
    cfg->ShowWindowsRectsType = WRT_WorkRect;
  if (cfg->ShowTablesRectsType < 0)
    cfg->ShowTablesRectsType = TRT_WorkRect;

  struct Funcs {
    static Rect GetTableRect(Table *table, int rect_type, int n) {
      TableInstanceData *table_instance = TableGetInstanceData(
          table,
          table->InstanceCurrent); // Always using last submitted instance
      if (rect_type == TRT_OuterRect) {
        return table->OuterRect;
      } else if (rect_type == TRT_InnerRect) {
        return table->InnerRect;
      } else if (rect_type == TRT_WorkRect) {
        return table->WorkRect;
      } else if (rect_type == TRT_HostClipRect) {
        return table->HostClipRect;
      } else if (rect_type == TRT_InnerClipRect) {
        return table->InnerClipRect;
      } else if (rect_type == TRT_BackgroundClipRect) {
        return table->BgClipRect;
      } else if (rect_type == TRT_ColumnsRect) {
        TableColumn *c = &table->Columns[n];
        return Rect(c->MinX, table->InnerClipRect.Min.y, c->MaxX,
                    table->InnerClipRect.Min.y +
                        table_instance->LastOuterHeight);
      } else if (rect_type == TRT_ColumnsWorkRect) {
        TableColumn *c = &table->Columns[n];
        return Rect(c->WorkMinX, table->WorkRect.Min.y, c->WorkMaxX,
                    table->WorkRect.Max.y);
      } else if (rect_type == TRT_ColumnsClipRect) {
        TableColumn *c = &table->Columns[n];
        return c->ClipRect;
      } else if (rect_type == TRT_ColumnsContentHeadersUsed) {
        TableColumn *c = &table->Columns[n];
        return Rect(c->WorkMinX, table->InnerClipRect.Min.y,
                    c->ContentMaxXHeadersUsed,
                    table->InnerClipRect.Min.y +
                        table_instance->LastTopHeadersRowHeight);
      } // Note: y1/y2 not always accurate
      else if (rect_type == TRT_ColumnsContentHeadersIdeal) {
        TableColumn *c = &table->Columns[n];
        return Rect(c->WorkMinX, table->InnerClipRect.Min.y,
                    c->ContentMaxXHeadersIdeal,
                    table->InnerClipRect.Min.y +
                        table_instance->LastTopHeadersRowHeight);
      } else if (rect_type == TRT_ColumnsContentFrozen) {
        TableColumn *c = &table->Columns[n];
        return Rect(
            c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXFrozen,
            table->InnerClipRect.Min.y + table_instance->LastFrozenHeight);
      } else if (rect_type == TRT_ColumnsContentUnfrozen) {
        TableColumn *c = &table->Columns[n];
        return Rect(c->WorkMinX,
                    table->InnerClipRect.Min.y +
                        table_instance->LastFrozenHeight,
                    c->ContentMaxXUnfrozen, table->InnerClipRect.Max.y);
      }
      ASSERT(0);
      return Rect();
    }

    static Rect GetWindowRect(Window *window, int rect_type) {
      if (rect_type == WRT_OuterRect) {
        return window->Rect();
      } else if (rect_type == WRT_OuterRectClipped) {
        return window->OuterRectClipped;
      } else if (rect_type == WRT_InnerRect) {
        return window->InnerRect;
      } else if (rect_type == WRT_InnerClipRect) {
        return window->InnerClipRect;
      } else if (rect_type == WRT_WorkRect) {
        return window->WorkRect;
      } else if (rect_type == WRT_Content) {
        Vec2 min =
            window->InnerRect.Min - window->Scroll + window->WindowPadding;
        return Rect(min, min + window->ContentSize);
      } else if (rect_type == WRT_ContentIdeal) {
        Vec2 min =
            window->InnerRect.Min - window->Scroll + window->WindowPadding;
        return Rect(min, min + window->ContentSizeIdeal);
      } else if (rect_type == WRT_ContentRegionRect) {
        return window->ContentRegionRect;
      }
      ASSERT(0);
      return Rect();
    }
  };

  // Tools
  if (TreeNode("Tools")) {
    bool show_encoding_viewer = TreeNode("UTF-8 Encoding viewer");
    SameLine();
    MetricsHelpMarker(
        "You can also call Gui::DebugTextEncoding() from your code with a "
        "given string to test that your UTF-8 encoding settings are correct.");
    if (show_encoding_viewer) {
      static char buf[100] = "";
      SetNextItemWidth(-FLT_MIN);
      InputText("##Text", buf, ARRAYSIZE(buf));
      if (buf[0] != 0)
        DebugTextEncoding(buf);
      TreePop();
    }

    // The Item Picker tool is super useful to visually select an item and break
    // into the call-stack of where it was submitted.
    if (Checkbox("Show Item Picker", &g.DebugItemPickerActive) &&
        g.DebugItemPickerActive)
      DebugStartItemPicker();
    SameLine();
    MetricsHelpMarker(
        "Will call the DEBUG_BREAK() macro to break in debugger.\nWarning: "
        "If you don't have a debugger attached, this will probably crash.");

    Checkbox("Show Debug Log", &cfg->ShowDebugLog);
    SameLine();
    MetricsHelpMarker(
        "You can also call Gui::ShowDebugLogWindow() from your code.");

    Checkbox("Show ID Stack Tool", &cfg->ShowIDStackTool);
    SameLine();
    MetricsHelpMarker(
        "You can also call Gui::ShowIDStackToolWindow() from your code.");

    Checkbox("Show windows begin order", &cfg->ShowWindowsBeginOrder);
    Checkbox("Show windows rectangles", &cfg->ShowWindowsRects);
    SameLine();
    SetNextItemWidth(GetFontSize() * 12);
    cfg->ShowWindowsRects |=
        Combo("##show_windows_rect_type", &cfg->ShowWindowsRectsType,
              wrt_rects_names, WRT_Count, WRT_Count);
    if (cfg->ShowWindowsRects && g.NavWindow != NULL) {
      BulletText("'%s':", g.NavWindow->Name);
      Indent();
      for (int rect_n = 0; rect_n < WRT_Count; rect_n++) {
        Rect r = Funcs::GetWindowRect(g.NavWindow, rect_n);
        Text("(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s", r.Min.x,
             r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(),
             wrt_rects_names[rect_n]);
      }
      Unindent();
    }

    Checkbox("Show tables rectangles", &cfg->ShowTablesRects);
    SameLine();
    SetNextItemWidth(GetFontSize() * 12);
    cfg->ShowTablesRects |=
        Combo("##show_table_rects_type", &cfg->ShowTablesRectsType,
              trt_rects_names, TRT_Count, TRT_Count);
    if (cfg->ShowTablesRects && g.NavWindow != NULL) {
      for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++) {
        Table *table = g.Tables.TryGetMapData(table_n);
        if (table == NULL || table->LastFrameActive < g.FrameCount - 1 ||
            (table->OuterWindow != g.NavWindow &&
             table->InnerWindow != g.NavWindow))
          continue;

        BulletText("Table 0x%08X (%d columns, in '%s')", table->ID,
                   table->ColumnsCount, table->OuterWindow->Name);
        if (IsItemHovered())
          GetForegroundDrawList()->AddRect(table->OuterRect.Min - Vec2(1, 1),
                                           table->OuterRect.Max + Vec2(1, 1),
                                           COL32(255, 255, 0, 255), 0.0f, 0,
                                           2.0f);
        Indent();
        char buf[128];
        for (int rect_n = 0; rect_n < TRT_Count; rect_n++) {
          if (rect_n >= TRT_ColumnsRect) {
            if (rect_n != TRT_ColumnsRect && rect_n != TRT_ColumnsClipRect)
              continue;
            for (int column_n = 0; column_n < table->ColumnsCount; column_n++) {
              Rect r = Funcs::GetTableRect(table, rect_n, column_n);
              FormatString(
                  buf, ARRAYSIZE(buf),
                  "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) Col %d %s",
                  r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(),
                  r.GetHeight(), column_n, trt_rects_names[rect_n]);
              Selectable(buf);
              if (IsItemHovered())
                GetForegroundDrawList()->AddRect(
                    r.Min - Vec2(1, 1), r.Max + Vec2(1, 1),
                    COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
            }
          } else {
            Rect r = Funcs::GetTableRect(table, rect_n, -1);
            FormatString(buf, ARRAYSIZE(buf),
                         "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s",
                         r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(),
                         r.GetHeight(), trt_rects_names[rect_n]);
            Selectable(buf);
            if (IsItemHovered())
              GetForegroundDrawList()->AddRect(
                  r.Min - Vec2(1, 1), r.Max + Vec2(1, 1),
                  COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
          }
        }
        Unindent();
      }
    }
    Checkbox(
        "Show groups rectangles",
        &g.DebugShowGroupRects); // Storing in context as this is used by group
                                 // code and prefers to be in hot-data

    Checkbox("Debug Begin/BeginChild return value",
             &io.ConfigDebugBeginReturnValueLoop);
    SameLine();
    MetricsHelpMarker("Some calls to Begin()/BeginChild() will return "
                      "false.\n\nWill cycle through window depths then repeat. "
                      "Windows should be flickering while running.");

    TreePop();
  }

  // Windows
  if (TreeNode("Windows", "Windows (%d)", g.Windows.Size)) {
    // SetNextItemOpen(true, Cond_Once);
    DebugNodeWindowsList(&g.Windows, "By display order");
    DebugNodeWindowsList(&g.WindowsFocusOrder, "By focus order (root windows)");
    if (TreeNode("By submission order (begin stack)")) {
      // Here we display windows in their submitted order/hierarchy, however
      // note that the Begin stack doesn't constitute a Parent<>Child
      // relationship!
      Vector<Window *> &temp_buffer = g.WindowsTempSortBuffer;
      temp_buffer.resize(0);
      for (Window *window : g.Windows)
        if (window->LastFrameActive + 1 >= g.FrameCount)
          temp_buffer.push_back(window);
      struct Func {
        static int CDECL WindowComparerByBeginOrder(const void *lhs,
                                                    const void *rhs) {
          return ((int)(*(const Window *const *)lhs)->BeginOrderWithinContext -
                  (*(const Window *const *)rhs)->BeginOrderWithinContext);
        }
      };
      Qsort(temp_buffer.Data, (size_t)temp_buffer.Size, sizeof(Window *),
            Func::WindowComparerByBeginOrder);
      DebugNodeWindowsListByBeginStackParent(temp_buffer.Data, temp_buffer.Size,
                                             NULL);
      TreePop();
    }

    TreePop();
  }

  // DrawLists
  int drawlist_count = 0;
  for (ViewportP *viewport : g.Viewports)
    drawlist_count += viewport->DrawDataP.CmdLists.Size;
  if (TreeNode("DrawLists", "DrawLists (%d)", drawlist_count)) {
    Checkbox("Show DrawCmd mesh when hovering", &cfg->ShowDrawCmdMesh);
    Checkbox("Show DrawCmd bounding boxes when hovering",
             &cfg->ShowDrawCmdBoundingBoxes);
    for (ViewportP *viewport : g.Viewports) {
      bool viewport_has_drawlist = false;
      for (DrawList *draw_list : viewport->DrawDataP.CmdLists) {
        if (!viewport_has_drawlist)
          Text("Active DrawLists in Viewport #%d, ID: 0x%08X", viewport->Idx,
               viewport->ID);
        viewport_has_drawlist = true;
        DebugNodeDrawList(NULL, viewport, draw_list, "DrawList");
      }
    }
    TreePop();
  }

  // Viewports
  if (TreeNode("Viewports", "Viewports (%d)", g.Viewports.Size)) {
    Indent(GetTreeNodeToLabelSpacing());
    RenderViewportsThumbnails();
    Unindent(GetTreeNodeToLabelSpacing());

    bool open =
        TreeNode("Monitors", "Monitors (%d)", g.PlatformIO.Monitors.Size);
    SameLine();
    MetricsHelpMarker("Gui uses monitor data:\n- to query DPI settings "
                      "on a per monitor basis\n- to position popup/tooltips so "
                      "they don't straddle monitors.");
    if (open) {
      for (int i = 0; i < g.PlatformIO.Monitors.Size; i++) {
        const PlatformMonitor &mon = g.PlatformIO.Monitors[i];
        BulletText(
            "Monitor #%d: DPI %.0f%%\n MainMin (%.0f,%.0f), MainMax "
            "(%.0f,%.0f), MainSize (%.0f,%.0f)\n WorkMin (%.0f,%.0f), WorkMax "
            "(%.0f,%.0f), WorkSize (%.0f,%.0f)",
            i, mon.DpiScale * 100.0f, mon.MainPos.x, mon.MainPos.y,
            mon.MainPos.x + mon.MainSize.x, mon.MainPos.y + mon.MainSize.y,
            mon.MainSize.x, mon.MainSize.y, mon.WorkPos.x, mon.WorkPos.y,
            mon.WorkPos.x + mon.WorkSize.x, mon.WorkPos.y + mon.WorkSize.y,
            mon.WorkSize.x, mon.WorkSize.y);
      }
      TreePop();
    }

    BulletText("MouseViewport: 0x%08X (UserHovered 0x%08X, LastHovered 0x%08X)",
               g.MouseViewport ? g.MouseViewport->ID : 0,
               g.IO.MouseHoveredViewport,
               g.MouseLastHoveredViewport ? g.MouseLastHoveredViewport->ID : 0);
    if (TreeNode("Inferred Z order (front-to-back)")) {
      static Vector<ViewportP *> viewports;
      viewports.resize(g.Viewports.Size);
      memcpy(viewports.Data, g.Viewports.Data, g.Viewports.size_in_bytes());
      if (viewports.Size > 1)
        Qsort(viewports.Data, viewports.Size, sizeof(Viewport *),
              ViewportComparerByLastFocusedStampCount);
      for (ViewportP *viewport : viewports)
        BulletText(
            "Viewport #%d, ID: 0x%08X, LastFocused = %08d, PlatformFocused = "
            "%s, Window: \"%s\"",
            viewport->Idx, viewport->ID, viewport->LastFocusedStampCount,
            (g.PlatformIO.Platform_GetWindowFocus &&
             viewport->PlatformWindowCreated)
                ? (g.PlatformIO.Platform_GetWindowFocus(viewport) ? "1" : "0")
                : "N/A",
            viewport->Window ? viewport->Window->Name : "N/A");
      TreePop();
    }
    for (ViewportP *viewport : g.Viewports)
      DebugNodeViewport(viewport);
    TreePop();
  }

  // Details for Popups
  if (TreeNode("Popups", "Popups (%d)", g.OpenPopupStack.Size)) {
    for (const PopupData &popup_data : g.OpenPopupStack) {
      // As it's difficult to interact with tree nodes while popups are open, we
      // display everything inline.
      Window *window = popup_data.Window;
      BulletText(
          "PopupID: %08x, Window: '%s' (%s%s), BackupNavWindow '%s', "
          "ParentWindow '%s'",
          popup_data.PopupId, window ? window->Name : "NULL",
          window && (window->Flags & WindowFlags_ChildWindow) ? "Child;" : "",
          window && (window->Flags & WindowFlags_ChildMenu) ? "Menu;" : "",
          popup_data.BackupNavWindow ? popup_data.BackupNavWindow->Name
                                     : "NULL",
          window && window->ParentWindow ? window->ParentWindow->Name : "NULL");
    }
    TreePop();
  }

  // Details for TabBars
  if (TreeNode("TabBars", "Tab Bars (%d)", g.TabBars.GetAliveCount())) {
    for (int n = 0; n < g.TabBars.GetMapSize(); n++)
      if (TabBar *tab_bar = g.TabBars.TryGetMapData(n)) {
        PushID(tab_bar);
        DebugNodeTabBar(tab_bar, "TabBar");
        PopID();
      }
    TreePop();
  }

  // Details for Tables
  if (TreeNode("Tables", "Tables (%d)", g.Tables.GetAliveCount())) {
    for (int n = 0; n < g.Tables.GetMapSize(); n++)
      if (Table *table = g.Tables.TryGetMapData(n))
        DebugNodeTable(table);
    TreePop();
  }

  // Details for Fonts
  FontAtlas *atlas = g.IO.Fonts;
  if (TreeNode("Fonts", "Fonts (%d)", atlas->Fonts.Size)) {
    ShowFontAtlas(atlas);
    TreePop();
  }

  // Details for InputText
  if (TreeNode("InputText")) {
    DebugNodeInputTextState(&g.InputTextState);
    TreePop();
  }

  // Details for TypingSelect
  if (TreeNode("TypingSelect", "TypingSelect (%d)",
               g.TypingSelectState.SearchBuffer[0] != 0 ? 1 : 0)) {
    DebugNodeTypingSelectState(&g.TypingSelectState);
    TreePop();
  }

  // Details for Docking
#ifdef HAS_DOCK
  if (TreeNode("Docking")) {
    static bool root_nodes_only = true;
    DockContext *dc = &g.DockContext;
    Checkbox("List root nodes", &root_nodes_only);
    Checkbox("Ctrl shows window dock info", &cfg->ShowDockingNodes);
    if (SmallButton("Clear nodes")) {
      DockContextClearNodes(&g, 0, true);
    }
    SameLine();
    if (SmallButton("Rebuild all")) {
      dc->WantFullRebuild = true;
    }
    for (int n = 0; n < dc->Nodes.Data.Size; n++)
      if (DockNode *node = (DockNode *)dc->Nodes.Data[n].val_p)
        if (!root_nodes_only || node->IsRootNode())
          DebugNodeDockNode(node, "Node");
    TreePop();
  }
#endif // #ifdef HAS_DOCK

  // Settings
  if (TreeNode("Settings")) {
    if (SmallButton("Clear"))
      ClearIniSettings();
    SameLine();
    if (SmallButton("Save to memory"))
      SaveIniSettingsToMemory();
    SameLine();
    if (SmallButton("Save to disk"))
      SaveIniSettingsToDisk(g.IO.IniFilename);
    SameLine();
    if (g.IO.IniFilename)
      Text("\"%s\"", g.IO.IniFilename);
    else
      TextUnformatted("<NULL>");
    Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
    Text("SettingsDirtyTimer %.2f", g.SettingsDirtyTimer);
    if (TreeNode("SettingsHandlers", "Settings handlers: (%d)",
                 g.SettingsHandlers.Size)) {
      for (SettingsHandler &handler : g.SettingsHandlers)
        BulletText("\"%s\"", handler.TypeName);
      TreePop();
    }
    if (TreeNode("SettingsWindows", "Settings packed data: Windows: %d bytes",
                 g.SettingsWindows.size())) {
      for (WindowSettings *settings = g.SettingsWindows.begin();
           settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        DebugNodeWindowSettings(settings);
      TreePop();
    }

    if (TreeNode("SettingsTables", "Settings packed data: Tables: %d bytes",
                 g.SettingsTables.size())) {
      for (TableSettings *settings = g.SettingsTables.begin(); settings != NULL;
           settings = g.SettingsTables.next_chunk(settings))
        DebugNodeTableSettings(settings);
      TreePop();
    }

#ifdef HAS_DOCK
    if (TreeNode("SettingsDocking", "Settings packed data: Docking")) {
      DockContext *dc = &g.DockContext;
      Text("In SettingsWindows:");
      for (WindowSettings *settings = g.SettingsWindows.begin();
           settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        if (settings->DockId != 0)
          BulletText("Window '%s' -> DockId %08X DockOrder=%d",
                     settings->GetName(), settings->DockId,
                     settings->DockOrder);
      Text("In SettingsNodes:");
      for (int n = 0; n < dc->NodesSettings.Size; n++) {
        DockNodeSettings *settings = &dc->NodesSettings[n];
        const char *selected_tab_name = NULL;
        if (settings->SelectedTabId) {
          if (Window *window = FindWindowByID(settings->SelectedTabId))
            selected_tab_name = window->Name;
          else if (WindowSettings *window_settings =
                       FindWindowSettingsByID(settings->SelectedTabId))
            selected_tab_name = window_settings->GetName();
        }
        BulletText("Node %08X, Parent %08X, SelectedTab %08X ('%s')",
                   settings->ID, settings->ParentNodeId,
                   settings->SelectedTabId,
                   selected_tab_name         ? selected_tab_name
                   : settings->SelectedTabId ? "N/A"
                                             : "");
      }
      TreePop();
    }
#endif // #ifdef HAS_DOCK

    if (TreeNode("SettingsIniData", "Settings unpacked data (.ini): %d bytes",
                 g.SettingsIniData.size())) {
      InputTextMultiline("##Ini", (char *)(void *)g.SettingsIniData.c_str(),
                         g.SettingsIniData.Buf.Size,
                         Vec2(-FLT_MIN, GetTextLineHeight() * 20),
                         InputTextFlags_ReadOnly);
      TreePop();
    }
    TreePop();
  }

  // Settings
  if (TreeNode("Memory allocations")) {
    DebugAllocInfo *info = &g.DebugAllocInfo;
    Text("%d current allocations",
         info->TotalAllocCount - info->TotalFreeCount);
    Text("Recent frames with allocations:");
    int buf_size = ARRAYSIZE(info->LastEntriesBuf);
    for (int n = buf_size - 1; n >= 0; n--) {
      DebugAllocEntry *entry =
          &info->LastEntriesBuf[(info->LastEntriesIdx - n + buf_size) %
                                buf_size];
      BulletText("Frame %06d: %+3d ( %2d malloc, %2d free )%s",
                 entry->FrameCount, entry->AllocCount - entry->FreeCount,
                 entry->AllocCount, entry->FreeCount,
                 (n == 0) ? " (most recent)" : "");
    }
    TreePop();
  }

  if (TreeNode("Inputs")) {
    Text("KEYBOARD/GAMEPAD/MOUSE KEYS");
    {
      // We iterate both legacy native range and named Key ranges, which is
      // a little odd but this allows displaying the data for old/new backends.
      // User code should never have to go through such hoops! You can generally
      // iterate between Key_NamedKey_BEGIN and Key_NamedKey_END.
      Indent();
#ifdef DISABLE_OBSOLETE_KEYIO
      struct funcs {
        static bool IsLegacyNativeDupe(Key) { return false; }
      };
#else
      struct funcs {
        static bool IsLegacyNativeDupe(Key key) {
          return key >= 0 && key < 512 && GetIO().KeyMap[key] != -1;
        }
      }; // Hide Native<>Key duplicates when both exists in the array
         // Text("Legacy raw:");      for (Key key = Key_KeysData_OFFSET;
         // key < Key_COUNT; key++) { if (io.KeysDown[key]) { SameLine();
         // Text("\"%s\" %d", GetKeyName(key), key); } }
#endif
      Text("Keys down:");
      for (Key key = Key_KeysData_OFFSET; key < Key_COUNT;
           key = (Key)(key + 1)) {
        if (funcs::IsLegacyNativeDupe(key) || !IsKeyDown(key))
          continue;
        SameLine();
        Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key);
        SameLine();
        Text("(%.02f)", GetKeyData(key)->DownDuration);
      }
      Text("Keys pressed:");
      for (Key key = Key_KeysData_OFFSET; key < Key_COUNT;
           key = (Key)(key + 1)) {
        if (funcs::IsLegacyNativeDupe(key) || !IsKeyPressed(key))
          continue;
        SameLine();
        Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key);
      }
      Text("Keys released:");
      for (Key key = Key_KeysData_OFFSET; key < Key_COUNT;
           key = (Key)(key + 1)) {
        if (funcs::IsLegacyNativeDupe(key) || !IsKeyReleased(key))
          continue;
        SameLine();
        Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key);
      }
      Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "",
           io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "",
           io.KeySuper ? "SUPER " : "");
      Text("Chars queue:");
      for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
        Wchar c = io.InputQueueCharacters[i];
        SameLine();
        Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c);
      } // FIXME: We should convert 'c' to UTF-8 here but the functions are not
        // public.
      DebugRenderKeyboardPreview(GetWindowDrawList());
      Unindent();
    }

    Text("MOUSE STATE");
    {
      Indent();
      if (IsMousePosValid())
        Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
      else
        Text("Mouse pos: <INVALID>");
      Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
      int count = ARRAYSIZE(io.MouseDown);
      Text("Mouse down:");
      for (int i = 0; i < count; i++)
        if (IsMouseDown(i)) {
          SameLine();
          Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
        }
      Text("Mouse clicked:");
      for (int i = 0; i < count; i++)
        if (IsMouseClicked(i)) {
          SameLine();
          Text("b%d (%d)", i, io.MouseClickedCount[i]);
        }
      Text("Mouse released:");
      for (int i = 0; i < count; i++)
        if (IsMouseReleased(i)) {
          SameLine();
          Text("b%d", i);
        }
      Text("Mouse wheel: %.1f", io.MouseWheel);
      Text("MouseStationaryTimer: %.2f", g.MouseStationaryTimer);
      Text("Mouse source: %s", GetMouseSourceName(io.MouseSource));
      Text("Pen Pressure: %.1f", io.PenPressure); // Note: currently unused
      Unindent();
    }

    Text("MOUSE WHEELING");
    {
      Indent();
      Text("WheelingWindow: '%s'",
           g.WheelingWindow ? g.WheelingWindow->Name : "NULL");
      Text("WheelingWindowReleaseTimer: %.2f", g.WheelingWindowReleaseTimer);
      Text("WheelingAxisAvg[] = { %.3f, %.3f }, Main Axis: %s",
           g.WheelingAxisAvg.x, g.WheelingAxisAvg.y,
           (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y)   ? "X"
           : (g.WheelingAxisAvg.x < g.WheelingAxisAvg.y) ? "Y"
                                                         : "<none>");
      Unindent();
    }

    Text("KEY OWNERS");
    {
      Indent();
      if (BeginChild("##owners",
                     Vec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 6),
                     ChildFlags_FrameStyle | ChildFlags_ResizeY,
                     WindowFlags_NoSavedSettings))
        for (Key key = Key_NamedKey_BEGIN; key < Key_NamedKey_END;
             key = (Key)(key + 1)) {
          KeyOwnerData *owner_data = GetKeyOwnerData(&g, key);
          if (owner_data->OwnerCurr == KeyOwner_None)
            continue;
          Text("%s: 0x%08X%s", GetKeyName(key), owner_data->OwnerCurr,
               owner_data->LockUntilRelease ? " LockUntilRelease"
               : owner_data->LockThisFrame  ? " LockThisFrame"
                                            : "");
          DebugLocateItemOnHover(owner_data->OwnerCurr);
        }
      EndChild();
      Unindent();
    }
    Text("SHORTCUT ROUTING");
    {
      Indent();
      if (BeginChild("##routes",
                     Vec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 6),
                     ChildFlags_FrameStyle | ChildFlags_ResizeY,
                     WindowFlags_NoSavedSettings))
        for (Key key = Key_NamedKey_BEGIN; key < Key_NamedKey_END;
             key = (Key)(key + 1)) {
          KeyRoutingTable *rt = &g.KeysRoutingTable;
          for (KeyRoutingIndex idx = rt->Index[key - Key_NamedKey_BEGIN];
               idx != -1;) {
            char key_chord_name[64];
            KeyRoutingData *routing_data = &rt->Entries[idx];
            GetKeyChordName(key | routing_data->Mods, key_chord_name,
                            ARRAYSIZE(key_chord_name));
            Text("%s: 0x%08X", key_chord_name, routing_data->RoutingCurr);
            DebugLocateItemOnHover(routing_data->RoutingCurr);
            idx = routing_data->NextEntryIndex;
          }
        }
      EndChild();
      Text("(ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: 0x%X)",
           g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
      Unindent();
    }
    TreePop();
  }

  if (TreeNode("Internal state")) {
    Text("WINDOWING");
    Indent();
    Text("HoveredWindow: '%s'",
         g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
    Text("HoveredWindow->Root: '%s'",
         g.HoveredWindow ? g.HoveredWindow->RootWindowDockTree->Name : "NULL");
    Text("HoveredWindowUnderMovingWindow: '%s'",
         g.HoveredWindowUnderMovingWindow
             ? g.HoveredWindowUnderMovingWindow->Name
             : "NULL");
    Text("HoveredDockNode: 0x%08X",
         g.DebugHoveredDockNode ? g.DebugHoveredDockNode->ID : 0);
    Text("MovingWindow: '%s'", g.MovingWindow ? g.MovingWindow->Name : "NULL");
    Text("MouseViewport: 0x%08X (UserHovered 0x%08X, LastHovered 0x%08X)",
         g.MouseViewport->ID, g.IO.MouseHoveredViewport,
         g.MouseLastHoveredViewport ? g.MouseLastHoveredViewport->ID : 0);
    Unindent();

    Text("ITEMS");
    Indent();
    Text("ActiveId: 0x%08X/0x%08X (%.2f sec), AllowOverlap: %d, Source: %s",
         g.ActiveId, g.ActiveIdPreviousFrame, g.ActiveIdTimer,
         g.ActiveIdAllowOverlap, GetInputSourceName(g.ActiveIdSource));
    DebugLocateItemOnHover(g.ActiveId);
    Text("ActiveIdWindow: '%s'",
         g.ActiveIdWindow ? g.ActiveIdWindow->Name : "NULL");
    Text("ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: %X",
         g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
    Text("HoveredId: 0x%08X (%.2f sec), AllowOverlap: %d",
         g.HoveredIdPreviousFrame, g.HoveredIdTimer,
         g.HoveredIdAllowOverlap); // Not displaying g.HoveredId as it is update
                                   // mid-frame
    Text("HoverItemDelayId: 0x%08X, Timer: %.2f, ClearTimer: %.2f",
         g.HoverItemDelayId, g.HoverItemDelayTimer, g.HoverItemDelayClearTimer);
    Text("DragDrop: %d, SourceId = 0x%08X, Payload \"%s\" (%d bytes)",
         g.DragDropActive, g.DragDropPayload.SourceId,
         g.DragDropPayload.DataType, g.DragDropPayload.DataSize);
    DebugLocateItemOnHover(g.DragDropPayload.SourceId);
    Unindent();

    Text("NAV,FOCUS");
    Indent();
    Text("NavWindow: '%s'", g.NavWindow ? g.NavWindow->Name : "NULL");
    Text("NavId: 0x%08X, NavLayer: %d", g.NavId, g.NavLayer);
    DebugLocateItemOnHover(g.NavId);
    Text("NavInputSource: %s", GetInputSourceName(g.NavInputSource));
    Text("NavLastValidSelectionUserData = %" PRId64 " (0x%" PRIX64 ")",
         g.NavLastValidSelectionUserData, g.NavLastValidSelectionUserData);
    Text("NavActive: %d, NavVisible: %d", g.IO.NavActive, g.IO.NavVisible);
    Text("NavActivateId/DownId/PressedId: %08X/%08X/%08X", g.NavActivateId,
         g.NavActivateDownId, g.NavActivatePressedId);
    Text("NavActivateFlags: %04X", g.NavActivateFlags);
    Text("NavDisableHighlight: %d, NavDisableMouseHover: %d",
         g.NavDisableHighlight, g.NavDisableMouseHover);
    Text("NavFocusScopeId = 0x%08X", g.NavFocusScopeId);
    Text("NavWindowingTarget: '%s'",
         g.NavWindowingTarget ? g.NavWindowingTarget->Name : "NULL");
    Unindent();

    TreePop();
  }

  // Overlay: Display windows Rectangles and Begin Order
  if (cfg->ShowWindowsRects || cfg->ShowWindowsBeginOrder) {
    for (Window *window : g.Windows) {
      if (!window->WasActive)
        continue;
      DrawList *draw_list = GetForegroundDrawList(window);
      if (cfg->ShowWindowsRects) {
        Rect r = Funcs::GetWindowRect(window, cfg->ShowWindowsRectsType);
        draw_list->AddRect(r.Min, r.Max, COL32(255, 0, 128, 255));
      }
      if (cfg->ShowWindowsBeginOrder &&
          !(window->Flags & WindowFlags_ChildWindow)) {
        char buf[32];
        FormatString(buf, ARRAYSIZE(buf), "%d",
                     window->BeginOrderWithinContext);
        float font_size = GetFontSize();
        draw_list->AddRectFilled(window->Pos,
                                 window->Pos + Vec2(font_size, font_size),
                                 COL32(200, 100, 100, 255));
        draw_list->AddText(window->Pos, COL32(255, 255, 255, 255), buf);
      }
    }
  }

  // Overlay: Display Tables Rectangles
  if (cfg->ShowTablesRects) {
    for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++) {
      Table *table = g.Tables.TryGetMapData(table_n);
      if (table == NULL || table->LastFrameActive < g.FrameCount - 1)
        continue;
      DrawList *draw_list = GetForegroundDrawList(table->OuterWindow);
      if (cfg->ShowTablesRectsType >= TRT_ColumnsRect) {
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++) {
          Rect r =
              Funcs::GetTableRect(table, cfg->ShowTablesRectsType, column_n);
          U32 col = (table->HoveredColumnBody == column_n)
                        ? COL32(255, 255, 128, 255)
                        : COL32(255, 0, 128, 255);
          float thickness =
              (table->HoveredColumnBody == column_n) ? 3.0f : 1.0f;
          draw_list->AddRect(r.Min, r.Max, col, 0.0f, 0, thickness);
        }
      } else {
        Rect r = Funcs::GetTableRect(table, cfg->ShowTablesRectsType, -1);
        draw_list->AddRect(r.Min, r.Max, COL32(255, 0, 128, 255));
      }
    }
  }

#ifdef HAS_DOCK
  // Overlay: Display Docking info
  if (cfg->ShowDockingNodes && g.IO.KeyCtrl && g.DebugHoveredDockNode) {
    char buf[64] = "";
    char *p = buf;
    DockNode *node = g.DebugHoveredDockNode;
    DrawList *overlay_draw_list =
        node->HostWindow ? GetForegroundDrawList(node->HostWindow)
                         : GetForegroundDrawList(GetMainViewport());
    p += FormatString(p, buf + ARRAYSIZE(buf) - p, "DockId: %X%s\n", node->ID,
                      node->IsCentralNode() ? " *CentralNode*" : "");
    p += FormatString(p, buf + ARRAYSIZE(buf) - p, "WindowClass: %08X\n",
                      node->WindowClass.ClassId);
    p += FormatString(p, buf + ARRAYSIZE(buf) - p, "Size: (%.0f, %.0f)\n",
                      node->Size.x, node->Size.y);
    p += FormatString(p, buf + ARRAYSIZE(buf) - p, "SizeRef: (%.0f, %.0f)\n",
                      node->SizeRef.x, node->SizeRef.y);
    int depth = DockNodeGetDepth(node);
    overlay_draw_list->AddRect(node->Pos + Vec2(3, 3) * (float)depth,
                               node->Pos + node->Size -
                                   Vec2(3, 3) * (float)depth,
                               COL32(200, 100, 100, 255));
    Vec2 pos = node->Pos + Vec2(3, 3) * (float)depth;
    overlay_draw_list->AddRectFilled(pos - Vec2(1, 1),
                                     pos + CalcTextSize(buf) + Vec2(1, 1),
                                     COL32(200, 100, 100, 255));
    overlay_draw_list->AddText(NULL, 0.0f, pos, COL32(255, 255, 255, 255), buf);
  }
#endif // #ifdef HAS_DOCK

  End();
}

// [DEBUG] Display contents of Columns
void Gui::DebugNodeColumns(OldColumns *columns) {
  if (!TreeNode((void *)(uintptr_t)columns->ID,
                "Columns Id: 0x%08X, Count: %d, Flags: 0x%04X", columns->ID,
                columns->Count, columns->Flags))
    return;
  BulletText("Width: %.1f (MinX: %.1f, MaxX: %.1f)",
             columns->OffMaxX - columns->OffMinX, columns->OffMinX,
             columns->OffMaxX);
  for (OldColumnData &column : columns->Columns)
    BulletText("Column %02d: OffsetNorm %.3f (= %.1f px)",
               (int)columns->Columns.index_from_ptr(&column), column.OffsetNorm,
               GetColumnOffsetFromNorm(columns, column.OffsetNorm));
  TreePop();
}

static void DebugNodeDockNodeFlags(DockNodeFlags *p_flags, const char *label,
                                   bool enabled) {
  using namespace Gui;
  PushID(label);
  PushStyleVar(StyleVar_FramePadding, Vec2(0.0f, 0.0f));
  Text("%s:", label);
  if (!enabled)
    BeginDisabled();
  CheckboxFlags("NoResize", p_flags, DockNodeFlags_NoResize);
  CheckboxFlags("NoResizeX", p_flags, DockNodeFlags_NoResizeX);
  CheckboxFlags("NoResizeY", p_flags, DockNodeFlags_NoResizeY);
  CheckboxFlags("NoTabBar", p_flags, DockNodeFlags_NoTabBar);
  CheckboxFlags("HiddenTabBar", p_flags, DockNodeFlags_HiddenTabBar);
  CheckboxFlags("NoWindowMenuButton", p_flags,
                DockNodeFlags_NoWindowMenuButton);
  CheckboxFlags("NoCloseButton", p_flags, DockNodeFlags_NoCloseButton);
  CheckboxFlags("NoDocking", p_flags,
                DockNodeFlags_NoDocking); // Multiple flags
  CheckboxFlags("NoDockingSplit", p_flags, DockNodeFlags_NoDockingSplit);
  CheckboxFlags("NoDockingSplitOther", p_flags,
                DockNodeFlags_NoDockingSplitOther);
  CheckboxFlags("NoDockingOver", p_flags, DockNodeFlags_NoDockingOverMe);
  CheckboxFlags("NoDockingOverOther", p_flags,
                DockNodeFlags_NoDockingOverOther);
  CheckboxFlags("NoDockingOverEmpty", p_flags,
                DockNodeFlags_NoDockingOverEmpty);
  CheckboxFlags("NoUndocking", p_flags, DockNodeFlags_NoUndocking);
  if (!enabled)
    EndDisabled();
  PopStyleVar();
  PopID();
}

// [DEBUG] Display contents of DockNode
void Gui::DebugNodeDockNode(DockNode *node, const char *label) {
  Context &g = *GGui;
  const bool is_alive = (g.FrameCount - node->LastFrameAlive <
                         2); // Submitted with DockNodeFlags_KeepAliveOnly
  const bool is_active =
      (g.FrameCount - node->LastFrameActive < 2); // Submitted
  if (!is_alive) {
    PushStyleColor(Col_Text, GetStyleColorVec4(Col_TextDisabled));
  }
  bool open;
  TreeNodeFlags tree_node_flags =
      node->IsFocused ? TreeNodeFlags_Selected : TreeNodeFlags_None;
  if (node->Windows.Size > 0)
    open = TreeNodeEx((void *)(intptr_t)node->ID, tree_node_flags,
                      "%s 0x%04X%s: %d windows (vis: '%s')", label, node->ID,
                      node->IsVisible ? "" : " (hidden)", node->Windows.Size,
                      node->VisibleWindow ? node->VisibleWindow->Name : "NULL");
  else
    open = TreeNodeEx((void *)(intptr_t)node->ID, tree_node_flags,
                      "%s 0x%04X%s: %s (vis: '%s')", label, node->ID,
                      node->IsVisible ? "" : " (hidden)",
                      (node->SplitAxis == Axis_X)   ? "horizontal split"
                      : (node->SplitAxis == Axis_Y) ? "vertical split"
                                                    : "empty",
                      node->VisibleWindow ? node->VisibleWindow->Name : "NULL");
  if (!is_alive) {
    PopStyleColor();
  }
  if (is_active && IsItemHovered())
    if (Window *window =
            node->HostWindow ? node->HostWindow : node->VisibleWindow)
      GetForegroundDrawList(window)->AddRect(node->Pos, node->Pos + node->Size,
                                             COL32(255, 255, 0, 255));
  if (open) {
    ASSERT(node->ChildNodes[0] == NULL ||
           node->ChildNodes[0]->ParentNode == node);
    ASSERT(node->ChildNodes[1] == NULL ||
           node->ChildNodes[1]->ParentNode == node);
    BulletText("Pos (%.0f,%.0f), Size (%.0f, %.0f) Ref (%.0f, %.0f)",
               node->Pos.x, node->Pos.y, node->Size.x, node->Size.y,
               node->SizeRef.x, node->SizeRef.y);
    DebugNodeWindow(node->HostWindow, "HostWindow");
    DebugNodeWindow(node->VisibleWindow, "VisibleWindow");
    BulletText("SelectedTabID: 0x%08X, LastFocusedNodeID: 0x%08X",
               node->SelectedTabId, node->LastFocusedNodeId);
    BulletText("Misc:%s%s%s%s%s%s%s", node->IsDockSpace() ? " IsDockSpace" : "",
               node->IsCentralNode() ? " IsCentralNode" : "",
               is_alive ? " IsAlive" : "", is_active ? " IsActive" : "",
               node->IsFocused ? " IsFocused" : "",
               node->WantLockSizeOnce ? " WantLockSizeOnce" : "",
               node->HasCentralNodeChild ? " HasCentralNodeChild" : "");
    if (TreeNode("flags",
                 "Flags Merged: 0x%04X, Local: 0x%04X, InWindows: 0x%04X, "
                 "Shared: 0x%04X",
                 node->MergedFlags, node->LocalFlags, node->LocalFlagsInWindows,
                 node->SharedFlags)) {
      if (BeginTable("flags", 4)) {
        TableNextColumn();
        DebugNodeDockNodeFlags(&node->MergedFlags, "MergedFlags", false);
        TableNextColumn();
        DebugNodeDockNodeFlags(&node->LocalFlags, "LocalFlags", true);
        TableNextColumn();
        DebugNodeDockNodeFlags(&node->LocalFlagsInWindows,
                               "LocalFlagsInWindows", false);
        TableNextColumn();
        DebugNodeDockNodeFlags(&node->SharedFlags, "SharedFlags", true);
        EndTable();
      }
      TreePop();
    }
    if (node->ParentNode)
      DebugNodeDockNode(node->ParentNode, "ParentNode");
    if (node->ChildNodes[0])
      DebugNodeDockNode(node->ChildNodes[0], "Child[0]");
    if (node->ChildNodes[1])
      DebugNodeDockNode(node->ChildNodes[1], "Child[1]");
    if (node->TabBar)
      DebugNodeTabBar(node->TabBar, "TabBar");
    DebugNodeWindowsList(&node->Windows, "Windows");

    TreePop();
  }
}

static void FormatTextureIDForDebugDisplay(char *buf, int buf_size,
                                           TextureID tex_id) {
  if (sizeof(tex_id) >= sizeof(void *))
    FormatString(buf, buf_size, "0x%p", (void *)*(intptr_t *)(void *)&tex_id);
  else
    FormatString(buf, buf_size, "0x%04X", *(int *)(void *)&tex_id);
}

// [DEBUG] Display contents of DrawList
// Note that both 'window' and 'viewport' may be NULL here. Viewport is
// generally null of destroyed popups which previously owned a viewport.
void Gui::DebugNodeDrawList(Window *window, ViewportP *viewport,
                            const DrawList *draw_list, const char *label) {
  Context &g = *GGui;
  MetricsConfig *cfg = &g.DebugMetricsConfig;
  int cmd_count = draw_list->CmdBuffer.Size;
  if (cmd_count > 0 && draw_list->CmdBuffer.back().ElemCount == 0 &&
      draw_list->CmdBuffer.back().UserCallback == NULL)
    cmd_count--;
  bool node_open =
      TreeNode(draw_list, "%s: '%s' %d vtx, %d indices, %d cmds", label,
               draw_list->_OwnerName ? draw_list->_OwnerName : "",
               draw_list->VtxBuffer.Size, draw_list->IdxBuffer.Size, cmd_count);
  if (draw_list == GetWindowDrawList()) {
    SameLine();
    TextColored(
        Vec4(1.0f, 0.4f, 0.4f, 1.0f),
        "CURRENTLY APPENDING"); // Can't display stats for active draw list! (we
                                // don't have the data double-buffered)
    if (node_open)
      TreePop();
    return;
  }

  DrawList *fg_draw_list =
      viewport ? GetForegroundDrawList(viewport)
               : NULL; // Render additional visuals into the top-most draw list
  if (window && IsItemHovered() && fg_draw_list)
    fg_draw_list->AddRect(window->Pos, window->Pos + window->Size,
                          COL32(255, 255, 0, 255));
  if (!node_open)
    return;

  if (window && !window->WasActive)
    TextDisabled("Warning: owning Window is inactive. This DrawList is not "
                 "being rendered!");

  for (const DrawCmd *pcmd = draw_list->CmdBuffer.Data;
       pcmd < draw_list->CmdBuffer.Data + cmd_count; pcmd++) {
    if (pcmd->UserCallback) {
      BulletText("Callback %p, user_data %p", pcmd->UserCallback,
                 pcmd->UserCallbackData);
      continue;
    }

    char texid_desc[20];
    FormatTextureIDForDebugDisplay(texid_desc, ARRAYSIZE(texid_desc),
                                   pcmd->TextureId);
    char buf[300];
    FormatString(
        buf, ARRAYSIZE(buf),
        "DrawCmd:%5d tris, Tex %s, ClipRect (%4.0f,%4.0f)-(%4.0f,%4.0f)",
        pcmd->ElemCount / 3, texid_desc, pcmd->ClipRect.x, pcmd->ClipRect.y,
        pcmd->ClipRect.z, pcmd->ClipRect.w);
    bool pcmd_node_open =
        TreeNode((void *)(pcmd - draw_list->CmdBuffer.begin()), "%s", buf);
    if (IsItemHovered() &&
        (cfg->ShowDrawCmdMesh || cfg->ShowDrawCmdBoundingBoxes) && fg_draw_list)
      DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd,
                                             cfg->ShowDrawCmdMesh,
                                             cfg->ShowDrawCmdBoundingBoxes);
    if (!pcmd_node_open)
      continue;

    // Calculate approximate coverage area (touched pixel count)
    // This will be in pixels squared as long there's no post-scaling happening
    // to the renderer output.
    const DrawIdx *idx_buffer =
        (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL;
    const DrawVert *vtx_buffer = draw_list->VtxBuffer.Data + pcmd->VtxOffset;
    float total_area = 0.0f;
    for (unsigned int idx_n = pcmd->IdxOffset;
         idx_n < pcmd->IdxOffset + pcmd->ElemCount;) {
      Vec2 triangle[3];
      for (int n = 0; n < 3; n++, idx_n++)
        triangle[n] = vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos;
      total_area += TriangleArea(triangle[0], triangle[1], triangle[2]);
    }

    // Display vertex information summary. Hover to get all triangles drawn in
    // wire-frame
    FormatString(
        buf, ARRAYSIZE(buf),
        "Mesh: ElemCount: %d, VtxOffset: +%d, IdxOffset: +%d, Area: ~%0.f px",
        pcmd->ElemCount, pcmd->VtxOffset, pcmd->IdxOffset, total_area);
    Selectable(buf);
    if (IsItemHovered() && fg_draw_list)
      DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd,
                                             true, false);

    // Display individual triangles/vertices. Hover on to get the corresponding
    // triangle highlighted.
    ListClipper clipper;
    clipper.Begin(pcmd->ElemCount /
                  3); // Manually coarse clip our print out of individual
                      // vertices to save CPU, only items that may be visible.
    while (clipper.Step())
      for (int prim = clipper.DisplayStart,
               idx_i = pcmd->IdxOffset + clipper.DisplayStart * 3;
           prim < clipper.DisplayEnd; prim++) {
        char *buf_p = buf, *buf_end = buf + ARRAYSIZE(buf);
        Vec2 triangle[3];
        for (int n = 0; n < 3; n++, idx_i++) {
          const DrawVert &v =
              vtx_buffer[idx_buffer ? idx_buffer[idx_i] : idx_i];
          triangle[n] = v.pos;
          buf_p += FormatString(
              buf_p, buf_end - buf_p,
              "%s %04d: pos (%8.2f,%8.2f), uv (%.6f,%.6f), col %08X\n",
              (n == 0) ? "Vert:" : "     ", idx_i, v.pos.x, v.pos.y, v.uv.x,
              v.uv.y, v.col);
        }

        Selectable(buf, false);
        if (fg_draw_list && IsItemHovered()) {
          DrawListFlags backup_flags = fg_draw_list->Flags;
          fg_draw_list->Flags &=
              ~DrawListFlags_AntiAliasedLines; // Disable AA on triangle
                                               // outlines is more readable
                                               // for very large and thin
                                               // triangles.
          fg_draw_list->AddPolyline(triangle, 3, COL32(255, 255, 0, 255),
                                    DrawFlags_Closed, 1.0f);
          fg_draw_list->Flags = backup_flags;
        }
      }
    TreePop();
  }
  TreePop();
}

// [DEBUG] Display mesh/aabb of a DrawCmd
void Gui::DebugNodeDrawCmdShowMeshAndBoundingBox(DrawList *out_draw_list,
                                                 const DrawList *draw_list,
                                                 const DrawCmd *draw_cmd,
                                                 bool show_mesh,
                                                 bool show_aabb) {
  ASSERT(show_mesh || show_aabb);

  // Draw wire-frame version of all triangles
  Rect clip_rect = draw_cmd->ClipRect;
  Rect vtxs_rect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
  DrawListFlags backup_flags = out_draw_list->Flags;
  out_draw_list->Flags &=
      ~DrawListFlags_AntiAliasedLines; // Disable AA on triangle outlines is
                                       // more readable for very large and
                                       // thin triangles.
  for (unsigned int idx_n = draw_cmd->IdxOffset,
                    idx_end = draw_cmd->IdxOffset + draw_cmd->ElemCount;
       idx_n < idx_end;) {
    DrawIdx *idx_buffer =
        (draw_list->IdxBuffer.Size > 0)
            ? draw_list->IdxBuffer.Data
            : NULL; // We don't hold on those pointers past iterations as
                    // ->AddPolyline() may invalidate them if
                    // out_draw_list==draw_list
    DrawVert *vtx_buffer = draw_list->VtxBuffer.Data + draw_cmd->VtxOffset;

    Vec2 triangle[3];
    for (int n = 0; n < 3; n++, idx_n++)
      vtxs_rect.Add(
          (triangle[n] =
               vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos));
    if (show_mesh)
      out_draw_list->AddPolyline(triangle, 3, COL32(255, 255, 0, 255),
                                 DrawFlags_Closed,
                                 1.0f); // In yellow: mesh triangles
  }
  // Draw bounding boxes
  if (show_aabb) {
    out_draw_list->AddRect(
        Trunc(clip_rect.Min), Trunc(clip_rect.Max),
        COL32(255, 0, 255,
              255)); // In pink: clipping rectangle submitted to GPU
    out_draw_list->AddRect(
        Trunc(vtxs_rect.Min), Trunc(vtxs_rect.Max),
        COL32(0, 255, 255, 255)); // In cyan: bounding box of triangles
  }
  out_draw_list->Flags = backup_flags;
}

// [DEBUG] Display details for a single font, called by ShowStyleEditor().
void Gui::DebugNodeFont(Font *font) {
  bool opened =
      TreeNode(font, "Font: \"%s\"\n%.2f px, %d glyphs, %d file(s)",
               font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize,
               font->Glyphs.Size, font->ConfigDataCount);
  SameLine();
  if (SmallButton("Set as default"))
    GetIO().FontDefault = font;
  if (!opened)
    return;

  // Display preview text
  PushFont(font);
  Text("The quick brown fox jumps over the lazy dog");
  PopFont();

  // Display details
  SetNextItemWidth(GetFontSize() * 8);
  DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");
  SameLine();
  MetricsHelpMarker(
      "Note than the default embedded font is NOT meant to be scaled.\n\n"
      "Font are currently rendered into bitmaps at a given size at the time of "
      "building the atlas. "
      "You may oversample them to get some flexibility with scaling. "
      "You can also render at multiple sizes and select which one to use at "
      "runtime.\n\n"
      "(Glimmer of hope: the atlas system will be rewritten in the future to "
      "make scaling more flexible.)");
  Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent,
       font->Ascent - font->Descent);
  char c_str[5];
  Text("Fallback character: '%s' (U+%04X)",
       TextCharToUtf8(c_str, font->FallbackChar), font->FallbackChar);
  Text("Ellipsis character: '%s' (U+%04X)",
       TextCharToUtf8(c_str, font->EllipsisChar), font->EllipsisChar);
  const int surface_sqrt = (int)Sqrt((float)font->MetricsTotalSurface);
  Text("Texture Area: about %d px ~%dx%d px", font->MetricsTotalSurface,
       surface_sqrt, surface_sqrt);
  for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
    if (font->ConfigData)
      if (const FontConfig *cfg = &font->ConfigData[config_i])
        BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d, "
                   "Offset: (%.1f,%.1f)",
                   config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV,
                   cfg->PixelSnapH, cfg->GlyphOffset.x, cfg->GlyphOffset.y);

  // Display all glyphs of the fonts in separate pages of 256 characters
  if (TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size)) {
    DrawList *draw_list = GetWindowDrawList();
    const U32 glyph_col = GetColorU32(Col_Text);
    const float cell_size = font->FontSize * 1;
    const float cell_spacing = GetStyle().ItemSpacing.y;
    for (unsigned int base = 0; base <= UNICODE_CODEPOINT_MAX; base += 256) {
      // Skip ahead if a large bunch of glyphs are not present in the font (test
      // in chunks of 4k) This is only a small optimization to reduce the number
      // of iterations when UNICODE_MAX_CODEPOINT is large // (if
      // Wchar==Wchar32 we will do at least about 272 queries here)
      if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095)) {
        base += 4096 - 256;
        continue;
      }

      int count = 0;
      for (unsigned int n = 0; n < 256; n++)
        if (font->FindGlyphNoFallback((Wchar)(base + n)))
          count++;
      if (count <= 0)
        continue;
      if (!TreeNode((void *)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base,
                    base + 255, count, count > 1 ? "glyphs" : "glyph"))
        continue;

      // Draw a 16x16 grid of glyphs
      Vec2 base_pos = GetCursorScreenPos();
      for (unsigned int n = 0; n < 256; n++) {
        // We use Font::RenderChar as a shortcut because we don't have UTF-8
        // conversion functions available here and thus cannot easily generate a
        // zero-terminated UTF-8 encoded string.
        Vec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing),
                     base_pos.y + (n / 16) * (cell_size + cell_spacing));
        Vec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
        const FontGlyph *glyph = font->FindGlyphNoFallback((Wchar)(base + n));
        draw_list->AddRect(cell_p1, cell_p2,
                           glyph ? COL32(255, 255, 255, 100)
                                 : COL32(255, 255, 255, 50));
        if (!glyph)
          continue;
        font->RenderChar(draw_list, cell_size, cell_p1, glyph_col,
                         (Wchar)(base + n));
        if (IsMouseHoveringRect(cell_p1, cell_p2) && BeginTooltip()) {
          DebugNodeFontGlyph(font, glyph);
          EndTooltip();
        }
      }
      Dummy(Vec2((cell_size + cell_spacing) * 16,
                 (cell_size + cell_spacing) * 16));
      TreePop();
    }
    TreePop();
  }
  TreePop();
}

void Gui::DebugNodeFontGlyph(Font *, const FontGlyph *glyph) {
  Text("Codepoint: U+%04X", glyph->Codepoint);
  Separator();
  Text("Visible: %d", glyph->Visible);
  Text("AdvanceX: %.1f", glyph->AdvanceX);
  Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1,
       glyph->Y1);
  Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1,
       glyph->V1);
}

// [DEBUG] Display contents of Storage
void Gui::DebugNodeStorage(Storage *storage, const char *label) {
  if (!TreeNode(label, "%s: %d entries, %d bytes", label, storage->Data.Size,
                storage->Data.size_in_bytes()))
    return;
  for (const Storage::StoragePair &p : storage->Data)
    BulletText("Key 0x%08X Value { i: %d }", p.key,
               p.val_i); // Important: we currently don't store a type, real
                         // value may not be integer.
  TreePop();
}

// [DEBUG] Display contents of TabBar
void Gui::DebugNodeTabBar(TabBar *tab_bar, const char *label) {
  // Standalone tab bars (not associated to docking/windows functionality)
  // currently hold no discernible strings.
  char buf[256];
  char *p = buf;
  const char *buf_end = buf + ARRAYSIZE(buf);
  const bool is_active = (tab_bar->PrevFrameVisible >= GetFrameCount() - 2);
  p += FormatString(p, buf_end - p, "%s 0x%08X (%d tabs)%s  {", label,
                    tab_bar->ID, tab_bar->Tabs.Size,
                    is_active ? "" : " *Inactive*");
  for (int tab_n = 0; tab_n < Min(tab_bar->Tabs.Size, 3); tab_n++) {
    TabItem *tab = &tab_bar->Tabs[tab_n];
    p += FormatString(p, buf_end - p, "%s'%s'", tab_n > 0 ? ", " : "",
                      TabBarGetTabName(tab_bar, tab));
  }
  p +=
      FormatString(p, buf_end - p, (tab_bar->Tabs.Size > 3) ? " ... }" : " } ");
  if (!is_active) {
    PushStyleColor(Col_Text, GetStyleColorVec4(Col_TextDisabled));
  }
  bool open = TreeNode(label, "%s", buf);
  if (!is_active) {
    PopStyleColor();
  }
  if (is_active && IsItemHovered()) {
    DrawList *draw_list = GetForegroundDrawList();
    draw_list->AddRect(tab_bar->BarRect.Min, tab_bar->BarRect.Max,
                       COL32(255, 255, 0, 255));
    draw_list->AddLine(Vec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Min.y),
                       Vec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Max.y),
                       COL32(0, 255, 0, 255));
    draw_list->AddLine(Vec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Min.y),
                       Vec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Max.y),
                       COL32(0, 255, 0, 255));
  }
  if (open) {
    for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++) {
      TabItem *tab = &tab_bar->Tabs[tab_n];
      PushID(tab);
      if (SmallButton("<")) {
        TabBarQueueReorder(tab_bar, tab, -1);
      }
      SameLine(0, 2);
      if (SmallButton(">")) {
        TabBarQueueReorder(tab_bar, tab, +1);
      }
      SameLine();
      Text("%02d%c Tab 0x%08X '%s' Offset: %.2f, Width: %.2f/%.2f", tab_n,
           (tab->ID == tab_bar->SelectedTabId) ? '*' : ' ', tab->ID,
           TabBarGetTabName(tab_bar, tab), tab->Offset, tab->Width,
           tab->ContentWidth);
      PopID();
    }
    TreePop();
  }
}

void Gui::DebugNodeViewport(ViewportP *viewport) {
  SetNextItemOpen(true, Cond_Once);
  if (TreeNode((void *)(intptr_t)viewport->ID,
               "Viewport #%d, ID: 0x%08X, Parent: 0x%08X, Window: \"%s\"",
               viewport->Idx, viewport->ID, viewport->ParentViewportId,
               viewport->Window ? viewport->Window->Name : "N/A")) {
    WindowFlags flags = viewport->Flags;
    BulletText(
        "Main Pos: (%.0f,%.0f), Size: (%.0f,%.0f)\nWorkArea Offset Left: %.0f "
        "Top: %.0f, Right: %.0f, Bottom: %.0f\nMonitor: %d, DpiScale: %.0f%%",
        viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y,
        viewport->WorkOffsetMin.x, viewport->WorkOffsetMin.y,
        viewport->WorkOffsetMax.x, viewport->WorkOffsetMax.y,
        viewport->PlatformMonitor, viewport->DpiScale * 100.0f);
    if (viewport->Idx > 0) {
      SameLine();
      if (SmallButton("Reset Pos")) {
        viewport->Pos = Vec2(200, 200);
        viewport->UpdateWorkRect();
        if (viewport->Window)
          viewport->Window->Pos = viewport->Pos;
      }
    }
    BulletText(
        "Flags: 0x%04X =%s%s%s%s%s%s%s%s%s%s%s%s%s", viewport->Flags,
        //(flags & ViewportFlags_IsPlatformWindow) ? " IsPlatformWindow" :
        //"", // Omitting because it is the standard
        (flags & ViewportFlags_IsPlatformMonitor) ? " IsPlatformMonitor" : "",
        (flags & ViewportFlags_IsMinimized) ? " IsMinimized" : "",
        (flags & ViewportFlags_IsFocused) ? " IsFocused" : "",
        (flags & ViewportFlags_OwnedByApp) ? " OwnedByApp" : "",
        (flags & ViewportFlags_NoDecoration) ? " NoDecoration" : "",
        (flags & ViewportFlags_NoTaskBarIcon) ? " NoTaskBarIcon" : "",
        (flags & ViewportFlags_NoFocusOnAppearing) ? " NoFocusOnAppearing" : "",
        (flags & ViewportFlags_NoFocusOnClick) ? " NoFocusOnClick" : "",
        (flags & ViewportFlags_NoInputs) ? " NoInputs" : "",
        (flags & ViewportFlags_NoRendererClear) ? " NoRendererClear" : "",
        (flags & ViewportFlags_NoAutoMerge) ? " NoAutoMerge" : "",
        (flags & ViewportFlags_TopMost) ? " TopMost" : "",
        (flags & ViewportFlags_CanHostOtherWindows) ? " CanHostOtherWindows"
                                                    : "");
    for (DrawList *draw_list : viewport->DrawDataP.CmdLists)
      DebugNodeDrawList(NULL, viewport, draw_list, "DrawList");
    TreePop();
  }
}

void Gui::DebugNodeWindow(Window *window, const char *label) {
  if (window == NULL) {
    BulletText("%s: NULL", label);
    return;
  }

  Context &g = *GGui;
  const bool is_active = window->WasActive;
  TreeNodeFlags tree_node_flags =
      (window == g.NavWindow) ? TreeNodeFlags_Selected : TreeNodeFlags_None;
  if (!is_active) {
    PushStyleColor(Col_Text, GetStyleColorVec4(Col_TextDisabled));
  }
  const bool open = TreeNodeEx(label, tree_node_flags, "%s '%s'%s", label,
                               window->Name, is_active ? "" : " *Inactive*");
  if (!is_active) {
    PopStyleColor();
  }
  if (IsItemHovered() && is_active)
    GetForegroundDrawList(window)->AddRect(
        window->Pos, window->Pos + window->Size, COL32(255, 255, 0, 255));
  if (!open)
    return;

  if (window->MemoryCompacted)
    TextDisabled("Note: some memory buffers have been compacted/freed.");

  WindowFlags flags = window->Flags;
  DebugNodeDrawList(window, window->Viewport, window->DrawList, "DrawList");
  BulletText("Pos: (%.1f,%.1f), Size: (%.1f,%.1f), ContentSize (%.1f,%.1f) "
             "Ideal (%.1f,%.1f)",
             window->Pos.x, window->Pos.y, window->Size.x, window->Size.y,
             window->ContentSize.x, window->ContentSize.y,
             window->ContentSizeIdeal.x, window->ContentSizeIdeal.y);
  BulletText("Flags: 0x%08X (%s%s%s%s%s%s%s%s%s..)", flags,
             (flags & WindowFlags_ChildWindow) ? "Child " : "",
             (flags & WindowFlags_Tooltip) ? "Tooltip " : "",
             (flags & WindowFlags_Popup) ? "Popup " : "",
             (flags & WindowFlags_Modal) ? "Modal " : "",
             (flags & WindowFlags_ChildMenu) ? "ChildMenu " : "",
             (flags & WindowFlags_NoSavedSettings) ? "NoSavedSettings " : "",
             (flags & WindowFlags_NoMouseInputs) ? "NoMouseInputs" : "",
             (flags & WindowFlags_NoNavInputs) ? "NoNavInputs" : "",
             (flags & WindowFlags_AlwaysAutoResize) ? "AlwaysAutoResize" : "");
  BulletText("WindowClassId: 0x%08X", window->WindowClass.ClassId);
  BulletText("Scroll: (%.2f/%.2f,%.2f/%.2f) Scrollbar:%s%s", window->Scroll.x,
             window->ScrollMax.x, window->Scroll.y, window->ScrollMax.y,
             window->ScrollbarX ? "X" : "", window->ScrollbarY ? "Y" : "");
  BulletText("Active: %d/%d, WriteAccessed: %d, BeginOrderWithinContext: %d",
             window->Active, window->WasActive, window->WriteAccessed,
             (window->Active || window->WasActive)
                 ? window->BeginOrderWithinContext
                 : -1);
  BulletText("Appearing: %d, Hidden: %d (CanSkip %d Cannot %d), SkipItems: %d",
             window->Appearing, window->Hidden,
             window->HiddenFramesCanSkipItems,
             window->HiddenFramesCannotSkipItems, window->SkipItems);
  for (int layer = 0; layer < NavLayer_COUNT; layer++) {
    Rect r = window->NavRectRel[layer];
    if (r.Min.x >= r.Max.y && r.Min.y >= r.Max.y)
      BulletText("NavLastIds[%d]: 0x%08X", layer, window->NavLastIds[layer]);
    else
      BulletText("NavLastIds[%d]: 0x%08X at +(%.1f,%.1f)(%.1f,%.1f)", layer,
                 window->NavLastIds[layer], r.Min.x, r.Min.y, r.Max.x, r.Max.y);
    DebugLocateItemOnHover(window->NavLastIds[layer]);
  }
  const Vec2 *pr = window->NavPreferredScoringPosRel;
  for (int layer = 0; layer < NavLayer_COUNT; layer++)
    BulletText("NavPreferredScoringPosRel[%d] = {%.1f,%.1f)", layer,
               (pr[layer].x == FLT_MAX ? -99999.0f : pr[layer].x),
               (pr[layer].y == FLT_MAX
                    ? -99999.0f
                    : pr[layer].y)); // Display as 99999.0f so it looks neater.
  BulletText("NavLayersActiveMask: %X, NavLastChildNavWindow: %s",
             window->DC.NavLayersActiveMask,
             window->NavLastChildNavWindow ? window->NavLastChildNavWindow->Name
                                           : "NULL");

  BulletText("Viewport: %d%s, ViewportId: 0x%08X, ViewportPos: (%.1f,%.1f)",
             window->Viewport ? window->Viewport->Idx : -1,
             window->ViewportOwned ? " (Owned)" : "", window->ViewportId,
             window->ViewportPos.x, window->ViewportPos.y);
  BulletText("ViewportMonitor: %d",
             window->Viewport ? window->Viewport->PlatformMonitor : -1);
  BulletText("DockId: 0x%04X, DockOrder: %d, Act: %d, Vis: %d", window->DockId,
             window->DockOrder, window->DockIsActive, window->DockTabIsVisible);
  if (window->DockNode || window->DockNodeAsHost)
    DebugNodeDockNode(window->DockNodeAsHost ? window->DockNodeAsHost
                                             : window->DockNode,
                      window->DockNodeAsHost ? "DockNodeAsHost" : "DockNode");

  if (window->RootWindow != window) {
    DebugNodeWindow(window->RootWindow, "RootWindow");
  }
  if (window->RootWindowDockTree != window->RootWindow) {
    DebugNodeWindow(window->RootWindowDockTree, "RootWindowDockTree");
  }
  if (window->ParentWindow != NULL) {
    DebugNodeWindow(window->ParentWindow, "ParentWindow");
  }
  if (window->DC.ChildWindows.Size > 0) {
    DebugNodeWindowsList(&window->DC.ChildWindows, "ChildWindows");
  }
  if (window->ColumnsStorage.Size > 0 &&
      TreeNode("Columns", "Columns sets (%d)", window->ColumnsStorage.Size)) {
    for (OldColumns &columns : window->ColumnsStorage)
      DebugNodeColumns(&columns);
    TreePop();
  }
  DebugNodeStorage(&window->StateStorage, "Storage");
  TreePop();
}

void Gui::DebugNodeWindowSettings(WindowSettings *settings) {
  if (settings->WantDelete)
    BeginDisabled();
  Text("0x%08X \"%s\" Pos (%d,%d) Size (%d,%d) Collapsed=%d", settings->ID,
       settings->GetName(), settings->Pos.x, settings->Pos.y, settings->Size.x,
       settings->Size.y, settings->Collapsed);
  if (settings->WantDelete)
    EndDisabled();
}

void Gui::DebugNodeWindowsList(Vector<Window *> *windows, const char *label) {
  if (!TreeNode(label, "%s (%d)", label, windows->Size))
    return;
  for (int i = windows->Size - 1; i >= 0; i--) // Iterate front to back
  {
    PushID((*windows)[i]);
    DebugNodeWindow((*windows)[i], "Window");
    PopID();
  }
  TreePop();
}

// FIXME-OPT: This is technically suboptimal, but it is simpler this way.
void Gui::DebugNodeWindowsListByBeginStackParent(
    Window **windows, int windows_size, Window *parent_in_begin_stack) {
  for (int i = 0; i < windows_size; i++) {
    Window *window = windows[i];
    if (window->ParentWindowInBeginStack != parent_in_begin_stack)
      continue;
    char buf[20];
    FormatString(buf, ARRAYSIZE(buf), "[%04d] Window",
                 window->BeginOrderWithinContext);
    // BulletText("[%04d] Window '%s'", window->BeginOrderWithinContext,
    // window->Name);
    DebugNodeWindow(window, buf);
    Indent();
    DebugNodeWindowsListByBeginStackParent(windows + i + 1,
                                           windows_size - i - 1, window);
    Unindent();
  }
}

//-----------------------------------------------------------------------------
// [SECTION] DEBUG LOG WINDOW
//-----------------------------------------------------------------------------

void Gui::DebugLog(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  DebugLogV(fmt, args);
  va_end(args);
}

void Gui::DebugLogV(const char *fmt, va_list args) {
  Context &g = *GGui;
  const int old_size = g.DebugLogBuf.size();
  g.DebugLogBuf.appendf("[%05d] ", g.FrameCount);
  g.DebugLogBuf.appendfv(fmt, args);
  g.DebugLogIndex.append(g.DebugLogBuf.c_str(), old_size, g.DebugLogBuf.size());
  if (g.DebugLogFlags & DebugLogFlags_OutputToTTY)
    DEBUG_PRINTF("%s", g.DebugLogBuf.begin() + old_size);
#ifdef ENABLE_TEST_ENGINE
  if (g.DebugLogFlags & DebugLogFlags_OutputToTestEngine)
    TEST_ENGINE_LOG("%s", g.DebugLogBuf.begin() + old_size);
#endif
}

void Gui::ShowDebugLogWindow(bool *p_open) {
  Context &g = *GGui;
  if (!(g.NextWindowData.Flags & NextWindowDataFlags_HasSize))
    SetNextWindowSize(Vec2(0.0f, GetFontSize() * 12.0f), Cond_FirstUseEver);
  if (!Begin("Gui Debug Log", p_open) || GetCurrentWindow()->BeginCount > 1) {
    End();
    return;
  }

  CheckboxFlags("All", &g.DebugLogFlags, DebugLogFlags_EventMask_);
  SameLine();
  CheckboxFlags("ActiveId", &g.DebugLogFlags, DebugLogFlags_EventActiveId);
  SameLine();
  CheckboxFlags("Focus", &g.DebugLogFlags, DebugLogFlags_EventFocus);
  SameLine();
  CheckboxFlags("Popup", &g.DebugLogFlags, DebugLogFlags_EventPopup);
  SameLine();
  CheckboxFlags("Nav", &g.DebugLogFlags, DebugLogFlags_EventNav);
  SameLine();
  if (CheckboxFlags("Clipper", &g.DebugLogFlags, DebugLogFlags_EventClipper)) {
    g.DebugLogClipperAutoDisableFrames = 2;
  }
  if (IsItemHovered())
    SetTooltip("Clipper log auto-disabled after 2 frames");
  // SameLine(); CheckboxFlags("Selection", &g.DebugLogFlags,
  // DebugLogFlags_EventSelection);
  SameLine();
  CheckboxFlags("IO", &g.DebugLogFlags, DebugLogFlags_EventIO);
  SameLine();
  CheckboxFlags("Docking", &g.DebugLogFlags, DebugLogFlags_EventDocking);
  SameLine();
  CheckboxFlags("Viewport", &g.DebugLogFlags, DebugLogFlags_EventViewport);

  if (SmallButton("Clear")) {
    g.DebugLogBuf.clear();
    g.DebugLogIndex.clear();
  }
  SameLine();
  if (SmallButton("Copy"))
    SetClipboardText(g.DebugLogBuf.c_str());
  BeginChild("##log", Vec2(0.0f, 0.0f), ChildFlags_Border,
             WindowFlags_AlwaysVerticalScrollbar |
                 WindowFlags_AlwaysHorizontalScrollbar);

  const DebugLogFlags backup_log_flags = g.DebugLogFlags;
  g.DebugLogFlags &= ~DebugLogFlags_EventClipper;

  ListClipper clipper;
  clipper.Begin(g.DebugLogIndex.size());
  while (clipper.Step())
    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd;
         line_no++) {
      const char *line_begin =
          g.DebugLogIndex.get_line_begin(g.DebugLogBuf.c_str(), line_no);
      const char *line_end =
          g.DebugLogIndex.get_line_end(g.DebugLogBuf.c_str(), line_no);
      TextUnformatted(line_begin, line_end); // Display line
      Rect text_rect = g.LastItemData.Rect;
      if (IsItemHovered())
        for (const char *p = line_begin; p <= line_end - 10;
             p++) // Search for 0x???????? identifiers
        {
          ID id = 0;
          if (p[0] != '0' || (p[1] != 'x' && p[1] != 'X') ||
              sscanf(p + 2, "%X", &id) != 1)
            continue;
          Vec2 p0 = CalcTextSize(line_begin, p);
          Vec2 p1 = CalcTextSize(p, p + 10);
          g.LastItemData.Rect = Rect(text_rect.Min + Vec2(p0.x, 0.0f),
                                     text_rect.Min + Vec2(p0.x + p1.x, p1.y));
          if (IsMouseHoveringRect(g.LastItemData.Rect.Min,
                                  g.LastItemData.Rect.Max, true))
            DebugLocateItemOnHover(id);
          p += 10;
        }
    }
  g.DebugLogFlags = backup_log_flags;
  if (GetScrollY() >= GetScrollMaxY())
    SetScrollHereY(1.0f);
  EndChild();

  End();
}

//-----------------------------------------------------------------------------
// [SECTION] OTHER DEBUG TOOLS (ITEM PICKER, ID STACK TOOL)
//-----------------------------------------------------------------------------

// Draw a small cross at current CursorPos in current window's DrawList
void Gui::DebugDrawCursorPos(U32 col) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Vec2 pos = window->DC.CursorPos;
  window->DrawList->AddLine(Vec2(pos.x, pos.y - 3.0f),
                            Vec2(pos.x, pos.y + 4.0f), col, 1.0f);
  window->DrawList->AddLine(Vec2(pos.x - 3.0f, pos.y),
                            Vec2(pos.x + 4.0f, pos.y), col, 1.0f);
}

// Draw a 10px wide rectangle around CurposPos.x using Line Y1/Y2 in current
// window's DrawList
void Gui::DebugDrawLineExtents(U32 col) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  float curr_x = window->DC.CursorPos.x;
  float line_y1 = (window->DC.IsSameLine ? window->DC.CursorPosPrevLine.y
                                         : window->DC.CursorPos.y);
  float line_y2 = line_y1 + (window->DC.IsSameLine ? window->DC.PrevLineSize.y
                                                   : window->DC.CurrLineSize.y);
  window->DrawList->AddLine(Vec2(curr_x - 5.0f, line_y1),
                            Vec2(curr_x + 5.0f, line_y1), col, 1.0f);
  window->DrawList->AddLine(Vec2(curr_x - 0.5f, line_y1),
                            Vec2(curr_x - 0.5f, line_y2), col, 1.0f);
  window->DrawList->AddLine(Vec2(curr_x - 5.0f, line_y2),
                            Vec2(curr_x + 5.0f, line_y2), col, 1.0f);
}

// Draw last item rect in ForegroundDrawList (so it is always visible)
void Gui::DebugDrawItemRect(U32 col) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  GetForegroundDrawList(window)->AddRect(g.LastItemData.Rect.Min,
                                         g.LastItemData.Rect.Max, col);
}

// [DEBUG] Locate item position/rectangle given an ID.
static const U32 DEBUG_LOCATE_ITEM_COLOR = COL32(0, 255, 0, 255); // Green

void Gui::DebugLocateItem(ID target_id) {
  Context &g = *GGui;
  g.DebugLocateId = target_id;
  g.DebugLocateFrames = 2;
}

void Gui::DebugLocateItemOnHover(ID target_id) {
  if (target_id == 0 ||
      !IsItemHovered(HoveredFlags_AllowWhenBlockedByActiveItem |
                     HoveredFlags_AllowWhenBlockedByPopup))
    return;
  Context &g = *GGui;
  DebugLocateItem(target_id);
  GetForegroundDrawList(g.CurrentWindow)
      ->AddRect(g.LastItemData.Rect.Min - Vec2(3.0f, 3.0f),
                g.LastItemData.Rect.Max + Vec2(3.0f, 3.0f),
                DEBUG_LOCATE_ITEM_COLOR);
}

void Gui::DebugLocateItemResolveWithLastItem() {
  Context &g = *GGui;
  LastItemData item_data = g.LastItemData;
  g.DebugLocateId = 0;
  DrawList *draw_list = GetForegroundDrawList(g.CurrentWindow);
  Rect r = item_data.Rect;
  r.Expand(3.0f);
  Vec2 p1 = g.IO.MousePos;
  Vec2 p2 = Vec2((p1.x < r.Min.x)   ? r.Min.x
                 : (p1.x > r.Max.x) ? r.Max.x
                                    : p1.x,
                 (p1.y < r.Min.y)   ? r.Min.y
                 : (p1.y > r.Max.y) ? r.Max.y
                                    : p1.y);
  draw_list->AddRect(r.Min, r.Max, DEBUG_LOCATE_ITEM_COLOR);
  draw_list->AddLine(p1, p2, DEBUG_LOCATE_ITEM_COLOR);
}

// [DEBUG] Item picker tool - start with DebugStartItemPicker() - useful to
// visually select an item and break into its call-stack.
void Gui::UpdateDebugToolItemPicker() {
  Context &g = *GGui;
  g.DebugItemPickerBreakId = 0;
  if (!g.DebugItemPickerActive)
    return;

  const ID hovered_id = g.HoveredIdPreviousFrame;
  SetMouseCursor(MouseCursor_Hand);
  if (IsKeyPressed(Key_Escape))
    g.DebugItemPickerActive = false;
  const bool change_mapping = g.IO.KeyMods == (Mod_Ctrl | Mod_Shift);
  if (!change_mapping && IsMouseClicked(g.DebugItemPickerMouseButton) &&
      hovered_id) {
    g.DebugItemPickerBreakId = hovered_id;
    g.DebugItemPickerActive = false;
  }
  for (int mouse_button = 0; mouse_button < 3; mouse_button++)
    if (change_mapping && IsMouseClicked(mouse_button))
      g.DebugItemPickerMouseButton = (U8)mouse_button;
  SetNextWindowBgAlpha(0.70f);
  if (!BeginTooltip())
    return;
  Text("HoveredId: 0x%08X", hovered_id);
  Text("Press ESC to abort picking.");
  const char *mouse_button_names[] = {"Left", "Right", "Middle"};
  if (change_mapping)
    Text("Remap w/ Ctrl+Shift: click anywhere to select new mouse button.");
  else
    TextColored(GetStyleColorVec4(hovered_id ? Col_Text : Col_TextDisabled),
                "Click %s Button to break in debugger! (remap w/ Ctrl+Shift)",
                mouse_button_names[g.DebugItemPickerMouseButton]);
  EndTooltip();
}

// [DEBUG] ID Stack Tool: update queries. Called by NewFrame()
void Gui::UpdateDebugToolStackQueries() {
  Context &g = *GGui;
  IDStackTool *tool = &g.DebugIDStackTool;

  // Clear hook when id stack tool is not visible
  g.DebugHookIdInfo = 0;
  if (g.FrameCount != tool->LastActiveFrame + 1)
    return;

  // Update queries. The steps are: -1: query Stack, >= 0: query each stack item
  // We can only perform 1 ID Info query every frame. This is designed so the
  // GetID() tests are cheap and constant-time
  const ID query_id =
      g.HoveredIdPreviousFrame ? g.HoveredIdPreviousFrame : g.ActiveId;
  if (tool->QueryId != query_id) {
    tool->QueryId = query_id;
    tool->StackLevel = -1;
    tool->Results.resize(0);
  }
  if (query_id == 0)
    return;

  // Advance to next stack level when we got our result, or after 2 frames (in
  // case we never get a result)
  int stack_level = tool->StackLevel;
  if (stack_level >= 0 && stack_level < tool->Results.Size)
    if (tool->Results[stack_level].QuerySuccess ||
        tool->Results[stack_level].QueryFrameCount > 2)
      tool->StackLevel++;

  // Update hook
  stack_level = tool->StackLevel;
  if (stack_level == -1)
    g.DebugHookIdInfo = query_id;
  if (stack_level >= 0 && stack_level < tool->Results.Size) {
    g.DebugHookIdInfo = tool->Results[stack_level].ID;
    tool->Results[stack_level].QueryFrameCount++;
  }
}

// [DEBUG] ID Stack tool: hooks called by GetID() family functions
void Gui::DebugHookIdInfo(ID id, DataType data_type, const void *data_id,
                          const void *data_id_end) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  IDStackTool *tool = &g.DebugIDStackTool;

  // Step 0: stack query
  // This assumes that the ID was computed with the current ID stack, which
  // tends to be the case for our widget.
  if (tool->StackLevel == -1) {
    tool->StackLevel++;
    tool->Results.resize(window->IDStack.Size + 1, StackLevelInfo());
    for (int n = 0; n < window->IDStack.Size + 1; n++)
      tool->Results[n].ID =
          (n < window->IDStack.Size) ? window->IDStack[n] : id;
    return;
  }

  // Step 1+: query for individual level
  ASSERT(tool->StackLevel >= 0);
  if (tool->StackLevel != window->IDStack.Size)
    return;
  StackLevelInfo *info = &tool->Results[tool->StackLevel];
  ASSERT(info->ID == id && info->QueryFrameCount > 0);

  switch (data_type) {
  case DataType_S32:
    FormatString(info->Desc, ARRAYSIZE(info->Desc), "%d",
                 (int)(intptr_t)data_id);
    break;
  case DataType_String:
    FormatString(info->Desc, ARRAYSIZE(info->Desc), "%.*s",
                 data_id_end
                     ? (int)((const char *)data_id_end - (const char *)data_id)
                     : (int)strlen((const char *)data_id),
                 (const char *)data_id);
    break;
  case DataType_Pointer:
    FormatString(info->Desc, ARRAYSIZE(info->Desc), "(void*)0x%p", data_id);
    break;
  case DataType_ID:
    if (info->Desc[0] !=
        0) // PushOverrideID() is often used to avoid hashing twice, which would
           // lead to 2 calls to DebugHookIdInfo(). We prioritize the first one.
      return;
    FormatString(info->Desc, ARRAYSIZE(info->Desc), "0x%08X [override]", id);
    break;
  default:
    ASSERT(0);
  }
  info->QuerySuccess = true;
  info->DataType = data_type;
}

static int StackToolFormatLevelInfo(IDStackTool *tool, int n,
                                    bool format_for_ui, char *buf,
                                    size_t buf_size) {
  StackLevelInfo *info = &tool->Results[n];
  Window *window =
      (info->Desc[0] == 0 && n == 0) ? Gui::FindWindowByID(info->ID) : NULL;
  if (window) // Source: window name (because the root ID don't call GetID() and
              // so doesn't get hooked)
    return FormatString(buf, buf_size, format_for_ui ? "\"%s\" [window]" : "%s",
                        window->Name);
  if (info->QuerySuccess) // Source: GetID() hooks (prioritize over ItemInfo()
                          // because we frequently use patterns like:
                          // PushID(str), Button("") where they both have same
                          // id)
    return FormatString(
        buf, buf_size,
        (format_for_ui && info->DataType == DataType_String) ? "\"%s\"" : "%s",
        info->Desc);
  if (tool->StackLevel <
      tool->Results
          .Size) // Only start using fallback below when all queries are done,
                 // so during queries we don't flickering ??? markers.
    return (*buf = 0);
#ifdef ENABLE_TEST_ENGINE
  if (const char *label = TestEngine_FindItemDebugLabel(
          GGui, info->ID)) // Source: TestEngine's ItemInfo()
    return FormatString(buf, buf_size, format_for_ui ? "??? \"%s\"" : "%s",
                        label);
#endif
  return FormatString(buf, buf_size, "???");
}

// ID Stack Tool: Display UI
void Gui::ShowIDStackToolWindow(bool *p_open) {
  Context &g = *GGui;
  if (!(g.NextWindowData.Flags & NextWindowDataFlags_HasSize))
    SetNextWindowSize(Vec2(0.0f, GetFontSize() * 8.0f), Cond_FirstUseEver);
  if (!Begin("Gui ID Stack Tool", p_open) ||
      GetCurrentWindow()->BeginCount > 1) {
    End();
    return;
  }

  // Display hovered/active status
  IDStackTool *tool = &g.DebugIDStackTool;
  const ID hovered_id = g.HoveredIdPreviousFrame;
  const ID active_id = g.ActiveId;
#ifdef ENABLE_TEST_ENGINE
  Text("HoveredId: 0x%08X (\"%s\"), ActiveId:  0x%08X (\"%s\")", hovered_id,
       hovered_id ? TestEngine_FindItemDebugLabel(&g, hovered_id) : "",
       active_id,
       active_id ? TestEngine_FindItemDebugLabel(&g, active_id) : "");
#else
  Text("HoveredId: 0x%08X, ActiveId:  0x%08X", hovered_id, active_id);
#endif
  SameLine();
  MetricsHelpMarker(
      "Hover an item with the mouse to display elements of the ID Stack "
      "leading to the item's final ID.\nEach level of the stack correspond to "
      "a PushID() call.\nAll levels of the stack are hashed together to make "
      "the final ID of a widget (ID displayed at the bottom level of the "
      "stack).\nRead FAQ entry about the ID stack for details.");

  // CTRL+C to copy path
  const float time_since_copy = (float)g.Time - tool->CopyToClipboardLastTime;
  Checkbox("Ctrl+C: copy path to clipboard", &tool->CopyToClipboardOnCtrlC);
  SameLine();
  TextColored((time_since_copy >= 0.0f && time_since_copy < 0.75f &&
               Fmod(time_since_copy, 0.25f) < 0.25f * 0.5f)
                  ? Vec4(1.f, 1.f, 0.3f, 1.f)
                  : Vec4(),
              "*COPIED*");
  if (tool->CopyToClipboardOnCtrlC && IsKeyDown(Mod_Ctrl) &&
      IsKeyPressed(Key_C)) {
    tool->CopyToClipboardLastTime = (float)g.Time;
    char *p = g.TempBuffer.Data;
    char *p_end = p + g.TempBuffer.Size;
    for (int stack_n = 0; stack_n < tool->Results.Size && p + 3 < p_end;
         stack_n++) {
      *p++ = '/';
      char level_desc[256];
      StackToolFormatLevelInfo(tool, stack_n, false, level_desc,
                               ARRAYSIZE(level_desc));
      for (int n = 0; level_desc[n] && p + 2 < p_end; n++) {
        if (level_desc[n] == '/')
          *p++ = '\\';
        *p++ = level_desc[n];
      }
    }
    *p = '\0';
    SetClipboardText(g.TempBuffer.Data);
  }

  // Display decorated stack
  tool->LastActiveFrame = g.FrameCount;
  if (tool->Results.Size > 0 && BeginTable("##table", 3, TableFlags_Borders)) {
    const float id_width = CalcTextSize("0xDDDDDDDD").x;
    TableSetupColumn("Seed", TableColumnFlags_WidthFixed, id_width);
    TableSetupColumn("PushID", TableColumnFlags_WidthStretch);
    TableSetupColumn("Result", TableColumnFlags_WidthFixed, id_width);
    TableHeadersRow();
    for (int n = 0; n < tool->Results.Size; n++) {
      StackLevelInfo *info = &tool->Results[n];
      TableNextColumn();
      Text("0x%08X", (n > 0) ? tool->Results[n - 1].ID : 0);
      TableNextColumn();
      StackToolFormatLevelInfo(tool, n, true, g.TempBuffer.Data,
                               g.TempBuffer.Size);
      TextUnformatted(g.TempBuffer.Data);
      TableNextColumn();
      Text("0x%08X", info->ID);
      if (n == tool->Results.Size - 1)
        TableSetBgColor(TableBgTarget_CellBg, GetColorU32(Col_Header));
    }
    EndTable();
  }
  End();
}

#else

void Gui::ShowMetricsWindow(bool *) {}
void Gui::ShowFontAtlas(FontAtlas *) {}
void Gui::DebugNodeColumns(OldColumns *) {}
void Gui::DebugNodeDrawList(Window *, ViewportP *, const DrawList *,
                            const char *) {}
void Gui::DebugNodeDrawCmdShowMeshAndBoundingBox(DrawList *, const DrawList *,
                                                 const DrawCmd *, bool, bool) {}
void Gui::DebugNodeFont(Font *) {}
void Gui::DebugNodeStorage(Storage *, const char *) {}
void Gui::DebugNodeTabBar(TabBar *, const char *) {}
void Gui::DebugNodeWindow(Window *, const char *) {}
void Gui::DebugNodeWindowSettings(WindowSettings *) {}
void Gui::DebugNodeWindowsList(Vector<Window *> *, const char *) {}
void Gui::DebugNodeViewport(ViewportP *) {}

void Gui::DebugLog(const char *, ...) {}
void Gui::DebugLogV(const char *, va_list) {}
void Gui::ShowDebugLogWindow(bool *) {}
void Gui::ShowIDStackToolWindow(bool *) {}
void Gui::DebugHookIdInfo(ID, DataType, const void *, const void *) {}
void Gui::UpdateDebugToolItemPicker() {}
void Gui::UpdateDebugToolStackQueries() {}
void Gui::UpdateDebugToolFlashStyleColor() {}

#endif // #ifndef DISABLE_DEBUG_TOOLS

//-----------------------------------------------------------------------------

// Include user.inl at the end of gui.cpp to access private
// data/functions that aren't exposed. Prefer just including internal.hpp
// from your code rather than using this define. If a declaration is missing
// from internal.hpp add it or request it on the github.
#ifdef INCLUDE_USER_INL
#include "user.inl"
#endif

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
