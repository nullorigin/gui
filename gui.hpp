// Library Version
// (Integer encoded as XYYZZ for use in #if preprocessor conditionals, e.g. '#if
// VERSION_NUM >= 12345')
#define VERSION "1.90.1 WIP"
#define VERSION_NUM 19002
#define HAS_TABLE
#define HAS_VIEWPORT // Viewport WIP branch
#define HAS_DOCK     // Docking WIP branch

// [SECTION] Header mess
// [SECTION] Forward declarations and basic types
// [SECTION] Gui end-user API functions
// [SECTION] Flags & Enumerations
// [SECTION] Tables API flags and structures (TableFlags, TableColumnFlags,
// TableRowFlags, TableBgTarget, TableSortSpecs, TableColumnSortSpecs) [SECTION]
// Helpers: Memory allocations macros, Vector<> [SECTION] Style [SECTION] IO
// [SECTION] Misc data structures (InputTextCallbackData, SizeCallbackData,
// WindowClass, Payload) [SECTION] Helpers (OnceUponAFrame, TextFilter,
// TextBuffer, Storage, ListClipper, Math Operators, Color) [SECTION] Drawing
// API (DrawCallback, DrawCmd, DrawIdx, DrawVert, DrawChannel, DrawListSplitter,
// DrawFlags, DrawListFlags, DrawList, DrawData) [SECTION] Font API (FontConfig,
// FontGlyph, FontGlyphRangesBuilder, FontAtlasFlags, FontAtlas, Font) [SECTION]
// Viewports (ViewportFlags, Viewport) [SECTION] Platform Dependent Interfaces
// (PlatformIO, PlatformMonitor, PlatformImeData) [SECTION] Obsolete functions
// and types

#pragma once

// Configuration file with compile-time options
// (edit config.hpp or '#define USER_CONFIG "myfilename.h" from your build
// system)
#ifdef USER_CONFIG
#include USER_CONFIG
#endif
#include "config.hpp"

#ifndef DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

// Includes
#include <float.h>  // FLT_MIN, FLT_MAX
#include <stdarg.h> // va_list, va_start, va_end
#include <stddef.h> // ptrdiff_t, NULL
#include <string.h> // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp

// Define attributes of all API symbols declarations (e.g. for DLL under
// Windows) API is used for core imgui functions, API is used for the
// default backends files (xxx.h) Using gui via a shared
// library is not recommended: we don't guarantee backward nor forward ABI
// compatibility + this is a call-heavy library and function call overhead adds
// up.
#ifndef API
#define API
#endif
#ifndef API
#define API API
#endif

// Helper Macros
#ifndef ASSERT
#include <assert.h>
#define ASSERT(_EXPR)                                                          \
  assert(_EXPR) // You can override the default assert handler by editing
                // config.hpp
#endif
#define ARRAYSIZE(_ARR)                                                        \
  ((int)(sizeof(_ARR) / sizeof(*(_ARR)))) // Size of a static C-style array.
                                          // Don't use on pointers!
#define UNUSED(_VAR)                                                           \
  ((void)(_VAR)) // Used to silence "unused variable warnings". Often useful as
                 // asserts may be stripped out from final builds.
#define CHECKVERSION()                                                         \
  Gui::DebugCheckVersionAndDataLayout(VERSION, sizeof(IO), sizeof(Style),      \
                                      sizeof(Vec2), sizeof(Vec4),              \
                                      sizeof(DrawVert), sizeof(DrawIdx))

// Helper Macros - FMTARGS, FMTLIST: Apply printf-style warnings to our
// formatting functions.
#if !defined(USE_SPRINTF) && defined(__MINGW32__) && !defined(__clang__)
#define FMTARGS(FMT) __attribute__((format(gnu_printf, FMT, FMT + 1)))
#define FMTLIST(FMT) __attribute__((format(gnu_printf, FMT, 0)))
#elif !defined(USE_SPRINTF) && (defined(__clang__) || defined(__GNUC__))
#define FMTARGS(FMT) __attribute__((format(printf, FMT, FMT + 1)))
#define FMTLIST(FMT) __attribute__((format(printf, FMT, 0)))
#else
#define FMTARGS(FMT)
#define FMTLIST(FMT)
#endif

// Disable some of MSVC most aggressive Debug runtime checks in function
// header/footer (used in some simple/low-level functions)
#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER) &&  \
    !defined(DEBUG_PARANOID)
#define MSVC_RUNTIME_CHECKS_OFF                                                \
  __pragma(runtime_checks("", off)) __pragma(check_stack(off))                 \
      __pragma(strict_gs_check(push, off))
#define MSVC_RUNTIME_CHECKS_RESTORE                                            \
  __pragma(runtime_checks("", restore)) __pragma(check_stack())                \
      __pragma(strict_gs_check(pop))
#else
#define MSVC_RUNTIME_CHECKS_OFF
#define MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Warnings
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(                                                               \
    disable : 26495) // [Static Analyzer] Variable 'XXX' is uninitialized.
                     // Always initialize a member variable (type.6).
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored                                               \
    "-Wunknown-warning-option" // warning: unknown warning group 'xxx'
#endif
#pragma clang diagnostic ignored                                               \
    "-Wunknown-pragmas" // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored                                               \
    "-Wfloat-equal" // warning: comparing floating point with == or !=
                    // is unsafe
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored                                               \
    "-Wreserved-identifier" // warning: identifier '_Xxx' is reserved because it
                            // starts with '_' followed by a capital letter
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored                                                 \
    "-Wpragmas" // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored                                                 \
    "-Wclass-memaccess" // [__GNUC__ >= 8] warning: 'memset/memcpy'
                        // clearing/writing an object of type 'xxxx' with no
                        // trivial copy-assignment; use assignment or
                        // value-initialization instead
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// Forward declarations
struct DrawChannel; // Temporary storage to output draw commands out of order,
                    // used by DrawListSplitter and
                    // DrawList::ChannelsSplit()
struct DrawCmd;     // A single draw command within a parent DrawList (generally
                    // maps to 1 GPU draw call, unless it is a callback)
struct DrawData;    // All draw command lists required to render the frame +
                    // pos/size coordinates to use for the projection matrix.
struct DrawList;    // A single draw command list (generally one per window,
                 // conceptually you may see this as a dynamic "mesh" builder)
struct DrawListSharedData; // Data shared among multiple draw lists (typically
                           // owned by parent Gui context, but you may
                           // create one yourself)
struct DrawListSplitter;   // Helper to split a draw list into different layers
                           // which can be drawn into out of order, then
                           // flattened back.
struct DrawVert;  // A single vertex (pos + uv + col = 20 bytes by default.
                  // Override layout with OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
struct Font;      // Runtime data for a single font within a parent FontAtlas
struct FontAtlas; // Runtime data for multiple fonts, bake multiple fonts into
                  // a single texture, TTF/OTF font loader
struct FontBuilderIO; // Opaque interface to a font builder (truetype or
                      // FreeType).
struct FontConfig;    // Configuration data when adding a font or merging fonts
struct FontGlyph;     // A single font glyph (code point + coordinates within in
                      // FontAtlas + offset)
struct FontGlyphRangesBuilder; // Helper to build glyph ranges from
                               // text/string data
struct Color;   // Helper functions to create a color that can be converted to
                // either u32 or float4 (*OBSOLETE* please avoid using)
struct Context; // Gui context (opaque structure, unless including
                // internal.hpp)
struct IO;      // Main configuration and I/O between your application and Gui
struct InputTextCallbackData; // Shared state of InputText() when using
                              // custom InputTextCallback
                              // (rare/advanced use)
struct KeyData;          // Storage for IO and IsKeyDown(), IsKeyPressed() etc
                         // functions.
struct ListClipper;      // Helper to manually clip large list of items
struct OnceUponAFrame;   // Helper for running a block of code not more than
                         // once a frame
struct Payload;          // User data payload for drag and drop operations
struct PlatformIO;       // Multi-viewport support: interface for
                         // Platform/Renderer backends + viewports to render
struct PlatformMonitor;  // Multi-viewport support: user-provided bounds for
                         // each connected monitor/display. Used when
                         // positioning popups and tooltips to avoid them
                         // straddling monitors
struct PlatformImeData;  // Platform IME data for io.SetPlatformImeDataFn()
                         // function.
struct SizeCallbackData; // Callback data when using
                         // SetNextWindowSizeConstraints() (rare/advanced
                         // use)
struct Storage;          // Helper for key->value storage
struct Style;            // Runtime data for styling/colors
struct TableSortSpecs;   // Sorting specifications for a table (often
                         // handling sort specs for a single column,
                         // occasionally more)
struct TableColumnSortSpecs; // Sorting specification for one column of a
                             // table
struct TextBuffer;  // Helper to hold and append into a text buffer (~string
                    // builder)
struct TextFilter;  // Helper to parse and apply text filters (e.g.
                    // "aaaaa[,bbbbb][,ccccc]")
struct Viewport;    // A Platform Window (always 1 unless multi-viewport are
                    // enabled. One per platform window to output to). In the
                    // future may represent Platform Monitor
struct WindowClass; // Window class (rare/advanced uses: provide hints to
                    // the platform backend via altered viewport flags and
                    // parent/child info)

// Enumerations
// - We don't use strongly typed enums much because they add constraints (can't
// extend in private code, can't store typed in bit fields, extra casting on
// iteration)
// - Tip: Use your programming IDE navigation facilities on the names in the
// _central column_ below to find the actual flags/enum lists!
//   In Visual Studio IDE: CTRL+comma ("Edit.GoToAll") can follow symbols in
//   comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot. With Visual
//   Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow
//   symbols in comments.
enum Key : int;         // -> enum Key              // Enum: A key identifier
                        // (Key_XXX or Mod_XXX value)
enum MouseSource : int; // -> enum MouseSource      // Enum; A mouse
                        // input source identifier (Mouse, TouchScreen,
                        // Pen)
typedef int Col;        // -> enum Col_             // Enum: A color
                        // identifier for styling
typedef int Cond;       // -> enum Cond_            // Enum: A condition for
                        // many Set*() functions
typedef int DataType;   // -> enum DataType_        // Enum: A primary data type
typedef int Dir; // -> enum Dir_             // Enum: A cardinal direction
typedef int MouseButton;   // -> enum MouseButton_     // Enum: A mouse
                           // button identifier (0=left, 1=right, 2=middle)
typedef int MouseCursor;   // -> enum MouseCursor_     // Enum: A mouse
                           // cursor shape
typedef int SortDirection; // -> enum SortDirection_   // Enum: A
                           // sorting direction (ascending or descending)
typedef int StyleVar;      // -> enum StyleVar_        // Enum: A variable
                           // identifier for styling
typedef int TableBgTarget; // -> enum TableBgTarget_   // Enum: A
                           // color target for TableSetBgColor()

// Flags (declared as int to allow using as flags without overhead, and to not
// pollute the top of this file)
// - Tip: Use your programming IDE navigation facilities on the names in the
// _central column_ below to find the actual flags/enum lists!
//   In Visual Studio IDE: CTRL+comma ("Edit.GoToAll") can follow symbols in
//   comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot. With Visual
//   Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow
//   symbols in comments.
typedef int DrawFlags;      // -> enum DrawFlags_          // Flags: for
                            // DrawList functions
typedef int DrawListFlags;  // -> enum DrawListFlags_      // Flags: for
                            // DrawList instance
typedef int FontAtlasFlags; // -> enum FontAtlasFlags_     // Flags: for
                            // FontAtlas build
typedef int BackendFlags;   // -> enum BackendFlags_    // Flags: for
                            // io.BackendFlags
typedef int ButtonFlags;    // -> enum ButtonFlags_     // Flags: for
                            // InvisibleButton()
typedef int ChildFlags; // -> enum ChildFlags_      // Flags: for BeginChild()
typedef int ColorEditFlags; // -> enum ColorEditFlags_  // Flags: for
                            // ColorEdit4(), ColorPicker4() etc.
typedef int ConfigFlags;    // -> enum ConfigFlags_     // Flags: for
                            // io.ConfigFlags
typedef int ComboFlags; // -> enum ComboFlags_      // Flags: for BeginCombo()
typedef int DockNodeFlags;  // -> enum DockNodeFlags_   // Flags: for
                            // DockSpace()
typedef int DragDropFlags;  // -> enum DragDropFlags_   // Flags: for
                            // BeginDragDropSource(), AcceptDragDropPayload()
typedef int FocusedFlags;   // -> enum FocusedFlags_    // Flags: for
                            // IsWindowFocused()
typedef int HoveredFlags;   // -> enum HoveredFlags_    // Flags: for
                            // IsItemHovered(), IsWindowHovered() etc.
typedef int InputTextFlags; // -> enum InputTextFlags_  // Flags: for
                            // InputText(), InputTextMultiline()
typedef int KeyChord;       // -> Key | Mod_XXX    // Flags: for
                            // IsKeyChordPressed(), Shortcut() etc. an Key
// optionally OR-ed with one or more Mod_XXX values.
typedef int PopupFlags; // -> enum PopupFlags_      // Flags: for
                        // OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
typedef int SelectableFlags; // -> enum SelectableFlags_ // Flags: for
                             // Selectable()
typedef int
    SliderFlags; // -> enum SliderFlags_     // Flags: for
                 // DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
typedef int TabBarFlags;  // -> enum TabBarFlags_     // Flags: for
                          // BeginTabBar()
typedef int TabItemFlags; // -> enum TabItemFlags_    // Flags: for
                          // BeginTabItem()
typedef int TableFlags;   // -> enum TableFlags_      // Flags: For BeginTable()
typedef int TableColumnFlags; // -> enum TableColumnFlags_// Flags:
                              // For TableSetupColumn()
typedef int TableRowFlags;    // -> enum TableRowFlags_   // Flags: For
                              // TableNextRow()
typedef int TreeNodeFlags;    // -> enum TreeNodeFlags_   // Flags: for
                              // TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int ViewportFlags;    // -> enum ViewportFlags_   // Flags: for
                              // Viewport
typedef int WindowFlags;      // -> enum WindowFlags_     // Flags: for
                              // Begin(), BeginChild()

// Texture: user data for renderer backend to identify a texture [Compile-time
// configurable type]
// - To use something else than an opaque void* pointer: override with e.g.
// '#define TextureID MyTextureType*' in your config.hpp file.
// - This can be whatever to you want it to be! read the FAQ about TextureID
// for details.
#ifndef TextureID
typedef void *TextureID; // Default: store a pointer or an integer fitting in a
                         // pointer (most renderer backends are ok with that)
#endif

// DrawIdx: vertex index. [Compile-time configurable type]
// - To use 16-bit indices + allow large meshes: backend need to set
// 'io.BackendFlags |= BackendFlags_RendererHasVtxOffset' and handle
// DrawCmd::VtxOffset (recommended).
// - To use 32-bit indices: override with '#define DrawIdx unsigned int' in
// your config.hpp file.
#ifndef DrawIdx
typedef unsigned short DrawIdx; // Default: 16-bit (for maximum compatibility
                                // with renderer backends)
#endif

// Scalar data types
typedef unsigned int ID;    // A unique ID used by widgets (typically the
                            // result of hashing a stack of string)
typedef signed char S8;     // 8-bit signed integer
typedef unsigned char U8;   // 8-bit unsigned integer
typedef signed short S16;   // 16-bit signed integer
typedef unsigned short U16; // 16-bit unsigned integer
typedef signed int S32;     // 32-bit signed integer == int
typedef unsigned int
    U32; // 32-bit unsigned integer (often used to store packed colors)
typedef signed long long S64;   // 64-bit signed integer
typedef unsigned long long U64; // 64-bit unsigned integer

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage
// specifically for a decoded character used for keyboard input and display)
typedef unsigned int
    Wchar32; // A single decoded U32 character/code point. We encode them as
             // multi bytes UTF-8 when used in strings.
typedef unsigned short
    Wchar16; // A single decoded U16 character/code point. We encode them as
             // multi bytes UTF-8 when used in strings.
#ifdef USE_WCHAR32 // Wchar [configurable type: override in config.hpp with
                   // '#define USE_WCHAR32' to support Unicode planes 1-16]
typedef Wchar32 Wchar;
#else
typedef Wchar16 Wchar;
#endif

// Callback and functions types
typedef int (*InputTextCallback)(
    InputTextCallbackData *data); // Callback function for Gui::InputText()
typedef void (*SizeCallback)(
    SizeCallbackData
        *data); // Callback function for Gui::SetNextWindowSizeConstraints()
typedef void *(*MemAllocFunc)(
    size_t sz,
    void *user_data); // Function signature for Gui::SetAllocatorFunctions()
typedef void (*MemFreeFunc)(
    void *ptr,
    void *user_data); // Function signature for Gui::SetAllocatorFunctions()

// Vec2: 2D vector used to store positions, sizes etc. [Compile-time
// configurable type] This is a frequently used type in the API. Consider using
// VEC2_CLASS_EXTRA to create implicit cast from/to our preferred type.
MSVC_RUNTIME_CHECKS_OFF
struct Vec2 {
  float x, y;
  constexpr Vec2() : x(0.0f), y(0.0f) {}
  constexpr Vec2(float _x, float _y) : x(_x), y(_y) {}
  float &operator[](size_t idx) {
    ASSERT(idx == 0 || idx == 1);
    return ((float *)(void *)(char *)this)[idx];
  } // We very rarely use this [] operator, so the assert overhead is fine.
  float operator[](size_t idx) const {
    ASSERT(idx == 0 || idx == 1);
    return ((const float *)(const void *)(const char *)this)[idx];
  }
#ifdef VEC2_CLASS_EXTRA
  VEC2_CLASS_EXTRA // Define additional constructors and implicit cast
                   // operators in config.hpp to convert back and forth
                   // between your math types and Vec2.
#endif
};

// Vec4: 4D vector used to store clipping rectangles, colors etc.
// [Compile-time configurable type]
struct Vec4 {
  float x, y, z, w;
  constexpr Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
  constexpr Vec4(float _x, float _y, float _z, float _w)
      : x(_x), y(_y), z(_z), w(_w) {}
#ifdef VEC4_CLASS_EXTRA
  VEC4_CLASS_EXTRA // Define additional constructors and implicit cast
                   // operators in config.hpp to convert back and forth
                   // between your math types and Vec4.
#endif
};
MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Gui end-user API functions
// (Note that Gui:: being a namespace, you can add extra Gui:: functions in
// your own separate file. Please don't modify imgui source files!)
//-----------------------------------------------------------------------------

namespace Gui {
// Context creation and access
// - Each context create its own FontAtlas by default. You may instance one
// yourself and pass it to CreateContext() to share a font atlas between
// contexts.
// - DLL users: heaps and globals are not shared across DLL boundaries! You will
// need to call SetCurrentContext() + SetAllocatorFunctions()
//   for each static/DLL boundary you are calling from. Read "Context and Memory
//   Allocators" section of gui.cpp for details.
API Context *CreateContext(FontAtlas *shared_font_atlas = NULL);
API void DestroyContext(Context *ctx = NULL); // NULL = destroy current context
API Context *GetCurrentContext();
API void SetCurrentContext(Context *ctx);

// Main
API IO &GetIO(); // access the IO structure (mouse/keyboard/gamepad inputs,
                 // time, various configuration options/flags)
API Style &
GetStyle(); // access the Style structure (colors, sizes). Always use
            // PushStyleColor(), PushStyleVar() to modify style mid-frame!
API void NewFrame(); // start a new Gui frame, you can submit any command
                     // from this point until Render()/EndFrame().
API void
EndFrame(); // ends the Gui frame. automatically called by Render(). If
            // you don't need to render data (skipping rendering) you may call
            // EndFrame() without Render()... but you'll have wasted CPU
            // already! If you don't need to render, better to not create any
            // windows and not call NewFrame() at all!
API void Render(); // ends the Gui frame, finalize the draw data. You can
                   // then get call GetDrawData().
API DrawData *GetDrawData(); // valid after Render() and until the next call to
                             // NewFrame(). this is what you have to render.

// Demo, Debug, Information
API void ShowDemoWindow(
    bool *p_open = NULL); // create Demo window. demonstrate most Gui
                          // features. call this to learn about the library! try
                          // to make it always available in your application!
API void ShowMetricsWindow(
    bool *p_open =
        NULL); // create Metrics/Debugger window. display Gui internals:
               // windows, draw commands, various internal state, etc.
API void ShowDebugLogWindow(
    bool *p_open = NULL); // create Debug Log window. display a simplified log
                          // of important gui events.
API void ShowIDStackToolWindow(
    bool *p_open =
        NULL); // create Stack Tool window. hover items with mouse to query
               // information about the source of their unique ID.
API void ShowAboutWindow(
    bool *p_open = NULL); // create About window. display Gui version,
                          // credits and build/system information.
API void ShowStyleEditor(
    Style *ref = NULL); // add style editor block (not a window). you can pass
                        // in a reference Style structure to compare to, revert
                        // to and save to (else it uses the default style)
API bool ShowStyleSelector(
    const char *label); // add style selector block (not a window), essentially
                        // a combo listing the default styles.
API void ShowFontSelector(
    const char *label); // add font selector block (not a window), essentially a
                        // combo listing the loaded fonts.
API void
ShowUserGuide(); // add basic help/info block (not a window): how to manipulate
                 // Gui as an end-user (mouse/keyboard controls).
API const char *
GetVersion(); // get the compiled version string e.g. "1.80 WIP" (essentially
              // the value for VERSION from the compiled version of gui.cpp)

// Styles
API void StyleColorsDark(Style *dst = NULL); // new, recommended style (default)
API void StyleColorsLight(
    Style *dst = NULL); // best used with borders and a custom, thicker font
API void StyleColorsClassic(Style *dst = NULL); // classic imgui style

// Windows
// - Begin() = push window to the stack and start appending to it. End() = pop
// window from the stack.
// - Passing 'bool* p_open != NULL' shows a window-closing widget in the
// upper-right corner of the window,
//   which clicking will set the boolean to false when clicked.
// - You may append multiple times to the same window during the same frame by
// calling Begin()/End() pairs multiple times.
//   Some information such as 'flags' or 'p_open' will only be considered by the
//   first call to Begin().
// - Begin() return false to indicate the window is collapsed or fully clipped,
// so you may early out and omit submitting
//   anything to the window. Always call a matching End() for each Begin() call,
//   regardless of its return value! [Important: due to legacy reason, Begin/End
//   and BeginChild/EndChild are inconsistent with all other functions
//    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call
//    should only be called if the corresponding BeginXXX function returned
//    true. Begin and BeginChild are the only odd ones out. Will be fixed in a
//    future update.]
// - Note that the bottom of window stack always contains a window called
// "Debug".
API bool Begin(const char *name, bool *p_open = NULL, WindowFlags flags = 0);
API void End();

// Child Windows
// - Use child windows to begin into a self-contained independent
// scrolling/clipping regions within a host window. Child windows can embed
// their own child.
// - Before 1.90 (November 2023), the "ChildFlags child_flags = 0"
// parameter was "bool border = false".
//   This API is backward compatible with old code, as we guarantee that
//   ChildFlags_Border == true. Consider updating your old call sites:
//      BeginChild("Name", size, false)   -> Begin("Name", size, 0); or
//      Begin("Name", size, ChildFlags_None); BeginChild("Name", size,
//      true)    -> Begin("Name", size, ChildFlags_Border);
// - Manual sizing (each axis can use a different setting e.g. Vec2(0.0f,
// 400.0f)):
//     == 0.0f: use remaining parent window size for this axis.
//      > 0.0f: use specified size for this axis.
//      < 0.0f: right/bottom-align to specified distance from available content
//      boundaries.
// - Specifying ChildFlags_AutoResizeX or ChildFlags_AutoResizeY makes
// the sizing automatic based on child contents.
//   Combining both ChildFlags_AutoResizeX _and_
//   ChildFlags_AutoResizeY defeats purpose of a scrolling region and is
//   NOT recommended.
// - BeginChild() returns false to indicate the window is collapsed or fully
// clipped, so you may early out and omit submitting
//   anything to the window. Always call a matching EndChild() for each
//   BeginChild() call, regardless of its return value. [Important: due to
//   legacy reason, Begin/End and BeginChild/EndChild are inconsistent with all
//   other functions
//    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call
//    should only be called if the corresponding BeginXXX function returned
//    true. Begin and BeginChild are the only odd ones out. Will be fixed in a
//    future update.]
API bool BeginChild(const char *str_id, const Vec2 &size = Vec2(0, 0),
                    ChildFlags child_flags = 0, WindowFlags window_flags = 0);
API bool BeginChild(ID id, const Vec2 &size = Vec2(0, 0),
                    ChildFlags child_flags = 0, WindowFlags window_flags = 0);
API void EndChild();

// Windows Utilities
// - 'current window' = the window we are appending into while inside a
// Begin()/End() block. 'next window' = next window we will Begin() into.
API bool IsWindowAppearing();
API bool IsWindowCollapsed();
API bool IsWindowFocused(
    FocusedFlags flags = 0); // is current window focused? or its root/child,
                             // depending on flags. see flags for options.
API bool IsWindowHovered(
    HoveredFlags flags =
        0); // is current window hovered and hoverable (e.g. not blocked by a
            // popup/modal)? See HoveredFlags_ for options. IMPORTANT: If
            // you are trying to check whether your mouse should be dispatched
            // to Gui or to your underlying app, you should not use this
            // function! Use the 'io.WantCaptureMouse' boolean for that! Refer
            // to FAQ entry "How can I tell whether to dispatch mouse/keyboard
            // to Gui or my application?" for details.
API DrawList *
GetWindowDrawList(); // get draw list associated to the current window, to
                     // append your own drawing primitives
API float GetWindowDpiScale(); // get DPI scale currently associated to the
                               // current window's viewport.
API Vec2 GetWindowPos(); // get current window position in screen space (note:
                         // it is unlikely you need to use this. Consider using
                         // current layout pos instead, GetCursorScreenPos())
API Vec2
GetWindowSize(); // get current window size (note: it is unlikely you need to
                 // use this. Consider using GetCursorScreenPos() and e.g.
                 // GetContentRegionAvail() instead)
API float
GetWindowWidth(); // get current window width (shortcut for GetWindowSize().x)
API float
GetWindowHeight(); // get current window height (shortcut for GetWindowSize().y)
API Viewport *
GetWindowViewport(); // get viewport currently associated to the current window.

// Window manipulation
// - Prefer using SetNextXXX functions (before Begin) rather that SetXXX
// functions (after Begin).
API void SetNextWindowPos(
    const Vec2 &pos, Cond cond = 0,
    const Vec2 &pivot =
        Vec2(0, 0)); // set next window position. call before Begin(). use
                     // pivot=(0.5f,0.5f) to center on given point, etc.
API void SetNextWindowSize(
    const Vec2 &size,
    Cond cond = 0); // set next window size. set axis to 0.0f to force an
                    // auto-fit on this axis. call before Begin()
API void SetNextWindowSizeConstraints(
    const Vec2 &size_min, const Vec2 &size_max,
    SizeCallback custom_callback = NULL,
    void *custom_callback_data =
        NULL); // set next window size limits. use 0.0f or FLT_MAX if you don't
               // want limits. Use -1 for both min and max of same axis to
               // preserve current size (which itself is a constraint). Use
               // callback to apply non-trivial programmatic constraints.
API void SetNextWindowContentSize(
    const Vec2 &
        size); // set next window content size (~ scrollable client area, which
               // enforce the range of scrollbars). Not including window
               // decorations (title bar, menu bar, etc.) nor WindowPadding. set
               // an axis to 0.0f to leave it automatic. call before Begin()
API void SetNextWindowCollapsed(
    bool collapsed,
    Cond cond = 0); // set next window collapsed state. call before Begin()
API void SetNextWindowFocus(); // set next window to be focused / top-most. call
                               // before Begin()
API void
SetNextWindowScroll(const Vec2 &scroll); // set next window scrolling value (use
                                         // < 0.0f to not affect a given axis).
API void
SetNextWindowBgAlpha(float alpha); // set next window background color alpha.
                                   // helper to easily override the Alpha
                                   // component of Col_WindowBg/ChildBg/PopupBg.
                                   // you may also use WindowFlags_NoBackground.
API void SetNextWindowViewport(ID viewport_id); // set next window viewport
API void SetWindowPos(
    const Vec2 &pos,
    Cond cond = 0); // (not recommended) set current window position - call
                    // within Begin()/End(). prefer using SetNextWindowPos(),
                    // as this may incur tearing and side-effects.
API void SetWindowSize(
    const Vec2 &size,
    Cond cond = 0); // (not recommended) set current window size - call
                    // within Begin()/End(). set to Vec2(0, 0) to force
                    // an auto-fit. prefer using SetNextWindowSize(), as
                    // this may incur tearing and minor side-effects.
API void SetWindowCollapsed(
    bool collapsed,
    Cond cond = 0);        // (not recommended) set current window collapsed
                           // state. prefer using SetNextWindowCollapsed().
API void SetWindowFocus(); // (not recommended) set current window to be focused
                           // / top-most. prefer using SetNextWindowFocus().
API void SetWindowFontScale(
    float scale); // [OBSOLETE] set font scale. Adjust IO.FontGlobalScale if you
                  // want to scale all windows. This is an old API! For correct
                  // scaling, prefer to reload font + rebuild FontAtlas + call
                  // style.ScaleAllSizes().
API void SetWindowPos(const char *name, const Vec2 &pos,
                      Cond cond = 0); // set named window position.
API void
SetWindowSize(const char *name, const Vec2 &size,
              Cond cond = 0); // set named window size. set axis to 0.0f to
                              // force an auto-fit on this axis.
API void SetWindowCollapsed(const char *name, bool collapsed,
                            Cond cond = 0); // set named window collapsed state
API void
SetWindowFocus(const char *name); // set named window to be focused / top-most.
                                  // use NULL to remove focus.

// Content region
// - Retrieve available space from a given point. GetContentRegionAvail() is
// frequently useful.
// - Those functions are bound to be redesigned (they are confusing, incomplete
// and the Min/Max return values are in local window coordinates which increases
// confusion)
API Vec2 GetContentRegionAvail(); // == GetContentRegionMax() - GetCursorPos()
API Vec2 GetContentRegionMax(); // current content boundaries (typically window
                                // boundaries including scrolling, or current
                                // column boundaries), in windows coordinates
API Vec2
GetWindowContentRegionMin(); // content boundaries min for the full window
                             // (roughly (0,0)-Scroll), in window coordinates
API Vec2
GetWindowContentRegionMax(); // content boundaries max for the full window
                             // (roughly (0,0)+Size-Scroll) where Size can be
                             // overridden with SetNextWindowContentSize(), in
                             // window coordinates

// Windows Scrolling
// - Any change of Scroll will be applied at the beginning of next frame in the
// first call to Begin().
// - You may instead use SetNextWindowScroll() prior to calling Begin() to avoid
// this delay, as an alternative to using SetScrollX()/SetScrollY().
API float GetScrollX(); // get scrolling amount [0 .. GetScrollMaxX()]
API float GetScrollY(); // get scrolling amount [0 .. GetScrollMaxY()]
API void
SetScrollX(float scroll_x); // set scrolling amount [0 .. GetScrollMaxX()]
API void
SetScrollY(float scroll_y); // set scrolling amount [0 .. GetScrollMaxY()]
API float GetScrollMaxX();  // get maximum scrolling amount ~~ ContentSize.x -
                            // WindowSize.x - DecorationsSize.x
API float GetScrollMaxY();  // get maximum scrolling amount ~~ ContentSize.y -
                            // WindowSize.y - DecorationsSize.y
API void SetScrollHereX(
    float center_x_ratio =
        0.5f); // adjust scrolling amount to make current cursor position
               // visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right.
               // When using to make a "default/current item" visible, consider
               // using SetItemDefaultFocus() instead.
API void SetScrollHereY(
    float center_y_ratio =
        0.5f); // adjust scrolling amount to make current cursor position
               // visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom.
               // When using to make a "default/current item" visible, consider
               // using SetItemDefaultFocus() instead.
API void
SetScrollFromPosX(float local_x,
                  float center_x_ratio =
                      0.5f); // adjust scrolling amount to make given position
                             // visible. Generally GetCursorStartPos() + offset
                             // to compute a valid position.
API void
SetScrollFromPosY(float local_y,
                  float center_y_ratio =
                      0.5f); // adjust scrolling amount to make given position
                             // visible. Generally GetCursorStartPos() + offset
                             // to compute a valid position.

// Parameters stacks (shared)
API void PushFont(Font *font); // use NULL as a shortcut to push default font
API void PopFont();
API void PushStyleColor(Col idx,
                        U32 col); // modify a style color. always use this if
                                  // you modify the style after NewFrame().
API void PushStyleColor(Col idx, const Vec4 &col);
API void PopStyleColor(int count = 1);
API void
PushStyleVar(StyleVar idx,
             float val); // modify a style float variable. always use this if
                         // you modify the style after NewFrame().
API void
PushStyleVar(StyleVar idx,
             const Vec2 &val); // modify a style Vec2 variable. always use this
                               // if you modify the style after NewFrame().
API void PopStyleVar(int count = 1);
API void PushTabStop(bool tab_stop); // == tab stop enable. Allow focusing using
                                     // TAB/Shift-TAB, enabled by default but
                                     // you can disable it for certain widgets
API void PopTabStop();
API void PushButtonRepeat(
    bool repeat); // in 'repeat' mode, Button*() functions return repeated true
                  // in a typematic manner (using
                  // io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you
                  // can call IsItemActive() after any Button() to tell if the
                  // button is held in the current frame.
API void PopButtonRepeat();

// Parameters stacks (current window)
API void PushItemWidth(
    float item_width); // push width of items for common large "item+label"
                       // widgets. >0.0f: width in pixels, <0.0f align xx pixels
                       // to the right of window (so -FLT_MIN always align width
                       // to the right side).
API void PopItemWidth();
API void SetNextItemWidth(
    float item_width); // set width of the _next_ common large "item+label"
                       // widget. >0.0f: width in pixels, <0.0f align xx pixels
                       // to the right of window (so -FLT_MIN always align width
                       // to the right side)
API float CalcItemWidth(); // width of item given pushed settings and current
                           // cursor position. NOT necessarily the width of last
                           // item unlike most 'Item' functions.
API void PushTextWrapPos(
    float wrap_local_pos_x =
        0.0f); // push word-wrapping position for Text*() commands. < 0.0f: no
               // wrapping; 0.0f: wrap to end of window (or column); > 0.0f:
               // wrap at 'wrap_pos_x' position in window local space
API void PopTextWrapPos();

// Style read access
// - Use the ShowStyleEditor() function to interactively see/edit the colors.
API Font *GetFont();     // get current font
API float GetFontSize(); // get current font size (= height in pixels) of
                         // current font with current scale applied
API Vec2
GetFontTexUvWhitePixel(); // get UV coordinate for a while pixel, useful to draw
                          // custom shapes via the DrawList API
API U32 GetColorU32(
    Col idx,
    float alpha_mul = 1.0f); // retrieve given style color with style alpha
                             // applied and optional extra alpha multiplier,
                             // packed as a 32-bit value suitable for DrawList
API U32
GetColorU32(const Vec4 &col); // retrieve given color with style alpha applied,
                              // packed as a 32-bit value suitable for DrawList
API U32 GetColorU32(U32 col); // retrieve given color with style alpha applied,
                              // packed as a 32-bit value suitable for DrawList
API const Vec4 &GetStyleColorVec4(
    Col idx); // retrieve style color as stored in Style structure. use to
              // feed back into PushStyleColor(), otherwise use GetColorU32() to
              // get style color with style alpha baked in.

// Layout cursor positioning
// - By "cursor" we mean the current output position.
// - The typical widget behavior is to output themselves at the current cursor
// position, then move the cursor one line down.
// - You can call SameLine() between widgets to undo the last carriage return
// and output at the right of the preceding widget.
// - Attention! We currently have inconsistencies between window-local and
// absolute positions we will aim to fix with future API:
//    - Absolute coordinate:        GetCursorScreenPos(), SetCursorScreenPos(),
//    all DrawList:: functions. -> this is the preferred way forward.
//    - Window-local coordinates:   SameLine(), GetCursorPos(), SetCursorPos(),
//    GetCursorStartPos(), GetContentRegionMax(), GetWindowContentRegion*(),
//    PushTextWrapPos()
// - GetCursorScreenPos() = GetCursorPos() + GetWindowPos(). GetWindowPos() is
// almost only ever useful to convert from window-local to absolute coordinates.
API Vec2
GetCursorScreenPos(); // cursor position in absolute coordinates (prefer using
                      // this, also more useful to work with DrawList API).
API void
SetCursorScreenPos(const Vec2 &pos); // cursor position in absolute coordinates
API Vec2 GetCursorPos();             // [window-local] cursor position in window
                         // coordinates (relative to window position)
API float GetCursorPosX();                    // [window-local] "
API float GetCursorPosY();                    // [window-local] "
API void SetCursorPos(const Vec2 &local_pos); // [window-local] "
API void SetCursorPosX(float local_x);        // [window-local] "
API void SetCursorPosY(float local_y);        // [window-local] "
API Vec2 GetCursorStartPos(); // [window-local] initial cursor position, in
                              // window coordinates

// Other layout functions
API void
Separator(); // separator, generally horizontal. inside a menu bar or in
             // horizontal layout mode, this becomes a vertical separator.
API void
SameLine(float offset_from_start_x = 0.0f,
         float spacing =
             -1.0f); // call between widgets or groups to layout them
                     // horizontally. X position given in window coordinates.
API void NewLine();  // undo a SameLine() or force a new line when in a
                     // horizontal-layout context.
API void Spacing();  // add vertical spacing.
API void Dummy(const Vec2 &size); // add a dummy item of given size. unlike
                                  // InvisibleButton(), Dummy() won't take the
                                  // mouse click or be navigable into.
API void Indent(
    float indent_w = 0.0f); // move content position toward the right, by
                            // indent_w, or style.IndentSpacing if indent_w <= 0
API void Unindent(
    float indent_w = 0.0f); // move content position back to the left, by
                            // indent_w, or style.IndentSpacing if indent_w <= 0
API void BeginGroup();      // lock horizontal starting position
API void
EndGroup(); // unlock horizontal starting position + capture the whole group
            // bounding box into one "item" (so you can use IsItemHovered() or
            // layout primitives such as SameLine() on whole group, etc.)
API void
AlignTextToFramePadding(); // vertically align upcoming text baseline to
                           // FramePadding.y so that it will align properly to
                           // regularly framed items (call if you have text on a
                           // line before a framed item)
API float GetTextLineHeight(); // ~ FontSize
API float
GetTextLineHeightWithSpacing(); // ~ FontSize + style.ItemSpacing.y (distance in
                                // pixels between 2 consecutive lines of text)
API float GetFrameHeight();     // ~ FontSize + style.FramePadding.y * 2
API float
GetFrameHeightWithSpacing(); // ~ FontSize + style.FramePadding.y * 2 +
                             // style.ItemSpacing.y (distance in pixels between
                             // 2 consecutive lines of framed widgets)

// ID stack/scopes
// - These questions are answered and impacted by understanding of the ID stack
// system:
//   - "Q: Why is my widget not reacting when I click on it?"
//   - "Q: How can I have widgets with an empty label?"
//   - "Q: How can I have multiple widgets with the same label?"
// - Short version: ID are hashes of the entire ID stack. If you are creating
// widgets in a loop you most likely
//   want to push a unique identifier (e.g. object pointer, loop index) to
//   uniquely differentiate them.
// - You can also use the "Label##foobar" syntax within widget label to
// distinguish them from each others.
// - In this header file we use the "label"/"name" terminology to denote a
// string that will be displayed + used as an ID,
//   whereas "str_id" denote a string that is only used as an ID and not
//   normally displayed.
API void
PushID(const char *str_id); // push string into the ID stack (will hash string).
API void PushID(const char *str_id_begin,
                const char *str_id_end); // push string into the ID stack (will
                                         // hash string).
API void PushID(
    const void *ptr_id); // push pointer into the ID stack (will hash pointer).
API void
PushID(int int_id); // push integer into the ID stack (will hash integer).
API void PopID();   // pop from the ID stack.
API ID GetID(const char *str_id); // calculate unique ID (hash of whole ID stack
                                  // + given parameter). e.g. if you want to
                                  // query into Storage yourself
API ID GetID(const char *str_id_begin, const char *str_id_end);
API ID GetID(const void *ptr_id);

// Widgets: Text
API void
TextUnformatted(const char *text,
                const char *text_end =
                    NULL); // raw text without formatting. Roughly equivalent to
                           // Text("%s", text) but: A) doesn't require null
                           // terminated string if 'text_end' is specified, B)
                           // it's faster, no memory copy is done, no buffer
                           // size limits, recommended for long chunks of text.
API void Text(const char *fmt, ...) FMTARGS(1); // formatted text
API void TextV(const char *fmt, va_list args) FMTLIST(1);
API void TextColored(const Vec4 &col, const char *fmt, ...)
    FMTARGS(2); // shortcut for PushStyleColor(Col_Text, col); Text(fmt,
                // ...); PopStyleColor();
API void TextColoredV(const Vec4 &col, const char *fmt, va_list args)
    FMTLIST(2);
API void TextDisabled(const char *fmt, ...)
    FMTARGS(1); // shortcut for PushStyleColor(Col_Text,
                // style.Colors[Col_TextDisabled]); Text(fmt, ...);
                // PopStyleColor();
API void TextDisabledV(const char *fmt, va_list args) FMTLIST(1);
API void TextWrapped(const char *fmt, ...) FMTARGS(
    1); // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...);
        // PopTextWrapPos();. Note that this won't work on an auto-resizing
        // window if there's no other widgets to extend the window width, yoy
        // may need to set a size using SetNextWindowSize().
API void TextWrappedV(const char *fmt, va_list args) FMTLIST(1);
API void LabelText(const char *label, const char *fmt, ...) FMTARGS(
    2); // display text+label aligned the same way as value+label widgets
API void LabelTextV(const char *label, const char *fmt, va_list args)
    FMTLIST(2);
API void BulletText(const char *fmt, ...)
    FMTARGS(1); // shortcut for Bullet()+Text()
API void BulletTextV(const char *fmt, va_list args) FMTLIST(1);
API void SeparatorText(
    const char *label); // currently: formatted text with an horizontal line

// Widgets: Main
// - Most widgets return true when the value has been changed or when
// pressed/selected
// - You may also use one of the many IsItemXXX functions (e.g. IsItemActive,
// IsItemHovered, etc.) to query widget state.
API bool Button(const char *label, const Vec2 &size = Vec2(0, 0)); // button
API bool SmallButton(const char *label); // button with (FramePadding.y == 0) to
                                         // easily embed within text
API bool InvisibleButton(
    const char *str_id, const Vec2 &size,
    ButtonFlags flags =
        0); // flexible button behavior without the visuals, frequently useful
            // to build custom behaviors using the public api (along with
            // IsItemActive, IsItemHovered, etc.)
API bool ArrowButton(const char *str_id,
                     Dir dir); // square button with an arrow shape
API bool Checkbox(const char *label, bool *v);
API bool CheckboxFlags(const char *label, int *flags, int flags_value);
API bool CheckboxFlags(const char *label, unsigned int *flags,
                       unsigned int flags_value);
API bool RadioButton(const char *label,
                     bool active); // use with e.g. if (RadioButton("one",
                                   // my_value==1)) { my_value = 1; }
API bool RadioButton(const char *label, int *v,
                     int v_button); // shortcut to handle the above pattern when
                                    // value is an integer
API void ProgressBar(float fraction, const Vec2 &size_arg = Vec2(-FLT_MIN, 0),
                     const char *overlay = NULL);
API void Bullet(); // draw a small circle + keep the cursor on the same line.
                   // advance cursor x position by GetTreeNodeToLabelSpacing(),
                   // same distance that TreeNode() uses

// Widgets: Images
// - Note that Image() may add +2.0f to provided size if a border is visible,
// ImageButton() adds style.FramePadding*2.0f to provided size.
API void Image(TextureID user_texture_id, const Vec2 &image_size,
               const Vec2 &uv0 = Vec2(0, 0), const Vec2 &uv1 = Vec2(1, 1),
               const Vec4 &tint_col = Vec4(1, 1, 1, 1),
               const Vec4 &border_col = Vec4(0, 0, 0, 0));
API bool ImageButton(const char *str_id, TextureID user_texture_id,
                     const Vec2 &image_size, const Vec2 &uv0 = Vec2(0, 0),
                     const Vec2 &uv1 = Vec2(1, 1),
                     const Vec4 &bg_col = Vec4(0, 0, 0, 0),
                     const Vec4 &tint_col = Vec4(1, 1, 1, 1));

// Widgets: Combo Box (Dropdown)
// - The BeginCombo()/EndCombo() api allows you to manage your contents and
// selection state however you want it, by creating e.g. Selectable() items.
// - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept
// available for convenience purpose. This is analogous to how ListBox are
// created.
API bool BeginCombo(const char *label, const char *preview_value,
                    ComboFlags flags = 0);
API void EndCombo(); // only call EndCombo() if BeginCombo() returns true!
API bool Combo(const char *label, int *current_item, const char *const items[],
               int items_count, int popup_max_height_in_items = -1);
API bool Combo(const char *label, int *current_item,
               const char *items_separated_by_zeros,
               int popup_max_height_in_items =
                   -1); // Separate items with \0 within a string, end item-list
                        // with \0\0. e.g. "One\0Two\0Three\0"
API bool Combo(const char *label, int *current_item,
               const char *(*getter)(void *user_data, int idx), void *user_data,
               int items_count, int popup_max_height_in_items = -1);

// Widgets: Drag Sliders
// - CTRL+Click on any drag box to turn them into an input box. Manually input
// values aren't clamped by default and can go off-bounds. Use
// SliderFlags_AlwaysClamp to always clamp.
// - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every function,
// note that a 'float v[X]' function argument is the same as 'float* v',
//   the array syntax is just a way to document the number of elements that are
//   expected to be accessible. You can pass address of your first element out
//   of a contiguous set, e.g. &myvector.x
// - Adjust format string to decorate the value with a prefix, a suffix, or
// adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" ->
// 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
// - Format string may also be set to NULL or use the default format ("%f" or
// "%d").
// - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by
// 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum
// speed is Max(v_speed, minimum_step_at_given_precision).
// - Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click
// manual input can override those limits if SliderFlags_AlwaysClamp is not
// used.
// - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with
// v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
// - We use the same sets of flags for DragXXX() and SliderXXX() functions as
// the features are the same and it makes it easier to swap them.
// - Legacy: Pre-1.78 there are DragXXX() function signatures that take a final
// `float power=1.0f' argument instead of the `SliderFlags flags=0'
// argument.
API bool DragFloat(const char *label, float *v, float v_speed = 1.0f,
                   float v_min = 0.0f, float v_max = 0.0f,
                   const char *format = "%.3f",
                   SliderFlags flags = 0); // If v_min >= v_max we have no bound
API bool DragFloat2(const char *label, float v[2], float v_speed = 1.0f,
                    float v_min = 0.0f, float v_max = 0.0f,
                    const char *format = "%.3f", SliderFlags flags = 0);
API bool DragFloat3(const char *label, float v[3], float v_speed = 1.0f,
                    float v_min = 0.0f, float v_max = 0.0f,
                    const char *format = "%.3f", SliderFlags flags = 0);
API bool DragFloat4(const char *label, float v[4], float v_speed = 1.0f,
                    float v_min = 0.0f, float v_max = 0.0f,
                    const char *format = "%.3f", SliderFlags flags = 0);
API bool DragFloatRange2(const char *label, float *v_current_min,
                         float *v_current_max, float v_speed = 1.0f,
                         float v_min = 0.0f, float v_max = 0.0f,
                         const char *format = "%.3f",
                         const char *format_max = NULL, SliderFlags flags = 0);
API bool DragInt(const char *label, int *v, float v_speed = 1.0f, int v_min = 0,
                 int v_max = 0, const char *format = "%d",
                 SliderFlags flags = 0); // If v_min >= v_max we have no bound
API bool DragInt2(const char *label, int v[2], float v_speed = 1.0f,
                  int v_min = 0, int v_max = 0, const char *format = "%d",
                  SliderFlags flags = 0);
API bool DragInt3(const char *label, int v[3], float v_speed = 1.0f,
                  int v_min = 0, int v_max = 0, const char *format = "%d",
                  SliderFlags flags = 0);
API bool DragInt4(const char *label, int v[4], float v_speed = 1.0f,
                  int v_min = 0, int v_max = 0, const char *format = "%d",
                  SliderFlags flags = 0);
API bool DragIntRange2(const char *label, int *v_current_min,
                       int *v_current_max, float v_speed = 1.0f, int v_min = 0,
                       int v_max = 0, const char *format = "%d",
                       const char *format_max = NULL, SliderFlags flags = 0);
API bool DragScalar(const char *label, DataType data_type, void *p_data,
                    float v_speed = 1.0f, const void *p_min = NULL,
                    const void *p_max = NULL, const char *format = NULL,
                    SliderFlags flags = 0);
API bool DragScalarN(const char *label, DataType data_type, void *p_data,
                     int components, float v_speed = 1.0f,
                     const void *p_min = NULL, const void *p_max = NULL,
                     const char *format = NULL, SliderFlags flags = 0);

// Widgets: Regular Sliders
// - CTRL+Click on any slider to turn them into an input box. Manually input
// values aren't clamped by default and can go off-bounds. Use
// SliderFlags_AlwaysClamp to always clamp.
// - Adjust format string to decorate the value with a prefix, a suffix, or
// adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" ->
// 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
// - Format string may also be set to NULL or use the default format ("%f" or
// "%d").
// - Legacy: Pre-1.78 there are SliderXXX() function signatures that take a
// final `float power=1.0f' argument instead of the `SliderFlags flags=0'
// argument.
API bool SliderFloat(
    const char *label, float *v, float v_min, float v_max,
    const char *format = "%.3f",
    SliderFlags flags = 0); // adjust format to decorate the value with a prefix
                            // or a suffix for in-slider labels or unit display.
API bool SliderFloat2(const char *label, float v[2], float v_min, float v_max,
                      const char *format = "%.3f", SliderFlags flags = 0);
API bool SliderFloat3(const char *label, float v[3], float v_min, float v_max,
                      const char *format = "%.3f", SliderFlags flags = 0);
API bool SliderFloat4(const char *label, float v[4], float v_min, float v_max,
                      const char *format = "%.3f", SliderFlags flags = 0);
API bool SliderAngle(const char *label, float *v_rad,
                     float v_degrees_min = -360.0f,
                     float v_degrees_max = +360.0f,
                     const char *format = "%.0f deg", SliderFlags flags = 0);
API bool SliderInt(const char *label, int *v, int v_min, int v_max,
                   const char *format = "%d", SliderFlags flags = 0);
API bool SliderInt2(const char *label, int v[2], int v_min, int v_max,
                    const char *format = "%d", SliderFlags flags = 0);
API bool SliderInt3(const char *label, int v[3], int v_min, int v_max,
                    const char *format = "%d", SliderFlags flags = 0);
API bool SliderInt4(const char *label, int v[4], int v_min, int v_max,
                    const char *format = "%d", SliderFlags flags = 0);
API bool SliderScalar(const char *label, DataType data_type, void *p_data,
                      const void *p_min, const void *p_max,
                      const char *format = NULL, SliderFlags flags = 0);
API bool SliderScalarN(const char *label, DataType data_type, void *p_data,
                       int components, const void *p_min, const void *p_max,
                       const char *format = NULL, SliderFlags flags = 0);
API bool VSliderFloat(const char *label, const Vec2 &size, float *v,
                      float v_min, float v_max, const char *format = "%.3f",
                      SliderFlags flags = 0);
API bool VSliderInt(const char *label, const Vec2 &size, int *v, int v_min,
                    int v_max, const char *format = "%d",
                    SliderFlags flags = 0);
API bool VSliderScalar(const char *label, const Vec2 &size, DataType data_type,
                       void *p_data, const void *p_min, const void *p_max,
                       const char *format = NULL, SliderFlags flags = 0);

// Widgets: Input with Keyboard
// - If you want to use InputText() with std::string or any custom dynamic
// string type, see misc/cpp/stdlib.h and comments in demo.cpp.
// - Most of the InputTextFlags flags are only useful for InputText() and
// not for InputFloatX, InputIntX, InputDouble etc.
API bool InputText(const char *label, char *buf, size_t buf_size,
                   InputTextFlags flags = 0, InputTextCallback callback = NULL,
                   void *user_data = NULL);
API bool InputTextMultiline(const char *label, char *buf, size_t buf_size,
                            const Vec2 &size = Vec2(0, 0),
                            InputTextFlags flags = 0,
                            InputTextCallback callback = NULL,
                            void *user_data = NULL);
API bool InputTextWithHint(const char *label, const char *hint, char *buf,
                           size_t buf_size, InputTextFlags flags = 0,
                           InputTextCallback callback = NULL,
                           void *user_data = NULL);
API bool InputFloat(const char *label, float *v, float step = 0.0f,
                    float step_fast = 0.0f, const char *format = "%.3f",
                    InputTextFlags flags = 0);
API bool InputFloat2(const char *label, float v[2], const char *format = "%.3f",
                     InputTextFlags flags = 0);
API bool InputFloat3(const char *label, float v[3], const char *format = "%.3f",
                     InputTextFlags flags = 0);
API bool InputFloat4(const char *label, float v[4], const char *format = "%.3f",
                     InputTextFlags flags = 0);
API bool InputInt(const char *label, int *v, int step = 1, int step_fast = 100,
                  InputTextFlags flags = 0);
API bool InputInt2(const char *label, int v[2], InputTextFlags flags = 0);
API bool InputInt3(const char *label, int v[3], InputTextFlags flags = 0);
API bool InputInt4(const char *label, int v[4], InputTextFlags flags = 0);
API bool InputDouble(const char *label, double *v, double step = 0.0,
                     double step_fast = 0.0, const char *format = "%.6f",
                     InputTextFlags flags = 0);
API bool InputScalar(const char *label, DataType data_type, void *p_data,
                     const void *p_step = NULL, const void *p_step_fast = NULL,
                     const char *format = NULL, InputTextFlags flags = 0);
API bool InputScalarN(const char *label, DataType data_type, void *p_data,
                      int components, const void *p_step = NULL,
                      const void *p_step_fast = NULL, const char *format = NULL,
                      InputTextFlags flags = 0);

// Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little
// color square that can be left-clicked to open a picker, and right-clicked to
// open an option menu.)
// - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float*
// v', the array syntax is just a way to document the number of elements that
// are expected to be accessible.
// - You can pass the address of a first float element out of a contiguous
// structure, e.g. &myvector.x
API bool ColorEdit3(const char *label, float col[3], ColorEditFlags flags = 0);
API bool ColorEdit4(const char *label, float col[4], ColorEditFlags flags = 0);
API bool ColorPicker3(const char *label, float col[3],
                      ColorEditFlags flags = 0);
API bool ColorPicker4(const char *label, float col[4], ColorEditFlags flags = 0,
                      const float *ref_col = NULL);
API bool ColorButton(
    const char *desc_id, const Vec4 &col, ColorEditFlags flags = 0,
    const Vec2 &size = Vec2(0, 0)); // display a color square/button, hover for
                                    // details, return true when pressed.
API void SetColorEditOptions(
    ColorEditFlags
        flags); // initialize current options (generally on application startup)
                // if you want to select a default format, picker type, etc.
                // User will be able to change many settings, unless you pass
                // the _NoOptions flag to your calls.

// Widgets: Trees
// - TreeNode functions return true when the node is open, in which case you
// need to also call TreePop() when you are finished displaying the tree node
// contents.
API bool TreeNode(const char *label);
API bool TreeNode(const char *str_id, const char *fmt, ...) FMTARGS(
    2); // helper variation to easily decorelate the id from the displayed
        // string. Read the FAQ about why and how to use ID. to align arbitrary
        // text at the same level as a TreeNode() you can use Bullet().
API bool TreeNode(const void *ptr_id, const char *fmt, ...) FMTARGS(2); // "
API bool TreeNodeV(const char *str_id, const char *fmt, va_list args)
    FMTLIST(2);
API bool TreeNodeV(const void *ptr_id, const char *fmt, va_list args)
    FMTLIST(2);
API bool TreeNodeEx(const char *label, TreeNodeFlags flags = 0);
API bool TreeNodeEx(const char *str_id, TreeNodeFlags flags, const char *fmt,
                    ...) FMTARGS(3);
API bool TreeNodeEx(const void *ptr_id, TreeNodeFlags flags, const char *fmt,
                    ...) FMTARGS(3);
API bool TreeNodeExV(const char *str_id, TreeNodeFlags flags, const char *fmt,
                     va_list args) FMTLIST(3);
API bool TreeNodeExV(const void *ptr_id, TreeNodeFlags flags, const char *fmt,
                     va_list args) FMTLIST(3);
API void
TreePush(const char *str_id); // ~ Indent()+PushID(). Already called by
                              // TreeNode() when returning true, but you can
                              // call TreePush/TreePop yourself if desired.
API void TreePush(const void *ptr_id); // "
API void TreePop();                    // ~ Unindent()+PopID()
API float GetTreeNodeToLabelSpacing(); // horizontal distance preceding label
                                       // when using TreeNode*() or Bullet() ==
                                       // (g.FontSize + style.FramePadding.x*2)
                                       // for a regular unframed TreeNode
API bool CollapsingHeader(
    const char *label,
    TreeNodeFlags flags =
        0); // if returning 'true' the header is open. doesn't indent nor push
            // on ID stack. user doesn't have to call TreePop().
API bool CollapsingHeader(
    const char *label, bool *p_visible,
    TreeNodeFlags flags =
        0); // when 'p_visible != NULL': if '*p_visible==true' display an
            // additional small close button on upper right of the header which
            // will set the bool to false when clicked, if '*p_visible==false'
            // don't display the header.
API void SetNextItemOpen(
    bool is_open,
    Cond cond = 0); // set next TreeNode/CollapsingHeader open state.

// Widgets: Selectables
// - A selectable highlights when hovered, and can display another color when
// selected.
// - Neighbors selectable extend their highlight bounds in order to leave no gap
// between them. This is so a series of selected Selectable appear contiguous.
API bool Selectable(
    const char *label, bool selected = false, SelectableFlags flags = 0,
    const Vec2 &size =
        Vec2(0, 0)); // "bool selected" carry the selection state (read-only).
                     // Selectable() is clicked is returns true so you can
                     // modify your selection state. size.x==0.0: use remaining
                     // width, size.x>0.0: specify width. size.y==0.0: use label
                     // height, size.y>0.0: specify height
API bool Selectable(const char *label, bool *p_selected,
                    SelectableFlags flags = 0,
                    const Vec2 &size = Vec2(
                        0, 0)); // "bool* p_selected" point to the selection
                                // state (read-write), as a convenient helper.

// Widgets: List Boxes
// - This is essentially a thin wrapper to using BeginChild/EndChild with the
// ChildFlags_FrameStyle flag for stylistic changes + displaying a label.
// - You can submit contents and manage your selection state however you want
// it, by creating e.g. Selectable() or any other items.
// - The simplified/old ListBox() api are helpers over
// BeginListBox()/EndListBox() which are kept available for convenience purpose.
// This is analoguous to how Combos are created.
// - Choose frame width:   size.x > 0.0f: custom  /  size.x < 0.0f or -FLT_MIN:
// right-align   /  size.x = 0.0f (default): use current ItemWidth
// - Choose frame height:  size.y > 0.0f: custom  /  size.y < 0.0f or -FLT_MIN:
// bottom-align  /  size.y = 0.0f (default): arbitrary default height which can
// fit ~7 items
API bool
BeginListBox(const char *label,
             const Vec2 &size = Vec2(0, 0)); // open a framed scrolling region
API void
EndListBox(); // only call EndListBox() if BeginListBox() returned true!
API bool ListBox(const char *label, int *current_item,
                 const char *const items[], int items_count,
                 int height_in_items = -1);
API bool ListBox(const char *label, int *current_item,
                 const char *(*getter)(void *user_data, int idx),
                 void *user_data, int items_count, int height_in_items = -1);

// Widgets: Data Plotting
// - Consider using Plot (https://github.com/epezent/implot) which is much
// better!
API void PlotLines(const char *label, const float *values, int values_count,
                   int values_offset = 0, const char *overlay_text = NULL,
                   float scale_min = FLT_MAX, float scale_max = FLT_MAX,
                   Vec2 graph_size = Vec2(0, 0), int stride = sizeof(float));
API void PlotLines(const char *label,
                   float (*values_getter)(void *data, int idx), void *data,
                   int values_count, int values_offset = 0,
                   const char *overlay_text = NULL, float scale_min = FLT_MAX,
                   float scale_max = FLT_MAX, Vec2 graph_size = Vec2(0, 0));
API void PlotHistogram(const char *label, const float *values, int values_count,
                       int values_offset = 0, const char *overlay_text = NULL,
                       float scale_min = FLT_MAX, float scale_max = FLT_MAX,
                       Vec2 graph_size = Vec2(0, 0),
                       int stride = sizeof(float));
API void PlotHistogram(const char *label,
                       float (*values_getter)(void *data, int idx), void *data,
                       int values_count, int values_offset = 0,
                       const char *overlay_text = NULL,
                       float scale_min = FLT_MAX, float scale_max = FLT_MAX,
                       Vec2 graph_size = Vec2(0, 0));

// Widgets: Value() Helpers.
// - Those are merely shortcut to calling Text() with a format string. Output
// single value in "name: value" format (tip: freely declare more in your code
// to handle your types. you can add functions to the Gui namespace)
API void Value(const char *prefix, bool b);
API void Value(const char *prefix, int v);
API void Value(const char *prefix, unsigned int v);
API void Value(const char *prefix, float v, const char *float_format = NULL);

// Widgets: Menus
// - Use BeginMenuBar() on a window WindowFlags_MenuBar to append to its
// menu bar.
// - Use BeginMainMenuBar() to create a menu bar at the top of the screen and
// append to it.
// - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time
// with the same identifier to append more items to it.
// - Not that MenuItem() keyboardshortcuts are displayed as a convenience but
// _not processed_ by Gui at the moment.
API bool BeginMenuBar(); // append to menu-bar of current window (requires
                         // WindowFlags_MenuBar flag set on parent window).
API void EndMenuBar(); // only call EndMenuBar() if BeginMenuBar() returns true!
API bool BeginMainMenuBar(); // create and append to a full screen menu-bar.
API void EndMainMenuBar();   // only call EndMainMenuBar() if BeginMainMenuBar()
                             // returns true!
API bool BeginMenu(const char *label,
                   bool enabled = true); // create a sub-menu entry. only call
                                         // EndMenu() if this returns true!
API void EndMenu(); // only call EndMenu() if BeginMenu() returns true!
API bool MenuItem(const char *label, const char *shortcut = NULL,
                  bool selected = false,
                  bool enabled = true); // return true when activated.
API bool MenuItem(const char *label, const char *shortcut, bool *p_selected,
                  bool enabled = true); // return true when activated + toggle
                                        // (*p_selected) if p_selected != NULL

// Tooltips
// - Tooltips are windows following the mouse. They do not take focus away.
// - A tooltip window can contain items of any types. SetTooltip() is a shortcut
// for the 'if (BeginTooltip()) { Text(...); EndTooltip(); }' idiom.
API bool BeginTooltip(); // begin/append a tooltip window.
API void EndTooltip();   // only call EndTooltip() if
                         // BeginTooltip()/BeginItemTooltip() returns true!
API void SetTooltip(const char *fmt, ...) FMTARGS(
    1); // set a text-only tooltip. Often used after a Gui::IsItemHovered()
        // check. Override any previous call to SetTooltip().
API void SetTooltipV(const char *fmt, va_list args) FMTLIST(1);

// Tooltips: helpers for showing a tooltip when hovering an item
// - BeginItemTooltip() is a shortcut for the 'if
// (IsItemHovered(HoveredFlags_ForTooltip) && BeginTooltip())' idiom.
// - SetItemTooltip() is a shortcut for the 'if
// (IsItemHovered(HoveredFlags_ForTooltip)) { SetTooltip(...); }' idiom.
// - Where 'HoveredFlags_ForTooltip' itself is a shortcut to use
// 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav'
// depending on active input type. For mouse it defaults to
// 'HoveredFlags_Stationary | HoveredFlags_DelayShort'.
API bool BeginItemTooltip(); // begin/append a tooltip window if preceding item
                             // was hovered.
API void SetItemTooltip(const char *fmt, ...)
    FMTARGS(1); // set a text-only tooltip if preceeding item was hovered.
                // override any previous call to SetTooltip().
API void SetItemTooltipV(const char *fmt, va_list args) FMTLIST(1);

// Popups, Modals
//  - They block normal mouse hovering detection (and therefore most mouse
//  interactions) behind them.
//  - If not modal: they can be closed by clicking anywhere outside them, or by
//  pressing ESCAPE.
//  - Their visibility state (~bool) is held internally instead of being held by
//  the programmer as we are used to with regular Begin*() calls.
//  - The 3 properties above are related: we need to retain popup visibility
//  state in the library because popups may be closed as any time.
//  - You can bypass the hovering restriction by using
//  HoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or
//  IsWindowHovered().
//  - IMPORTANT: Popup identifiers are relative to the current ID stack, so
//  OpenPopup and BeginPopup generally needs to be at the same level of the
//  stack.
//    This is sometimes leading to confusing mistakes. May rework this in the
//    future.
//  - BeginPopup(): query popup state, if open start appending into the window.
//  Call EndPopup() afterwards if returned true. WindowFlags are forwarded
//  to the window.
//  - BeginPopupModal(): block every interaction behind the window, cannot be
//  closed by user, add a dimming background, has a title bar.
API bool
BeginPopup(const char *str_id,
           WindowFlags flags = 0); // return true if the popup is open, and
                                   // you can start outputting to it.
API bool
BeginPopupModal(const char *name, bool *p_open = NULL,
                WindowFlags flags = 0); // return true if the modal is open, and
                                        // you can start outputting to it.
API void EndPopup(); // only call EndPopup() if BeginPopupXXX() returns true!

// Popups: open/close functions
//  - OpenPopup(): set popup state to open. PopupFlags are available for
//  opening options.
//  - If not modal: they can be closed by clicking anywhere outside them, or by
//  pressing ESCAPE.
//  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close
//  manually.
//  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when
//  activated (FIXME: need some options).
//  - Use PopupFlags_NoOpenOverExistingPopup to avoid opening a popup if
//  there's already one at the same level. This is equivalent to e.g. testing
//  for !IsAnyPopupOpen() prior to OpenPopup().
//  - Use IsWindowAppearing() after BeginPopup() to tell if a window just
//  opened.
//  - IMPORTANT: Notice that for OpenPopupOnItemClick() we exceptionally default
//  flags to 1 (== PopupFlags_MouseButtonRight) for backward compatibility
//  with older API taking 'int mouse_button = 1' parameter
API void
OpenPopup(const char *str_id,
          PopupFlags popup_flags =
              0); // call to mark popup as open (don't call every frame!).
API void
OpenPopup(ID id,
          PopupFlags popup_flags =
              0); // id overload to facilitate calling from nested stacks
API void
OpenPopupOnItemClick(const char *str_id = NULL,
                     PopupFlags popup_flags =
                         1); // helper to open popup when clicked on last item.
                             // Default to PopupFlags_MouseButtonRight == 1.
                             // (note: actually triggers on the mouse _released_
                             // event to be consistent with popup behaviors)
API void CloseCurrentPopup(); // manually close the popup we have begin-ed into.

// Popups: open+begin combined functions helpers
//  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by
//  e.g. hovering an item and right-clicking.
//  - They are convenient to easily create context menus, hence the name.
//  - IMPORTANT: Notice that BeginPopupContextXXX takes PopupFlags just
//  like OpenPopup() and unlike BeginPopup(). For full consistency, we may add
//  WindowFlags to the BeginPopupContextXXX functions in the future.
//  - IMPORTANT: Notice that we exceptionally default their flags to 1 (==
//  PopupFlags_MouseButtonRight) for backward compatibility with older API
//  taking 'int mouse_button = 1' parameter, so if you add other flags remember
//  to re-add the PopupFlags_MouseButtonRight.
API bool BeginPopupContextItem(
    const char *str_id = NULL,
    PopupFlags popup_flags =
        1); // open+begin popup when clicked on last item. Use str_id==NULL to
            // associate the popup to previous item. If you want to use that on
            // a non-interactive item such as Text() you need to pass in an
            // explicit ID here. read comments in .cpp!
API bool BeginPopupContextWindow(
    const char *str_id = NULL,
    PopupFlags popup_flags =
        1); // open+begin popup when clicked on current window.
API bool BeginPopupContextVoid(
    const char *str_id = NULL,
    PopupFlags popup_flags = 1); // open+begin popup when clicked in void
                                 // (where there are no windows).

// Popups: query functions
//  - IsPopupOpen(): return true if the popup is open at the current
//  BeginPopup() level of the popup stack.
//  - IsPopupOpen() with PopupFlags_AnyPopupId: return true if any popup is
//  open at the current BeginPopup() level of the popup stack.
//  - IsPopupOpen() with PopupFlags_AnyPopupId +
//  PopupFlags_AnyPopupLevel: return true if any popup is open.
API bool IsPopupOpen(const char *str_id,
                     PopupFlags flags = 0); // return true if the popup is open.

// Tables
// - Full-featured replacement for old Columns API.
// - See Demo->Tables for demo code. See top of tables.cpp for general
// commentary.
// - See TableFlags_ and TableColumnFlags_ enums for a description of
// available flags. The typical call flow is:
// - 1. Call BeginTable(), early out if returning false.
// - 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
// - 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of
// columns/rows.
// - 4. Optionally call TableHeadersRow() to submit a header row. Names are
// pulled from TableSetupColumn() data.
// - 5. Populate contents:
//    - In most situations you can use TableNextRow() + TableSetColumnIndex(N)
//    to start appending into a column.
//    - If you are using tables as a sort of grid, where every column is holding
//    the same type of contents,
//      you may prefer using TableNextColumn() instead of TableNextRow() +
//      TableSetColumnIndex(). TableNextColumn() will automatically wrap-around
//      into the next row if needed.
//    - IMPORTANT: Comparatively to the old Columns() API, we need to call
//    TableNextColumn() for the first column!
//    - Summary of possible call flow:
//        - TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") ->
//        TableSetColumnIndex(1) -> Text("Hello 1")  // OK
//        - TableNextRow() -> TableNextColumn()      -> Text("Hello 0") ->
//        TableNextColumn()      -> Text("Hello 1")  // OK
//        -                   TableNextColumn()      -> Text("Hello 0") ->
//        TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn()
//        automatically gets to next row!
//        - TableNextRow()                           -> Text("Hello 0") // Not
//        OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not
//        appear!
// - 5. Call EndTable()
API bool BeginTable(const char *str_id, int column, TableFlags flags = 0,
                    const Vec2 &outer_size = Vec2(0.0f, 0.0f),
                    float inner_width = 0.0f);
API void EndTable(); // only call EndTable() if BeginTable() returns true!
API void TableNextRow(
    TableRowFlags row_flags = 0,
    float min_row_height = 0.0f); // append into the first cell of a new row.
API bool TableNextColumn(); // append into the next column (or first column of
                            // next row if currently in last column). Return
                            // true when column is visible.
API bool
TableSetColumnIndex(int column_n); // append into the specified column. Return
                                   // true when column is visible.

// Tables: Headers & Columns declaration
// - Use TableSetupColumn() to specify label, resizing policy, default
// width/weight, id, various other flags etc.
// - Use TableHeadersRow() to create a header row and automatically submit a
// TableHeader() for each column.
//   Headers are required to perform: reordering, sorting, and opening the
//   context menu. The context menu can also be made available in columns body
//   using TableFlags_ContextMenuInBody.
// - You may manually submit headers using TableNextRow() + TableHeader() calls,
// but this is only useful in
//   some advanced use cases (e.g. adding custom widgets in header row).
// - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when
// scrolled.
API void TableSetupColumn(const char *label, TableColumnFlags flags = 0,
                          float init_width_or_weight = 0.0f, ID user_id = 0);
API void TableSetupScrollFreeze(
    int cols,
    int rows); // lock columns/rows so they stay visible when scrolled.
API void
TableHeader(const char *label); // submit one header cell manually (rarely used)
API void
TableHeadersRow(); // submit a row with headers cells based on data provided to
                   // TableSetupColumn() + submit context menu
API void
TableAngledHeadersRow(); // submit a row with angled headers for every column
                         // with the TableColumnFlags_AngledHeader flag.
                         // MUST BE FIRST ROW.

// Tables: Sorting & Miscellaneous functions
// - Sorting: call TableGetSortSpecs() to retrieve latest sort specs for the
// table. NULL when not sorting.
//   When 'sort_specs->SpecsDirty == true' you should sort your data. It will be
//   true when sorting specs have changed since last call, or the first time.
//   Make sure to set 'SpecsDirty = false' after sorting, else you may
//   wastefully sort your data every frame!
// - Functions args 'int column_n' treat the default value of -1 as the same as
// passing the current column index.
API TableSortSpecs *
TableGetSortSpecs(); // get latest sort specs for the table (NULL if not
                     // sorting).  Lifetime: don't hold on this pointer over
                     // multiple frames or past any subsequent call to
                     // BeginTable().
API int
TableGetColumnCount(); // return number of columns (value passed to BeginTable)
API int TableGetColumnIndex(); // return current column index.
API int TableGetRowIndex();    // return current row index.
API const char *TableGetColumnName(
    int column_n = -1); // return "" if column didn't have a name declared by
                        // TableSetupColumn(). Pass -1 to use current column.
API TableColumnFlags TableGetColumnFlags(
    int column_n = -1); // return column flags so you can query their
                        // Enabled/Visible/Sorted/Hovered status flags. Pass -1
                        // to use current column.
API void TableSetColumnEnabled(
    int column_n,
    bool v); // change user accessible enabled/disabled state of a column. Set
             // to false to hide the column. User can use the context menu to
             // change this themselves (right-click in headers, or right-click
             // in columns body with TableFlags_ContextMenuInBody)
API void TableSetBgColor(
    TableBgTarget target, U32 color,
    int column_n = -1); // change the color of a cell, row, or column. See
                        // TableBgTarget_ flags for details.

// Legacy Columns API (prefer using Tables!)
// - You can also use SameLine(pos_x) to mimic simplified columns.
API void Columns(int count = 1, const char *id = NULL, bool border = true);
API void NextColumn();    // next column, defaults to current row or next row if
                          // the current row is finished
API int GetColumnIndex(); // get current column index
API float GetColumnWidth(
    int column_index =
        -1); // get column width (in pixels). pass -1 to use current column
API void SetColumnWidth(
    int column_index,
    float width); // set column width (in pixels). pass -1 to use current column
API float GetColumnOffset(
    int column_index =
        -1); // get position of column line (in pixels, from the left side of
             // the contents region). pass -1 to use current column, otherwise
             // 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
API void
SetColumnOffset(int column_index,
                float offset_x); // set position of column line (in pixels, from
                                 // the left side of the contents region). pass
                                 // -1 to use current column
API int GetColumnsCount();

// Tab Bars, Tabs
// - Note: Tabs are automatically created by the docking system (when in
// 'docking' branch). Use this to create tab bars/tabs yourself.
API bool BeginTabBar(const char *str_id,
                     TabBarFlags flags = 0); // create and append into a TabBar
API void EndTabBar(); // only call EndTabBar() if BeginTabBar() returns true!
API bool
BeginTabItem(const char *label, bool *p_open = NULL,
             TabItemFlags flags =
                 0);   // create a Tab. Returns true if the Tab is selected.
API void EndTabItem(); // only call EndTabItem() if BeginTabItem() returns true!
API bool TabItemButton(
    const char *label,
    TabItemFlags flags = 0); // create a Tab behaving like a button. return true
                             // when clicked. cannot be selected in the tab bar.
API void SetTabItemClosed(
    const char
        *tab_or_docked_window_label); // notify TabBar or Docking system of a
                                      // closed tab/window ahead (useful to
                                      // reduce visual flicker on reorderable
                                      // tab bars). For tab-bar: call after
                                      // BeginTabBar() and before Tab
                                      // submissions. Otherwise call with a
                                      // window name.

// Docking
// [BETA API] Enable with io.ConfigFlags |= ConfigFlags_DockingEnable.
// Note: You can use most Docking facilities without calling any API. You DO NOT
// need to call DockSpace() to use Docking!
// - Drag from window title bar or their tab to dock/undock. Hold SHIFT to
// disable docking.
// - Drag from window menu button (upper-left button) to undock an entire node
// (all windows).
// - When io.ConfigDockingWithShift == true, you instead need to hold SHIFT to
// enable docking. About dockspaces:
// - Use DockSpaceOverViewport() to create an explicit dock node covering the
// screen or a specific viewport.
//   This is often used with DockNodeFlags_PassthruCentralNode to make it
//   transparent.
// - Use DockSpace() to create an explicit dock node _within_ an existing
// window. See Docking demo for details.
// - Important: Dockspaces need to be submitted _before_ any window they can
// host. Submit it early in your frame!
// - Important: Dockspaces need to be kept alive if hidden, otherwise windows
// docked into it will be undocked.
//   e.g. if you have multiple tabs with a dockspace inside each tab: submit the
//   non-visible dockspaces with DockNodeFlags_KeepAliveOnly.
API ID DockSpace(ID id, const Vec2 &size = Vec2(0, 0), DockNodeFlags flags = 0,
                 const WindowClass *window_class = NULL);
API ID DockSpaceOverViewport(const Viewport *viewport = NULL,
                             DockNodeFlags flags = 0,
                             const WindowClass *window_class = NULL);
API void SetNextWindowDockID(ID dock_id,
                             Cond cond = 0); // set next window dock id
API void SetNextWindowClass(
    const WindowClass *
        window_class); // set next window class (control docking compatibility +
                       // provide hints to platform backend via custom viewport
                       // flags and platform parent/child relationship)
API ID GetWindowDockID();
API bool IsWindowDocked(); // is current window docked into another window?

// Logging/Capture
// - All text output from the interface can be captured into tty/file/clipboard.
// By default, tree nodes are automatically opened during logging.
API void LogToTTY(int auto_open_depth = -1); // start logging to tty (stdout)
API void LogToFile(int auto_open_depth = -1,
                   const char *filename = NULL); // start logging to file
API void
LogToClipboard(int auto_open_depth = -1); // start logging to OS clipboard
API void LogFinish();                     // stop logging (close file, etc.)
API void
LogButtons(); // helper to display buttons for logging to tty/file/clipboard
API void LogText(const char *fmt, ...)
    FMTARGS(1); // pass text data straight to log (without being displayed)
API void LogTextV(const char *fmt, va_list args) FMTLIST(1);

// Drag and Drop
// - On source items, call BeginDragDropSource(), if it returns true also call
// SetDragDropPayload() + EndDragDropSource().
// - On target candidates, call BeginDragDropTarget(), if it returns true also
// call AcceptDragDropPayload() + EndDragDropTarget().
// - If you stop calling BeginDragDropSource() the payload is preserved however
// it won't have a preview tooltip (we currently display a fallback "..."
// tooltip, see #1725)
// - An item can be both drag source and drop target.
API bool BeginDragDropSource(
    DragDropFlags flags = 0); // call after submitting an item which may be
                              // dragged. when this return true, you can call
                              // SetDragDropPayload() + EndDragDropSource()
API bool SetDragDropPayload(
    const char *type, const void *data, size_t sz,
    Cond cond = 0); // type is a user defined string of maximum 32 characters.
                    // Strings starting with '_' are reserved for gui
                    // internal types. Data is copied and held by imgui. Return
                    // true when payload has been accepted.
API void EndDragDropSource(); // only call EndDragDropSource() if
                              // BeginDragDropSource() returns true!
API bool
BeginDragDropTarget(); // call after submitting an item that may receive a
                       // payload. If this returns true, you can call
                       // AcceptDragDropPayload() + EndDragDropTarget()
API const Payload *AcceptDragDropPayload(
    const char *type,
    DragDropFlags flags =
        0); // accept contents of a given type. If
            // DragDropFlags_AcceptBeforeDelivery is set you can peek into
            // the payload before the mouse button is released.
API void EndDragDropTarget(); // only call EndDragDropTarget() if
                              // BeginDragDropTarget() returns true!
API const Payload *
GetDragDropPayload(); // peek directly into the current payload from anywhere.
                      // returns NULL when drag and drop is finished or
                      // inactive. use Payload::IsDataType() to test for
                      // the payload type.

// Disabling [BETA API]
// - Disable all user interactions and dim items visuals (applying
// style.DisabledAlpha over current colors)
// - Those can be nested but it cannot be used to enable an already disabled
// section (a single BeginDisabled(true) in the stack is enough to keep
// everything disabled)
// - BeginDisabled(false) essentially does nothing useful but is provided to
// facilitate use of boolean expressions. If you can avoid calling
// BeginDisabled(False)/EndDisabled() best to avoid it.
API void BeginDisabled(bool disabled = true);
API void EndDisabled();

// Clipping
// - Mouse hovering is affected by Gui::PushClipRect() calls, unlike direct
// calls to DrawList::PushClipRect() which are render only.
API void PushClipRect(const Vec2 &clip_rect_min, const Vec2 &clip_rect_max,
                      bool intersect_with_current_clip_rect);
API void PopClipRect();

// Focus, Activation
// - Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing())
// SetScrollHereY()" when applicable to signify "this is the default item"
API void
SetItemDefaultFocus(); // make last item the default focused item of a window.
API void SetKeyboardFocusHere(
    int offset = 0); // focus keyboard on the next widget. Use positive 'offset'
                     // to access sub components of a multiple component widget.
                     // Use -1 to access previous widget.

// Overlapping mode
API void
SetNextItemAllowOverlap(); // allow next item to be overlapped by a subsequent
                           // item. Useful with invisible buttons, selectable,
                           // treenode covering an area where subsequent items
                           // may need to be added. Note that both Selectable()
                           // and TreeNode() have dedicated flags doing this.

// Item/Widgets Utilities and Query Functions
// - Most of the functions are referring to the previous Item that has been
// submitted.
// - See Demo Window under "Widgets->Querying Status" for an interactive
// visualization of most of those functions.
API bool
IsItemHovered(HoveredFlags flags =
                  0); // is the last item hovered? (and usable, aka not blocked
                      // by a popup, etc.). See HoveredFlags for more options.
API bool IsItemActive(); // is the last item active? (e.g. button being held,
                         // text field being edited. This will continuously
                         // return true while holding mouse button on an item.
                         // Items that don't interact will always return false)
API bool
IsItemFocused(); // is the last item focused for keyboard/gamepad navigation?
API bool IsItemClicked(
    MouseButton mouse_button =
        0); // is the last item hovered and mouse clicked on? (**)  ==
            // IsMouseClicked(mouse_button) && IsItemHovered()Important. (**)
            // this is NOT equivalent to the behavior of e.g. Button(). Read
            // comments in function definition.
API bool IsItemVisible(); // is the last item visible? (items may be out of
                          // sight because of clipping/scrolling)
API bool IsItemEdited();  // did the last item modify its underlying value this
                         // frame? or was pressed? This is generally the same as
                         // the "bool" return value of many widgets.
API bool IsItemActivated(); // was the last item just made active (item was
                            // previously inactive).
API bool
IsItemDeactivated(); // was the last item just made inactive (item was
                     // previously active). Useful for Undo/Redo patterns with
                     // widgets that require continuous editing.
API bool
IsItemDeactivatedAfterEdit(); // was the last item just made inactive and made a
                              // value change when it was active? (e.g.
                              // Slider/Drag moved). Useful for Undo/Redo
                              // patterns with widgets that require continuous
                              // editing. Note that you may get false positives
                              // (some widgets such as
                              // Combo()/ListBox()/Selectable() will return true
                              // even when clicking an already selected item).
API bool
IsItemToggledOpen(); // was the last item open state toggled? set by TreeNode().
API bool IsAnyItemHovered(); // is any item hovered?
API bool IsAnyItemActive();  // is any item active?
API bool IsAnyItemFocused(); // is any item focused?
API ID GetItemID();          // get ID of last item (~~ often same
                             // Gui::GetID(label) beforehand)
API Vec2 GetItemRectMin();   // get upper-left bounding rectangle of the last
                             // item (screen space)
API Vec2 GetItemRectMax();   // get lower-right bounding rectangle of the last
                             // item (screen space)
API Vec2 GetItemRectSize();  // get size of last item

// Viewports
// - Currently represents the Platform Window created by the application which
// is hosting our Gui windows.
// - In 'docking' branch with multi-viewport enabled, we extend this concept to
// have multiple active viewports.
// - In the future we will extend this concept further to also represent
// Platform Monitor and support a "no main platform window" operation mode.
API Viewport *
GetMainViewport(); // return primary/default viewport. This can never be NULL.

// Background/Foreground Draw Lists
API DrawList *
GetBackgroundDrawList(); // get background draw list for the viewport associated
                         // to the current window. this draw list will be the
                         // first rendering one. Useful to quickly draw
                         // shapes/text behind gui contents.
API DrawList *
GetForegroundDrawList(); // get foreground draw list for the viewport associated
                         // to the current window. this draw list will be the
                         // last rendered one. Useful to quickly draw
                         // shapes/text over gui contents.
API DrawList *GetBackgroundDrawList(
    Viewport
        *viewport); // get background draw list for the given viewport. this
                    // draw list will be the first rendering one. Useful to
                    // quickly draw shapes/text behind gui contents.
API DrawList *GetForegroundDrawList(
    Viewport *viewport); // get foreground draw list for the given viewport.
                         // this draw list will be the last rendered one. Useful
                         // to quickly draw shapes/text over gui contents.

// Miscellaneous Utilities
API bool IsRectVisible(
    const Vec2 &size); // test if rectangle (of given size, starting from
                       // cursor position) is visible / not clipped.
API bool IsRectVisible(
    const Vec2 &rect_min,
    const Vec2
        &rect_max); // test if rectangle (in screen space) is visible / not
                    // clipped. to perform coarse clipping on user's side.
API double
GetTime(); // get global imgui time. incremented by io.DeltaTime every frame.
API int
GetFrameCount(); // get global imgui frame count. incremented by 1 every frame.
API DrawListSharedData *
GetDrawListSharedData(); // you may use this when creating your own DrawList
                         // instances.
API const char *
GetStyleColorName(Col idx); // get a string corresponding to the enum value
                            // (for display, saving, etc.).
API void SetStateStorage(
    Storage *
        storage); // replace current window storage with our own (if you want to
                  // manipulate it yourself, typically clear subsection of it)
API Storage *GetStateStorage();

// Text Utilities
API Vec2 CalcTextSize(const char *text, const char *text_end = NULL,
                      bool hide_text_after_double_hash = false,
                      float wrap_width = -1.0f);

// Color Utilities
API Vec4 ColorConvertU32ToFloat4(U32 in);
API U32 ColorConvertFloat4ToU32(const Vec4 &in);
API void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h,
                              float &out_s, float &out_v);
API void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r,
                              float &out_g, float &out_b);

// Inputs Utilities: Keyboard/Mouse/Gamepad
// - the Key enum contains all possible keyboard, mouse and gamepad inputs
// (e.g. Key_A, Key_MouseLeft, Key_GamepadDpadUp...).
// - before v1.87, we used Key to carry native/user indices as defined by
// each backends. About use of those legacy Key values:
//  - without DISABLE_OBSOLETE_KEYIO (legacy support): you can still use your
//  legacy native/user indices (< 512) according to how your backend/engine
//  stored them in io.KeysDown[], but need to cast them to Key.
//  - with    DISABLE_OBSOLETE_KEYIO (this is the way forward): any use of
//  Key will assert with key < 512. GetKeyIndex() is pass-through and
//  therefore deprecated (gone if DISABLE_OBSOLETE_KEYIO is defined).
API bool IsKeyDown(Key key); // is key being held.
API bool IsKeyPressed(
    Key key,
    bool repeat = true); // was key pressed (went from !Down to Down)? if
                         // repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
API bool IsKeyReleased(Key key); // was key released (went from Down to !Down)?
API bool IsKeyChordPressed(
    KeyChord key_chord); // was key chord (mods + key) pressed, e.g. you can
                         // pass 'Mod_Ctrl | Key_S' as a key-chord. This
                         // doesn't do any routing or focus check, please
                         // consider using Shortcut() function instead.
API int GetKeyPressedAmount(
    Key key, float repeat_delay,
    float rate); // uses provided repeat rate/delay. return a count, most often
                 // 0 or 1 but might be >1 if RepeatRate is small enough that
                 // DeltaTime > RepeatRate
API const char *
GetKeyName(Key key); // [DEBUG] returns English name of the key. Those
                     // names a provided for debugging purpose and are not
                     // meant to be saved persistently not compared.
API void SetNextFrameWantCaptureKeyboard(
    bool want_capture_keyboard); // Override io.WantCaptureKeyboard flag next
                                 // frame (said flag is left for your
                                 // application to handle, typically when true
                                 // it instructs your app to ignore inputs).
                                 // e.g. force capture keyboard when your widget
                                 // is being hovered. This is equivalent to
                                 // setting "io.WantCaptureKeyboard =
                                 // want_capture_keyboard"; after the next
                                 // NewFrame() call.

// Inputs Utilities: Mouse specific
// - To refer to a mouse button, you may use named enums in your code e.g.
// MouseButton_Left, MouseButton_Right.
// - You can also use regular integer: it is forever guaranteed that 0=Left,
// 1=Right, 2=Middle.
// - Dragging operations are only reported after mouse has moved a certain
// distance away from the initial clicking position (see 'lock_threshold' and
// 'io.MouseDraggingThreshold')
API bool IsMouseDown(MouseButton button); // is mouse button held?
API bool IsMouseClicked(
    MouseButton button,
    bool repeat = false); // did mouse button clicked? (went from !Down to
                          // Down). Same as GetMouseClickedCount() == 1.
API bool IsMouseReleased(MouseButton button); // did mouse button released?
                                              // (went from Down to !Down)
API bool IsMouseDoubleClicked(
    MouseButton
        button); // did mouse button double-clicked? Same as
                 // GetMouseClickedCount() == 2. (note that a double-click will
                 // also report IsMouseClicked() == true)
API int GetMouseClickedCount(
    MouseButton button); // return the number of successive mouse-clicks at
                         // the time where a click happen (otherwise 0).
API bool IsMouseHoveringRect(
    const Vec2 &r_min, const Vec2 &r_max,
    bool clip =
        true); // is mouse hovering given bounding rect (in screen space).
               // clipped by current clipping settings, but disregarding of
               // other consideration of focus/window ordering/popup-block.
API bool IsMousePosValid(
    const Vec2 *mouse_pos = NULL); // by convention we use (-FLT_MAX,-FLT_MAX)
                                   // to denote that there is no mouse available
API bool
IsAnyMouseDown(); // [WILL OBSOLETE] is any mouse button held? This was designed
                  // for backends, but prefer having backend maintain a mask of
                  // held mouse buttons, because upcoming input queue system
                  // will make this invalid.
API Vec2 GetMousePos(); // shortcut to Gui::GetIO().MousePos provided by
                        // user, to be consistent with other calls
API Vec2
GetMousePosOnOpeningCurrentPopup(); // retrieve mouse position at the time of
                                    // opening popup we have BeginPopup() into
                                    // (helper to avoid user backing that value
                                    // themselves)
API bool IsMouseDragging(
    MouseButton button,
    float lock_threshold = -1.0f); // is mouse dragging? (if lock_threshold <
                                   // -1.0f, uses io.MouseDraggingThreshold)
API Vec2 GetMouseDragDelta(
    MouseButton button = 0,
    float lock_threshold =
        -1.0f); // return the delta from the initial clicking position while the
                // mouse button is pressed or was just released. This is locked
                // and return 0.0f until the mouse moves past a distance
                // threshold at least once (if lock_threshold < -1.0f, uses
                // io.MouseDraggingThreshold)
API void ResetMouseDragDelta(MouseButton button = 0); //
API MouseCursor
GetMouseCursor(); // get desired mouse cursor shape. Important: reset in
                  // Gui::NewFrame(), this is updated during the frame. valid
                  // before Render(). If you use software rendering by setting
                  // io.MouseDrawCursor Gui will render those for you
API void
SetMouseCursor(MouseCursor cursor_type); // set desired mouse cursor shape
API void SetNextFrameWantCaptureMouse(
    bool
        want_capture_mouse); // Override io.WantCaptureMouse flag next frame
                             // (said flag is left for your application to
                             // handle, typical when true it instucts your app
                             // to ignore inputs). This is equivalent to setting
                             // "io.WantCaptureMouse = want_capture_mouse;"
                             // after the next NewFrame() call.

// Clipboard Utilities
// - Also see the LogToClipboard() function to capture GUI into clipboard, or
// easily output text data to the clipboard.
API const char *GetClipboardText();
API void SetClipboardText(const char *text);

// Settings/.Ini Utilities
// - The disk functions are automatically called if io.IniFilename != NULL
// (default is "gui.ini").
// - Set io.IniFilename to NULL to load/save manually. Read
// io.WantSaveIniSettings description about handling .ini saving manually.
// - Important: default value "gui.ini" is relative to current working dir!
// Most apps will want to lock this to an absolute path (e.g. same path as
// executables).
API void LoadIniSettingsFromDisk(
    const char
        *ini_filename); // call after CreateContext() and before the first call
                        // to NewFrame(). NewFrame() automatically calls
                        // LoadIniSettingsFromDisk(io.IniFilename).
API void LoadIniSettingsFromMemory(
    const char *ini_data,
    size_t ini_size =
        0); // call after CreateContext() and before the first call to
            // NewFrame() to provide .ini data from your own data source.
API void SaveIniSettingsToDisk(
    const char
        *ini_filename); // this is automatically called (if io.IniFilename is
                        // not empty) a few seconds after any modification that
                        // should be reflected in the .ini file (and also by
                        // DestroyContext).
API const char *SaveIniSettingsToMemory(
    size_t *out_ini_size =
        NULL); // return a zero-terminated string with the .ini data which you
               // can save by your own mean. call when io.WantSaveIniSettings is
               // set, then save data by your own mean and clear
               // io.WantSaveIniSettings.

// Debug Utilities
// - Your main debugging friend is the ShowMetricsWindow() function, which is
// also accessible from Demo->Tools->Metrics Debugger
API void DebugTextEncoding(const char *text);
API void DebugFlashStyleColor(Col idx);
API bool DebugCheckVersionAndDataLayout(
    const char *version_str, size_t sz_io, size_t sz_style, size_t sz_vec2,
    size_t sz_vec4, size_t sz_drawvert,
    size_t sz_drawidx); // This is called by CHECKVERSION() macro.

// Memory Allocators
// - Those functions are not reliant on the current context.
// - DLL users: heaps and globals are not shared across DLL boundaries! You will
// need to call SetCurrentContext() + SetAllocatorFunctions()
//   for each static/DLL boundary you are calling from. Read "Context and Memory
//   Allocators" section of gui.cpp for more details.
API void SetAllocatorFunctions(MemAllocFunc alloc_func, MemFreeFunc free_func,
                               void *user_data = NULL);
API void GetAllocatorFunctions(MemAllocFunc *p_alloc_func,
                               MemFreeFunc *p_free_func, void **p_user_data);
API void *MemAlloc(size_t size);
API void MemFree(void *ptr);

// (Optional) Platform/OS interface for multi-viewport support
// Read comments around the PlatformIO structure for more details.
// Note: You may use GetWindowViewport() to get the current viewport of the
// current window.
API PlatformIO &GetPlatformIO();  // platform/renderer functions, for
                                  // backend to setup + viewports list.
API void UpdatePlatformWindows(); // call in main loop. will call
                                  // CreateWindow/ResizeWindow/etc. platform
// functions for each secondary viewport, and
// DestroyWindow for each inactive viewport.
API void RenderPlatformWindowsDefault(
    void *platform_render_arg = NULL,
    void *renderer_render_arg =
        NULL); // call in main loop. will call RenderWindow/SwapBuffers platform
               // functions for each secondary viewport which doesn't have the
               // ViewportFlags_Minimized flag set. May be reimplemented by
               // user for custom rendering needs.
API void
DestroyPlatformWindows(); // call DestroyWindow platform functions for all
                          // viewports. call from backend Shutdown() if you need
                          // to close platform windows before imgui shutdown.
                          // otherwise will be called by DestroyContext().
API Viewport *FindViewportByID(ID id); // this is a helper for backends.
API Viewport *FindViewportByPlatformHandle(
    void *platform_handle); // this is a helper for backends. the type
                            // platform_handle is decided by the backend (e.g.
                            // HWND, MyWindow*, GLFWwindow* etc.)

} // namespace Gui

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------

// Flags for Gui::Begin()
// (Those are per-window flags. There are shared flags in IO:
// io.ConfigWindowsResizeFromEdges and io.ConfigWindowsMoveFromTitleBarOnly)
enum WindowFlags_ {
  WindowFlags_None = 0,
  WindowFlags_NoTitleBar = 1 << 0, // Disable title-bar
  WindowFlags_NoResize =
      1 << 1, // Disable user resizing with the lower-right grip
  WindowFlags_NoMove = 1 << 2,      // Disable user moving the window
  WindowFlags_NoScrollbar = 1 << 3, // Disable scrollbars (window can still
                                    // scroll with mouse or programmatically)
  WindowFlags_NoScrollWithMouse =
      1 << 4, // Disable user vertically scrolling with mouse wheel. On child
              // window, mouse wheel will be forwarded to the parent unless
              // NoScrollbar is also set.
  WindowFlags_NoCollapse =
      1 << 5, // Disable user collapsing window by double-clicking on it. Also
              // referred to as Window Menu Button (e.g. within a docking node).
  WindowFlags_AlwaysAutoResize =
      1 << 6, // Resize every window to its content every frame
  WindowFlags_NoBackground =
      1 << 7, // Disable drawing background color (WindowBg, etc.) and outside
              // border. Similar as using SetNextWindowBgAlpha(0.0f).
  WindowFlags_NoSavedSettings = 1 << 8, // Never load/save settings in .ini file
  WindowFlags_NoMouseInputs =
      1 << 9, // Disable catching mouse, hovering test with pass through.
  WindowFlags_MenuBar = 1 << 10, // Has a menu-bar
  WindowFlags_HorizontalScrollbar =
      1 << 11, // Allow horizontal scrollbar to appear (off by default). You may
               // use SetNextWindowContentSize(Vec2(width,0.0f)); prior to
               // calling Begin() to specify width. Read code in demo in
               // the "Horizontal Scrolling" section.
  WindowFlags_NoFocusOnAppearing =
      1 << 12, // Disable taking focus when transitioning from hidden to visible
               // state
  WindowFlags_NoBringToFrontOnFocus =
      1 << 13, // Disable bringing window to front when taking focus (e.g.
               // clicking on it or programmatically giving it focus)
  WindowFlags_AlwaysVerticalScrollbar =
      1
      << 14, // Always show vertical scrollbar (even if ContentSize.y < Size.y)
  WindowFlags_AlwaysHorizontalScrollbar =
      1 << 15, // Always show horizontal scrollbar (even if ContentSize.x <
               // Size.x)
  WindowFlags_NoNavInputs =
      1 << 16, // No gamepad/keyboard navigation within the window
  WindowFlags_NoNavFocus =
      1 << 17, // No focusing toward this window with gamepad/keyboard
               // navigation (e.g. skipped by CTRL+TAB)
  WindowFlags_UnsavedDocument =
      1 << 18, // Display a dot next to the title. When used in a tab/docking
               // context, tab is selected when clicking the X + closure is not
               // assumed (will wait for user to stop submitting the tab).
               // Otherwise closure is assumed when pressing the X, so if you
               // keep submitting the tab may reappear at end of tab bar.
  WindowFlags_NoDocking = 1 << 19, // Disable docking of this window
  WindowFlags_NoNav = WindowFlags_NoNavInputs | WindowFlags_NoNavFocus,
  WindowFlags_NoDecoration = WindowFlags_NoTitleBar | WindowFlags_NoResize |
                             WindowFlags_NoScrollbar | WindowFlags_NoCollapse,
  WindowFlags_NoInputs = WindowFlags_NoMouseInputs | WindowFlags_NoNavInputs |
                         WindowFlags_NoNavFocus,

  // [Internal]
  WindowFlags_NavFlattened =
      1 << 23, // [BETA] On child window: share focus scope, allow
               // gamepad/keyboard navigation to cross over parent border to
               // this child or between sibling child windows.
  WindowFlags_ChildWindow =
      1 << 24, // Don't use! For internal use by BeginChild()
  WindowFlags_Tooltip =
      1 << 25,                 // Don't use! For internal use by BeginTooltip()
  WindowFlags_Popup = 1 << 26, // Don't use! For internal use by BeginPopup()
  WindowFlags_Modal =
      1 << 27, // Don't use! For internal use by BeginPopupModal()
  WindowFlags_ChildMenu = 1 << 28, // Don't use! For internal use by BeginMenu()
  WindowFlags_DockNodeHost =
      1 << 29, // Don't use! For internal use by Begin()/NewFrame()

// Obsolete names
#ifndef DISABLE_OBSOLETE_FUNCTIONS
  WindowFlags_AlwaysUseWindowPadding =
      1 << 30, // Obsoleted in 1.90: Use ChildFlags_AlwaysUseWindowPadding
               // in BeginChild() call.
#endif
};

// Flags for Gui::BeginChild()
// (Legacy: bot 0 must always correspond to ChildFlags_Border to be
// backward compatible with old API using 'bool border = false'. About using
// AutoResizeX/AutoResizeY flags:
// - May be combined with SetNextWindowSizeConstraints() to set a min/max size
// for each axis (see "Demo->Child->Auto-resize with Constraints").
// - Size measurement for a given axis is only performed when the child window
// is within visible boundaries, or is just appearing.
//   - This allows BeginChild() to return false when not within boundaries (e.g.
//   when scrolling), which is more optimal. BUT it won't update its auto-size
//   while clipped.
//     While not perfect, it is a better default behavior as the always-on
//     performance gain is more valuable than the occasional "resizing after
//     becoming visible again" glitch.
//   - You may also use ChildFlags_AlwaysAutoResize to force an update even
//   when child window is not in view.
//     HOWEVER PLEASE UNDERSTAND THAT DOING SO WILL PREVENT BeginChild() FROM
//     EVER RETURNING FALSE, disabling benefits of coarse clipping.
enum ChildFlags_ {
  ChildFlags_None = 0,
  ChildFlags_Border =
      1 << 0, // Show an outer border and enable WindowPadding. (Important: this
              // is always == 1 == true for legacy reason)
  ChildFlags_AlwaysUseWindowPadding =
      1 << 1, // Pad with style.WindowPadding even if no border are drawn (no
              // padding by default for non-bordered child windows because it
              // makes more sense)
  ChildFlags_ResizeX =
      1 << 2, // Allow resize from right border (layout direction). Enable .ini
              // saving (unless WindowFlags_NoSavedSettings passed to
              // window flags)
  ChildFlags_ResizeY =
      1 << 3, // Allow resize from bottom border (layout direction). "
  ChildFlags_AutoResizeX = 1
                           << 4, // Enable auto-resizing width. Read "IMPORTANT:
                                 // Size measurement" details above.
  ChildFlags_AutoResizeY = 1
                           << 5, // Enable auto-resizing height. Read
                                 // "IMPORTANT: Size measurement" details above.
  ChildFlags_AlwaysAutoResize =
      1 << 6, // Combined with AutoResizeX/AutoResizeY. Always measure size even
              // when child is hidden, always return true, always disable
              // clipping optimization! NOT RECOMMENDED.
  ChildFlags_FrameStyle =
      1 << 7, // Style the child window like a framed item: use FrameBg,
              // FrameRounding, FrameBorderSize, FramePadding instead of
              // ChildBg, ChildRounding, ChildBorderSize, WindowPadding.
};

// Flags for Gui::InputText()
// (Those are per-item flags. There are shared flags in IO:
// io.ConfigInputTextCursorBlink and io.ConfigInputTextEnterKeepActive)
enum InputTextFlags_ {
  InputTextFlags_None = 0,
  InputTextFlags_CharsDecimal = 1 << 0,     // Allow 0123456789.+-*/
  InputTextFlags_CharsHexadecimal = 1 << 1, // Allow 0123456789ABCDEFabcdef
  InputTextFlags_CharsUppercase = 1 << 2,   // Turn a..z into A..Z
  InputTextFlags_CharsNoBlank = 1 << 3,     // Filter out spaces, tabs
  InputTextFlags_AutoSelectAll =
      1 << 4, // Select entire text when first taking mouse focus
  InputTextFlags_EnterReturnsTrue =
      1 << 5, // Return 'true' when Enter is pressed (as opposed to every time
              // the value was modified). Consider looking at the
              // IsItemDeactivatedAfterEdit() function.
  InputTextFlags_CallbackCompletion =
      1 << 6, // Callback on pressing TAB (for completion handling)
  InputTextFlags_CallbackHistory =
      1 << 7, // Callback on pressing Up/Down arrows (for history handling)
  InputTextFlags_CallbackAlways =
      1 << 8, // Callback on each iteration. User code may query cursor
              // position, modify text buffer.
  InputTextFlags_CallbackCharFilter =
      1 << 9, // Callback on character inputs to replace or discard them. Modify
              // 'EventChar' to replace or discard, or return 1 in callback to
              // discard.
  InputTextFlags_AllowTabInput =
      1 << 10, // Pressing TAB input a '\t' character into the text field
  InputTextFlags_CtrlEnterForNewLine =
      1 << 11, // In multi-line mode, unfocus with Enter, add new line with
               // Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add
               // line with Enter).
  InputTextFlags_NoHorizontalScroll =
      1 << 12, // Disable following the cursor horizontally
  InputTextFlags_AlwaysOverwrite = 1 << 13, // Overwrite mode
  InputTextFlags_ReadOnly = 1 << 14,        // Read-only mode
  InputTextFlags_Password =
      1 << 15, // Password mode, display all characters as '*'
  InputTextFlags_NoUndoRedo =
      1 << 16, // Disable undo/redo. Note that input text owns the text data
               // while active, if you want to provide your own undo/redo stack
               // you need e.g. to call ClearActiveID().
  InputTextFlags_CharsScientific =
      1 << 17, // Allow 0123456789.+-*/eE (Scientific notation input)
  InputTextFlags_CallbackResize =
      1 << 18, // Callback on buffer capacity changes request (beyond 'buf_size'
               // parameter value), allowing the string to grow. Notify when the
               // string wants to be resized (for string types which hold a
               // cache of their Size). You will be provided a new BufSize in
               // the callback and NEED to honor it. (see
               // misc/cpp/stdlib.h for an example of using this)
  InputTextFlags_CallbackEdit =
      1 << 19, // Callback on any edit (note that InputText() already returns
               // true on edit, the callback is useful mainly to manipulate the
               // underlying buffer while focus is active)
  InputTextFlags_EscapeClearsAll =
      1 << 20, // Escape key clears content if not empty, and deactivate
               // otherwise (contrast to default behavior of Escape to revert)

  // Obsolete names
  // InputTextFlags_AlwaysInsertMode  = InputTextFlags_AlwaysOverwrite
  // // [renamed in 1.82] name was not matching behavior
};

// Flags for Gui::TreeNodeEx(), Gui::CollapsingHeader*()
enum TreeNodeFlags_ {
  TreeNodeFlags_None = 0,
  TreeNodeFlags_Selected = 1 << 0, // Draw as selected
  TreeNodeFlags_Framed =
      1 << 1, // Draw frame with background (e.g. for CollapsingHeader)
  TreeNodeFlags_AllowOverlap =
      1 << 2, // Hit testing to allow subsequent widgets to overlap this one
  TreeNodeFlags_NoTreePushOnOpen =
      1 << 3, // Don't do a TreePush() when open (e.g. for CollapsingHeader) =
              // no extra indent nor pushing on ID stack
  TreeNodeFlags_NoAutoOpenOnLog =
      1 << 4, // Don't automatically and temporarily open node when Logging is
              // active (by default logging will automatically open tree nodes)
  TreeNodeFlags_DefaultOpen = 1 << 5,       // Default node to be open
  TreeNodeFlags_OpenOnDoubleClick = 1 << 6, // Need double-click to open node
  TreeNodeFlags_OpenOnArrow =
      1 << 7, // Only open when clicking on the arrow part. If
              // TreeNodeFlags_OpenOnDoubleClick is also set, single-click
              // arrow or double-click all box to open.
  TreeNodeFlags_Leaf =
      1 << 8, // No collapsing, no arrow (use as a convenience for leaf nodes).
  TreeNodeFlags_Bullet =
      1 << 9, // Display a bullet instead of arrow. IMPORTANT: node can still be
              // marked open/close if you don't set the _Leaf flag!
  TreeNodeFlags_FramePadding =
      1 << 10, // Use FramePadding (even for an unframed text node) to
               // vertically align text baseline to regular widget height.
               // Equivalent to calling AlignTextToFramePadding().
  TreeNodeFlags_SpanAvailWidth =
      1 << 11, // Extend hit box to the right-most edge, even if not framed.
               // This is not the default in order to allow adding other items
               // on the same line. In the future we may refactor the hit system
               // to be front-to-back, allowing natural overlaps and then this
               // can become the default.
  TreeNodeFlags_SpanFullWidth =
      1 << 12, // Extend hit box to the left-most and right-most edges (bypass
               // the indented area).
  TreeNodeFlags_SpanAllColumns =
      1 << 13, // Frame will span all columns of its container table (text will
               // still fit in current column)
  TreeNodeFlags_NavLeftJumpsBackHere =
      1 << 14, // (WIP) Nav: left direction may move to this TreeNode() from any
               // of its child (items submitted between TreeNode and TreePop)
  // TreeNodeFlags_NoScrollOnOpen     = 1 << 15,  // FIXME: TODO: Disable
  // automatic scroll on TreePop() if node got just open and contents is not
  // visible
  TreeNodeFlags_CollapsingHeader = TreeNodeFlags_Framed |
                                   TreeNodeFlags_NoTreePushOnOpen |
                                   TreeNodeFlags_NoAutoOpenOnLog,

#ifndef DISABLE_OBSOLETE_FUNCTIONS
  TreeNodeFlags_AllowItemOverlap =
      TreeNodeFlags_AllowOverlap, // Renamed in 1.89.7
#endif
};

// Flags for OpenPopup*(), BeginPopupContext*(), IsPopupOpen() functions.
// - To be backward compatible with older API which took an 'int mouse_button =
// 1' argument, we need to treat
//   small flags values as a mouse button index, so we encode the mouse button
//   in the first few bits of the flags. It is therefore guaranteed to be legal
//   to pass a mouse button index in PopupFlags.
// - For the same reason, we exceptionally default the PopupFlags argument
// of BeginPopupContextXXX functions to 1 instead of 0.
//   IMPORTANT: because the default parameter is 1
//   (==PopupFlags_MouseButtonRight), if you rely on the default parameter
//   and want to use another flag, you need to pass in the
//   PopupFlags_MouseButtonRight flag explicitly.
// - Multiple buttons currently cannot be combined/or-ed in those functions (we
// could allow it later).
enum PopupFlags_ {
  PopupFlags_None = 0,
  PopupFlags_MouseButtonLeft =
      0, // For BeginPopupContext*(): open on Left Mouse release. Guaranteed to
         // always be == 0 (same as MouseButton_Left)
  PopupFlags_MouseButtonRight =
      1, // For BeginPopupContext*(): open on Right Mouse release. Guaranteed to
         // always be == 1 (same as MouseButton_Right)
  PopupFlags_MouseButtonMiddle =
      2, // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed
         // to always be == 2 (same as MouseButton_Middle)
  PopupFlags_MouseButtonMask_ = 0x1F,
  PopupFlags_MouseButtonDefault_ = 1,
  PopupFlags_NoOpenOverExistingPopup =
      1 << 5, // For OpenPopup*(), BeginPopupContext*(): don't open if there's
              // already a popup at the same level of the popup stack
  PopupFlags_NoOpenOverItems =
      1 << 6, // For BeginPopupContextWindow(): don't return true when hovering
              // items, only when hovering empty space
  PopupFlags_AnyPopupId = 1 << 7, // For IsPopupOpen(): ignore the ID
                                  // parameter and test for any popup.
  PopupFlags_AnyPopupLevel =
      1 << 8, // For IsPopupOpen(): search/test at any level of the popup stack
              // (default test in the current level)
  PopupFlags_AnyPopup = PopupFlags_AnyPopupId | PopupFlags_AnyPopupLevel,
};

// Flags for Gui::Selectable()
enum SelectableFlags_ {
  SelectableFlags_None = 0,
  SelectableFlags_DontClosePopups =
      1 << 0, // Clicking this doesn't close parent popup window
  SelectableFlags_SpanAllColumns =
      1 << 1, // Frame will span all columns of its container table (text will
              // still fit in current column)
  SelectableFlags_AllowDoubleClick =
      1 << 2, // Generate press events on double clicks too
  SelectableFlags_Disabled =
      1 << 3, // Cannot be selected, display grayed out text
  SelectableFlags_AllowOverlap =
      1
      << 4, // (WIP) Hit testing to allow subsequent widgets to overlap this one

#ifndef DISABLE_OBSOLETE_FUNCTIONS
  SelectableFlags_AllowItemOverlap =
      SelectableFlags_AllowOverlap, // Renamed in 1.89.7
#endif
};

// Flags for Gui::BeginCombo()
enum ComboFlags_ {
  ComboFlags_None = 0,
  ComboFlags_PopupAlignLeft =
      1 << 0, // Align the popup toward the left by default
  ComboFlags_HeightSmall =
      1 << 1, // Max ~4 items visible. Tip: If you want your combo popup to be a
              // specific size you can use SetNextWindowSizeConstraints() prior
              // to calling BeginCombo()
  ComboFlags_HeightRegular = 1 << 2, // Max ~8 items visible (default)
  ComboFlags_HeightLarge = 1 << 3,   // Max ~20 items visible
  ComboFlags_HeightLargest = 1 << 4, // As many fitting items as possible
  ComboFlags_NoArrowButton =
      1 << 5, // Display on the preview box without the square arrow button
  ComboFlags_NoPreview = 1 << 6, // Display only a square arrow button
  ComboFlags_WidthFitPreview =
      1 << 7, // Width dynamically calculated from preview contents
  ComboFlags_HeightMask_ = ComboFlags_HeightSmall | ComboFlags_HeightRegular |
                           ComboFlags_HeightLarge | ComboFlags_HeightLargest,
};

// Flags for Gui::BeginTabBar()
enum TabBarFlags_ {
  TabBarFlags_None = 0,
  TabBarFlags_Reorderable =
      1 << 0, // Allow manually dragging tabs to re-order them + New tabs are
              // appended at the end of list
  TabBarFlags_AutoSelectNewTabs =
      1 << 1, // Automatically select new tabs when they appear
  TabBarFlags_TabListPopupButton =
      1 << 2, // Disable buttons to open the tab list popup
  TabBarFlags_NoCloseWithMiddleMouseButton =
      1 << 3, // Disable behavior of closing tabs (that are submitted with
              // p_open != NULL) with middle mouse button. You may handle this
              // behavior manually on user's side with if (IsItemHovered() &&
              // IsMouseClicked(2)) *p_open = false.
  TabBarFlags_NoTabListScrollingButtons =
      1 << 4, // Disable scrolling buttons (apply when fitting policy is
              // TabBarFlags_FittingPolicyScroll)
  TabBarFlags_NoTooltip = 1 << 5, // Disable tooltips when hovering a tab
  TabBarFlags_FittingPolicyResizeDown = 1
                                        << 6, // Resize tabs when they don't fit
  TabBarFlags_FittingPolicyScroll =
      1 << 7, // Add scroll buttons when tabs don't fit
  TabBarFlags_FittingPolicyMask_ =
      TabBarFlags_FittingPolicyResizeDown | TabBarFlags_FittingPolicyScroll,
  TabBarFlags_FittingPolicyDefault_ = TabBarFlags_FittingPolicyResizeDown,
};

// Flags for Gui::BeginTabItem()
enum TabItemFlags_ {
  TabItemFlags_None = 0,
  TabItemFlags_UnsavedDocument = 1 << 0, // Display a dot next to the title +
                                         // set TabItemFlags_NoAssumedClosure.
  TabItemFlags_SetSelected = 1
                             << 1, // Trigger flag to programmatically make the
                                   // tab selected when calling BeginTabItem()
  TabItemFlags_NoCloseWithMiddleMouseButton =
      1 << 2, // Disable behavior of closing tabs (that are submitted with
              // p_open != NULL) with middle mouse button. You may handle this
              // behavior manually on user's side with if (IsItemHovered() &&
              // IsMouseClicked(2)) *p_open = false.
  TabItemFlags_NoPushId =
      1 << 3, // Don't call PushID()/PopID() on BeginTabItem()/EndTabItem()
  TabItemFlags_NoTooltip = 1 << 4, // Disable tooltip for the given tab
  TabItemFlags_NoReorder = 1 << 5, // Disable reordering this tab or having
                                   // another tab cross over this tab
  TabItemFlags_Leading = 1 << 6,  // Enforce the tab position to the left of the
                                  // tab bar (after the tab list popup button)
  TabItemFlags_Trailing = 1 << 7, // Enforce the tab position to the right of
                                  // the tab bar (before the scrolling buttons)
  TabItemFlags_NoAssumedClosure =
      1 << 8, // Tab is selected when trying to close + closure is not
              // immediately assumed (will wait for user to stop submitting the
              // tab). Otherwise closure is assumed when pressing the X, so if
              // you keep submitting the tab may reappear at end of tab bar.
};

// Flags for Gui::IsWindowFocused()
enum FocusedFlags_ {
  FocusedFlags_None = 0,
  FocusedFlags_ChildWindows =
      1 << 0, // Return true if any children of the window is focused
  FocusedFlags_RootWindow =
      1
      << 1, // Test from root window (top most parent of the current hierarchy)
  FocusedFlags_AnyWindow =
      1
      << 2, // Return true if any window is focused. Important: If you are
            // trying to tell how to dispatch your low-level inputs, do NOT use
            // this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
  FocusedFlags_NoPopupHierarchy =
      1 << 3, // Do not consider popup hierarchy (do not treat popup emitter as
              // parent of popup) (when used with _ChildWindows or _RootWindow)
  FocusedFlags_DockHierarchy =
      1 << 4, // Consider docking hierarchy (treat dockspace host as parent of
              // docked window) (when used with _ChildWindows or _RootWindow)
  FocusedFlags_RootAndChildWindows =
      FocusedFlags_RootWindow | FocusedFlags_ChildWindows,
};

// Flags for Gui::IsItemHovered(), Gui::IsWindowHovered()
// Note: if you are trying to check whether your mouse should be dispatched to
// Gui or to your app, you should use 'io.WantCaptureMouse' instead!
// Please read the FAQ! Note: windows with the WindowFlags_NoInputs flag
// are ignored by IsWindowHovered() calls.
enum HoveredFlags_ {
  HoveredFlags_None = 0, // Return true if directly over the item/window, not
                         // obstructed by another window, not obstructed by an
                         // active popup or modal blocking inputs under them.
  HoveredFlags_ChildWindows = 1 << 0, // IsWindowHovered() only: Return true if
                                      // any children of the window is hovered
  HoveredFlags_RootWindow =
      1 << 1, // IsWindowHovered() only: Test from root window (top most parent
              // of the current hierarchy)
  HoveredFlags_AnyWindow =
      1 << 2, // IsWindowHovered() only: Return true if any window is hovered
  HoveredFlags_NoPopupHierarchy =
      1 << 3, // IsWindowHovered() only: Do not consider popup hierarchy (do not
              // treat popup emitter as parent of popup) (when used with
              // _ChildWindows or _RootWindow)
  HoveredFlags_DockHierarchy =
      1 << 4, // IsWindowHovered() only: Consider docking hierarchy (treat
              // dockspace host as parent of docked window) (when used with
              // _ChildWindows or _RootWindow)
  HoveredFlags_AllowWhenBlockedByPopup =
      1 << 5, // Return true even if a popup window is normally blocking access
              // to this item/window
  // HoveredFlags_AllowWhenBlockedByModal     = 1 << 6,   // Return true
  // even if a modal popup window is normally blocking access to this
  // item/window. FIXME-TODO: Unavailable yet.
  HoveredFlags_AllowWhenBlockedByActiveItem =
      1 << 7, // Return true even if an active item is blocking access to this
              // item/window. Useful for Drag and Drop patterns.
  HoveredFlags_AllowWhenOverlappedByItem =
      1 << 8, // IsItemHovered() only: Return true even if the item uses
              // AllowOverlap mode and is overlapped by another hoverable item.
  HoveredFlags_AllowWhenOverlappedByWindow =
      1 << 9, // IsItemHovered() only: Return true even if the position is
              // obstructed or overlapped by another window.
  HoveredFlags_AllowWhenDisabled =
      1 << 10, // IsItemHovered() only: Return true even if the item is disabled
  HoveredFlags_NoNavOverride =
      1 << 11, // IsItemHovered() only: Disable using gamepad/keyboard
               // navigation state when active, always query mouse
  HoveredFlags_AllowWhenOverlapped = HoveredFlags_AllowWhenOverlappedByItem |
                                     HoveredFlags_AllowWhenOverlappedByWindow,
  HoveredFlags_RectOnly = HoveredFlags_AllowWhenBlockedByPopup |
                          HoveredFlags_AllowWhenBlockedByActiveItem |
                          HoveredFlags_AllowWhenOverlapped,
  HoveredFlags_RootAndChildWindows =
      HoveredFlags_RootWindow | HoveredFlags_ChildWindows,

  // Tooltips mode
  // - typically used in IsItemHovered() + SetTooltip() sequence.
  // - this is a shortcut to pull flags from 'style.HoverFlagsForTooltipMouse'
  // or 'style.HoverFlagsForTooltipNav' where you can reconfigure desired
  // behavior.
  //   e.g. 'TooltipHoveredFlagsForMouse' defaults to
  //   'HoveredFlags_Stationary | HoveredFlags_DelayShort'.
  // - for frequently actioned or hovered items providing a tooltip, you want
  // may to use HoveredFlags_ForTooltip (stationary + delay) so the tooltip
  // doesn't show too often.
  // - for items which main purpose is to be hovered, or items with low
  // affordance, or in less consistent apps, prefer no delay or shorter delay.
  HoveredFlags_ForTooltip = 1 << 12, // Shortcut for standard flags when using
                                     // IsItemHovered() + SetTooltip() sequence.

  // (Advanced) Mouse Hovering delays.
  // - generally you can use HoveredFlags_ForTooltip to use
  // application-standardized flags.
  // - use those if you need specific overrides.
  HoveredFlags_Stationary =
      1 << 13, // Require mouse to be stationary for style.HoverStationaryDelay
               // (~0.15 sec) _at least one time_. After this, can move on same
               // item/window. Using the stationary test tends to reduces the
               // need for a long delay.
  HoveredFlags_DelayNone =
      1 << 14, // IsItemHovered() only: Return true immediately (default). As
               // this is the default you generally ignore this.
  HoveredFlags_DelayShort =
      1 << 15, // IsItemHovered() only: Return true after style.HoverDelayShort
               // elapsed (~0.15 sec) (shared between items) + requires mouse to
               // be stationary for style.HoverStationaryDelay (once per item).
  HoveredFlags_DelayNormal =
      1 << 16, // IsItemHovered() only: Return true after style.HoverDelayNormal
               // elapsed (~0.40 sec) (shared between items) + requires mouse to
               // be stationary for style.HoverStationaryDelay (once per item).
  HoveredFlags_NoSharedDelay =
      1 << 17, // IsItemHovered() only: Disable shared delay system where moving
               // from one item to the next keeps the previous timer for a short
               // time (standard for tooltips with long delays)
};

// Flags for Gui::DockSpace(), shared/inherited by child nodes.
// (Some flags can be applied to individual nodes directly)
// FIXME-DOCK: Also see DockNodeFlagsPrivate_ which may involve using the
// WIP and internal DockBuilder api.
enum DockNodeFlags_ {
  DockNodeFlags_None = 0,
  DockNodeFlags_KeepAliveOnly =
      1
      << 0, //       // Don't display the dockspace node but keep it alive.
            //       Windows docked into this dockspace node won't be undocked.
  // DockNodeFlags_NoCentralNode              = 1 << 1,   //       //
  // Disable Central Node (the node which can stay empty)
  DockNodeFlags_NoDockingOverCentralNode =
      1 << 2, //       // Disable docking over the Central Node, which will be
              //       always kept empty.
  DockNodeFlags_PassthruCentralNode =
      1 << 3, //       // Enable passthru dockspace: 1) DockSpace() will render
              //       a Col_WindowBg background covering everything
              //       excepted the Central Node when empty. Meaning the host
              //       window should probably use SetNextWindowBgAlpha(0.0f)
              //       prior to Begin() when using this. 2) When Central Node is
              //       empty: let inputs pass-through + won't display a
              //       DockingEmptyBg background. See demo for details.
  DockNodeFlags_NoDockingSplit =
      1 << 4, //       // Disable other windows/nodes from splitting this node.
  DockNodeFlags_NoResize =
      1 << 5, // Saved // Disable resizing node using the splitter/separators.
              // Useful with programmatically setup dockspaces.
  DockNodeFlags_AutoHideTabBar =
      1 << 6, //       // Tab bar will automatically hide when there is a single
              //       window in the dock node.
  DockNodeFlags_NoUndocking = 1 << 7, //       // Disable undocking this node.

#ifndef DISABLE_OBSOLETE_FUNCTIONS
  DockNodeFlags_NoSplit = DockNodeFlags_NoDockingSplit, // Renamed in 1.90
  DockNodeFlags_NoDockingInCentralNode =
      DockNodeFlags_NoDockingOverCentralNode, // Renamed in 1.90
#endif
};

// Flags for Gui::BeginDragDropSource(), Gui::AcceptDragDropPayload()
enum DragDropFlags_ {
  DragDropFlags_None = 0,
  // BeginDragDropSource() flags
  DragDropFlags_SourceNoPreviewTooltip =
      1 << 0, // Disable preview tooltip. By default, a successful call to
              // BeginDragDropSource opens a tooltip so you can display a
              // preview or description of the source contents. This flag
              // disables this behavior.
  DragDropFlags_SourceNoDisableHover =
      1 << 1, // By default, when dragging we clear data so that IsItemHovered()
              // will return false, to avoid subsequent user code submitting
              // tooltips. This flag disables this behavior so you can still
              // call IsItemHovered() on the source item.
  DragDropFlags_SourceNoHoldToOpenOthers =
      1 << 2, // Disable the behavior that allows to open tree nodes and
              // collapsing header by holding over them while dragging a source
              // item.
  DragDropFlags_SourceAllowNullID =
      1 << 3, // Allow items such as Text(), Image() that have no unique
              // identifier to be used as drag source, by manufacturing a
              // temporary identifier based on their window-relative position.
              // This is extremely unusual within the gui ecosystem and
              // so we made it explicit.
  DragDropFlags_SourceExtern =
      1 << 4, // External source (from outside of gui), won't attempt to
              // read current item/window info. Will always return true. Only
              // one Extern source can be active simultaneously.
  DragDropFlags_SourceAutoExpirePayload =
      1
      << 5, // Automatically expire the payload if the source cease to be
            // submitted (otherwise payloads are persisting while being dragged)
  // AcceptDragDropPayload() flags
  DragDropFlags_AcceptBeforeDelivery =
      1 << 10, // AcceptDragDropPayload() will returns true even before the
               // mouse button is released. You can then call IsDelivery() to
               // test if the payload needs to be delivered.
  DragDropFlags_AcceptNoDrawDefaultRect =
      1 << 11, // Do not draw the default highlight rectangle when hovering over
               // target.
  DragDropFlags_AcceptNoPreviewTooltip =
      1 << 12, // Request hiding the BeginDragDropSource tooltip from the
               // BeginDragDropTarget site.
  DragDropFlags_AcceptPeekOnly =
      DragDropFlags_AcceptBeforeDelivery |
      DragDropFlags_AcceptNoDrawDefaultRect, // For peeking ahead and
                                             // inspecting the payload
                                             // before delivery.
};

// Standard Drag and Drop payload types. You can define you own payload types
// using short strings. Types starting with '_' are defined by Gui.
#define PAYLOAD_TYPE_COLOR_3F                                                  \
  "_COL3F" // float[3]: Standard type for colors, without alpha. User code may
           // use this type.
#define PAYLOAD_TYPE_COLOR_4F                                                  \
  "_COL4F" // float[4]: Standard type for colors. User code may use this type.

// A primary data type
enum DataType_ {
  DataType_S8,     // signed char / char (with sensible compilers)
  DataType_U8,     // unsigned char
  DataType_S16,    // short
  DataType_U16,    // unsigned short
  DataType_S32,    // int
  DataType_U32,    // unsigned int
  DataType_S64,    // long long / __int64
  DataType_U64,    // unsigned long long / unsigned __int64
  DataType_Float,  // float
  DataType_Double, // double
  DataType_COUNT
};

// A cardinal direction
enum Dir_ {
  Dir_None = -1,
  Dir_Left = 0,
  Dir_Right = 1,
  Dir_Up = 2,
  Dir_Down = 3,
  Dir_COUNT
};

// A sorting direction
enum SortDirection_ {
  SortDirection_None = 0,
  SortDirection_Ascending = 1, // Ascending = 0->9, A->Z etc.
  SortDirection_Descending = 2 // Descending = 9->0, Z->A etc.
};

// Since 1.90, defining DISABLE_OBSOLETE_FUNCTIONS automatically defines
// DISABLE_OBSOLETE_KEYIO as well.
#if defined(DISABLE_OBSOLETE_FUNCTIONS) && !defined(DISABLE_OBSOLETE_KEYIO)
#define DISABLE_OBSOLETE_KEYIO
#endif

// A key identifier (Key_XXX or Mod_XXX value): can represent
// Keyboard, Mouse and Gamepad values. All our named keys are >= 512. Keys value
// 0 to 511 are left unused as legacy native/opaque key values (< 1.87). Since
// >= 1.89 we increased typing (went from int to enum), some legacy code may
// need a cast to Key.
// Physical keys and are not the same concept as input "Characters", the later
// are submitted via io.AddInputCharacter().
enum Key : int {
  // Keyboard
  Key_None = 0,
  Key_Tab = 512, // == Key_NamedKey_BEGIN
  Key_LeftArrow,
  Key_RightArrow,
  Key_UpArrow,
  Key_DownArrow,
  Key_PageUp,
  Key_PageDown,
  Key_Home,
  Key_End,
  Key_Insert,
  Key_Delete,
  Key_Backspace,
  Key_Space,
  Key_Enter,
  Key_Escape,
  Key_LeftCtrl,
  Key_LeftShift,
  Key_LeftAlt,
  Key_LeftSuper,
  Key_RightCtrl,
  Key_RightShift,
  Key_RightAlt,
  Key_RightSuper,
  Key_Menu,
  Key_0,
  Key_1,
  Key_2,
  Key_3,
  Key_4,
  Key_5,
  Key_6,
  Key_7,
  Key_8,
  Key_9,
  Key_A,
  Key_B,
  Key_C,
  Key_D,
  Key_E,
  Key_F,
  Key_G,
  Key_H,
  Key_I,
  Key_J,
  Key_K,
  Key_L,
  Key_M,
  Key_N,
  Key_O,
  Key_P,
  Key_Q,
  Key_R,
  Key_S,
  Key_T,
  Key_U,
  Key_V,
  Key_W,
  Key_X,
  Key_Y,
  Key_Z,
  Key_F1,
  Key_F2,
  Key_F3,
  Key_F4,
  Key_F5,
  Key_F6,
  Key_F7,
  Key_F8,
  Key_F9,
  Key_F10,
  Key_F11,
  Key_F12,
  Key_F13,
  Key_F14,
  Key_F15,
  Key_F16,
  Key_F17,
  Key_F18,
  Key_F19,
  Key_F20,
  Key_F21,
  Key_F22,
  Key_F23,
  Key_F24,
  Key_Apostrophe,   // '
  Key_Comma,        // ,
  Key_Minus,        // -
  Key_Period,       // .
  Key_Slash,        // /
  Key_Semicolon,    // ;
  Key_Equal,        // =
  Key_LeftBracket,  // [
  Key_Backslash,    // \ (this text inhibit multiline comment caused by
                    // backslash)
  Key_RightBracket, // ]
  Key_GraveAccent,  // `
  Key_CapsLock,
  Key_ScrollLock,
  Key_NumLock,
  Key_PrintScreen,
  Key_Pause,
  Key_Keypad0,
  Key_Keypad1,
  Key_Keypad2,
  Key_Keypad3,
  Key_Keypad4,
  Key_Keypad5,
  Key_Keypad6,
  Key_Keypad7,
  Key_Keypad8,
  Key_Keypad9,
  Key_KeypadDecimal,
  Key_KeypadDivide,
  Key_KeypadMultiply,
  Key_KeypadSubtract,
  Key_KeypadAdd,
  Key_KeypadEnter,
  Key_KeypadEqual,
  Key_AppBack, // Available on some keyboard/mouses. Often referred as
               // "Browser Back"
  Key_AppForward,

  // Gamepad (some of those are analog values, 0.0f to 1.0f)
  Key_GamepadStart,       // Menu (Xbox)      + (Switch)   Start/Options (PS)
  Key_GamepadBack,        // View (Xbox)      - (Switch)   Share (PS)
  Key_GamepadFaceLeft,    // X (Xbox)         Y (Switch)   Square (PS) // Tap:
                          // Toggle Menu. Hold: Windowing mode
                          // (Focus/Move/Resize windows)
  Key_GamepadFaceRight,   // B (Xbox)         A (Switch)   Circle (PS) //
                          // Cancel / Close / Exit
  Key_GamepadFaceUp,      // Y (Xbox)         X (Switch)   Triangle (PS)      //
                          // Text Input / On-screen Keyboard
  Key_GamepadFaceDown,    // A (Xbox)         B (Switch)   Cross (PS) //
                          // Activate / Open / Toggle / Tweak
  Key_GamepadDpadLeft,    // D-pad Left // Move / Tweak / Resize Window (in
                          // Windowing mode)
  Key_GamepadDpadRight,   // D-pad Right // Move / Tweak / Resize Window (in
                          // Windowing mode)
  Key_GamepadDpadUp,      // D-pad Up                                         //
                          // Move / Tweak / Resize Window (in Windowing mode)
  Key_GamepadDpadDown,    // D-pad Down // Move / Tweak / Resize Window (in
                          // Windowing mode)
  Key_GamepadL1,          // L Bumper (Xbox)  L (Switch)   L1 (PS)            //
                          // Tweak Slower / Focus Previous (in Windowing mode)
  Key_GamepadR1,          // R Bumper (Xbox)  R (Switch)   R1 (PS)            //
                          // Tweak Faster / Focus Next (in Windowing mode)
  Key_GamepadL2,          // L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
  Key_GamepadR2,          // R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
  Key_GamepadL3,          // L Stick (Xbox)   L3 (Switch)  L3 (PS)
  Key_GamepadR3,          // R Stick (Xbox)   R3 (Switch)  R3 (PS)
  Key_GamepadLStickLeft,  // [Analog] // Move Window (in Windowing mode)
  Key_GamepadLStickRight, // [Analog] // Move Window (in Windowing mode)
  Key_GamepadLStickUp,    // [Analog] // Move Window (in Windowing mode)
  Key_GamepadLStickDown,  // [Analog] // Move Window (in Windowing mode)
  Key_GamepadRStickLeft,  // [Analog]
  Key_GamepadRStickRight, // [Analog]
  Key_GamepadRStickUp,    // [Analog]
  Key_GamepadRStickDown,  // [Analog]

  // Aliases: Mouse Buttons (auto-submitted from AddMouseButtonEvent() calls)
  // - This is mirroring the data also written to io.MouseDown[], io.MouseWheel,
  // in a format allowing them to be accessed via standard key API.
  Key_MouseLeft,
  Key_MouseRight,
  Key_MouseMiddle,
  Key_MouseX1,
  Key_MouseX2,
  Key_MouseWheelX,
  Key_MouseWheelY,

  // [Internal] Reserved for mod storage
  Key_ReservedForModCtrl,
  Key_ReservedForModShift,
  Key_ReservedForModAlt,
  Key_ReservedForModSuper,
  Key_COUNT,

  // Keyboard Modifiers (explicitly submitted by backend via AddKeyEvent()
  // calls)
  // - This is mirroring the data also written to io.KeyCtrl, io.KeyShift,
  // io.KeyAlt, io.KeySuper, in a format allowing
  //   them to be accessed via standard key API, allowing calls such as
  //   IsKeyPressed(), IsKeyReleased(), querying duration etc.
  // - Code polling every key (e.g. an interface to detect a key press for input
  // mapping) might want to ignore those
  //   and prefer using the real keys (e.g. Key_LeftCtrl,
  //   Key_RightCtrl instead of Mod_Ctrl).
  // - In theory the value of keyboard modifiers should be roughly equivalent to
  // a logical or of the equivalent left/right keys.
  //   In practice: it's complicated; mods are often provided from different
  //   sources. Keyboard layout, IME, sticky keys and backends tend to interfere
  //   and break that equivalence. The safer decision is to relay that ambiguity
  //   down to the end-user...
  Mod_None = 0,
  Mod_Ctrl = 1 << 12,     // Ctrl
  Mod_Shift = 1 << 13,    // Shift
  Mod_Alt = 1 << 14,      // Option/Menu
  Mod_Super = 1 << 15,    // Cmd/Super/Windows
  Mod_Shortcut = 1 << 11, // Alias for Ctrl (non-macOS) _or_ Super (macOS).
  Mod_Mask_ = 0xF800,     // 5-bits

  // [Internal] Prior to 1.87 we required user to fill io.KeysDown[512] using
  // their own native index + the io.KeyMap[] array. We are ditching this method
  // but keeping a legacy path for user code doing e.g.
  // IsKeyPressed(MY_NATIVE_KEY_CODE) If you need to iterate all keys (for e.g.
  // an input mapper) you may use
  // Key_NamedKey_BEGIN..Key_NamedKey_END.
  Key_NamedKey_BEGIN = 512,
  Key_NamedKey_END = Key_COUNT,
  Key_NamedKey_COUNT = Key_NamedKey_END - Key_NamedKey_BEGIN,
#ifdef DISABLE_OBSOLETE_KEYIO
  Key_KeysData_SIZE =
      Key_NamedKey_COUNT, // Size of KeysData[]: only hold named keys
  Key_KeysData_OFFSET =
      Key_NamedKey_BEGIN, // Accesses to io.KeysData[] must use (key -
                          // Key_KeysData_OFFSET) index.
#else
  Key_KeysData_SIZE = Key_COUNT, // Size of KeysData[]: hold legacy
                                 // 0..512 keycodes + named keys
  Key_KeysData_OFFSET = 0,       // Accesses to io.KeysData[] must use (key -
                                 // Key_KeysData_OFFSET) index.
#endif

#ifndef DISABLE_OBSOLETE_FUNCTIONS
  Key_ModCtrl = Mod_Ctrl,
  Key_ModShift = Mod_Shift,
  Key_ModAlt = Mod_Alt,
  Key_ModSuper = Mod_Super, // Renamed in 1.89
                            // Key_KeyPadEnter = Key_KeypadEnter, //
                            // Renamed in 1.87
#endif
};

#ifndef DISABLE_OBSOLETE_KEYIO
// OBSOLETED in 1.88 (from July 2022): NavInput and io.NavInputs[].
// Official backends between 1.60 and 1.86: will keep working and feed gamepad
// inputs as long as DISABLE_OBSOLETE_KEYIO is not set. Custom backends: feed
// gamepad inputs via io.AddKeyEvent() and Key_GamepadXXX enums.
enum NavInput {
  NavInput_Activate,
  NavInput_Cancel,
  NavInput_Input,
  NavInput_Menu,
  NavInput_DpadLeft,
  NavInput_DpadRight,
  NavInput_DpadUp,
  NavInput_DpadDown,
  NavInput_LStickLeft,
  NavInput_LStickRight,
  NavInput_LStickUp,
  NavInput_LStickDown,
  NavInput_FocusPrev,
  NavInput_FocusNext,
  NavInput_TweakSlow,
  NavInput_TweakFast,
  NavInput_COUNT,
};
#endif

// Configuration flags stored in io.ConfigFlags. Set by user/application.
enum ConfigFlags_ {
  ConfigFlags_None = 0,
  ConfigFlags_NavEnableKeyboard =
      1 << 0, // Master keyboard navigation enable flag. Enable full Tabbing +
              // directional arrows + space/enter to activate.
  ConfigFlags_NavEnableGamepad =
      1 << 1, // Master gamepad navigation enable flag. Backend also needs to
              // set BackendFlags_HasGamepad.
  ConfigFlags_NavEnableSetMousePos =
      1 << 2, // Instruct navigation to move the mouse cursor. May be useful on
              // TV/console systems where moving a virtual mouse is awkward.
              // Will update io.MousePos and set io.WantSetMousePos=true. If
              // enabled you MUST honor io.WantSetMousePos requests in your
              // backend, otherwise Gui will react as if the mouse is jumping
              // around back and forth.
  ConfigFlags_NavNoCaptureKeyboard =
      1 << 3, // Instruct navigation to not set the io.WantCaptureKeyboard flag
              // when io.NavActive is set.
  ConfigFlags_NoMouse =
      1 << 4, // Instruct imgui to clear mouse position/buttons in NewFrame().
              // This allows ignoring the mouse information set by the backend.
  ConfigFlags_NoMouseCursorChange =
      1 << 5, // Instruct backend to not alter mouse cursor shape and
              // visibility. Use if the backend cursor changes are interfering
              // with yours and you don't want to use SetMouseCursor() to change
              // mouse cursor. You may want to honor requests from imgui by
              // reading GetMouseCursor() yourself instead.

  // [BETA] Docking
  ConfigFlags_DockingEnable = 1 << 6, // Docking enable flags.

  // [BETA] Viewports
  // When using viewports it is recommended that your default value for
  // Col_WindowBg is opaque (Alpha=1.0) so transition to a viewport won't
  // be noticeable.
  ConfigFlags_ViewportsEnable = 1 << 10, // Viewport enable flags (require both
                                         // BackendFlags_PlatformHasViewports +
                                         // BackendFlags_RendererHasViewports
                                         // set by the respective backends)
  ConfigFlags_DpiEnableScaleViewports =
      1 << 14, // [BETA: Don't use] FIXME-DPI: Reposition and resize imgui
               // windows when the DpiScale of a viewport changed (mostly useful
               // for the main viewport hosting other window). Note that
               // resizing the main window itself is up to your application.
  ConfigFlags_DpiEnableScaleFonts =
      1 << 15, // [BETA: Don't use] FIXME-DPI: Request bitmap-scaled fonts to
               // match DpiScale. This is a very low-quality workaround. The
               // correct way to handle DPI is _currently_ to replace the atlas
               // and/or fonts in the Platform_OnChangedViewport callback, but
               // this is all early work in progress.

  // User storage (to allow your backend/engine to communicate to code that may
  // be shared between multiple projects. Those flags are NOT used by core
  // Gui)
  ConfigFlags_IsSRGB = 1 << 20, // Application is SRGB-aware.
  ConfigFlags_IsTouchScreen =
      1 << 21, // Application is using a touch screen instead of a mouse.
};

// Backend capabilities flags stored in io.BackendFlags. Set by xxx
// or custom backend.
enum BackendFlags_ {
  BackendFlags_None = 0,
  BackendFlags_HasGamepad = 1 << 0, // Backend Platform supports gamepad
                                    // and currently has one connected.
  BackendFlags_HasMouseCursors =
      1 << 1, // Backend Platform supports honoring GetMouseCursor() value to
              // change the OS cursor shape.
  BackendFlags_HasSetMousePos =
      1 << 2, // Backend Platform supports io.WantSetMousePos requests to
              // reposition the OS mouse position (only used if
              // ConfigFlags_NavEnableSetMousePos is set).
  BackendFlags_RendererHasVtxOffset =
      1 << 3, // Backend Renderer supports DrawCmd::VtxOffset. This enables
              // output of large meshes (64K+ vertices) while still using 16-bit
              // indices.

  // [BETA] Viewports
  BackendFlags_PlatformHasViewports =
      1 << 10, // Backend Platform supports multiple viewports.
  BackendFlags_HasMouseHoveredViewport =
      1 << 11, // Backend Platform supports calling io.AddMouseViewportEvent()
               // with the viewport under the mouse. IF POSSIBLE, ignore
               // viewports with the ViewportFlags_NoInputs flag (Win32
               // backend, GLFW 3.30+ backend can do this, SDL backend cannot).
               // If this cannot be done, Gui needs to use a flawed
               // heuristic to find the viewport under.
  BackendFlags_RendererHasViewports =
      1 << 12, // Backend Renderer supports multiple viewports.
};

// Enumeration for PushStyleColor() / PopStyleColor()
enum Col_ {
  Col_Text,
  Col_TextDisabled,
  Col_WindowBg, // Background of normal windows
  Col_ChildBg,  // Background of child windows
  Col_PopupBg,  // Background of popups, menus, tooltips windows
  Col_Border,
  Col_BorderShadow,
  Col_FrameBg, // Background of checkbox, radio button, plot, slider, text
               // input
  Col_FrameBgHovered,
  Col_FrameBgActive,
  Col_TitleBg,          // Title bar
  Col_TitleBgActive,    // Title bar when focused
  Col_TitleBgCollapsed, // Title bar when collapsed
  Col_MenuBarBg,
  Col_ScrollbarBg,
  Col_ScrollbarGrab,
  Col_ScrollbarGrabHovered,
  Col_ScrollbarGrabActive,
  Col_CheckMark, // Checkbox tick and RadioButton circle
  Col_SliderGrab,
  Col_SliderGrabActive,
  Col_Button,
  Col_ButtonHovered,
  Col_ButtonActive,
  Col_Header, // Header* colors are used for CollapsingHeader, TreeNode,
              // Selectable, MenuItem
  Col_HeaderHovered,
  Col_HeaderActive,
  Col_Separator,
  Col_SeparatorHovered,
  Col_SeparatorActive,
  Col_ResizeGrip, // Resize grip in lower-right and lower-left corners of
                  // windows.
  Col_ResizeGripHovered,
  Col_ResizeGripActive,
  Col_Tab, // TabItem in a TabBar
  Col_TabHovered,
  Col_TabActive,
  Col_TabUnfocused,
  Col_TabUnfocusedActive,
  Col_DockingPreview, // Preview overlay color when about to docking
                      // something
  Col_DockingEmptyBg, // Background color for empty node (e.g. CentralNode
                      // with no window docked into it)
  Col_PlotLines,
  Col_PlotLinesHovered,
  Col_PlotHistogram,
  Col_PlotHistogramHovered,
  Col_TableHeaderBg,     // Table header background
  Col_TableBorderStrong, // Table outer and header borders (prefer using
                         // Alpha=1.0 here)
  Col_TableBorderLight,  // Table inner borders (prefer using Alpha=1.0
                         // here)
  Col_TableRowBg,        // Table row background (even rows)
  Col_TableRowBgAlt,     // Table row background (odd rows)
  Col_TextSelectedBg,
  Col_DragDropTarget,        // Rectangle highlighting a drop target
  Col_NavHighlight,          // Gamepad/keyboard: current highlighted item
  Col_NavWindowingHighlight, // Highlight window when using CTRL+TAB
  Col_NavWindowingDimBg,     // Darken/colorize entire screen behind the
                             // CTRL+TAB window list, when active
  Col_ModalWindowDimBg,      // Darken/colorize entire screen behind a modal
                             // window, when one is active
  Col_COUNT
};

// Enumeration for PushStyleVar() / PopStyleVar() to temporarily modify the
// Style structure.
// - The enum only refers to fields of Style which makes sense to be
// pushed/popped inside UI code.
//   During initialization or between frames, feel free to just poke into
//   Style directly.
// - Tip: Use your programming IDE navigation facilities on the names in the
// _second column_ below to find the actual members and their description.
//   In Visual Studio IDE: CTRL+comma ("Edit.GoToAll") can follow symbols in
//   comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot. With Visual
//   Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow
//   symbols in comments.
// - When changing this enum, you need to update the associated internal table
// GStyleVarInfo[] accordingly. This is where we link enum values to members
// offset/type.
enum StyleVar_ {
  // Enum name --------------------- // Member in Style structure (see
  // Style for descriptions)
  StyleVar_Alpha,                   // float     Alpha
  StyleVar_DisabledAlpha,           // float     DisabledAlpha
  StyleVar_WindowPadding,           // Vec2    WindowPadding
  StyleVar_WindowRounding,          // float     WindowRounding
  StyleVar_WindowBorderSize,        // float     WindowBorderSize
  StyleVar_WindowMinSize,           // Vec2    WindowMinSize
  StyleVar_WindowTitleAlign,        // Vec2    WindowTitleAlign
  StyleVar_ChildRounding,           // float     ChildRounding
  StyleVar_ChildBorderSize,         // float     ChildBorderSize
  StyleVar_PopupRounding,           // float     PopupRounding
  StyleVar_PopupBorderSize,         // float     PopupBorderSize
  StyleVar_FramePadding,            // Vec2    FramePadding
  StyleVar_FrameRounding,           // float     FrameRounding
  StyleVar_FrameBorderSize,         // float     FrameBorderSize
  StyleVar_ItemSpacing,             // Vec2    ItemSpacing
  StyleVar_ItemInnerSpacing,        // Vec2    ItemInnerSpacing
  StyleVar_IndentSpacing,           // float     IndentSpacing
  StyleVar_CellPadding,             // Vec2    CellPadding
  StyleVar_ScrollbarSize,           // float     ScrollbarSize
  StyleVar_ScrollbarRounding,       // float     ScrollbarRounding
  StyleVar_GrabMinSize,             // float     GrabMinSize
  StyleVar_GrabRounding,            // float     GrabRounding
  StyleVar_TabRounding,             // float     TabRounding
  StyleVar_TabBarBorderSize,        // float     TabBarBorderSize
  StyleVar_ButtonTextAlign,         // Vec2    ButtonTextAlign
  StyleVar_SelectableTextAlign,     // Vec2    SelectableTextAlign
  StyleVar_SeparatorTextBorderSize, // float  SeparatorTextBorderSize
  StyleVar_SeparatorTextAlign,      // Vec2    SeparatorTextAlign
  StyleVar_SeparatorTextPadding,    // Vec2    SeparatorTextPadding
  StyleVar_DockingSeparatorSize,    // float     DockingSeparatorSize
  StyleVar_COUNT
};

// Flags for InvisibleButton() [extended in internal.hpp]
enum ButtonFlags_ {
  ButtonFlags_None = 0,
  ButtonFlags_MouseButtonLeft = 1 << 0,  // React on left mouse button (default)
  ButtonFlags_MouseButtonRight = 1 << 1, // React on right mouse button
  ButtonFlags_MouseButtonMiddle = 1 << 2, // React on center mouse button

  // [Internal]
  ButtonFlags_MouseButtonMask_ = ButtonFlags_MouseButtonLeft |
                                 ButtonFlags_MouseButtonRight |
                                 ButtonFlags_MouseButtonMiddle,
  ButtonFlags_MouseButtonDefault_ = ButtonFlags_MouseButtonLeft,
};

// Flags for ColorEdit3() / ColorEdit4() / ColorPicker3() / ColorPicker4() /
// ColorButton()
enum ColorEditFlags_ {
  ColorEditFlags_None = 0,
  ColorEditFlags_NoAlpha =
      1 << 1, //              // ColorEdit, ColorPicker, ColorButton: ignore
              //              Alpha component (will only read 3 components from
              //              the input pointer).
  ColorEditFlags_NoPicker = 1
                            << 2, //              // ColorEdit: disable picker
                                  //              when clicking on color square.
  ColorEditFlags_NoOptions =
      1 << 3, //              // ColorEdit: disable toggling options menu when
              //              right-clicking on inputs/small preview.
  ColorEditFlags_NoSmallPreview =
      1 << 4, //              // ColorEdit, ColorPicker: disable color square
              //              preview next to the inputs. (e.g. to show only the
              //              inputs)
  ColorEditFlags_NoInputs =
      1 << 5, //              // ColorEdit, ColorPicker: disable inputs
              //              sliders/text widgets (e.g. to show only the small
              //              preview color square).
  ColorEditFlags_NoTooltip =
      1 << 6, //              // ColorEdit, ColorPicker, ColorButton: disable
              //              tooltip when hovering the preview.
  ColorEditFlags_NoLabel =
      1 << 7, //              // ColorEdit, ColorPicker: disable display of
              //              inline text label (the label is still forwarded to
              //              the tooltip and picker).
  ColorEditFlags_NoSidePreview =
      1 << 8, //              // ColorPicker: disable bigger color preview on
              //              right side of the picker, use small color square
              //              preview instead.
  ColorEditFlags_NoDragDrop =
      1 << 9, //              // ColorEdit: disable drag and drop target.
              //              ColorButton: disable drag and drop source.
  ColorEditFlags_NoBorder =
      1 << 10, //              // ColorButton: disable border (which is enforced
               //              by default)

  // User Options (right-click on widget to change some of them).
  ColorEditFlags_AlphaBar =
      1 << 16, //              // ColorEdit, ColorPicker: show vertical alpha
               //              bar/gradient in picker.
  ColorEditFlags_AlphaPreview =
      1 << 17, //              // ColorEdit, ColorPicker, ColorButton: display
               //              preview as a transparent color over a
               //              checkerboard, instead of opaque.
  ColorEditFlags_AlphaPreviewHalf =
      1
      << 18, //              // ColorEdit, ColorPicker, ColorButton: display
             //              half opaque / half checkerboard, instead of opaque.
  ColorEditFlags_HDR =
      1 << 19, //              // (WIP) ColorEdit: Currently only disable
               //              0.0f..1.0f limits in RGBA edition (note: you
               //              probably want to use ColorEditFlags_Float
               //              flag as well).
  ColorEditFlags_DisplayRGB =
      1 << 20, // [Display]    // ColorEdit: override _display_ type among
               // RGB/HSV/Hex. ColorPicker: select any combination using one or
               // more of RGB/HSV/Hex.
  ColorEditFlags_DisplayHSV = 1 << 21, // [Display]    // "
  ColorEditFlags_DisplayHex = 1 << 22, // [Display]    // "
  ColorEditFlags_Uint8 =
      1 << 23, // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_
               // values formatted as 0..255.
  ColorEditFlags_Float =
      1 << 24, // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_
               // values formatted as 0.0f..1.0f floats instead of 0..255
               // integers. No round-trip of value via integers.
  ColorEditFlags_PickerHueBar = 1 << 25,   // [Picker]     // ColorPicker: bar
                                           // for Hue, rectangle for Sat/Value.
  ColorEditFlags_PickerHueWheel = 1 << 26, // [Picker]     // ColorPicker: wheel
                                           // for Hue, triangle for Sat/Value.
  ColorEditFlags_InputRGB = 1 << 27, // [Input]      // ColorEdit, ColorPicker:
                                     // input and output data in RGB format.
  ColorEditFlags_InputHSV = 1 << 28, // [Input]      // ColorEdit, ColorPicker:
                                     // input and output data in HSV format.

  // Defaults Options. You can set application defaults using
  // SetColorEditOptions(). The intent is that you probably don't want to
  // override them in most of your calls. Let the user choose via the option
  // menu and/or call SetColorEditOptions() once during startup.
  ColorEditFlags_DefaultOptions_ =
      ColorEditFlags_Uint8 | ColorEditFlags_DisplayRGB |
      ColorEditFlags_InputRGB | ColorEditFlags_PickerHueBar,

  // [Internal] Masks
  ColorEditFlags_DisplayMask_ = ColorEditFlags_DisplayRGB |
                                ColorEditFlags_DisplayHSV |
                                ColorEditFlags_DisplayHex,
  ColorEditFlags_DataTypeMask_ = ColorEditFlags_Uint8 | ColorEditFlags_Float,
  ColorEditFlags_PickerMask_ =
      ColorEditFlags_PickerHueWheel | ColorEditFlags_PickerHueBar,
  ColorEditFlags_InputMask_ = ColorEditFlags_InputRGB | ColorEditFlags_InputHSV,

  // Obsolete names
  // ColorEditFlags_RGB = ColorEditFlags_DisplayRGB,
  // ColorEditFlags_HSV = ColorEditFlags_DisplayHSV,
  // ColorEditFlags_HEX = ColorEditFlags_DisplayHex  // [renamed
  // in 1.69]
};

// Flags for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
// We use the same sets of flags for DragXXX() and SliderXXX() functions as the
// features are the same and it makes it easier to swap them. (Those are
// per-item flags. There are shared flags in IO:
// io.ConfigDragClickToInputText)
enum SliderFlags_ {
  SliderFlags_None = 0,
  SliderFlags_AlwaysClamp =
      1 << 4, // Clamp value to min/max bounds when input manually with
              // CTRL+Click. By default CTRL+Click allows going out of bounds.
  SliderFlags_Logarithmic =
      1 << 5, // Make the widget logarithmic (linear otherwise). Consider using
              // SliderFlags_NoRoundToFormat with this if using a
              // format-string with small amount of digits.
  SliderFlags_NoRoundToFormat =
      1 << 6, // Disable rounding underlying value to match precision of the
              // display format string (e.g. %.3f values are rounded to those 3
              // digits)
  SliderFlags_NoInput = 1 << 7, // Disable CTRL+Click or Enter key allowing
                                // to input text directly into the widget
  SliderFlags_InvalidMask_ =
      0x7000000F, // [Internal] We treat using those bits as being potentially a
                  // 'float power' argument from the previous API that has got
                  // miscast to this enum, and will trigger an assert if needed.

  // Obsolete names
  // SliderFlags_ClampOnInput = SliderFlags_AlwaysClamp, // [renamed
  // in 1.79]
};

// Identify a mouse button.
// Those values are guaranteed to be stable and we frequently use 0/1 directly.
// Named enums provided for convenience.
enum MouseButton_ {
  MouseButton_Left = 0,
  MouseButton_Right = 1,
  MouseButton_Middle = 2,
  MouseButton_COUNT = 5
};

// Enumeration for GetMouseCursor()
// User code may request backend to display given cursor by calling
// SetMouseCursor(), which is why we have some cursors that are marked unused
// here
enum MouseCursor_ {
  MouseCursor_None = -1,
  MouseCursor_Arrow = 0,
  MouseCursor_TextInput,  // When hovering over InputText, etc.
  MouseCursor_ResizeAll,  // (Unused by Gui functions)
  MouseCursor_ResizeNS,   // When hovering over a horizontal border
  MouseCursor_ResizeEW,   // When hovering over a vertical border or a column
  MouseCursor_ResizeNESW, // When hovering over the bottom-left corner of a
                          // window
  MouseCursor_ResizeNWSE, // When hovering over the bottom-right corner of
                          // a window
  MouseCursor_Hand,       // (Unused by Gui functions. Use for e.g.
                          // hyperlinks)
  MouseCursor_NotAllowed, // When hovering something with disallowed
                          // interaction. Usually a crossed circle.
  MouseCursor_COUNT
};

// Enumeration for AddMouseSourceEvent() actual source of Mouse Input data.
// Historically we use "Mouse" terminology everywhere to indicate pointer data,
// e.g. MousePos, IsMousePressed(), io.AddMousePosEvent() But that "Mouse" data
// can come from different source which occasionally may be useful for
// application to know about. You can submit a change of pointer type using
// io.AddMouseSourceEvent().
enum MouseSource : int {
  MouseSource_Mouse = 0,   // Input is coming from an actual mouse.
  MouseSource_TouchScreen, // Input is coming from a touch screen (no
                           // hovering prior to initial press, less precise
                           // initial press aiming, dual-axis wheeling
                           // possible).
  MouseSource_Pen, // Input is coming from a pressure/magnetic pen (often
                   // used in conjunction with high-sampling rates).
  MouseSource_COUNT
};

// Enumeration for Gui::SetNextWindow***(), SetWindow***(), SetNextItem***()
// functions Represent a condition. Important: Treat as a regular enum! Do NOT
// combine multiple values using binary operators! All the functions above treat
// 0 as a shortcut to Cond_Always.
enum Cond_ {
  Cond_None = 0, // No condition (always set the variable), same as _Always
  Cond_Always = 1 << 0, // No condition (always set the variable), same as _None
  Cond_Once = 1 << 1,   // Set the variable once per runtime session (only
                        // the first call will succeed)
  Cond_FirstUseEver = 1 << 2, // Set the variable if the object/window has no
                              // persistently saved data (no entry in .ini file)
  Cond_Appearing = 1 << 3, // Set the variable if the object/window is appearing
                           // after being hidden/inactive (or the first time)
};

//-----------------------------------------------------------------------------
// [SECTION] Tables API flags and structures (TableFlags,
// TableColumnFlags, TableRowFlags, TableBgTarget,
// TableSortSpecs, TableColumnSortSpecs)
//-----------------------------------------------------------------------------

// Flags for Gui::BeginTable()
// - Important! Sizing policies have complex and subtle side effects, much more
// so than you would expect.
//   Read comments/demos carefully + experiment with live demos to get
//   acquainted with them.
// - The DEFAULT sizing policies are:
//    - Default to TableFlags_SizingFixedFit    if ScrollX is on, or if
//    host window has WindowFlags_AlwaysAutoResize.
//    - Default to TableFlags_SizingStretchSame if ScrollX is off.
// - When ScrollX is off:
//    - Table defaults to TableFlags_SizingStretchSame -> all Columns
//    defaults to TableColumnFlags_WidthStretch with same weight.
//    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
//    - Fixed Columns (if any) will generally obtain their requested width
//    (unless the table cannot fit them all).
//    - Stretch Columns will share the remaining width according to their
//    respective weight.
//    - Mixed Fixed/Stretch columns is possible but has various side-effects on
//    resizing behaviors.
//      The typical use of mixing sizing policies is: any number of LEADING
//      Fixed columns, followed by one or two TRAILING Stretch columns. (this is
//      because the visible order of columns have subtle but necessary effects
//      on how they react to manual resizing).
// - When ScrollX is on:
//    - Table defaults to TableFlags_SizingFixedFit -> all Columns defaults
//    to TableColumnFlags_WidthFixed
//    - Columns sizing policy allowed: Fixed/Auto mostly.
//    - Fixed Columns can be enlarged as needed. Table will show a horizontal
//    scrollbar if needed.
//    - When using auto-resizing (non-resizable) fixed columns, querying the
//    content width to use item right-alignment e.g. SetNextItemWidth(-FLT_MIN)
//    doesn't make sense, would create a feedback loop.
//    - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS
//    you have specified a value for 'inner_width' in BeginTable().
//      If you specify a value for 'inner_width' then effectively the scrolling
//      space is known and Stretch or mixed Fixed/Stretch columns become
//      meaningful again.
// - Read on documentation at the top of tables.cpp for details.
enum TableFlags_ {
  // Features
  TableFlags_None = 0,
  TableFlags_Resizable = 1 << 0, // Enable resizing columns.
  TableFlags_Reorderable =
      1 << 1, // Enable reordering columns in header row (need calling
              // TableSetupColumn() + TableHeadersRow() to display headers)
  TableFlags_Hideable =
      1 << 2, // Enable hiding/disabling columns in context menu.
  TableFlags_Sortable =
      1 << 3, // Enable sorting. Call TableGetSortSpecs() to obtain sort specs.
              // Also see TableFlags_SortMulti and
              // TableFlags_SortTristate.
  TableFlags_NoSavedSettings =
      1 << 4, // Disable persisting columns order, width and sort settings in
              // the .ini file.
  TableFlags_ContextMenuInBody =
      1 << 5, // Right-click on columns body/contents will display table context
              // menu. By default it is available in TableHeadersRow().
              // Decorations
  TableFlags_RowBg = 1 << 6, // Set each RowBg color with Col_TableRowBg or
  // Col_TableRowBgAlt (equivalent of calling TableSetBgColor
  // with TableBgFlags_RowBg0 on each row manually)
  TableFlags_BordersInnerH = 1 << 7, // Draw horizontal borders between rows.
  TableFlags_BordersOuterH =
      1 << 8, // Draw horizontal borders at the top and bottom.
  TableFlags_BordersInnerV = 1 << 9, // Draw vertical borders between columns.
  TableFlags_BordersOuterV =
      1 << 10, // Draw vertical borders on the left and right sides.
  TableFlags_BordersH = TableFlags_BordersInnerH |
                        TableFlags_BordersOuterH, // Draw horizontal borders.
  TableFlags_BordersV = TableFlags_BordersInnerV |
                        TableFlags_BordersOuterV, // Draw vertical borders.
  TableFlags_BordersInner = TableFlags_BordersInnerV |
                            TableFlags_BordersInnerH, // Draw inner borders.
  TableFlags_BordersOuter = TableFlags_BordersOuterV |
                            TableFlags_BordersOuterH, // Draw outer borders.
  TableFlags_Borders =
      TableFlags_BordersInner | TableFlags_BordersOuter, // Draw all borders.
  TableFlags_NoBordersInBody =
      1 << 11, // [ALPHA] Disable vertical borders in columns Body (borders will
               // always appear in Headers). -> May move to style
  TableFlags_NoBordersInBodyUntilResize =
      1 << 12, // [ALPHA] Disable vertical borders in columns Body until hovered
               // for resize (borders will always appear in Headers). -> May
               // move to style Sizing Policy (read above for defaults)
  TableFlags_SizingFixedFit =
      1 << 13, // Columns default to _WidthFixed or _WidthAuto (if resizable or
               // not resizable), matching contents width.
  TableFlags_SizingFixedSame =
      2 << 13, // Columns default to _WidthFixed or _WidthAuto (if resizable or
               // not resizable), matching the maximum contents width of all
  // columns. Implicitly enable TableFlags_NoKeepColumnsVisible.
  TableFlags_SizingStretchProp =
      3 << 13, // Columns default to _WidthStretch with default weights
               // proportional to each columns contents widths.
  TableFlags_SizingStretchSame =
      4
      << 13, // Columns default to _WidthStretch with default weights all equal,
             // unless overridden by TableSetupColumn(). Sizing Extra Options
  TableFlags_NoHostExtendX =
      1 << 16, // Make outer width auto-fit to columns, overriding outer_size.x
               // value. Only available when ScrollX/ScrollY are disabled and
               // Stretch columns are not used.
  TableFlags_NoHostExtendY =
      1 << 17, // Make outer height stop exactly at outer_size.y (prevent
               // auto-extending table past the limit). Only available when
               // ScrollX/ScrollY are disabled. Data below the limit will be
               // clipped and not visible.
  TableFlags_NoKeepColumnsVisible =
      1 << 18, // Disable keeping column always minimally visible when ScrollX
               // is off and table gets too small. Not recommended if columns
               // are resizable.
  TableFlags_PreciseWidths =
      1
      << 19, // Disable distributing remainder width to stretched columns (width
             // allocation on a 100-wide table with 3 columns: Without this
             // flag: 33,33,34. With this flag: 33,33,33). With larger number of
             // columns, resizing will appear to be less smooth. Clipping
  TableFlags_NoClip =
      1 << 20, // Disable clipping rectangle for every individual columns
               // (reduce draw command count, items will be able to overflow
               // into other columns). Generally incompatible with
               // TableSetupScrollFreeze(). Padding
  TableFlags_PadOuterX =
      1 << 21, // Default if BordersOuterV is on. Enable outermost padding.
               // Generally desirable if you have headers.
  TableFlags_NoPadOuterX =
      1 << 22, // Default if BordersOuterV is off. Disable outermost padding.
  TableFlags_NoPadInnerX =
      1 << 23, // Disable inner padding between columns (double inner padding if
               // BordersOuterV is on, single inner padding if BordersOuterV is
               // off). Scrolling
  TableFlags_ScrollX =
      1 << 24, // Enable horizontal scrolling. Require 'outer_size' parameter of
               // BeginTable() to specify the container size. Changes default
               // sizing policy. Because this creates a child window, ScrollY is
               // currently generally recommended when using ScrollX.
  TableFlags_ScrollY =
      1 << 25, // Enable vertical scrolling. Require 'outer_size' parameter of
               // BeginTable() to specify the container size. Sorting
  TableFlags_SortMulti =
      1 << 26, // Hold shift when clicking headers to sort on multiple column.
               // TableGetSortSpecs() may return specs where (SpecsCount > 1).
  TableFlags_SortTristate =
      1 << 27, // Allow no sorting, disable default sorting. TableGetSortSpecs()
               // may return specs where (SpecsCount == 0). Miscellaneous
  TableFlags_HighlightHoveredColumn =
      1 << 28, // Highlight column headers when hovered (may evolve into a
               // fuller highlight)

  // [Internal] Combinations and masks
  TableFlags_SizingMask_ =
      TableFlags_SizingFixedFit | TableFlags_SizingFixedSame |
      TableFlags_SizingStretchProp | TableFlags_SizingStretchSame,
};

// Flags for Gui::TableSetupColumn()
enum TableColumnFlags_ {
  // Input configuration flags
  TableColumnFlags_None = 0,
  TableColumnFlags_Disabled =
      1 << 0, // Overriding/master disable flag: hide column, won't show in
              // context menu (unlike calling TableSetColumnEnabled() which
              // manipulates the user accessible state)
  TableColumnFlags_DefaultHide = 1 << 1, // Default as a hidden/disabled column.
  TableColumnFlags_DefaultSort = 1 << 2, // Default as a sorting column.
  TableColumnFlags_WidthStretch =
      1 << 3, // Column will stretch. Preferable with horizontal scrolling
              // disabled (default if table sizing policy is _SizingStretchSame
              // or _SizingStretchProp).
  TableColumnFlags_WidthFixed =
      1 << 4, // Column will not stretch. Preferable with horizontal scrolling
              // enabled (default if table sizing policy is _SizingFixedFit and
              // table is resizable).
  TableColumnFlags_NoResize = 1 << 5, // Disable manual resizing.
  TableColumnFlags_NoReorder =
      1 << 6, // Disable manual reordering this column, this will also prevent
              // other columns from crossing over this column.
  TableColumnFlags_NoHide =
      1 << 7, // Disable ability to hide/disable this column.
  TableColumnFlags_NoClip =
      1 << 8, // Disable clipping for this column (all NoClip columns will
              // render in a same draw command).
  TableColumnFlags_NoSort =
      1 << 9, // Disable ability to sort on this field (even if
              // TableFlags_Sortable is set on the table).
  TableColumnFlags_NoSortAscending =
      1 << 10, // Disable ability to sort in the ascending direction.
  TableColumnFlags_NoSortDescending =
      1 << 11, // Disable ability to sort in the descending direction.
  TableColumnFlags_NoHeaderLabel =
      1 << 12, // TableHeadersRow() will not submit horizontal label for this
               // column. Convenient for some small columns. Name will still
               // appear in context menu or in angled headers.
  TableColumnFlags_NoHeaderWidth =
      1 << 13, // Disable header text width contribution to automatic column
               // width.
  TableColumnFlags_PreferSortAscending =
      1 << 14, // Make the initial sort direction Ascending when first sorting
               // on this column (default).
  TableColumnFlags_PreferSortDescending =
      1 << 15, // Make the initial sort direction Descending when first sorting
               // on this column.
  TableColumnFlags_IndentEnable =
      1 << 16, // Use current Indent value when entering cell (default for
               // column 0).
  TableColumnFlags_IndentDisable =
      1 << 17, // Ignore current Indent value when entering cell (default for
               // columns > 0). Indentation changes _within_ the cell will still
               // be honored.
  TableColumnFlags_AngledHeader =
      1 << 18, // TableHeadersRow() will submit an angled header row for this
               // column. Note this will add an extra row.

  // Output status flags, read-only via TableGetColumnFlags()
  TableColumnFlags_IsEnabled =
      1 << 24, // Status: is enabled == not hidden by user/api (referred to as
               // "Hide" in _DefaultHide and _NoHide) flags.
  TableColumnFlags_IsVisible =
      1 << 25, // Status: is visible == is enabled AND not clipped by scrolling.
  TableColumnFlags_IsSorted =
      1 << 26, // Status: is currently part of the sort specs
  TableColumnFlags_IsHovered = 1 << 27, // Status: is hovered by mouse

  // [Internal] Combinations and masks
  TableColumnFlags_WidthMask_ =
      TableColumnFlags_WidthStretch | TableColumnFlags_WidthFixed,
  TableColumnFlags_IndentMask_ =
      TableColumnFlags_IndentEnable | TableColumnFlags_IndentDisable,
  TableColumnFlags_StatusMask_ =
      TableColumnFlags_IsEnabled | TableColumnFlags_IsVisible |
      TableColumnFlags_IsSorted | TableColumnFlags_IsHovered,
  TableColumnFlags_NoDirectResize_ =
      1 << 30, // [Internal] Disable user resizing this column directly (it may
               // however we resized indirectly from its left edge)
};

// Flags for Gui::TableNextRow()
enum TableRowFlags_ {
  TableRowFlags_None = 0,
  TableRowFlags_Headers =
      1 << 0, // Identify header row (set default background color + width of
              // its contents accounted differently for auto column width)
};

// Enum for Gui::TableSetBgColor()
// Background colors are rendering in 3 layers:
//  - Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if
//  set.
//  - Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if
//  set.
//  - Layer 2: draw with CellBg color if set.
// The purpose of the two row/columns layers is to let you decide if a
// background color change should override or blend with the existing color.
// When using TableFlags_RowBg on the table, each row has the RowBg0 color
// automatically set for odd/even rows. If you set the color of RowBg0 target,
// your color will override the existing RowBg0 color. If you set the color of
// RowBg1 or ColumnBg1 target, your color will blend over the RowBg0 color.
enum TableBgTarget_ {
  TableBgTarget_None = 0,
  TableBgTarget_RowBg0 =
      1, // Set row background color 0 (generally used for background,
         // automatically set when TableFlags_RowBg is used)
  TableBgTarget_RowBg1 =
      2, // Set row background color 1 (generally used for selection marking)
  TableBgTarget_CellBg = 3, // Set cell background color (top-most color)
};

// Sorting specifications for a table (often handling sort specs for a single
// column, occasionally more) Obtained by calling TableGetSortSpecs(). When
// 'SpecsDirty == true' you can sort your data. It will be true with sorting
// specs have changed since last call, or the first time. Make sure to set
// 'SpecsDirty = false' after sorting, else you may wastefully sort your data
// every frame!
struct TableSortSpecs {
  const TableColumnSortSpecs *Specs; // Pointer to sort spec array.
  int SpecsCount;  // Sort spec count. Most often 1. May be > 1 when
                   // TableFlags_SortMulti is enabled. May be == 0 when
                   // TableFlags_SortTristate is enabled.
  bool SpecsDirty; // Set to true when specs have changed since last time! Use
                   // this to sort again, then clear the flag.

  TableSortSpecs() { memset(this, 0, sizeof(*this)); }
};

// Sorting specification for one column of a table (sizeof == 12 bytes)
struct TableColumnSortSpecs {
  ID ColumnUserID; // User id of the column (if specified by a
                   // TableSetupColumn() call)
  S16 ColumnIndex; // Index of the column
  S16 SortOrder;   // Index within parent TableSortSpecs (always stored in
                   // order starting from 0, tables sorted on a single criteria
                   // will always have a 0 here)
  SortDirection SortDirection : 8; // SortDirection_Ascending or
                                   // SortDirection_Descending

  TableColumnSortSpecs() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers: Memory allocations macros, Vector<>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// MALLOC(), FREE(), NEW(), PLACEMENT_NEW(), DELETE()
// We call C++ constructor on own allocated memory via the placement "new(ptr)
// Type()" syntax. Defining a custom placement new() with a custom parameter
// allows us to bypass including <new> which on some platforms complains when
// user has disabled exceptions.
//-----------------------------------------------------------------------------

struct NewWrapper {};
inline void *operator new(size_t, NewWrapper, void *ptr) { return ptr; }
inline void operator delete(void *, NewWrapper, void *) {
} // This is only required so we can use the symmetrical new()
#define ALLOC(_SIZE) Gui::MemAlloc(_SIZE)
#define FREE(_PTR) Gui::MemFree(_PTR)
#define PLACEMENT_NEW(_PTR) new (NewWrapper(), _PTR)
#define NEW(_TYPE) new (NewWrapper(), Gui::MemAlloc(sizeof(_TYPE))) _TYPE
template <typename T> void DELETE(T *p) {
  if (p) {
    p->~T();
    Gui::MemFree(p);
  }
}

//-----------------------------------------------------------------------------
// Vector<>
// Lightweight std::vector<>-like class to avoid dragging dependencies (also,
// some implementations of STL with debug enabled are absurdly slow, we bypass
// it so our code runs fast in debug).
//-----------------------------------------------------------------------------
// - You generally do NOT need to care or use this ever. But we need to make it
// available in gui.hpp because some of our public structures are relying on it.
// - We use std-like naming convention here, which is a little unusual for this
// codebase.
// - Important: clear() frees memory, resize(0) keep the allocated buffer. We
// use resize(0) a lot to intentionally recycle allocated buffers across frames
// and amortize our costs.
// - Important: our implementation does NOT call C++ constructors/destructors,
// we treat everything as raw data! This is intentional but be extra mindful of
// that,
//   Do NOT use this class as a std::vector replacement in your own code! Many
//   of the structures used by gui can be safely initialized by a
//   zero-memset.
//-----------------------------------------------------------------------------

MSVC_RUNTIME_CHECKS_OFF
template <typename T> struct Vector {
  int Size;
  int Capacity;
  T *Data;

  // Provide standard typedefs but we don't use them ourselves.
  typedef T value_type;
  typedef value_type *iterator;
  typedef const value_type *const_iterator;

  // Constructors, destructor
  inline Vector() {
    Size = Capacity = 0;
    Data = NULL;
  }
  inline Vector(const Vector<T> &src) {
    Size = Capacity = 0;
    Data = NULL;
    operator=(src);
  }
  inline Vector<T> &operator=(const Vector<T> &src) {
    clear();
    resize(src.Size);
    if (src.Data)
      memcpy(Data, src.Data, (size_t)Size * sizeof(T));
    return *this;
  }
  inline ~Vector() {
    if (Data)
      FREE(Data);
  } // Important: does not destruct anything

  inline void clear() {
    if (Data) {
      Size = Capacity = 0;
      FREE(Data);
      Data = NULL;
    }
  } // Important: does not destruct anything
  inline void clear_delete() {
    for (int n = 0; n < Size; n++)
      DELETE(Data[n]);
    clear();
  } // Important: never called automatically! always explicit.
  inline void clear_destruct() {
    for (int n = 0; n < Size; n++)
      Data[n].~T();
    clear();
  } // Important: never called automatically! always explicit.

  inline bool empty() const { return Size == 0; }
  inline int size() const { return Size; }
  inline int size_in_bytes() const { return Size * (int)sizeof(T); }
  inline int max_size() const { return 0x7FFFFFFF / (int)sizeof(T); }
  inline int capacity() const { return Capacity; }
  inline T &operator[](int i) {
    ASSERT(i >= 0 && i < Size);
    return Data[i];
  }
  inline const T &operator[](int i) const {
    ASSERT(i >= 0 && i < Size);
    return Data[i];
  }

  inline T *begin() { return Data; }
  inline const T *begin() const { return Data; }
  inline T *end() { return Data + Size; }
  inline const T *end() const { return Data + Size; }
  inline T &front() {
    ASSERT(Size > 0);
    return Data[0];
  }
  inline const T &front() const {
    ASSERT(Size > 0);
    return Data[0];
  }
  inline T &back() {
    ASSERT(Size > 0);
    return Data[Size - 1];
  }
  inline const T &back() const {
    ASSERT(Size > 0);
    return Data[Size - 1];
  }
  inline void swap(Vector<T> &rhs) {
    int rhs_size = rhs.Size;
    rhs.Size = Size;
    Size = rhs_size;
    int rhs_cap = rhs.Capacity;
    rhs.Capacity = Capacity;
    Capacity = rhs_cap;
    T *rhs_data = rhs.Data;
    rhs.Data = Data;
    Data = rhs_data;
  }

  inline int _grow_capacity(int sz) const {
    int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8;
    return new_capacity > sz ? new_capacity : sz;
  }
  inline void resize(int new_size) {
    if (new_size > Capacity)
      reserve(_grow_capacity(new_size));
    Size = new_size;
  }
  inline void resize(int new_size, const T &v) {
    if (new_size > Capacity)
      reserve(_grow_capacity(new_size));
    if (new_size > Size)
      for (int n = Size; n < new_size; n++)
        memcpy(&Data[n], &v, sizeof(v));
    Size = new_size;
  }
  inline void shrink(int new_size) {
    ASSERT(new_size <= Size);
    Size = new_size;
  } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
  inline void reserve(int new_capacity) {
    if (new_capacity <= Capacity)
      return;
    T *new_data = (T *)ALLOC((size_t)new_capacity * sizeof(T));
    if (Data) {
      memcpy(new_data, Data, (size_t)Size * sizeof(T));
      FREE(Data);
    }
    Data = new_data;
    Capacity = new_capacity;
  }
  inline void reserve_discard(int new_capacity) {
    if (new_capacity <= Capacity)
      return;
    if (Data)
      FREE(Data);
    Data = (T *)ALLOC((size_t)new_capacity * sizeof(T));
    Capacity = new_capacity;
  }

  // NB: It is illegal to call push_back/push_front/insert with a reference
  // pointing inside the Vector data itself! e.g. v.push_back(v[10]) is
  // forbidden.
  inline void push_back(const T &v) {
    if (Size == Capacity)
      reserve(_grow_capacity(Size + 1));
    memcpy(&Data[Size], &v, sizeof(v));
    Size++;
  }
  inline void pop_back() {
    ASSERT(Size > 0);
    Size--;
  }
  inline void push_front(const T &v) {
    if (Size == 0)
      push_back(v);
    else
      insert(Data, v);
  }
  inline T *erase(const T *it) {
    ASSERT(it >= Data && it < Data + Size);
    const ptrdiff_t off = it - Data;
    memmove(Data + off, Data + off + 1,
            ((size_t)Size - (size_t)off - 1) * sizeof(T));
    Size--;
    return Data + off;
  }
  inline T *erase(const T *it, const T *it_last) {
    ASSERT(it >= Data && it < Data + Size && it_last >= it &&
           it_last <= Data + Size);
    const ptrdiff_t count = it_last - it;
    const ptrdiff_t off = it - Data;
    memmove(Data + off, Data + off + count,
            ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T));
    Size -= (int)count;
    return Data + off;
  }
  inline T *erase_unsorted(const T *it) {
    ASSERT(it >= Data && it < Data + Size);
    const ptrdiff_t off = it - Data;
    if (it < Data + Size - 1)
      memcpy(Data + off, Data + Size - 1, sizeof(T));
    Size--;
    return Data + off;
  }
  inline T *insert(const T *it, const T &v) {
    ASSERT(it >= Data && it <= Data + Size);
    const ptrdiff_t off = it - Data;
    if (Size == Capacity)
      reserve(_grow_capacity(Size + 1));
    if (off < (int)Size)
      memmove(Data + off + 1, Data + off,
              ((size_t)Size - (size_t)off) * sizeof(T));
    memcpy(&Data[off], &v, sizeof(v));
    Size++;
    return Data + off;
  }
  inline bool contains(const T &v) const {
    const T *data = Data;
    const T *data_end = Data + Size;
    while (data < data_end)
      if (*data++ == v)
        return true;
    return false;
  }
  inline T *find(const T &v) {
    T *data = Data;
    const T *data_end = Data + Size;
    while (data < data_end)
      if (*data == v)
        break;
      else
        ++data;
    return data;
  }
  inline const T *find(const T &v) const {
    const T *data = Data;
    const T *data_end = Data + Size;
    while (data < data_end)
      if (*data == v)
        break;
      else
        ++data;
    return data;
  }
  inline int find_index(const T &v) const {
    const T *data_end = Data + Size;
    const T *it = find(v);
    if (it == data_end)
      return -1;
    const ptrdiff_t off = it - Data;
    return (int)off;
  }
  inline bool find_erase(const T &v) {
    const T *it = find(v);
    if (it < Data + Size) {
      erase(it);
      return true;
    }
    return false;
  }
  inline bool find_erase_unsorted(const T &v) {
    const T *it = find(v);
    if (it < Data + Size) {
      erase_unsorted(it);
      return true;
    }
    return false;
  }
  inline int index_from_ptr(const T *it) const {
    ASSERT(it >= Data && it < Data + Size);
    const ptrdiff_t off = it - Data;
    return (int)off;
  }
};
MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] Style
//-----------------------------------------------------------------------------
// You may modify the Gui::GetStyle() main instance during initialization and
// before NewFrame(). During the frame, use
// Gui::PushStyleVar(StyleVar_XXXX)/PopStyleVar() to alter the main style
// values, and Gui::PushStyleColor(Col_XXX)/PopStyleColor() for colors.
//-----------------------------------------------------------------------------

struct Style {
  float Alpha;         // Global alpha applies to everything in Gui.
  float DisabledAlpha; // Additional alpha multiplier applied by
                       // BeginDisabled(). Multiply over current value of Alpha.
  Vec2 WindowPadding;  // Padding within a window.
  float WindowRounding;   // Radius of window corners rounding. Set to 0.0f to
                          // have rectangular windows. Large values tend to lead
                          // to variety of artifacts and are not recommended.
  float WindowBorderSize; // Thickness of border around windows. Generally set
                          // to 0.0f or 1.0f. (Other values are not well tested
                          // and more CPU/GPU costly).
  Vec2 WindowMinSize; // Minimum window size. This is a global setting. If you
                      // want to constrain individual windows, use
                      // SetNextWindowSizeConstraints().
  Vec2 WindowTitleAlign; // Alignment for title bar text. Defaults to
                         // (0.0f,0.5f) for left-aligned,vertically centered.
  Dir WindowMenuButtonPosition; // Side of the collapsing/docking button in
                                // the title bar (None/Left/Right).
                                // Defaults to Dir_Left.
  float ChildRounding;   // Radius of child window corners rounding. Set to 0.0f
                         // to have rectangular windows.
  float ChildBorderSize; // Thickness of border around child windows. Generally
                         // set to 0.0f or 1.0f. (Other values are not well
                         // tested and more CPU/GPU costly).
  float PopupRounding;   // Radius of popup window corners rounding. (Note that
                         // tooltip windows use WindowRounding)
  float PopupBorderSize; // Thickness of border around popup/tooltip windows.
                         // Generally set to 0.0f or 1.0f. (Other values are not
                         // well tested and more CPU/GPU costly).
  Vec2
      FramePadding; // Padding within a framed rectangle (used by most widgets).
  float FrameRounding; // Radius of frame corners rounding. Set to 0.0f to have
                       // rectangular frame (used by most widgets).
  float FrameBorderSize; // Thickness of border around frames. Generally set to
                         // 0.0f or 1.0f. (Other values are not well tested and
                         // more CPU/GPU costly).
  Vec2 ItemSpacing; // Horizontal and vertical spacing between widgets/lines.
  Vec2 ItemInnerSpacing;  // Horizontal and vertical spacing between within
                          // elements of a composed widget (e.g. a slider and
                          // its label).
  Vec2 CellPadding;       // Padding within a table cell. CellPadding.y may be
                          // altered between different rows.
  Vec2 TouchExtraPadding; // Expand reactive bounding box for touch-based
                          // system where touch position is not accurate
                          // enough. Unfortunately we don't sort widgets so
                          // priority on overlap will always be given to the
                          // first widget. So don't grow this too much!
  float IndentSpacing; // Horizontal indentation when e.g. entering a tree node.
                       // Generally == (FontSize + FramePadding.x*2).
  float ColumnsMinSpacing; // Minimum horizontal spacing between two columns.
                           // Preferably > (FramePadding.x + 1).
  float ScrollbarSize;     // Width of the vertical scrollbar, Height of the
                           // horizontal scrollbar.
  float ScrollbarRounding; // Radius of grab corners for scrollbar.
  float GrabMinSize; // Minimum width/height of a grab box for slider/scrollbar.
  float GrabRounding; // Radius of grabs corners rounding. Set to 0.0f to have
                      // rectangular slider grabs.
  float LogSliderDeadzone; // The size in pixels of the dead-zone around zero on
                           // logarithmic sliders that cross zero.
  float TabRounding;   // Radius of upper corners of a tab. Set to 0.0f to have
                       // rectangular tabs.
  float TabBorderSize; // Thickness of border around tabs.
  float
      TabMinWidthForCloseButton; // Minimum width for close button to appear on
                                 // an unselected tab when hovered. Set to 0.0f
                                 // to always show when hovering, set to FLT_MAX
                                 // to never show close button unless selected.
  float TabBarBorderSize; // Thickness of tab-bar separator, which takes on the
                          // tab active color to denote focus.
  float
      TableAngledHeadersAngle; // Angle of angled headers (supported values
                               // range from -50.0f degrees to +50.0f degrees).
  Dir ColorButtonPosition; // Side of the color button in the ColorEdit4 widget
                           // (left/right). Defaults to Dir_Right.
  Vec2 ButtonTextAlign; // Alignment of button text when button is larger than
                        // text. Defaults to (0.5f, 0.5f) (centered).
  Vec2 SelectableTextAlign; // Alignment of selectable text. Defaults to
                            // (0.0f, 0.0f) (top-left aligned). It's generally
                            // important to keep this left-aligned if you want
                            // to lay multiple items on a same line.
  float SeparatorTextBorderSize; // Thickkness of border in SeparatorText()
  Vec2 SeparatorTextAlign;   // Alignment of text within the separator. Defaults
                             // to (0.0f, 0.5f) (left aligned, center).
  Vec2 SeparatorTextPadding; // Horizontal offset of text from each edge of
                             // the separator + spacing on other axis.
                             // Generally small values. .y is recommended to
                             // be == FramePadding.y.
  Vec2 DisplayWindowPadding; // Window position are clamped to be visible within
                             // the display area or monitors by at least this
                             // amount. Only applies to regular windows.
  Vec2 DisplaySafeAreaPadding; // If you cannot see the edges of your screen
                               // (e.g. on a TV) increase the safe area
                               // padding. Apply to popups/tooltips as well
                               // regular windows. NB: Prefer configuring your
                               // TV sets correctly!
  float DockingSeparatorSize;  // Thickness of resizing border between docked
                               // windows
  float
      MouseCursorScale;  // Scale software rendered mouse cursor (when
                         // io.MouseDrawCursor is enabled). We apply per-monitor
                         // DPI scaling over this scale. May be removed later.
  bool AntiAliasedLines; // Enable anti-aliased lines/borders. Disable if you
                         // are really tight on CPU/GPU. Latched at the
                         // beginning of the frame (copied to DrawList).
  bool AntiAliasedLinesUseTex; // Enable anti-aliased lines/borders using
                               // textures where possible. Require backend to
                               // render with bilinear filtering (NOT
                               // point/nearest filtering). Latched at the
                               // beginning of the frame (copied to DrawList).
  bool AntiAliasedFill;        // Enable anti-aliased edges around filled shapes
                        // (rounded rectangles, circles, etc.). Disable if you
                        // are really tight on CPU/GPU. Latched at the beginning
                        // of the frame (copied to DrawList).
  float CurveTessellationTol; // Tessellation tolerance when using
                              // PathBezierCurveTo() without a specific number
                              // of segments. Decrease for highly tessellated
                              // curves (higher quality, more polygons),
                              // increase to reduce quality.
  float
      CircleTessellationMaxError; // Maximum error (in pixels) allowed when
                                  // using AddCircle()/AddCircleFilled() or
                                  // drawing rounded corner rectangles with no
                                  // explicit segment count specified. Decrease
                                  // for higher quality but more geometry.
  Vec4 Colors[Col_COUNT];

  // Behaviors
  // (It is possible to modify those fields mid-frame if specific behavior need
  // it, unlike e.g. configuration fields in IO)
  float HoverStationaryDelay; // Delay for
                              // IsItemHovered(HoveredFlags_Stationary).
                              // Time required to consider mouse stationary.
  float HoverDelayShort;  // Delay for IsItemHovered(HoveredFlags_DelayShort).
                          // Usually used along with HoverStationaryDelay.
  float HoverDelayNormal; // Delay for
                          // IsItemHovered(HoveredFlags_DelayNormal). "
  HoveredFlags
      HoverFlagsForTooltipMouse; // Default flags when using
                                 // IsItemHovered(HoveredFlags_ForTooltip)
                                 // or BeginItemTooltip()/SetItemTooltip() while
                                 // using mouse.
  HoveredFlags
      HoverFlagsForTooltipNav; // Default flags when using
                               // IsItemHovered(HoveredFlags_ForTooltip) or
                               // BeginItemTooltip()/SetItemTooltip() while
                               // using keyboard/gamepad.

  API Style();
  API void ScaleAllSizes(float scale_factor);
};

//-----------------------------------------------------------------------------
// [SECTION] IO
//-----------------------------------------------------------------------------
// Communicate most settings and inputs/outputs to Gui using this
// structure. Access via Gui::GetIO(). Read 'Programmer guide' section in .cpp
// file for general usage.
//-----------------------------------------------------------------------------

// [Internal] Storage used by IsKeyDown(), IsKeyPressed() etc functions.
// If prior to 1.87 you used io.KeysDownDuration[] (which was marked as
// internal), you should use GetKeyData(key)->DownDuration and *NOT*
// io.KeysData[key]->DownDuration.
struct KeyData {
  bool Down;              // True for if key is down
  float DownDuration;     // Duration the key has been down (<0.0f: not pressed,
                          // 0.0f: just pressed, >0.0f: time held)
  float DownDurationPrev; // Last frame duration the key has been down
  float AnalogValue;      // 0.0f..1.0f for gamepad values
};

struct IO {
  //------------------------------------------------------------------
  // Configuration                            // Default value
  //------------------------------------------------------------------

  ConfigFlags ConfigFlags;   // = 0              // See ConfigFlags_
                             // enum. Set by user/application.
                             // Gamepad/keyboard navigation options, etc.
  BackendFlags BackendFlags; // = 0              // See BackendFlags_ enum. Set
                             // by backend (xxx files or custom backend) to
                             // communicate features supported by the backend.
  Vec2 DisplaySize;          // <unset>          // Main display size, in pixels
                    // (generally == GetMainViewport()->Size). May change
                    // every frame.
  float DeltaTime;     // = 1.0f/60.0f     // Time elapsed since last frame, in
                       // seconds. May change every frame.
  float IniSavingRate; // = 5.0f           // Minimum time between saving
                       // positions/sizes to .ini file, in seconds.
  const char
      *IniFilename; // = "gui.ini"    // Path to .ini file (important: default
                    // "gui.ini" is relative to current working dir!). Set
                    // NULL to disable automatic .ini loading/saving or if you
                    // want to manually call LoadIniSettingsXXX() /
                    // SaveIniSettingsXXX() functions.
  const char *LogFilename; // = "log.txt"// Path to .log file (default parameter
                           // to Gui::LogToFile when no file is specified).
  void *UserData;          // = NULL           // Store your own data.

  FontAtlas *Fonts;      // <auto>           // Font atlas: load, rasterize and
                         // pack one or more fonts into a single texture.
  float FontGlobalScale; // = 1.0f           // Global scale all fonts
  bool FontAllowUserScaling; // = false          // Allow user scaling text of
                             // individual window with CTRL+Wheel.
  Font *FontDefault; // = NULL           // Font to use on NewFrame(). Use
                     // NULL to uses Fonts->Fonts[0].
  Vec2 DisplayFramebufferScale; // = (1, 1)         // For retina display or
                                // other situations where window coordinates
                                // are different from framebuffer coordinates.
                                // This generally ends up in
                                // DrawData::FramebufferScale.

  // Docking options (when ConfigFlags_DockingEnable is set)
  bool ConfigDockingNoSplit; // = false          // Simplified docking mode:
                             // disable window splitting, so docking is limited
                             // to merging multiple windows together into
                             // tab-bars.
  bool ConfigDockingWithShift; // = false          // Enable docking with
                               // holding Shift key (reduce visual noise, allows
                               // dropping in wider space)
  bool ConfigDockingAlwaysTabBar; // = false          // [BETA] [FIXME: This
                                  // currently creates regression with
                                  // auto-sizing and general overhead] Make
                                  // every single floating window display within
                                  // a docking node.
  bool ConfigDockingTransparentPayload; // = false          // [BETA] Make
                                        // window or viewport transparent when
                                        // docking and only display docking
                                        // boxes on the target viewport. Useful
                                        // if rendering of multiple viewport
                                        // cannot be synced. Best used with
                                        // ConfigViewportsNoAutoMerge.

  // Viewport options (when ConfigFlags_ViewportsEnable is set)
  bool ConfigViewportsNoAutoMerge; // = false;         // Set to make all
                                   // floating imgui windows always create their
                                   // own viewport. Otherwise, they are merged
                                   // into the main host viewports when
                                   // overlapping it. May also set
                                   // ViewportFlags_NoAutoMerge on
                                   // individual viewport.
  bool ConfigViewportsNoTaskBarIcon; // = false          // Disable default OS
                                     // task bar icon flag for secondary
                                     // viewports. When a viewport doesn't want
                                     // a task bar icon,
                                     // ViewportFlags_NoTaskBarIcon will be
                                     // set on it.
  bool ConfigViewportsNoDecoration;  // = true           // Disable default OS
                                     // window decoration flag for secondary
                                     // viewports. When a viewport doesn't want
                                     // window decorations,
                                     // ViewportFlags_NoDecoration will be
  // set on it. Enabling decoration can create
  // subsequent issues at OS levels (e.g.
  // minimum window size).
  bool
      ConfigViewportsNoDefaultParent; // = false          // Disable default OS
                                      // parenting to main viewport for
                                      // secondary viewports. By default,
                                      // viewports are marked with
                                      // ParentViewportId = <main_viewport>,
                                      // expecting the platform backend to setup
                                      // a parent/child relationship between the
                                      // OS windows (some backend may ignore
                                      // this). Set to true if you want the
                                      // default to be 0, then all viewports
                                      // will be top-level OS windows.

  // Miscellaneous options
  bool MouseDrawCursor; // = false          // Request Gui to draw a mouse
                        // cursor for you (if you are on a platform without a
                        // mouse cursor). Cannot be easily renamed to
                        // 'io.ConfigXXX' because this is frequently used by
                        // backend implementations.
  bool
      ConfigMacOSXBehaviors; // = defined(__APPLE__) // OS X style: Text editing
                             // cursor movement using Alt instead of Ctrl,
                             // Shortcuts using Cmd/Super instead of Ctrl,
                             // Line/Text Start and End using Cmd+Arrows instead
                             // of Home/End, Double click selects by word
                             // instead of selecting whole text, Multi-selection
                             // in lists uses Cmd/Super instead of Ctrl.
  bool ConfigInputTrickleEventQueue; // = true           // Enable input queue
                                     // trickling: some types of events
                                     // submitted during the same frame (e.g.
                                     // button down + up) will be spread over
                                     // multiple frames, improving interactions
                                     // with low framerates.
  bool ConfigInputTextCursorBlink; // = true           // Enable blinking cursor
                                   // (optional as some users consider it to be
                                   // distracting).
  bool ConfigInputTextEnterKeepActive; // = false          // [BETA] Pressing
                                       // Enter will keep item active and select
                                       // contents (single-line only).
  bool ConfigDragClickToInputText; // = false          // [BETA] Enable turning
                                   // DragXXX widgets into text input with a
                                   // simple mouse click-release (without
                                   // moving). Not desirable on devices without
                                   // a keyboard.
  bool ConfigWindowsResizeFromEdges; // = true           // Enable resizing of
                                     // windows from their edges and from the
                                     // lower-left corner. This requires
                                     // (io.BackendFlags &
                                     // BackendFlags_HasMouseCursors)
                                     // because it needs mouse cursor feedback.
                                     // (This used to be a per-window
                                     // WindowFlags_ResizeFromAnySide flag)
  bool ConfigWindowsMoveFromTitleBarOnly; // = false       // Enable allowing to
                                          // move windows only when clicking on
                                          // their title bar. Does not apply to
                                          // windows without a title bar.
  float
      ConfigMemoryCompactTimer; // = 60.0f          // Timer (in seconds) to
                                // free transient windows/tables memory buffers
                                // when unused. Set to -1.0f to disable.

  // Inputs Behaviors
  // (other variables, ones which are expected to be tweaked within UI code, are
  // exposed in Style)
  float MouseDoubleClickTime; // = 0.30f          // Time for a double-click, in
                              // seconds.
  float
      MouseDoubleClickMaxDist; // = 6.0f           // Distance threshold to stay
                               // in to validate a double-click, in pixels.
  float MouseDragThreshold;    // = 6.0f           // Distance threshold before
                               // considering we are dragging.
  float KeyRepeatDelay; // = 0.275f         // When holding a key/button, time
                        // before it starts repeating, in seconds (for buttons
                        // in Repeat mode, etc.).
  float KeyRepeatRate; // = 0.050f         // When holding a key/button, rate at
                       // which it repeats, in seconds.

  //------------------------------------------------------------------
  // Debug options
  //------------------------------------------------------------------

  // Tools to test correct Begin/End and BeginChild/EndChild behaviors.
  // Presently Begin()/End() and BeginChild()/EndChild() needs to ALWAYS be
  // called in tandem, regardless of return value of BeginXXX() This is
  // inconsistent with other BeginXXX functions and create confusion for many
  // users. We expect to update the API eventually. In the meanwhile we provide
  // tools to facilitate checking user-code behavior.
  bool ConfigDebugBeginReturnValueOnce; // = false          // First-time calls
                                        // to Begin()/BeginChild() will return
                                        // false. NEEDS TO BE SET AT APPLICATION
                                        // BOOT TIME if you don't want to miss
                                        // windows.
  bool ConfigDebugBeginReturnValueLoop; // = false          // Some calls to
                                        // Begin()/BeginChild() will return
                                        // false. Will cycle through window
                                        // depths then repeat. Suggested use:
                                        // add "io.ConfigDebugBeginReturnValue =
                                        // io.KeyShift" in your main loop then
                                        // occasionally press SHIFT. Windows
                                        // should be flickering while running.

  // Option to deactivate io.AddFocusEvent(false) handling. May facilitate
  // interactions with a debugger when focus loss leads to clearing inputs data.
  // Backends may have other side-effects on focus loss, so this will reduce
  // side-effects but not necessary remove all of them. Consider using e.g.
  // Win32's IsDebuggerPresent() as an additional filter (or see
  // OsIsDebuggerPresent() in test_engine/te_utils.cpp for a Unix
  // compatible version).
  bool ConfigDebugIgnoreFocusLoss; // = false          // Ignore
                                   // io.AddFocusEvent(false), consequently not
                                   // calling io.ClearInputKeys() in input
                                   // processing.

  // Option to audit .ini data
  bool ConfigDebugIniSettings; // = false          // Save .ini data with extra
                               // comments (particularly helpful for Docking,
                               // but makes saving slower)

  //------------------------------------------------------------------
  // Platform Functions
  // (the xxxx backend files are setting those up for you)
  //------------------------------------------------------------------

  // Optional: Platform/Renderer backend name (informational only! will be
  // displayed in About Window) + User data for backend/wrappers to store their
  // own stuff.
  const char *BackendPlatformName; // = NULL
  const char *BackendRendererName; // = NULL
  void *BackendPlatformUserData;   // = NULL           // User data for platform
                                   // backend
  void *BackendRendererUserData;   // = NULL           // User data for renderer
                                   // backend
  void *BackendLanguageUserData;   // = NULL           // User data for non C++
                                   // programming language backend

  // Optional: Access OS clipboard
  // (default to use native Win32 clipboard on Windows, otherwise uses a private
  // clipboard. Override to access OS clipboard on other architectures)
  const char *(*GetClipboardTextFn)(void *user_data);
  void (*SetClipboardTextFn)(void *user_data, const char *text);
  void *ClipboardUserData;

  // Optional: Notify OS Input Method Editor of the screen position of your
  // cursor for text input position (e.g. when using Japanese/Chinese IME on
  // Windows) (default to use native imm32 api on Windows)
  void (*SetPlatformImeDataFn)(Viewport *viewport, PlatformImeData *data);

  // Optional: Platform locale
  Wchar PlatformLocaleDecimalPoint; // '.'              // [Experimental]
                                    // Configure decimal point e.g. '.' or ','
                                    // useful for some languages (e.g.
                                    // German), generally pulled from
                                    // *localeconv()->decimal_point

  //------------------------------------------------------------------
  // Input - Call before calling NewFrame()
  //------------------------------------------------------------------

  // Input Functions
  API void
  AddKeyEvent(Key key,
              bool down); // Queue a new key down/up event. Key should be
                          // "translated" (as in, generally Key_A matches the
                          // key end-user would use to emit an 'A' character)
  API void
  AddKeyAnalogEvent(Key key, bool down,
                    float v); // Queue a new key down/up event for analog values
                              // (e.g. Key_Gamepad_ values). Dead-zones
                              // should be handled by the backend.
  API void AddMousePosEvent(
      float x,
      float y); // Queue a mouse position update. Use -FLT_MAX,-FLT_MAX to
                // signify no mouse (e.g. app not focused and not hovered)
  API void AddMouseButtonEvent(int button,
                               bool down); // Queue a mouse button change
  API void AddMouseWheelEvent(
      float wheel_x,
      float wheel_y); // Queue a mouse wheel update. wheel_y<0: scroll down,
                      // wheel_y>0: scroll up, wheel_x<0: scroll right,
                      // wheel_x>0: scroll left.
  API void
  AddMouseSourceEvent(MouseSource source); // Queue a mouse source change
                                           // (Mouse/TouchScreen/Pen)
  API void AddMouseViewportEvent(
      ID id); // Queue a mouse hovered viewport. Requires backend to set
              // BackendFlags_HasMouseHoveredViewport to call this
              // (for multi-viewport support).
  API void AddFocusEvent(
      bool focused); // Queue a gain/loss of focus for the application
                     // (generally based on OS/platform focus of your window)
  API void AddInputCharacter(unsigned int c); // Queue a new character input
  API void
  AddInputCharacterUTF16(Wchar16 c); // Queue a new character input from a
                                     // UTF-16 character, it can be a surrogate
  API void AddInputCharactersUTF8(
      const char *str); // Queue a new characters input from a UTF-8 string

  API void SetKeyEventNativeData(
      Key key, int native_keycode, int native_scancode,
      int native_legacy_index =
          -1); // [Optional] Specify index for legacy <1.87 IsKeyXXX() functions
               // with native indices + specify native keycode, scancode.
  API void SetAppAcceptingEvents(
      bool accepting_events);  // Set master flag for accepting key/mouse/text
                               // events (default to true). Useful if you have
                               // native dialog boxes that are interrupting your
                               // application loop/refresh, and you want to
                               // disable events being queued while your app is
                               // frozen.
  API void ClearEventsQueue(); // Clear all incoming events.
  API void ClearInputKeys();   // Clear current keyboard/mouse/gamepad state +
                               // current frame text input buffer. Equivalent to
                               // releasing all keys/buttons.
#ifndef DISABLE_OBSOLETE_FUNCTIONS
  API void
  ClearInputCharacters(); // [Obsoleted in 1.89.8] Clear the current frame text
                          // input buffer. Now included within ClearInputKeys().
#endif

  //------------------------------------------------------------------
  // Output - Updated by NewFrame() or EndFrame()/Render()
  // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
  // dispatch your inputs, it is
  //  generally easier and more correct to use their state BEFORE calling
  //  NewFrame(). See FAQ for details!)
  //------------------------------------------------------------------

  bool WantCaptureMouse; // Set when Gui will use mouse inputs, in this
                         // case do not dispatch them to your main
                         // game/application (either way, always pass on mouse
                         // inputs to imgui). (e.g. unclicked mouse is hovering
                         // over an imgui window, widget is active, mouse was
                         // clicked over an imgui window, etc.).
  bool WantCaptureKeyboard; // Set when Gui will use keyboard inputs, in
                            // this case do not dispatch them to your main
                            // game/application (either way, always pass
                            // keyboard inputs to imgui). (e.g. InputText
                            // active, or an imgui window is focused and
                            // navigation is enabled, etc.).
  bool WantTextInput; // Mobile/console: when set, you may display an on-screen
                      // keyboard. This is set by Gui when it wants
                      // textual keyboard input to happen (e.g. when a InputText
                      // widget is active).
  bool WantSetMousePos; // MousePos has been altered, backend should reposition
                        // mouse on next frame. Rarely used! Set only when
                        // ConfigFlags_NavEnableSetMousePos flag is enabled.
  bool WantSaveIniSettings; // When manual .ini load/save is active
                            // (io.IniFilename == NULL), this will be set to
                            // notify your application that you can call
                            // SaveIniSettingsToMemory() and save yourself.
                            // Important: clear io.WantSaveIniSettings yourself
                            // after saving!
  bool NavActive;  // Keyboard/Gamepad navigation is currently allowed (will
                   // handle Key_NavXXX events) = a window is focused and it
                   // doesn't use the WindowFlags_NoNavInputs flag.
  bool NavVisible; // Keyboard/Gamepad navigation is visible and allowed (will
                   // handle Key_NavXXX events).
  float Framerate; // Estimate of application framerate (rolling average over 60
                   // frames, based on io.DeltaTime), in frame per second.
                   // Solely for convenience. Slow applications may not want to
                   // use a moving average or may want to reset underlying
                   // buffers occasionally.
  int MetricsRenderVertices; // Vertices output during last call to Render()
  int MetricsRenderIndices;  // Indices output during last call to Render() =
                             // number of triangles * 3
  int MetricsRenderWindows;  // Number of visible windows
  int MetricsActiveWindows;  // Number of active windows
  Vec2 MouseDelta; // Mouse delta. Note that this is zero if either current or
                   // previous position are invalid (-FLT_MAX,-FLT_MAX), so a
                   // disappearing/reappearing mouse won't have a huge delta.

  // Legacy: before 1.87, we required backend to fill io.KeyMap[] (imgui->native
  // map) during initialization and io.KeysDown[] (native indices) every frame.
  // This is still temporarily supported as a legacy feature. However the new
  // preferred scheme is for backend to call io.AddKeyEvent().
  //   Old (<1.87):  Gui::IsKeyPressed(Gui::GetIO().KeyMap[Key_Space])
  //   --> New (1.87+) Gui::IsKeyPressed(Key_Space)
#ifndef DISABLE_OBSOLETE_KEYIO
  int KeyMap[Key_COUNT];    // [LEGACY] Input: map of indices into the
                            // KeysDown[512] entries array which represent
                            // your "native" keyboard state. The first 512 are
                            // now unused and should be kept zero. Legacy
                            // backend will write into KeyMap[] using
                            // Key_ indices which are always >512.
  bool KeysDown[Key_COUNT]; // [LEGACY] Input: Keyboard keys that are
                            // pressed (ideally left in the "native" order
                            // your engine has access to keyboard keys, so
                            // you can use your own defines/enums for
                            // keys). This used to be [512] sized. It is
                            // now Key_COUNT to allow legacy
                            // io.KeysDown[GetKeyIndex(...)] to work
                            // without an overflow.
  float NavInputs[NavInput_COUNT]; // [LEGACY] Since 1.88, NavInputs[] was
                                   // removed. Backends from 1.60 to 1.86
                                   // won't build. Feed gamepad inputs via
                                   // io.AddKeyEvent() and
                                   // Key_GamepadXXX enums.
#endif
#ifndef DISABLE_OBSOLETE_FUNCTIONS
  void *ImeWindowHandle; // = NULL   // [Obsoleted in 1.87] Set
                         // Viewport::PlatformHandleRaw instead. Set this to
                         // your HWND to get automatic IME cursor positioning.
#else
  void *_UnusedPadding;
#endif

  //------------------------------------------------------------------
  // [Internal] Gui will maintain those fields. Forward compatibility not
  // guaranteed!
  //------------------------------------------------------------------

  Context *Ctx; // Parent UI context (needs to be set explicitly by parent).

  // Main Input State
  // (this block used to be written by backend, since 1.87 it is best to NOT
  // write to those directly, call the AddXXX functions above instead) (reading
  // from those variables is fair game, as they are extremely unlikely to be
  // moving anywhere)
  Vec2 MousePos; // Mouse position, in pixels. Set to Vec2(-FLT_MAX, -FLT_MAX)
                 // if mouse is unavailable (on another screen, etc.)
  bool MouseDown[5]; // Mouse buttons: 0=left, 1=right, 2=middle + extras
                     // (MouseButton_COUNT == 5). Gui mostly uses
                     // left and right buttons. Other buttons allow us to track
                     // if the mouse is being used by your application +
                     // available to user as a convenience via IsMouse** API.
  float MouseWheel;  // Mouse wheel Vertical: 1 unit scrolls about 5 lines text.
                     // >0 scrolls Up, <0 scrolls Down. Hold SHIFT to turn
                     // vertical scroll into horizontal scroll.
  float MouseWheelH; // Mouse wheel Horizontal. >0 scrolls Left, <0 scrolls
                     // Right. Most users don't have a mouse with a horizontal
                     // wheel, may not be filled by all backends.
  MouseSource
      MouseSource; // Mouse actual input peripheral (Mouse/TouchScreen/Pen).
  ID MouseHoveredViewport; // (Optional) Modify using
                           // io.AddMouseViewportEvent(). With
                           // multi-viewports: viewport the OS mouse is
                           // hovering. If possible _IGNORING_ viewports
                           // with the ViewportFlags_NoInputs flag is
                           // much better (few backends can handle that).
                           // Set io.BackendFlags |=
                           // BackendFlags_HasMouseHoveredViewport if
                           // you can provide this info. If you don't imgui
                           // will infer the value using the rectangles and
                           // last focused time of the viewports it knows
                           // about (ignoring other OS windows).
  bool KeyCtrl;            // Keyboard modifier down: Control
  bool KeyShift;           // Keyboard modifier down: Shift
  bool KeyAlt;             // Keyboard modifier down: Alt
  bool KeySuper;           // Keyboard modifier down: Cmd/Super/Windows

  // Other state maintained from data above + IO function calls
  KeyChord KeyMods; // Key mods flags (any of
                    // Mod_Ctrl/Mod_Shift/Mod_Alt/Mod_Super
                    // flags, same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but
                    // merged into flags. DOES NOT CONTAINS Mod_Shortcut
                    // which is pretranslated). Read-only, updated by NewFrame()
  KeyData KeysData[Key_KeysData_SIZE];   // Key state for all known keys. Use
                                         // IsKeyXXX() functions to access this.
  bool WantCaptureMouseUnlessPopupClose; // Alternative to WantCaptureMouse:
                                         // (WantCaptureMouse == true &&
                                         // WantCaptureMouseUnlessPopupClose ==
                                         // false) when a click over void is
                                         // expected to close a popup.
  Vec2 MousePosPrev; // Previous mouse position (note that MouseDelta is not
                     // necessary == MousePos-MousePosPrev, in case either
                     // position is invalid)
  Vec2 MouseClickedPos[5];    // Position at time of clicking
  double MouseClickedTime[5]; // Time of last click (used to figure out
                              // double-click)
  bool MouseClicked[5];       // Mouse button went from !Down to Down (same as
                              // MouseClickedCount[x] != 0)
  bool MouseDoubleClicked[5]; // Has mouse button been double-clicked? (same as
                              // MouseClickedCount[x] == 2)
  U16 MouseClickedCount[5];   // == 0 (not clicked), == 1 (same as
                              // MouseClicked[]), == 2 (double-clicked), == 3
                              // (triple-clicked) etc. when going from !Down to
                              // Down
  U16 MouseClickedLastCount[5]; // Count successive number of clicks. Stays
                                // valid after mouse release. Reset after
                                // another click is done.
  bool MouseReleased[5];        // Mouse button went from Down to !Down
  bool MouseDownOwned[5];       // Track if button was clicked inside a gui
                          // window or over void blocked by a popup. We don't
                          // request mouse capture from the application if click
                          // started outside Gui bounds.
  bool MouseDownOwnedUnlessPopupClose[5]; // Track if button was clicked inside
                                          // a gui window.
  bool MouseWheelRequestAxisSwap; // On a non-Mac system, holding SHIFT requests
                                  // WheelY to perform the equivalent of a
                                  // WheelX event. On a Mac system this is
                                  // already enforced by the system.
  float MouseDownDuration[5]; // Duration the mouse button has been down (0.0f
                              // == just clicked)
  float
      MouseDownDurationPrev[5]; // Previous time the mouse button has been down
  Vec2 MouseDragMaxDistanceAbs[5];  // Maximum distance, absolute, on each
                                    // axis, of how much mouse has traveled
                                    // from the clicking point
  float MouseDragMaxDistanceSqr[5]; // Squared maximum distance of how much
                                    // mouse has traveled from the clicking
                                    // point (used for moving thresholds)
  float PenPressure; // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only
                     // when MouseDown[0] == true). Helper storage currently
                     // unused by Gui.
  bool AppFocusLost; // Only modify via AddFocusEvent()
  bool AppAcceptingEvents;        // Only modify via SetAppAcceptingEvents()
  S8 BackendUsingLegacyKeyArrays; // -1: unknown, 0: using AddKeyEvent(), 1:
                                  // using legacy io.KeysDown[]
  bool BackendUsingLegacyNavInputArray; // 0: using AddKeyAnalogEvent(), 1:
                                        // writing to legacy io.NavInputs[]
                                        // directly
  Wchar16 InputQueueSurrogate;          // For AddInputCharacterUTF16()
  Vector<Wchar>
      InputQueueCharacters; // Queue of _characters_ input (obtained by platform
                            // backend). Fill using AddInputCharacter() helper.

  API IO();
};

//-----------------------------------------------------------------------------
// [SECTION] Misc data structures (InputTextCallbackData,
// SizeCallbackData, Payload)
//-----------------------------------------------------------------------------

// Shared state of InputText(), passed as an argument to your callback when a
// InputTextFlags_Callback* flag is used. The callback function should
// return 0 by default. Callbacks (follow a flag name and see comments in
// InputTextFlags_ declarations for more details)
// - InputTextFlags_CallbackEdit:        Callback on buffer edit (note that
// InputText() already returns true on edit, the callback is useful mainly to
// manipulate the underlying buffer while focus is active)
// - InputTextFlags_CallbackAlways:      Callback on each iteration
// - InputTextFlags_CallbackCompletion:  Callback on pressing TAB
// - InputTextFlags_CallbackHistory:     Callback on pressing Up/Down
// arrows
// - InputTextFlags_CallbackCharFilter:  Callback on character inputs to
// replace or discard them. Modify 'EventChar' to replace or discard, or return
// 1 in callback to discard.
// - InputTextFlags_CallbackResize:      Callback on buffer capacity
// changes request (beyond 'buf_size' parameter value), allowing the string to
// grow.
struct InputTextCallbackData {
  Context *Ctx;             // Parent UI context
  InputTextFlags EventFlag; // One InputTextFlags_Callback*    // Read-only
  InputTextFlags Flags;     // What user passed to InputText()      // Read-only
  void *UserData;           // What user passed to InputText()      // Read-only

  // Arguments for the different callback events
  // - To modify the text buffer in a callback, prefer using the InsertChars() /
  // DeleteChars() function. InsertChars() will take care of calling the resize
  // callback if necessary.
  // - If you know your edits are not going to resize the underlying buffer
  // allocation, you may modify the contents of 'Buf[]' directly. You need to
  // update 'BufTextLen' accordingly (0 <= BufTextLen < BufSize) and set
  // 'BufDirty'' to true so InputText can update its internal state.
  Wchar
      EventChar; // Character input                      // Read-write   //
                 // [CharFilter] Replace character with another one, or set to
                 // zero to drop. return 1 is equivalent to setting EventChar=0;
  Key EventKey;  // Key pressed (Up/Down/TAB)            // Read-only    //
                 // [Completion,History]
  char *Buf; // Text buffer                          // Read-write   // [Resize]
             // Can replace pointer / [Completion,History,Always] Only write to
             // pointed data, don't replace the actual pointer!
  int BufTextLen; // Text length (in bytes)               // Read-write   //
                  // [Resize,Completion,History,Always] Exclude zero-terminator
                  // storage. In C land: == strlen(some_text), in C++ land:
                  // string.length()
  int BufSize;    // Buffer size (in bytes) = capacity+1  // Read-only    //
                  // [Resize,Completion,History,Always] Include zero-terminator
  // storage. In C land == ARRAYSIZE(my_char_array), in C++ land:
  // string.capacity()+1
  bool BufDirty; // Set if you modify Buf/BufTextLen!    // Write        //
                 // [Completion,History,Always]
  int CursorPos; //                                      // Read-write   //
                 //                                      [Completion,History,Always]
  int SelectionStart; //                                      // Read-write   //
                      //                                      [Completion,History,Always]
                      //                                      == to SelectionEnd
                      //                                      when no selection)
  int SelectionEnd; //                                      // Read-write   //
                    //                                      [Completion,History,Always]

  // Helper functions for text manipulation.
  // Use those function to benefit from the CallbackResize behaviors. Calling
  // those function reset the selection.
  API InputTextCallbackData();
  API void DeleteChars(int pos, int bytes_count);
  API void InsertChars(int pos, const char *text, const char *text_end = NULL);
  void SelectAll() {
    SelectionStart = 0;
    SelectionEnd = BufTextLen;
  }
  void ClearSelection() { SelectionStart = SelectionEnd = BufTextLen; }
  bool HasSelection() const { return SelectionStart != SelectionEnd; }
};

// Resizing callback data to apply custom constraint. As enabled by
// SetNextWindowSizeConstraints(). Callback is called during the next Begin().
// NB: For basic min/max size constraint on each axis you don't need to use the
// callback! The SetNextWindowSizeConstraints() parameters are enough.
struct SizeCallbackData {
  void *UserData; // Read-only.   What user passed to
                  // SetNextWindowSizeConstraints(). Generally store an integer
                  // or float in here (need reinterpret_cast<>).
  Vec2 Pos;       // Read-only.   Window position, for reference.
  Vec2 CurrentSize; // Read-only.   Current window size.
  Vec2 DesiredSize; // Read-write.  Desired size, based on user's mouse
                    // position. Write to this field to restrain resizing.
};

// [ALPHA] Rarely used / very advanced uses only. Use with SetNextWindowClass()
// and DockSpace() functions. Important: the content of this class is still
// highly WIP and likely to change and be refactored before we stabilize Docking
// features. Please be mindful if using this. Provide hints:
// - To the platform backend via altered viewport flags (enable/disable OS
// decoration, OS task bar icons, etc.)
// - To the platform backend for OS level parent/child relationships of
// viewport.
// - To the docking system for various options and filtering.
struct WindowClass {
  ID ClassId;          // User data. 0 = Default class (unclassed). Windows of
                       // different classes cannot be docked with each others.
  ID ParentViewportId; // Hint for the platform backend. -1: use default. 0:
                       // request platform backend to not parent the platform.
                       // != 0: request platform backend to create a
                       // parent<>child relationship between the platform
                       // windows. Not conforming backends are free to e.g.
                       // parent every viewport to the main viewport or not.
  ViewportFlags
      ViewportFlagsOverrideSet; // Viewport flags to set when a window of this
                                // class owns a viewport. This allows you to
                                // enforce OS decoration or task bar icon,
                                // override the defaults on a per-window basis.
  ViewportFlags
      ViewportFlagsOverrideClear; // Viewport flags to clear when a window of
                                  // this class owns a viewport. This allows you
                                  // to enforce OS decoration or task bar icon,
                                  // override the defaults on a per-window
                                  // basis.
  TabItemFlags TabItemFlagsOverrideSet; // [EXPERIMENTAL] TabItem flags to set
                                        // when a window of this class gets
                                        // submitted into a dock node tab bar.
                                        // May use with TabItemFlags_Leading or
                                        // TabItemFlags_Trailing.
  DockNodeFlags
      DockNodeFlagsOverrideSet; // [EXPERIMENTAL] Dock node flags to set when a
                                // window of this class is hosted by a dock node
                                // (it doesn't have to be selected!)
  bool DockingAlwaysTabBar; // Set to true to enforce single floating windows of
                            // this class always having their own docking node
                            // (equivalent of setting the global
                            // io.ConfigDockingAlwaysTabBar)
  bool DockingAllowUnclassed; // Set to true to allow windows of this class to
                              // be docked/merged with an unclassed window. //
                              // FIXME-DOCK: Move to DockNodeFlags override?

  WindowClass() {
    memset(this, 0, sizeof(*this));
    ParentViewportId = (ID)-1;
    DockingAllowUnclassed = true;
  }
};

// Data payload for Drag and Drop operations: AcceptDragDropPayload(),
// GetDragDropPayload()
struct Payload {
  // Members
  void *Data;   // Data (copied and owned by gui)
  int DataSize; // Data size

  // [Internal]
  ID SourceId;           // Source item id
  ID SourceParentId;     // Source parent id (if available)
  int DataFrameCount;    // Data timestamp
  char DataType[32 + 1]; // Data type tag (short user-supplied string, 32
                         // characters max)
  bool
      Preview; // Set when AcceptDragDropPayload() was called and mouse has been
               // hovering the target item (nb: handle overlapping drag targets)
  bool Delivery; // Set when AcceptDragDropPayload() was called and mouse button
                 // is released over the target item.

  Payload() { Clear(); }
  void Clear() {
    SourceId = SourceParentId = 0;
    Data = NULL;
    DataSize = 0;
    memset(DataType, 0, sizeof(DataType));
    DataFrameCount = -1;
    Preview = Delivery = false;
  }
  bool IsDataType(const char *type) const {
    return DataFrameCount != -1 && strcmp(type, DataType) == 0;
  }
  bool IsPreview() const { return Preview; }
  bool IsDelivery() const { return Delivery; }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers (OnceUponAFrame, TextFilter, TextBuffer,
// Storage, ListClipper, Math Operators, Color)
//-----------------------------------------------------------------------------

// Helper: Unicode defines
#define UNICODE_CODEPOINT_INVALID                                              \
  0xFFFD // Invalid Unicode code point (standard value).
#ifdef USE_WCHAR32
#define UNICODE_CODEPOINT_MAX                                                  \
  0x10FFFF // Maximum Unicode code point supported by this build.
#else
#define UNICODE_CODEPOINT_MAX                                                  \
  0xFFFF // Maximum Unicode code point supported by this build.
#endif

// Helper: Execute a block of code at maximum once a frame. Convenient if you
// want to quickly create a UI within deep-nested code that runs multiple times
// every frame. Usage: static OnceUponAFrame oaf; if (oaf)
// Gui::Text("This will be called only once per frame");
struct OnceUponAFrame {
  OnceUponAFrame() { RefFrame = -1; }
  mutable int RefFrame;
  operator bool() const {
    int current_frame = Gui::GetFrameCount();
    if (RefFrame == current_frame)
      return false;
    RefFrame = current_frame;
    return true;
  }
};

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
struct TextFilter {
  API TextFilter(const char *default_filter = "");
  API bool Draw(const char *label = "Filter (inc,-exc)",
                float width = 0.0f); // Helper calling InputText+Build
  API bool PassFilter(const char *text, const char *text_end = NULL) const;
  API void Build();
  void Clear() {
    InputBuf[0] = 0;
    Build();
  }
  bool IsActive() const { return !Filters.empty(); }

  // [Internal]
  struct TextRange {
    const char *b;
    const char *e;

    TextRange() { b = e = NULL; }
    TextRange(const char *_b, const char *_e) {
      b = _b;
      e = _e;
    }
    bool empty() const { return b == e; }
    API void split(char separator, Vector<TextRange> *out) const;
  };
  char InputBuf[256];
  Vector<TextRange> Filters;
  int CountGrep;
};

// Helper: Growable text buffer for logging/accumulating text
// (this could be called 'TextBuilder' / 'StringBuilder')
struct TextBuffer {
  Vector<char> Buf;
  API static char EmptyString[1];

  TextBuffer() {}
  inline char operator[](int i) const {
    ASSERT(Buf.Data != NULL);
    return Buf.Data[i];
  }
  const char *begin() const { return Buf.Data ? &Buf.front() : EmptyString; }
  const char *end() const {
    return Buf.Data ? &Buf.back() : EmptyString;
  } // Buf is zero-terminated, so end() will point on the zero-terminator
  int size() const { return Buf.Size ? Buf.Size - 1 : 0; }
  bool empty() const { return Buf.Size <= 1; }
  void clear() { Buf.clear(); }
  void reserve(int capacity) { Buf.reserve(capacity); }
  const char *c_str() const { return Buf.Data ? Buf.Data : EmptyString; }
  API void append(const char *str, const char *str_end = NULL);
  API void appendf(const char *fmt, ...) FMTARGS(2);
  API void appendfv(const char *fmt, va_list args) FMTLIST(2);
};

// Helper: Key->Value storage
// Typically you don't have to worry about this since a storage is held within
// each Window. We use it to e.g. store collapse state for a tree (Int 0/1) This
// is optimized for efficient lookup (dichotomy into a contiguous buffer) and
// rare insertion (typically tied to user interactions aka max once a frame) You
// can use it as custom user storage for temporary values. Declare your own
// storage if, for example:
// - You want to manipulate the open/close state of a particular sub-tree in
// your interface (tree node uses Int 0/1 to store their state).
// - You want to store custom debug data easily without adding or editing
// structures in your code (probably not efficient, but convenient) Types are
// NOT stored, so it is up to you to make sure your Key don't collide with
// different types.
struct Storage {
  // [Internal]
  struct StoragePair {
    ID key;
    union {
      int val_i;
      float val_f;
      void *val_p;
    };
    StoragePair(ID _key, int _val) {
      key = _key;
      val_i = _val;
    }
    StoragePair(ID _key, float _val) {
      key = _key;
      val_f = _val;
    }
    StoragePair(ID _key, void *_val) {
      key = _key;
      val_p = _val;
    }
  };

  Vector<StoragePair> Data;

  // - Get***() functions find pair, never add/allocate. Pairs are sorted so a
  // query is O(log N)
  // - Set***() functions find pair, insertion on demand if missing.
  // - Sorted insertion is costly, paid once. A typical frame shouldn't need to
  // insert any new pair.
  void Clear() { Data.clear(); }
  API int GetInt(ID key, int default_val = 0) const;
  API void SetInt(ID key, int val);
  API bool GetBool(ID key, bool default_val = false) const;
  API void SetBool(ID key, bool val);
  API float GetFloat(ID key, float default_val = 0.0f) const;
  API void SetFloat(ID key, float val);
  API void *GetVoidPtr(ID key) const; // default_val is NULL
  API void SetVoidPtr(ID key, void *val);

  // - Get***Ref() functions finds pair, insert on demand if missing, return
  // pointer. Useful if you intend to do Get+Set.
  // - References are only valid until a new value is added to the storage.
  // Calling a Set***() function or a Get***Ref() function invalidates the
  // pointer.
  // - A typical use case where this is convenient for quick hacking (e.g. add
  // storage during a live Edit&Continue session if you can't modify existing
  // struct)
  //      float* pvar = Gui::GetFloatRef(key); Gui::SliderFloat("var", pvar,
  //      0, 100.0f); some_var += *pvar;
  API int *GetIntRef(ID key, int default_val = 0);
  API bool *GetBoolRef(ID key, bool default_val = false);
  API float *GetFloatRef(ID key, float default_val = 0.0f);
  API void **GetVoidPtrRef(ID key, void *default_val = NULL);

  // Advanced: for quicker full rebuild of a storage (instead of an incremental
  // one), you may add all your contents and then sort once.
  API void BuildSortByKey();
  // Obsolete: use on your own storage if you know only integer are being stored
  // (open/close all tree nodes)
  API void SetAllInt(int val);
};

// Helper: Manually clip large list of items.
// If you have lots evenly spaced items and you have random access to the list,
// you can perform coarse clipping based on visibility to only submit items that
// are in view. The clipper calculates the range of visible items and advance
// the cursor to compensate for the non-visible items we have skipped. (
// Gui already clip items based on their bounds but: it needs to first layout
// the item to do so, and generally
//  fetching/submitting your own data incurs additional cost. Coarse clipping
//  using ListClipper allows you to easily scale using lists with tens of
//  thousands of items without a problem)
// Usage:
//   ListClipper clipper;
//   clipper.Begin(1000);         // We have 1000 elements, evenly spaced.
//   while (clipper.Step())
//       for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
//           Gui::Text("line number %d", i);
// Generally what happens is:
// - Clipper lets you process the first element (DisplayStart = 0, DisplayEnd =
// 1) regardless of it being visible or not.
// - User code submit that one element.
// - Clipper can measure the height of the first element
// - Clipper calculate the actual range of elements to display based on the
// current clipping rectangle, position the cursor before the first visible
// element.
// - User code submit visible elements.
// - The clipper also handles various subtleties related to keyboard/gamepad
// navigation, wrapping etc.
struct ListClipper {
  Context *Ctx;      // Parent UI context
  int DisplayStart;  // First item to display, updated by each call to Step()
  int DisplayEnd;    // End of items to display (exclusive)
  int ItemsCount;    // [Internal] Number of items
  float ItemsHeight; // [Internal] Height of item after a first step and item
                     // submission can calculate it
  float StartPosY; // [Internal] Cursor position at the time of Begin() or after
                   // table frozen rows are all processed
  void *TempData;  // [Internal] Internal data

  // items_count: Use INT_MAX if you don't know how many items you have (in
  // which case the cursor won't be advanced in the final step) items_height:
  // Use -1.0f to be calculated automatically on first step. Otherwise pass in
  // the distance between your items, typically GetTextLineHeightWithSpacing()
  // or GetFrameHeightWithSpacing().
  API ListClipper();
  API ~ListClipper();
  API void Begin(int items_count, float items_height = -1.0f);
  API void
  End(); // Automatically called on the last call of Step() that returns false.
  API bool Step(); // Call until it returns false. The DisplayStart/DisplayEnd
                   // fields will be set and you can process/draw those items.

  // Call IncludeItemByIndex() or IncludeItemsByIndex() *BEFORE* first call to
  // Step() if you need a range of items to not be clipped, regardless of their
  // visibility. (Due to alignment / padding of certain items it is possible
  // that an extra item may be included on either end of the display range).
  inline void IncludeItemByIndex(int item_index) {
    IncludeItemsByIndex(item_index, item_index + 1);
  }
  API void
  IncludeItemsByIndex(int item_begin,
                      int item_end); // item_end is exclusive e.g. use (42,
                                     // 42+1) to make item 42 never clipped.

#ifndef DISABLE_OBSOLETE_FUNCTIONS
  inline void IncludeRangeByIndices(int item_begin, int item_end) {
    IncludeItemsByIndex(item_begin, item_end);
  } // [renamed in 1.89.9]
  inline void ForceDisplayRangeByIndices(int item_begin, int item_end) {
    IncludeItemsByIndex(item_begin, item_end);
  } // [renamed in 1.89.6]
    // inline ListClipper(int items_count, float items_height = -1.0f) {
  // memset(this, 0, sizeof(*this)); ItemsCount = -1; Begin(items_count,
  // items_height); } // [removed in 1.79]
#endif
};

// Helpers: Vec2/Vec4 operators
// - It is important that we are keeping those disabled by default so they don't
// leak in user space.
// - This is in order to allow user enabling implicit cast operators between
// Vec2/Vec4 and their own types (using VEC2_CLASS_EXTRA in config.hpp)
// - You can use '#define DEFINE_MATH_OPERATORS' to import our operators,
// provided as a courtesy.
#ifdef DEFINE_MATH_OPERATORS
#define DEFINE_MATH_OPERATORS_IMPLEMENTED
MSVC_RUNTIME_CHECKS_OFF
static inline Vec2 operator*(const Vec2 &lhs, const float rhs) {
  return Vec2(lhs.x * rhs, lhs.y * rhs);
}
static inline Vec2 operator/(const Vec2 &lhs, const float rhs) {
  return Vec2(lhs.x / rhs, lhs.y / rhs);
}
static inline Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs) {
  return Vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
static inline Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs) {
  return Vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}
static inline Vec2 operator*(const Vec2 &lhs, const Vec2 &rhs) {
  return Vec2(lhs.x * rhs.x, lhs.y * rhs.y);
}
static inline Vec2 operator/(const Vec2 &lhs, const Vec2 &rhs) {
  return Vec2(lhs.x / rhs.x, lhs.y / rhs.y);
}
static inline Vec2 operator-(const Vec2 &lhs) { return Vec2(-lhs.x, -lhs.y); }
static inline Vec2 &operator*=(Vec2 &lhs, const float rhs) {
  lhs.x *= rhs;
  lhs.y *= rhs;
  return lhs;
}
static inline Vec2 &operator/=(Vec2 &lhs, const float rhs) {
  lhs.x /= rhs;
  lhs.y /= rhs;
  return lhs;
}
static inline Vec2 &operator+=(Vec2 &lhs, const Vec2 &rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}
static inline Vec2 &operator-=(Vec2 &lhs, const Vec2 &rhs) {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  return lhs;
}
static inline Vec2 &operator*=(Vec2 &lhs, const Vec2 &rhs) {
  lhs.x *= rhs.x;
  lhs.y *= rhs.y;
  return lhs;
}
static inline Vec2 &operator/=(Vec2 &lhs, const Vec2 &rhs) {
  lhs.x /= rhs.x;
  lhs.y /= rhs.y;
  return lhs;
}
static inline bool operator==(const Vec2 &lhs, const Vec2 &rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}
static inline bool operator!=(const Vec2 &lhs, const Vec2 &rhs) {
  return lhs.x != rhs.x || lhs.y != rhs.y;
}
static inline Vec4 operator+(const Vec4 &lhs, const Vec4 &rhs) {
  return Vec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}
static inline Vec4 operator-(const Vec4 &lhs, const Vec4 &rhs) {
  return Vec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}
static inline Vec4 operator*(const Vec4 &lhs, const Vec4 &rhs) {
  return Vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}
static inline bool operator==(const Vec4 &lhs, const Vec4 &rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}
static inline bool operator!=(const Vec4 &lhs, const Vec4 &rhs) {
  return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w;
}
MSVC_RUNTIME_CHECKS_RESTORE
#endif

// Helpers macros to generate 32-bit encoded colors
// User can declare their own format by #defining the 5 _SHIFT/_MASK macros in
// their imconfig file.
#ifndef COL32_R_SHIFT
#ifdef USE_BGRA_PACKED_COLOR
#define COL32_R_SHIFT 16
#define COL32_G_SHIFT 8
#define COL32_B_SHIFT 0
#define COL32_A_SHIFT 24
#define COL32_A_MASK 0xFF000000
#else
#define COL32_R_SHIFT 0
#define COL32_G_SHIFT 8
#define COL32_B_SHIFT 16
#define COL32_A_SHIFT 24
#define COL32_A_MASK 0xFF000000
#endif
#endif
#define COL32(R, G, B, A)                                                      \
  (((U32)(A) << COL32_A_SHIFT) | ((U32)(B) << COL32_B_SHIFT) |                 \
   ((U32)(G) << COL32_G_SHIFT) | ((U32)(R) << COL32_R_SHIFT))
#define COL32_WHITE COL32(255, 255, 255, 255) // Opaque white = 0xFFFFFFFF
#define COL32_BLACK COL32(0, 0, 0, 255)       // Opaque black
#define COL32_BLACK_TRANS COL32(0, 0, 0, 0)   // Transparent black = 0x00000000

// Helper: Color() implicitly converts colors to either U32 (packed 4x1
// byte) or Vec4 (4x1 float) Prefer using COL32() macros if you want a
// guaranteed compile-time U32 for usage with DrawList API.
// **Avoid storing Color! Store either u32 of Vec4. This is not a
// full-featured color class. MAY OBSOLETE.
// **None of the Gui API are using Color directly but you can use it as a
// convenience to pass colors in either U32 or Vec4 formats. Explicitly cast
// to U32 or Vec4 if needed.
struct Color {
  Vec4 Value;

  constexpr Color() {}
  constexpr Color(float r, float g, float b, float a = 1.0f)
      : Value(r, g, b, a) {}
  constexpr Color(const Vec4 &col) : Value(col) {}
  constexpr Color(int r, int g, int b, int a = 255)
      : Value((float)r * (1.0f / 255.0f), (float)g * (1.0f / 255.0f),
              (float)b * (1.0f / 255.0f), (float)a * (1.0f / 255.0f)) {}
  constexpr Color(U32 rgba)
      : Value((float)((rgba >> COL32_R_SHIFT) & 0xFF) * (1.0f / 255.0f),
              (float)((rgba >> COL32_G_SHIFT) & 0xFF) * (1.0f / 255.0f),
              (float)((rgba >> COL32_B_SHIFT) & 0xFF) * (1.0f / 255.0f),
              (float)((rgba >> COL32_A_SHIFT) & 0xFF) * (1.0f / 255.0f)) {}
  inline operator U32() const { return Gui::ColorConvertFloat4ToU32(Value); }
  inline operator Vec4() const { return Value; }

  // FIXME-OBSOLETE: May need to obsolete/cleanup those helpers.
  inline void SetHSV(float h, float s, float v, float a = 1.0f) {
    Gui::ColorConvertHSVtoRGB(h, s, v, Value.x, Value.y, Value.z);
    Value.w = a;
  }
  static Color HSV(float h, float s, float v, float a = 1.0f) {
    float r, g, b;
    Gui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
    return Color(r, g, b, a);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Drawing API (DrawCmd, DrawIdx, DrawVert, DrawChannel,
// DrawListSplitter, DrawListFlags, DrawList, DrawData) Hold a series of
// drawing commands. The user provides a renderer for DrawData which
// essentially contains an array of DrawList.
//-----------------------------------------------------------------------------

// The maximum line width to bake anti-aliased textures for. Build atlas with
// FontAtlasFlags_NoBakedLines to disable baking.
#ifndef DRAWLIST_TEX_LINES_WIDTH_MAX
#define DRAWLIST_TEX_LINES_WIDTH_MAX (63)
#endif

// DrawCallback: Draw callbacks for advanced uses [configurable type: override
// in config.hpp] NB: You most likely do NOT need to use draw callbacks just to
// create your own widget or customized UI rendering, you can poke into the draw
// list for that! Draw callback may be useful for example to:
//  A) Change your GPU render state,
//  B) render a complex 3D scene inside a UI element without an intermediate
//  texture/render target, etc.
// The expected behavior from your rendering function is 'if (cmd.UserCallback
// != NULL) { cmd.UserCallback(parent_list, cmd); } else { RenderTriangles() }'
// If you want to override the signature of DrawCallback, you can simply use
// e.g. '#define DrawCallback MyDrawCallback' (in config.hpp) + update
// rendering backend accordingly.
#ifndef DrawCallback
typedef void (*DrawCallback)(const DrawList *parent_list, const DrawCmd *cmd);
#endif

// Special Draw callback value to request renderer backend to reset the
// graphics/render state. The renderer backend needs to handle this special
// value, otherwise it will crash trying to call a function at this address.
// This is useful, for example, if you submitted callbacks which you know have
// altered the render state and you want it to be restored. Render state is not
// reset by default because they are many perfectly useful way of altering
// render state (e.g. changing shader/blending settings before an Image call).
#define DrawCallback_ResetRenderState (DrawCallback)(-8)

// Typically, 1 command = 1 GPU draw call (unless command is a callback)
// - VtxOffset: When 'io.BackendFlags & BackendFlags_RendererHasVtxOffset'
// is enabled,
//   this fields allow us to render meshes larger than 64K vertices while
//   keeping 16-bit indices. Backends made for <1.71. will typically ignore the
//   VtxOffset fields.
// - The ClipRect/TextureId/VtxOffset fields must be contiguous as we memcmp()
// them together (this is asserted for).
struct DrawCmd {
  Vec4 ClipRect;       // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract
                       // DrawData->DisplayPos to get clipping rectangle in
                       // "viewport" coordinates
  TextureID TextureId; // 4-8  // User-provided texture ID. Set by user in
                       // ImfontAtlas::SetTexID() for fonts or passed to
                       // Image*() functions. Ignore if never using images or
                       // multiple fonts atlas.
  unsigned int VtxOffset; // 4    // Start offset in vertex buffer.
                          // BackendFlags_RendererHasVtxOffset: always 0,
                          // otherwise may be >0 to support meshes larger than
                          // 64K vertices with 16-bit indices.
  unsigned int IdxOffset; // 4    // Start offset in index buffer.
  unsigned int
      ElemCount; // 4    // Number of indices (multiple of 3) to be rendered as
                 // triangles. Vertices are stored in the callee DrawList's
                 // vtx_buffer[] array, indices in idx_buffer[].
  DrawCallback UserCallback; // 4-8  // If != NULL, call the function instead
                             // of rendering the vertices. clip_rect and
                             // texture_id will be set normally.
  void *UserCallbackData;    // 4-8  // The draw callback code can access this.

  DrawCmd() {
    memset(this, 0, sizeof(*this));
  } // Also ensure our padding fields are zeroed

  // Since 1.83: returns TextureID associated with this draw call. Warning: DO
  // NOT assume this is always same as 'TextureId' (we will change this function
  // for an upcoming feature)
  inline TextureID GetTexID() const { return TextureId; }
};

// Vertex layout
#ifndef OVERRIDE_DRAWVERT_STRUCT_LAYOUT
struct DrawVert {
  Vec2 pos;
  Vec2 uv;
  U32 col;
};
#else
// You can override the vertex format layout by defining
// OVERRIDE_DRAWVERT_STRUCT_LAYOUT in config.hpp The code expect Vec2 pos (8
// bytes), Vec2 uv (8 bytes), U32 col (4 bytes), but you can re-order them
// or add other fields as needed to simplify integration in your engine. The
// type has to be described within the macro (you can either declare the struct
// or use a typedef). This is because Vec2/U32 are likely not declared at
// the time you'd want to set your type up. NOTE: GUI DOESN'T CLEAR THE
// STRUCTURE AND DOESN'T CALL A CONSTRUCTOR SO ANY CUSTOM FIELD WILL BE
// UNINITIALIZED. IF YOU ADD EXTRA FIELDS (SUCH AS A 'Z' COORDINATES) YOU WILL
// NEED TO CLEAR THEM DURING RENDER OR TO IGNORE THEM.
OVERRIDE_DRAWVERT_STRUCT_LAYOUT;
#endif

// [Internal] For use by DrawList
struct DrawCmdHeader {
  Vec4 ClipRect;
  TextureID TextureId;
  unsigned int VtxOffset;
};

// [Internal] For use by DrawListSplitter
struct DrawChannel {
  Vector<DrawCmd> _CmdBuffer;
  Vector<DrawIdx> _IdxBuffer;
};

// Split/Merge functions are used to split the draw list into different layers
// which can be drawn into out of order. This is used by the Columns/Tables API,
// so items of each column can be batched together in a same draw call.
struct DrawListSplitter {
  int _Current;                  // Current channel number (0)
  int _Count;                    // Number of active channels (1+)
  Vector<DrawChannel> _Channels; // Draw channels (not resized down so
                                 // _Count might be < Channels.Size)

  inline DrawListSplitter() { memset(this, 0, sizeof(*this)); }
  inline ~DrawListSplitter() { ClearFreeMemory(); }
  inline void Clear() {
    _Current = 0;
    _Count = 1;
  } // Do not clear Channels[] so our allocations are reused next frame
  API void ClearFreeMemory();
  API void Split(DrawList *draw_list, int count);
  API void Merge(DrawList *draw_list);
  API void SetCurrentChannel(DrawList *draw_list, int channel_idx);
};

// Flags for DrawList functions
// (Legacy: bit 0 must always correspond to DrawFlags_Closed to be backward
// compatible with old API using a bool. Bits 1..3 must be unused)
enum DrawFlags_ {
  DrawFlags_None = 0,
  DrawFlags_Closed =
      1 << 0, // PathStroke(), AddPolyline(): specify that shape should be
              // closed (Important: this is always == 1 for legacy reason)
  DrawFlags_RoundCornersTopLeft =
      1 << 4, // AddRect(), AddRectFilled(), PathRect(): enable rounding
              // top-left corner only (when rounding > 0.0f, we default to all
              // corners). Was 0x01.
  DrawFlags_RoundCornersTopRight =
      1 << 5, // AddRect(), AddRectFilled(), PathRect(): enable rounding
              // top-right corner only (when rounding > 0.0f, we default to all
              // corners). Was 0x02.
  DrawFlags_RoundCornersBottomLeft =
      1 << 6, // AddRect(), AddRectFilled(), PathRect(): enable rounding
              // bottom-left corner only (when rounding > 0.0f, we default to
              // all corners). Was 0x04.
  DrawFlags_RoundCornersBottomRight =
      1 << 7, // AddRect(), AddRectFilled(), PathRect(): enable rounding
              // bottom-right corner only (when rounding > 0.0f, we default to
              // all corners). Wax 0x08.
  DrawFlags_RoundCornersNone =
      1 << 8, // AddRect(), AddRectFilled(), PathRect(): disable rounding on all
              // corners (when rounding > 0.0f). This is NOT zero, NOT an
              // implicit flag!
  DrawFlags_RoundCornersTop =
      DrawFlags_RoundCornersTopLeft | DrawFlags_RoundCornersTopRight,
  DrawFlags_RoundCornersBottom =
      DrawFlags_RoundCornersBottomLeft | DrawFlags_RoundCornersBottomRight,
  DrawFlags_RoundCornersLeft =
      DrawFlags_RoundCornersBottomLeft | DrawFlags_RoundCornersTopLeft,
  DrawFlags_RoundCornersRight =
      DrawFlags_RoundCornersBottomRight | DrawFlags_RoundCornersTopRight,
  DrawFlags_RoundCornersAll =
      DrawFlags_RoundCornersTopLeft | DrawFlags_RoundCornersTopRight |
      DrawFlags_RoundCornersBottomLeft | DrawFlags_RoundCornersBottomRight,
  DrawFlags_RoundCornersDefault_ =
      DrawFlags_RoundCornersAll, // Default to ALL corners if none of the
                                 // _RoundCornersXX flags are specified.
  DrawFlags_RoundCornersMask_ =
      DrawFlags_RoundCornersAll | DrawFlags_RoundCornersNone,
};

// Flags for DrawList instance. Those are set automatically by Gui::
// functions from IO settings, and generally not manipulated directly. It
// is however possible to temporarily alter flags between calls to DrawList::
// functions.
enum DrawListFlags_ {
  DrawListFlags_None = 0,
  DrawListFlags_AntiAliasedLines =
      1 << 0, // Enable anti-aliased lines/borders (*2 the number of triangles
              // for 1.0f wide line or lines thin enough to be drawn using
              // textures, otherwise *3 the number of triangles)
  DrawListFlags_AntiAliasedLinesUseTex =
      1 << 1, // Enable anti-aliased lines/borders using textures when possible.
              // Require backend to render with bilinear filtering (NOT
              // point/nearest filtering).
  DrawListFlags_AntiAliasedFill =
      1 << 2, // Enable anti-aliased edge around filled shapes (rounded
              // rectangles, circles).
  DrawListFlags_AllowVtxOffset =
      1 << 3, // Can emit 'VtxOffset > 0' to allow large meshes. Set when
              // 'BackendFlags_RendererHasVtxOffset' is enabled.
};

// Draw command list
// This is the low-level list of polygons that Gui:: functions are filling. At
// the end of the frame, all command lists are passed to your
// IO::RenderDrawListFn function for rendering. Each gui window
// contains its own DrawList. You can use Gui::GetWindowDrawList() to access
// the current window draw list and draw custom primitives. You can interleave
// normal Gui:: calls and adding primitives to the current draw list. In
// single viewport mode, top-left is == GetMainViewport()->Pos (generally 0,0),
// bottom-right is == GetMainViewport()->Pos+Size (generally io.DisplaySize).
// You are totally free to apply whatever transformation matrix to want to the
// data (depending on the use of the transformation you may want to apply it to
// ClipRect as well!) Important: Primitives are always added to the list and not
// culled (culling is done at higher-level by Gui:: functions), if you use
// this API a lot consider coarse culling your drawn objects.
struct DrawList {
  // This is what you have to render
  Vector<DrawCmd> CmdBuffer;  // Draw commands. Typically 1 command = 1 GPU
                              // draw call, unless the command is a callback.
  Vector<DrawIdx> IdxBuffer;  // Index buffer. Each command consume
                              // DrawCmd::ElemCount of those
  Vector<DrawVert> VtxBuffer; // Vertex buffer.
  DrawListFlags Flags;        // Flags, you may poke into these to adjust
                              // anti-aliasing settings per-primitive.

  // [Internal, used while building lists]
  unsigned int
      _VtxCurrentIdx; // [Internal] generally == VtxBuffer.Size unless we are
                      // past 64K vertices, in which case this gets reset to 0.
  DrawListSharedData *_Data; // Pointer to shared draw data (you can use
                             // Gui::GetDrawListSharedData() to get the one
                             // from current Gui context)
  const char *_OwnerName;    // Pointer to owner window's name for debugging
  DrawVert *_VtxWritePtr; // [Internal] point within VtxBuffer.Data after each
                          // add command (to avoid using the Vector<>
                          // operators too much)
  DrawIdx *_IdxWritePtr;  // [Internal] point within IdxBuffer.Data after each
                          // add command (to avoid using the Vector<>
                          // operators too much)
  Vector<Vec4> _ClipRectStack;       // [Internal]
  Vector<TextureID> _TextureIdStack; // [Internal]
  Vector<Vec2> _Path;                // [Internal] current path building
  DrawCmdHeader _CmdHeader; // [Internal] template of active commands. Fields
                            // should match those of CmdBuffer.back().
  DrawListSplitter
      _Splitter; // [Internal] for channels api (note: prefer using your own
                 // persistent instance of DrawListSplitter!)
  float _FringeScale; // [Internal] anti-alias fringe is scaled by this value,
                      // this helps to keep things sharp while zooming at vertex
                      // buffer content

  // If you want to create DrawList instances, pass them
  // Gui::GetDrawListSharedData() or create and use your own
  // DrawListSharedData (so you can use DrawList without Gui)
  DrawList(DrawListSharedData *shared_data) {
    memset(this, 0, sizeof(*this));
    _Data = shared_data;
  }

  ~DrawList() { _ClearFreeMemory(); }
  API void PushClipRect(
      const Vec2 &clip_rect_min, const Vec2 &clip_rect_max,
      bool intersect_with_current_clip_rect =
          false); // Render-level scissoring. This is passed down to your render
                  // function but not used for CPU-side coarse clipping. Prefer
                  // using higher-level Gui::PushClipRect() to affect logic
                  // (hit-testing and widget culling)
  API void PushClipRectFullScreen();
  API void PopClipRect();
  API void PushTextureID(TextureID texture_id);
  API void PopTextureID();
  inline Vec2 GetClipRectMin() const {
    const Vec4 &cr = _ClipRectStack.back();
    return Vec2(cr.x, cr.y);
  }
  inline Vec2 GetClipRectMax() const {
    const Vec4 &cr = _ClipRectStack.back();
    return Vec2(cr.z, cr.w);
  }

  // Primitives
  // - Filled shapes must always use clockwise winding order. The anti-aliasing
  // fringe depends on it. Counter-clockwise shapes will have "inward"
  // anti-aliasing.
  // - For rectangular primitives, "p_min" and "p_max" represent the upper-left
  // and lower-right corners.
  // - For circle primitives, use "num_segments == 0" to automatically calculate
  // tessellation (preferred).
  //   In older versions (until Gui 1.77) the AddCircle functions
  //   defaulted to num_segments == 12. In future versions we will use textures
  //   to provide cheaper and higher-quality circles. Use AddNgon() and
  //   AddNgonFilled() functions if you need to guarantee a specific number of
  //   sides.
  API void AddLine(const Vec2 &p1, const Vec2 &p2, U32 col,
                   float thickness = 1.0f);
  API void
  AddRect(const Vec2 &p_min, const Vec2 &p_max, U32 col, float rounding = 0.0f,
          DrawFlags flags = 0,
          float thickness =
              1.0f); // a: upper-left, b: lower-right (== upper-left + size)
  API void
  AddRectFilled(const Vec2 &p_min, const Vec2 &p_max, U32 col,
                float rounding = 0.0f,
                DrawFlags flags =
                    0); // a: upper-left, b: lower-right (== upper-left + size)
  API void AddRectFilledMultiColor(const Vec2 &p_min, const Vec2 &p_max,
                                   U32 col_upr_left, U32 col_upr_right,
                                   U32 col_bot_right, U32 col_bot_left);
  API void AddQuad(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                   const Vec2 &p4, U32 col, float thickness = 1.0f);
  API void AddQuadFilled(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                         const Vec2 &p4, U32 col);
  API void AddTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, U32 col,
                       float thickness = 1.0f);
  API void AddTriangleFilled(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                             U32 col);
  API void AddCircle(const Vec2 &center, float radius, U32 col,
                     int num_segments = 0, float thickness = 1.0f);
  API void AddCircleFilled(const Vec2 &center, float radius, U32 col,
                           int num_segments = 0);
  API void AddNgon(const Vec2 &center, float radius, U32 col, int num_segments,
                   float thickness = 1.0f);
  API void AddNgonFilled(const Vec2 &center, float radius, U32 col,
                         int num_segments);
  API void AddEllipse(const Vec2 &center, float radius_x, float radius_y,
                      U32 col, float rot = 0.0f, int num_segments = 0,
                      float thickness = 1.0f);
  API void AddEllipseFilled(const Vec2 &center, float radius_x, float radius_y,
                            U32 col, float rot = 0.0f, int num_segments = 0);
  API void AddText(const Vec2 &pos, U32 col, const char *text_begin,
                   const char *text_end = NULL);
  API void AddText(const Font *font, float font_size, const Vec2 &pos, U32 col,
                   const char *text_begin, const char *text_end = NULL,
                   float wrap_width = 0.0f,
                   const Vec4 *cpu_fine_clip_rect = NULL);
  API void AddPolyline(const Vec2 *points, int num_points, U32 col,
                       DrawFlags flags, float thickness);
  API void AddConvexPolyFilled(const Vec2 *points, int num_points, U32 col);
  API void
  AddBezierCubic(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4,
                 U32 col, float thickness,
                 int num_segments = 0); // Cubic Bezier (4 control points)
  API void AddBezierQuadratic(
      const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, U32 col, float thickness,
      int num_segments = 0); // Quadratic Bezier (3 control points)

  // Image primitives
  // - Read FAQ to understand what TextureID is.
  // - "p_min" and "p_max" represent the upper-left and lower-right corners of
  // the rectangle.
  // - "uv_min" and "uv_max" represent the normalized texture coordinates to use
  // for those corners. Using (0,0)->(1,1) texture coordinates will generally
  // display the entire texture.
  API void AddImage(TextureID user_texture_id, const Vec2 &p_min,
                    const Vec2 &p_max, const Vec2 &uv_min = Vec2(0, 0),
                    const Vec2 &uv_max = Vec2(1, 1), U32 col = COL32_WHITE);
  API void AddImageQuad(TextureID user_texture_id, const Vec2 &p1,
                        const Vec2 &p2, const Vec2 &p3, const Vec2 &p4,
                        const Vec2 &uv1 = Vec2(0, 0),
                        const Vec2 &uv2 = Vec2(1, 0),
                        const Vec2 &uv3 = Vec2(1, 1),
                        const Vec2 &uv4 = Vec2(0, 1), U32 col = COL32_WHITE);
  API void AddImageRounded(TextureID user_texture_id, const Vec2 &p_min,
                           const Vec2 &p_max, const Vec2 &uv_min,
                           const Vec2 &uv_max, U32 col, float rounding,
                           DrawFlags flags = 0);

  // Stateful path API, add points then finish with PathFillConvex() or
  // PathStroke()
  // - Filled shapes must always use clockwise winding order. The anti-aliasing
  // fringe depends on it. Counter-clockwise shapes will have "inward"
  // anti-aliasing.
  inline void PathClear() { _Path.Size = 0; }
  inline void PathLineTo(const Vec2 &pos) { _Path.push_back(pos); }
  inline void PathLineToMergeDuplicate(const Vec2 &pos) {
    if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0)
      _Path.push_back(pos);
  }
  inline void PathFillConvex(U32 col) {
    AddConvexPolyFilled(_Path.Data, _Path.Size, col);
    _Path.Size = 0;
  }
  inline void PathStroke(U32 col, DrawFlags flags = 0, float thickness = 1.0f) {
    AddPolyline(_Path.Data, _Path.Size, col, flags, thickness);
    _Path.Size = 0;
  }
  API void PathArcTo(const Vec2 &center, float radius, float a_min, float a_max,
                     int num_segments = 0);
  API void PathArcToFast(
      const Vec2 &center, float radius, int a_min_of_12,
      int a_max_of_12); // Use precomputed angles for a 12 steps circle
  API void PathEllipticalArcTo(const Vec2 &center, float radius_x,
                               float radius_y, float rot, float a_min,
                               float a_max, int num_segments = 0); // Ellipse
  API void PathBezierCubicCurveTo(
      const Vec2 &p2, const Vec2 &p3, const Vec2 &p4,
      int num_segments = 0); // Cubic Bezier (4 control points)
  API void PathBezierQuadraticCurveTo(
      const Vec2 &p2, const Vec2 &p3,
      int num_segments = 0); // Quadratic Bezier (3 control points)
  API void PathRect(const Vec2 &rect_min, const Vec2 &rect_max,
                    float rounding = 0.0f, DrawFlags flags = 0);

  // Advanced
  API void
  AddCallback(DrawCallback callback,
              void *callback_data); // Your rendering function must check for
                                    // 'UserCallback' in DrawCmd and call the
                                    // function instead of rendering triangles.
  API void AddDrawCmd(); // This is useful if you need to forcefully create a
                         // new draw call (to allow for dependent rendering /
                         // blending). Otherwise primitives are merged into the
                         // same draw-call as much as possible
  API DrawList *
  CloneOutput() const; // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer.

  // Advanced: Channels
  // - Use to split render into layers. By switching channels to can render
  // out-of-order (e.g. submit FG primitives before BG primitives)
  // - Use to minimize draw calls (e.g. if going back-and-forth between multiple
  // clipping rectangles, prefer to append into separate channels then merge at
  // the end)
  // - This API shouldn't have been in DrawList in the first place!
  //   Prefer using your own persistent instance of DrawListSplitter as you
  //   can stack them. Using the DrawList::ChannelsXXXX you cannot stack a
  //   split over another.
  inline void ChannelsSplit(int count) { _Splitter.Split(this, count); }
  inline void ChannelsMerge() { _Splitter.Merge(this); }
  inline void ChannelsSetCurrent(int n) {
    _Splitter.SetCurrentChannel(this, n);
  }

  // Advanced: Primitives allocations
  // - We render triangles (three vertices)
  // - All primitives needs to be reserved via PrimReserve() beforehand.
  API void PrimReserve(int idx_count, int vtx_count);
  API void PrimUnreserve(int idx_count, int vtx_count);
  API void
  PrimRect(const Vec2 &a, const Vec2 &b,
           U32 col); // Axis aligned rectangle (composed of two triangles)
  API void PrimRectUV(const Vec2 &a, const Vec2 &b, const Vec2 &uv_a,
                      const Vec2 &uv_b, U32 col);
  API void PrimQuadUV(const Vec2 &a, const Vec2 &b, const Vec2 &c,
                      const Vec2 &d, const Vec2 &uv_a, const Vec2 &uv_b,
                      const Vec2 &uv_c, const Vec2 &uv_d, U32 col);
  inline void PrimWriteVtx(const Vec2 &pos, const Vec2 &uv, U32 col) {
    _VtxWritePtr->pos = pos;
    _VtxWritePtr->uv = uv;
    _VtxWritePtr->col = col;
    _VtxWritePtr++;
    _VtxCurrentIdx++;
  }
  inline void PrimWriteIdx(DrawIdx idx) {
    *_IdxWritePtr = idx;
    _IdxWritePtr++;
  }
  inline void PrimVtx(const Vec2 &pos, const Vec2 &uv, U32 col) {
    PrimWriteIdx((DrawIdx)_VtxCurrentIdx);
    PrimWriteVtx(pos, uv, col);
  } // Write vertex with unique index

  // Obsolete names
  // inline  void  AddBezierCurve(const Vec2& p1, const Vec2& p2, const
  // Vec2& p3, const Vec2& p4, U32 col, float thickness, int num_segments
  // = 0) { AddBezierCubic(p1, p2, p3, p4, col, thickness, num_segments); } //
  // OBSOLETED in 1.80 (Jan 2021) inline  void  PathBezierCurveTo(const Vec2&
  // p2, const Vec2& p3, const Vec2& p4, int num_segments = 0) {
  // PathBezierCubicCurveTo(p2, p3, p4, num_segments); } // OBSOLETED in 1.80
  // (Jan 2021)

  // [Internal helpers]
  API void _ResetForNewFrame();
  API void _ClearFreeMemory();
  API void _PopUnusedDrawCmd();
  API void _TryMergeDrawCmds();
  API void _OnChangedClipRect();
  API void _OnChangedTextureID();
  API void _OnChangedVtxOffset();
  API int _CalcCircleAutoSegmentCount(float radius) const;
  API void _PathArcToFastEx(const Vec2 &center, float radius, int a_min_sample,
                            int a_max_sample, int a_step);
  API void _PathArcToN(const Vec2 &center, float radius, float a_min,
                       float a_max, int num_segments);
};

// All draw data to render a Gui frame
// (NB: the style and the naming convention here is a little inconsistent, we
// currently preserve them for backward compatibility purpose, as this is one of
// the oldest structure exposed by the library! Basically, DrawList ==
// CmdList)
struct DrawData {
  bool Valid;        // Only valid after Render() is called and before the next
                     // NewFrame() is called.
  int CmdListsCount; // Number of DrawList* to render
  int TotalIdxCount; // For convenience, sum of all DrawList's IdxBuffer.Size
  int TotalVtxCount; // For convenience, sum of all DrawList's VtxBuffer.Size
  Vector<DrawList *>
      CmdLists;     // Array of DrawList* to render. The DrawLists are owned by
                    // Context and only pointed to from here.
  Vec2 DisplayPos;  // Top-left position of the viewport to render (== top-left
                    // of the orthogonal projection matrix to use) (==
                    // GetMainViewport()->Pos for the main viewport, == (0.0)
                    // in most single-viewport applications)
  Vec2 DisplaySize; // Size of the viewport to render (==
                    // GetMainViewport()->Size for the main viewport, ==
                    // io.DisplaySize in most single-viewport applications)
  Vec2 FramebufferScale; // Amount of pixels for each unit of DisplaySize. Based
                         // on io.DisplayFramebufferScale. Generally (1,1) on
                         // normal display, (2,2) on OSX with Retina display.
  Viewport *OwnerViewport; // Viewport carrying the DrawData instance, might
                           // be of use to the renderer (generally not).

  // Functions
  DrawData() { Clear(); }
  API void Clear();
  API void AddDrawList(DrawList *draw_list); // Helper to add an external draw
                                             // list into an existing DrawData.
  API void
  DeIndexAllBuffers(); // Helper to convert all buffers from indexed to
                       // non-indexed, in case you cannot render indexed. Note:
                       // this is slow and most likely a waste of resources.
                       // Always prefer indexed rendering!
  API void ScaleClipRects(
      const Vec2 &fb_scale); // Helper to scale the ClipRect field of each
                             // DrawCmd. Use if your final output buffer is
                             // at a different scale than Gui expects,
                             // or if there is a difference between your
                             // window resolution and framebuffer resolution.
};

//-----------------------------------------------------------------------------
// [SECTION] Font API (FontConfig, FontGlyph, FontAtlasFlags, FontAtlas,
// FontGlyphRangesBuilder, Font)
//-----------------------------------------------------------------------------

struct FontConfig {
  void *FontData;            //          // TTF/OTF data
  int FontDataSize;          //          // TTF/OTF data size
  bool FontDataOwnedByAtlas; // true     // TTF/OTF data ownership taken by the
                             // container FontAtlas (will delete memory
                             // itself).
  int FontNo;                // 0        // Index of font within TTF/OTF file
  float SizePixels; //          // Size in pixels for rasterizer (more or less
                    //          maps to the resulting font height).
  int OversampleH;  // 2        // Rasterize at higher quality for sub-pixel
                    // positioning. Note the difference between 2 and 3 is
                    // minimal. You can reduce this to 1 for large glyphs save
                    // memory.
  int OversampleV;  // 1        // Rasterize at higher quality for sub-pixel
                    // positioning. This is not really useful as we don't use
                    // sub-pixel positions on the Y axis.
  bool PixelSnapH;  // false    // Align every glyph to pixel boundary. Useful
                    // e.g. if you are merging a non-pixel aligned font with the
                    // default font. If enabled, you can set OversampleH/V to 1.
  Vec2 GlyphExtraSpacing; // 0, 0     // Extra spacing (in pixels) between
                          // glyphs. Only X axis is supported for now.
  Vec2 GlyphOffset;       // 0, 0     // Offset all glyphs from this font input.
  const Wchar
      *GlyphRanges; // NULL     // THE ARRAY DATA NEEDS TO PERSIST AS LONG AS
                    // THE FONT IS ALIVE. Pointer to a user-provided list of
                    // Unicode range (2 value per range, values are inclusive,
                    // zero-terminated list).
  float GlyphMinAdvanceX; // 0        // Minimum AdvanceX for glyphs, set Min to
                          // align font icons, set both Min/Max to enforce
                          // mono-space font
  float GlyphMaxAdvanceX; // FLT_MAX  // Maximum AdvanceX for glyphs
  bool MergeMode; // false    // Merge into previous Font, so you can combine
                  // multiple inputs font into one Font (e.g. ASCII font +
                  // icons + Japanese glyphs). You may want to use GlyphOffset.y
                  // when merge font of different heights.
  unsigned int FontBuilderFlags; // 0        // Settings for custom font
                                 // builder. THIS IS BUILDER IMPLEMENTATION
                                 // DEPENDENT. Leave as zero if unsure.
  float
      RasterizerMultiply; // 1.0f     // Linearly brighten (>1.0f) or darken
                          // (<1.0f) font output. Brightening small fonts may be
                          // a good workaround to make them more readable. This
                          // is a silly thing we may remove in the future.
  float RasterizerDensity; // 1.0f     // DPI scale for rasterization, not
                           // altering other font metrics: make it easy to swap
                           // between e.g. a 100% and a 400% fonts for a zooming
                           // display. IMPORTANT: If you increase this it is
                           // expected that you increase font scale accordingly,
                           // otherwise quality may look lowered.
  Wchar EllipsisChar; // -1       // Explicitly specify unicode codepoint of
                      // ellipsis character. When fonts are being merged first
                      // specified ellipsis will be used.

  // [Internal]
  char Name[40]; // Name (strictly to ease debugging)
  Font *DstFont;

  API FontConfig();
};

// Hold rendering data for one glyph.
// (Note: some language parsers may fail to convert the 31+1 bitfield members,
// in this case maybe drop store a single u32 or we can rework this)
struct FontGlyph {
  unsigned int Colored : 1; // Flag to indicate glyph is colored and should
                            // generally ignore tinting (make it usable with no
                            // shift on little-endian as this is used in loops)
  unsigned int Visible : 1; // Flag to indicate glyph has no visible pixels
                            // (e.g. space). Allow early out when rendering.
  unsigned int Codepoint : 30; // 0x0000..0x10FFFF
  float AdvanceX;              // Distance to next character (= data from font +
                               // FontConfig::GlyphExtraSpacing.x baked in)
  float X0, Y0, X1, Y1;        // Glyph corners
  float U0, V0, U1, V1;        // Texture coordinates
};

// Helper to build glyph ranges from text/string data. Feed your application
// strings/characters to it then call BuildRanges(). This is essentially a
// tightly packed of vector of 64k booleans = 8KB storage.
struct FontGlyphRangesBuilder {
  Vector<U32>
      UsedChars; // Store 1-bit per Unicode code point (0=unused, 1=used)

  FontGlyphRangesBuilder() { Clear(); }
  inline void Clear() {
    int size_in_bytes = (UNICODE_CODEPOINT_MAX + 1) / 8;
    UsedChars.resize(size_in_bytes / (int)sizeof(U32));
    memset(UsedChars.Data, 0, (size_t)size_in_bytes);
  }
  inline bool GetBit(size_t n) const {
    int off = (int)(n >> 5);
    U32 mask = 1u << (n & 31);
    return (UsedChars[off] & mask) != 0;
  } // Get bit n in the array
  inline void SetBit(size_t n) {
    int off = (int)(n >> 5);
    U32 mask = 1u << (n & 31);
    UsedChars[off] |= mask;
  }                                           // Set bit n in the array
  inline void AddChar(Wchar c) { SetBit(c); } // Add character
  API void AddText(
      const char *text,
      const char *text_end =
          NULL); // Add string (each character of the UTF-8 string are added)
  API void AddRanges(
      const Wchar
          *ranges); // Add ranges, e.g.
                    // builder.AddRanges(FontAtlas::GetGlyphRangesDefault())
                    // to force add all of ASCII/Latin+Ext
  API void BuildRanges(Vector<Wchar> *out_ranges); // Output new ranges
};

// See FontAtlas::AddCustomRectXXX functions.
struct FontAtlasCustomRect {
  unsigned short Width, Height; // Input    // Desired rectangle dimension
  unsigned short X, Y;          // Output   // Packed position in Atlas
  unsigned int
      GlyphID; // Input    // For custom font glyphs only (ID < 0x110000)
  float
      GlyphAdvanceX; // Input    // For custom font glyphs only: glyph xadvance
  Vec2 GlyphOffset;  // Input    // For custom font glyphs only: glyph display
                     // offset
  Font *Font;        // Input    // For custom font glyphs only: target font
  FontAtlasCustomRect() {
    Width = Height = 0;
    X = Y = 0xFFFF;
    GlyphID = 0;
    GlyphAdvanceX = 0.0f;
    GlyphOffset = Vec2(0, 0);
    Font = NULL;
  }
  bool IsPacked() const { return X != 0xFFFF; }
};

// Flags for FontAtlas build
enum FontAtlasFlags_ {
  FontAtlasFlags_None = 0,
  FontAtlasFlags_NoPowerOfTwoHeight =
      1 << 0, // Don't round the height to next power of two
  FontAtlasFlags_NoMouseCursors =
      1 << 1, // Don't build software mouse cursors into the atlas (save a
              // little texture memory)
  FontAtlasFlags_NoBakedLines =
      1 << 2, // Don't build thick line textures into the atlas (save a little
              // texture memory, allow support for point/nearest filtering). The
              // AntiAliasedLinesUseTex features uses them, otherwise they will
              // be rendered using polygons (more expensive for CPU/GPU).
};

// Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas
// will build a single texture holding:
//  - One or more fonts.
//  - Custom graphics data needed to render the shapes needed by Gui.
//  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags
//  |= FontAtlasFlags_NoMouseCursors' in the font atlas).
// It is the user-code responsibility to setup/build the atlas, then upload the
// pixel data into a texture accessible by your graphics api.
//  - Optionally, call any of the AddFont*** functions. If you don't call any,
//  the default font embedded in the code will be loaded for you.
//  - Call GetTexDataAsAlpha8() or GetTexDataAsRGBA32() to build and retrieve
//  pixels data.
//  - Upload the pixels data into a texture within your graphics system (see
//  xxxx.cpp examples)
//  - Call SetTexID(my_tex_id); and pass the pointer/identifier to your texture
//  in a format natural to your graphics API.
//    This value will be passed back to you during rendering to identify the
//    texture. Read FAQ entry about TextureID for more details.
// Common pitfalls:
// - If you pass a 'glyph_ranges' array to AddFont*** functions, you need to
// make sure that your array persist up until the
//   atlas is build (when calling GetTexData*** or Build()). We only copy the
//   pointer, not the data.
// - Important: By default, AddFontFromMemoryTTF() takes ownership of the data.
// Even though we are not writing to it, we will free the pointer on
// destruction.
//   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your
//   data and it won't be freed,
// - Even though many functions are suffixed with "TTF", OTF data is supported
// just as well.
// - This is an old API and it is currently awkward for those and various other
// reasons! We will address them in the future!
struct FontAtlas {
  API FontAtlas();
  API ~FontAtlas();
  API Font *AddFont(const FontConfig *font_cfg);
  API Font *AddFontDefault(const FontConfig *font_cfg = NULL);
  API Font *AddFontFromFileTTF(const char *filename, float size_pixels,
                               const FontConfig *font_cfg = NULL,
                               const Wchar *glyph_ranges = NULL);
  API Font *AddFontFromMemoryTTF(
      void *font_data, int font_data_size, float size_pixels,
      const FontConfig *font_cfg = NULL,
      const Wchar *glyph_ranges =
          NULL); // Note: Transfer ownership of 'ttf_data' to FontAtlas! Will
                 // be deleted after destruction of the atlas. Set
                 // font_cfg->FontDataOwnedByAtlas=false to keep ownership of
                 // your data and it won't be freed.
  API Font *AddFontFromMemoryCompressedTTF(
      const void *compressed_font_data, int compressed_font_data_size,
      float size_pixels, const FontConfig *font_cfg = NULL,
      const Wchar *glyph_ranges =
          NULL); // 'compressed_font_data' still owned by caller. Compress with
                 // binary_to_compressed_c.cpp.
  API Font *AddFontFromMemoryCompressedBase85TTF(
      const char *compressed_font_data_base85, float size_pixels,
      const FontConfig *font_cfg = NULL,
      const Wchar *glyph_ranges =
          NULL); // 'compressed_font_data_base85' still owned by caller.
                 // Compress with binary_to_compressed_c.cpp with -base85
                 // parameter.
  API void
  ClearInputData(); // Clear input data (all FontConfig structures including
                    // sizes, TTF data, glyph ranges, etc.) = all the data used
                    // to build the texture and fonts.
  API void
  ClearTexData(); // Clear output texture data (CPU side). Saves RAM once the
                  // texture has been copied to graphics memory.
  API void
  ClearFonts();     // Clear output font data (glyphs storage, UV coordinates).
  API void Clear(); // Clear all input and output.

  // Build atlas, retrieve pixel data.
  // User is in charge of copying the pixels into graphics memory (e.g. create a
  // texture with your engine). Then store your texture handle with SetTexID().
  // The pitch is always = Width * BytesPerPixels (1 or 4)
  // Building in RGBA32 format is provided for convenience and compatibility,
  // but note that unless you manually manipulate or copy color data into the
  // texture (e.g. when using the AddCustomRect*** api), then the RGB pixels
  // emitted will always be white (~75% of memory/bandwidth waste.
  API bool Build(); // Build pixels data. This is called automatically for you
                    // by the GetTexData*** functions.
  API void
  GetTexDataAsAlpha8(unsigned char **out_pixels, int *out_width,
                     int *out_height,
                     int *out_bytes_per_pixel = NULL); // 1 byte per-pixel
  API void
  GetTexDataAsRGBA32(unsigned char **out_pixels, int *out_width,
                     int *out_height,
                     int *out_bytes_per_pixel = NULL); // 4 bytes-per-pixel
  bool IsBuilt() const {
    return Fonts.Size > 0 && TexReady;
  } // Bit ambiguous: used to detect when user didn't build texture but
    // effectively we should check TexID != 0 except that would be backend
    // dependent...
  void SetTexID(TextureID id) { TexID = id; }

  //-------------------------------------------
  // Glyph Ranges
  //-------------------------------------------

  // Helpers to retrieve list of common Unicode ranges (2 value per range,
  // values are inclusive, zero-terminated list) NB: Make sure that your string
  // are UTF-8 and NOT in your local code page.
  // NB: Consider using FontGlyphRangesBuilder to build glyph
  // ranges from textual data.
  API const Wchar *GetGlyphRangesDefault(); // Basic Latin, Extended Latin
  API const Wchar *GetGlyphRangesGreek();   // Default + Greek and Coptic
  API const Wchar *GetGlyphRangesKorean();  // Default + Korean characters
  API const Wchar *
  GetGlyphRangesJapanese(); // Default + Hiragana, Katakana, Half-Width,
                            // Selection of 2999 Ideographs
  API const Wchar *
  GetGlyphRangesChineseFull(); // Default + Half-Width + Japanese
                               // Hiragana/Katakana + full set of about 21000
                               // CJK Unified Ideographs
  API const Wchar *
  GetGlyphRangesChineseSimplifiedCommon(); // Default + Half-Width + Japanese
                                           // Hiragana/Katakana + set of 2500
                                           // CJK Unified Ideographs for common
                                           // simplified Chinese
  API const Wchar *
  GetGlyphRangesCyrillic(); // Default + about 400 Cyrillic characters
  API const Wchar *GetGlyphRangesThai(); // Default + Thai characters
  API const Wchar *
  GetGlyphRangesVietnamese(); // Default + Vietnamese characters

  //-------------------------------------------
  // [BETA] Custom Rectangles/Glyphs API
  //-------------------------------------------

  // You can request arbitrary rectangles to be packed into the atlas, for your
  // own purposes.
  // - After calling Build(), you can query the rectangle position and render
  // your pixels.
  // - If you render colored output, set 'atlas->TexPixelsUseColors = true' as
  // this may help some backends decide of prefered texture format.
  // - You can also request your rectangles to be mapped as font glyph (given a
  // font + Unicode point),
  //   so you can render e.g. custom colorful icons and use them as regular
  //   glyphs.
  // - Read docs/FONTS.md for more details about using colorful icons.
  // - Note: this API may be redesigned later in order to support multi-monitor
  // varying DPI settings.
  API int AddCustomRectRegular(int width, int height);
  API int AddCustomRectFontGlyph(Font *font, Wchar id, int width, int height,
                                 float advance_x,
                                 const Vec2 &offset = Vec2(0, 0));
  FontAtlasCustomRect *GetCustomRectByIndex(int index) {
    ASSERT(index >= 0);
    return &CustomRects[index];
  }

  // [Internal]
  API void CalcCustomRectUV(const FontAtlasCustomRect *rect, Vec2 *out_uv_min,
                            Vec2 *out_uv_max) const;
  API bool GetMouseCursorTexData(MouseCursor cursor, Vec2 *out_offset,
                                 Vec2 *out_size, Vec2 out_uv_border[2],
                                 Vec2 out_uv_fill[2]);

  //-------------------------------------------
  // Members
  //-------------------------------------------

  FontAtlasFlags Flags; // Build flags (see FontAtlasFlags_)
  TextureID TexID;      // User data to refer to the texture once it has been
                   // uploaded to user's graphic systems. It is passed back to
                   // you during rendering via the DrawCmd structure.
  int TexDesiredWidth; // Texture width desired by user before Build(). Must be
                       // a power-of-two. If have many glyphs your graphics API
                       // have texture size restrictions you may want to
                       // increase texture width to decrease height.
  int TexGlyphPadding; // Padding between glyphs within texture in pixels.
                       // Defaults to 1. If your rendering method doesn't rely
                       // on bilinear filtering you may set this to 0 (will also
                       // need to set AntiAliasedLinesUseTex = false).
  bool Locked; // Marked as Locked by Gui::NewFrame() so attempt to modify the
               // atlas will assert.
  void *UserData; // Store your own atlas related user-data (if e.g. you have
                  // multiple font atlas).

  // [Internal]
  // NB: Access texture data via GetTexData*() calls! Which will setup a default
  // font for you.
  bool TexReady; // Set when texture was built matching current font input
  bool TexPixelsUseColors; // Tell whether our texture data is known to use
                           // colors (rather than just alpha channel), in order
                           // to help backend select a format.
  unsigned char
      *TexPixelsAlpha8; // 1 component per pixel, each component is unsigned
                        // 8-bit. Total size = TexWidth * TexHeight
  unsigned int
      *TexPixelsRGBA32; // 4 component per pixel, each component is unsigned
                        // 8-bit. Total size = TexWidth * TexHeight * 4
  int TexWidth;         // Texture width calculated during Build().
  int TexHeight;        // Texture height calculated during Build().
  Vec2 TexUvScale;      // = (1.0f/TexWidth, 1.0f/TexHeight)
  Vec2 TexUvWhitePixel; // Texture coordinates to a white pixel
  Vector<Font *> Fonts; // Hold all the fonts returned by AddFont*. Fonts[0] is
                        // the default font upon calling Gui::NewFrame(), use
                        // Gui::PushFont()/PopFont() to change the current font.
  Vector<FontAtlasCustomRect>
      CustomRects; // Rectangles for packing custom texture data into the atlas.
  Vector<FontConfig> ConfigData; // Configuration data
  Vec4 TexUvLines[DRAWLIST_TEX_LINES_WIDTH_MAX +
                  1]; // UVs for baked anti-aliased lines

  // [Internal] Font builder
  const FontBuilderIO
      *FontBuilderIO; // Opaque interface to a font builder (default to
                      // truetype, can be changed to use FreeType by
                      // defining ENABLE_FREETYPE).
  unsigned int
      FontBuilderFlags; // Shared flags (for all fonts) for custom font builder.
                        // THIS IS BUILD IMPLEMENTATION DEPENDENT. Per-font
                        // override is also available in FontConfig.

  // [Internal] Packing data
  int PackIdMouseCursors; // Custom texture rectangle ID for white pixel and
                          // mouse cursors
  int PackIdLines; // Custom texture rectangle ID for baked anti-aliased lines

  // [Obsolete]
  // typedef FontAtlasCustomRect    CustomRect;         // OBSOLETED in 1.72+
  // typedef FontGlyphRangesBuilder GlyphRangesBuilder; // OBSOLETED in 1.67+
};

// Font runtime data and rendering
// FontAtlas automatically loads a default embedded font for you when you call
// GetTexDataAsAlpha8() or GetTexDataAsRGBA32().
struct Font {
  // Members: Hot ~20/24 bytes (for CalcTextSize)
  Vector<float>
      IndexAdvanceX; // 12-16 // out //            // Sparse. Glyphs->AdvanceX
                     // in a directly indexable way (cache-friendly for
                     // CalcTextSize functions which only this this info, and
                     // are often bottleneck in large UI).
  float FallbackAdvanceX; // 4     // out // = FallbackGlyph->AdvanceX
  float FontSize; // 4     // in  //            // Height of characters/line,
                  // set during loading (don't change after loading)

  // Members: Hot ~28/40 bytes (for CalcTextSize + render loop)
  Vector<Wchar> IndexLookup; // 12-16 // out //            // Sparse. Index
                             // glyphs by Unicode code-point.
  Vector<FontGlyph> Glyphs;  // 12-16 // out //            // All glyphs.
  const FontGlyph
      *FallbackGlyph; // 4-8   // out // = FindGlyph(FontFallbackChar)

  // Members: Cold ~32/40 bytes
  FontAtlas *ContainerAtlas; // 4-8   // out //            // What we has been
                             // loaded into
  const FontConfig *ConfigData; // 4-8   // in  //            // Pointer
                                // within ContainerAtlas->ConfigData
  short ConfigDataCount;   // 2     // in  // ~ 1        // Number of FontConfig
                           // involved in creating this font. Bigger than 1 when
                           // merging multiple font sources into one Font.
  Wchar FallbackChar;      // 2     // out // = FFFD/'?' // Character used if a
                           // glyph isn't found.
  Wchar EllipsisChar;      // 2     // out // = '...'/'.'// Character used for
                           // ellipsis rendering.
  short EllipsisCharCount; // 1     // out // 1 or 3
  float EllipsisWidth;     // 4     // out               // Width
  float EllipsisCharStep;  // 4     // out               // Step between
                           // characters when EllipsisCount > 0
  bool DirtyLookupTables;  // 1     // out //
  float Scale; // 4     // in  // = 1.f      // Base font scale, multiplied by
               // the per-window font scale which you can adjust with
               // SetWindowFontScale()
  float Ascent, Descent; // 4+4   // out //            // Ascent: distance from
                         // top to bottom of e.g. 'A' [0..FontSize]
  int MetricsTotalSurface; // 4     // out //            // Total surface in
                           // pixels to get an idea of the font
                           // rasterization/texture cost (not exact, we
                           // approximate the cost of padding between glyphs)
  U8 Used4kPagesMap[(UNICODE_CODEPOINT_MAX + 1) / 4096 /
                    8]; // 2 bytes if Wchar=Wchar16, 34 bytes if
                        // Wchar==Wchar32. Store 1-bit for each block of
                        // 4K codepoints that has one active glyph. This is
                        // mainly used to facilitate iterations across all
                        // used codepoints.

  // Methods
  API Font();
  API ~Font();
  API const FontGlyph *FindGlyph(Wchar c) const;
  API const FontGlyph *FindGlyphNoFallback(Wchar c) const;
  float GetCharAdvance(Wchar c) const {
    return ((int)c < IndexAdvanceX.Size) ? IndexAdvanceX[(int)c]
                                         : FallbackAdvanceX;
  }
  bool IsLoaded() const { return ContainerAtlas != NULL; }
  const char *GetDebugName() const {
    return ConfigData ? ConfigData->Name : "<unknown>";
  }

  // 'max_width' stops rendering after a certain width (could be turned into a
  // 2d size). FLT_MAX to disable. 'wrap_width' enable automatic word-wrapping
  // across multiple lines to fit into given width. 0.0f to disable.
  API Vec2 CalcTextSizeA(float size, float max_width, float wrap_width,
                         const char *text_begin, const char *text_end = NULL,
                         const char **remaining = NULL) const; // utf8
  API const char *CalcWordWrapPositionA(float scale, const char *text,
                                        const char *text_end,
                                        float wrap_width) const;
  API void RenderChar(DrawList *draw_list, float size, const Vec2 &pos, U32 col,
                      Wchar c) const;
  API void RenderText(DrawList *draw_list, float size, const Vec2 &pos, U32 col,
                      const Vec4 &clip_rect, const char *text_begin,
                      const char *text_end, float wrap_width = 0.0f,
                      bool cpu_fine_clip = false) const;

  // [Internal] Don't use!
  API void BuildLookupTable();
  API void ClearOutputData();
  API void GrowIndex(int new_size);
  API void AddGlyph(const FontConfig *src_cfg, Wchar c, float x0, float y0,
                    float x1, float y1, float u0, float v0, float u1, float v1,
                    float advance_x);
  API void AddRemapChar(
      Wchar dst, Wchar src,
      bool overwrite_dst =
          true); // Makes 'dst' character/glyph points to 'src' character/glyph.
                 // Currently needs to be called AFTER fonts have been built.
  API void SetGlyphVisible(Wchar c, bool visible);
  API bool IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

//-----------------------------------------------------------------------------
// [SECTION] Viewports
//-----------------------------------------------------------------------------

// Flags stored in Viewport::Flags, giving indications to the platform
// backends.
enum ViewportFlags_ {
  ViewportFlags_None = 0,
  ViewportFlags_IsPlatformWindow = 1 << 0, // Represent a Platform Window
  ViewportFlags_IsPlatformMonitor =
      1 << 1, // Represent a Platform Monitor (unused yet)
  ViewportFlags_OwnedByApp =
      1 << 2, // Platform Window: Was created/managed by the user application?
              // (rather than our backend)
  ViewportFlags_NoDecoration =
      1 << 3, // Platform Window: Disable platform decorations: title bar,
              // borders, etc. (generally set all windows, but if
              // ConfigFlags_ViewportsDecoration is set we only set this on
              // popups/tooltips)
  ViewportFlags_NoTaskBarIcon =
      1 << 4, // Platform Window: Disable platform task bar icon (generally set
              // on popups/tooltips, or all windows if
              // ConfigFlags_ViewportsNoTaskBarIcon is set)
  ViewportFlags_NoFocusOnAppearing =
      1 << 5, // Platform Window: Don't take focus when created.
  ViewportFlags_NoFocusOnClick =
      1 << 6, // Platform Window: Don't take focus when clicked on.
  ViewportFlags_NoInputs =
      1 << 7, // Platform Window: Make mouse pass through so we can drag this
              // window while peaking behind it.
  ViewportFlags_NoRendererClear =
      1 << 8, // Platform Window: Renderer doesn't need to clear the framebuffer
              // ahead (because we will fill it entirely).
  ViewportFlags_NoAutoMerge =
      1 << 9, // Platform Window: Avoid merging this window into another host
              // window. This can only be set via WindowClass viewport
              // flags override (because we need to now ahead if we are going to
              // create a viewport in the first place!).
  ViewportFlags_TopMost =
      1 << 10, // Platform Window: Display on top (for tooltips only).
  ViewportFlags_CanHostOtherWindows =
      1 << 11, // Viewport can host multiple imgui windows (secondary viewports
               // are associated to a single window). // FIXME: In practice
               // there's still probably code making the assumption that this is
               // always and only on the MainViewport. Will fix once we add
               // support for "no main viewport".

  // Output status flags (from Platform)
  ViewportFlags_IsMinimized =
      1 << 12, // Platform Window: Window is minimized, can skip render. When
               // minimized we tend to avoid using the viewport pos/size for
               // clipping window or testing if they are contained in the
               // viewport.
  ViewportFlags_IsFocused =
      1 << 13, // Platform Window: Window is focused (last call to
               // Platform_GetWindowFocus() returned true)
};

// - Currently represents the Platform Window created by the application which
// is hosting our Gui windows.
// - With multi-viewport enabled, we extend this concept to have multiple active
// viewports.
// - In the future we will extend this concept further to also represent
// Platform Monitor and support a "no main platform window" operation mode.
// - About Main Area vs Work Area:
//   - Main Area = entire viewport.
//   - Work Area = entire viewport minus sections used by main menu bars (for
//   platform windows), or by task bar (for platform monitor).
//   - Windows are generally trying to stay within the Work Area of their host
//   viewport.
struct Viewport {
  ID ID;               // Unique identifier for the viewport
  ViewportFlags Flags; // See ViewportFlags_
  Vec2 Pos;       // Main Area: Position of the viewport (Gui coordinates are
                  // the same as OS desktop/native coordinates)
  Vec2 Size;      // Main Area: Size of the viewport.
  Vec2 WorkPos;   // Work Area: Position of the viewport minus task bars, menus
                  // bars, status bars (>= Pos)
  Vec2 WorkSize;  // Work Area: Size of the viewport minus task bars, menu
                  // bars, status bars (<= Size)
  float DpiScale; // 1.0f = 96 DPI = No extra scale.
  ::ID ParentViewportId; // (Advanced) 0: no parent. Instruct the platform
                         // backend to setup a parent/child relationship
                         // between platform windows.
  DrawData *DrawData;    // The DrawData corresponding to this viewport. Valid
                      // after Render() and until the next call to NewFrame().

  // Platform/Backend Dependent Data
  // Our design separate the Renderer and Platform backends to facilitate
  // combining default backends with each others. When our create your own
  // backend for a custom engine, it is possible that both Renderer and Platform
  // will be handled by the same system and you may not need to use all the
  // UserData/Handle fields. The library never uses those fields, they are
  // merely storage to facilitate backend implementation.
  void
      *RendererUserData; // void* to hold custom data structure for the renderer
                         // (e.g. swap chain, framebuffers etc.). generally set
                         // by your Renderer_CreateWindow function.
  void *
      PlatformUserData; // void* to hold custom data structure for the OS /
                        // platform (e.g. windowing info, render context).
                        // generally set by your Platform_CreateWindow function.
  void *PlatformHandle; // void* for FindViewportByPlatformHandle(). (e.g.
                        // suggested to use natural platform handle such as
                        // HWND, GLFWWindow*, SDL_Window*)
  void *PlatformHandleRaw; // void* to hold lower-level, platform-native window
                           // handle (under Win32 this is expected to be a HWND,
                           // unused for other platforms), when using an
                           // abstraction layer like GLFW or SDL (where
                           // PlatformHandle would be a SDL_Window*)
  bool PlatformWindowCreated; // Platform window has been created
                              // (Platform_CreateWindow() has been called). This
                              // is false during the first frame where a
                              // viewport is being created.
  bool
      PlatformRequestMove; // Platform window requested move (e.g. window was
                           // moved by the OS / host window manager,
                           // authoritative position will be OS window position)
  bool PlatformRequestResize; // Platform window requested resize (e.g. window
                              // was resized by the OS / host window manager,
                              // authoritative size will be OS window size)
  bool PlatformRequestClose;  // Platform window requested closure (e.g. window
                              // was moved by the OS / host window manager, e.g.
                              // pressing ALT-F4)

  Viewport() { memset(this, 0, sizeof(*this)); }
  ~Viewport() { ASSERT(PlatformUserData == NULL && RendererUserData == NULL); }

  // Helpers
  Vec2 GetCenter() const {
    return Vec2(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f);
  }
  Vec2 GetWorkCenter() const {
    return Vec2(WorkPos.x + WorkSize.x * 0.5f, WorkPos.y + WorkSize.y * 0.5f);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Platform Dependent Interfaces (for e.g. multi-viewport support)
//-----------------------------------------------------------------------------
// [BETA] (Optional) This is completely optional, for advanced users!
// If you are new to Gui and trying to integrate it into your engine, you
// can probably ignore this for now.
//
// This feature allows you to seamlessly drag Gui windows outside of your
// application viewport. This is achieved by creating new Platform/OS windows on
// the fly, and rendering into them. Gui manages the viewport structures,
// and the backend create and maintain one Platform/OS window for each of those
// viewports.
//
// About the coordinates system:
// - When multi-viewports are enabled, all Gui coordinates become
// absolute coordinates (same as OS coordinates!)
// - So e.g. Gui::SetNextWindowPos(Vec2(0,0)) will position a window
// relative to your primary monitor!
// - If you want to position windows relative to your main application viewport,
// use Gui::GetMainViewport()->Pos as a base position.
//
// Steps to use multi-viewports in your application, when using a default
// backend from the examples/ folder:
// - Application:  Enable feature with 'io.ConfigFlags |=
// ConfigFlags_ViewportsEnable'.
// - Backend:      The backend initialization will setup all necessary
// PlatformIO's functions and update monitors info every frame.
// - Application:  In your main loop, call Gui::UpdatePlatformWindows(),
// Gui::RenderPlatformWindowsDefault() after EndFrame() or Render().
// - Application:  Fix absolute coordinates used in Gui::SetWindowPos() or
// Gui::SetNextWindowPos() calls.
//
// Steps to use multi-viewports in your application, when using a custom
// backend:
// - Important:    THIS IS NOT EASY TO DO and comes with many subtleties not
// described here!
//                 It's also an experimental feature, so some of the
//                 requirements may evolve. Consider using default backends if
//                 you can. Either way, carefully follow and refer to examples/
//                 backends for details.
// - Application:  Enable feature with 'io.ConfigFlags |=
// ConfigFlags_ViewportsEnable'.
// - Backend:      Hook PlatformIO's Platform_* and Renderer_* callbacks
// (see below).
//                 Set 'io.BackendFlags |=
//                 BackendFlags_PlatformHasViewports' and 'io.BackendFlags
//                 |= BackendFlags_PlatformHasViewports'. Update
//                 PlatformIO's Monitors list every frame. Update MousePos
//                 every frame, in absolute coordinates.
// - Application:  In your main loop, call Gui::UpdatePlatformWindows(),
// Gui::RenderPlatformWindowsDefault() after EndFrame() or Render().
//                 You may skip calling RenderPlatformWindowsDefault() if its
//                 API is not convenient for your needs. Read comments below.
// - Application:  Fix absolute coordinates used in Gui::SetWindowPos() or
// Gui::SetNextWindowPos() calls.
//
// About Gui::RenderPlatformWindowsDefault():
// - This function is a mostly a _helper_ for the common-most cases, and to
// facilitate using default backends.
// - You can check its simple source code to understand what it does.
//   It basically iterates secondary viewports and call 4 functions that are
//   setup in PlatformIO, if available:
//     Platform_RenderWindow(), Renderer_RenderWindow(), Platform_SwapBuffers(),
//     Renderer_SwapBuffers()
//   Those functions pointers exists only for the benefit of
//   RenderPlatformWindowsDefault().
// - If you have very specific rendering needs (e.g. flipping multiple
// swap-chain simultaneously, unusual sync/threading issues, etc.),
//   you may be tempted to ignore RenderPlatformWindowsDefault() and write
//   customized code to perform your renderingg. You may decide to setup the
//   platform_io's *RenderWindow and *SwapBuffers pointers and call your
//   functions through those pointers, or you may decide to never setup those
//   pointers and call your code directly. They are a convenience, not an
//   obligatory interface.
//-----------------------------------------------------------------------------

// (Optional) Access via Gui::GetPlatformIO()
struct PlatformIO {
  //------------------------------------------------------------------
  // Input - Backend interface/functions + Monitor List
  //------------------------------------------------------------------

  // (Optional) Platform functions (e.g. Win32, GLFW, SDL2)
  // For reference, the second column shows which function are generally calling
  // the Platform Functions:
  //   N = Gui::NewFrame()                        ~ beginning of the dear
  //   imgui frame: read info from platform/OS windows (latest size/position) F
  //   = Gui::Begin(), Gui::EndFrame()        ~ during the gui frame
  //   U = Gui::UpdatePlatformWindows()           ~ after the gui
  //   frame: create and update all platform/OS windows R =
  //   Gui::RenderPlatformWindowsDefault()    ~ render D =
  //   Gui::DestroyPlatformWindows()          ~ shutdown
  // The general idea is that NewFrame() we will read the current Platform/OS
  // state, and UpdatePlatformWindows() will write to it.
  //
  // The functions are designed so we can mix and match 2 xxxx files,
  // one for the Platform (~window/input handling), one for Renderer. Custom
  // engine backends will often provide both Platform and Renderer interfaces
  // and so may not need to use all functions. Platform functions are typically
  // called before their Renderer counterpart, apart from Destroy which are
  // called the other way.

  // Platform function ---------------------------------------------------
  // Called by -----
  void (*Platform_CreateWindow)(
      Viewport *vp); // . . U . .  // Create a new platform window for the
                     // given viewport
  void (*Platform_DestroyWindow)(Viewport *vp); // N . U . D  //
  void (*Platform_ShowWindow)(
      Viewport *vp); // . . U . .  // Newly created windows are initially
                     // hidden so SetWindowPos/Size/Title can be called on
                     // them before showing the window
  void (*Platform_SetWindowPos)(
      Viewport *vp,
      Vec2 pos); // . . U . .  // Set platform window position (given the
                 // upper-left corner of client area)
  Vec2 (*Platform_GetWindowPos)(Viewport *vp); // N . . . .  //
  void (*Platform_SetWindowSize)(
      Viewport *vp,
      Vec2 size); // . . U . .  // Set platform window client area size
                  // (ignoring OS decorations such as OS title bar etc.)
  Vec2 (*Platform_GetWindowSize)(
      Viewport *vp); // N . . . .  // Get platform window client area size
  void (*Platform_SetWindowFocus)(
      Viewport *vp); // N . . . .  // Move window to front and set input focus
  bool (*Platform_GetWindowFocus)(Viewport *vp); // . . U . .  //
  bool (*Platform_GetWindowMinimized)(
      Viewport *vp); // N . . . .  // Get platform window minimized state. When
                     // minimized, we generally won't attempt to get/set size
                     // and contents will be culled more easily
  void (*Platform_SetWindowTitle)(
      Viewport *vp, const char *str); // . . U . .  // Set platform window
                                      // title (given an UTF-8 string)
  void (*Platform_SetWindowAlpha)(
      Viewport *vp,
      float alpha); // . . U . .  // (Optional) Setup global transparency (not
                    // per-pixel transparency)
  void (*Platform_UpdateWindow)(
      Viewport
          *vp); // . . U . .  // (Optional) Called by UpdatePlatformWindows().
                // Optional hook to allow the platform backend from doing
                // general book-keeping every frame.
  void (*Platform_RenderWindow)(
      Viewport *vp,
      void *
          render_arg); // . . . R .  // (Optional) Main rendering (platform
                       // side! This is often unused, or just setting a
                       // "current" context for OpenGL bindings). 'render_arg'
                       // is the value passed to RenderPlatformWindowsDefault().
  void (*Platform_SwapBuffers)(
      Viewport *vp,
      void *
          render_arg); // . . . R .  // (Optional) Call Present/SwapBuffers
                       // (platform side! This is often unused!). 'render_arg'
                       // is the value passed to RenderPlatformWindowsDefault().
  float (*Platform_GetWindowDpiScale)(
      Viewport *vp); // N . . . .  // (Optional) [BETA] FIXME-DPI: DPI handling:
                     // Return DPI scale for this viewport. 1.0f = 96 DPI.
  void (*Platform_OnChangedViewport)(
      Viewport *vp); // . F . . .  // (Optional) [BETA] FIXME-DPI: DPI
                     // handling: Called during Begin() every time the
                     // viewport we are outputting into changes, so backend
                     // has a chance to swap fonts to adjust style.
  int (*Platform_CreateVkSurface)(
      Viewport *vp, U64 vk_inst, const void *vk_allocators,
      U64 *out_vk_surface); // (Optional) For a Vulkan Renderer to call into
                            // Platform code (since the surface creation needs
                            // to tie them both).

  // (Optional) Renderer functions (e.g. DirectX, OpenGL, Vulkan)
  void (*Renderer_CreateWindow)(
      Viewport *vp); // . . U . .  // Create swap chain, frame buffers etc.
                     // (called after Platform_CreateWindow)
  void (*Renderer_DestroyWindow)(
      Viewport *vp); // N . U . D  // Destroy swap chain, frame buffers
                     // etc. (called before Platform_DestroyWindow)
  void (*Renderer_SetWindowSize)(
      Viewport *vp,
      Vec2 size); // . . U . .  // Resize swap chain, frame buffers etc.
                  // (called after Platform_SetWindowSize)
  void (*Renderer_RenderWindow)(
      Viewport *vp,
      void *render_arg); // . . . R .  // (Optional) Clear framebuffer, setup
                         // render target, then render the viewport->DrawData.
                         // 'render_arg' is the value passed to
                         // RenderPlatformWindowsDefault().
  void (*Renderer_SwapBuffers)(
      Viewport *vp,
      void *render_arg); // . . . R .  // (Optional) Call Present/SwapBuffers.
                         // 'render_arg' is the value passed to
                         // RenderPlatformWindowsDefault().

  // (Optional) Monitor list
  // - Updated by: app/backend. Update every frame to dynamically support
  // changing monitor or DPI configuration.
  // - Used by: gui to query DPI info, clamp popups/tooltips within same
  // monitor and not have them straddle monitors.
  Vector<PlatformMonitor> Monitors;

  //------------------------------------------------------------------
  // Output - List of viewports to render into platform windows
  //------------------------------------------------------------------

  // Viewports list (the list is updated by calling Gui::EndFrame or
  // Gui::Render) (in the future we will attempt to organize this feature to
  // remove the need for a "main viewport")
  Vector<Viewport *>
      Viewports; // Main viewports, followed by all secondary viewports.
  PlatformIO() { memset(this, 0, sizeof(*this)); } // Zero clear
};

// (Optional) This is required when enabling multi-viewport. Represent the
// bounds of each connected monitor/display and their DPI. We use this
// information for multiple DPI support + clamping the position of popups and
// tooltips so they don't straddle multiple monitors.
struct PlatformMonitor {
  Vec2 MainPos, MainSize; // Coordinates of the area displayed on this monitor
                          // (Min = upper left, Max = bottom right)
  Vec2 WorkPos, WorkSize; // Coordinates without task bars / side bars / menu
                          // bars. Used to avoid positioning popups/tooltips
                          // inside this region. If you don't have this info,
                          // please copy the value for MainPos/MainSize.
  float DpiScale;         // 1.0f = 96 DPI
  void *PlatformHandle; // Backend dependant data (e.g. HMONITOR, GLFWmonitor*,
                        // SDL Display Index, NSScreen*)
  PlatformMonitor() {
    MainPos = MainSize = WorkPos = WorkSize = Vec2(0, 0);
    DpiScale = 1.0f;
    PlatformHandle = NULL;
  }
};

// (Optional) Support for IME (Input Method Editor) via the
// io.SetPlatformImeDataFn() function.
struct PlatformImeData {
  bool WantVisible;      // A widget wants the IME to be visible
  Vec2 InputPos;         // Position of the input cursor
  float InputLineHeight; // Line height

  PlatformImeData() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Obsolete functions and types
// (Will be removed! Read 'API BREAKING CHANGES' section in gui.cpp for
// details) Please keep your copy of gui up to date! Occasionally set
// '#define DISABLE_OBSOLETE_FUNCTIONS' in config.hpp to stay ahead.
//-----------------------------------------------------------------------------

namespace Gui {
#ifndef DISABLE_OBSOLETE_KEYIO
API Key GetKeyIndex(Key key); // map Key_* values into legacy
                              // native key index. == io.KeyMap[key]
#else
static inline Key GetKeyIndex(Key key) {
  ASSERT(key >= Key_NamedKey_BEGIN && key < Key_NamedKey_END &&
         "Key and native_index was merged together and native_index is "
         "disabled by DISABLE_OBSOLETE_KEYIO. Please switch to Key.");
  return key;
}
#endif
} // namespace Gui

#ifndef DISABLE_OBSOLETE_FUNCTIONS
namespace Gui {
// OBSOLETED in 1.90.0 (from September 2023)
static inline bool BeginChildFrame(ID id, const Vec2 &size,
                                   WindowFlags window_flags = 0) {
  return BeginChild(id, size, ChildFlags_FrameStyle, window_flags);
}
static inline void EndChildFrame() { EndChild(); }
// static inline bool BeginChild(const char* str_id, const Vec2& size_arg,
// bool border, WindowFlags window_flags){ return BeginChild(str_id,
// size_arg, border ? ChildFlags_Border : ChildFlags_None,
// window_flags); } // Unnecessary as true == ChildFlags_Border static
// inline bool BeginChild(ID id, const Vec2& size_arg, bool border,
// WindowFlags window_flags)        { return BeginChild(id, size_arg,
// border ? ChildFlags_Border : ChildFlags_None, window_flags);     }
// // Unnecessary as true == ChildFlags_Border
static inline void ShowStackToolWindow(bool *p_open = NULL) {
  ShowIDStackToolWindow(p_open);
}
API bool ListBox(const char *label, int *current_item,
                 bool (*old_callback)(void *user_data, int idx,
                                      const char **out_text),
                 void *user_data, int items_count, int height_in_items = -1);
API bool
Combo(const char *label, int *current_item,
      bool (*old_callback)(void *user_data, int idx, const char **out_text),
      void *user_data, int items_count, int popup_max_height_in_items = -1);
// OBSOLETED in 1.89.7 (from June 2023)
API void SetItemAllowOverlap(); // Use SetNextItemAllowOverlap() before item.
// OBSOLETED in 1.89.4 (from March 2023)
static inline void PushAllowKeyboardFocus(bool tab_stop) {
  PushTabStop(tab_stop);
}
static inline void PopAllowKeyboardFocus() { PopTabStop(); }
// OBSOLETED in 1.89 (from August 2022)
API bool ImageButton(
    TextureID user_texture_id, const Vec2 &size, const Vec2 &uv0 = Vec2(0, 0),
    const Vec2 &uv1 = Vec2(1, 1), int frame_padding = -1,
    const Vec4 &bg_col = Vec4(0, 0, 0, 0),
    const Vec4 &tint_col = Vec4(1, 1, 1,
                                1)); // Use new ImageButton() signature
                                     // (explicit item id, regular FramePadding)
// OBSOLETED in 1.88 (from May 2022)
static inline void CaptureKeyboardFromApp(bool want_capture_keyboard = true) {
  SetNextFrameWantCaptureKeyboard(want_capture_keyboard);
} // Renamed as name was misleading + removed default value.
static inline void CaptureMouseFromApp(bool want_capture_mouse = true) {
  SetNextFrameWantCaptureMouse(want_capture_mouse);
} // Renamed as name was misleading + removed default value.

// Some of the older obsolete names along with their replacement (commented out
// so they are not reported in IDE)
//-- OBSOLETED in 1.86 (from November 2021)
// API void      CalcListClipping(int items_count, float items_height, int*
// out_items_display_start, int* out_items_display_end); // Code removed,
// see 1.90 for last version of the code. Calculate range of visible items for
// large list of evenly sized items. Prefer using ListClipper.
//-- OBSOLETED in 1.85 (from August 2021)
// static inline float GetWindowContentRegionWidth() { return
// GetWindowContentRegionMax().x - GetWindowContentRegionMin().x; }
//-- OBSOLETED in 1.81 (from February 2021)
// static inline bool  ListBoxHeader(const char* label, const Vec2& size =
// Vec2(0, 0))         { return BeginListBox(label, size); } static inline
// bool  ListBoxHeader(const char* label, int items_count, int height_in_items =
// -1) { float height = GetTextLineHeightWithSpacing() * ((height_in_items < 0 ?
// Min(items_count, 7) : height_in_items) + 0.25f) + GetStyle().FramePadding.y
// * 2.0f; return BeginListBox(label, Vec2(0.0f, height)); } // Helper to
// calculate size from items_count and height_in_items static inline void
// ListBoxFooter()                                                             {
// EndListBox(); }
//-- OBSOLETED in 1.79 (from August 2020)
// static inline void  OpenPopupContextItem(const char* str_id = NULL,
// MouseButton mb = 1)    { OpenPopupOnItemClick(str_id, mb); } // Bool
// return value removed. Use IsWindowAppearing() in BeginPopup() instead.
// Renamed in 1.77, renamed back in 1.79. Sorry!
//-- OBSOLETED in 1.78 (from June 2020): Old drag/sliders functions that took a
//'float power > 1.0f' argument instead of SliderFlags_Logarithmic. API bool
// DragScalar(const
// char* label, DataType data_type, void* p_data, float v_speed, const
// void* p_min, const void* p_max, const char* format, float power = 1.0f) //
// OBSOLETED in 1.78 (from June 2020) API bool      DragScalarN(const char*
// label, DataType data_type, void* p_data, int components, float v_speed,
// const void* p_min, const void* p_max, const char* format, float power
// = 1.0f);                                          // OBSOLETED in 1.78 (from
// June 2020) API bool      SliderScalar(const char* label, DataType
// data_type, void* p_data, const void* p_min, const void* p_max, const char*
// format, float power = 1.0f); // OBSOLETED in 1.78 (from June 2020) API bool
// SliderScalarN(const char* label, DataType data_type, void* p_data, int
// components, const void* p_min, const void* p_max, const char* format, float
// power = 1.0f);                                                       //
// OBSOLETED in 1.78 (from June 2020) static inline bool  DragFloat(const char*
// label, float* v, float v_speed, float v_min, float v_max, const char* format,
// float power = 1.0f)    { return DragScalar(label, DataType_Float, v,
// v_speed, &v_min, &v_max, format, power); }     // OBSOLETED in 1.78 (from
// June 2020) static inline bool  DragFloat2(const char* label, float v[2],
// float v_speed, float v_min, float v_max, const char* format, float power
// = 1.0f) { return DragScalarN(label, DataType_Float, v, 2, v_speed,
// &v_min, &v_max, format, power); } // OBSOLETED in 1.78 (from June 2020)
// static inline bool  DragFloat3(const char* label, float v[3], float v_speed,
// float v_min, float v_max, const char* format, float power = 1.0f) { return
// DragScalarN(label, DataType_Float, v, 3, v_speed, &v_min, &v_max,
// format, power); } // OBSOLETED in 1.78 (from June 2020) static inline bool
// DragFloat4(const char* label, float v[4], float v_speed, float v_min, float
// v_max, const char* format, float power = 1.0f) { return DragScalarN(label,
// DataType_Float, v, 4, v_speed, &v_min, &v_max, format, power); } //
// OBSOLETED in 1.78 (from June 2020) static inline bool  SliderFloat(const
// char* label, float* v, float v_min, float v_max, const char* format, float
// power = 1.0f)                 { return SliderScalar(label,
// DataType_Float, v, &v_min, &v_max, format, power); }            //
// OBSOLETED in 1.78 (from June 2020) static inline bool  SliderFloat2(const
// char* label, float v[2], float v_min, float v_max, const char* format, float
// power = 1.0f)              { return SliderScalarN(label, DataType_Float,
// v, 2, &v_min, &v_max, format, power); }        // OBSOLETED in 1.78 (from
// June 2020) static inline bool  SliderFloat3(const char* label, float v[3],
// float v_min, float v_max, const char* format, float power = 1.0f) { return
// SliderScalarN(label, DataType_Float, v, 3, &v_min, &v_max, format,
// power); }        // OBSOLETED in 1.78 (from June 2020) static inline bool
// SliderFloat4(const char* label, float v[4], float v_min, float v_max, const
// char* format, float power = 1.0f)              { return SliderScalarN(label,
// DataType_Float, v, 4, &v_min, &v_max, format, power); }        //
// OBSOLETED in 1.78 (from June 2020)
//-- OBSOLETED in 1.77 and before
// static inline bool  BeginPopupContextWindow(const char* str_id,
// MouseButton mb, bool over_items) { return
// BeginPopupContextWindow(str_id, mb | (over_items ? 0 :
// PopupFlags_NoOpenOverItems)); } // OBSOLETED in 1.77 (from June 2020)
// static inline void  TreeAdvanceToLabelPos()               {
// SetCursorPosX(GetCursorPosX() + GetTreeNodeToLabelSpacing()); }   //
// OBSOLETED in 1.72 (from July 2019) static inline void
// SetNextTreeNodeOpen(bool open, Cond cond = 0) { SetNextItemOpen(open,
// cond); }                       // OBSOLETED in 1.71 (from June 2019) static
// inline float GetContentRegionAvailWidth()          { return
// GetContentRegionAvail().x; }                               // OBSOLETED
// in 1.70 (from May 2019) static inline DrawList* GetOverlayDrawList() {
// return GetForegroundDrawList(); }                                 //
// OBSOLETED in 1.69 (from Mar 2019) static inline void  SetScrollHere(float
// ratio = 0.5f)     { SetScrollHereY(ratio); } // OBSOLETED in 1.66 (from Nov
// 2018) static inline bool  IsItemDeactivatedAfterChange()        { return
// IsItemDeactivatedAfterEdit(); }                            // OBSOLETED
// in 1.63 (from Aug 2018)
//-- OBSOLETED in 1.60 and before
// static inline bool  IsAnyWindowFocused()                  { return
// IsWindowFocused(FocusedFlags_AnyWindow); }            // OBSOLETED
// in 1.60 (from Apr 2018) static inline bool  IsAnyWindowHovered() { return
// IsWindowHovered(HoveredFlags_AnyWindow); }            // OBSOLETED
// in 1.60 (between Dec 2017 and Apr 2018) static inline void  ShowTestWindow()
// { return ShowDemoWindow(); }                                        //
// OBSOLETED in 1.53 (between Oct 2017 and Dec 2017) static inline bool
// IsRootWindowFocused()                 { return
// IsWindowFocused(FocusedFlags_RootWindow); }           // OBSOLETED
// in 1.53 (between Oct 2017 and Dec 2017) static inline bool
// IsRootWindowOrAnyChildFocused()       { return
// IsWindowFocused(FocusedFlags_RootAndChildWindows); }  // OBSOLETED
// in 1.53 (between Oct 2017 and Dec 2017) static inline void
// SetNextWindowContentWidth(float w)    { SetNextWindowContentSize(Vec2(w,
// 0.0f)); }                      // OBSOLETED in 1.53 (between Oct 2017 and Dec
// 2017) static inline float GetItemsLineHeightWithSpacing()       { return
// GetFrameHeightWithSpacing(); }                             // OBSOLETED
// in 1.53 (between Oct 2017 and Dec 2017) API bool      Begin(char* name, bool*
// p_open, Vec2 size_first_use, float bg_alpha = -1.0f, WindowFlags
// flags=0); // OBSOLETED in 1.52 (between Aug 2017 and Oct 2017): Equivalent of
// using SetNextWindowSize(size, Cond_FirstUseEver) and
// SetNextWindowBgAlpha(). static inline bool  IsRootWindowOrAnyChildHovered()
// { return IsWindowHovered(HoveredFlags_RootAndChildWindows); }  //
// OBSOLETED in 1.52 (between Aug 2017 and Oct 2017) static inline void
// AlignFirstTextHeightToWidgets()       { AlignTextToFramePadding(); } //
// OBSOLETED in 1.52 (between Aug 2017 and Oct 2017) static inline void
// SetNextWindowPosCenter(Cond c=0) {
// SetNextWindowPos(GetMainViewport()->GetCenter(), c, Vec2(0.5f,0.5f)); } //
// OBSOLETED in 1.52 (between Aug 2017 and Oct 2017) static inline bool
// IsItemHoveredRect()                   { return
// IsItemHovered(HoveredFlags_RectOnly); }               // OBSOLETED
// in 1.51 (between Jun 2017 and Aug 2017) static inline bool
// IsPosHoveringAnyWindow(const Vec2&) { ASSERT(0); return false; } //
// OBSOLETED in 1.51 (between Jun 2017 and Aug 2017): This was misleading and
// partly broken. You probably want to use the io.WantCaptureMouse flag instead.
// static inline bool  IsMouseHoveringAnyWindow()            { return
// IsWindowHovered(HoveredFlags_AnyWindow); }            // OBSOLETED
// in 1.51 (between Jun 2017 and Aug 2017) static inline bool
// IsMouseHoveringWindow()               { return
// IsWindowHovered(HoveredFlags_AllowWhenBlockedByPopup |
// HoveredFlags_AllowWhenBlockedByActiveItem); }       // OBSOLETED in 1.51
// (between Jun 2017 and Aug 2017)
//-- OBSOLETED in 1.50 and before
// static inline bool  CollapsingHeader(char* label, const char* str_id, bool
// framed = true, bool default_open = false) { return CollapsingHeader(label,
// (default_open ? (1 << 5) : 0)); } // OBSOLETED in 1.49 static inline
// Font*GetWindowFont()                      { return GetFont(); } //
// OBSOLETED in 1.48 static inline float GetWindowFontSize()                   {
// return GetFontSize(); }                                           //
// OBSOLETED in 1.48 static inline void  SetScrollPosHere()                    {
// SetScrollHere(); }                                                //
// OBSOLETED in 1.42
} // namespace Gui

//-- OBSOLETED in 1.82 (from Mars 2021): flags for AddRect(), AddRectFilled(),
// AddImageRounded(), PathRect() typedef DrawFlags DrawCornerFlags; enum
// DrawCornerFlags_
//{
//    DrawCornerFlags_None      = DrawFlags_RoundCornersNone,         // Was
//    == 0 prior to 1.82, this is now == DrawFlags_RoundCornersNone which is
//    != 0 and not implicit DrawCornerFlags_TopLeft   =
//    DrawFlags_RoundCornersTopLeft,      // Was == 0x01 (1 << 0) prior
//    to 1.82. Order matches DrawFlags_NoRoundCorner* flag (we exploit this
//    internally). DrawCornerFlags_TopRight  =
//    DrawFlags_RoundCornersTopRight,     // Was == 0x02 (1 << 1) prior
//    to 1.82. DrawCornerFlags_BotLeft   = DrawFlags_RoundCornersBottomLeft,
//    // Was == 0x04 (1 << 2) prior to 1.82. DrawCornerFlags_BotRight  =
//    DrawFlags_RoundCornersBottomRight,  // Was == 0x08 (1 << 3) prior
//    to 1.82. DrawCornerFlags_All       = DrawFlags_RoundCornersAll, // Was
//    == 0x0F prior to 1.82 DrawCornerFlags_Top       =
//    DrawCornerFlags_TopLeft | DrawCornerFlags_TopRight,
//    DrawCornerFlags_Bot       = DrawCornerFlags_BotLeft |
//    DrawCornerFlags_BotRight, DrawCornerFlags_Left      =
//    DrawCornerFlags_TopLeft | DrawCornerFlags_BotLeft,
//    DrawCornerFlags_Right     = DrawCornerFlags_TopRight |
//    DrawCornerFlags_BotRight,
//};

// RENAMED and MERGED both Key_ModXXX and ModFlags_XXX into
// Mod_XXX (from September 2022) RENAMED KeyModFlags -> ModFlags
// in 1.88 (from April 2022). Exceptionally commented out ahead of obscolescence
// schedule to reduce confusion and because they were not meant to be used in
// the first place.
typedef KeyChord ModFlags; // == int. We generally use KeyChord to mean "a
                           // Key or-ed with any number of Mod_XXX
                           // value", but you may store only mods in there.
enum ModFlags_ {
  ModFlags_None = 0,
  ModFlags_Ctrl = Mod_Ctrl,
  ModFlags_Shift = Mod_Shift,
  ModFlags_Alt = Mod_Alt,
  ModFlags_Super = Mod_Super
};
// typedef KeyChord KeyModFlags; // == int
// enum KeyModFlags_ { KeyModFlags_None = 0, KeyModFlags_Ctrl =
// Mod_Ctrl, KeyModFlags_Shift = Mod_Shift, KeyModFlags_Alt
// = Mod_Alt, KeyModFlags_Super = Mod_Super };

#define OFFSETOF(_TYPE, _MEMBER)                                               \
  offsetof(_TYPE,                                                              \
           _MEMBER) // OBSOLETED IN 1.90 (now using C++11 standard version)

#endif // #ifndef DISABLE_OBSOLETE_FUNCTIONS

// RENAMED DISABLE_METRICS_WINDOW > DISABLE_DEBUG_TOOLS in 1.88 (from June 2022)
#if defined(DISABLE_METRICS_WINDOW) && !defined(DISABLE_OBSOLETE_FUNCTIONS) && \
    !defined(DISABLE_DEBUG_TOOLS)
#define DISABLE_DEBUG_TOOLS
#endif
#if defined(DISABLE_METRICS_WINDOW) && defined(DISABLE_OBSOLETE_FUNCTIONS)
#error DISABLE_METRICS_WINDOW was renamed to DISABLE_DEBUG_TOOLS, please use new name.
#endif

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Include user.h at the end of gui.hpp
// May be convenient for some users to only explicitly include vanilla gui.hpp
// and have extra stuff included.
#ifdef INCLUDE_USER_H
#ifdef USER_H_FILENAME
#include USER_H_FILENAME
#else
#include "user.h"
#endif
#endif

#endif // #ifndef DISABLE
