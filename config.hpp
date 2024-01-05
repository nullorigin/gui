//-----------------------------------------------------------------------------
// GUI COMPILE-TIME OPTIONS
// Runtime options (clipboard callbacks, enabling various features, etc.) can
// generally be set via the IO structure. You can use
// Gui::SetAllocatorFunctions() before calling Gui::CreateContext() to
// rewire memory allocation functions.
//-----------------------------------------------------------------------------
// A) You may edit config.hpp (and not overwrite it when updating Gui, or
// maintain a patch/rebased branch with your modifications to it) B) or '#define
// USER_CONFIG "my_config.h"' in your project and then add directives in
// your own file without touching this template.
//-----------------------------------------------------------------------------
// You need to make sure that configuration settings are defined consistently
// _everywhere_ Gui is used, which include the imgui*.cpp files but also
// _any_ of your code that uses Gui. This is because some compile-time
// options have an affect on data structures. Defining those options in
// config.hpp will ensure every compilation unit gets to see the same data
// structure layouts. Call CHECKVERSION() from your .cpp file to verify that the
// data structures your files are using are matching the ones gui.cpp is
// using.
//-----------------------------------------------------------------------------

#pragma once

//---- Define assertion handler. Defaults to calling assert().
// If your macro uses multiple statements, make sure is enclosed in a 'do { .. }
// while (0)' block so it can be used as a single statement.
// #define ASSERT(_EXPR)  MyAssert(_EXPR)
// #define ASSERT(_EXPR)  ((void)(_EXPR))     // Disable asserts

//---- Define attributes of all API symbols declarations, e.g. for DLL under
// Windows
// Using Gui via a shared library is not recommended, because of function
// call overhead and because we don't guarantee backward nor forward ABI
// compatibility. DLL users: heaps and globals are not shared across DLL
// boundaries! You will need to call SetCurrentContext() +
// SetAllocatorFunctions() for each static/DLL boundary you are calling from.
// Read "Context and Memory Allocators" section of gui.cpp for more details.
// #define API __declspec( dllexport )
// #define API __declspec( dllimport )

//---- Don't define obsolete functions/enums/behaviors. Consider enabling from
// time to time after updating to clean your code of obsolete function/names.
// #define DISABLE_OBSOLETE_FUNCTIONS
// #define DISABLE_OBSOLETE_KEYIO                      // 1.87+ disable legacy
// io.KeyMap[]+io.KeysDown[] in favor io.AddKeyEvent(). This is automatically
// done by DISABLE_OBSOLETE_FUNCTIONS.

//---- Disable all of Gui or don't implement standard windows/tools.
// It is very strongly recommended to NOT disable the demo windows and debug
// tool during development. They are extremely useful in day to day work. Please
// read comments in demo.cpp.
// #define DISABLE                                     // Disable everything:
// all headers and source files will be empty. #define DISABLE_DEMO_WINDOWS //
// Disable demo windows: ShowDemoWindow()/ShowStyleEditor() will be empty.
// #define DISABLE_DEBUG_TOOLS                         // Disable
// metrics/debugger and other debug tools: ShowMetricsWindow(),
// ShowDebugLogWindow() and ShowIDStackToolWindow() will be empty.

//---- Don't implement some functions to reduce linkage requirements.
// #define DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS   // [Win32] Don't
// implement default clipboard handler. Won't use and link with
// OpenClipboard/GetClipboardData/CloseClipboard etc. (user32.lib/.a,
// kernel32.lib/.a) #define ENABLE_WIN32_DEFAULT_IME_FUNCTIONS          //
// [Win32] [Default with Visual Studio] Implement default IME handler (require
// imm32.lib/.a, auto-link for Visual Studio, -limm32 on command-line for MinGW)
// #define DISABLE_WIN32_DEFAULT_IME_FUNCTIONS         // [Win32] [Default with
// non-Visual Studio compilers] Don't implement default IME handler (won't
// require imm32.lib/.a) #define DISABLE_WIN32_FUNCTIONS                     //
// [Win32] Won't use and link with any Win32 function (clipboard, IME). #define
// ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS      // [OSX] Implement default OSX
// clipboard handler (need to link with '-framework ApplicationServices', this
// is why this is not the default). #define DISABLE_DEFAULT_FORMAT_FUNCTIONS //
// Don't implement FormatString/FormatStringV so you can implement them
// yourself (e.g. if you don't want to link with vsnprintf) #define
// DISABLE_DEFAULT_MATH_FUNCTIONS              // Don't implement
// Fabs/Sqrt/Pow/Fmod/Cos/Sin/Acos/Atan2 so you can implement
// them yourself. #define DISABLE_FILE_FUNCTIONS                      // Don't
// implement FileOpen/FileClose/FileRead/FileWrite and FileHandle at
// all (replace them with dummies) #define DISABLE_DEFAULT_FILE_FUNCTIONS //
// Don't implement FileOpen/FileClose/FileRead/FileWrite and
// FileHandle so you can implement them yourself if you don't want to link
// with fopen/fclose/fread/fwrite. This will also disable the LogToTTY()
// function. #define DISABLE_DEFAULT_ALLOCATORS                  // Don't
// implement default allocators calling malloc()/free() to avoid linking with
// them. You will need to call Gui::SetAllocatorFunctions(). #define
// DISABLE_SSE                                 // Disable use of SSE intrinsics
// even if available

//---- Include user.h at the end of gui.hpp as a convenience
// May be convenient for some users to only explicitly include vanilla gui.hpp
// and have extra stuff included.
// #define INCLUDE_USER_H
// #define USER_H_FILENAME         "my_folder/my_user.h"

//---- Pack colors to BGRA8 instead of RGBA8 (to avoid converting from one to
// another) #define USE_BGRA_PACKED_COLOR

//---- Use 32-bit for Wchar (default is 16-bit) to support Unicode planes
// 1-16. (e.g. point beyond 0xFFFF like emoticons, dingbats, symbols, shapes,
// ancient languages, etc...) #define USE_WCHAR32

//---- Avoid multiple STB libraries implementations, or redefine path/filenames
// to prioritize another version
// By default the embedded implementations are declared static and not available
// outside of Gui sources files.
// #define TRUETYPE_FILENAME   "my_folder/truetype.h"
// #define RECT_PACK_FILENAME  "my_folder/rect_pack.h"
// #define SPRINTF_FILENAME    "my_folder/sprintf.h"    // only used if
// USE_SPRINTF is defined. #define DISABLE_TRUETYPE_IMPLEMENTATION
// #define DISABLE_RECT_PACK_IMPLEMENTATION
// #define DISABLE_SPRINTF_IMPLEMENTATION                   // only disabled
// if USE_SPRINTF is defined.

//---- Use sprintf.h for a faster implementation of vsnprintf instead of the
// one from libc (unless DISABLE_DEFAULT_FORMAT_FUNCTIONS is defined)
// Compatibility checks of arguments and formats done by clang and GCC will be
// disabled in order to support the extra formats provided by sprintf.h.
// #define USE_SPRINTF

//---- Use FreeType to build and rasterize the font atlas (instead of
// truetype which is embedded by default in Gui)
// Requires FreeType headers to be available in the include path. Requires
// program to be compiled with 'misc/freetype/freetype.cpp' (in this
// repository) + the FreeType library (not provided). On Windows you may use
// vcpkg with 'vcpkg install freetype --triplet=x64-windows' + 'vcpkg integrate
// install'.
// #define ENABLE_FREETYPE

//---- Use FreeType+lunasvg library to render OpenType SVG fonts (SVGinOT)
// Requires lunasvg headers to be available in the include path + program to be
// linked with the lunasvg library (not provided). Only works in combination
// with ENABLE_FREETYPE. (implementation is based on Freetype's rsvg-port.c
// which is licensed under CeCILL-C Free Software License Agreement)
// #define ENABLE_FREETYPE_LUNASVG

//---- Use truetype to build and rasterize the font atlas (default)
// The only purpose of this define is if you want force compilation of the
// truetype backend ALONG with the FreeType backend.
// #define ENABLE_TRUETYPE

//---- Define constructor and implicit cast operators to convert back<>forth
// between your math types and Vec2/Vec4.
// This will be inlined as part of Vec2 and Vec4 class declarations.
/*
#define VEC2_CLASS_EXTRA \
        constexpr Vec2(const MyVec2& f) : x(f.x), y(f.y) {} \ operator
MyVec2() const { return MyVec2(x,y); }

#define VEC4_CLASS_EXTRA \
        constexpr Vec4(const MyVec4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {} \
        operator MyVec4() const { return MyVec4(x,y,z,w); }
*/
//---- ...Or use Gui's own very basic math operators.
// #define DEFINE_MATH_OPERATORS

//---- Use 32-bit vertex indices (default is 16-bit) is one way to allow large
// meshes with more than 64K vertices.
// Your renderer backend will need to support it (most example renderer backends
// support both 16/32-bit indices). Another way to allow large meshes while
// keeping 16-bit indices is to handle DrawCmd::VtxOffset in your renderer.
// Read about BackendFlags_RendererHasVtxOffset for details.
// #define DrawIdx unsigned int

//---- Override DrawCallback signature (will need to modify renderer backends
// accordingly) struct DrawList; struct DrawCmd; typedef void
// (*MyDrawCallback)(const DrawList* draw_list, const DrawCmd* cmd, void*
// my_renderer_user_data); #define DrawCallback MyDrawCallback

//---- Debug Tools: Macro to break in Debugger (we provide a default
// implementation of this in the codebase)
// (use 'Metrics->Tools->Item Picker' to pick widgets with the mouse and break
// into them for easy debugging.)
// #define DEBUG_BREAK  ASSERT(0)
// #define DEBUG_BREAK  __debugbreak()

//---- Debug Tools: Enable slower asserts
// #define DEBUG_PARANOID

//---- Tip: You can add extra functions within the Gui:: namespace from
// anywhere (e.g. your own sources/header files)
/*
namespace Gui
{
    void MyFunction(const char* name, MyMatrix44* mtx);
}
*/
