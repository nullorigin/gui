// [SECTION] Header mess
// [SECTION] Forward declarations
// [SECTION] Context pointer
// [SECTION] STB libraries includes
// [SECTION] Macros
// [SECTION] Generic helpers
// [SECTION] DrawList support
// [SECTION] Widgets support: flags, enums, data structures
// [SECTION] Inputs support
// [SECTION] Clipper support
// [SECTION] Navigation support
// [SECTION] Typing-select support
// [SECTION] Columns support
// [SECTION] Multi-select support
// [SECTION] Docking support
// [SECTION] Viewport support
// [SECTION] Settings support
// [SECTION] Localization support
// [SECTION] Metrics, Debug tools
// [SECTION] Generic context hooks
// [SECTION] Context (main imgui context)
// [SECTION] WindowTempData, Window
// [SECTION] Tab bar, Tab item support
// [SECTION] Table support
// [SECTION] Gui internal API
// [SECTION] FontAtlas internal API
// [SECTION] Test Engine specific hooks (test_engine)

#pragma once
#ifndef DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#ifndef VERSION
#include "gui.hpp"
#endif

#include <limits.h> // INT_MIN, INT_MAX
#include <math.h>   // sqrtf, fabsf, fmodf, powf, floorf, ceilf, cosf, sinf
#include <stdio.h>  // FILE*, sscanf
#include <stdlib.h> // NULL, malloc, free, qsort, atoi, atof

// Enable SSE intrinsics if available
#if (defined __SSE__ || defined __x86_64__ || defined _M_X64 ||                \
     (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))) &&                            \
    !defined(DISABLE_SSE)
#define ENABLE_SSE
#include <immintrin.h>
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251) // class 'xxx' needs to have dll-interface to be
                                // used by clients of struct 'xxx' // when API
                                // is set to__declspec(dllexport)
#pragma warning(                                                               \
    disable : 26812) // The enum type 'xxx' is unscoped. Prefer 'enum class'
                     // over 'enum' (Enum.3). [MSVC Static Analyzer)
#pragma warning(                                                               \
    disable : 26495) // [Static Analyzer] Variable 'XXX' is uninitialized.
                     // Always initialize a member variable (type.6).
#if defined(_MSC_VER) && _MSC_VER >= 1922 // MSVC 2019 16.2 or later
#pragma warning(disable : 5054) // operator '|': deprecated between enumerations
                                // of different types
#endif
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored                                               \
    "-Wunknown-warning-option" // warning: unknown warning group 'xxx'
#endif
#pragma clang diagnostic ignored                                               \
    "-Wunknown-pragmas" // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored                                               \
    "-Wfloat-equal" // warning: comparing floating point with == or != is unsafe
                    // // storing and comparing against same constants ok, for
                    // Floor()
#pragma clang diagnostic ignored "-Wunused-function"    // for textedit.h
#pragma clang diagnostic ignored "-Wmissing-prototypes" // for textedit.h
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored                                               \
    "-Wimplicit-int-float-conversion" // warning: implicit conversion from 'xxx'
                                      // to 'float' may lose precision
#pragma clang diagnostic ignored                                               \
    "-Wmissing-noreturn" // warning: function 'xxx' could be declared with
                         // attribute 'noreturn'
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

// In 1.89.4, we moved the implementation of "courtesy maths operators" from
// internal.hpp in gui.hpp As they are frequently requested, we do not want
// to encourage to many people using internal.hpp
#if defined(DEFINE_MATH_OPERATORS) &&                                          \
    !defined(DEFINE_MATH_OPERATORS_IMPLEMENTED)
#error Please '#define DEFINE_MATH_OPERATORS' _BEFORE_ including gui.hpp!
#endif

// Legacy defines
#ifdef DISABLE_FORMAT_STRING_FUNCTIONS // Renamed in 1.74
#error Use DISABLE_DEFAULT_FORMAT_FUNCTIONS
#endif
#ifdef DISABLE_MATH_FUNCTIONS // Renamed in 1.74
#error Use DISABLE_DEFAULT_MATH_FUNCTIONS
#endif

// Enable truetype by default unless FreeType is enabled.
// You can compile with both by defining both ENABLE_FREETYPE and
// ENABLE_TRUETYPE together.
#ifndef ENABLE_FREETYPE
#define ENABLE_TRUETYPE
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------

struct BitVector;          // Store 1-bit per value
struct Rect;               // An axis-aligned rectangle (2 points)
struct DrawDataBuilder;    // Helper to build a DrawData instance
struct DrawListSharedData; // Data shared between all DrawList instances
struct ColorMod;     // Stacked color modifier, backup of modified data so we
                     // can restore it
struct Context;      // Main Gui context
struct ContextHook;  // Hook for extensions like TestEngine
struct DataVarInfo;  // Variable information (e.g. to avoid style variables
                     // from an enum)
struct DataTypeInfo; // Type information associated to a int enum
struct DockContext;  // Docking system context
struct DockRequest;  // Docking system dock/undock queued request
struct DockNode;     // Docking system node (hold a list of Windows OR two
                     // child dock nodes)
struct DockNodeSettings; // Storage for a dock node in .ini file (we
                         // preserve those even if the associated dock node
                         // isn't active during the session)
struct GroupData;        // Stacked storage data for BeginGroup()/EndGroup()
struct InputTextState;   // Internal state of the currently focused/edited
                         // text input box
struct InputTextDeactivateData; // Short term storage to backup text of a
                                // deactivating InputText() while another
                                // is stealing active id
struct LastItemData;            // Status storage for last submitted items
struct LocEntry;                // A localization entry.
struct MenuColumns;             // Simple column measurement, currently used for
                                // MenuItem() only
struct NavItemData;     // Result of a gamepad/keyboard directional navigation
                        // move query result
struct NavTreeNodeData; // Temporary storage for last TreeNode() being a
                        // Left arrow landing candidate.
struct MetricsConfig;   // Storage for ShowMetricsWindow() and DebugNodeXXX()
                        // functions
struct NextWindowData;  // Storage for SetNextWindow** functions
struct NextItemData;    // Storage for SetNextItem** functions
struct OldColumnData;   // Storage data for a single column for legacy
                        // Columns() api
struct OldColumns;      // Storage data for a columns set for legacy Columns()
                        // api
struct PopupData;       // Storage for current popup stack
struct SettingsHandler; // Storage for one type registered in the .ini file
struct StackSizes;      // Storage of stack sizes for debugging/asserting
struct StyleMod;        // Stacked style modifier, backup of modified data so we
                        // can restore it
struct TabBar;          // Storage for a tab bar
struct TabItem;         // Storage for a tab item (within a tab bar)
struct Table;           // Storage for a table
struct TableColumn;     // Storage for one column of a table
struct TableInstanceData; // Storage for one instance of a same table
struct TableTempData;     // Temporary storage for one table (one per table in
                          // the stack), shared between tables.
struct TableSettings;     // Storage for a table .ini settings
struct TableColumnsSettings; // Storage for a column .ini settings
struct TypingSelectState;    // Storage for GetTypingSelectRequest()
struct TypingSelectRequest;  // Storage for GetTypingSelectRequest() (aimed
                             // to be public)
struct Window;               // Storage for one window
struct WindowTempData; // Temporary storage for one window (that's the data
                       // which in theory we could ditch at the end of the
                       // frame, in practice we currently keep it for each
                       // window)
struct WindowSettings; // Storage for a window .ini settings (we keep one
                       // of those even if the actual window wasn't
                       // instanced during this session)

// Enumerations
// Use your programming IDE "Go to definition" facility on the names of the
// center columns to find the actual flags/enum lists.
enum LocKey : int; // -> enum LocKey              // Enum: a
                   // localization entry for translation.
                   // -> enum DataAuthority_      // Enum: for storing
                   // the source authority (dock node vs window) of a field
                   // -> enum LayoutType_         // Enum:
                   // Horizontal or vertical

// Flags
// -> enum ActivateFlags_      // Flags:
// for navigation/focus function (will be for
// ActivateItem() later)
// -> enum DebugLogFlags_      // Flags:
// for ShowDebugLogWindow(), g.DebugLogFlags
// -> enum FocusRequestFlags_  //
// Flags: for FocusWindow();
// -> enum InputFlags_         // Flags: for
// IsKeyPressed(), IsMouseClicked(), SetKeyOwner(),
// SetItemKeyOwner() etc.
// -> enum ItemFlags_          // Flags: for
// PushItemFlag(), g.LastItemData.InFlags
// -> enum ItemStatusFlags_    // Flags:
// for g.LastItemData.StatusFlags
// -> enum OldColumnFlags_     // Flags:
// for BeginColumns()
// -> enum NavHighlightFlags_  //
// Flags: for RenderNavHighlight()
// -> enum NavMoveFlags_       // Flags: for
// navigation requests
// -> enum NextItemDataFlags_  //
// Flags: for SetNextItemXXX() functions
// -> enum NextWindowDataFlags_//
// Flags: for SetNextWindowXXX() functions
// // -> enum ScrollFlags_        // Flags: for
// ScrollToItem() and navigation requests
// -> enum SeparatorFlags_     // Flags:
// for SeparatorEx()
typedef void (*ErrorLogCallback)(void *user_data, const char *fmt, ...);

//-----------------------------------------------------------------------------
// [SECTION] Context pointer
// See implementation of this variable in gui.cpp for comments and details.
//-----------------------------------------------------------------------------

#ifndef GGui
extern API Context *GGui; // Current implicit context pointer
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries includes
//-------------------------------------------------------------------------

namespace Gui {

#undef TEXTEDIT_STRING
#undef TEXTEDIT_CHARTYPE
#define TEXTEDIT_STRING InputTextState
#define TEXTEDIT_CHARTYPE Wchar
#define TEXTEDIT_GETWIDTH_NEWLINE (-1.0f)
#define TEXTEDIT_UNDOSTATECOUNT 99
#define TEXTEDIT_UNDOCHARCOUNT 999
#include "textedit.hpp"

} // namespace Gui

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

// Internal Drag and Drop payload types. String starting with '_' are reserved
// for Gui.
#define PAYLOAD_TYPE_WINDOW "_IMWINDOW" // Payload == Window*

// Debug Printing Into TTY
// (since VERSION_NUM >= 18729: DEBUG_LOG was reworked into DEBUG_PRINTF (and
// removed framecount from it). If you were using a #define DEBUG_LOG please
// rename)
#ifndef DEBUG_PRINTF
#ifndef DISABLE_DEFAULT_FORMAT_FUNCTIONS
#define DEBUG_PRINTF(_FMT, ...) printf(_FMT, __VA_ARGS__)
#else
#define DEBUG_PRINTF(_FMT, ...) ((void)0)
#endif
#endif

// Debug Logging for ShowDebugLogWindow(). This is designed for relatively rare
// events so please don't spam.
#ifndef DISABLE_DEBUG_TOOLS
#define DEBUG_LOG(...) Gui::DebugLog(__VA_ARGS__)
#else
#define DEBUG_LOG(...) ((void)0)
#endif
#define DEBUG_LOG_ACTIVEID(...)                                                \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventActiveId)                         \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_FOCUS(...)                                                   \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventFocus)                            \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_POPUP(...)                                                   \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventPopup)                            \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_NAV(...)                                                     \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventNav)                              \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_SELECTION(...)                                               \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventSelection)                        \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_CLIPPER(...)                                                 \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventClipper)                          \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_IO(...)                                                      \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventIO)                               \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_DOCKING(...)                                                 \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventDocking)                          \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)
#define DEBUG_LOG_VIEWPORT(...)                                                \
  do {                                                                         \
    if (g.DebugLogFlags & DebugLogFlags_EventViewport)                         \
      DEBUG_LOG(__VA_ARGS__);                                                  \
  } while (0)

// Static Asserts
#define STATIC_assert(_COND) static_assert(_COND, "")

// "Paranoid" Debug Asserts are meant to only be enabled during specific
// debugging/work, otherwise would slow down the code too much. We currently
// don't have many of those so the effect is currently negligible, but onward
// intent to add more aggressive ones in the code.
// #define DEBUG_PARANOID
#ifdef DEBUG_PARANOID
#define ASSERT_PARANOID(_EXPR) assert(_EXPR)
#else
#define ASSERT_PARANOID(_EXPR)
#endif

// Error handling
// Down the line in some frameworks/languages we would like to have a way to
// redirect those to the programmer and recover from more faults.
#ifndef ASSERT_USER_ERROR
#define ASSERT_USER_ERROR(_EXP, _MSG)                                          \
  assert((_EXP) && _MSG) // Recoverable User Error
#endif

// Misc Macros
#define PI 3.14159265358979323846f
#ifdef _WIN32
#define NEWLINE                                                                \
  "\r\n" // Play it nice with Windows users (Update: since 2018-05, Notepad
         // finally appears to support Unix-style carriage returns!)
#else
#define NEWLINE "\n"
#endif
#ifndef TABSIZE // Until we move this to runtime and/or add proper tab
                // support, at least allow users to compile-time override
#define TABSIZE (4)
#endif
#define MEMALIGN(_OFF, _ALIGN)                                                 \
  (((_OFF) + ((_ALIGN)-1)) &                                                   \
   ~((_ALIGN)-1)) // Memory align e.g. ALIGN(0,4)=0, ALIGN(1,4)=4,
                  // ALIGN(4,4)=4, ALIGN(5,4)=8
#define F32_TO_INT8_UNBOUND(_VAL)                                              \
  ((int)((_VAL) * 255.0f +                                                     \
         ((_VAL) >= 0 ? 0.5f : -0.5f))) // Unsaturated, for display purpose
#define F32_TO_INT8_SAT(_VAL)                                                  \
  ((int)(Saturate(_VAL) * 255.0f + 0.5f)) // Saturated, always output 0..255
#define TRUNC(_VAL)                                                            \
  ((float)(int)(_VAL)) // Trunc() is not inlined in MSVC debug builds
#define ROUND(_VAL) ((float)(int)((_VAL) + 0.5f)) //
#define STRINGIFY_HELPER(_X) #_X
#define STRINGIFY(_X)                                                          \
  STRINGIFY_HELPER(_X) // Preprocessor idiom to stringify e.g. an integer.
#ifndef DISABLE_OBSOLETE_FUNCTIONS
#define FLOOR TRUNC
#endif

// Enforce cdecl calling convention for functions called by the standard
// library, in case compilation settings changed the default to e.g.
// __vectorcall
#ifdef _MSC_VER
#define CDECL __cdecl
#else
#define CDECL
#endif

// Warnings
#if defined(_MSC_VER) && !defined(__clang__)
#define MSVC_WARNING_SUPPRESS(XXXX) __pragma(warning(suppress : XXXX))
#else
#define MSVC_WARNING_SUPPRESS(XXXX)
#endif

// Debug Tools
// Use 'Metrics/Debugger->Tools->Item Picker' to break into the call-stack of a
// specific item. This will call DEBUG_BREAK() which you may redefine
// yourself. See https://github.com/scottt/debugbreak for more reference.
#ifndef DEBUG_BREAK
#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
#define DEBUG_BREAK() __builtin_debugtrap()
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define DEBUG_BREAK() __asm__ volatile("int $0x03")
#elif defined(__GNUC__) && defined(__thumb__)
#define DEBUG_BREAK() __asm__ volatile(".inst 0xde01")
#elif defined(__GNUC__) && defined(__arm__) && !defined(__thumb__)
#define DEBUG_BREAK() __asm__ volatile(".inst 0xe7f001f0");
#else
#define DEBUG_BREAK()                                                          \
  assert(0) // It is expected that you define DEBUG_BREAK() into something
            // that will break nicely in a debugger!
#endif
#endif // #ifndef DEBUG_BREAK

// Format specifiers, printing 64-bit hasn't been decently standardized...
// In a real application you should be using PRId64 and PRIu64 from <inttypes.h>
// (non-windows) and on Windows define them yourself.
#if defined(_MSC_VER) && !defined(__clang__)
#define PRId64 "I64d"
#define PRIu64 "I64u"
#define PRIX64 "I64X"
#else
#define PRId64 "lld"
#define PRIu64 "llu"
#define PRIX64 "llX"
#endif

//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
// Note that the XXX helpers functions are lower-level than Gui functions.
// Gui functions or the Gui context are never called/used from other XXX
// functions.
//-----------------------------------------------------------------------------
// - Helpers: Hashing
// - Helpers: Sorting
// - Helpers: Bit manipulation
// - Helpers: String
// - Helpers: Formatting
// - Helpers: UTF-8 <> wchar conversions
// - Helpers: Vec2/Vec4 operators
// - Helpers: Maths
// - Helpers: Geometry
// - Helper: Vec1
// - Helper: Vec2ih
// - Helper: Rect
// - Helper: BitArray
// - Helper: BitVector
// - Helper: Span<>, SpanAllocator<>
// - Helper: Pool<>
// - Helper: ChunkStream<>
// - Helper: TextIndex
//-----------------------------------------------------------------------------

// Helpers: Hashing
API int HashData(const void *data, size_t data_size, int seed = 0);
API int HashStr(const char *data, size_t data_size = 0, int seed = 0);

// Helpers: Sorting
#ifndef Qsort
static inline void Qsort(void *base, size_t count, size_t size_of_element,
                         int(CDECL *compare_func)(void const *, void const *)) {
  if (count > 1)
    qsort(base, count, size_of_element, compare_func);
}
#endif

// Helpers: Color Blending
API unsigned int AlphaBlendColors(unsigned int col_a, unsigned int col_b);

// Helpers: Bit manipulation
static inline bool IsPowerOfTwo(int v) { return v != 0 && (v & (v - 1)) == 0; }
static inline bool IsPowerOfTwo(unsigned long long v) {
  return v != 0 && (v & (v - 1)) == 0;
}
static inline int UpperPowerOfTwo(int v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

// Helpers: String
API int Stricmp(const char *str1,
                const char *str2); // Case insensitive compare.
API int Strnicmp(const char *str1, const char *str2,
                 size_t count); // Case insensitive compare to a certain count.
API void Strncpy(char *dst, const char *src,
                 size_t count);    // Copy to a certain count and always zero
                                   // terminate (strncpy doesn't).
API char *Strdup(const char *str); // Duplicate a string.
API char *Strdupcpy(
    char *dst, size_t *p_dst_size,
    const char *str); // Copy in provided buffer, recreate buffer if needed.
API const char *
StrchrRange(const char *str_begin, const char *str_end,
            char c); // Find first occurrence of 'c' in string range.
API const char *StreolRange(const char *str,
                            const char *str_end); // End end-of-line
API const char *
Stristr(const char *haystack, const char *haystack_end, const char *needle,
        const char *needle_end); // Find a substring in a string range.
API void
StrTrimBlanks(char *str); // Remove leading and trailing blanks from a buffer.
API const char *
StrSkipBlank(const char *str);     // Find first non-blank character.
API int StrlenW(const Wchar *str); // Computer string length (Wchar string)
API const Wchar *
StrbolW(const Wchar *buf_mid_line,
        const Wchar *buf_begin); // Find beginning-of-line (Wchar string)
MSVC_RUNTIME_CHECKS_OFF
static inline char ToUpper(char c) {
  return (c >= 'a' && c <= 'z') ? c &= ~32 : c;
}
static inline bool CharIsBlankA(char c) { return c == ' ' || c == '\t'; }
static inline bool CharIsBlankW(unsigned int c) {
  return c == ' ' || c == '\t' || c == 0x3000;
}
MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Formatting
API int FormatString(char *buf, size_t buf_size, const char *fmt, ...)
    FMTARGS(3);
API int FormatStringV(char *buf, size_t buf_size, const char *fmt, va_list args)
    FMTLIST(3);
API void FormatStringToTempBuffer(const char **out_buf,
                                  const char **out_buf_end, const char *fmt,
                                  ...) FMTARGS(3);
API void FormatStringToTempBufferV(const char **out_buf,
                                   const char **out_buf_end, const char *fmt,
                                   va_list args) FMTLIST(3);
API const char *ParseFormatFindStart(const char *format);
API const char *ParseFormatFindEnd(const char *format);
API const char *ParseFormatTrimDecorations(const char *format, char *buf,
                                           size_t buf_size);
API void ParseFormatSanitizeForPrinting(const char *fmt_in, char *fmt_out,
                                        size_t fmt_out_size);
API const char *ParseFormatSanitizeForScanning(const char *fmt_in,
                                               char *fmt_out,
                                               size_t fmt_out_size);
API int ParseFormatPrecision(const char *format, int default_value);

// Helpers: UTF-8 <> wchar conversions
API const char *TextCharToUtf8(char out_buf[5],
                               unsigned int c); // return out_buf
API int
TextStrToUtf8(char *out_buf, int out_buf_size, const Wchar *in_text,
              const Wchar *in_text_end); // return output UTF-8 bytes count
API int TextCharFromUtf8(unsigned int *out_char, const char *in_text,
                         const char *in_text_end); // read one character. return
                                                   // input UTF-8 bytes count
API int TextStrFromUtf8(
    Wchar *out_buf, int out_buf_size, const char *in_text,
    const char *in_text_end,
    const char **in_remaining = NULL); // return input UTF-8 bytes count
API int TextCountCharsFromUtf8(
    const char *in_text,
    const char
        *in_text_end); // return number of UTF-8 code-points (NOT bytes count)
API int TextCountUtf8BytesFromChar(
    const char *in_text,
    const char
        *in_text_end); // return number of bytes to express one char in UTF-8
API int TextCountUtf8BytesFromStr(
    const Wchar *in_text,
    const Wchar
        *in_text_end); // return number of bytes to express string in UTF-8
API const char *TextFindPreviousUtf8Codepoint(
    const char *in_text_start,
    const char *in_text_curr); // return previous UTF-8 code-point.

// Helpers: File System
#ifdef DISABLE_FILE_FUNCTIONS
#define DISABLE_DEFAULT_FILE_FUNCTIONS
typedef void *FileHandle;
static inline FileHandle FileOpen(const char *, const char *) { return NULL; }
static inline bool FileClose(FileHandle) { return false; }
static inline unsigned long long FileGetSize(FileHandle) {
  return (unsigned long long)-1;
}
static inline unsigned long long FileRead(void *, unsigned long long,
                                          unsigned long long, FileHandle) {
  return 0;
}
static inline unsigned long long FileWrite(const void *, unsigned long long,
                                           unsigned long long, FileHandle) {
  return 0;
}
#endif
#ifndef DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE *FileHandle;
API FileHandle FileOpen(const char *filename, const char *mode);
API bool FileClose(FileHandle file);
API unsigned long long FileGetSize(FileHandle file);
API unsigned long long FileRead(void *data, unsigned long long size,
                                unsigned long long count, FileHandle file);
API unsigned long long FileWrite(const void *data, unsigned long long size,
                                 unsigned long long count, FileHandle file);
#else
#define DISABLE_TTY_FUNCTIONS // Can't use stdout, fflush if we are not using
                              // default file functions
#endif
API void *FileLoadToMemory(const char *filename, const char *mode,
                           size_t *out_file_size = NULL, int padding_bytes = 0);

// Helpers: Maths
MSVC_RUNTIME_CHECKS_OFF
// - Wrapper for standard libs functions. (Note that demo.cpp does _not_
// use them to keep the code easy to copy)
#ifndef DISABLE_DEFAULT_MATH_FUNCTIONS
#define Fabs(X) fabsf(X)
#define Sqrt(X) sqrtf(X)
#define Fmod(X, Y) fmodf((X), (Y))
#define Cos(X) cosf(X)
#define Sin(X) sinf(X)
#define Acos(X) acosf(X)
#define Atan2(Y, X) atan2f((Y), (X))
#define Atof(STR) atof(STR)
#define Ceil(X) ceilf(X)
static inline float Pow(float x, float y) {
  return powf(x, y);
} // DragBehaviorT/SliderBehaviorT uses Pow with either float/double and need
  // the precision
static inline double Pow(double x, double y) { return pow(x, y); }
static inline float Log(float x) {
  return logf(x);
} // DragBehaviorT/SliderBehaviorT uses Log with either float/double and need
  // the precision
static inline double Log(double x) { return log(x); }
static inline int Abs(int x) { return x < 0 ? -x : x; }
static inline float Abs(float x) { return fabsf(x); }
static inline double Abs(double x) { return fabs(x); }
static inline float Sign(float x) {
  return (x < 0.0f) ? -1.0f : (x > 0.0f) ? 1.0f : 0.0f;
} // Sign operator - returns -1, 0 or 1 based on sign of argument
static inline double Sign(double x) {
  return (x < 0.0) ? -1.0 : (x > 0.0) ? 1.0 : 0.0;
}
#ifdef ENABLE_SSE
static inline float Rsqrt(float x) {
  return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
}
#else
static inline float Rsqrt(float x) { return 1.0f / sqrtf(x); }
#endif
static inline double Rsqrt(double x) { return 1.0 / sqrt(x); }
#endif
// - Min/Max/Clamp/Lerp/Swap are used by widgets which support variety
// of types: signed/unsigned int/long long float/double (Exceptionally using
// templates here but we could also redefine them for those types)
template <typename T> static inline T Min(T lhs, T rhs) {
  return lhs < rhs ? lhs : rhs;
}
template <typename T> static inline T Max(T lhs, T rhs) {
  return lhs >= rhs ? lhs : rhs;
}
template <typename T> static inline T Clamp(T v, T mn, T mx) {
  return (v < mn) ? mn : (v > mx) ? mx : v;
}
template <typename T> static inline T Lerp(T a, T b, float t) {
  return (T)(a + (b - a) * t);
}
template <typename T> static inline void Swap(T &a, T &b) {
  T tmp = a;
  a = b;
  b = tmp;
}
template <typename T> static inline T AddClampOverflow(T a, T b, T mn, T mx) {
  if (b < 0 && (a < mn - b))
    return mn;
  if (b > 0 && (a > mx - b))
    return mx;
  return a + b;
}
template <typename T> static inline T SubClampOverflow(T a, T b, T mn, T mx) {
  if (b > 0 && (a < mn + b))
    return mn;
  if (b < 0 && (a > mx + b))
    return mx;
  return a - b;
}
// - Misc maths helpers
static inline Vec2 Min(const Vec2 &lhs, const Vec2 &rhs) {
  return Vec2(lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y);
}
static inline Vec2 Max(const Vec2 &lhs, const Vec2 &rhs) {
  return Vec2(lhs.x >= rhs.x ? lhs.x : rhs.x, lhs.y >= rhs.y ? lhs.y : rhs.y);
}
static inline Vec2 Clamp(const Vec2 &v, const Vec2 &mn, Vec2 mx) {
  return Vec2((v.x < mn.x)   ? mn.x
              : (v.x > mx.x) ? mx.x
                             : v.x,
              (v.y < mn.y)   ? mn.y
              : (v.y > mx.y) ? mx.y
                             : v.y);
}
static inline Vec2 Lerp(const Vec2 &a, const Vec2 &b, float t) {
  return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}
static inline Vec2 Lerp(const Vec2 &a, const Vec2 &b, const Vec2 &t) {
  return Vec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y);
}
static inline Vec4 Lerp(const Vec4 &a, const Vec4 &b, float t) {
  return Vec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
              a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}
static inline float Saturate(float f) {
  return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f;
}
static inline float LengthSqr(const Vec2 &lhs) {
  return (lhs.x * lhs.x) + (lhs.y * lhs.y);
}
static inline float LengthSqr(const Vec4 &lhs) {
  return (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w);
}
static inline float InvLength(const Vec2 &lhs, float fail_value) {
  float d = (lhs.x * lhs.x) + (lhs.y * lhs.y);
  if (d > 0.0f)
    return Rsqrt(d);
  return fail_value;
}
static inline float Trunc(float f) { return (float)(int)(f); }
static inline Vec2 Trunc(const Vec2 &v) {
  return Vec2((float)(int)(v.x), (float)(int)(v.y));
}
static inline float Floor(float f) {
  return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1);
} // Decent replacement for floorf()
static inline Vec2 Floor(const Vec2 &v) { return Vec2(Floor(v.x), Floor(v.y)); }
static inline int ModPositive(int a, int b) { return (a + b) % b; }
static inline float Dot(const Vec2 &a, const Vec2 &b) {
  return a.x * b.x + a.y * b.y;
}
static inline Vec2 Rotate(const Vec2 &v, float cos_a, float sin_a) {
  return Vec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a);
}
static inline float LinearSweep(float current, float target, float speed) {
  if (current < target)
    return Min(current + speed, target);
  if (current > target)
    return Max(current - speed, target);
  return current;
}
static inline Vec2 Mul(const Vec2 &lhs, const Vec2 &rhs) {
  return Vec2(lhs.x * rhs.x, lhs.y * rhs.y);
}
static inline bool IsFloatAboveGuaranteedIntegerPrecision(float f) {
  return f <= -16777216 || f >= 16777216;
}
static inline float ExponentialMovingAverage(float avg, float sample, int n) {
  avg -= avg / n;
  avg += sample / n;
  return avg;
}
MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Geometry
API Vec2 BezierCubicCalc(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                         const Vec2 &p4, float t);
API Vec2 BezierCubicClosestPoint(
    const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4,
    const Vec2 &p,
    int num_segments); // For curves with explicit number of segments
API Vec2 BezierCubicClosestPointCasteljau(
    const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4,
    const Vec2 &p, float tess_tol); // For auto-tessellated curves you can use
                                    // tess_tol = style.CurveTessellationTol
API Vec2 BezierQuadraticCalc(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                             float t);
API Vec2 LineClosestPoint(const Vec2 &a, const Vec2 &b, const Vec2 &p);
API bool TriangleContainsPoint(const Vec2 &a, const Vec2 &b, const Vec2 &c,
                               const Vec2 &p);
API Vec2 TriangleClosestPoint(const Vec2 &a, const Vec2 &b, const Vec2 &c,
                              const Vec2 &p);
API void TriangleBarycentricCoords(const Vec2 &a, const Vec2 &b, const Vec2 &c,
                                   const Vec2 &p, float &out_u, float &out_v,
                                   float &out_w);
inline float TriangleArea(const Vec2 &a, const Vec2 &b, const Vec2 &c) {
  return Fabs((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y))) *
         0.5f;
}

// Helper: Vec1 (1D vector)
// (this odd construct is used to facilitate the transition between 1D and 2D,
// and the maintenance of some branches/patches)
MSVC_RUNTIME_CHECKS_OFF
struct Vec1 {
  float x;
  constexpr Vec1() : x(0.0f) {}
  constexpr Vec1(float _x) : x(_x) {}
};

// Helper: Vec2ih (2D vector, half-size integer, for long-term packed storage)
struct Vec2ih {
  short x, y;
  constexpr Vec2ih() : x(0), y(0) {}
  constexpr Vec2ih(short _x, short _y) : x(_x), y(_y) {}
  constexpr explicit Vec2ih(const Vec2 &rhs)
      : x((short)rhs.x), y((short)rhs.y) {}
};

// Helper: Rect (2D axis aligned bounding-box)
// NB: we can't rely on Vec2 math operators being available here!
struct API Rect {
  Vec2 Min; // Upper-left
  Vec2 Max; // Lower-right

  constexpr Rect() : Min(0.0f, 0.0f), Max(0.0f, 0.0f) {}
  constexpr Rect(const Vec2 &min, const Vec2 &max) : Min(min), Max(max) {}
  constexpr Rect(const Vec4 &v) : Min(v.x, v.y), Max(v.z, v.w) {}
  constexpr Rect(float x1, float y1, float x2, float y2)
      : Min(x1, y1), Max(x2, y2) {}

  Vec2 GetCenter() const {
    return Vec2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f);
  }
  Vec2 GetSize() const { return Vec2(Max.x - Min.x, Max.y - Min.y); }
  float GetWidth() const { return Max.x - Min.x; }
  float GetHeight() const { return Max.y - Min.y; }
  float GetArea() const { return (Max.x - Min.x) * (Max.y - Min.y); }
  Vec2 GetTL() const { return Min; }                // Top-left
  Vec2 GetTR() const { return Vec2(Max.x, Min.y); } // Top-right
  Vec2 GetBL() const { return Vec2(Min.x, Max.y); } // Bottom-left
  Vec2 GetBR() const { return Max; }                // Bottom-right
  bool Contains(const Vec2 &p) const {
    return p.x >= Min.x && p.y >= Min.y && p.x < Max.x && p.y < Max.y;
  }
  bool Contains(const Rect &r) const {
    return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x &&
           r.Max.y <= Max.y;
  }
  bool ContainsWithPad(const Vec2 &p, const Vec2 &pad) const {
    return p.x >= Min.x - pad.x && p.y >= Min.y - pad.y &&
           p.x < Max.x + pad.x && p.y < Max.y + pad.y;
  }
  bool Overlaps(const Rect &r) const {
    return r.Min.y < Max.y && r.Max.y > Min.y && r.Min.x < Max.x &&
           r.Max.x > Min.x;
  }
  void Add(const Vec2 &p) {
    if (Min.x > p.x)
      Min.x = p.x;
    if (Min.y > p.y)
      Min.y = p.y;
    if (Max.x < p.x)
      Max.x = p.x;
    if (Max.y < p.y)
      Max.y = p.y;
  }
  void Add(const Rect &r) {
    if (Min.x > r.Min.x)
      Min.x = r.Min.x;
    if (Min.y > r.Min.y)
      Min.y = r.Min.y;
    if (Max.x < r.Max.x)
      Max.x = r.Max.x;
    if (Max.y < r.Max.y)
      Max.y = r.Max.y;
  }
  void Expand(const float amount) {
    Min.x -= amount;
    Min.y -= amount;
    Max.x += amount;
    Max.y += amount;
  }
  void Expand(const Vec2 &amount) {
    Min.x -= amount.x;
    Min.y -= amount.y;
    Max.x += amount.x;
    Max.y += amount.y;
  }
  void Translate(const Vec2 &d) {
    Min.x += d.x;
    Min.y += d.y;
    Max.x += d.x;
    Max.y += d.y;
  }
  void TranslateX(float dx) {
    Min.x += dx;
    Max.x += dx;
  }
  void TranslateY(float dy) {
    Min.y += dy;
    Max.y += dy;
  }
  void ClipWith(const Rect &r) {
    Min = ::Max(Min, r.Min);
    Max = ::Min(Max, r.Max);
  } // Simple version, may lead to an inverted rectangle, which is fine for
    // Contains/Overlaps test but not for display.
  void ClipWithFull(const Rect &r) {
    Min = Clamp(Min, r.Min, r.Max);
    Max = Clamp(Max, r.Min, r.Max);
  } // Full version, ensure both points are fully clipped.
  void Floor() {
    Min.x = TRUNC(Min.x);
    Min.y = TRUNC(Min.y);
    Max.x = TRUNC(Max.x);
    Max.y = TRUNC(Max.y);
  }
  bool IsInverted() const { return Min.x > Max.x || Min.y > Max.y; }
  Vec4 ToVec4() const { return Vec4(Min.x, Min.y, Max.x, Max.y); }
};

// Helper: BitArray
#define BITARRAY_TESTBIT(_ARRAY, _N)                                           \
  ((_ARRAY[(_N) >> 5] & ((unsigned int)1 << ((_N) & 31))) !=                   \
   0) // Macro version of BitArrayTestBit(): ensure args have side-effect or
      // are costly!
#define BITARRAY_CLEARBIT(_ARRAY, _N)                                          \
  ((_ARRAY[(_N) >> 5] &=                                                       \
    ~((unsigned int)1 << ((_N) &                                               \
                          31)))) // Macro version of BitArrayClearBit(): ensure
                                 // args have side-effect or are costly!
inline size_t BitArrayGetStorageSizeInBytes(int bitcount) {
  return (size_t)((bitcount + 31) >> 5) << 2;
}
inline void BitArrayClearAllBits(unsigned int *arr, int bitcount) {
  memset(arr, 0, BitArrayGetStorageSizeInBytes(bitcount));
}
inline bool BitArrayTestBit(const unsigned int *arr, int n) {
  unsigned int mask = (unsigned int)1 << (n & 31);
  return (arr[n >> 5] & mask) != 0;
}
inline void BitArrayClearBit(unsigned int *arr, int n) {
  unsigned int mask = (unsigned int)1 << (n & 31);
  arr[n >> 5] &= ~mask;
}
inline void BitArraySetBit(unsigned int *arr, int n) {
  unsigned int mask = (unsigned int)1 << (n & 31);
  arr[n >> 5] |= mask;
}
inline void BitArraySetBitRange(unsigned int *arr, int n,
                                int n2) // Works on range [n..n2)
{
  n2--;
  while (n <= n2) {
    int a_mod = (n & 31);
    int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
    unsigned int mask = (unsigned int)(((unsigned long long)1 << b_mod) - 1) &
                        ~(unsigned int)(((unsigned long long)1 << a_mod) - 1);
    arr[n >> 5] |= mask;
    n = (n + 32) & ~31;
  }
}

typedef unsigned int *BitArrayPtr; // Name for use in structs

// Helper: BitArray class (wrapper over BitArray functions)
// Store 1-bit per value.
template <int BITCOUNT, int OFFSET = 0> struct BitArray {
  unsigned int Storage[(BITCOUNT + 31) >> 5];
  BitArray() { ClearAllBits(); }
  void ClearAllBits() { memset(Storage, 0, sizeof(Storage)); }
  void SetAllBits() { memset(Storage, 255, sizeof(Storage)); }
  bool TestBit(int n) const {
    n += OFFSET;
    assert(n >= 0 && n < BITCOUNT);
    return BITARRAY_TESTBIT(Storage, n);
  }
  void SetBit(int n) {
    n += OFFSET;
    assert(n >= 0 && n < BITCOUNT);
    BitArraySetBit(Storage, n);
  }
  void ClearBit(int n) {
    n += OFFSET;
    assert(n >= 0 && n < BITCOUNT);
    BitArrayClearBit(Storage, n);
  }
  void SetBitRange(int n, int n2) {
    n += OFFSET;
    n2 += OFFSET;
    assert(n >= 0 && n < BITCOUNT && n2 > n && n2 <= BITCOUNT);
    BitArraySetBitRange(Storage, n, n2);
  } // Works on range [n..n2)
  bool operator[](int n) const {
    n += OFFSET;
    assert(n >= 0 && n < BITCOUNT);
    return BITARRAY_TESTBIT(Storage, n);
  }
};

// Helper: BitVector
// Store 1-bit per value.
struct API BitVector {
  Vector<unsigned int> Storage;
  void Create(int sz) {
    Storage.resize((sz + 31) >> 5);
    memset(Storage.Data, 0, (size_t)Storage.Size * sizeof(Storage.Data[0]));
  }
  void Clear() { Storage.clear(); }
  bool TestBit(int n) const {
    assert(n < (Storage.Size << 5));
    return BITARRAY_TESTBIT(Storage.Data, n);
  }
  void SetBit(int n) {
    assert(n < (Storage.Size << 5));
    BitArraySetBit(Storage.Data, n);
  }
  void ClearBit(int n) {
    assert(n < (Storage.Size << 5));
    BitArrayClearBit(Storage.Data, n);
  }
};
MSVC_RUNTIME_CHECKS_RESTORE

// Helper: Span<>
// Pointing to a span of data we don't own.
template <typename T> struct Span {
  T *Data;
  T *DataEnd;

  // Constructors, destructor
  inline Span() { Data = DataEnd = NULL; }
  inline Span(T *data, int size) {
    Data = data;
    DataEnd = data + size;
  }
  inline Span(T *data, T *data_end) {
    Data = data;
    DataEnd = data_end;
  }

  inline void set(T *data, int size) {
    Data = data;
    DataEnd = data + size;
  }
  inline void set(T *data, T *data_end) {
    Data = data;
    DataEnd = data_end;
  }
  inline int size() const { return (int)(ptrdiff_t)(DataEnd - Data); }
  inline int size_in_bytes() const {
    return (int)(ptrdiff_t)(DataEnd - Data) * (int)sizeof(T);
  }
  inline T &operator[](int i) {
    T *p = Data + i;
    assert(p >= Data && p < DataEnd);
    return *p;
  }
  inline const T &operator[](int i) const {
    const T *p = Data + i;
    assert(p >= Data && p < DataEnd);
    return *p;
  }

  inline T *begin() { return Data; }
  inline const T *begin() const { return Data; }
  inline T *end() { return DataEnd; }
  inline const T *end() const { return DataEnd; }

  // Utilities
  inline int index_from_ptr(const T *it) const {
    assert(it >= Data && it < DataEnd);
    const ptrdiff_t off = it - Data;
    return (int)off;
  }
};

// Helper: SpanAllocator<>
// Facilitate storing multiple chunks into a single large block (the "arena")
// - Usage: call Reserve() N times, allocate GetArenaSizeInBytes() worth, pass
// it to SetArenaBasePtr(), call GetSpan() N times to retrieve the aligned
// ranges.
template <int CHUNKS> struct SpanAllocator {
  char *BasePtr;
  int CurrOff;
  int CurrIdx;
  int Offsets[CHUNKS];
  int Sizes[CHUNKS];

  SpanAllocator() { memset(this, 0, sizeof(*this)); }
  inline void Reserve(int n, size_t sz, int a = 4) {
    assert(n == CurrIdx && n < CHUNKS);
    CurrOff = MEMALIGN(CurrOff, a);
    Offsets[n] = CurrOff;
    Sizes[n] = (int)sz;
    CurrIdx++;
    CurrOff += (int)sz;
  }
  inline int GetArenaSizeInBytes() { return CurrOff; }
  inline void SetArenaBasePtr(void *base_ptr) { BasePtr = (char *)base_ptr; }
  inline void *GetSpanPtrBegin(int n) {
    assert(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS);
    return (void *)(BasePtr + Offsets[n]);
  }
  inline void *GetSpanPtrEnd(int n) {
    assert(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS);
    return (void *)(BasePtr + Offsets[n] + Sizes[n]);
  }
  template <typename T> inline void GetSpan(int n, Span<T> *span) {
    span->set((T *)GetSpanPtrBegin(n), (T *)GetSpanPtrEnd(n));
  }
};

// Helper: Pool<>
// Basic keyed storage for contiguous instances, slow/amortized insertion, O(1)
// indexable, O(Log N) queries by int over a dense/hot buffer, Honor
// constructor/destructor. Add/remove invalidate all pointers. Indexes have the
// same lifetime as the associated object.
typedef int PoolIdx;
template <typename T> struct Pool {
  Vector<T> Buf;      // Contiguous data
  Storage Map;        // unsigned int->Index
  PoolIdx FreeIdx;    // Next free idx to use
  PoolIdx AliveCount; // Number of active/alive items (for display purpose)

  Pool() { FreeIdx = AliveCount = 0; }
  ~Pool() { Clear(); }
  T *GetByKey(int key) {
    int idx = Map.GetInt(key, -1);
    return (idx != -1) ? &Buf[idx] : NULL;
  }
  T *GetByIndex(PoolIdx n) { return &Buf[n]; }
  PoolIdx GetIndex(const T *p) const {
    assert(p >= Buf.Data && p < Buf.Data + Buf.Size);
    return (PoolIdx)(p - Buf.Data);
  }
  T *GetOrAddByKey(int key) {
    int *p_idx = Map.GetIntRef(key, -1);
    if (*p_idx != -1)
      return &Buf[*p_idx];
    *p_idx = FreeIdx;
    return Add();
  }
  bool Contains(const T *p) const {
    return (p >= Buf.Data && p < Buf.Data + Buf.Size);
  }
  void Clear() {
    for (int n = 0; n < Map.Data.Size; n++) {
      int idx = Map.Data[n].val_i;
      if (idx != -1)
        Buf[idx].~T();
    }
    Map.Clear();
    Buf.clear();
    FreeIdx = AliveCount = 0;
  }
  T *Add() {
    int idx = FreeIdx;
    if (idx == Buf.Size) {
      Buf.resize(Buf.Size + 1);
      FreeIdx++;
    } else {
      FreeIdx = *(int *)&Buf[idx];
    }
    PLACEMENT_NEW(&Buf[idx]) T();
    AliveCount++;
    return &Buf[idx];
  }
  void Remove(int key, const T *p) { Remove(key, GetIndex(p)); }
  void Remove(int key, PoolIdx idx) {
    Buf[idx].~T();
    *(int *)&Buf[idx] = FreeIdx;
    FreeIdx = idx;
    Map.SetInt(key, -1);
    AliveCount--;
  }
  void Reserve(int capacity) {
    Buf.reserve(capacity);
    Map.Data.reserve(capacity);
  }

  // To iterate a Pool: for (int n = 0; n < pool.GetMapSize(); n++) if (T* t =
  // pool.TryGetMapData(n)) { ... } Can be avoided if you know .Remove() has
  // never been called on the pool, or AliveCount == GetMapSize()
  int GetAliveCount() const {
    return AliveCount;
  } // Number of active/alive items in the pool (for display purpose)
  int GetBufSize() const { return Buf.Size; }
  int GetMapSize() const {
    return Map.Data.Size;
  } // It is the map we need iterate to find valid items, since we don't have
    // "alive" storage anywhere
  T *TryGetMapData(PoolIdx n) {
    int idx = Map.Data[n].val_i;
    if (idx == -1)
      return NULL;
    return GetByIndex(idx);
  }
#ifndef DISABLE_OBSOLETE_FUNCTIONS
  int GetSize() {
    return GetMapSize();
  } // For Plot: should use GetMapSize() from (VERSION_NUM >= 18304)
#endif
};

// Helper: ChunkStream<>
// Build and iterate a contiguous stream of variable-sized structures.
// This is used by Settings to store persistent data while reducing allocation
// count. We store the chunk size first, and align the final size on 4 bytes
// boundaries. The tedious/zealous amount of casting is to avoid -Wcast-align
// warnings.
template <typename T> struct ChunkStream {
  Vector<char> Buf;

  void clear() { Buf.clear(); }
  bool empty() const { return Buf.Size == 0; }
  int size() const { return Buf.Size; }
  T *alloc_chunk(size_t sz) {
    size_t HDR_SZ = 4;
    sz = MEMALIGN(HDR_SZ + sz, 4u);
    int off = Buf.Size;
    Buf.resize(off + (int)sz);
    ((int *)(void *)(Buf.Data + off))[0] = (int)sz;
    return (T *)(void *)(Buf.Data + off + (int)HDR_SZ);
  }
  T *begin() {
    size_t HDR_SZ = 4;
    if (!Buf.Data)
      return NULL;
    return (T *)(void *)(Buf.Data + HDR_SZ);
  }
  T *next_chunk(T *p) {
    size_t HDR_SZ = 4;
    assert(p >= begin() && p < end());
    p = (T *)(void *)((char *)(void *)p + chunk_size(p));
    if (p == (T *)(void *)((char *)end() + HDR_SZ))
      return (T *)0;
    assert(p < end());
    return p;
  }
  int chunk_size(const T *p) { return ((const int *)p)[-1]; }
  T *end() { return (T *)(void *)(Buf.Data + Buf.Size); }
  int offset_from_ptr(const T *p) {
    assert(p >= begin() && p < end());
    const ptrdiff_t off = (const char *)p - Buf.Data;
    return (int)off;
  }
  T *ptr_from_offset(int off) {
    assert(off >= 4 && off < Buf.Size);
    return (T *)(void *)(Buf.Data + off);
  }
  void swap(ChunkStream<T> &rhs) { rhs.Buf.swap(Buf); }
};

// Helper: TextIndex<>
// Maintain a line index for a text buffer. This is a strong candidate to be
// moved into the public API.
struct TextIndex {
  Vector<int> LineOffsets;
  int EndOffset = 0; // Because we don't own text buffer we need to maintain
                     // EndOffset (may bake in LineOffsets?)

  void clear() {
    LineOffsets.clear();
    EndOffset = 0;
  }
  int size() { return LineOffsets.Size; }
  const char *get_line_begin(const char *base, int n) {
    return base + LineOffsets[n];
  }
  const char *get_line_end(const char *base, int n) {
    return base +
           (n + 1 < LineOffsets.Size ? (LineOffsets[n + 1] - 1) : EndOffset);
  }
  void append(const char *base, int old_size, int new_size);
};

//-----------------------------------------------------------------------------
// [SECTION] DrawList support
//-----------------------------------------------------------------------------

// DrawList: Helper function to calculate a circle's segment count given its
// radius and a "maximum error" value. Estimation of number of circle segment
// based on error is derived using method described in
// https://stackoverflow.com/a/2244088/15194693 Number of segments (N) is
// calculated using equation:
//   N = ceil ( pi / acos(1 - error / r) )     where r > 0, error <= r
// Our equation is significantly simpler that one in the post thanks for
// choosing segment that is perpendicular to X axis. Follow steps in the article
// from this starting condition and you will will get this result.
//
// Rendering circles with an odd number of segments, while mathematically
// correct will produce asymmetrical results on the raster grid. Therefore we're
// rounding N to next even number (7->8, 8->8, 9->10 etc.)
#define ROUNDUP_TO_EVEN(_V) ((((_V) + 1) / 2) * 2)
#define DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN 4
#define DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX 512
#define DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD, _MAXERROR)                     \
  Clamp(ROUNDUP_TO_EVEN(                                                       \
            (int)Ceil(PI / Acos(1 - Min((_MAXERROR), (_RAD)) / (_RAD)))),      \
        DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

// Raw equation from DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC rewritten for 'r' and
// 'error'.
#define DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N, _MAXERROR)                     \
  ((_MAXERROR) / (1 - Cos(PI / Max((float)(_N), PI))))
#define DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N, _RAD)                      \
  ((1 - Cos(PI / Max((float)(_N), PI))) / (_RAD))

// DrawList: Lookup table size for adaptive arc drawing, cover full circle.
#ifndef DRAWLIST_ARCFAST_TABLE_SIZE
#define DRAWLIST_ARCFAST_TABLE_SIZE 48 // Number of samples in lookup table.
#endif
#define DRAWLIST_ARCFAST_SAMPLE_MAX                                            \
  DRAWLIST_ARCFAST_TABLE_SIZE // Sample index _PathArcToFastEx() for 360
                              // angle.

// Data shared between all DrawList instances
// You may want to create your own instance of this if you want to use
// DrawList completely without Gui. In that case, watch out for future
// changes to this structure.
struct API DrawListSharedData {
  Vec2 TexUvWhitePixel; // UV of white pixel in the atlas
  Font
      *Font; // Current/default font (optional, for simplified AddText overload)
  float FontSize; // Current/default font size (optional, for simplified AddText
                  // overload)
  float CurveTessellationTol;  // Tessellation tolerance when using
                               // PathBezierCurveTo()
  float CircleSegmentMaxError; // Number of circle segments to use per pixel of
                               // radius for AddCircle() etc
  Vec4 ClipRectFullscreen;     // Value for PushClipRectFullscreen()
  int InitialFlags;            // Initial flags at the beginning of the frame
                               // (it is possible to alter flags on a
                               // per-drawlist basis afterwards)

  // [Internal] Temp write buffer
  Vector<Vec2> TempBuffer;

  // [Internal] Lookup tables
  Vec2 ArcFastVtx[DRAWLIST_ARCFAST_TABLE_SIZE]; // Sample points on the
                                                // quarter of the circle.
  float ArcFastRadiusCutoff; // Cutoff radius after which arc drawing will
                             // fallback to slower PathArcTo()
  unsigned char
      CircleSegmentCounts[64]; // Precomputed segment count for given radius
                               // before we calculate it dynamically (to avoid
                               // calculation overhead)
  const Vec4 *TexUvLines;      // UV of anti-aliased lines in the atlas

  DrawListSharedData();
  void SetCircleTessellationMaxError(float max_error);
};

struct DrawDataBuilder {
  Vector<DrawList *> *Layers[2]; // Pointers to global layers for: regular,
                                 // tooltip. LayersP[0] is owned by DrawData.
  Vector<DrawList *> LayerData1;

  DrawDataBuilder() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Widgets support: flags, enums, data structures
//-----------------------------------------------------------------------------

// Flags used by upcoming items
// - input: PushItemFlag() manipulates g.CurrentItemFlags, ItemAdd() calls may
// add extra flags.
// - output: stored in g.LastItemData.InFlags
// Current window shared by all windows.
// This is going to be exposed in gui.hpp when stabilized enough.
enum ItemFlags_ {
  // Controlled by user
  ItemFlags_None = 0,
  ItemFlags_NoTabStop =
      1 << 0, // false     // Disable keyboard tabbing. This is a "lighter"
              // version of ItemFlags_NoNav.
  ItemFlags_ButtonRepeat =
      1 << 1, // false     // Button() will return true multiple times based on
              // io.KeyRepeatDelay and io.KeyRepeatRate settings.
  ItemFlags_Disabled =
      1 << 2, // false     // Disable interactions but doesn't affect visuals.
              // See BeginDisabled()/EndDisabled().
  ItemFlags_NoNav =
      1 << 3, // false     // Disable any form of focusing (keyboard/gamepad
              // directional navigation and SetKeyboardFocusHere() calls)
  ItemFlags_NoNavDefaultFocus =
      1 << 4, // false     // Disable item being a candidate for default focus
              // (e.g. used by title bar items)
  ItemFlags_SelectableDontClosePopup =
      1 << 5, // false     // Disable MenuItem/Selectable() automatically
              // closing their popup window
  ItemFlags_MixedValue =
      1 << 6, // false     // [BETA] Represent a mixed/indeterminate value,
              // generally multi-selection where values differ. Currently only
              // supported by Checkbox() (later should support all sorts of
              // widgets)
  ItemFlags_ReadOnly =
      1 << 7, // false     // [ALPHA] Allow hovering interactions but underlying
              // value is not changed.
  ItemFlags_NoWindowHoverableCheck =
      1 << 8, // false     // Disable hoverable check in ItemHoverable()
  ItemFlags_AllowOverlap =
      1 << 9, // false     // Allow being overlapped by another widget.
              // Not-hovered to Hovered transition deferred by a frame.

  // Controlled by widget code
  ItemFlags_Inputable =
      1 << 10, // false     // [WIP] Auto-activate input mode when tab focused.
               // Currently only used and supported by a few items before it
               // becomes a generic feature.
  ItemFlags_HasSelectionUserData =
      1 << 11, // false     // Set by SetNextItemSelectionUserData()
};

// Status flags for an already submitted item
// - output: stored in g.LastItemData.StatusFlags
enum ItemStatusFlags_ {
  ItemStatusFlags_None = 0,
  ItemStatusFlags_HoveredRect =
      1 << 0, // Mouse position is within item rectangle (does NOT mean that the
              // window is in correct z-order and can be hovered!, this is only
              // one part of the most-common IsItemHovered test)
  ItemStatusFlags_HasDisplayRect = 1
                                   << 1, // g.LastItemData.DisplayRect is valid
  ItemStatusFlags_Edited =
      1 << 2, // Value exposed by item was edited in the current frame (should
              // match the bool return value of most widgets)
  ItemStatusFlags_ToggledSelection =
      1 << 3, // Set when Selectable(), TreeNode() reports toggling a selection.
              // We can't report "Selected", only state changes, in order to
              // easily handle clipping with less issues.
  ItemStatusFlags_ToggledOpen =
      1 << 4, // Set when TreeNode() reports toggling their open state.
  ItemStatusFlags_HasDeactivated =
      1 << 5, // Set if the widget/group is able to provide data for the
              // ItemStatusFlags_Deactivated flag.
  ItemStatusFlags_Deactivated =
      1 << 6, // Only valid if ItemStatusFlags_HasDeactivated is set.
  ItemStatusFlags_HoveredWindow = 1 << 7, // Override the HoveredWindow test to
                                          // allow cross-window hover testing.
  ItemStatusFlags_Visible =
      1 << 8, // [WIP] Set when item is overlapping the current clipping
              // rectangle (Used internally. Please don't use yet: API/system
              // will change as we refactor Itemadd()).
  ItemStatusFlags_HasClipRect = 1 << 9, // g.LastItemData.ClipRect is valid

// Additional status + semantic for TestEngine
#ifdef ENABLE_TEST_ENGINE
  ItemStatusFlags_Openable = 1 << 20, // Item is an openable (e.g. TreeNode)
  ItemStatusFlags_Opened = 1 << 21,   // Opened status
  ItemStatusFlags_Checkable =
      1 << 22, // Item is a checkable (e.g. CheckBox, MenuItem)
  ItemStatusFlags_Checked = 1 << 23, // Checked status
  ItemStatusFlags_Inputable =
      1 << 24, // Item is a text-inputable (e.g. InputText, SliderXXX, DragXXX)
#endif
};

// Extend HoveredFlags_
enum HoveredFlagsPrivate_ {
  HoveredFlags_DelayMask_ = HoveredFlags_DelayNone | HoveredFlags_DelayShort |
                            HoveredFlags_DelayNormal |
                            HoveredFlags_NoSharedDelay,
  HoveredFlags_AllowedMaskForIsWindowHovered =
      HoveredFlags_ChildWindows | HoveredFlags_RootWindow |
      HoveredFlags_AnyWindow | HoveredFlags_NoPopupHierarchy |
      HoveredFlags_DockHierarchy | HoveredFlags_AllowWhenBlockedByPopup |
      HoveredFlags_AllowWhenBlockedByActiveItem | HoveredFlags_ForTooltip |
      HoveredFlags_Stationary,
  HoveredFlags_AllowedMaskForIsItemHovered =
      HoveredFlags_AllowWhenBlockedByPopup |
      HoveredFlags_AllowWhenBlockedByActiveItem |
      HoveredFlags_AllowWhenOverlapped | HoveredFlags_AllowWhenDisabled |
      HoveredFlags_NoNavOverride | HoveredFlags_ForTooltip |
      HoveredFlags_Stationary | HoveredFlags_DelayMask_,
};

// Extend InputTextFlags_
enum InputTextFlagsPrivate_ {
  // [Internal]
  InputTextFlags_Multiline = 1
                             << 26, // For internal use by InputTextMultiline()
  InputTextFlags_NoMarkEdited =
      1 << 27, // For internal use by functions using InputText() before
               // reformatting data
  InputTextFlags_MergedItem =
      1 << 28, // For internal use by TempInputText(), will skip calling
               // ItemAdd(). Require bounding-box to strictly match.
};

// Extend ButtonFlags_
enum ButtonFlagsPrivate_ {
  ButtonFlags_PressedOnClick = 1
                               << 4, // return true on click (mouse down event)
  ButtonFlags_PressedOnClickRelease =
      1 << 5, // [Default] return true on click + release on same item <-- this
              // is what the majority of Button are using
  ButtonFlags_PressedOnClickReleaseAnywhere =
      1 << 6, // return true on click + release even if the release event is not
              // done while hovering the item
  ButtonFlags_PressedOnRelease =
      1 << 7, // return true on release (default requires click+release)
  ButtonFlags_PressedOnDoubleClick =
      1 << 8, // return true on double-click (default requires click+release)
  ButtonFlags_PressedOnDragDropHold =
      1 << 9, // return true when held into while we are drag and dropping
              // another item (used by e.g. tree nodes, collapsing headers)
  ButtonFlags_Repeat = 1 << 10, // hold to repeat
  ButtonFlags_FlattenChildren =
      1 << 11, // allow interactions even if a child window is overlapping
  ButtonFlags_AllowOverlap =
      1 << 12, // require previous frame HoveredId to either match id or be null
               // before being usable.
  ButtonFlags_DontClosePopups =
      1
      << 13, // disable automatically closing parent popup on press // [UNUSED]
  // ButtonFlags_Disabled             = 1 << 14,  // disable interactions
  // -> use BeginDisabled() or ItemFlags_Disabled
  ButtonFlags_AlignTextBaseLine =
      1 << 15, // vertically align button to match text baseline - ButtonEx()
               // only // FIXME: Should be removed and handled by SmallButton(),
               // not possible currently because of DC.CursorPosPrevLine
  ButtonFlags_NoKeyModifiers =
      1 << 16, // disable mouse interaction if a key modifier is held
  ButtonFlags_NoHoldingActiveId =
      1 << 17, // don't set ActiveId while holding the mouse
               // (ButtonFlags_PressedOnClick only)
  ButtonFlags_NoNavFocus =
      1 << 18, // don't override navigation focus when activated (FIXME: this is
               // essentially used everytime an item uses ItemFlags_NoNav,
               // but because legacy specs don't requires LastItemData to be set
               // ButtonBehavior(), we can't poll g.LastItemData.InFlags)
  ButtonFlags_NoHoveredOnFocus =
      1 << 19, // don't report as hovered when nav focus is on this item
  ButtonFlags_NoSetKeyOwner =
      1 << 20, // don't set key/input owner on the initial click (note: mouse
               // buttons are keys! often, the key in question will be
               // Key_MouseLeft!)
  ButtonFlags_NoTestKeyOwner =
      1 << 21, // don't test key/input owner when polling the key (note: mouse
               // buttons are keys! often, the key in question will be
               // Key_MouseLeft!)
  ButtonFlags_PressedOnMask_ =
      ButtonFlags_PressedOnClick | ButtonFlags_PressedOnClickRelease |
      ButtonFlags_PressedOnClickReleaseAnywhere | ButtonFlags_PressedOnRelease |
      ButtonFlags_PressedOnDoubleClick | ButtonFlags_PressedOnDragDropHold,
  ButtonFlags_PressedOnDefault_ = ButtonFlags_PressedOnClickRelease,
};

// Extend ComboFlags_
enum ComboFlagsPrivate_ {
  ComboFlags_CustomPreview = 1 << 20, // enable BeginComboPreview()
};

// Extend SliderFlags_
enum SliderFlagsPrivate_ {
  SliderFlags_Vertical =
      1 << 20, // Should this slider be orientated vertically?
  SliderFlags_ReadOnly = 1 << 21, // Consider using g.NextItemData.ItemFlags
                                  // |= ItemFlags_ReadOnly instead.
};

// Extend SelectableFlags_
enum SelectableFlagsPrivate_ {
  // NB: need to be in sync with last value of SelectableFlags_
  SelectableFlags_NoHoldingActiveID = 1 << 20,
  SelectableFlags_SelectOnNav =
      1 << 21, // (WIP) Auto-select when moved into. This is not exposed in
               // public API as to handle multi-select and modifiers we will
               // need user to explicitly control focus scope. May be replaced
               // with a BeginSelection() API.
  SelectableFlags_SelectOnClick =
      1 << 22, // Override button behavior to react on Click (default is
               // Click+Release)
  SelectableFlags_SelectOnRelease =
      1 << 23, // Override button behavior to react on Release (default is
               // Click+Release)
  SelectableFlags_SpanAvailWidth =
      1 << 24, // Span all avail width even if we declared less for layout
               // purpose. FIXME: We may be able to remove this (added in
               // 6251d379, 2bcafc86 for menus)
  SelectableFlags_SetNavIdOnHover =
      1 << 25, // Set Nav/Focus int on mouse hover (used by MenuItem)
  SelectableFlags_NoPadWithHalfSpacing =
      1 << 26, // Disable padding each side with ItemSpacing * 0.5f
  SelectableFlags_NoSetKeyOwner =
      1 << 27, // Don't set key/input owner on the initial click (note: mouse
               // buttons are keys! often, the key in question will be
               // Key_MouseLeft!)
};

// Extend TreeNodeFlags_
enum TreeNodeFlagsPrivate_ {
  TreeNodeFlags_ClipLabelForTrailingButton = 1 << 20,
  TreeNodeFlags_UpsideDownArrow = 1
                                  << 21, // (FIXME-WIP) Turn Down arrow into an
                                         // Up arrow, but reversed trees (#6517)
};

enum SeparatorFlags_ {
  SeparatorFlags_None = 0,
  SeparatorFlags_Horizontal =
      1 << 0, // Axis default to current layout type, so generally Horizontal
              // unless e.g. in a menu bar
  SeparatorFlags_Vertical = 1 << 1,
  SeparatorFlags_SpanAllColumns =
      1 << 2, // Make separator cover all columns of a legacy Columns() set.
};

// Flags for FocusWindow(). This is not called FocusFlags to avoid
// confusion with public-facing int.
// FIXME: Once we finishing replacing more uses of
// GetTopMostPopupModal()+IsWindowWithinBeginStackOf() and FindBlockingModal()
// with this, we may want to change the flag to be opt-out instead of opt-in.
enum FocusRequestFlags_ {
  FocusRequestFlags_None = 0,
  FocusRequestFlags_RestoreFocusedChild =
      1 << 0, // Find last focused child (if any) and focus it instead.
  FocusRequestFlags_UnlessBelowModal =
      1 << 1, // Do not set focus if the window is below a modal.
};

enum TextFlags_ {
  TextFlags_None = 0,
  TextFlags_NoWidthForLargeClippedText = 1 << 0,
};

enum TooltipFlags_ {
  TooltipFlags_None = 0,
  TooltipFlags_OverridePrevious =
      1 << 1, // Clear/ignore previously submitted tooltip (defaults to append)
};

// FIXME: this is in development, not exposed/functional as a generic feature
// yet. Horizontal/Vertical enums are fixed to 0/1 so they may be used to index
// Vec2
enum LayoutType_ { LayoutType_Horizontal = 0, LayoutType_Vertical = 1 };

enum LogType {
  LogType_None = 0,
  LogType_TTY,
  LogType_File,
  LogType_Buffer,
  LogType_Clipboard,
};

// X/Y enums are fixed to 0/1 so they may be used to index Vec2
enum Axis { Axis_None = -1, Axis_X = 0, Axis_Y = 1 };

enum PlotType {
  PlotType_Lines,
  PlotType_Histogram,
};

enum PopupPositionPolicy {
  PopupPositionPolicy_Default,
  PopupPositionPolicy_ComboBox,
  PopupPositionPolicy_Tooltip,
};

struct DataVarInfo {
  int Type;
  unsigned int Count;  // 1+
  unsigned int Offset; // Offset in parent structure
  void *GetVarPtr(void *parent) const {
    return (void *)((unsigned char *)parent + Offset);
  }
};

struct DataTypeTempStorage {
  unsigned char Data[8]; // Can fit any data up to DataType_COUNT
};

// Type information associated to one DataType. Retrieve with
// DataTypeGetInfo().
struct DataTypeInfo {
  size_t Size;          // Size in bytes
  const char *Name;     // Short descriptive name for the type, for debugging
  const char *PrintFmt; // Default printf format for the type
  const char *ScanFmt;  // Default scanf format for the type
};

// Extend DataType_
enum DataTypePrivate_ {
  DataType_String = DataType_COUNT + 1,
  DataType_Pointer,
  DataType_ID,
};

// Stacked color modifier, backup of modified data so we can restore it
struct ColorMod {
  int Col;
  Vec4 BackupValue;
};

// Stacked style modifier, backup of modified data so we can restore it. Data
// type inferred from the variable.
struct StyleMod {
  int VarIdx;
  union {
    int BackupInt[2];
    float BackupFloat[2];
  };
  StyleMod(int idx, int v) {
    VarIdx = idx;
    BackupInt[0] = v;
  }
  StyleMod(int idx, float v) {
    VarIdx = idx;
    BackupFloat[0] = v;
  }
  StyleMod(int idx, Vec2 v) {
    VarIdx = idx;
    BackupFloat[0] = v.x;
    BackupFloat[1] = v.y;
  }
};

// Storage data for BeginComboPreview()/EndComboPreview()
struct API ComboPreviewData {
  Rect PreviewRect;
  Vec2 BackupCursorPos;
  Vec2 BackupCursorMaxPos;
  Vec2 BackupCursorPosPrevLine;
  float BackupPrevLineTextBaseOffset;
  int BackupLayout;

  ComboPreviewData() { memset(this, 0, sizeof(*this)); }
};

// Stacked storage data for BeginGroup()/EndGroup()
struct API GroupData {
  int WindowID;
  Vec2 BackupCursorPos;
  Vec2 BackupCursorMaxPos;
  Vec2 BackupCursorPosPrevLine;
  Vec1 BackupIndent;
  Vec1 BackupGroupOffset;
  Vec2 BackupCurrLineSize;
  float BackupCurrLineTextBaseOffset;
  int BackupActiveIdIsAlive;
  bool BackupActiveIdPreviousFrameIsAlive;
  bool BackupHoveredIdIsAlive;
  bool BackupIsSameLine;
  bool EmitItem;
};

// Simple column measurement, currently used for MenuItem() only.. This is very
// short-sighted/throw-away code and NOT a generic helper.
struct API MenuColumns {
  unsigned int TotalWidth;
  unsigned int NextTotalWidth;
  unsigned short Spacing;
  unsigned short OffsetIcon;  // Always zero for now
  unsigned short OffsetLabel; // Offsets are locked in Update()
  unsigned short OffsetShortcut;
  unsigned short OffsetMark;
  unsigned short Widths[4]; // Width of:   Icon, Label, Shortcut, Mark
                            // (accumulators for current frame)

  MenuColumns() { memset(this, 0, sizeof(*this)); }
  void Update(float spacing, bool window_reappearing);
  float DeclColumns(float w_icon, float w_label, float w_shortcut,
                    float w_mark);
  void CalcNextTotalWidth(bool update_offsets);
};

// Internal temporary state for deactivating InputText() instances.
struct API InputTextDeactivatedState {
  int ID;             // widget id owning the text state (which just got
                      // deactivated)
  Vector<char> TextA; // text buffer

  InputTextDeactivatedState() { memset(this, 0, sizeof(*this)); }
  void ClearFreeMemory() {
    ID = 0;
    TextA.clear();
  }
};
// Internal state of the currently focused/edited text input box
// For a given item unsigned int, access with Gui::GetInputTextState()
struct API InputTextState {
  Context *Ctx; // parent UI context (needs to be set explicitly by parent).
  int ID;       // widget id owning the text state
  int CurLenW,
      CurLenA; // we need to maintain our buffer length in both UTF-8 and wchar
               // format. UTF-8 length is valid even if TextA is not.
  Vector<Wchar> TextW; // edit buffer, we need to persist but can't
                       // guarantee the persistence of the user-provided
                       // buffer. so we copy into own buffer.
  Vector<char>
      TextA; // temporary UTF8 buffer for callbacks and other operations. this
             // is not updated in every code-path! size=capacity.
  Vector<char> InitialTextA; // backup of end-user buffer at the time of focus
                             // (in UTF-8, unaltered)
  bool TextAIsValid; // temporary UTF8 buffer is not initially valid before we
                     // make the widget active (until then we pull the data from
                     // user argument)
  int BufCapacityA;  // end-user buffer capacity
  float ScrollX;     // horizontal scrolling/offset
  Gui::TexteditState Stb; // state for textedit.h
  float CursorAnim; // timer for cursor blink, reset on every user action so the
                    // cursor reappears immediately
  bool CursorFollow; // set when we want scrolling to follow the current cursor
                     // position (not always!)
  bool SelectedAllMouseLock; // after a double-click to select all, we ignore
                             // further mouse drags to update selection
  bool Edited;               // edited this frame
  int Flags;                 // copy of InputText() flags. may be used to check
                             // if e.g. InputTextFlags_Password is set.

  InputTextState() { memset(this, 0, sizeof(*this)); }
  void ClearText() {
    CurLenW = CurLenA = 0;
    TextW[0] = 0;
    TextA[0] = 0;
    CursorClamp();
  }
  void ClearFreeMemory() {
    TextW.clear();
    TextA.clear();
    InitialTextA.clear();
  }
  int GetUndoAvailCount() const { return Stb.undostate.undo_point; }
  int GetRedoAvailCount() const {
    return TEXTEDIT_UNDOSTATECOUNT - Stb.undostate.redo_point;
  }
  void OnKeyPressed(int key); // Cannot be inline because we call in code in
                              // textedit.h implementation

  // Cursor & Selection
  void CursorAnimReset() {
    CursorAnim = -0.30f;
  } // After a user-input the cursor stays on for a while without blinking
  void CursorClamp() {
    Stb.cursor = Min(Stb.cursor, CurLenW);
    Stb.select_start = Min(Stb.select_start, CurLenW);
    Stb.select_end = Min(Stb.select_end, CurLenW);
  }
  bool HasSelection() const { return Stb.select_start != Stb.select_end; }
  void ClearSelection() { Stb.select_start = Stb.select_end = Stb.cursor; }
  int GetCursorPos() const { return Stb.cursor; }
  int GetSelectionStart() const { return Stb.select_start; }
  int GetSelectionEnd() const { return Stb.select_end; }
  void SelectAll() {
    Stb.select_start = 0;
    Stb.cursor = Stb.select_end = CurLenW;
    Stb.has_preferred_x = 0;
  }
};

// Storage for current popup stack
struct PopupData {
  int PopupId;    // Set on OpenPopup()
  Window *Window; // Resolved on BeginPopup() - may stay unresolved if user
                  // never calls OpenPopup()
  struct Window *BackupNavWindow; // Set on OpenPopup(), a NavWindow that will
                                  // be restored on popup close
  int ParentNavLayer; // Resolved on BeginPopup(). Actually a NavLayer type
                      // (declared down below), initialized to -1 which is not
                      // part of an enum, but serves well-enough as "not any of
                      // layers" value
  int OpenFrameCount; // Set on OpenPopup()
  int OpenParentId;   // Set on OpenPopup(), we need this to differentiate
                      // multiple menu sets from each others (e.g. inside menu
                      // bar vs loose menu items)
  Vec2 OpenPopupPos;  // Set on OpenPopup(), preferred popup position
                      // (typically == OpenMousePos when using mouse)
  Vec2 OpenMousePos;  // Set on OpenPopup(), copy of mouse position at the time
                      // of opening popup

  PopupData() {
    memset(this, 0, sizeof(*this));
    ParentNavLayer = OpenFrameCount = -1;
  }
};

enum NextWindowDataFlags_ {
  NextWindowDataFlags_None = 0,
  NextWindowDataFlags_HasPos = 1 << 0,
  NextWindowDataFlags_HasSize = 1 << 1,
  NextWindowDataFlags_HasContentSize = 1 << 2,
  NextWindowDataFlags_HasCollapsed = 1 << 3,
  NextWindowDataFlags_HasSizeConstraint = 1 << 4,
  NextWindowDataFlags_HasFocus = 1 << 5,
  NextWindowDataFlags_HasBgAlpha = 1 << 6,
  NextWindowDataFlags_HasScroll = 1 << 7,
  NextWindowDataFlags_HasChildFlags = 1 << 8,
  NextWindowDataFlags_HasViewport = 1 << 9,
  NextWindowDataFlags_HasDock = 1 << 10,
  NextWindowDataFlags_HasWindowClass = 1 << 11,
};

// Storage for SetNexWindow** functions
struct NextWindowData {
  int Flags;
  int PosCond;
  int SizeCond;
  int CollapsedCond;
  int DockCond;
  Vec2 PosVal;
  Vec2 PosPivotVal;
  Vec2 SizeVal;
  Vec2 ContentSizeVal;
  Vec2 ScrollVal;
  int ChildFlags;
  bool PosUndock;
  bool CollapsedVal;
  Rect SizeConstraintRect;
  SizeCallback SizeCallback;
  void *SizeCallbackUserData;
  float BgAlphaVal; // Override background alpha
  int ViewportId;
  int DockId;
  WindowClass WindowClass;
  Vec2 MenuBarOffsetMinVal; // (Always on) This is not exposed publicly, so we
                            // don't clear it and it doesn't have a
                            // corresponding flag (could we? for consistency?)

  NextWindowData() { memset(this, 0, sizeof(*this)); }
  inline void ClearFlags() { Flags = NextWindowDataFlags_None; }
};

// Multi-Selection item index or identifier when using
// SetNextItemSelectionUserData()/BeginMultiSelect() (Most users are likely to
// use this store an item INDEX but this may be used to store a POINTER as
// well.)
typedef signed long long SelectionUserData;

enum NextItemDataFlags_ {
  NextItemDataFlags_None = 0,
  NextItemDataFlags_HasWidth = 1 << 0,
  NextItemDataFlags_HasOpen = 1 << 1,
};

struct NextItemData {
  int Flags;
  int ItemFlags; // Currently only tested/used for ItemFlags_AllowOverlap.
  // Non-flags members are NOT cleared by ItemAdd() meaning they are still valid
  // during NavProcessItem()
  float Width;                         // Set by SetNextItemWidth()
  SelectionUserData SelectionUserData; // Set by SetNextItemSelectionUserData()
                                       // (note that NULL/0 is a valid value, we
                                       // use -1 == SelectionUserData_Invalid to
                                       // mark invalid values)
  int OpenCond;
  bool OpenVal; // Set by SetNextItemOpen()

  NextItemData() {
    memset(this, 0, sizeof(*this));
    SelectionUserData = -1;
  }
  inline void ClearFlags() {
    Flags = NextItemDataFlags_None;
    ItemFlags = ItemFlags_None;
  } // Also cleared manually by ItemAdd()!
};

// Status storage for the last submitted item
struct LastItemData {
  int ID;
  int InFlags;         // See ItemFlags_
  int StatusFlags;     // See ItemStatusFlags_
  Rect Rect;           // Full rectangle
  struct Rect NavRect; // Navigation scoring rectangle (not displayed)
  // Rarely used fields are not explicitly cleared, only valid when the
  // corresponding int is set.
  struct Rect DisplayRect; // Display rectangle (ONLY VALID IF
                           // ItemStatusFlags_HasDisplayRect is set)
  struct Rect ClipRect; // Clip rectangle at the time of submitting item (ONLY
                        // VALID IF ItemStatusFlags_HasClipRect is set)

  LastItemData() { memset(this, 0, sizeof(*this)); }
};

// Store data emitted by TreeNode() for usage by TreePop() to implement
// TreeNodeFlags_NavLeftJumpsBackHere. This is the minimum amount of data
// that we need to perform the equivalent of NavApplyItemToResult() and which we
// can't infer in TreePop() Only stored when the node is a potential candidate
// for landing on a Left arrow jump.
struct NavTreeNodeData {
  int ID;
  int InFlags;
  Rect NavRect;
};

struct API StackSizes {
  short SizeOfIDStack;
  short SizeOfColorStack;
  short SizeOfStyleVarStack;
  short SizeOfFontStack;
  short SizeOfFocusScopeStack;
  short SizeOfGroupStack;
  short SizeOfItemFlagsStack;
  short SizeOfBeginPopupStack;
  short SizeOfDisabledStack;

  StackSizes() { memset(this, 0, sizeof(*this)); }
  void SetToContextState(Context *ctx);
  void CompareWithContextState(Context *ctx);
};

// Data saved for each window pushed into the stack
struct WindowStackData {
  Window *Window;
  LastItemData ParentLastItemDataBackup;
  StackSizes StackSizesOnBegin; // Store size of various stacks for asserting
};

struct ShrinkWidthItem {
  int Index;
  float Width;
  float InitialWidth;
};

struct PtrOrIndex {
  void *Ptr; // Either field can be set, not both. e.g. Dock node tab bars are
             // loose while BeginTabBar() ones are in a pool.
  int Index; // Usually index in a main pool.

  PtrOrIndex(void *ptr) {
    Ptr = ptr;
    Index = -1;
  }
  PtrOrIndex(int index) {
    Ptr = NULL;
    Index = index;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Inputs support
//-----------------------------------------------------------------------------

// Bit array for named keys
typedef BitArray<Key_NamedKey_COUNT, -Key_NamedKey_BEGIN> BitArrayForNamedKeys;

// [Internal] Key ranges
#define Key_LegacyNativeKey_BEGIN 0
#define Key_LegacyNativeKey_END 512
#define Key_Keyboard_BEGIN (Key_NamedKey_BEGIN)
#define Key_Keyboard_END (Key_GamepadStart)
#define Key_Gamepad_BEGIN (Key_GamepadStart)
#define Key_Gamepad_END (Key_GamepadRStickDown + 1)
#define Key_Mouse_BEGIN (Key_MouseLeft)
#define Key_Mouse_END (Key_MouseWheelY + 1)
#define Key_Aliases_BEGIN (Key_Mouse_BEGIN)
#define Key_Aliases_END (Key_Mouse_END)

// [Internal] Named shortcuts for Navigation
#define Key_NavKeyboardTweakSlow Mod_Ctrl
#define Key_NavKeyboardTweakFast Mod_Shift
#define Key_NavGamepadTweakSlow Key_GamepadL1
#define Key_NavGamepadTweakFast Key_GamepadR1
#define Key_NavGamepadActivate Key_GamepadFaceDown
#define Key_NavGamepadCancel Key_GamepadFaceRight
#define Key_NavGamepadMenu Key_GamepadFaceLeft
#define Key_NavGamepadInput Key_GamepadFaceUp

enum InputEventType {
  InputEventType_None = 0,
  InputEventType_MousePos,
  InputEventType_MouseWheel,
  InputEventType_MouseButton,
  InputEventType_MouseViewport,
  InputEventType_Key,
  InputEventType_Text,
  InputEventType_Focus,
  InputEventType_COUNT
};

enum InputSource {
  InputSource_None = 0,
  InputSource_Mouse, // Note: may be Mouse or TouchScreen or Pen. See
                     // io.MouseSource to distinguish them.
  InputSource_Keyboard,
  InputSource_Gamepad,
  InputSource_Clipboard, // Currently only used by InputText()
  InputSource_COUNT
};

// FIXME: Structures in the union below need to be declared as anonymous unions
// appears to be an extension? Using Vec2() would fail on Clang 'union member
// 'MousePos' has a non-trivial default constructor'
struct InputEventMousePos {
  float PosX, PosY;
  MouseSource MouseSource;
};
struct InputEventMouseWheel {
  float WheelX, WheelY;
  MouseSource MouseSource;
};
struct InputEventMouseButton {
  int Button;
  bool Down;
  MouseSource MouseSource;
};
struct InputEventMouseViewport {
  int HoveredViewportID;
};
struct InputEventKey {
  Key Key;
  bool Down;
  float AnalogValue;
};
struct InputEventText {
  unsigned int Char;
};
struct InputEventAppFocused {
  bool Focused;
};

struct InputEvent {
  InputEventType Type;
  InputSource Source;
  unsigned int EventId; // Unique, sequential increasing integer to identify an
                        // event (if you need to correlate them to other data).
  union {
    InputEventMousePos MousePos;       // if Type == InputEventType_MousePos
    InputEventMouseWheel MouseWheel;   // if Type == InputEventType_MouseWheel
    InputEventMouseButton MouseButton; // if Type == InputEventType_MouseButton
    InputEventMouseViewport
        MouseViewport;               // if Type == InputEventType_MouseViewport
    InputEventKey Key;               // if Type == InputEventType_Key
    InputEventText Text;             // if Type == InputEventType_Text
    InputEventAppFocused AppFocused; // if Type == InputEventType_Focus
  };
  bool AddedByTestEngine;

  InputEvent() { memset(this, 0, sizeof(*this)); }
};

// Input function taking an 'unsigned int owner_id' argument defaults to
// (KeyOwner_Any == 0) aka don't test ownership, which matches legacy
// behavior.
#define KeyOwner_Any                                                           \
  ((unsigned int)0) // Accept key that have an owner, UNLESS a call to
                    // SetKeyOwner()
                    // explicitly used InputFlags_LockThisFrame or
                    // InputFlags_LockUntilRelease.
#define KeyOwner_None ((unsigned int)-1) // Require key to have no owner.

typedef signed short KeyRoutingIndex;

// Routing table entry (sizeof() == 16 bytes)
struct KeyRoutingData {
  KeyRoutingIndex NextEntryIndex;
  unsigned short Mods; // Technically we'd only need 4-bits but for simplify we
                       // store Mod_ values which need 16-bits. Mod_Shortcut is
                       // already translated to Ctrl/Super.
  unsigned char RoutingNextScore; // Lower is better (0: perfect score)
  int RoutingCurr;
  int RoutingNext;

  KeyRoutingData() {
    NextEntryIndex = -1;
    Mods = 0;
    RoutingNextScore = 255;
    RoutingCurr = RoutingNext = KeyOwner_None;
  }
};

// Routing table: maintain a desired owner for each possible key-chord (key +
// mods), and setup owner in NewFrame() when mods are matching. Stored in main
// context (1 instance)
struct KeyRoutingTable {
  KeyRoutingIndex
      Index[Key_NamedKey_COUNT]; // Index of first entry in Entries[]
  Vector<KeyRoutingData> Entries;
  Vector<KeyRoutingData> EntriesNext; // Double-buffer to avoid reallocation
                                      // (could use a shared buffer)

  KeyRoutingTable() { Clear(); }
  void Clear() {
    for (int n = 0; n < ARRAYSIZE(Index); n++)
      Index[n] = -1;
    Entries.clear();
    EntriesNext.clear();
  }
};

// This extends KeyData but only for named keys (legacy keys don't support
// the new features) Stored in main context (1 per named key). In the future it
// might be merged into KeyData.
struct KeyOwnerData {
  int OwnerCurr;
  int OwnerNext;
  bool LockThisFrame; // Reading this key requires explicit owner id (until end
                      // of frame). Set by InputFlags_LockThisFrame.
  bool LockUntilRelease; // Reading this key requires explicit owner id (until
                         // key is released). Set by
                         // InputFlags_LockUntilRelease. When this is true
                         // LockThisFrame is always true as well.

  KeyOwnerData() {
    OwnerCurr = OwnerNext = KeyOwner_None;
    LockThisFrame = LockUntilRelease = false;
  }
};

// Flags for extended versions of IsKeyPressed(), IsMouseClicked(), Shortcut(),
// SetKeyOwner(), SetItemKeyOwner() Don't mistake with InputTextFlags! (for
// Gui::InputText() function)
enum InputFlags_ {
  // Flags for IsKeyPressed(), IsMouseClicked(), Shortcut()
  InputFlags_None = 0,
  InputFlags_Repeat = 1 << 0, // Return true on successive repeats. Default
                              // for legacy IsKeyPressed(). NOT Default for
                              // legacy IsMouseClicked(). MUST BE == 1.
  InputFlags_RepeatRateDefault = 1 << 1,  // Repeat rate: Regular (default)
  InputFlags_RepeatRateNavMove = 1 << 2,  // Repeat rate: Fast
  InputFlags_RepeatRateNavTweak = 1 << 3, // Repeat rate: Faster
  InputFlags_RepeatRateMask_ = InputFlags_RepeatRateDefault |
                               InputFlags_RepeatRateNavMove |
                               InputFlags_RepeatRateNavTweak,

  // Flags for SetItemKeyOwner()
  InputFlags_CondHovered =
      1 << 4, // Only set if item is hovered (default to both)
  InputFlags_CondActive = 1
                          << 5, // Only set if item is active (default to both)
  InputFlags_CondDefault_ = InputFlags_CondHovered | InputFlags_CondActive,
  InputFlags_CondMask_ = InputFlags_CondHovered | InputFlags_CondActive,

  // Flags for SetKeyOwner(), SetItemKeyOwner()
  InputFlags_LockThisFrame =
      1 << 6, // Access to key data will require EXPLICIT owner unsigned int
              // (KeyOwner_Any/0 will NOT accepted for polling). Cleared at
              // end of frame. This is useful to make input-owner-aware code
              // steal keys from non-input-owner-aware code.
  InputFlags_LockUntilRelease =
      1 << 7, // Access to key data will require EXPLICIT owner unsigned int
              // (KeyOwner_Any/0 will NOT accepted for polling). Cleared
              // when the key is released or at end of each frame if key is
              // released. This is useful to make input-owner-aware code steal
              // keys from non-input-owner-aware code.

  // Routing policies for Shortcut() + low-level SetShortcutRouting()
  // - The general idea is that several callers register interest in a shortcut,
  // and only one owner gets it.
  // - When a policy (other than _RouteAlways) is set, Shortcut() will register
  // itself with SetShortcutRouting(),
  //   allowing the system to decide where to route the input among other
  //   route-aware calls.
  // - Shortcut() uses InputFlags_RouteFocused by default: meaning that a
  // simple Shortcut() poll
  //   will register a route and only succeed when parent window is in the focus
  //   stack and if no-one with a higher priority is claiming the shortcut.
  // - Using InputFlags_RouteAlways is roughly equivalent to doing e.g.
  // IsKeyPressed(key) + testing mods.
  // - Priorities: GlobalHigh > Focused (when owner is active item) > Global >
  // Focused (when focused window) > GlobalLow.
  // - Can select only 1 policy among all available.
  InputFlags_RouteFocused =
      1 << 8, // (Default) Register focused route: Accept inputs if window is in
              // focus stack. Deep-most focused window takes inputs. ActiveId
              // takes inputs over deep-most focused window.
  InputFlags_RouteGlobalLow =
      1 << 9, // Register route globally (lowest priority: unless a focused
              // window or active item registered the route) -> recommended
              // Global priority.
  InputFlags_RouteGlobal =
      1
      << 10, // Register route globally (medium priority: unless an active item
             // registered the route, e.g. CTRL+A registered by InputText).
  InputFlags_RouteGlobalHigh =
      1 << 11, // Register route globally (highest priority: unlikely you need
               // to use that: will interfere with every active items)
  InputFlags_RouteMask_ =
      InputFlags_RouteFocused | InputFlags_RouteGlobal |
      InputFlags_RouteGlobalLow |
      InputFlags_RouteGlobalHigh, // _Always not part of this!
  InputFlags_RouteAlways = 1
                           << 12, // Do not register route, poll keys directly.
  InputFlags_RouteUnlessBgFocused =
      1 << 13, // Global routes will not be applied if underlying
               // background/void is focused (== no Gui windows are
               // focused). Useful for overlay applications.
  InputFlags_RouteExtraMask_ =
      InputFlags_RouteAlways | InputFlags_RouteUnlessBgFocused,

  // [Internal] Mask of which function support which flags
  InputFlags_SupportedByIsKeyPressed =
      InputFlags_Repeat | InputFlags_RepeatRateMask_,
  InputFlags_SupportedByShortcut =
      InputFlags_Repeat | InputFlags_RepeatRateMask_ | InputFlags_RouteMask_ |
      InputFlags_RouteExtraMask_,
  InputFlags_SupportedBySetKeyOwner =
      InputFlags_LockThisFrame | InputFlags_LockUntilRelease,
  InputFlags_SupportedBySetItemKeyOwner =
      InputFlags_SupportedBySetKeyOwner | InputFlags_CondMask_,
};

//-----------------------------------------------------------------------------
// [SECTION] Clipper support
//-----------------------------------------------------------------------------

// Note that Max is exclusive, so perhaps should be using a Begin/End
// convention.
struct ListClipperRange {
  int Min;
  int Max;
  bool PosToIndexConvert; // Begin/End are absolute position (will be converted
                          // to indices later)
  signed char PosToIndexOffsetMin; // Add to Min after converting to indices
  signed char PosToIndexOffsetMax; // Add to Min after converting to indices

  static ListClipperRange FromIndices(int min, int max) {
    ListClipperRange r = {min, max, false, 0, 0};
    return r;
  }
  static ListClipperRange FromPositions(float y1, float y2, int off_min,
                                        int off_max) {
    ListClipperRange r = {(int)y1, (int)y2, true, (signed char)off_min,
                          (signed char)off_max};
    return r;
  }
};

// Temporary clipper data, buffers shared/reused between instances
struct ListClipperData {
  ListClipper *ListClipper;
  float LossynessOffset;
  int StepNo;
  int ItemsFrozen;
  Vector<ListClipperRange> Ranges;

  ListClipperData() { memset(this, 0, sizeof(*this)); }
  void Reset(::ListClipper *clipper) {
    ListClipper = clipper;
    StepNo = ItemsFrozen = 0;
    Ranges.resize(0);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Navigation support
//-----------------------------------------------------------------------------

enum ActivateFlags_ {
  ActivateFlags_None = 0,
  ActivateFlags_PreferInput =
      1 << 0, // Favor activation that requires keyboard text input (e.g. for
              // Slider/Drag). Default for Enter key.
  ActivateFlags_PreferTweak =
      1
      << 1, // Favor activation for tweaking with arrows or gamepad (e.g. for
            // Slider/Drag). Default for Space key and if keyboard is not used.
  ActivateFlags_TryToPreserveState =
      1 << 2, // Request widget to preserve state if it can (e.g. InputText will
              // try to preserve cursor/selection)
  ActivateFlags_FromTabbing = 1
                              << 3, // Activation requested by a tabbing request
};

// Early work-in-progress API for ScrollToItem()
enum ScrollFlags_ {
  ScrollFlags_None = 0,
  ScrollFlags_KeepVisibleEdgeX =
      1 << 0, // If item is not visible: scroll as little as possible on X axis
              // to bring item back into view [default for X axis]
  ScrollFlags_KeepVisibleEdgeY =
      1 << 1, // If item is not visible: scroll as little as possible on Y axis
              // to bring item back into view [default for Y axis for windows
              // that are already visible]
  ScrollFlags_KeepVisibleCenterX =
      1 << 2, // If item is not visible: scroll to make the item centered on X
              // axis [rarely used]
  ScrollFlags_KeepVisibleCenterY = 1 << 3, // If item is not visible: scroll to
                                           // make the item centered on Y axis
  ScrollFlags_AlwaysCenterX =
      1 << 4, // Always center the result item on X axis [rarely used]
  ScrollFlags_AlwaysCenterY = 1
                              << 5, // Always center the result item on Y axis
                                    // [default for Y axis for appearing window)
  ScrollFlags_NoScrollParent =
      1 << 6, // Disable forwarding scrolling to parent window if required to
              // keep item/rect visible (only scroll window the function was
              // applied to).
  ScrollFlags_MaskX_ = ScrollFlags_KeepVisibleEdgeX |
                       ScrollFlags_KeepVisibleCenterX |
                       ScrollFlags_AlwaysCenterX,
  ScrollFlags_MaskY_ = ScrollFlags_KeepVisibleEdgeY |
                       ScrollFlags_KeepVisibleCenterY |
                       ScrollFlags_AlwaysCenterY,
};

enum NavHighlightFlags_ {
  NavHighlightFlags_None = 0,
  NavHighlightFlags_TypeDefault = 1 << 0,
  NavHighlightFlags_TypeThin = 1 << 1,
  NavHighlightFlags_AlwaysDraw = 1
                                 << 2, // Draw rectangular highlight if (g.NavId
                                       // == id) _even_ when using the mouse.
  NavHighlightFlags_NoRounding = 1 << 3,
};

enum NavMoveFlags_ {
  NavMoveFlags_None = 0,
  NavMoveFlags_LoopX = 1 << 0, // On failed request, restart from opposite side
  NavMoveFlags_LoopY = 1 << 1,
  NavMoveFlags_WrapX =
      1 << 2, // On failed request, request from opposite side one line down
              // (when NavDir==right) or one line up (when NavDir==left)
  NavMoveFlags_WrapY =
      1 << 3, // This is not super useful but provided for completeness
  NavMoveFlags_WrapMask_ = NavMoveFlags_LoopX | NavMoveFlags_LoopY |
                           NavMoveFlags_WrapX | NavMoveFlags_WrapY,
  NavMoveFlags_AllowCurrentNavId =
      1 << 4, // Allow scoring and considering the current NavId as a move
              // target candidate. This is used when the move source is offset
              // (e.g. pressing PageDown actually needs to send a Up move
              // request, if we are pressing PageDown from the bottom-most item
              // we need to stay in place)
  NavMoveFlags_AlsoScoreVisibleSet =
      1 << 5, // Store alternate result in NavMoveResultLocalVisible that only
              // comprise elements that are already fully visible (used by
              // PageUp/PageDown)
  NavMoveFlags_ScrollToEdgeY =
      1 << 6, // Force scrolling to min/max (used by Home/End) // FIXME-NAV: Aim
              // to remove or reword, probably unnecessary
  NavMoveFlags_Forwarded = 1 << 7,
  NavMoveFlags_DebugNoResult =
      1 << 8, // Dummy scoring for debug purpose, don't apply result
  NavMoveFlags_FocusApi =
      1 << 9, // Requests from focus API can land/focus/activate items even if
              // they are marked with _NoTabStop (see
              // NavProcessItemForTabbingRequest() for details)
  NavMoveFlags_IsTabbing = 1 << 10,  // == Focus + Activate if item is
                                     // Inputable + DontChangeNavHighlight
  NavMoveFlags_IsPageMove = 1 << 11, // Identify a PageDown/PageUp request.
  NavMoveFlags_Activate = 1 << 12,   // Activate/select target item.
  NavMoveFlags_NoSelect =
      1 << 13, // Don't trigger selection by not setting g.NavJustMovedTo
  NavMoveFlags_NoSetNavHighlight = 1 << 14, // Do not alter the visible state of
                                            // keyboard vs mouse nav highlight
};

enum NavLayer {
  NavLayer_Main = 0, // Main scrolling layer
  NavLayer_Menu = 1, // Menu layer (access with Alt)
  NavLayer_COUNT
};

struct NavItemData {
  Window *Window;   // Init,Move    // Best candidate window
                    // (result->ItemWindow->RootWindowForNav == request->Window)
  int ID;           // Init,Move    // Best candidate item unsigned int
  int FocusScopeId; // Init,Move    // Best candidate focus scope unsigned int
  Rect RectRel;     // Init,Move    // Best candidate bounding box in window
                    // relative space
  int InFlags;      // ????,Move    // Best candidate item flags
  SelectionUserData SelectionUserData; // I+Mov    // Best candidate
                                       // SetNextItemSelectionData() value.
  float DistBox; //      Move    // Best candidate box distance to current NavId
  float DistCenter; //      Move    // Best candidate center distance to current
                    //      NavId
  float DistAxial;  //      Move    // Best candidate axial distance to current
                    //      NavId

  NavItemData() { Clear(); }
  void Clear() {
    Window = NULL;
    ID = FocusScopeId = 0;
    InFlags = 0;
    SelectionUserData = -1;
    DistBox = DistCenter = DistAxial = FLT_MAX;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Typing-select support
//-----------------------------------------------------------------------------

// Flags for GetTypingSelectRequest()
enum TypingSelectFlags_ {
  TypingSelectFlags_None = 0,
  TypingSelectFlags_AllowBackspace =
      1 << 0, // Backspace to delete character inputs. If using: ensure
              // GetTypingSelectRequest() is not called more than once per frame
              // (filter by e.g. focus state)
  TypingSelectFlags_AllowSingleCharMode =
      1 << 1, // Allow "single char" search mode which is activated when
              // pressing the same character multiple times.
};

// Returned by GetTypingSelectRequest(), designed to eventually be public.
struct API TypingSelectRequest {
  int Flags; // Flags passed to GetTypingSelectRequest()
  int SearchBufferLen;
  const char *
      SearchBuffer; // Search buffer contents (use full string. unless
                    // SingleCharMode is set, in which case use SingleCharSize).
  bool SelectRequest; // Set when buffer was modified this frame, requesting a
                      // selection.
  bool
      SingleCharMode; // Notify when buffer contains same character repeated, to
                      // implement special mode. In this situation it preferred
                      // to not display any on-screen search indication.
  signed char
      SingleCharSize; // Length in bytes of first letter codepoint (1 for ascii,
                      // 2-4 for UTF-8). If (SearchBufferLen==RepeatCharSize)
                      // only 1 letter has been input.
};

// Storage for GetTypingSelectRequest()
struct API TypingSelectState {
  TypingSelectRequest Request; // User-facing data
  char SearchBuffer[64];       // Search buffer: no need to make dynamic as this
                               // search is very transient.
  int FocusScope;
  int LastRequestFrame = 0;
  float LastRequestTime = 0.0f;
  bool SingleCharModeLock =
      false; // After a certain single char repeat count we lock into
             // SingleCharMode. Two benefits: 1) buffer never fill, 2) we can
             // provide an immediate SingleChar mode without timer elapsing.

  TypingSelectState() { memset(this, 0, sizeof(*this)); }
  void Clear() {
    SearchBuffer[0] = 0;
    SingleCharModeLock = false;
  } // We preserve remaining data for easier debugging
};

//-----------------------------------------------------------------------------
// [SECTION] Columns support
//-----------------------------------------------------------------------------

// Flags for internal's BeginColumns(). This is an obsolete API. Prefer using
// BeginTable() nowadays!
enum OldColumnFlags_ {
  OldColumnFlags_None = 0,
  OldColumnFlags_NoBorder = 1 << 0, // Disable column dividers
  OldColumnFlags_NoResize =
      1 << 1, // Disable resizing columns when clicking on the dividers
  OldColumnFlags_NoPreserveWidths =
      1 << 2, // Disable column width preservation when adjusting columns
  OldColumnFlags_NoForceWithinWindow =
      1 << 3, // Disable forcing columns to fit within window
  OldColumnFlags_GrowParentContentsSize =
      1 << 4, // Restore pre-1.51 behavior of extending the parent window
              // contents size but _without affecting the columns width at all_.
              // Will eventually remove.

// Obsolete names (will be removed)
#ifndef DISABLE_OBSOLETE_FUNCTIONS
// ColumnsFlags_None                    = OldColumnFlags_None,
// ColumnsFlags_NoBorder                = OldColumnFlags_NoBorder,
// ColumnsFlags_NoResize                = OldColumnFlags_NoResize,
// ColumnsFlags_NoPreserveWidths        =
// OldColumnFlags_NoPreserveWidths, ColumnsFlags_NoForceWithinWindow
// = OldColumnFlags_NoForceWithinWindow,
// ColumnsFlags_GrowParentContentsSize  =
// OldColumnFlags_GrowParentContentsSize,
#endif
};

struct OldColumnData {
  float OffsetNorm; // Column start offset, normalized 0.0 (far left) -> 1.0
                    // (far right)
  float OffsetNormBeforeResize;
  int Flags; // Not exposed
  Rect ClipRect;

  OldColumnData() { memset(this, 0, sizeof(*this)); }
};

struct OldColumns {
  int ID;
  int Flags;
  bool IsFirstFrame;
  bool IsBeingResized;
  int Current;
  int Count;
  float OffMinX, OffMaxX; // Offsets from HostWorkRect.Min.x
  float LineMinY, LineMaxY;
  float HostCursorPosY; // Backup of CursorPos at the time of BeginColumns()
  float
      HostCursorMaxPosX; // Backup of CursorMaxPos at the time of BeginColumns()
  Rect HostInitialClipRect; // Backup of ClipRect at the time of BeginColumns()
  Rect HostBackupClipRect;  // Backup of ClipRect during
                            // PushColumnsBackground()/PopColumnsBackground()
  Rect HostBackupParentWorkRect; // Backup of WorkRect at the time of
                                 // BeginColumns()
  Vector<OldColumnData> Columns;
  DrawListSplitter Splitter;

  OldColumns() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Multi-select support
//-----------------------------------------------------------------------------

// We always assume that -1 is an invalid value (which works for indices and
// pointers)
#define SelectionUserData_Invalid ((SelectionUserData)-1)

#ifdef HAS_MULTI_SELECT
// <this is filled in 'range_select' branch>
#endif // #ifdef HAS_MULTI_SELECT

//-----------------------------------------------------------------------------
// [SECTION] Docking support
//-----------------------------------------------------------------------------

#define DOCKING_HOST_DRAW_CHANNEL_BG 0 // Dock host: background fill
#define DOCKING_HOST_DRAW_CHANNEL_FG 1 // Dock host: decorations and contents

#ifdef HAS_DOCK

// Extend DockNodeFlags_
enum DockNodeFlagsPrivate_ {
  // [Internal]
  DockNodeFlags_DockSpace =
      1 << 10, // Saved // A dockspace is a node that occupy space within an
               // existing user window. Otherwise the node is floating and
               // create its own window.
  DockNodeFlags_CentralNode =
      1 << 11, // Saved // The central node has 2 main properties: stay visible
               // when empty, only use "remaining" spaces from its neighbor.
  DockNodeFlags_NoTabBar =
      1 << 12, // Saved // Tab bar is completely unavailable. No triangle in the
               // corner to enable it back.
  DockNodeFlags_HiddenTabBar =
      1 << 13, // Saved // Tab bar is hidden, with a triangle in the corner to
               // show it again (NB: actual tab-bar instance may be destroyed as
               // this is only used for single-window tab bar)
  DockNodeFlags_NoWindowMenuButton =
      1 << 14, // Saved // Disable window/docking menu (that one that appears
               // instead of the collapse button)
  DockNodeFlags_NoCloseButton = 1 << 15, // Saved // Disable close button
  DockNodeFlags_NoResizeX = 1 << 16,     //       //
  DockNodeFlags_NoResizeY = 1 << 17,     //       //
  // Disable docking/undocking actions in this dockspace or
  // individual node (existing docked nodes will be preserved)
  // Those are not exposed in public because the desirable
  // sharing/inheriting/copy-flag-on-split behaviors are quite
  // difficult to design and understand. The two public flags
  // DockNodeFlags_NoDockingOverCentralNode/DockNodeFlags_NoDockingSplit
  // don't have those issues.
  DockNodeFlags_NoDockingSplitOther =
      1 << 19, //       // Disable this node from splitting other windows/nodes.
  DockNodeFlags_NoDockingOverMe =
      1 << 20, //       // Disable other windows/nodes from being docked over
               //       this node.
  DockNodeFlags_NoDockingOverOther =
      1 << 21, //       // Disable this node from being docked over another
               //       window or non-empty node.
  DockNodeFlags_NoDockingOverEmpty =
      1 << 22, //       // Disable this node from being docked over an empty
               //       node (e.g. DockSpace with no other windows)
  DockNodeFlags_NoDocking =
      DockNodeFlags_NoDockingOverMe | DockNodeFlags_NoDockingOverOther |
      DockNodeFlags_NoDockingOverEmpty | DockNodeFlags_NoDockingSplit |
      DockNodeFlags_NoDockingSplitOther,
  // Masks
  DockNodeFlags_SharedFlagsInheritMask_ = ~0,
  DockNodeFlags_NoResizeFlagsMask_ = DockNodeFlags_NoResize |
                                     DockNodeFlags_NoResizeX |
                                     DockNodeFlags_NoResizeY,
  // When splitting, those local flags are moved to the inheriting child, never
  // duplicated
  DockNodeFlags_LocalFlagsTransferMask_ =
      DockNodeFlags_NoDockingSplit | DockNodeFlags_NoResizeFlagsMask_ |
      DockNodeFlags_AutoHideTabBar | DockNodeFlags_CentralNode |
      DockNodeFlags_NoTabBar | DockNodeFlags_HiddenTabBar |
      DockNodeFlags_NoWindowMenuButton | DockNodeFlags_NoCloseButton,
  DockNodeFlags_SavedFlagsMask_ =
      DockNodeFlags_NoResizeFlagsMask_ | DockNodeFlags_DockSpace |
      DockNodeFlags_CentralNode | DockNodeFlags_NoTabBar |
      DockNodeFlags_HiddenTabBar | DockNodeFlags_NoWindowMenuButton |
      DockNodeFlags_NoCloseButton,
};

// Store the source authority (dock node vs window) of a field
enum DataAuthority_ {
  DataAuthority_Auto,
  DataAuthority_DockNode,
  DataAuthority_Window,
};

enum DockNodeState {
  DockNodeState_Unknown,
  DockNodeState_HostWindowHiddenBecauseSingleWindow,
  DockNodeState_HostWindowHiddenBecauseWindowsAreResizing,
  DockNodeState_HostWindowVisible,
};

// sizeof() 156~192
struct API DockNode {
  int ID;
  int SharedFlags; // (Write) Flags shared by all nodes of a same dockspace
                   // hierarchy (inherited from the root node)
  int LocalFlags;  // (Write) Flags specific to this node
  int LocalFlagsInWindows; // (Write) Flags specific to this
                           // node, applied from windows
  int MergedFlags;         // (Read)  Effective flags (== SharedFlags |
                           // LocalFlagsInNode | LocalFlagsInWindows)
  DockNodeState State;
  DockNode *ParentNode;
  DockNode *ChildNodes[2]; // [Split node only] Child nodes (left/right or
                           // top/bottom). Consider switching to an array.
  Vector<Window *>
      Windows; // Note: unordered list! Iterate TabBar->Tabs for user-order.
  TabBar *TabBar;
  Vec2 Pos;       // Current position
  Vec2 Size;      // Current size
  Vec2 SizeRef;   // [Split node only] Last explicitly written-to size
                  // (overridden when using a splitter affecting the node), used
                  // to calculate Size.
  Axis SplitAxis; // [Split node only] Split axis (X or Y)
  WindowClass WindowClass; // [Root node only]
  unsigned int LastBgColor;

  Window *HostWindow;
  Window *VisibleWindow; // Generally point to window which is int is ==
                         // SelectedTabID, but when CTRL+Tabbing this can
                         // be a different window.
  DockNode *CentralNode; // [Root node only] Pointer to central node.
  DockNode *OnlyNodeWithWindows; // [Root node only] Set when there is a single
                                 // visible node within the hierarchy.
  int CountNodeWithWindows;      // [Root node only]
  int LastFrameAlive;    // Last frame number the node was updated or kept alive
                         // explicitly with DockSpace() +
                         // DockNodeFlags_KeepAliveOnly
  int LastFrameActive;   // Last frame number the node was updated.
  int LastFrameFocused;  // Last frame number the node was focused.
  int LastFocusedNodeId; // [Root node only] Which of our child docking node
                         // (any ancestor in the hierarchy) was last focused.
  int SelectedTabId;  // [Leaf node only] Which of our tab/window is selected.
  int WantCloseTabId; // [Leaf node only] Set when closing a specific
                      // tab/window.
  int RefViewportId;  // Reference viewport int from visible window when
                      // HostWindow == NULL.
  int AuthorityForPos : 3;
  int AuthorityForSize : 3;
  int AuthorityForViewport : 3;
  bool IsVisible : 1; // Set to false when the node is hidden (usually disabled
                      // as it has no active window)
  bool IsFocused : 1;
  bool IsBgDrawnThisFrame : 1;
  bool HasCloseButton : 1; // Provide space for a close button (if any of the
                           // docked window has one). Note that button may be
                           // hidden on window without one.
  bool HasWindowMenuButton : 1;
  bool HasCentralNodeChild : 1;
  bool WantCloseAll : 1; // Set when closing all tabs at once.
  bool WantLockSizeOnce : 1;
  bool WantMouseMove : 1; // After a node extraction we need to transition
                          // toward moving the newly created host window
  bool WantHiddenTabBarUpdate : 1;
  bool WantHiddenTabBarToggle : 1;

  DockNode(int id);
  ~DockNode();
  bool IsRootNode() const { return ParentNode == NULL; }
  bool IsDockSpace() const {
    return (MergedFlags & DockNodeFlags_DockSpace) != 0;
  }
  bool IsFloatingNode() const {
    return ParentNode == NULL && (MergedFlags & DockNodeFlags_DockSpace) == 0;
  }
  bool IsCentralNode() const {
    return (MergedFlags & DockNodeFlags_CentralNode) != 0;
  }
  bool IsHiddenTabBar() const {
    return (MergedFlags & DockNodeFlags_HiddenTabBar) != 0;
  } // Hidden tab bar can be shown back by clicking the small triangle
  bool IsNoTabBar() const {
    return (MergedFlags & DockNodeFlags_NoTabBar) != 0;
  } // Never show a tab bar
  bool IsSplitNode() const { return ChildNodes[0] != NULL; }
  bool IsLeafNode() const { return ChildNodes[0] == NULL; }
  bool IsEmpty() const { return ChildNodes[0] == NULL && Windows.Size == 0; }
  Rect Rect() const {
    return ::Rect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
  }

  void SetLocalFlags(int flags) {
    LocalFlags = flags;
    UpdateMergedFlags();
  }
  void UpdateMergedFlags() {
    MergedFlags = SharedFlags | LocalFlags | LocalFlagsInWindows;
  }
};

// List of colors that are stored at the time of Begin() into Docked Windows.
// We currently store the packed colors in a simple array
// window->DockStyle.Colors[]. A better solution may involve appending into a
// log of colors in Context + store offsets into those arrays in
// Window, but it would be more complex as we'd need to double-buffer both
// as e.g. drop target may refer to window from last frame.
enum WindowDockStyleCol {
  WindowDockStyleCol_Text,
  WindowDockStyleCol_Tab,
  WindowDockStyleCol_TabHovered,
  WindowDockStyleCol_TabActive,
  WindowDockStyleCol_TabUnfocused,
  WindowDockStyleCol_TabUnfocusedActive,
  WindowDockStyleCol_COUNT
};

struct WindowDockStyle {
  unsigned int Colors[WindowDockStyleCol_COUNT];
};

struct DockContext {
  Storage Nodes; // Map int -> DockNode*: Active nodes
  Vector<DockRequest> Requests;
  Vector<DockNodeSettings> NodesSettings;
  bool WantFullRebuild;
  DockContext() { memset(this, 0, sizeof(*this)); }
};

#endif // #ifdef HAS_DOCK

//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// Viewport Private/Internals fields (cardinal sin: we are using
// inheritance!) Every instance of Viewport is in fact a ViewportP.
struct ViewportP : public Viewport {
  Window *Window; // Set when the viewport is owned by a window (and
                  // ViewportFlags_CanHostOtherWindows is NOT set)
  int Idx;
  int LastFrameActive; // Last frame number this viewport was activated by a
                       // window
  int LastFocusedStampCount; // Last stamp number from when a window hosted by
                             // this viewport was focused (by comparing this
                             // value between two viewport we have an implicit
                             // viewport z-order we use as fallback)
  int LastNameHash;
  Vec2 LastPos;
  float Alpha; // Window opacity (when dragging dockable windows/viewports we
               // make them transparent)
  float LastAlpha;
  bool LastFocusedHadNavWindow; // Instead of maintaining a LastFocusedWindow
                                // (which may harder to correctly maintain), we
                                // merely store weither NavWindow != NULL last
                                // time the viewport was focused.
  short PlatformMonitor;
  int BgFgDrawListsLastFrame[2]; // Last frame number the background (0) and
                                 // foreground (1) draw lists were used
  DrawList *BgFgDrawLists[2]; // Convenience background (0) and foreground (1)
                              // draw lists. We use them to draw software
                              // mouser cursor when io.MouseDrawCursor is set
                              // and to draw most debug overlays.
  struct DrawData DrawDataP;
  DrawDataBuilder
      DrawDataBuilder; // Temporary data while building final DrawData
  Vec2 LastPlatformPos;
  Vec2 LastPlatformSize;
  Vec2 LastRendererSize;
  Vec2
      WorkOffsetMin; // Work Area: Offset from Pos to top-left corner of Work
                     // Area. Generally (0,0) or (0,+main_menu_bar_height). Work
                     // Area is Full Area but without menu-bars/status-bars (so
                     // WorkArea always fit inside Pos/Size!)
  Vec2
      WorkOffsetMax; // Work Area: Offset from Pos+Size to bottom-right corner
                     // of Work Area. Generally (0,0) or (0,-status_bar_height).
  Vec2 BuildWorkOffsetMin; // Work Area: Offset being built during current
                           // frame. Generally >= 0.0f.
  Vec2 BuildWorkOffsetMax; // Work Area: Offset being built during current
                           // frame. Generally <= 0.0f.

  ViewportP() {
    Window = NULL;
    Idx = -1;
    LastFrameActive = BgFgDrawListsLastFrame[0] = BgFgDrawListsLastFrame[1] =
        LastFocusedStampCount = -1;
    LastNameHash = 0;
    Alpha = LastAlpha = 1.0f;
    LastFocusedHadNavWindow = false;
    PlatformMonitor = -1;
    BgFgDrawLists[0] = BgFgDrawLists[1] = NULL;
    LastPlatformPos = LastPlatformSize = LastRendererSize =
        Vec2(FLT_MAX, FLT_MAX);
  }
  ~ViewportP() {
    if (BgFgDrawLists[0])
      DELETE(BgFgDrawLists[0]);
    if (BgFgDrawLists[1])
      DELETE(BgFgDrawLists[1]);
  }
  void ClearRequestFlags() {
    PlatformRequestClose = PlatformRequestMove = PlatformRequestResize = false;
  }

  // Calculate work rect pos/size given a set of offset (we have 1 pair of
  // offset for rect locked from last frame data, and 1 pair for currently
  // building rect)
  Vec2 CalcWorkRectPos(const Vec2 &off_min) const {
    return Vec2(Pos.x + off_min.x, Pos.y + off_min.y);
  }
  Vec2 CalcWorkRectSize(const Vec2 &off_min, const Vec2 &off_max) const {
    return Vec2(Max(0.0f, Size.x - off_min.x + off_max.x),
                Max(0.0f, Size.y - off_min.y + off_max.y));
  }
  void UpdateWorkRect() {
    WorkPos = CalcWorkRectPos(WorkOffsetMin);
    WorkSize = CalcWorkRectSize(WorkOffsetMin, WorkOffsetMax);
  } // Update public fields

  // Helpers to retrieve Rect (we don't need to store BuildWorkRect as every
  // access tend to change it, hence the code asymmetry)
  Rect GetMainRect() const {
    return Rect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
  }
  Rect GetWorkRect() const {
    return Rect(WorkPos.x, WorkPos.y, WorkPos.x + WorkSize.x,
                WorkPos.y + WorkSize.y);
  }
  Rect GetBuildWorkRect() const {
    Vec2 pos = CalcWorkRectPos(BuildWorkOffsetMin);
    Vec2 size = CalcWorkRectSize(BuildWorkOffsetMin, BuildWorkOffsetMax);
    return Rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Settings support
//-----------------------------------------------------------------------------

// Windows data saved in gui.ini file
// Because we never destroy or rename WindowSettings, we can store the
// names in a separate buffer easily. (this is designed to be stored in a
// ChunkStream buffer, with the variable-length Name following our structure)
struct WindowSettings {
  int ID;
  Vec2ih Pos; // NB: Settings position are stored RELATIVE to the viewport!
              // Whereas runtime ones are absolute positions.
  Vec2ih Size;
  Vec2ih ViewportPos;
  int ViewportId;
  int DockId;  // int of last known DockNode (even if the DockNode is invisible
               // because it has only 1 active window), or 0 if none.
  int ClassId; // int of window class if specified
  short DockOrder; // Order of the last time the window was visible within its
                   // DockNode. This is used to reorder windows that are
                   // reappearing on the same frame. Same value between windows
                   // that were active and windows that were none are possible.
  bool Collapsed;
  bool IsChild;
  bool WantApply;  // Set when loaded from .ini data (to enable merging/loading
                   // .ini data into an already running context)
  bool WantDelete; // Set to invalidate/delete the settings entry

  WindowSettings() {
    memset(this, 0, sizeof(*this));
    DockOrder = -1;
  }
  char *GetName() { return (char *)(this + 1); }
};

struct SettingsHandler {
  const char *TypeName; // Short description stored in .ini file. Disallowed
                        // characters: '[' ']'
  int TypeHash;         // == HashStr(TypeName)
  void (*ClearAllFn)(Context *ctx,
                     SettingsHandler *handler); // Clear all settings data
  void (*ReadInitFn)(Context *ctx,
                     SettingsHandler *handler); // Read: Called before reading
                                                // (in registration order)
  void *(*ReadOpenFn)(Context *ctx, SettingsHandler *handler,
                      const char *name); // Read: Called when entering into a
                                         // new ini entry e.g. "[Window][Name]"
  void (*ReadLineFn)(Context *ctx, SettingsHandler *handler, void *entry,
                     const char *line); // Read: Called for every line of text
                                        // within an ini entry
  void (*ApplyAllFn)(Context *ctx,
                     SettingsHandler *handler); // Read: Called after reading
                                                // (in registration order)
  void (*WriteAllFn)(
      Context *ctx, SettingsHandler *handler,
      TextBuffer *out_buf); // Write: Output every entries into 'out_buf'
  void *UserData;

  SettingsHandler() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Localization support
//-----------------------------------------------------------------------------

// This is experimental and not officially supported, it'll probably fall short
// of features, if/when it does we may backtrack.
enum LocKey : int {
  LocKey_VersionStr,
  LocKey_TableSizeOne,
  LocKey_TableSizeAllFit,
  LocKey_TableSizeAllDefault,
  LocKey_TableResetOrder,
  LocKey_WindowingMainMenuBar,
  LocKey_WindowingPopup,
  LocKey_WindowingUntitled,
  LocKey_DockingHideTabBar,
  LocKey_DockingHoldShiftToDock,
  LocKey_DockingDragToUndockOrMoveNode,
  LocKey_COUNT
};

struct LocEntry {
  LocKey Key;
  const char *Text;
};

//-----------------------------------------------------------------------------
// [SECTION] Metrics, Debug Tools
//-----------------------------------------------------------------------------

enum DebugLogFlags_ {
  // Event types
  DebugLogFlags_None = 0,
  DebugLogFlags_EventActiveId = 1 << 0,
  DebugLogFlags_EventFocus = 1 << 1,
  DebugLogFlags_EventPopup = 1 << 2,
  DebugLogFlags_EventNav = 1 << 3,
  DebugLogFlags_EventClipper = 1 << 4,
  DebugLogFlags_EventSelection = 1 << 5,
  DebugLogFlags_EventIO = 1 << 6,
  DebugLogFlags_EventDocking = 1 << 7,
  DebugLogFlags_EventViewport = 1 << 8,
  DebugLogFlags_EventMask_ =
      DebugLogFlags_EventActiveId | DebugLogFlags_EventFocus |
      DebugLogFlags_EventPopup | DebugLogFlags_EventNav |
      DebugLogFlags_EventClipper | DebugLogFlags_EventSelection |
      DebugLogFlags_EventIO | DebugLogFlags_EventDocking |
      DebugLogFlags_EventViewport,
  DebugLogFlags_OutputToTTY = 1 << 10,        // Also send output to TTY
  DebugLogFlags_OutputToTestEngine = 1 << 11, // Also send output to Test Engine
};

struct DebugAllocEntry {
  int FrameCount;
  signed short AllocCount;
  signed short FreeCount;
};

struct DebugAllocInfo {
  int TotalAllocCount; // Number of call to MemAlloc().
  int TotalFreeCount;
  signed short LastEntriesIdx;       // Current index in buffer
  DebugAllocEntry LastEntriesBuf[6]; // Track last 6 frames that had allocations

  DebugAllocInfo() { memset(this, 0, sizeof(*this)); }
};

struct MetricsConfig {
  bool ShowDebugLog = false;
  bool ShowIDStackTool = false;
  bool ShowWindowsRects = false;
  bool ShowWindowsBeginOrder = false;
  bool ShowTablesRects = false;
  bool ShowDrawCmdMesh = true;
  bool ShowDrawCmdBoundingBoxes = true;
  bool ShowAtlasTintedWithTextColor = false;
  bool ShowDockingNodes = false;
  int ShowWindowsRectsType = -1;
  int ShowTablesRectsType = -1;
};

struct StackLevelInfo {
  int ID;
  signed char QueryFrameCount; // >= 1: Query in progress
  bool QuerySuccess;           // Obtained result from DebugHookIdInfo()
  int DataType : 8;
  char Desc[57]; // Arbitrarily sized buffer to hold a result (FIXME: could
                 // replace Results[] with a chunk stream?) FIXME: Now that we
                 // added CTRL+C this should be fixed.

  StackLevelInfo() { memset(this, 0, sizeof(*this)); }
};

// State for int Stack tool queries
struct IDStackTool {
  int LastActiveFrame;
  int StackLevel; // -1: query stack and resize Results, >= 0: individual stack
                  // level
  int QueryId;    // int to query details for
  Vector<StackLevelInfo> Results;
  bool CopyToClipboardOnCtrlC;
  float CopyToClipboardLastTime;

  IDStackTool() {
    memset(this, 0, sizeof(*this));
    CopyToClipboardLastTime = -FLT_MAX;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*ContextHookCallback)(Context *ctx, ContextHook *hook);
enum ContextHookType {
  ContextHookType_NewFramePre,
  ContextHookType_NewFramePost,
  ContextHookType_EndFramePre,
  ContextHookType_EndFramePost,
  ContextHookType_RenderPre,
  ContextHookType_RenderPost,
  ContextHookType_Shutdown,
  ContextHookType_PendingRemoval_
};

struct ContextHook {
  int HookId; // A unique int assigned by AddContextHook()
  ContextHookType Type;
  int Owner;
  ContextHookCallback Callback;
  void *UserData;

  ContextHook() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// [SECTION] Context (main Gui context)
//-----------------------------------------------------------------------------

struct Context {
  bool Initialized;
  bool FontAtlasOwnedByContext; // IO.Fonts-> is owned by the Context and
                                // will be destructed along with it.
  IO IO;
  PlatformIO PlatformIO;
  Style Style;
  int ConfigFlagsCurrFrame; // = g.IO.ConfigFlags at the time of NewFrame()
  int ConfigFlagsLastFrame;
  Font *Font; // (Shortcut) == FontStack.empty() ? IO.Font : FontStack.back()
  float
      FontSize; // (Shortcut) == FontBaseSize * g.CurrentWindow->FontWindowScale
                // == window->FontSize(). Text height for current window.
  float FontBaseSize; // (Shortcut) == IO.FontGlobalScale * Font->Scale *
                      // Font->FontSize. Base text height.
  DrawListSharedData DrawListSharedData;
  double Time;
  int FrameCount;
  int FrameCountEnded;
  int FrameCountPlatformEnded;
  int FrameCountRendered;
  bool WithinFrameScope; // Set by NewFrame(), cleared by EndFrame()
  bool WithinFrameScopeWithImplicitWindow; // Set by NewFrame(), cleared by
                                           // EndFrame() when the implicit debug
                                           // window has been pushed
  bool WithinEndChild;                     // Set within EndChild()
  bool GcCompactAll;                       // Request full GC
  bool TestEngineHookItems;                // Will call test engine hooks:
                                           // TestEngineHook_ItemAdd(),
                                           // TestEngineHook_ItemInfo(),
                                           // TestEngineHook_Log()
  void *TestEngine;                        // Test engine user data

  // Inputs
  Vector<InputEvent> InputEventsQueue; // Input events which will be
                                       // trickled/written into IO structure.
  Vector<InputEvent>
      InputEventsTrail; // Past input events processed in NewFrame(). This is to
                        // allow domain-specific application to access e.g
                        // mouse/pen trail.
  MouseSource InputEventsNextMouseSource;
  unsigned int InputEventsNextEventId;

  // Windows state
  Vector<Window *> Windows; // Windows, sorted in display order, back to front
  Vector<Window *>
      WindowsFocusOrder; // Root windows, sorted in focus order, back to front.
  Vector<Window *>
      WindowsTempSortBuffer; // Temporary buffer used in EndFrame() to reorder
                             // windows so parents are kept before their child
  Vector<WindowStackData> CurrentWindowStack;
  Storage WindowsById;      // Map window's int to Window*
  int WindowsActiveCount;   // Number of unique windows submitted by frame
  Vec2 WindowsHoverPadding; // Padding around resizable windows for which
                            // hovering on counts as hovering the window ==
                            // Max(style.TouchExtraPadding,
                            // WINDOWS_HOVER_PADDING)
  Window *CurrentWindow;    // Window being drawn into
  Window *HoveredWindow;    // Window the mouse is hovering. Will typically
                            // catch mouse inputs.
  Window
      *HoveredWindowUnderMovingWindow; // Hovered window ignoring MovingWindow.
                                       // Only set if MovingWindow is set.
  Window *MovingWindow; // Track the window we clicked on (in order to
                        // preserve focus). The actual window that is moved
                        // is generally MovingWindow->RootWindowDockTree.
  Window
      *WheelingWindow; // Track the window we started mouse-wheeling on. Until a
                       // timer elapse or mouse has moved, generally keep
                       // scrolling the same window even if during the course of
                       // scrolling the mouse ends up hovering a child window.
  Vec2 WheelingWindowRefMousePos;
  int WheelingWindowStartFrame; // This may be set one frame before
                                // WheelingWindow is != NULL
  int WheelingWindowScrolledFrame;
  float WheelingWindowReleaseTimer;
  Vec2 WheelingWindowWheelRemainder;
  Vec2 WheelingAxisAvg;

  // Item/widgets state and tracking information
  int DebugHookIdInfo; // Will call core hooks: DebugHookIdInfo() from GetID
                       // functions, used by int Stack Tool [next
                       // HoveredId/ActiveId to not pull in an extra cache-line]
  int HoveredId;       // Hovered widget, filled during the frame
  int HoveredIdPreviousFrame;
  bool HoveredIdAllowOverlap;
  bool HoveredIdDisabled; // At least one widget passed the rect test, but has
                          // been discarded by disabled flag or popup inhibit.
                          // May be true even if HoveredId == 0.
  float HoveredIdTimer;   // Measure contiguous hovering time
  float HoveredIdNotActiveTimer; // Measure contiguous hovering time where the
                                 // item has not been active
  int ActiveId;                  // Active widget
  int ActiveIdIsAlive; // Active widget has been seen this frame (we can't use a
                       // bool as the ActiveId may change within the frame)
  float ActiveIdTimer;
  bool ActiveIdIsJustActivated; // Set at the time of activation for one frame
  bool ActiveIdAllowOverlap;    // Active widget allows another widget to steal
                                // active id (generally for overlapping widgets,
                                // but not always)
  bool ActiveIdNoClearOnFocusLoss; // Disable losing active id if the active id
                                   // window gets unfocused.
  bool ActiveIdHasBeenPressedBefore; // Track whether the active id led to a
                                     // press (this is to allow changing between
                                     // PressOnClick and PressOnRelease without
                                     // pressing twice). Used by range_select
                                     // branch.
  bool ActiveIdHasBeenEditedBefore;  // Was the value associated to the widget
                                     // Edited over the course of the Active
                                     // state.
  bool ActiveIdHasBeenEditedThisFrame;
  Vec2 ActiveIdClickOffset; // Clicked offset from upper-left corner, if
                            // applicable (currently only set by ButtonBehavior)
  Window *ActiveIdWindow;
  InputSource ActiveIdSource; // Activating source: InputSource_Mouse OR
                              // InputSource_Keyboard OR InputSource_Gamepad
  int ActiveIdMouseButton;
  int ActiveIdPreviousFrame;
  bool ActiveIdPreviousFrameIsAlive;
  bool ActiveIdPreviousFrameHasBeenEditedBefore;
  Window *ActiveIdPreviousFrameWindow;
  int LastActiveId; // Store the last non-zero ActiveId, useful for animation.
  float LastActiveIdTimer; // Store the last non-zero ActiveId timer since the
                           // beginning of activation, useful for animation.

  // [EXPERIMENTAL] Key/Input Ownership + Shortcut Routing system
  // - The idea is that instead of "eating" a given key, we can link to an
  // owner.
  // - Input query can then read input by specifying KeyOwner_Any (== 0),
  // KeyOwner_None (== -1) or a custom unsigned int.
  // - Routing is requested ahead of time for a given chord (Key + Mods) and
  // granted in NewFrame().
  KeyOwnerData KeysOwnerData[Key_NamedKey_COUNT];
  KeyRoutingTable KeysRoutingTable;
  unsigned int ActiveIdUsingNavDirMask; // Active widget will want to read those
                                        // nav move requests (e.g. can activate
                                        // a button and move away from it)
  bool ActiveIdUsingAllKeyboardKeys;    // Active widget will want to read all
                                     // keyboard keys inputs. (FIXME: This is a
                                     // shortcut for not taking ownership of
                                     // 100+ keys but perhaps best to not have
                                     // the inconsistency)
#ifndef DISABLE_OBSOLETE_KEYIO
  unsigned int
      ActiveIdUsingNavInputMask; // If you used this. Since (VERSION_NUM >=
                                 // 18804) : 'g.ActiveIdUsingNavInputMask |=
                                 // (1 << NavInput_Cancel);' becomes
                                 // 'SetKeyOwner(Key_Escape, g.ActiveId)
                                 // and/or
                                 // SetKeyOwner(Key_NavGamepadCancel,
                                 // g.ActiveId);'
#endif

  // Next window/item data
  int CurrentFocusScopeId; // == g.FocusScopeStack.back()
  int CurrentItemFlags;    // == g.ItemFlagsStack.back()
  int DebugLocateId; // Storage for DebugLocateItemOnHover() feature: this is
                     // read by ItemAdd() so we keep it in a hot/cached location
  NextItemData NextItemData; // Storage for SetNextItem** functions
  LastItemData
      LastItemData; // Storage for last submitted item (setup by ItemAdd)
  NextWindowData NextWindowData; // Storage for SetNextWindow** functions
  bool DebugShowGroupRects;

  // Shared stacks
  int DebugFlashStyleColorIdx; // (Keep close to ColorStack to share cache line)
  Vector<ColorMod> ColorStack; // Stack for PushStyleColor()/PopStyleColor() -
                               // inherited by Begin()
  Vector<StyleMod> StyleVarStack; // Stack for PushStyleVar()/PopStyleVar() -
                                  // inherited by Begin()
  Vector<::Font *>
      FontStack; // Stack for PushFont()/PopFont() - inherited by Begin()
  Vector<unsigned int>
      FocusScopeStack; // Stack for PushFocusScope()/PopFocusScope() - inherited
                       // by BeginChild(), pushed into by Begin()
  Vector<int> ItemFlagsStack;        // Stack for PushItemFlag()/PopItemFlag()
                                     // - inherited by Begin()
  Vector<GroupData> GroupStack;      // Stack for BeginGroup()/EndGroup() -
                                     // not inherited by Begin()
  Vector<PopupData> OpenPopupStack;  // Which popups are open (persistent)
  Vector<PopupData> BeginPopupStack; // Which level of BeginPopup() we
                                     // are in (reset every frame)
  Vector<NavTreeNodeData> NavTreeNodeStack; // Stack for TreeNode() when a
                                            // NavLeft requested is emitted.

  int BeginMenuCount;

  // Viewports
  Vector<ViewportP *> Viewports; // Active viewports (always 1+, and generally 1
                                 // unless multi-viewports are enabled). Each
                                 // viewports hold their copy of DrawData.
  float CurrentDpiScale;         // == CurrentViewport->DpiScale
  ViewportP
      *CurrentViewport; // We track changes of viewport (happening in Begin) so
                        // we can call Platform_OnChangedViewport()
  ViewportP *MouseViewport;
  ViewportP *
      MouseLastHoveredViewport; // Last known viewport that was hovered by mouse
                                // (even if we are not hovering any viewport any
                                // more) + honoring the _NoInputs flag.
  int PlatformLastFocusedViewportId;
  PlatformMonitor
      FallbackMonitor; // Virtual monitor used as fallback if backend doesn't
                       // provide monitor information.
  int ViewportCreatedCount; // Unique sequential creation counter (mostly for
                            // testing/debugging)
  int PlatformWindowsCreatedCount; // Unique sequential creation counter (mostly
                                   // for testing/debugging)
  int ViewportFocusedStampCount; // Every time the front-most window changes, we
                                 // stamp its viewport with an incrementing
                                 // counter

  // Gamepad/keyboard Navigation
  Window *NavWindow;     // Focused window for navigation. Could be called
                         // 'FocusedWindow'
  int NavId;             // Focused item for navigation
  int NavFocusScopeId;   // Identify a selection scope (selection code often
                         // wants to "clear other items" when landing on an
                         // item of the selection set)
  int NavActivateId;     // ~~ (g.ActiveId == 0) && (IsKeyPressed(Key_Space) ||
                         // IsKeyDown(Key_Enter) ||
                         // IsKeyPressed(Key_NavGamepadActivate)) ? NavId : 0,
                         // also set when calling ActivateItem()
  int NavActivateDownId; // ~~ IsKeyDown(Key_Space) ||
                         // IsKeyDown(Key_Enter) ||
                         // IsKeyDown(Key_NavGamepadActivate) ? NavId : 0
  int NavActivatePressedId; // ~~ IsKeyPressed(Key_Space) ||
                            // IsKeyPressed(Key_Enter) ||
                            // IsKeyPressed(Key_NavGamepadActivate) ?
                            // NavId : 0 (no repeat)
  int NavActivateFlags;
  int NavJustMovedToId;           // Just navigated to this id (result of a
                                  // successfully MoveRequest).
  int NavJustMovedToFocusScopeId; // Just navigated to this focus scope id
                                  // (result of a successfully MoveRequest).
  int NavJustMovedToKeyMods;
  int NavNextActivateId; // Set by ActivateItem(), queued until next frame.
  int NavNextActivateFlags;
  InputSource NavInputSource; // Keyboard or Gamepad mode? THIS CAN ONLY BE
                              // InputSource_Keyboard or InputSource_Mouse
  NavLayer NavLayer; // Layer we are navigating on. For now the system is
                     // hard-coded for 0=main contents and 1=menu/title
                     // bar, may expose layers later.
  SelectionUserData
      NavLastValidSelectionUserData; // Last valid data passed to
                                     // SetNextItemSelectionUser(), or -1. For
                                     // current window. Not reset when focusing
                                     // an item that doesn't have selection
                                     // data.
  bool NavIdIsAlive;     // Nav widget has been seen this frame ~~ NavRectRel is
                         // valid
  bool NavMousePosDirty; // When set we will update mouse position if
                         // (io.ConfigFlags &
                         // ConfigFlags_NavEnableSetMousePos) if set (NB:
                         // this not enabled by default)
  bool NavDisableHighlight; // When user starts using mouse, we hide
                            // gamepad/keyboard highlight (NB: but they are
                            // still available, which is why NavDisableHighlight
                            // isn't always != NavDisableMouseHover)
  bool NavDisableMouseHover; // When user starts using gamepad/keyboard, we hide
                             // mouse hovering highlight until mouse is touched
                             // again.

  // Navigation: Init & Move Requests
  bool NavAnyRequest;  // ~~ NavMoveRequest || NavInitRequest this is to perform
                       // early out in ItemAdd()
  bool NavInitRequest; // Init request for appearing window to select first item
  bool NavInitRequestFromMove;
  NavItemData
      NavInitResult; // Init request result (first item of the window, or one
                     // for which SetItemDefaultFocus() was called)
  bool NavMoveSubmitted; // Move request submitted, will process result on next
                         // NewFrame()
  bool NavMoveScoringItems; // Move request submitted, still scoring incoming
                            // items
  bool NavMoveForwardToNextFrame;
  int NavMoveFlags;
  int NavMoveScrollFlags;
  int NavMoveKeyMods;
  int NavMoveDir; // Direction of the move request (left/right/up/down)
  int NavMoveDirForDebug;
  int NavMoveClipDir;  // FIXME-NAV: Describe the purpose of this better.
                       // Might want to rename?
  Rect NavScoringRect; // Rectangle used for scoring, in screen space. Based
                       // of window->NavRectRel[], modified for directional
                       // navigation scoring.
  Rect NavScoringNoClipRect; // Some nav operations (such as PageUp/PageDown)
                             // enforce a region which clipper will attempt to
                             // always keep submitted
  int NavScoringDebugCount;  // Metrics for debugging
  int NavTabbingDir;     // Generally -1 or +1, 0 when tabbing without a nav id
  int NavTabbingCounter; // >0 when counting items for tabbing
  NavItemData
      NavMoveResultLocal; // Best move request candidate within NavWindow
  NavItemData
      NavMoveResultLocalVisible; // Best move request candidate within NavWindow
                                 // that are mostly visible (when using
                                 // NavMoveFlags_AlsoScoreVisibleSet flag)
  NavItemData NavMoveResultOther; // Best move request candidate within
                                  // NavWindow's flattened hierarchy (when using
                                  // WindowFlags_NavFlattened flag)
  NavItemData NavTabbingResultFirst; // First tabbing request candidate within
                                     // NavWindow and flattened hierarchy

  // Navigation: Windowing (CTRL+TAB for list, or Menu button + keys or
  // directional pads to move/resize)
  int ConfigNavWindowingKeyNext; // = Mod_Ctrl | Key_Tab,
                                 // for reconfiguration (see #4828)
  int ConfigNavWindowingKeyPrev; // = Mod_Ctrl | Mod_Shift |
                                 // Key_Tab
  Window *NavWindowingTarget;    // Target window when doing CTRL+Tab (or Pad
                                 // Menu + FocusPrev/Next), this window is
                                 // temporarily displayed top-most!
  Window
      *NavWindowingTargetAnim; // Record of last valid NavWindowingTarget until
                               // DimBgRatio and NavWindowingHighlightAlpha
                               // becomes 0.0f, so the fade-out can stay on it.
  Window *NavWindowingListWindow; // Internal window actually listing the
                                  // CTRL+Tab contents
  float NavWindowingTimer;
  float NavWindowingHighlightAlpha;
  bool NavWindowingToggleLayer;
  Vec2 NavWindowingAccumDeltaPos;
  Vec2 NavWindowingAccumDeltaSize;

  // Render
  float DimBgRatio; // 0.0..1.0 animation when fading in a dimming background
                    // (for modal window and CTRL+TAB list)

  // Drag and Drop
  bool DragDropActive;
  bool
      DragDropWithinSource; // Set when within a BeginDragDropXXX/EndDragDropXXX
                            // block for a drag source.
  bool
      DragDropWithinTarget; // Set when within a BeginDragDropXXX/EndDragDropXXX
                            // block for a drag target.
  int DragDropSourceFlags;
  int DragDropSourceFrameCount;
  int DragDropMouseButton;
  Payload DragDropPayload;
  Rect DragDropTargetRect; // Store rectangle of current target candidate (we
                           // favor small targets when overlapping)
  Rect DragDropTargetClipRect; // Store ClipRect at the time of item's drawing
  int DragDropTargetId;
  int DragDropAcceptFlags;
  float DragDropAcceptIdCurrRectSurface; // Target item surface (we resolve
                                         // overlapping targets by prioritizing
                                         // the smaller surface)
  int DragDropAcceptIdCurr;      // Target item id (set at the time of accepting
                                 // the payload)
  int DragDropAcceptIdPrev;      // Target item id from previous frame (we need
                                 // to store this to allow for overlapping drag
                                 // and drop targets)
  int DragDropAcceptFrameCount;  // Last time a target expressed a desire to
                                 // accept the source
  int DragDropHoldJustPressedId; // Set when holding a payload just made
                                 // ButtonBehavior() return a press.
  Vector<unsigned char>
      DragDropPayloadBufHeap; // We don't expose the Vector<> directly,
                              // Payload only holds pointer+size
  unsigned char DragDropPayloadBufLocal[16]; // Local buffer for small payloads

  // Clipper
  int ClipperTempDataStacked;
  Vector<ListClipperData> ClipperTempData;

  // Tables
  Table *CurrentTable;
  int TablesTempDataStacked; // Temporary table data size (because we leave
                             // previous instances undestructed, we generally
                             // don't use TablesTempData.Size)
  Vector<TableTempData>
      TablesTempData; // Temporary table data (buffers reused/shared across
                      // instances, support nesting)
  Pool<Table> Tables; // Persistent table data
  Vector<float> TablesLastTimeActive; // Last used timestamp of each tables
                                      // (SOA, for efficient GC)
  Vector<DrawChannel> DrawChannelsTempMergeBuffer;

  // Tab bars
  TabBar *CurrentTabBar;
  Pool<TabBar> TabBars;
  Vector<PtrOrIndex> CurrentTabBarStack;
  Vector<ShrinkWidthItem> ShrinkWidthBuffer;

  // Hover Delay system
  int HoverItemDelayId;
  int HoverItemDelayIdPreviousFrame;
  float HoverItemDelayTimer; // Currently used by IsItemHovered()
  float
      HoverItemDelayClearTimer; // Currently used by IsItemHovered(): grace time
                                // before g.TooltipHoverTimer gets cleared.
  int HoverItemUnlockedStationaryId;   // Mouse has once been stationary on
                                       // this item. Only reset after
                                       // departing the item.
  int HoverWindowUnlockedStationaryId; // Mouse has once been stationary on
                                       // this window. Only reset after
                                       // departing the window.

  // Mouse state
  int MouseCursor;
  float MouseStationaryTimer; // Time the mouse has been stationary (with some
                              // loose heuristic)
  Vec2 MouseLastValidPos;

  // Widget state
  InputTextState InputTextState;
  InputTextDeactivatedState InputTextDeactivatedState;
  struct Font InputTextPasswordFont;
  int TempInputId; // Temporary text input when CTRL+clicking on a slider, etc.
  int ColorEditOptions;    // Store user options for color edit widgets
  int ColorEditCurrentID;  // Set temporarily while inside of the parent-most
                           // ColorEdit4/ColorPicker4 (because they call each
                           // others).
  int ColorEditSavedID;    // int we are saving/restoring HS for
  float ColorEditSavedHue; // Backup of last Hue associated to LastColor, so we
                           // can restore Hue in lossy RGB<>HSV round trips
  float ColorEditSavedSat; // Backup of last Saturation associated to LastColor,
                           // so we can restore Saturation in lossy RGB<>HSV
                           // round trips
  unsigned int ColorEditSavedColor; // RGB value with alpha set to 0.
  Vec4 ColorPickerRef; // Initial/reference color at the time of opening the
                       // color picker.
  ComboPreviewData ComboPreviewData;
  Rect WindowResizeBorderExpectedRect; // Expected border rect, switch to
                                       // relative edit if moving
  bool WindowResizeRelativeMode;
  float SliderGrabClickOffset;
  float SliderCurrentAccum; // Accumulated slider delta when using navigation
                            // controls.
  bool SliderCurrentAccumDirty; // Has the accumulated slider delta changed
                                // since last time we tried to apply it?
  bool DragCurrentAccumDirty;
  float DragCurrentAccum; // Accumulator for dragging modification. Always
                          // high-precision, not rounded by end-user precision
                          // settings
  float DragSpeedDefaultRatio;           // If speed == 0.0f, uses (max-min) *
                                         // DragSpeedDefaultRatio
  float ScrollbarClickDeltaToGrabCenter; // Distance between mouse and center of
                                         // grab box, normalized in parent
                                         // space. Use storage?
  float DisabledAlphaBackup; // Backup for style.Alpha for BeginDisabled()
  short DisabledStackSize;
  short LockMarkEdited;
  short TooltipOverrideCount;
  Vector<char>
      ClipboardHandlerData; // If no custom clipboard handler is defined
  Vector<unsigned int> MenusIdSubmittedThisFrame; // A list of menu IDs that
                                                  // were rendered at least once
  TypingSelectState TypingSelectState; // State for GetTypingSelectRequest()

  // Platform support
  PlatformImeData PlatformImeData; // Data updated by current frame
  struct PlatformImeData
      PlatformImeDataPrev; // Previous frame data (when changing
                           // we will call io.SetPlatformImeDataFn
  int PlatformImeViewport;

  // Extensions
  // FIXME: We could provide an API to register one slot in an array held in
  // Context?
  DockContext DockContext;
  void (*DockNodeWindowMenuHandler)(Context *ctx, DockNode *node,
                                    TabBar *tab_bar);

  // Settings
  bool SettingsLoaded;
  float
      SettingsDirtyTimer; // Save .ini Settings to memory when time reaches zero
  TextBuffer SettingsIniData;                  // In memory .ini settings
  Vector<SettingsHandler> SettingsHandlers;    // List of .ini settings handlers
  ChunkStream<WindowSettings> SettingsWindows; // Window .ini settings entries
  ChunkStream<TableSettings> SettingsTables;   // Table .ini settings entries
  Vector<ContextHook> Hooks; // Hooks for extensions (e.g. test engine)
  int HookIdNext;            // Next available HookId

  // Localization
  const char *LocalizationTable[LocKey_COUNT];

  // Capture/Logging
  bool LogEnabled;      // Currently capturing
  LogType LogType;      // Capture target
  FileHandle LogFile;   // If != NULL log to stdout/ file
  TextBuffer LogBuffer; // Accumulation buffer when log to clipboard. This
                        // is pointer so our GGui static constructor
                        // doesn't call heap allocators.
  const char *LogNextPrefix;
  const char *LogNextSuffix;
  float LogLinePosY;
  bool LogLineFirstItem;
  int LogDepthRef;
  int LogDepthToExpand;
  int LogDepthToExpandDefault; // Default/stored value for LogDepthMaxExpand if
                               // not specified in the LogXXX function call.

  // Debug Tools
  int DebugLogFlags;
  TextBuffer DebugLogBuf;
  TextIndex DebugLogIndex;
  unsigned char DebugLogClipperAutoDisableFrames;
  unsigned char DebugLocateFrames; // For DebugLocateItemOnHover(). This is used
                                   // together with DebugLocateId which is in a
                                   // hot/cached spot above.
  signed char
      DebugBeginReturnValueCullDepth; // Cycle between 0..9 then wrap around.
  bool DebugItemPickerActive;         // Item picker is active (started with
                                      // DebugStartItemPicker())
  unsigned char DebugItemPickerMouseButton;
  int DebugItemPickerBreakId; // Will call DEBUG_BREAK() when
                              // encountering this unsigned int
  float DebugFlashStyleColorTime;
  Vec4 DebugFlashStyleColorBackup;
  MetricsConfig DebugMetricsConfig;
  IDStackTool DebugIDStackTool;
  DebugAllocInfo DebugAllocInfo;
  DockNode *DebugHoveredDockNode; // Hovered dock node.

  // Misc
  float FramerateSecPerFrame[60]; // Calculate estimate of framerate for user
                                  // over the last 60 frames..
  int FramerateSecPerFrameIdx;
  int FramerateSecPerFrameCount;
  float FramerateSecPerFrameAccum;
  int WantCaptureMouseNextFrame; // Explicit capture override via
                                 // SetNextFrameWantCaptureMouse()/SetNextFrameWantCaptureKeyboard().
                                 // Default to -1.
  int WantCaptureKeyboardNextFrame; // "
  int WantTextInputNextFrame;
  Vector<char> TempBuffer; // Temporary text buffer

  Context(FontAtlas *shared_font_atlas) {
    IO.Ctx = this;
    InputTextState.Ctx = this;

    Initialized = false;
    ConfigFlagsCurrFrame = ConfigFlagsLastFrame = ConfigFlags_None;
    FontAtlasOwnedByContext = shared_font_atlas ? false : true;
    Font = NULL;
    FontSize = FontBaseSize = 0.0f;
    IO.Fonts = shared_font_atlas ? shared_font_atlas : NEW(FontAtlas)();
    Time = 0.0f;
    FrameCount = 0;
    FrameCountEnded = FrameCountPlatformEnded = FrameCountRendered = -1;
    WithinFrameScope = WithinFrameScopeWithImplicitWindow = WithinEndChild =
        false;
    GcCompactAll = false;
    TestEngineHookItems = false;
    TestEngine = NULL;

    InputEventsNextMouseSource = MouseSource_Mouse;
    InputEventsNextEventId = 1;

    WindowsActiveCount = 0;
    CurrentWindow = NULL;
    HoveredWindow = NULL;
    HoveredWindowUnderMovingWindow = NULL;
    MovingWindow = NULL;
    WheelingWindow = NULL;
    WheelingWindowStartFrame = WheelingWindowScrolledFrame = -1;
    WheelingWindowReleaseTimer = 0.0f;

    DebugHookIdInfo = 0;
    HoveredId = HoveredIdPreviousFrame = 0;
    HoveredIdAllowOverlap = false;
    HoveredIdDisabled = false;
    HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
    ActiveId = 0;
    ActiveIdIsAlive = 0;
    ActiveIdTimer = 0.0f;
    ActiveIdIsJustActivated = false;
    ActiveIdAllowOverlap = false;
    ActiveIdNoClearOnFocusLoss = false;
    ActiveIdHasBeenPressedBefore = false;
    ActiveIdHasBeenEditedBefore = false;
    ActiveIdHasBeenEditedThisFrame = false;
    ActiveIdClickOffset = Vec2(-1, -1);
    ActiveIdWindow = NULL;
    ActiveIdSource = InputSource_None;
    ActiveIdMouseButton = -1;
    ActiveIdPreviousFrame = 0;
    ActiveIdPreviousFrameIsAlive = false;
    ActiveIdPreviousFrameHasBeenEditedBefore = false;
    ActiveIdPreviousFrameWindow = NULL;
    LastActiveId = 0;
    LastActiveIdTimer = 0.0f;

    ActiveIdUsingNavDirMask = 0x00;
    ActiveIdUsingAllKeyboardKeys = false;
#ifndef DISABLE_OBSOLETE_KEYIO
    ActiveIdUsingNavInputMask = 0x00;
#endif

    CurrentFocusScopeId = 0;
    CurrentItemFlags = ItemFlags_None;
    DebugShowGroupRects = false;
    BeginMenuCount = 0;

    CurrentDpiScale = 0.0f;
    CurrentViewport = NULL;
    MouseViewport = MouseLastHoveredViewport = NULL;
    PlatformLastFocusedViewportId = 0;
    ViewportCreatedCount = PlatformWindowsCreatedCount = 0;
    ViewportFocusedStampCount = 0;

    NavWindow = NULL;
    NavId = NavFocusScopeId = NavActivateId = NavActivateDownId =
        NavActivatePressedId = 0;
    NavJustMovedToId = NavJustMovedToFocusScopeId = NavNextActivateId = 0;
    NavActivateFlags = NavNextActivateFlags = ActivateFlags_None;
    NavJustMovedToKeyMods = Mod_None;
    NavInputSource = InputSource_Keyboard;
    NavLayer = NavLayer_Main;
    NavLastValidSelectionUserData = SelectionUserData_Invalid;
    NavIdIsAlive = false;
    NavMousePosDirty = false;
    NavDisableHighlight = true;
    NavDisableMouseHover = false;
    NavAnyRequest = false;
    NavInitRequest = false;
    NavInitRequestFromMove = false;
    NavMoveSubmitted = false;
    NavMoveScoringItems = false;
    NavMoveForwardToNextFrame = false;
    NavMoveFlags = NavMoveFlags_None;
    NavMoveScrollFlags = ScrollFlags_None;
    NavMoveKeyMods = Mod_None;
    NavMoveDir = NavMoveDirForDebug = NavMoveClipDir = Dir_None;
    NavScoringDebugCount = 0;
    NavTabbingDir = 0;
    NavTabbingCounter = 0;

    ConfigNavWindowingKeyNext = Mod_Ctrl | Key_Tab;
    ConfigNavWindowingKeyPrev = Mod_Ctrl | Mod_Shift | Key_Tab;
    NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
    NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
    NavWindowingToggleLayer = false;

    DimBgRatio = 0.0f;

    DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
    DragDropSourceFlags = DragDropFlags_None;
    DragDropSourceFrameCount = -1;
    DragDropMouseButton = -1;
    DragDropTargetId = 0;
    DragDropAcceptFlags = DragDropFlags_None;
    DragDropAcceptIdCurrRectSurface = 0.0f;
    DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
    DragDropAcceptFrameCount = -1;
    DragDropHoldJustPressedId = 0;
    memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

    ClipperTempDataStacked = 0;

    CurrentTable = NULL;
    TablesTempDataStacked = 0;
    CurrentTabBar = NULL;

    HoverItemDelayId = HoverItemDelayIdPreviousFrame =
        HoverItemUnlockedStationaryId = HoverWindowUnlockedStationaryId = 0;
    HoverItemDelayTimer = HoverItemDelayClearTimer = 0.0f;

    MouseCursor = MouseCursor_Arrow;
    MouseStationaryTimer = 0.0f;

    TempInputId = 0;
    ColorEditOptions = ColorEditFlags_DefaultOptions_;
    ColorEditCurrentID = ColorEditSavedID = 0;
    ColorEditSavedHue = ColorEditSavedSat = 0.0f;
    ColorEditSavedColor = 0;
    WindowResizeRelativeMode = false;
    SliderGrabClickOffset = 0.0f;
    SliderCurrentAccum = 0.0f;
    SliderCurrentAccumDirty = false;
    DragCurrentAccumDirty = false;
    DragCurrentAccum = 0.0f;
    DragSpeedDefaultRatio = 1.0f / 100.0f;
    ScrollbarClickDeltaToGrabCenter = 0.0f;
    DisabledAlphaBackup = 0.0f;
    DisabledStackSize = 0;
    LockMarkEdited = 0;
    TooltipOverrideCount = 0;

    PlatformImeData.InputPos = Vec2(0.0f, 0.0f);
    PlatformImeDataPrev.InputPos =
        Vec2(-1.0f, -1.0f); // Different to ensure initial submission
    PlatformImeViewport = 0;

    DockNodeWindowMenuHandler = NULL;

    SettingsLoaded = false;
    SettingsDirtyTimer = 0.0f;
    HookIdNext = 0;

    memset(LocalizationTable, 0, sizeof(LocalizationTable));

    LogEnabled = false;
    LogType = LogType_None;
    LogNextPrefix = LogNextSuffix = NULL;
    LogFile = NULL;
    LogLinePosY = FLT_MAX;
    LogLineFirstItem = false;
    LogDepthRef = 0;
    LogDepthToExpand = LogDepthToExpandDefault = 2;

    DebugLogFlags = DebugLogFlags_OutputToTTY;
    DebugLocateId = 0;
    DebugLogClipperAutoDisableFrames = 0;
    DebugLocateFrames = 0;
    DebugBeginReturnValueCullDepth = -1;
    DebugItemPickerActive = false;
    DebugItemPickerMouseButton = MouseButton_Left;
    DebugItemPickerBreakId = 0;
    DebugFlashStyleColorTime = 0.0f;
    DebugFlashStyleColorIdx = Col_COUNT;
    DebugHoveredDockNode = NULL;

    memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
    FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
    FramerateSecPerFrameAccum = 0.0f;
    WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame =
        WantTextInputNextFrame = -1;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] WindowTempData, Window
//-----------------------------------------------------------------------------

// Transient per-window data, reset at the beginning of the frame. This used to
// be called DrawContext, hence the DC variable name in Window.
// (That's theory, in practice the delimitation between Window and
// WindowTempData is quite tenuous and could be reconsidered..) (This
// doesn't need a constructor because we zero-clear it as part of Window
// and all frame-temporary data are setup on Begin)
struct API WindowTempData {
  // Layout
  Vec2 CursorPos; // Current emitting position, in absolute coordinates.
  Vec2 CursorPosPrevLine;
  Vec2 CursorStartPos; // Initial position after Begin(), generally ~ window
                       // position + WindowPadding.
  Vec2 CursorMaxPos;   // Used to implicitly calculate ContentSize at the
                       // beginning of next frame, for scrolling range and
                       // auto-resize. Always growing during the frame.
  Vec2 IdealMaxPos;    // Used to implicitly calculate ContentSizeIdeal at the
                       // beginning of next frame, for auto-resize only. Always
                       // growing during the frame.
  Vec2 CurrLineSize;
  Vec2 PrevLineSize;
  float CurrLineTextBaseOffset; // Baseline offset (0.0f by default on a new
                                // line, generally == style.FramePadding.y when
                                // a framed item has been added).
  float PrevLineTextBaseOffset;
  bool IsSameLine;
  bool IsSetPos;
  Vec1 Indent; // Indentation / start position from left of window (increased
               // by TreePush/TreePop, etc.)
  Vec1
      ColumnsOffset; // Offset to the current column (if ColumnsCurrent > 0).
                     // FIXME: This and the above should be a stack to allow use
                     // cases like Tree->Column->Tree. Need revamp columns API.
  Vec1 GroupOffset;
  Vec2
      CursorStartPosLossyness; // Record the loss of precision of CursorStartPos
                               // due to really large scrolling amount. This is
                               // used by clipper to compensate and fix the most
                               // common use case of large scroll area.

  // Keyboard/Gamepad navigation
  NavLayer NavLayerCurrent; // Current layer, 0..31 (we currently only use 0..1)
  short NavLayersActiveMask; // Which layers have been written to (result from
                             // previous frame)
  short NavLayersActiveMaskNext; // Which layers have been written to
                                 // (accumulator for current frame)
  bool NavIsScrollPushableX; // Set when current work location may be scrolled
                             // horizontally when moving left / right. This is
                             // generally always true UNLESS within a column.
  bool NavHideHighlightOneFrame;
  bool NavWindowHasScrollY; // Set per window when scrolling can be used (==
                            // ScrollMax.y > 0.0f)

  // Miscellaneous
  bool MenuBarAppending; // FIXME: Remove this
  Vec2 MenuBarOffset;    // MenuBarOffset.x is sort of equivalent of a per-layer
                         // CursorPos.x, saved/restored as we switch to the menu
                         // bar. The only situation when MenuBarOffset.y is > 0
                         // if when (SafeAreaPadding.y > FramePadding.y), often
                         // used on TVs.
  MenuColumns
      MenuColumns; // Simplified columns storage for menu items measurement
  int TreeDepth;   // Current tree depth.
  unsigned int
      TreeJumpToParentOnPopMask; // Store a copy of !g.NavIdIsAlive for
                                 // TreeDepth 0..31.. Could be turned
                                 // into a unsigned long long if necessary.
  Vector<Window *> ChildWindows;
  Storage *StateStorage;      // Current persistent per-window storage (store
                              // e.g. tree node open/close state)
  OldColumns *CurrentColumns; // Current columns set
  int CurrentTableIdx;        // Current table index (into g.Tables)
  int LayoutType;
  int ParentLayoutType; // Layout type of parent window at the time of Begin()

  // Local parameters stacks
  // We store the current settings outside of the vectors to increase memory
  // locality (reduce cache misses). The vectors are rarely modified. Also it
  // allows us to not heap allocate for short-lived windows which are not using
  // those settings.
  float ItemWidth; // Current item width (>0.0: width in pixels, <0.0: align xx
                   // pixels to the right of window).
  float TextWrapPos;              // Current text wrap pos.
  Vector<float> ItemWidthStack;   // Store item widths to restore (attention:
                                  // .back() is not == ItemWidth)
  Vector<float> TextWrapPosStack; // Store text wrap pos to restore (attention:
                                  // .back() is not == TextWrapPos)
};

// Storage for one window
struct API Window {
  Context *Ctx; // Parent UI context (needs to be set explicitly by parent).
  char *Name;   // Window name, owned by the window.
  int ID;       // == HashStr(Name)
  int Flags, FlagsPreviousFrame; // See enum WindowFlags_
  int ChildFlags;                // Set when window is a child window. See enum
                                 // ChildFlags_
  WindowClass WindowClass; // Advanced users only. Set with SetNextWindowClass()
  ViewportP *Viewport;     // Always set in Begin(). Inactive windows may have a
                           // NULL value here if their viewport was discarded.
  int ViewportId;          // We backup the viewport id (since the viewport may
                  // disappear or never be created if the window is inactive)
  Vec2 ViewportPos; // We backup the viewport position (since the viewport may
                    // disappear or never be created if the window is inactive)
  int ViewportAllowPlatformMonitorExtend; // Reset to -1 every frame (index is
                                          // guaranteed to be valid between
                                          // NewFrame..EndFrame), only used in
                                          // the Appearing frame of a
                                          // tooltip/popup to enforce clamping
                                          // to a given monitor
  Vec2 Pos;         // Position (always rounded-up to nearest pixel)
  Vec2 Size;        // Current size (==SizeFull or collapsed title bar size)
  Vec2 SizeFull;    // Size when non collapsed
  Vec2 ContentSize; // Size of contents/scrollable client area (calculated from
                    // the extents reach of the cursor) from previous frame.
                    // Does not include window decoration or window padding.
  Vec2 ContentSizeIdeal;
  Vec2 ContentSizeExplicit; // Size of contents/scrollable client area
                            // explicitly request by the user via
                            // SetNextWindowContentSize().
  Vec2 WindowPadding;       // Window padding at the time of Begin().
  float WindowRounding;     // Window rounding at the time of Begin(). May be
                        // clamped lower to avoid rendering artifacts with title
                        // bar, menu bar etc.
  float WindowBorderSize; // Window border size at the time of Begin().
  float DecoOuterSizeX1,
      DecoOuterSizeY1; // Left/Up offsets. Sum of non-scrolling outer
                       // decorations (X1 generally == 0.0f. Y1 generally =
                       // TitleBarHeight + MenuBarHeight). Locked during
                       // Begin().
  float DecoOuterSizeX2,
      DecoOuterSizeY2; // Right/Down offsets (X2 generally == ScrollbarSize.x,
                       // Y2 == ScrollbarSizes.y).
  float DecoInnerSizeX1,
      DecoInnerSizeY1; // Applied AFTER/OVER InnerRect. Specialized for Tables
                       // as they use specialized form of clipping and frozen
                       // rows/columns are inside InnerRect (and not part of
                       // regular decoration sizes).
  int NameBufLen;      // Size of buffer storing Name. May be larger than
                       // strlen(Name)!
  int MoveId;          // == window->GetID("#MOVE")
  int TabId;           // == window->GetID("#TAB")
  int ChildId; // int of corresponding item in parent window (for navigation
               // to return from child window to parent window)
  Vec2 Scroll;
  Vec2 ScrollMax;
  Vec2 ScrollTarget; // target scroll position. stored as cursor position with
                     // scrolling canceled out, so the highest point is always
                     // 0.0f. (FLT_MAX for no change)
  Vec2 ScrollTargetCenterRatio;  // 0.0f = scroll so that target position is at
                                 // top, 0.5f = scroll so that target position
                                 // is centered
  Vec2 ScrollTargetEdgeSnapDist; // 0.0f = no snapping, >0.0f snapping threshold
  Vec2 ScrollbarSizes; // Size taken by each scrollbars on their smaller axis.
                       // Pay attention! ScrollbarSizes.x == width of the
                       // vertical scrollbar, ScrollbarSizes.y = height of the
                       // horizontal scrollbar.
  bool ScrollbarX, ScrollbarY; // Are scrollbars visible?
  bool ViewportOwned;
  bool Active; // Set to true on Begin(), unless Collapsed
  bool WasActive;
  bool WriteAccessed; // Set to true when any widget access the current window
  bool Collapsed;     // Set when collapsing window to become only title-bar
  bool WantCollapseToggle;
  bool SkipItems; // Set when items can safely be all clipped (e.g. window not
                  // visible or collapsed)
  bool Appearing; // Set during the frame where the window is appearing (or
                  // re-appearing)
  bool Hidden;    // Do not display (== HiddenFrames*** > 0)
  bool IsFallbackWindow; // Set on the "Debug##Default" window.
  bool IsExplicitChild;  // Set when passed _ChildWindow, left to false by
                         // BeginDocked()
  bool
      HasCloseButton; // Set when the window has a close button (p_open != NULL)
  signed char ResizeBorderHovered; // Current border being hovered for resize
                                   // (-1: none, otherwise 0-3)
  signed char ResizeBorderHeld;    // Current border being held for resize (-1:
                                   // none, otherwise 0-3)
  short BeginCount; // Number of Begin() during the current frame (generally 0
                    // or 1, 1+ if appending via multiple Begin/End pairs)
  short BeginCountPreviousFrame; // Number of Begin() during the previous frame
  short BeginOrderWithinParent; // Begin() order within immediate parent window,
                                // if we are a child window. Otherwise 0.
  short BeginOrderWithinContext; // Begin() order within entire imgui context.
                                 // This is mostly used for debugging submission
                                 // order related issues.
  short FocusOrder; // Order within WindowsFocusOrder[], altered when windows
                    // are focused.
  int PopupId;      // int in the popup stack when this window is used as a
                    // popup/menu (because we use generic Name/unsigned int for
                    // recycling)
  signed char AutoFitFramesX, AutoFitFramesY;
  bool AutoFitOnlyGrows;
  int AutoPosLastDirection;
  signed char HiddenFramesCanSkipItems;    // Hide the window for N frames
  signed char HiddenFramesCannotSkipItems; // Hide the window for N frames while
                                           // allowing items to be submitted so
                                           // we can measure their size
  signed char HiddenFramesForRenderOnly;   // Hide the window until frame N at
                                           // Render() time only
  signed char DisableInputsFrames; // Disable window interactions for N frames
  int SetWindowPosAllowFlags : 8;  // store acceptable condition flags for
                                   // SetNextWindowPos() use.
  int SetWindowSizeAllowFlags : 8; // store acceptable condition flags for
                                   // SetNextWindowSize() use.
  int SetWindowCollapsedAllowFlags : 8; // store acceptable condition flags for
                                        // SetNextWindowCollapsed() use.
  int SetWindowDockAllowFlags : 8;      // store acceptable condition flags for
                                        // SetNextWindowDock() use.
  Vec2 SetWindowPosVal;   // store window position when using a non-zero Pivot
                          // (position set needs to be processed when we know
                          // the window size)
  Vec2 SetWindowPosPivot; // store window pivot for positioning. Vec2(0, 0)
                          // when positioning from top-left corner; Vec2(0.5f,
                          // 0.5f) for centering; Vec2(1, 1) for bottom right.

  Vector<int> IDStack; // int stack. int are hashes seeded with the value at
                       // the top of the stack. (In theory this should be
                       // in the TempData structure)
  WindowTempData DC;   // Temporary per-window data, reset at the beginning
                       // of the frame. This used to be called
                       // DrawContext, hence the "DC" variable name.

  // The best way to understand what those rectangles are is to use the
  // 'Metrics->Tools->Show Windows Rectangles' viewer. The main 'OuterRect',
  // omitted as a field, is window->Rect().
  Rect OuterRectClipped; // == Window->Rect() just after setup in Begin(). ==
                         // window->Rect() for root window.
  Rect InnerRect;     // Inner rectangle (omit title bar, menu bar, scroll bar)
  Rect InnerClipRect; // == InnerRect shrunk by WindowPadding*0.5f on each
                      // side, clipped within viewport or parent clip rect.
  Rect WorkRect;      // Initially covers the whole scrolling region. Reduced by
                      // containers e.g columns/tables when active. Shrunk by
  // WindowPadding*1.0f on each side. This is meant to replace
  // ContentRegionRect over time (from 1.71+ onward).
  Rect ParentWorkRect; // Backup of WorkRect before entering a container such
                       // as columns/tables. Used by e.g. SpanAllColumns
                       // functions to easily access. Stacked containers are
                       // responsible for maintaining this. // FIXME-WORKRECT:
                       // Could be a stack?
  Rect
      ClipRect; // Current clipping/scissoring rectangle, evolve as we are using
                // PushClipRect(), etc. == DrawList->clip_rect_stack.back().
  Rect ContentRegionRect; // FIXME: This is currently confusing/misleading. It
                          // is essentially WorkRect but not handling of
                          // scrolling. We currently rely on it as right/bottom
                          // aligned sizing operation need some size to rely on.
  Vec2ih HitTestHoleSize; // Define an optional rectangular hole where mouse
                          // will pass-through the window.
  Vec2ih HitTestHoleOffset;

  int LastFrameActive;      // Last frame number the window was Active.
  int LastFrameJustFocused; // Last frame number the window was made Focused.
  float LastTimeActive; // Last timestamp the window was Active (using float as
                        // we don't need high precision there)
  float ItemWidthDefault;
  Storage StateStorage;
  Vector<OldColumns> ColumnsStorage;
  float FontWindowScale; // User scale multiplier per-window, via
                         // SetWindowFontScale()
  float FontDpiScale;
  int SettingsOffset; // Offset into SettingsWindows[] (offsets are always valid
                      // as we only grow the array from the back)

  DrawList *DrawList; // == &DrawListInst (for backward compatibility reason
                      // with code using internal.hpp we keep this a pointer)
  struct DrawList DrawListInst;
  Window *ParentWindow; // If we are a child _or_ popup _or_ docked window,
                        // this is pointing to our parent. Otherwise NULL.
  Window *ParentWindowInBeginStack;
  Window *RootWindow; // Point to ourself or first ancestor that is not a child
                      // window. Doesn't cross through popups/dock nodes.
  Window
      *RootWindowPopupTree; // Point to ourself or first ancestor that is not a
                            // child window. Cross through popups parent<>child.
  Window *RootWindowDockTree; // Point to ourself or first ancestor that is not
                              // a child window. Cross through dock nodes.
  Window *RootWindowForTitleBarHighlight; // Point to ourself or first ancestor
                                          // which will display TitleBgActive
                                          // color when this window is active.
  Window *RootWindowForNav; // Point to ourself or first ancestor which
                            // doesn't have the NavFlattened flag.

  Window
      *NavLastChildNavWindow; // When going to the menu bar, we remember the
                              // child window we came from. (This could probably
                              // be made implicit if we kept g.Windows sorted by
                              // last focused including child window.)
  int NavLastIds[NavLayer_COUNT];  // Last known NavId for this window,
                                   // per layer (0/1)
  Rect NavRectRel[NavLayer_COUNT]; // Reference rectangle, in window
                                   // relative space
  Vec2 NavPreferredScoringPosRel
      [NavLayer_COUNT];    // Preferred X/Y position updated when moving on a
                           // given axis, reset to FLT_MAX.
  int NavRootFocusScopeId; // Focus Scope int at the time of Begin()

  int MemoryDrawListIdxCapacity; // Backup of last idx/vtx count, so when waking
                                 // up the window we can preallocate and avoid
                                 // iterative alloc/copy
  int MemoryDrawListVtxCapacity;
  bool MemoryCompacted; // Set when window extraneous data have been garbage
                        // collected

  // Docking
  bool
      DockIsActive : 1; // When docking artifacts are actually visible. When
                        // this is set, DockNode is guaranteed to be != NULL. ~~
                        // (DockNode != NULL) && (DockNode->Windows.Size > 1).
  bool DockNodeIsVisible : 1;
  bool DockTabIsVisible : 1; // Is our window visible this frame? ~~ is the
                             // corresponding tab selected?
  bool DockTabWantClose : 1;
  short DockOrder; // Order of the last time the window was visible within its
                   // DockNode. This is used to reorder windows that are
                   // reappearing on the same frame. Same value between windows
                   // that were active and windows that were none are possible.
  WindowDockStyle DockStyle;
  DockNode *DockNode; // Which node are we docked into. Important: Prefer
                      // testing DockIsActive in many cases as this will
                      // still be set when the dock node is hidden.
  struct DockNode
      *DockNodeAsHost; // Which node are we owning (for parent windows)
  int DockId;          // Backup of last valid DockNode->ID, so single window
              // remember their dock node id even when they are not bound any
              // more
  int DockTabItemStatusFlags;
  Rect DockTabItemRect;

public:
  Window(Context *context, const char *name);
  ~Window();

  int GetID(const char *str, const char *str_end = NULL);
  int GetID(const void *ptr);
  int GetID(int n);
  int GetIDFromRectangle(const Rect &r_abs);

  // We don't use g.FontSize because the window may be != g.CurrentWindow.
  Rect Rect() const {
    return ::Rect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
  }
  float CalcFontSize() const {
    Context &g = *Ctx;
    float scale = g.FontBaseSize * FontWindowScale * FontDpiScale;
    if (ParentWindow)
      scale *= ParentWindow->FontWindowScale;
    return scale;
  }
  float TitleBarHeight() const {
    Context &g = *Ctx;
    return (Flags & WindowFlags_NoTitleBar)
               ? 0.0f
               : CalcFontSize() + g.Style.FramePadding.y * 2.0f;
  }
  ::Rect TitleBarRect() const {
    return ::Rect(Pos, Vec2(Pos.x + SizeFull.x, Pos.y + TitleBarHeight()));
  }
  float MenuBarHeight() const {
    Context &g = *Ctx;
    return (Flags & WindowFlags_MenuBar) ? DC.MenuBarOffset.y + CalcFontSize() +
                                               g.Style.FramePadding.y * 2.0f
                                         : 0.0f;
  }
  ::Rect MenuBarRect() const {
    float y1 = Pos.y + TitleBarHeight();
    return ::Rect(Pos.x, y1, Pos.x + SizeFull.x, y1 + MenuBarHeight());
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Tab bar, Tab item support
//-----------------------------------------------------------------------------

// Extend TabBarFlags_
enum TabBarFlagsPrivate_ {
  TabBarFlags_DockNode =
      1 << 20, // Part of a dock node [we don't use this in the master branch
               // but it facilitate branch syncing to keep this around]
  TabBarFlags_IsFocused = 1 << 21,
  TabBarFlags_SaveSettings =
      1
      << 22, // FIXME: Settings are handled by the docking system, this only
             // request the tab bar to mark settings dirty when reordering tabs
};

// Extend TabItemFlags_
enum TabItemFlagsPrivate_ {
  TabItemFlags_SectionMask_ = TabItemFlags_Leading | TabItemFlags_Trailing,
  TabItemFlags_NoCloseButton =
      1 << 20, // Track whether p_open was set or not (we'll need this info on
               // the next frame to recompute ContentWidth during layout)
  TabItemFlags_Button = 1 << 21, // Used by TabItemButton, change the tab
                                 // item behavior to mimic a button
  TabItemFlags_Unsorted =
      1 << 22, // [Docking] Trailing tabs with the _Unsorted flag will be sorted
               // based on the DockOrder of their Window.
};

// Storage for one active tab item (sizeof() 48 bytes)
struct TabItem {
  int ID;
  int Flags;
  Window *Window; // When TabItem is part of a DockNode's TabBar, we hold
                  // on to a window.
  int LastFrameVisible;
  int LastFrameSelected; // This allows us to infer an ordered list of the last
                         // activated tabs with little maintenance
  float Offset;          // Position relative to beginning of tab
  float Width;           // Width currently displayed
  float ContentWidth;    // Width of label, stored during BeginTabItem() call
  float RequestedWidth; // Width optionally requested by caller, -1.0f is unused
  signed int NameOffset;   // When Window==NULL, offset to name within parent
                           // TabBar::TabsNames
  signed short BeginOrder; // BeginTabItem() order, used to re-order tabs after
                           // toggling TabBarFlags_Reorderable
  signed short
      IndexDuringLayout; // Index only used during TabBarLayout(). Tabs gets
                         // reordered so 'Tabs[n].IndexDuringLayout == n' but
                         // may mismatch during additions.
  bool WantClose;        // Marked as closed by SetTabItemClosed()

  TabItem() {
    memset(this, 0, sizeof(*this));
    LastFrameVisible = LastFrameSelected = -1;
    RequestedWidth = -1.0f;
    NameOffset = -1;
    BeginOrder = IndexDuringLayout = -1;
  }
};

// Storage for a tab bar (sizeof() 152 bytes)
struct API TabBar {
  Vector<TabItem> Tabs;
  int Flags;
  int ID;                // Zero for tab-bars used by docking
  int SelectedTabId;     // Selected tab/window
  int NextSelectedTabId; // Next selected tab/window. Will also trigger a
                         // scrolling animation
  int VisibleTabId;      // Can occasionally be != SelectedTabId (e.g. when
                         // previewing contents for CTRL+TAB preview)
  int CurrFrameVisible;
  int PrevFrameVisible;
  Rect BarRect;
  float CurrTabsContentsHeight;
  float PrevTabsContentsHeight; // Record the height of contents submitted below
                                // the tab bar
  float WidthAllTabs; // Actual width of all tabs (locked during layout)
  float
      WidthAllTabsIdeal; // Ideal width if all tabs were visible and not clipped
  float ScrollingAnim;
  float ScrollingTarget;
  float ScrollingTargetDistToVisibility;
  float ScrollingSpeed;
  float ScrollingRectMinX;
  float ScrollingRectMaxX;
  float SeparatorMinX;
  float SeparatorMaxX;
  int ReorderRequestTabId;
  signed short ReorderRequestOffset;
  signed char BeginCount;
  bool WantLayout;
  bool VisibleTabWasSubmitted;
  bool TabsAddedNew; // Set to true when a new tab item or button has been added
                     // to the tab bar during last frame
  signed short TabsActiveCount; // Number of tabs submitted this frame.
  signed short LastTabItemIdx;  // Index of last BeginTabItem() tab for use by
                                // EndTabItem()
  float ItemSpacingY;
  Vec2 FramePadding; // style.FramePadding locked at the time of BeginTabBar()
  Vec2 BackupCursorPos;
  TextBuffer TabsNames; // For non-docking tab bar we re-append names in a
                        // contiguous buffer.

  TabBar();
};

//-----------------------------------------------------------------------------
// [SECTION] Table support
//-----------------------------------------------------------------------------

#define COL32_DISABLE                                                          \
  COL32(0, 0, 0,                                                               \
        1) // Special sentinel code which cannot be used as a regular color.
#define TABLE_MAX_COLUMNS 512 // May be further lifted

// Our current column maximum is 64 but we may raise that in the future.
typedef signed short TableColumnIdx;
typedef unsigned short TableDrawChannelIdx;

// [Internal] sizeof() ~ 112
// We use the terminology "Enabled" to refer to a column that is not Hidden by
// user/api. We use the terminology "Clipped" to refer to a column that is out
// of sight because of scrolling/clipping. This is in contrast with some
// user-facing api such as IsItemVisible() / IsRectVisible() which use "Visible"
// to mean "not clipped".
struct TableColumn {
  int Flags;        // Flags after some patching (not directly same as
                    // provided by user). See TableColumnFlags_
  float WidthGiven; // Final/actual width visible == (MaxX - MinX), locked in
                    // TableUpdateLayout(). May be > WidthRequest to honor
                    // minimum width, may be < WidthRequest to honor shrinking
                    // columns down in tight space.
  float MinX;       // Absolute positions
  float MaxX;
  float WidthRequest;  // Master width absolute value when !(Flags &
                       // _WidthStretch). When Stretch this is derived every
                       // frame from StretchWeight in TableUpdateLayout()
  float WidthAuto;     // Automatic width
  float StretchWeight; // Master width weight when (Flags & _WidthStretch).
                       // Often around ~1.0f initially.
  float
      InitStretchWeightOrWidth; // Value passed to TableSetupColumn(). For Width
                                // it is a content width (_without padding_).
  Rect ClipRect;                // Clipping rectangle for the column
  int UserID;                   // Optional, value passed to TableSetupColumn()
  float WorkMinX;  // Contents region min ~(MinX + CellPaddingX + CellSpacingX1)
                   // == cursor start position when entering column
  float WorkMaxX;  // Contents region max ~(MaxX - CellPaddingX - CellSpacingX2)
  float ItemWidth; // Current item width for the column, preserved across rows
  float ContentMaxXFrozen; // Contents maximum position for frozen rows (apart
                           // from headers), from which we can infer content
                           // width.
  float ContentMaxXUnfrozen;
  float ContentMaxXHeadersUsed; // Contents maximum position for headers rows
                                // (regardless of freezing). TableHeader()
                                // automatically softclip itself + report ideal
                                // desired size, to avoid creating extraneous
                                // draw calls
  float ContentMaxXHeadersIdeal;
  signed short NameOffset;     // Offset into parent ColumnsNames[]
  TableColumnIdx DisplayOrder; // Index within Table's IndexToDisplayOrder[]
                               // (column may be reordered by users)
  TableColumnIdx IndexWithinEnabledSet; // Index within enabled/visible set
                                        // (<= IndexToDisplayOrder)
  TableColumnIdx
      PrevEnabledColumn; // Index of prev enabled/visible column within
                         // Columns[], -1 if first enabled/visible column
  TableColumnIdx
      NextEnabledColumn; // Index of next enabled/visible column within
                         // Columns[], -1 if last enabled/visible column
  TableColumnIdx
      SortOrder; // Index of this column within sort specs, -1 if not sorting on
                 // this column, 0 for single-sort, may be >0 on multi-sort
  TableDrawChannelIdx
      DrawChannelCurrent; // Index within DrawSplitter.Channels[]
  TableDrawChannelIdx
      DrawChannelFrozen; // Draw channels for frozen rows (often headers)
  TableDrawChannelIdx DrawChannelUnfrozen; // Draw channels for unfrozen rows
  bool IsEnabled;     // IsUserEnabled && (Flags & TableColumnFlags_Disabled)
                      // == 0
  bool IsUserEnabled; // Is the column not marked Hidden by the user? (unrelated
                      // to being off view, e.g. clipped by scrolling).
  bool IsUserEnabledNextFrame;
  bool IsVisibleX; // Is actually in view (e.g. overlapping the host window
                   // clipping rectangle, not scrolled).
  bool IsVisibleY;
  bool IsRequestOutput; // Return value for TableSetColumnIndex() /
                        // TableNextColumn(): whether we request user to output
                        // contents or not.
  bool IsSkipItems;     // Do we want item submissions to this column to be
                        // completely ignored (no layout will happen).
  bool IsPreserveWidthAuto;
  signed char NavLayerCurrent; // NavLayer in 1 byte
  unsigned char AutoFitQueue;  // Queue of 8 values for the next 8 frames to
                               // request auto-fit
  unsigned char CannotSkipItemsQueue; // Queue of 8 values for the next 8 frames
                                      // to disable Clipped/SkipItem
  unsigned char SortDirection : 2;    // SortDirection_Ascending or
                                      // SortDirection_Descending
  unsigned char SortDirectionsAvailCount : 2; // Number of available sort
                                              // directions (0 to 3)
  unsigned char SortDirectionsAvailMask : 4;  // Mask of available sort
                                              // directions (1-bit each)
  unsigned char
      SortDirectionsAvailList; // Ordered list of available sort directions
                               // (2-bits each, total 8-bits)

  TableColumn() {
    memset(this, 0, sizeof(*this));
    StretchWeight = WidthRequest = -1.0f;
    NameOffset = -1;
    DisplayOrder = IndexWithinEnabledSet = -1;
    PrevEnabledColumn = NextEnabledColumn = -1;
    SortOrder = -1;
    SortDirection = SortDirection_None;
    DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen =
        (unsigned char)-1;
  }
};

// Transient cell data stored per row.
// sizeof() ~ 6
struct TableCellData {
  unsigned int BgColor;  // Actual color
  TableColumnIdx Column; // Column number
};

// Per-instance data that needs preserving across frames (seemingly most others
// do not need to be preserved aside from debug needs. Does that means they
// could be moved to TableTempData?) sizeof() ~ 24 bytes
struct TableInstanceData {
  int TableInstanceID;
  float LastOuterHeight;         // Outer height from last frame
  float LastTopHeadersRowHeight; // Height of first consecutive header rows from
                                 // last frame (FIXME: this is used assuming
                                 // consecutive headers are in same frozen set)
  float LastFrozenHeight;        // Height of frozen section from last frame
  int HoveredRowLast;            // Index of row which was hovered last frame.
  int HoveredRowNext; // Index of row hovered this frame, set after encountering
                      // it.

  TableInstanceData() {
    TableInstanceID = 0;
    LastOuterHeight = LastTopHeadersRowHeight = LastFrozenHeight = 0.0f;
    HoveredRowLast = HoveredRowNext = -1;
  }
};

// FIXME-TABLE: more transient data could be stored in a stacked
// TableTempData: e.g. SortSpecs, incoming RowData sizeof() ~ 580 bytes +
// heap allocs described in TableBeginInitMemory()
struct API Table {
  int ID;
  int Flags;
  void *RawData; // Single allocation to hold Columns[], DisplayOrderToIndex[]
                 // and RowCellData[]
  TableTempData *TempData;   // Transient data while table is active. Point
                             // within g.CurrentTableStack[]
  Span<TableColumn> Columns; // Point within RawData[]
  Span<TableColumnIdx>
      DisplayOrderToIndex; // Point within RawData[]. Store display order of
                           // columns (when not reordered, the values are
                           // 0...Count-1)
  Span<TableCellData> RowCellData;       // Point within RawData[]. Store cells
                                         // background requests for current row.
  BitArrayPtr EnabledMaskByDisplayOrder; // Column DisplayOrder -> IsEnabled map
  BitArrayPtr
      EnabledMaskByIndex; // Column Index -> IsEnabled map (== not hidden by
                          // user/api) in a format adequate for iterating column
                          // without touching cold data
  BitArrayPtr VisibleMaskByIndex; // Column Index -> IsVisibleX|IsVisibleY map
                                  // (== not hidden by user/api && not hidden
                                  // by scrolling/cliprect)

  int SettingsLoadedFlags; // Which data were loaded from the .ini file (e.g.
                           // when order is not altered we won't save order)
  int SettingsOffset;      // Offset in g.SettingsTables
  int LastFrameActive;
  int ColumnsCount; // Number of columns declared in BeginTable()
  int CurrentRow;
  int CurrentColumn;
  signed short
      InstanceCurrent; // Count of BeginTable() calls with same int in the same
                       // frame (generally 0). This is a little bit similar to
                       // BeginCount for a window, but multiple table with same
                       // int look are multiple tables, they are just synched.
  signed short InstanceInteracted; // Mark which instance (generally 0) of the
                                   // same unsigned int is being interacted with
  float RowPosY1;
  float RowPosY2;
  float RowMinHeight;    // Height submitted to TableNextRow()
  float RowCellPaddingY; // Top and bottom padding. Reloaded during row change.
  float RowTextBaseline;
  float RowIndentOffsetX;
  int RowFlags : 16; // Current row flags, see TableRowFlags_
  int LastRowFlags : 16;
  int RowBgColorCounter; // Counter for alternating background colors (can be
                         // fast-forwarded by e.g clipper), not same as
                         // CurrentRow because header rows typically don't
                         // increase this.
  unsigned int RowBgColor[2]; // Background color override for current row.
  unsigned int BorderColorStrong;
  unsigned int BorderColorLight;
  float BorderX1;
  float BorderX2;
  float HostIndentX;
  float MinColumnWidth;
  float OuterPaddingX;
  float
      CellPaddingX; // Padding from each borders. Locked in BeginTable()/Layout.
  float CellSpacingX1; // Spacing between non-bordered cells. Locked in
                       // BeginTable()/Layout.
  float CellSpacingX2;
  float InnerWidth; // User value passed to BeginTable(), see comments at the
                    // top of BeginTable() for details.
  float ColumnsGivenWidth;   // Sum of current column width
  float ColumnsAutoFitWidth; // Sum of ideal column width in order nothing to be
                             // clipped, used for auto-fitting and content width
                             // submission in outer window
  float ColumnsStretchSumWeights; // Sum of weight of all enabled stretching
                                  // columns
  float ResizedColumnNextWidth;
  float ResizeLockMinContentsX2; // Lock minimum contents width while resizing
                                 // down in order to not create feedback loops.
                                 // But we allow growing the table.
  float RefScale; // Reference scale to be able to rescale columns on font/dpi
                  // changes.
  float AngledHeadersHeight; // Set by TableAngledHeadersRow(), used in
                             // TableUpdateLayout()
  float AngledHeadersSlope;  // Set by TableAngledHeadersRow(), used in
                             // TableUpdateLayout()
  Rect OuterRect; // Note: for non-scrolling table, OuterRect.Max.y is often
                  // FLT_MAX until EndTable(), unless a height has been
                  // specified in BeginTable().
  Rect InnerRect; // InnerRect but without decoration. As with OuterRect, for
                  // non-scrolling tables, InnerRect.Max.y is
  Rect WorkRect;
  Rect InnerClipRect;
  Rect BgClipRect; // We use this to cpu-clip cell background color fill, evolve
                   // during the frame as we cross frozen rows boundaries
  Rect Bg0ClipRectForDrawCmd; // Actual DrawCmd clip rect for BG0/1 channel.
                              // This tends to be == OuterWindow->ClipRect at
                              // BeginTable() because output in BG0/BG1 is
                              // cpu-clipped
  Rect Bg2ClipRectForDrawCmd; // Actual DrawCmd clip rect for BG2 channel.
                              // This tends to be a correct, tight-fit,
                              // because output to BG2 are done by widgets
                              // relying on regular ClipRect.
  Rect HostClipRect; // This is used to check if we can eventually merge our
                     // columns draw calls into the current draw call of the
                     // current window.
  Rect HostBackupInnerClipRect; // Backup of InnerWindow->ClipRect during
                                // PushTableBackground()/PopTableBackground()
  Window *OuterWindow;          // Parent window for the table
  Window *InnerWindow;     // Window holding the table data (== OuterWindow or
                           // a child window)
  TextBuffer ColumnsNames; // Contiguous buffer holding columns names
  DrawListSplitter *DrawSplitter; // Shortcut to TempData->DrawSplitter while in
                                  // table. Isolate draw commands per columns to
                                  // avoid switching clip rect constantly
  TableInstanceData InstanceDataFirst;
  Vector<TableInstanceData>
      InstanceDataExtra; // FIXME-OPT: Using a small-vector pattern would be
                         // good.
  TableColumnSortSpecs SortSpecsSingle;
  Vector<TableColumnSortSpecs>
      SortSpecsMulti; // FIXME-OPT: Using a small-vector pattern would be good.
  TableSortSpecs SortSpecs; // Public facing sorts specs, this is what we
                            // return in TableGetSortSpecs()
  TableColumnIdx SortSpecsCount;
  TableColumnIdx
      ColumnsEnabledCount; // Number of enabled columns (<= ColumnsCount)
  TableColumnIdx
      ColumnsEnabledFixedCount; // Number of enabled columns (<= ColumnsCount)
  TableColumnIdx DeclColumnsCount;   // Count calls to TableSetupColumn()
  TableColumnIdx AngledHeadersCount; // Count columns with angled headers
  TableColumnIdx
      HoveredColumnBody; // Index of column whose visible region is being
                         // hovered. Important: == ColumnsCount when hovering
                         // empty region after the right-most column!
  TableColumnIdx HoveredColumnBorder; // Index of column whose right-border
                                      // is being hovered (for resizing).
  TableColumnIdx
      HighlightColumnHeader; // Index of column which should be highlighted.
  TableColumnIdx
      AutoFitSingleColumn;      // Index of single column requesting auto-fit.
  TableColumnIdx ResizedColumn; // Index of column being resized. Reset
                                // when InstanceCurrent==0.
  TableColumnIdx
      LastResizedColumn; // Index of column being resized from previous frame.
  TableColumnIdx HeldHeaderColumn; // Index of column header being held.
  TableColumnIdx
      ReorderColumn; // Index of column being reordered. (not cleared)
  TableColumnIdx ReorderColumnDir;      // -1 or +1
  TableColumnIdx LeftMostEnabledColumn; // Index of left-most non-hidden column.
  TableColumnIdx
      RightMostEnabledColumn; // Index of right-most non-hidden column.
  TableColumnIdx
      LeftMostStretchedColumn; // Index of left-most stretched column.
  TableColumnIdx
      RightMostStretchedColumn; // Index of right-most stretched column.
  TableColumnIdx
      ContextPopupColumn; // Column right-clicked on, of -1 if opening context
                          // menu from a neutral/empty spot
  TableColumnIdx FreezeRowsRequest; // Requested frozen rows count
  TableColumnIdx
      FreezeRowsCount; // Actual frozen row count (== FreezeRowsRequest, or == 0
                       // when no scrolling offset)
  TableColumnIdx FreezeColumnsRequest; // Requested frozen columns count
  TableColumnIdx FreezeColumnsCount;   // Actual frozen columns count (==
                                       // FreezeColumnsRequest, or == 0 when
                                       // no scrolling offset)
  TableColumnIdx
      RowCellDataCurrent; // Index of current RowCellData[] entry in current row
  TableDrawChannelIdx DummyDrawChannel; // Redirect non-visible columns here.
  TableDrawChannelIdx
      Bg2DrawChannelCurrent; // For Selectable() and other widgets drawing
                             // across columns after the freezing line. Index
                             // within DrawSplitter.Channels[]
  TableDrawChannelIdx Bg2DrawChannelUnfrozen;
  bool IsLayoutLocked; // Set by TableUpdateLayout() which is called when
                       // beginning the first row.
  bool IsInsideRow;    // Set when inside TableBeginRow()/TableEndRow().
  bool IsInitializing;
  bool IsSortSpecsDirty;
  bool IsUsingHeaders;     // Set when the first row had the
                           // TableRowFlags_Headers flag.
  bool IsContextPopupOpen; // Set when default context menu is open (also see:
                           // ContextPopupColumn, InstanceInteracted).
  bool DisableDefaultContextMenu; // Disable default context menu contents. You
                                  // may submit your own using
                                  // TableBeginContextMenuPopup()/EndPopup()
  bool IsSettingsRequestLoad;
  bool IsSettingsDirty; // Set when table settings have changed and needs to be
                        // reported into TableSetttings data.
  bool IsDefaultDisplayOrder; // Set when display order is unchanged from
                              // default (DisplayOrder contains 0...Count-1)
  bool IsResetAllRequest;
  bool IsResetDisplayOrderRequest;
  bool IsUnfrozenRows;        // Set when we got past the frozen row.
  bool IsDefaultSizingPolicy; // Set if user didn't explicitly set a sizing
                              // policy in BeginTable()
  bool IsActiveIdAliveBeforeTable;
  bool IsActiveIdInTable;
  bool HasScrollbarYCurr; // Whether ANY instance of this table had a vertical
                          // scrollbar during the current frame.
  bool HasScrollbarYPrev; // Whether ANY instance of this table had a vertical
                          // scrollbar during the previous.
  bool MemoryCompacted;
  bool HostSkipItems; // Backup of InnerWindow->SkipItem at the end of
                      // BeginTable(), because we will overwrite
                      // InnerWindow->SkipItem on a per-column basis

  Table() {
    memset(this, 0, sizeof(*this));
    LastFrameActive = -1;
  }
  ~Table() { FREE(RawData); }
};

// Transient data that are only needed between BeginTable() and EndTable(),
// those buffers are shared (1 per level of stacked table).
// - Accessing those requires chasing an extra pointer so for very frequently
// used data we leave them in the main table structure.
// - We also leave out of this structure data that tend to be particularly
// useful for debugging/metrics. sizeof() ~ 120 bytes.
struct API TableTempData {
  int TableIndex;                // Index in g.Tables.Buf[] pool
  float LastTimeActive;          // Last timestamp this structure was used
  float AngledheadersExtraWidth; // Used in EndTable()

  Vec2 UserOuterSize; // outer_size.x passed to BeginTable()
  DrawListSplitter DrawSplitter;

  Rect HostBackupWorkRect; // Backup of InnerWindow->WorkRect at the end of
                           // BeginTable()
  Rect HostBackupParentWorkRect; // Backup of InnerWindow->ParentWorkRect at
                                 // the end of BeginTable()
  Vec2 HostBackupPrevLineSize;   // Backup of InnerWindow->DC.PrevLineSize at
                                 // the end of BeginTable()
  Vec2 HostBackupCurrLineSize;   // Backup of InnerWindow->DC.CurrLineSize at
                                 // the end of BeginTable()
  Vec2 HostBackupCursorMaxPos;   // Backup of InnerWindow->DC.CursorMaxPos at
                                 // the end of BeginTable()
  Vec1 HostBackupColumnsOffset;  // Backup of OuterWindow->DC.ColumnsOffset at
                                 // the end of BeginTable()
  float HostBackupItemWidth; // Backup of OuterWindow->DC.ItemWidth at the end
                             // of BeginTable()
  int HostBackupItemWidthStackSize; // Backup of
                                    // OuterWindow->DC.ItemWidthStack.Size at
                                    // the end of BeginTable()

  TableTempData() {
    memset(this, 0, sizeof(*this));
    LastTimeActive = -1.0f;
  }
};

// sizeof() ~ 12
struct TableColumnSettings {
  float WidthOrWeight;
  int UserID;
  TableColumnIdx Index;
  TableColumnIdx DisplayOrder;
  TableColumnIdx SortOrder;
  unsigned char SortDirection : 2;
  unsigned char IsEnabled : 1; // "Visible" in ini file
  unsigned char IsStretch : 1;

  TableColumnSettings() {
    WidthOrWeight = 0.0f;
    UserID = 0;
    Index = -1;
    DisplayOrder = SortOrder = -1;
    SortDirection = SortDirection_None;
    IsEnabled = 1;
    IsStretch = 0;
  }
};

// This is designed to be stored in a single ChunkStream (1 header followed by
// N TableColumnSettings, etc.)
struct TableSettings {
  int ID;         // Set to 0 to invalidate/delete the setting
  int SaveFlags;  // Indicate data we want to save using the
                  // Resizable/Reorderable/Sortable/Hideable flags
                  // (could be using its own flags..)
  float RefScale; // Reference scale to be able to rescale columns on font/dpi
                  // changes.
  TableColumnIdx ColumnsCount;
  TableColumnIdx
      ColumnsCountMax; // Maximum number of columns this settings instance can
                       // store, we can recycle a settings instance with lower
                       // number of columns but not higher
  bool WantApply; // Set when loaded from .ini data (to enable merging/loading
                  // .ini data into an already running context)

  TableSettings() { memset(this, 0, sizeof(*this)); }
  TableColumnSettings *GetColumnSettings() {
    return (TableColumnSettings *)(this + 1);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Gui internal API
// No guarantee of forward compatibility here!
//-----------------------------------------------------------------------------

namespace Gui {
// Windows
// We should always have a CurrentWindow in the stack (there is an implicit
// "Debug" window) If this ever crash because g.CurrentWindow is NULL it means
// that either
// - Gui::NewFrame() has never been called, which is illegal.
// - You are calling Gui functions after Gui::EndFrame()/Gui::Render() and
// before the next Gui::NewFrame(), which is also illegal.
inline Window *GetCurrentWindowRead() {
  Context &g = *GGui;
  return g.CurrentWindow;
}
inline Window *GetCurrentWindow() {
  Context &g = *GGui;
  g.CurrentWindow->WriteAccessed = true;
  return g.CurrentWindow;
}
API Window *FindWindowByID(int id);
API Window *FindWindowByName(const char *name);
API void UpdateWindowParentAndRootLinks(Window *window, int flags,
                                        Window *parent_window);
API Vec2 CalcWindowNextAutoFitSize(Window *window);
API bool IsWindowChildOf(Window *window, Window *potential_parent,
                         bool popup_hierarchy, bool dock_hierarchy);
API bool IsWindowWithinBeginStackOf(Window *window, Window *potential_parent);
API bool IsWindowAbove(Window *potential_above, Window *potential_below);
API bool IsWindowNavFocusable(Window *window);
API void SetWindowPos(Window *window, const Vec2 &pos, int cond = 0);
API void SetWindowSize(Window *window, const Vec2 &size, int cond = 0);
API void SetWindowCollapsed(Window *window, bool collapsed, int cond = 0);
API void SetWindowHitTestHole(Window *window, const Vec2 &pos,
                              const Vec2 &size);
API void SetWindowHiddenAndSkipItemsForCurrentFrame(Window *window);
inline Rect WindowRectAbsToRel(Window *window, const Rect &r) {
  Vec2 off = window->DC.CursorStartPos;
  return Rect(r.Min.x - off.x, r.Min.y - off.y, r.Max.x - off.x,
              r.Max.y - off.y);
}
inline Rect WindowRectRelToAbs(Window *window, const Rect &r) {
  Vec2 off = window->DC.CursorStartPos;
  return Rect(r.Min.x + off.x, r.Min.y + off.y, r.Max.x + off.x,
              r.Max.y + off.y);
}
inline Vec2 WindowPosRelToAbs(Window *window, const Vec2 &p) {
  Vec2 off = window->DC.CursorStartPos;
  return Vec2(p.x + off.x, p.y + off.y);
}

// Windows: Display Order and Focus Order
API void FocusWindow(Window *window, int flags = 0);
API void FocusTopMostWindowUnderOne(Window *under_this_window,
                                    Window *ignore_window,
                                    Viewport *filter_viewport, int flags);
API void BringWindowToFocusFront(Window *window);
API void BringWindowToDisplayFront(Window *window);
API void BringWindowToDisplayBack(Window *window);
API void BringWindowToDisplayBehind(Window *window, Window *above_window);
API int FindWindowDisplayIndex(Window *window);
API Window *FindBottomMostVisibleWindowWithinBeginStack(Window *window);

// Fonts, drawing
API void SetCurrentFont(Font *font);
inline Font *GetDefaultFont() {
  Context &g = *GGui;
  return g.IO.FontDefault ? g.IO.FontDefault : g.IO.Fonts->Fonts[0];
}
inline DrawList *GetForegroundDrawList(Window *window) {
  return GetForegroundDrawList(window->Viewport);
}
API void AddDrawListToDrawDataEx(DrawData *draw_data,
                                 Vector<DrawList *> *out_list,
                                 DrawList *draw_list);

// Init
API void Initialize();
API void Shutdown(); // Since 1.60 this is a _private_ function. You can call
                     // DestroyContext() to destroy the context created by
                     // CreateContext().

// NewFrame
API void UpdateInputEvents(bool trickle_fast_inputs);
API void UpdateHoveredWindowAndCaptureFlags();
API void StartMouseMovingWindow(Window *window);
API void StartMouseMovingWindowOrNode(Window *window, DockNode *node,
                                      bool undock);
API void UpdateMouseMovingWindowNewFrame();
API void UpdateMouseMovingWindowEndFrame();

// Generic context hooks
API int AddContextHook(Context *context, const ContextHook *hook);
API void RemoveContextHook(Context *context, int hook_to_remove);
API void CallContextHooks(Context *context, ContextHookType type);

// Viewports
API void TranslateWindowsInViewport(ViewportP *viewport, const Vec2 &old_pos,
                                    const Vec2 &new_pos);
API void ScaleWindowsInViewport(ViewportP *viewport, float scale);
API void DestroyPlatformWindow(ViewportP *viewport);
API void SetWindowViewport(Window *window, ViewportP *viewport);
API void SetCurrentViewport(Window *window, ViewportP *viewport);
API const PlatformMonitor *GetViewportPlatformMonitor(Viewport *viewport);
API ViewportP *
FindHoveredViewportFromPlatformWindowStack(const Vec2 &mouse_platform_pos);

// Settings
API void MarkIniSettingsDirty();
API void MarkIniSettingsDirty(Window *window);
API void ClearIniSettings();
API void AddSettingsHandler(const SettingsHandler *handler);
API void RemoveSettingsHandler(const char *type_name);
API SettingsHandler *FindSettingsHandler(const char *type_name);

// Settings - Windows
API WindowSettings *CreateNewWindowSettings(const char *name);
API WindowSettings *FindWindowSettingsByID(int id);
API WindowSettings *FindWindowSettingsByWindow(Window *window);
API void ClearWindowSettings(const char *name);

// Localization
API void LocalizeRegisterEntries(const LocEntry *entries, int count);
inline const char *LocalizeGetMsg(LocKey key) {
  Context &g = *GGui;
  const char *msg = g.LocalizationTable[key];
  return msg ? msg : "*Missing Text*";
}

// Scrolling
API void SetScrollX(Window *window, float scroll_x);
API void SetScrollY(Window *window, float scroll_y);
API void SetScrollFromPosX(Window *window, float local_x, float center_x_ratio);
API void SetScrollFromPosY(Window *window, float local_y, float center_y_ratio);

// Early work-in-progress API (ScrollToItem() will become public)
API void ScrollToItem(int flags = 0);
API void ScrollToRect(Window *window, const Rect &rect, int flags = 0);
API Vec2 ScrollToRectEx(Window *window, const Rect &rect, int flags = 0);
// #ifndef DISABLE_OBSOLETE_FUNCTIONS
inline void ScrollToBringRectIntoView(Window *window, const Rect &rect) {
  ScrollToRect(window, rect, ScrollFlags_KeepVisibleEdgeY);
}
// #endif

// Basic Accessors
inline int GetItemStatusFlags() {
  Context &g = *GGui;
  return g.LastItemData.StatusFlags;
}
inline int GetItemFlags() {
  Context &g = *GGui;
  return g.LastItemData.InFlags;
}
inline int GetActiveID() {
  Context &g = *GGui;
  return g.ActiveId;
}
inline int GetFocusID() {
  Context &g = *GGui;
  return g.NavId;
}
API void SetActiveID(int id, Window *window);
API void SetFocusID(int id, Window *window);
API void ClearActiveID();
API int GetHoveredID();
API void SetHoveredID(int id);
API void KeepAliveID(int id);
API void
MarkItemEdited(int id); // Mark data associated to given item as "edited",
                        // used by IsItemDeactivatedAfterEdit() function.
API void
PushOverrideID(int id); // Push given value as-is at the top of the int stack
                        // (whereas PushID combines old and new hashes)
API int GetIDWithSeed(const char *str_id_begin, const char *str_id_end,
                      int seed);
API int GetIDWithSeed(int n, int seed);

// Basic Helpers for widget code
API void ItemSize(const Vec2 &size, float text_baseline_y = -1.0f);
inline void ItemSize(const Rect &bb, float text_baseline_y = -1.0f) {
  ItemSize(bb.GetSize(), text_baseline_y);
} // FIXME: This is a misleading API since we expect CursorPos to be bb.Min.
API bool ItemAdd(const Rect &bb, int id, const Rect *nav_bb = NULL,
                 int extra_flags = 0);
API bool ItemHoverable(const Rect &bb, int id, int item_flags);
API bool IsWindowContentHoverable(Window *window, int flags = 0);
API bool IsClippedEx(const Rect &bb, int id);
API void SetLastItemData(int item_id, int in_flags, int status_flags,
                         const Rect &item_rect);
API Vec2 CalcItemSize(Vec2 size, float default_w, float default_h);
API float CalcWrapWidthForPos(const Vec2 &pos, float wrap_pos_x);
API void PushMultiItemsWidths(int components, float width_full);
API bool IsItemToggledSelection(); // Was the last item selection toggled?
                                   // (after Selectable(), TreeNode() etc. We
                                   // only returns toggle _event_ in order to
                                   // handle clipping correctly)
API Vec2 GetContentRegionMaxAbs();
API void ShrinkWidths(ShrinkWidthItem *items, int count, float width_excess);

// Parameter stacks (shared)
API void PushItemFlag(int option, bool enabled);
API void PopItemFlag();
API const DataVarInfo *GetStyleVarInfo(int idx);

// Logging/Capture
API void LogBegin(
    LogType type,
    int auto_open_depth); // -> BeginCapture() when we design v2 api, for now
                          // stay under the radar by using the old name.
API void LogToBuffer(
    int auto_open_depth = -1); // Start logging/capturing to internal buffer
API void LogRenderedText(const Vec2 *ref_pos, const char *text,
                         const char *text_end = NULL);
API void LogSetNextTextDecoration(const char *prefix, const char *suffix);

// Popups, Modals, Tooltips
API bool BeginChildEx(const char *name, int id, const Vec2 &size_arg,
                      int child_flags, int window_flags);
API void OpenPopupEx(int id, int popup_flags = PopupFlags_None);
API void ClosePopupToLevel(int remaining,
                           bool restore_focus_to_window_under_popup);
API void ClosePopupsOverWindow(Window *ref_window,
                               bool restore_focus_to_window_under_popup);
API void ClosePopupsExceptModals();
API bool IsPopupOpen(int id, int popup_flags);
API bool BeginPopupEx(int id, int extra_flags);
API bool BeginTooltipEx(int tooltip_flags, int extra_window_flags);
API bool BeginTooltipHidden();
API Rect GetPopupAllowedExtentRect(Window *window);
API Window *GetTopMostPopupModal();
API Window *GetTopMostAndVisiblePopupModal();
API Window *FindBlockingModal(Window *window);
API Vec2 FindBestWindowPosForPopup(Window *window);
API Vec2 FindBestWindowPosForPopupEx(const Vec2 &ref_pos, const Vec2 &size,
                                     int *last_dir, const Rect &r_outer,
                                     const Rect &r_avoid,
                                     PopupPositionPolicy policy);

// Menus
API bool BeginViewportSideBar(const char *name, Viewport *viewport, int dir,
                              float size, int window_flags);
API bool BeginMenuEx(const char *label, const char *icon, bool enabled = true);
API bool MenuItemEx(const char *label, const char *icon,
                    const char *shortcut = NULL, bool selected = false,
                    bool enabled = true);

// Combos
API bool BeginComboPopup(int popup_id, const Rect &bb, int flags);
API bool BeginComboPreview();
API void EndComboPreview();

// Gamepad/Keyboard Navigation
API void NavInitWindow(Window *window, bool force_reinit);
API void NavInitRequestApplyResult();
API bool NavMoveRequestButNoResultYet();
API void NavMoveRequestSubmit(int move_dir, int clip_dir, int move_flags,
                              int scroll_flags);
API void NavMoveRequestForward(int move_dir, int clip_dir, int move_flags,
                               int scroll_flags);
API void NavMoveRequestResolveWithLastItem(NavItemData *result);
API void NavMoveRequestResolveWithPastTreeNode(NavItemData *result,
                                               NavTreeNodeData *tree_node_data);
API void NavMoveRequestCancel();
API void NavMoveRequestApplyResult();
API void NavMoveRequestTryWrapping(Window *window, int move_flags);
API void NavClearPreferredPosForAxis(Axis axis);
API void NavRestoreHighlightAfterMove();
API void NavUpdateCurrentWindowIsScrollPushableX();
API void SetNavWindow(Window *window);
API void SetNavID(int id, NavLayer nav_layer, int focus_scope_id,
                  const Rect &rect_rel);

// Focus/Activation
// This should be part of a larger set of API: FocusItem(offset = -1),
// FocusItemByID(id), ActivateItem(offset = -1), ActivateItemByID(id) etc. which
// are much harder to design and implement than expected. I have a couple of
// private branches on this matter but it's not simple. For now implementing the
// easy ones.
API void FocusItem(); // Focus last item (no selection/activation).
API void
ActivateItemByID(int id); // Activate an item by int (button, checkbox, tree
                          // node etc.). Activation is queued and processed on
                          // the next frame when the item is encountered again.

// Inputs
// FIXME: Eventually we should aim to move e.g. IsActiveIdUsingKey() into
// IsKeyXXX functions.
inline bool IsNamedKey(Key key) {
  return key >= Key_NamedKey_BEGIN && key < Key_NamedKey_END;
}
inline bool IsNamedKeyOrModKey(Key key) {
  return (key >= Key_NamedKey_BEGIN && key < Key_NamedKey_END) ||
         key == Mod_Ctrl || key == Mod_Shift || key == Mod_Alt ||
         key == Mod_Super || key == Mod_Shortcut;
}
inline bool IsLegacyKey(Key key) {
  return key >= Key_LegacyNativeKey_BEGIN && key < Key_LegacyNativeKey_END;
}
inline bool IsKeyboardKey(Key key) {
  return key >= Key_Keyboard_BEGIN && key < Key_Keyboard_END;
}
inline bool IsGamepadKey(Key key) {
  return key >= Key_Gamepad_BEGIN && key < Key_Gamepad_END;
}
inline bool IsMouseKey(Key key) {
  return key >= Key_Mouse_BEGIN && key < Key_Mouse_END;
}
inline bool IsAliasKey(Key key) {
  return key >= Key_Aliases_BEGIN && key < Key_Aliases_END;
}
inline int ConvertShortcutMod(int key_chord) {
  Context &g = *GGui;
  ASSERT_PARANOID(key_chord & Mod_Shortcut);
  return (key_chord & ~Mod_Shortcut) |
         (g.IO.ConfigMacOSXBehaviors ? Mod_Super : Mod_Ctrl);
}
inline Key ConvertSingleModFlagToKey(Context *ctx, Key key) {
  Context &g = *ctx;
  if (key == Mod_Ctrl)
    return Key_ReservedForModCtrl;
  if (key == Mod_Shift)
    return Key_ReservedForModShift;
  if (key == Mod_Alt)
    return Key_ReservedForModAlt;
  if (key == Mod_Super)
    return Key_ReservedForModSuper;
  if (key == Mod_Shortcut)
    return (g.IO.ConfigMacOSXBehaviors ? Key_ReservedForModSuper
                                       : Key_ReservedForModCtrl);
  return key;
}

API KeyData *GetKeyData(Context *ctx, Key key);
inline KeyData *GetKeyData(Key key) {
  Context &g = *GGui;
  return GetKeyData(&g, key);
}
API void GetKeyChordName(int key_chord, char *out_buf, int out_buf_size);
inline Key MouseButtonToKey(int button) {
  assert(button >= 0 && button < MouseButton_COUNT);
  return (Key)(Key_MouseLeft + button);
}
API bool IsMouseDragPastThreshold(int button, float lock_threshold = -1.0f);
API Vec2 GetKeyMagnitude2d(Key key_left, Key key_right, Key key_up,
                           Key key_down);
API float GetNavTweakPressedAmount(Axis axis);
API int CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay,
                                  float repeat_rate);
API void GetTypematicRepeatRate(int flags, float *repeat_delay,
                                float *repeat_rate);
API void TeleportMousePos(const Vec2 &pos);
API void SetActiveIdUsingAllKeyboardKeys();
inline bool IsActiveIdUsingNavDir(int dir) {
  Context &g = *GGui;
  return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0;
}

// [EXPERIMENTAL] Low-Level: Key/Input Ownership
// - The idea is that instead of "eating" a given input, we can link to an owner
// id.
// - Ownership is most often claimed as a result of reacting to a press/down
// event (but occasionally may be claimed ahead).
// - Input queries can then read input by specifying KeyOwner_Any (== 0),
// KeyOwner_None (== -1) or a custom unsigned int.
// - Legacy input queries (without specifying an owner or _Any or _None) are
// equivalent to using KeyOwner_Any (== 0).
// - Input ownership is automatically released on the frame after a key is
// released. Therefore:
//   - for ownership registration happening as a result of a down/press event,
//   the SetKeyOwner() call may be done once (common case).
//   - for ownership registration happening ahead of a down/press event, the
//   SetKeyOwner() call needs to be made every frame (happens if e.g. claiming
//   ownership on hover).
// - SetItemKeyOwner() is a shortcut for common simple case. A custom widget
// will probably want to call SetKeyOwner() multiple times directly based on its
// interaction state.
// - This is marked experimental because not all widgets are fully honoring the
// Set/Test idioms. We will need to move forward step by step.
//   Please open a GitHub Issue to submit your usage scenario or if there's a
//   use case you need solved.
API int GetKeyOwner(Key key);
API void SetKeyOwner(Key key, int owner_id, int flags = 0);
API void SetKeyOwnersForKeyChord(int key, int owner_id, int flags = 0);
API void SetItemKeyOwner(
    Key key,
    int flags = 0); // Set key owner to last item if it is hovered or
                    // active. Equivalent to 'if (IsItemHovered() ||
                    // IsItemActive()) { SetKeyOwner(key, GetItemID());'.
API bool TestKeyOwner(Key key,
                      int owner_id); // Test that key is either not owned,
                                     // either owned by 'owner_id'
inline KeyOwnerData *GetKeyOwnerData(Context *ctx, Key key) {
  if (key & Mod_Mask_)
    key = ConvertSingleModFlagToKey(ctx, key);
  assert(IsNamedKey(key));
  return &ctx->KeysOwnerData[key - Key_NamedKey_BEGIN];
}

// [EXPERIMENTAL] High-Level: Input Access functions w/ support for Key/Input
// Ownership
// - Important: legacy IsKeyPressed(Key, bool repeat=true) _DEFAULTS_ to
// repeat, new IsKeyPressed() requires _EXPLICIT_ InputFlags_Repeat flag.
// - Expected to be later promoted to public API, the prototypes are designed to
// replace existing ones (since owner_id can default to Any == 0)
// - Specifying a value for 'unsigned int owner' will test that EITHER the key
// is NOT owned (UNLESS locked), EITHER the key is owned by 'owner'.
//   Legacy functions use KeyOwner_Any meaning that they typically ignore
//   ownership, unless a call to SetKeyOwner() explicitly used
//   InputFlags_LockThisFrame or InputFlags_LockUntilRelease.
// - Binding generators may want to ignore those for now, or suffix them with
// Ex() until we decide if this gets moved into public API.
API bool IsKeyDown(Key key, int owner_id);
API bool IsKeyPressed(
    Key key, int owner_id,
    int flags); // Important: when transitioning from old to new IsKeyPressed():
                // old API has "bool repeat = true", so would default to repeat.
                // New API requiress explicit InputFlags_Repeat.
API bool IsKeyReleased(Key key, int owner_id);
API bool IsMouseDown(int button, int owner_id);
API bool IsMouseClicked(int button, int owner_id, int flags = 0);
API bool IsMouseReleased(int button, int owner_id);
API bool IsMouseDoubleClicked(int button, int owner_id);

// [EXPERIMENTAL] Shortcut Routing
// - int = a Key optionally OR-red with
// Mod_Alt/Mod_Ctrl/Mod_Shift/Mod_Super.
//     Key_C                 (accepted by functions taking Key or
//     int) Key_C | Mod_Ctrl (accepted by functions taking
//     int)
//   ONLY Mod_XXX values are legal to 'OR' with an Key. You CANNOT
//   'OR' two Key values.
// - When using one of the routing flags (e.g. InputFlags_RouteFocused):
// routes requested ahead of time given a chord (key + modifiers) and a routing
// policy.
// - Routes are resolved during NewFrame(): if keyboard modifiers are matching
// current ones: SetKeyOwner() is called + route is granted for the frame.
// - Route is granted to a single owner. When multiple requests are made we have
// policies to select the winning route.
// - Multiple read sites may use the same owner id and will all get the granted
// route.
// - For routing: when owner_id is 0 we use the current Focus Scope int as a
// default owner in order to identify our location.
// - TL;DR;
//   - IsKeyChordPressed() compares mods + call IsKeyPressed() -> function has
//   no side-effect.
//   - Shortcut() submits a route then if currently can be routed calls
//   IsKeyChordPressed() -> function has (desirable) side-effects.
API bool IsKeyChordPressed(int key_chord, int owner_id, int flags = 0);
API bool Shortcut(int key_chord, int owner_id = 0, int flags = 0);
API bool SetShortcutRouting(int key_chord, int owner_id = 0, int flags = 0);
API bool TestShortcutRouting(int key_chord, int owner_id);
API KeyRoutingData *GetShortcutRoutingData(int key_chord);

// Docking
// (some functions are only declared in gui.cpp, see Docking section)
API void DockContextInitialize(Context *ctx);
API void DockContextShutdown(Context *ctx);
API void
DockContextClearNodes(Context *ctx, int root_id,
                      bool clear_settings_refs); // Use root_id==0 to clear all
API void DockContextRebuildNodes(Context *ctx);
API void DockContextNewFrameUpdateUndocking(Context *ctx);
API void DockContextNewFrameUpdateDocking(Context *ctx);
API void DockContextEndFrame(Context *ctx);
API int DockContextGenNodeID(Context *ctx);
API void DockContextQueueDock(Context *ctx, Window *target,
                              DockNode *target_node, Window *payload,
                              int split_dir, float split_ratio,
                              bool split_outer);
API void DockContextQueueUndockWindow(Context *ctx, Window *window);
API void DockContextQueueUndockNode(Context *ctx, DockNode *node);
API void
DockContextProcessUndockWindow(Context *ctx, Window *window,
                               bool clear_persistent_docking_ref = true);
API void DockContextProcessUndockNode(Context *ctx, DockNode *node);
API bool DockContextCalcDropPosForDocking(Window *target, DockNode *target_node,
                                          Window *payload_window,
                                          DockNode *payload_node, int split_dir,
                                          bool split_outer, Vec2 *out_pos);
API DockNode *DockContextFindNodeByID(Context *ctx, int id);
API void DockNodeWindowMenuHandler_Default(Context *ctx, DockNode *node,
                                           TabBar *tab_bar);
API bool DockNodeBeginAmendTabBar(DockNode *node);
API void DockNodeEndAmendTabBar();
inline DockNode *DockNodeGetRootNode(DockNode *node) {
  while (node->ParentNode)
    node = node->ParentNode;
  return node;
}
inline bool DockNodeIsInHierarchyOf(DockNode *node, DockNode *parent) {
  while (node) {
    if (node == parent)
      return true;
    node = node->ParentNode;
  }
  return false;
}
inline int DockNodeGetDepth(const DockNode *node) {
  int depth = 0;
  while (node->ParentNode) {
    node = node->ParentNode;
    depth++;
  }
  return depth;
}
inline int DockNodeGetWindowMenuButtonId(const DockNode *node) {
  return HashStr("#COLLAPSE", 0, node->ID);
}
inline DockNode *GetWindowDockNode() {
  Context &g = *GGui;
  return g.CurrentWindow->DockNode;
}
API bool GetWindowAlwaysWantOwnTabBar(Window *window);
API void BeginDocked(Window *window, bool *p_open);
API void BeginDockableDragDropSource(Window *window);
API void BeginDockableDragDropTarget(Window *window);
API void SetWindowDock(Window *window, int dock_id, int cond);

// Docking - Builder function needs to be generally called before the node is
// used/submitted.
// - The DockBuilderXXX functions are designed to _eventually_ become a public
// API, but it is too early to expose it and guarantee stability.
// - Do not hold on DockNode* pointers! They may be invalidated by any
// split/merge/remove operation and every frame.
// - To create a DockSpace() node, make sure to set the
// DockNodeFlags_DockSpace flag when calling DockBuilderAddNode().
//   You can create dockspace nodes (attached to a window) _or_ floating nodes
//   (carry its own window) with this API.
// - DockBuilderSplitNode() create 2 child nodes within 1 node. The initial node
// becomes a parent node.
// - If you intend to split the node immediately after creation using
// DockBuilderSplitNode(), make sure
//   to call DockBuilderSetNodeSize() beforehand. If you don't, the resulting
//   split sizes may not be reliable.
// - Call DockBuilderFinish() after you are done.
API void DockBuilderDockWindow(const char *window_name, int node_id);
API DockNode *DockBuilderGetNode(int node_id);
inline DockNode *DockBuilderGetCentralNode(int node_id) {
  DockNode *node = DockBuilderGetNode(node_id);
  if (!node)
    return NULL;
  return DockNodeGetRootNode(node)->CentralNode;
}
API int DockBuilderAddNode(int node_id = 0, int flags = 0);
API void DockBuilderRemoveNode(
    int node_id); // Remove node and all its child, undock all windows
API void DockBuilderRemoveNodeDockedWindows(int node_id,
                                            bool clear_settings_refs = true);
API void DockBuilderRemoveNodeChildNodes(
    int node_id); // Remove all split/hierarchy. All remaining docked windows
                  // will be re-docked to the remaining root node (node_id).
API void DockBuilderSetNodePos(int node_id, Vec2 pos);
API void DockBuilderSetNodeSize(int node_id, Vec2 size);
API int DockBuilderSplitNode(
    int node_id, int split_dir, float size_ratio_for_node_at_dir,
    int *out_id_at_dir,
    int *out_id_at_opposite_dir); // Create 2 child nodes in this parent node.
API void DockBuilderCopyDockSpace(int src_dockspace_id, int dst_dockspace_id,
                                  Vector<const char *> *in_window_remap_pairs);
API void DockBuilderCopyNode(int src_node_id, int dst_node_id,
                             Vector<unsigned int> *out_node_remap_pairs);
API void DockBuilderCopyWindowSettings(const char *src_name,
                                       const char *dst_name);
API void DockBuilderFinish(int node_id);

// [EXPERIMENTAL] Focus Scope
// This is generally used to identify a unique input location (for e.g. a
// selection set) There is one per window (automatically set in Begin), but:
// - Selection patterns generally need to react (e.g. clear a selection) when
// landing on one item of the set.
//   So in order to identify a set multiple lists in same window may each need a
//   focus scope. If you imagine an hypothetical
//   BeginSelectionGroup()/EndSelectionGroup() api, it would likely call
//   PushFocusScope()/EndFocusScope()
// - Shortcut routing also use focus scope as a default location identifier if
// an owner is not provided. We don't use the int Stack for this as it is common
// to want them separate.
API void PushFocusScope(int id);
API void PopFocusScope();
inline int GetCurrentFocusScope() {
  Context &g = *GGui;
  return g.CurrentFocusScopeId;
} // Focus scope we are outputting into, set by PushFocusScope()

// Drag and Drop
API bool IsDragDropActive();
API bool BeginDragDropTargetCustom(const Rect &bb, int id);
API void ClearDragDrop();
API bool IsDragDropPayloadBeingAccepted();
API void RenderDragDropTargetRect(const Rect &bb, const Rect &item_clip_rect);

// Typing-Select API
API TypingSelectRequest *
GetTypingSelectRequest(int flags = TypingSelectFlags_None);
API int TypingSelectFindMatch(TypingSelectRequest *req, int items_count,
                              const char *(*get_item_name_func)(void *, int),
                              void *user_data, int nav_item_idx);
API int TypingSelectFindNextSingleCharMatch(
    TypingSelectRequest *req, int items_count,
    const char *(*get_item_name_func)(void *, int), void *user_data,
    int nav_item_idx);
API int
TypingSelectFindBestLeadingMatch(TypingSelectRequest *req, int items_count,
                                 const char *(*get_item_name_func)(void *, int),
                                 void *user_data);

// Internal Columns API (this is not exposed because we will encourage
// transitioning to the Tables API)
API void SetWindowClipRectBeforeSetChannel(Window *window,
                                           const Rect &clip_rect);
API void BeginColumns(
    const char *str_id, int count,
    int flags = 0); // setup number of columns. use an identifier to distinguish
                    // multiple column sets. close with EndColumns().
API void EndColumns(); // close columns
API void PushColumnClipRect(int column_index);
API void PushColumnsBackground();
API void PopColumnsBackground();
API int GetColumnsID(const char *str_id, int count);
API OldColumns *FindOrCreateColumns(Window *window, int id);
API float GetColumnOffsetFromNorm(const OldColumns *columns, float offset_norm);
API float GetColumnNormFromOffset(const OldColumns *columns, float offset);

// Tables: Candidates for public API
API void TableOpenContextMenu(int column_n = -1);
API void TableSetColumnWidth(int column_n, float width);
API void TableSetColumnSortDirection(int column_n, int sort_direction,
                                     bool append_to_sort_specs);
API int
TableGetHoveredColumn(); // May use (TableGetColumnFlags() &
                         // TableColumnFlags_IsHovered) instead. Return
                         // hovered column. return -1 when table is not hovered.
                         // return columns_count if the unused space at the
                         // right of visible columns is hovered.
API int TableGetHoveredRow(); // Retrieve *PREVIOUS FRAME* hovered row. This
                              // difference with TableGetHoveredColumn() is the
                              // reason why this is not public yet.
API float TableGetHeaderRowHeight();
API float TableGetHeaderAngledMaxLabelWidth();
API void TablePushBackgroundChannel();
API void TablePopBackgroundChannel();
API void TableAngledHeadersRowEx(float angle, float label_width = 0.0f);

// Tables: Internals
inline Table *GetCurrentTable() {
  Context &g = *GGui;
  return g.CurrentTable;
}
API Table *TableFindByID(int id);
API bool BeginTableEx(const char *name, int id, int columns_count,
                      int flags = 0, const Vec2 &outer_size = Vec2(0, 0),
                      float inner_width = 0.0f);
API void TableBeginInitMemory(Table *table, int columns_count);
API void TableBeginApplyRequests(Table *table);
API void TableSetupDrawChannels(Table *table);
API void TableUpdateLayout(Table *table);
API void TableUpdateBorders(Table *table);
API void TableUpdateColumnsWeightFromWidth(Table *table);
API void TableDrawBorders(Table *table);
API void TableDrawDefaultContextMenu(Table *table,
                                     int flags_for_section_to_display);
API bool TableBeginContextMenuPopup(Table *table);
API void TableMergeDrawChannels(Table *table);
inline TableInstanceData *TableGetInstanceData(Table *table, int instance_no) {
  if (instance_no == 0)
    return &table->InstanceDataFirst;
  return &table->InstanceDataExtra[instance_no - 1];
}
inline int TableGetInstanceID(Table *table, int instance_no) {
  return TableGetInstanceData(table, instance_no)->TableInstanceID;
}
API void TableSortSpecsSanitize(Table *table);
API void TableSortSpecsBuild(Table *table);
API int TableGetColumnNextSortDirection(TableColumn *column);
API void TableFixColumnSortDirection(Table *table, TableColumn *column);
API float TableGetColumnWidthAuto(Table *table, TableColumn *column);
API void TableBeginRow(Table *table);
API void TableEndRow(Table *table);
API void TableBeginCell(Table *table, int column_n);
API void TableEndCell(Table *table);
API Rect TableGetCellBgRect(const Table *table, int column_n);
API const char *TableGetColumnName(const Table *table, int column_n);
API int TableGetColumnResizeID(Table *table, int column_n, int instance_no = 0);
API float TableGetMaxColumnWidth(const Table *table, int column_n);
API void TableSetColumnWidthAutoSingle(Table *table, int column_n);
API void TableSetColumnWidthAutoAll(Table *table);
API void TableRemove(Table *table);
API void TableGcCompactTransientBuffers(Table *table);
API void TableGcCompactTransientBuffers(TableTempData *table);
API void TableGcCompactSettings();

// Tables: Settings
API void TableLoadSettings(Table *table);
API void TableSaveSettings(Table *table);
API void TableResetSettings(Table *table);
API TableSettings *TableGetBoundSettings(Table *table);
API void TableSettingsAddSettingsHandler();
API TableSettings *TableSettingsCreate(int id, int columns_count);
API TableSettings *TableSettingsFindByID(int id);

// Tab Bars
inline TabBar *GetCurrentTabBar() {
  Context &g = *GGui;
  return g.CurrentTabBar;
}
API bool BeginTabBarEx(TabBar *tab_bar, const Rect &bb, int flags);
API TabItem *TabBarFindTabByID(TabBar *tab_bar, int tab_id);
API TabItem *TabBarFindTabByOrder(TabBar *tab_bar, int order);
API TabItem *TabBarFindMostRecentlySelectedTabForActiveWindow(TabBar *tab_bar);
API TabItem *TabBarGetCurrentTab(TabBar *tab_bar);
inline int TabBarGetTabOrder(TabBar *tab_bar, TabItem *tab) {
  return tab_bar->Tabs.index_from_ptr(tab);
}
API const char *TabBarGetTabName(TabBar *tab_bar, TabItem *tab);
API void TabBarAddTab(TabBar *tab_bar, int tab_flags, Window *window);
API void TabBarRemoveTab(TabBar *tab_bar, int tab_id);
API void TabBarCloseTab(TabBar *tab_bar, TabItem *tab);
API void TabBarQueueFocus(TabBar *tab_bar, TabItem *tab);
API void TabBarQueueReorder(TabBar *tab_bar, TabItem *tab, int offset);
API void TabBarQueueReorderFromMousePos(TabBar *tab_bar, TabItem *tab,
                                        Vec2 mouse_pos);
API bool TabBarProcessReorder(TabBar *tab_bar);
API bool TabItemEx(TabBar *tab_bar, const char *label, bool *p_open, int flags,
                   Window *docked_window);
API Vec2 TabItemCalcSize(const char *label,
                         bool has_close_button_or_unsaved_marker);
API Vec2 TabItemCalcSize(Window *window);
API void TabItemBackground(DrawList *draw_list, const Rect &bb, int flags,
                           unsigned int col);
API void
TabItemLabelAndCloseButton(DrawList *draw_list, const Rect &bb, int flags,
                           Vec2 frame_padding, const char *label, int tab_id,
                           int close_button_id, bool is_contents_visible,
                           bool *out_just_closed, bool *out_text_clipped);

// Render helpers
// AVOID USING OUTSIDE OF GUI.CPP! NOT FOR PUBLIC CONSUMPTION. THOSE FUNCTIONS
// ARE A MESS. THEIR SIGNATURE AND BEHAVIOR WILL CHANGE, THEY NEED TO BE
// REFACTORED INTO SOMETHING DECENT. NB: All position are in absolute pixels
// coordinates (we are never using window coordinates internally)
API void RenderText(Vec2 pos, const char *text, const char *text_end = NULL,
                    bool hide_text_after_hash = true);
API void RenderTextWrapped(Vec2 pos, const char *text, const char *text_end,
                           float wrap_width);
API void RenderTextClipped(const Vec2 &pos_min, const Vec2 &pos_max,
                           const char *text, const char *text_end,
                           const Vec2 *text_size_if_known,
                           const Vec2 &align = Vec2(0, 0),
                           const Rect *clip_rect = NULL);
API void RenderTextClippedEx(DrawList *draw_list, const Vec2 &pos_min,
                             const Vec2 &pos_max, const char *text,
                             const char *text_end,
                             const Vec2 *text_size_if_known,
                             const Vec2 &align = Vec2(0, 0),
                             const Rect *clip_rect = NULL);
API void RenderTextEllipsis(DrawList *draw_list, const Vec2 &pos_min,
                            const Vec2 &pos_max, float clip_max_x,
                            float ellipsis_max_x, const char *text,
                            const char *text_end,
                            const Vec2 *text_size_if_known);
API void RenderFrame(Vec2 p_min, Vec2 p_max, unsigned int fill_col,
                     bool border = true, float rounding = 0.0f);
API void RenderFrameBorder(Vec2 p_min, Vec2 p_max, float rounding = 0.0f);
API void RenderColorRectWithAlphaCheckerboard(DrawList *draw_list, Vec2 p_min,
                                              Vec2 p_max, unsigned int fill_col,
                                              float grid_step, Vec2 grid_off,
                                              float rounding = 0.0f,
                                              int flags = 0);
API void RenderNavHighlight(
    const Rect &bb, int id,
    int flags = NavHighlightFlags_TypeDefault); // Navigation highlight
API const char *FindRenderedTextEnd(
    const char *text,
    const char *text_end =
        NULL); // Find the optional ## from which we stop displaying text.
API void RenderMouseCursor(Vec2 pos, float scale, int mouse_cursor,
                           unsigned int col_fill, unsigned int col_border,
                           unsigned int col_shadow);

// Render helpers (those functions don't access any Gui state!)
API void RenderArrow(DrawList *draw_list, Vec2 pos, unsigned int col, int dir,
                     float scale = 1.0f);
API void RenderBullet(DrawList *draw_list, Vec2 pos, unsigned int col);
API void RenderCheckMark(DrawList *draw_list, Vec2 pos, unsigned int col,
                         float sz);
API void RenderArrowPointingAt(DrawList *draw_list, Vec2 pos, Vec2 half_sz,
                               int direction, unsigned int col);
API void RenderArrowDockMenu(DrawList *draw_list, Vec2 p_min, float sz,
                             unsigned int col);
API void RenderRectFilledRangeH(DrawList *draw_list, const Rect &rect,
                                unsigned int col, float x_start_norm,
                                float x_end_norm, float rounding);
API void RenderRectFilledWithHole(DrawList *draw_list, const Rect &outer,
                                  const Rect &inner, unsigned int col,
                                  float rounding);
API int CalcRoundingFlagsForRectInRect(const Rect &r_in, const Rect &r_outer,
                                       float threshold);

// Widgets
API void TextEx(const char *text, const char *text_end = NULL, int flags = 0);
API bool ButtonEx(const char *label, const Vec2 &size_arg = Vec2(0, 0),
                  int flags = 0);
API bool ArrowButtonEx(const char *str_id, int dir, Vec2 size_arg,
                       int flags = 0);
API bool ImageButtonEx(int id, TextureID texture_id, const Vec2 &image_size,
                       const Vec2 &uv0, const Vec2 &uv1, const Vec4 &bg_col,
                       const Vec4 &tint_col, int flags = 0);
API void SeparatorEx(int flags, float thickness = 1.0f);
API void SeparatorTextEx(int id, const char *label, const char *label_end,
                         float extra_width);
API bool CheckboxFlags(const char *label, signed long long *flags,
                       signed long long flags_value);
API bool CheckboxFlags(const char *label, unsigned long long *flags,
                       unsigned long long flags_value);

// Widgets: Window Decorations
API bool CloseButton(int id, const Vec2 &pos);
API bool CollapseButton(int id, const Vec2 &pos, DockNode *dock_node);
API void Scrollbar(Axis axis);
API bool ScrollbarEx(const Rect &bb, int id, Axis axis,
                     signed long long *p_scroll_v, signed long long avail_v,
                     signed long long contents_v, int flags);
API Rect GetWindowScrollbarRect(Window *window, Axis axis);
API int GetWindowScrollbarID(Window *window, Axis axis);
API int GetWindowResizeCornerID(Window *window,
                                int n); // 0..3: corners
API int GetWindowResizeBorderID(Window *window, int dir);

// Widgets low-level behaviors
API bool ButtonBehavior(const Rect &bb, int id, bool *out_hovered,
                        bool *out_held, int flags = 0);
API bool DragBehavior(int id, int data_type, void *p_v, float v_speed,
                      const void *p_min, const void *p_max, const char *format,
                      int flags);
API bool SliderBehavior(const Rect &bb, int id, int data_type, void *p_v,
                        const void *p_min, const void *p_max,
                        const char *format, int flags, Rect *out_grab_bb);
API bool SplitterBehavior(const Rect &bb, int id, Axis axis, float *size1,
                          float *size2, float min_size1, float min_size2,
                          float hover_extend = 0.0f,
                          float hover_visibility_delay = 0.0f,
                          unsigned int bg_col = 0);
API bool TreeNodeBehavior(int id, int flags, const char *label,
                          const char *label_end = NULL);
API void TreePushOverrideID(int id);
API void TreeNodeSetOpen(int id, bool open);
API bool TreeNodeUpdateNextOpen(
    int id,
    int flags); // Return open state. Consume previous SetNextItemOpen() data,
                // if any. May return true when logging.
API void SetNextItemSelectionUserData(SelectionUserData selection_user_data);

// Template functions are instantiated in widgets.cpp for a finite number
// of types. To use them externally (for custom widget) you may need an "extern
// template" statement in your code in order to link to existing instances and
// silence Clang warnings (see #2036). e.g. " extern template API float
// RoundScalarWithFormatT<float, float>(const char* format, DataType
// data_type, float v); "
template <typename T, typename SIGNED_T, typename FLOAT_T>
API float
ScaleRatioFromValueT(int data_type, T v, T v_min, T v_max, bool is_logarithmic,
                     float logarithmic_zero_epsilon, float zero_deadzone_size);
template <typename T, typename SIGNED_T, typename FLOAT_T>
API T ScaleValueFromRatioT(int data_type, float t, T v_min, T v_max,
                           bool is_logarithmic, float logarithmic_zero_epsilon,
                           float zero_deadzone_size);
template <typename T, typename SIGNED_T, typename FLOAT_T>
API bool DragBehaviorT(int data_type, T *v, float v_speed, T v_min, T v_max,
                       const char *format, int flags);
template <typename T, typename SIGNED_T, typename FLOAT_T>
API bool SliderBehaviorT(const Rect &bb, int id, int data_type, T *v, T v_min,
                         T v_max, const char *format, int flags,
                         Rect *out_grab_bb);
template <typename T>
API T RoundScalarWithFormatT(const char *format, int data_type, T v);
template <typename T>
API bool CheckboxFlagsT(const char *label, T *flags, T flags_value);

// Data type helpers
API const DataTypeInfo *DataTypeGetInfo(int data_type);
API int DataTypeFormatString(char *buf, int buf_size, int data_type,
                             const void *p_data, const char *format);
API void DataTypeApplyOp(int data_type, int op, void *output, const void *arg_1,
                         const void *arg_2);
API bool DataTypeApplyFromText(const char *buf, int data_type, void *p_data,
                               const char *format);
API int DataTypeCompare(int data_type, const void *arg_1, const void *arg_2);
API bool DataTypeClamp(int data_type, void *p_data, const void *p_min,
                       const void *p_max);

// InputText
API bool InputTextEx(const char *label, const char *hint, char *buf,
                     int buf_size, const Vec2 &size_arg, int flags,
                     InputTextCallback callback = NULL, void *user_data = NULL);
API void InputTextDeactivateHook(int id);
API bool TempInputText(const Rect &bb, int id, const char *label, char *buf,
                       int buf_size, int flags);
API bool TempInputScalar(const Rect &bb, int id, const char *label,
                         int data_type, void *p_data, const char *format,
                         const void *p_clamp_min = NULL,
                         const void *p_clamp_max = NULL);
inline bool TempInputIsActive(int id) {
  Context &g = *GGui;
  return (g.ActiveId == id && g.TempInputId == id);
}
inline InputTextState *GetInputTextState(int id) {
  Context &g = *GGui;
  return (id != 0 && g.InputTextState.ID == id) ? &g.InputTextState : NULL;
} // Get input text state if active

// Color
API void ColorTooltip(const char *text, const float *col, int flags);
API void ColorEditOptionsPopup(const float *col, int flags);
API void ColorPickerOptionsPopup(const float *ref_col, int flags);

// Plot
API int PlotEx(PlotType plot_type, const char *label,
               float (*values_getter)(void *data, int idx), void *data,
               int values_count, int values_offset, const char *overlay_text,
               float scale_min, float scale_max, const Vec2 &size_arg);

// Shade functions (write over already created vertices)
API void ShadeVertsLinearColorGradientKeepAlpha(
    DrawList *draw_list, int vert_start_idx, int vert_end_idx, Vec2 gradient_p0,
    Vec2 gradient_p1, unsigned int col0, unsigned int col1);
API void ShadeVertsLinearUV(DrawList *draw_list, int vert_start_idx,
                            int vert_end_idx, const Vec2 &a, const Vec2 &b,
                            const Vec2 &uv_a, const Vec2 &uv_b, bool clamp);
API void ShadeVertsTransformPos(DrawList *draw_list, int vert_start_idx,
                                int vert_end_idx, const Vec2 &pivot_in,
                                float cos_a, float sin_a,
                                const Vec2 &pivot_out);

// Garbage collection
API void GcCompactTransientMiscBuffers();
API void GcCompactTransientWindowBuffers(Window *window);
API void GcAwakeTransientWindowBuffers(Window *window);

// Debug Log
API void DebugLog(const char *fmt, ...) FMTARGS(1);
API void DebugLogV(const char *fmt, va_list args) FMTLIST(1);
API void DebugAllocHook(DebugAllocInfo *info, int frame_count, void *ptr,
                        size_t size); // size >= 0 : alloc, size = -1 : free

// Debug Tools
API void ErrorCheckEndFrameRecover(ErrorLogCallback log_callback,
                                   void *user_data = NULL);
API void ErrorCheckEndWindowRecover(ErrorLogCallback log_callback,
                                    void *user_data = NULL);
API void ErrorCheckUsingSetCursorPosToExtendParentBoundaries();
API void DebugDrawCursorPos(unsigned int col = COL32(255, 0, 0, 255));
API void DebugDrawLineExtents(unsigned int col = COL32(255, 0, 0, 255));
API void DebugDrawItemRect(unsigned int col = COL32(255, 0, 0, 255));
API void
DebugLocateItem(int target_id); // Call sparingly: only 1 at the same time!
API void DebugLocateItemOnHover(
    int target_id); // Only call on reaction to a mouse Hover:
                    // because only 1 at the same time!
API void DebugLocateItemResolveWithLastItem();
inline void DebugStartItemPicker() {
  Context &g = *GGui;
  g.DebugItemPickerActive = true;
}
API void ShowFontAtlas(FontAtlas *atlas);
API void DebugHookIdInfo(int id, int data_type, const void *data_id,
                         const void *data_id_end);
API void DebugNodeColumns(OldColumns *columns);
API void DebugNodeDockNode(DockNode *node, const char *label);
API void DebugNodeDrawList(Window *window, ViewportP *viewport,
                           const DrawList *draw_list, const char *label);
API void DebugNodeDrawCmdShowMeshAndBoundingBox(DrawList *out_draw_list,
                                                const DrawList *draw_list,
                                                const DrawCmd *draw_cmd,
                                                bool show_mesh, bool show_aabb);
API void DebugNodeFont(Font *font);
API void DebugNodeFontGlyph(Font *font, const FontGlyph *glyph);
API void DebugNodeStorage(Storage *storage, const char *label);
API void DebugNodeTabBar(TabBar *tab_bar, const char *label);
API void DebugNodeTable(Table *table);
API void DebugNodeTableSettings(TableSettings *settings);
API void DebugNodeInputTextState(InputTextState *state);
API void DebugNodeTypingSelectState(TypingSelectState *state);
API void DebugNodeWindow(Window *window, const char *label);
API void DebugNodeWindowSettings(WindowSettings *settings);
API void DebugNodeWindowsList(Vector<Window *> *windows, const char *label);
API void DebugNodeWindowsListByBeginStackParent(Window **windows,
                                                int windows_size,
                                                Window *parent_in_begin_stack);
API void DebugNodeViewport(ViewportP *viewport);
API void DebugRenderKeyboardPreview(DrawList *draw_list);
API void DebugRenderViewportThumbnail(DrawList *draw_list, ViewportP *viewport,
                                      const Rect &bb);

// Obsolete functions
#ifndef DISABLE_OBSOLETE_FUNCTIONS
inline void SetItemUsingMouseWheel() {
  SetItemKeyOwner(Key_MouseWheelY);
} // Changed in 1.89
inline bool TreeNodeBehaviorIsOpen(int id, int flags = 0) {
  return TreeNodeUpdateNextOpen(id, flags);
} // Renamed in 1.89

// Refactored focus/nav/tabbing system in 1.82 and 1.84. If you have old/custom
// copy-and-pasted widgets that used FocusableItemRegister():
//  (Old) VERSION_NUM  < 18209: using 'ItemAdd(....)' and 'bool tab_focused =
//  FocusableItemRegister(...)' (Old) VERSION_NUM >= 18209: using 'ItemAdd(...,
//  ItemAddFlags_Focusable)'  and 'bool tab_focused = (GetItemStatusFlags()
//  & ItemStatusFlags_Focused) != 0' (New) VERSION_NUM >= 18413: using
//  'ItemAdd(..., ItemFlags_Inputable)'     and 'bool tab_focused =
//  (GetItemStatusFlags() & ItemStatusFlags_FocusedTabbing) != 0 ||
//  (g.NavActivateId == id && (g.NavActivateFlags &
//  ActivateFlags_PreferInput))' (WIP)
// Widget code are simplified as there's no need to call
// FocusableItemUnregister() while managing the transition from regular widget
// to TempInputText()
inline bool FocusableItemRegister(Window *window, int id) {
  assert(0);
  UNUSED(window);
  UNUSED(id);
  return false;
} // -> pass ItemAddFlags_Inputable flag to ItemAdd()
inline void FocusableItemUnregister(Window *window) {
  assert(0);
  UNUSED(window);
} // -> unnecessary: TempInputText() uses InputTextFlags_MergedItem
#endif
#ifndef DISABLE_OBSOLETE_KEYIO
inline bool IsKeyPressedMap(Key key, bool repeat = true) {
  assert(IsNamedKey(key));
  return IsKeyPressed(key, repeat);
} // Removed in 1.87: Mapping from named key is always identity!
#endif

} // namespace Gui

//-----------------------------------------------------------------------------
// [SECTION] FontAtlas internal API
//-----------------------------------------------------------------------------

// This structure is likely to evolve as we add support for incremental atlas
// updates
struct FontBuilderIO {
  bool (*FontBuilder_Build)(FontAtlas *atlas);
};

// Helper for font builder
#ifdef ENABLE_TRUETYPE
API const FontBuilderIO *FontAtlasGetBuilderForStbTruetype();
#endif
API void FontAtlasUpdateConfigDataPointers(FontAtlas *atlas);
API void FontAtlasBuildInit(FontAtlas *atlas);
API void FontAtlasBuildSetupFont(FontAtlas *atlas, Font *font,
                                 FontConfig *font_config, float ascent,
                                 float descent);
API void FontAtlasBuildPackCustomRects(FontAtlas *atlas, void *context_opaque);
API void FontAtlasBuildFinish(FontAtlas *atlas);
API void FontAtlasBuildRender8bppRectFromString(
    FontAtlas *atlas, int x, int y, int w, int h, const char *in_str,
    char in_marker_char, unsigned char in_marker_pixel_value);
API void FontAtlasBuildRender32bppRectFromString(
    FontAtlas *atlas, int x, int y, int w, int h, const char *in_str,
    char in_marker_char, unsigned int in_marker_pixel_value);
API void FontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256],
                                               float in_multiply_factor);
API void FontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256],
                                          unsigned char *pixels, int x, int y,
                                          int w, int h, int stride);

//-----------------------------------------------------------------------------
// [SECTION] Test Engine specific hooks (test_engine)
//-----------------------------------------------------------------------------

#ifdef ENABLE_TEST_ENGINE
extern void
TestEngineHook_ItemAdd(Context *ctx, int id, const Rect &bb,
                       const LastItemData *item_data); // item_data may be NULL
extern void TestEngineHook_ItemInfo(Context *ctx, int id, const char *label,
                                    int flags);
extern void TestEngineHook_Log(Context *ctx, const char *fmt, ...);
extern const char *TestEngine_FindItemDebugLabel(Context *ctx, int id);

// In VERSION_NUM >= 18934: changed TEST_ENGINE_ITEM_ADD(bb,id) to
// TEST_ENGINE_ITEM_ADD(id,bb,item_data);
#define TEST_ENGINE_ITEM_ADD(_ID, _BB, _ITEM_DATA)                             \
  if (g.TestEngineHookItems)                                                   \
  TestEngineHook_ItemAdd(&g, _ID, _BB, _ITEM_DATA) // Register item bounding box
#define TEST_ENGINE_ITEM_INFO(_ID, _LABEL, _FLAGS)                             \
  if (g.TestEngineHookItems)                                                   \
  TestEngineHook_ItemInfo(                                                     \
      &g, _ID, _LABEL,                                                         \
      _FLAGS) // Register item label and status flags (optional)
#define TEST_ENGINE_LOG(_FMT, ...)                                             \
  if (g.TestEngineHookItems)                                                   \
  TestEngineHook_Log(                                                          \
      &g, _FMT, __VA_ARGS__) // Custom log entry from user land into test log
#else
#define TEST_ENGINE_ITEM_ADD(_BB, _ID) ((void)0)
#define TEST_ENGINE_ITEM_INFO(_ID, _LABEL, _FLAGS) ((void)g)
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

#endif // #ifndef DISABLE
