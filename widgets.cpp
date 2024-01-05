// dear imgui, v1.90.1 WIP
// (widgets code)

/*

Index of this file:

// [SECTION] Forward Declarations
// [SECTION] Widgets: Text, etc.
// [SECTION] Widgets: Main (Button, Image, Checkbox, RadioButton, ProgressBar,
Bullet, etc.)
// [SECTION] Widgets: Low-level Layout helpers (Spacing, Dummy, NewLine,
Separator, etc.)
// [SECTION] Widgets: ComboBox
// [SECTION] Data Type and Data Formatting Helpers
// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
// [SECTION] Widgets: InputText, InputTextMultiline
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
// [SECTION] Widgets: Selectable
// [SECTION] Widgets: Typing-Select support
// [SECTION] Widgets: Multi-Select support
// [SECTION] Widgets: ListBox
// [SECTION] Widgets: PlotLines, PlotHistogram
// [SECTION] Widgets: Value helpers
// [SECTION] Widgets: MenuItem, BeginMenu, EndMenu, etc.
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.
// [SECTION] Widgets: Columns, BeginColumns, EndColumns, etc.

*/

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

//-------------------------------------------------------------------------
// Warnings
//-------------------------------------------------------------------------

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
                     // 'xxx' on a 4 byte value and then casting the result to a
                     // 8 byte value. Cast the value to the wider type before
                     // calling operator 'xxx' to avoid overflow(io.2).
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
    "-Wsign-conversion" // warning: implicit conversion changes signedness
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
    "-Wenum-enum-conversion" // warning: bitwise operation between different
                             // enumeration types ('XXXFlags_' and
                             // 'XXXFlagsPrivate_')
#pragma clang diagnostic ignored                                               \
    "-Wdeprecated-enum-enum-conversion" // warning: bitwise operation between
                                        // different enumeration types
                                        // ('XXXFlags_' and 'XXXFlagsPrivate_')
                                        // is deprecated
#pragma clang diagnostic ignored                                               \
    "-Wimplicit-int-float-conversion" // warning: implicit conversion from 'xxx'
                                      // to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored                                                 \
    "-Wpragmas" // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored                                                 \
    "-Wformat-nonliteral" // warning: format not a string literal, format string
                          // not checked
#pragma GCC diagnostic ignored                                                 \
    "-Wclass-memaccess" // [__GNUC__ >= 8] warning: 'memset/memcpy'
                        // clearing/writing an object of type 'xxxx' with no
                        // trivial copy-assignment; use assignment or
                        // value-initialization instead
#pragma GCC diagnostic ignored                                                 \
    "-Wdeprecated-enum-enum-conversion" // warning: bitwise operation between
                                        // different enumeration types
                                        // ('XXXFlags_' and 'XXXFlagsPrivate_')
                                        // is deprecated
#endif

//-------------------------------------------------------------------------
// Data
//-------------------------------------------------------------------------

// Widgets
static const float DRAGDROP_HOLD_TO_OPEN_TIMER =
    0.70f; // Time for drag-hold to activate items accepting the
           // ButtonFlags_PressedOnDragDropHold button behavior.
static const float DRAG_MOUSE_THRESHOLD_FACTOR =
    0.50f; // Multiplier for the default value of io.MouseDragThreshold to make
           // DragFloat/DragInt react faster to mouse drags.

// Those MIN/MAX values are not define because we need to point to them
static const signed char S8_MIN = -128;
static const signed char S8_MAX = 127;
static const unsigned char U8_MIN = 0;
static const unsigned char U8_MAX = 0xFF;
static const signed short S16_MIN = -32768;
static const signed short S16_MAX = 32767;
static const unsigned short U16_MIN = 0;
static const unsigned short U16_MAX = 0xFFFF;
static const S32 S32_MIN = INT_MIN; // (-2147483647 - 1), (0x80000000);
static const S32 S32_MAX = INT_MAX; // (2147483647), (0x7FFFFFFF)
static const U32 U32_MIN = 0;
static const U32 U32_MAX = UINT_MAX; // (0xFFFFFFFF)
#ifdef LLONG_MIN
static const S64 S64_MIN = LLONG_MIN; // (-9223372036854775807ll - 1ll);
static const S64 S64_MAX = LLONG_MAX; // (9223372036854775807ll);
#else
static const S64 S64_MIN = -9223372036854775807LL - 1;
static const S64 S64_MAX = 9223372036854775807LL;
#endif
static const U64 U64_MIN = 0;
#ifdef ULLONG_MAX
static const U64 U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
static const U64 U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

//-------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-------------------------------------------------------------------------

// For InputTextEx()
static bool InputTextFilterCharacter(Context *ctx, unsigned int *p_char,
                                     InputTextFlags flags,
                                     InputTextCallback callback,
                                     void *user_data, InputSource input_source);
static int InputTextCalcTextLenAndLineCount(const char *text_begin,
                                            const char **out_text_end);
static Vec2 InputTextCalcTextSizeW(Context *ctx, const Wchar *text_begin,
                                   const Wchar *text_end,
                                   const Wchar **remaining = NULL,
                                   Vec2 *out_offset = NULL,
                                   bool stop_on_new_line = false);

//-------------------------------------------------------------------------
// [SECTION] Widgets: Text, etc.
//-------------------------------------------------------------------------
// - TextEx() [Internal]
// - TextUnformatted()
// - Text()
// - TextV()
// - TextColored()
// - TextColoredV()
// - TextDisabled()
// - TextDisabledV()
// - TextWrapped()
// - TextWrappedV()
// - LabelText()
// - LabelTextV()
// - BulletText()
// - BulletTextV()
//-------------------------------------------------------------------------

void Gui::TextEx(const char *text, const char *text_end, TextFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;
  Context &g = *GGui;

  // Accept null ranges
  if (text == text_end)
    text = text_end = "";

  // Calculate length
  const char *text_begin = text;
  if (text_end == NULL)
    text_end = text + strlen(text); // FIXME-OPT

  const Vec2 text_pos(window->DC.CursorPos.x,
                      window->DC.CursorPos.y +
                          window->DC.CurrLineTextBaseOffset);
  const float wrap_pos_x = window->DC.TextWrapPos;
  const bool wrap_enabled = (wrap_pos_x >= 0.0f);
  if (text_end - text <= 2000 || wrap_enabled) {
    // Common case
    const float wrap_width =
        wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x)
                     : 0.0f;
    const Vec2 text_size =
        CalcTextSize(text_begin, text_end, false, wrap_width);

    Rect bb(text_pos, text_pos + text_size);
    ItemSize(text_size, 0.0f);
    if (!ItemAdd(bb, 0))
      return;

    // Render (we don't hide text after ## in this end-user function)
    RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
  } else {
    // Long text!
    // Perform manual coarse clipping to optimize for long multi-line text
    // - From this point we will only compute the width of lines that are
    // visible. Optimization only available when word-wrapping is disabled.
    // - We also don't vertically center the text within the line full height,
    // which is unlikely to matter because we are likely the biggest and only
    // item on the line.
    // - We use memchr(), pay attention that well optimized versions of those
    // str/mem functions are much faster than a casually written loop.
    const char *line = text;
    const float line_height = GetTextLineHeight();
    Vec2 text_size(0, 0);

    // Lines to skip (can't skip when logging text)
    Vec2 pos = text_pos;
    if (!g.LogEnabled) {
      int lines_skippable =
          (int)((window->ClipRect.Min.y - text_pos.y) / line_height);
      if (lines_skippable > 0) {
        int lines_skipped = 0;
        while (line < text_end && lines_skipped < lines_skippable) {
          const char *line_end =
              (const char *)memchr(line, '\n', text_end - line);
          if (!line_end)
            line_end = text_end;
          if ((flags & TextFlags_NoWidthForLargeClippedText) == 0)
            text_size.x = Max(text_size.x, CalcTextSize(line, line_end).x);
          line = line_end + 1;
          lines_skipped++;
        }
        pos.y += lines_skipped * line_height;
      }
    }

    // Lines to render
    if (line < text_end) {
      Rect line_rect(pos, pos + Vec2(FLT_MAX, line_height));
      while (line < text_end) {
        if (IsClippedEx(line_rect, 0))
          break;

        const char *line_end =
            (const char *)memchr(line, '\n', text_end - line);
        if (!line_end)
          line_end = text_end;
        text_size.x = Max(text_size.x, CalcTextSize(line, line_end).x);
        RenderText(pos, line, line_end, false);
        line = line_end + 1;
        line_rect.Min.y += line_height;
        line_rect.Max.y += line_height;
        pos.y += line_height;
      }

      // Count remaining lines
      int lines_skipped = 0;
      while (line < text_end) {
        const char *line_end =
            (const char *)memchr(line, '\n', text_end - line);
        if (!line_end)
          line_end = text_end;
        if ((flags & TextFlags_NoWidthForLargeClippedText) == 0)
          text_size.x = Max(text_size.x, CalcTextSize(line, line_end).x);
        line = line_end + 1;
        lines_skipped++;
      }
      pos.y += lines_skipped * line_height;
    }
    text_size.y = (pos - text_pos).y;

    Rect bb(text_pos, text_pos + text_size);
    ItemSize(text_size, 0.0f);
    ItemAdd(bb, 0);
  }
}

void Gui::TextUnformatted(const char *text, const char *text_end) {
  TextEx(text, text_end, TextFlags_NoWidthForLargeClippedText);
}

void Gui::Text(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  TextV(fmt, args);
  va_end(args);
}

void Gui::TextV(const char *fmt, va_list args) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  const char *text, *text_end;
  FormatStringToTempBufferV(&text, &text_end, fmt, args);
  TextEx(text, text_end, TextFlags_NoWidthForLargeClippedText);
}

void Gui::TextColored(const Vec4 &col, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  TextColoredV(col, fmt, args);
  va_end(args);
}

void Gui::TextColoredV(const Vec4 &col, const char *fmt, va_list args) {
  PushStyleColor(Col_Text, col);
  TextV(fmt, args);
  PopStyleColor();
}

void Gui::TextDisabled(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  TextDisabledV(fmt, args);
  va_end(args);
}

void Gui::TextDisabledV(const char *fmt, va_list args) {
  Context &g = *GGui;
  PushStyleColor(Col_Text, g.Style.Colors[Col_TextDisabled]);
  TextV(fmt, args);
  PopStyleColor();
}

void Gui::TextWrapped(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  TextWrappedV(fmt, args);
  va_end(args);
}

void Gui::TextWrappedV(const char *fmt, va_list args) {
  Context &g = *GGui;
  const bool need_backup =
      (g.CurrentWindow->DC.TextWrapPos <
       0.0f); // Keep existing wrap position if one is already set
  if (need_backup)
    PushTextWrapPos(0.0f);
  TextV(fmt, args);
  if (need_backup)
    PopTextWrapPos();
}

void Gui::LabelText(const char *label, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  LabelTextV(label, fmt, args);
  va_end(args);
}

// Add a label+text combo aligned to other label+value widgets
void Gui::LabelTextV(const char *label, const char *fmt, va_list args) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  Context &g = *GGui;
  const Style &style = g.Style;
  const float w = CalcItemWidth();

  const char *value_text_begin, *value_text_end;
  FormatStringToTempBufferV(&value_text_begin, &value_text_end, fmt, args);
  const Vec2 value_size = CalcTextSize(value_text_begin, value_text_end, false);
  const Vec2 label_size = CalcTextSize(label, NULL, true);

  const Vec2 pos = window->DC.CursorPos;
  const Rect value_bb(pos,
                      pos + Vec2(w, value_size.y + style.FramePadding.y * 2));
  const Rect total_bb(
      pos,
      pos + Vec2(w + (label_size.x > 0.0f
                          ? style.ItemInnerSpacing.x + label_size.x
                          : 0.0f),
                 Max(value_size.y, label_size.y) + style.FramePadding.y * 2));
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, 0))
    return;

  // Render
  RenderTextClipped(value_bb.Min + style.FramePadding, value_bb.Max,
                    value_text_begin, value_text_end, &value_size,
                    Vec2(0.0f, 0.0f));
  if (label_size.x > 0.0f)
    RenderText(Vec2(value_bb.Max.x + style.ItemInnerSpacing.x,
                    value_bb.Min.y + style.FramePadding.y),
               label);
}

void Gui::BulletText(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  BulletTextV(fmt, args);
  va_end(args);
}

// Text with a little bullet aligned to the typical tree node.
void Gui::BulletTextV(const char *fmt, va_list args) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  Context &g = *GGui;
  const Style &style = g.Style;

  const char *text_begin, *text_end;
  FormatStringToTempBufferV(&text_begin, &text_end, fmt, args);
  const Vec2 label_size = CalcTextSize(text_begin, text_end, false);
  const Vec2 total_size =
      Vec2(g.FontSize + (label_size.x > 0.0f
                             ? (label_size.x + style.FramePadding.x * 2)
                             : 0.0f),
           label_size.y); // Empty text doesn't add padding
  Vec2 pos = window->DC.CursorPos;
  pos.y += window->DC.CurrLineTextBaseOffset;
  ItemSize(total_size, 0.0f);
  const Rect bb(pos, pos + total_size);
  if (!ItemAdd(bb, 0))
    return;

  // Render
  U32 text_col = GetColorU32(Col_Text);
  RenderBullet(window->DrawList,
               bb.Min + Vec2(style.FramePadding.x + g.FontSize * 0.5f,
                             g.FontSize * 0.5f),
               text_col);
  RenderText(bb.Min + Vec2(g.FontSize + style.FramePadding.x * 2, 0.0f),
             text_begin, text_end, false);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Main
//-------------------------------------------------------------------------
// - ButtonBehavior() [Internal]
// - Button()
// - SmallButton()
// - InvisibleButton()
// - ArrowButton()
// - CloseButton() [Internal]
// - CollapseButton() [Internal]
// - GetWindowScrollbarID() [Internal]
// - GetWindowScrollbarRect() [Internal]
// - Scrollbar() [Internal]
// - ScrollbarEx() [Internal]
// - Image()
// - ImageButton()
// - Checkbox()
// - CheckboxFlagsT() [Internal]
// - CheckboxFlags()
// - RadioButton()
// - ProgressBar()
// - Bullet()
//-------------------------------------------------------------------------

// The ButtonBehavior() function is key to many interactions and used by
// many/most widgets. Because we handle so many cases (keyboard/gamepad
// navigation, drag and drop) and many specific behavior (via
// ButtonFlags_), this code is a little complex. By far the most common
// path is interacting with the Mouse using the default
// ButtonFlags_PressedOnClickRelease button behavior. See the series of
// events below and the corresponding state reported by dear imgui:
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnClickRelease:             return-value  IsItemHovered()
// IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+0 (mouse is outside bb)        -             -                - -
//   -                    - Frame N+1 (mouse moves inside bb)      - true - - -
//   - Frame N+2 (mouse button is down)       -             true true true -
//   true Frame N+3 (mouse button is down)       -             true true - - -
//   Frame N+4 (mouse moves outside bb)     -             -                true
//   -                  -                    - Frame N+5 (mouse moves inside bb)
//   -             true             true            -                  - - Frame
//   N+6 (mouse button is released)   true          true             - - true -
//   Frame N+7 (mouse button is released)   -             true             - -
//   -                    - Frame N+8 (mouse moves outside bb)     - - - - - -
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnClick:                    return-value  IsItemHovered()
// IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+2 (mouse button is down)       true          true             true
//   true               -                    true Frame N+3 (mouse button is
//   down)       -             true             true            - - - Frame N+6
//   (mouse button is released)   -             true             - - true -
//   Frame N+7 (mouse button is released)   -             true             - -
//   -                    -
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnRelease:                  return-value  IsItemHovered()
// IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+2 (mouse button is down)       -             true             - -
//   -                    true Frame N+3 (mouse button is down)       - true -
//   -                  -                    - Frame N+6 (mouse button is
//   released)   true          true             -               - - - Frame N+7
//   (mouse button is released)   -             true             - - - -
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnDoubleClick:              return-value  IsItemHovered()
// IsItemActive()  IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+0 (mouse button is down)       -             true             - -
//   -                    true Frame N+1 (mouse button is down)       - true -
//   -                  -                    - Frame N+2 (mouse button is
//   released)   -             true             -               - - - Frame N+3
//   (mouse button is released)   -             true             - - - - Frame
//   N+4 (mouse button is down)       true          true             true true
//   -                    true Frame N+5 (mouse button is down)       - true
//   true            -                  -                    - Frame N+6 (mouse
//   button is released)   -             true             -               - true
//   - Frame N+7 (mouse button is released)   -             true             -
//   -                  -                    -
//------------------------------------------------------------------------------------------------------------------------------------------------
// Note that some combinations are supported,
// - PressedOnDragDropHold can generally be associated with any flag.
// - PressedOnDoubleClick can be associated by
// PressedOnClickRelease/PressedOnRelease, in which case the second release
// event won't be reported.
//------------------------------------------------------------------------------------------------------------------------------------------------
// The behavior of the return-value changes when ButtonFlags_Repeat is set:
//                                         Repeat+                  Repeat+
//                                         Repeat+             Repeat+
//                                         PressedOnClickRelease PressedOnClick
//                                         PressedOnRelease PressedOnDoubleClick
//-------------------------------------------------------------------------------------------------------------------------------------------------
//   Frame N+0 (mouse button is down)       -                        true - true
//   ...                                    -                        - - - Frame
//   N + RepeatDelay                  true                     true - true
//   ...                                    -                        - - - Frame
//   N + RepeatDelay + RepeatRate*N   true                     true - true
//-------------------------------------------------------------------------------------------------------------------------------------------------

bool Gui::ButtonBehavior(const Rect &bb, ID id, bool *out_hovered,
                         bool *out_held, ButtonFlags flags) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();

  // Default only reacts to left mouse button
  if ((flags & ButtonFlags_MouseButtonMask_) == 0)
    flags |= ButtonFlags_MouseButtonDefault_;

  // Default behavior requires click + release inside bounding box
  if ((flags & ButtonFlags_PressedOnMask_) == 0)
    flags |= ButtonFlags_PressedOnDefault_;

  // Default behavior inherited from item flags
  // Note that _both_ ButtonFlags and ItemFlags are valid sources, so copy one
  // into the item_flags and only check that.
  ItemFlags item_flags =
      (g.LastItemData.ID == id ? g.LastItemData.InFlags : g.CurrentItemFlags);
  if (flags & ButtonFlags_AllowOverlap)
    item_flags |= ItemFlags_AllowOverlap;
  if (flags & ButtonFlags_Repeat)
    item_flags |= ItemFlags_ButtonRepeat;

  Window *backup_hovered_window = g.HoveredWindow;
  const bool flatten_hovered_children =
      (flags & ButtonFlags_FlattenChildren) && g.HoveredWindow &&
      g.HoveredWindow->RootWindowDockTree == window->RootWindowDockTree;
  if (flatten_hovered_children)
    g.HoveredWindow = window;

#ifdef ENABLE_TEST_ENGINE
  // Alternate registration spot, for when caller didn't use ItemAdd()
  if (id != 0 && g.LastItemData.ID != id)
    TEST_ENGINE_ITEM_ADD(id, bb, NULL);
#endif

  bool pressed = false;
  bool hovered = ItemHoverable(bb, id, item_flags);

  // Special mode for Drag and Drop where holding button pressed for a long time
  // while dragging another item triggers the button
  if (g.DragDropActive && (flags & ButtonFlags_PressedOnDragDropHold) &&
      !(g.DragDropSourceFlags & DragDropFlags_SourceNoHoldToOpenOthers))
    if (IsItemHovered(HoveredFlags_AllowWhenBlockedByActiveItem)) {
      hovered = true;
      SetHoveredID(id);
      if (g.HoveredIdTimer - g.IO.DeltaTime <= DRAGDROP_HOLD_TO_OPEN_TIMER &&
          g.HoveredIdTimer >= DRAGDROP_HOLD_TO_OPEN_TIMER) {
        pressed = true;
        g.DragDropHoldJustPressedId = id;
        FocusWindow(window);
      }
    }

  if (flatten_hovered_children)
    g.HoveredWindow = backup_hovered_window;

  // Mouse handling
  const ID test_owner_id =
      (flags & ButtonFlags_NoTestKeyOwner) ? KeyOwner_Any : id;
  if (hovered) {
    // Poll mouse buttons
    // - 'mouse_button_clicked' is generally carried into ActiveIdMouseButton
    // when setting ActiveId.
    // - Technically we only need some values in one code path, but since this
    // is gated by hovered test this is fine.
    int mouse_button_clicked = -1;
    int mouse_button_released = -1;
    for (int button = 0; button < 3; button++)
      if (flags & (ButtonFlags_MouseButtonLeft
                   << button)) // Handle ButtonFlags_MouseButtonRight and
                               // ButtonFlags_MouseButtonMiddle here.
      {
        if (IsMouseClicked(button, test_owner_id) &&
            mouse_button_clicked == -1) {
          mouse_button_clicked = button;
        }
        if (IsMouseReleased(button, test_owner_id) &&
            mouse_button_released == -1) {
          mouse_button_released = button;
        }
      }

    // Process initial action
    if (!(flags & ButtonFlags_NoKeyModifiers) ||
        (!g.IO.KeyCtrl && !g.IO.KeyShift && !g.IO.KeyAlt)) {
      if (mouse_button_clicked != -1 && g.ActiveId != id) {
        if (!(flags & ButtonFlags_NoSetKeyOwner))
          SetKeyOwner(MouseButtonToKey(mouse_button_clicked), id);
        if (flags & (ButtonFlags_PressedOnClickRelease |
                     ButtonFlags_PressedOnClickReleaseAnywhere)) {
          SetActiveID(id, window);
          g.ActiveIdMouseButton = mouse_button_clicked;
          if (!(flags & ButtonFlags_NoNavFocus))
            SetFocusID(id, window);
          FocusWindow(window);
        }
        if ((flags & ButtonFlags_PressedOnClick) ||
            ((flags & ButtonFlags_PressedOnDoubleClick) &&
             g.IO.MouseClickedCount[mouse_button_clicked] == 2)) {
          pressed = true;
          if (flags & ButtonFlags_NoHoldingActiveId)
            ClearActiveID();
          else
            SetActiveID(id, window); // Hold on ID
          if (!(flags & ButtonFlags_NoNavFocus))
            SetFocusID(id, window);
          g.ActiveIdMouseButton = mouse_button_clicked;
          FocusWindow(window);
        }
      }
      if (flags & ButtonFlags_PressedOnRelease) {
        if (mouse_button_released != -1) {
          const bool has_repeated_at_least_once =
              (item_flags & ItemFlags_ButtonRepeat) &&
              g.IO.MouseDownDurationPrev[mouse_button_released] >=
                  g.IO.KeyRepeatDelay; // Repeat mode trumps on release behavior
          if (!has_repeated_at_least_once)
            pressed = true;
          if (!(flags & ButtonFlags_NoNavFocus))
            SetFocusID(id, window);
          ClearActiveID();
        }
      }

      // 'Repeat' mode acts when held regardless of _PressedOn flags (see table
      // above). Relies on repeat logic of IsMouseClicked() but we may as well
      // do it ourselves if we end up exposing finer RepeatDelay/RepeatRate
      // settings.
      if (g.ActiveId == id && (item_flags & ItemFlags_ButtonRepeat))
        if (g.IO.MouseDownDuration[g.ActiveIdMouseButton] > 0.0f &&
            IsMouseClicked(g.ActiveIdMouseButton, test_owner_id,
                           InputFlags_Repeat))
          pressed = true;
    }

    if (pressed)
      g.NavDisableHighlight = true;
  }

  // Gamepad/Keyboard navigation
  // We report navigated item as hovered but we don't set g.HoveredId to not
  // interfere with mouse.
  if (g.NavId == id && !g.NavDisableHighlight && g.NavDisableMouseHover &&
      (g.ActiveId == 0 || g.ActiveId == id || g.ActiveId == window->MoveId))
    if (!(flags & ButtonFlags_NoHoveredOnFocus))
      hovered = true;
  if (g.NavActivateDownId == id) {
    bool nav_activated_by_code = (g.NavActivateId == id);
    bool nav_activated_by_inputs = (g.NavActivatePressedId == id);
    if (!nav_activated_by_inputs && (item_flags & ItemFlags_ButtonRepeat)) {
      // Avoid pressing multiple keys from triggering excessive amount of repeat
      // events
      const KeyData *key1 = GetKeyData(Key_Space);
      const KeyData *key2 = GetKeyData(Key_Enter);
      const KeyData *key3 = GetKeyData(Key_NavGamepadActivate);
      const float t1 =
          Max(Max(key1->DownDuration, key2->DownDuration), key3->DownDuration);
      nav_activated_by_inputs =
          CalcTypematicRepeatAmount(t1 - g.IO.DeltaTime, t1,
                                    g.IO.KeyRepeatDelay,
                                    g.IO.KeyRepeatRate) > 0;
    }
    if (nav_activated_by_code || nav_activated_by_inputs) {
      // Set active id so it can be queried by user via IsItemActive(),
      // equivalent of holding the mouse button.
      pressed = true;
      SetActiveID(id, window);
      g.ActiveIdSource = g.NavInputSource;
      if (!(flags & ButtonFlags_NoNavFocus))
        SetFocusID(id, window);
    }
  }

  // Process while held
  bool held = false;
  if (g.ActiveId == id) {
    if (g.ActiveIdSource == InputSource_Mouse) {
      if (g.ActiveIdIsJustActivated)
        g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;

      const int mouse_button = g.ActiveIdMouseButton;
      if (mouse_button == -1) {
        // Fallback for the rare situation were g.ActiveId was set
        // programmatically or from another widget (e.g. #6304).
        ClearActiveID();
      } else if (IsMouseDown(mouse_button, test_owner_id)) {
        held = true;
      } else {
        bool release_in =
            hovered && (flags & ButtonFlags_PressedOnClickRelease) != 0;
        bool release_anywhere =
            (flags & ButtonFlags_PressedOnClickReleaseAnywhere) != 0;
        if ((release_in || release_anywhere) && !g.DragDropActive) {
          // Report as pressed when releasing the mouse (this is the most common
          // path)
          bool is_double_click_release =
              (flags & ButtonFlags_PressedOnDoubleClick) &&
              g.IO.MouseReleased[mouse_button] &&
              g.IO.MouseClickedLastCount[mouse_button] == 2;
          bool is_repeating_already =
              (item_flags & ItemFlags_ButtonRepeat) &&
              g.IO.MouseDownDurationPrev[mouse_button] >=
                  g.IO.KeyRepeatDelay; // Repeat mode trumps <on release>
          bool is_button_avail_or_owned =
              TestKeyOwner(MouseButtonToKey(mouse_button), test_owner_id);
          if (!is_double_click_release && !is_repeating_already &&
              is_button_avail_or_owned)
            pressed = true;
        }
        ClearActiveID();
      }
      if (!(flags & ButtonFlags_NoNavFocus))
        g.NavDisableHighlight = true;
    } else if (g.ActiveIdSource == InputSource_Keyboard ||
               g.ActiveIdSource == InputSource_Gamepad) {
      // When activated using Nav, we hold on the ActiveID until activation
      // button is released
      if (g.NavActivateDownId != id)
        ClearActiveID();
    }
    if (pressed)
      g.ActiveIdHasBeenPressedBefore = true;
  }

  if (out_hovered)
    *out_hovered = hovered;
  if (out_held)
    *out_held = held;

  return pressed;
}

bool Gui::ButtonEx(const char *label, const Vec2 &size_arg, ButtonFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const ID id = window->GetID(label);
  const Vec2 label_size = CalcTextSize(label, NULL, true);

  Vec2 pos = window->DC.CursorPos;
  if ((flags & ButtonFlags_AlignTextBaseLine) &&
      style.FramePadding.y <
          window->DC
              .CurrLineTextBaseOffset) // Try to vertically align buttons that
                                       // are smaller/have no padding so that
                                       // text baseline matches (bit hacky,
                                       // since it shouldn't be a flag)
    pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
  Vec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f,
                           label_size.y + style.FramePadding.y * 2.0f);

  const Rect bb(pos, pos + size);
  ItemSize(size, style.FramePadding.y);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  // Render
  const U32 col = GetColorU32((held && hovered) ? Col_ButtonActive
                              : hovered         ? Col_ButtonHovered
                                                : Col_Button);
  RenderNavHighlight(bb, id);
  RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

  if (g.LogEnabled)
    LogSetNextTextDecoration("[", "]");
  RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding,
                    label, NULL, &label_size, style.ButtonTextAlign, &bb);

  // Automatically close popups
  // if (pressed && !(flags & ButtonFlags_DontClosePopups) &&
  // (window->Flags & WindowFlags_Popup))
  //    CloseCurrentPopup();

  TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
  return pressed;
}

bool Gui::Button(const char *label, const Vec2 &size_arg) {
  return ButtonEx(label, size_arg, ButtonFlags_None);
}

// Small buttons fits within text without additional vertical spacing.
bool Gui::SmallButton(const char *label) {
  Context &g = *GGui;
  float backup_padding_y = g.Style.FramePadding.y;
  g.Style.FramePadding.y = 0.0f;
  bool pressed = ButtonEx(label, Vec2(0, 0), ButtonFlags_AlignTextBaseLine);
  g.Style.FramePadding.y = backup_padding_y;
  return pressed;
}

// Tip: use Gui::PushID()/PopID() to push indices or pointers in the ID stack.
// Then you can keep 'str_id' empty or the same for all your buttons (instead of
// creating a string based on a non-string id)
bool Gui::InvisibleButton(const char *str_id, const Vec2 &size_arg,
                          ButtonFlags flags) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  // Cannot use zero-size for InvisibleButton(). Unlike Button() there is not
  // way to fallback using the label size.
  ASSERT(size_arg.x != 0.0f && size_arg.y != 0.0f);

  const ID id = window->GetID(str_id);
  Vec2 size = CalcItemSize(size_arg, 0.0f, 0.0f);
  const Rect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  ItemSize(size);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  TEST_ENGINE_ITEM_INFO(id, str_id, g.LastItemData.StatusFlags);
  return pressed;
}

bool Gui::ArrowButtonEx(const char *str_id, Dir dir, Vec2 size,
                        ButtonFlags flags) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  const ID id = window->GetID(str_id);
  const Rect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  const float default_size = GetFrameHeight();
  ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  // Render
  const U32 bg_col = GetColorU32((held && hovered) ? Col_ButtonActive
                                 : hovered         ? Col_ButtonHovered
                                                   : Col_Button);
  const U32 text_col = GetColorU32(Col_Text);
  RenderNavHighlight(bb, id);
  RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
  RenderArrow(window->DrawList,
              bb.Min + Vec2(Max(0.0f, (size.x - g.FontSize) * 0.5f),
                            Max(0.0f, (size.y - g.FontSize) * 0.5f)),
              text_col, dir);

  TEST_ENGINE_ITEM_INFO(id, str_id, g.LastItemData.StatusFlags);
  return pressed;
}

bool Gui::ArrowButton(const char *str_id, Dir dir) {
  float sz = GetFrameHeight();
  return ArrowButtonEx(str_id, dir, Vec2(sz, sz), ButtonFlags_None);
}

// Button to close a window
bool Gui::CloseButton(ID id, const Vec2 &pos) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  // Tweak 1: Shrink hit-testing area if button covers an abnormally large
  // proportion of the visible region. That's in order to facilitate moving the
  // window away. (#3825) This may better be applied as a general hit-rect
  // reduction mechanism for all widgets to ensure the area to move window is
  // always accessible?
  const Rect bb(pos, pos + Vec2(g.FontSize, g.FontSize));
  Rect bb_interact = bb;
  const float area_to_visible_ratio =
      window->OuterRectClipped.GetArea() / bb.GetArea();
  if (area_to_visible_ratio < 1.5f)
    bb_interact.Expand(Trunc(bb_interact.GetSize() * -0.25f));

  // Tweak 2: We intentionally allow interaction when clipped so that a
  // mechanical Alt,Right,Activate sequence can always close a window. (this
  // isn't the common behavior of buttons, but it doesn't affect the user
  // because navigation tends to keep items visible in scrolling layer).
  bool is_clipped = !ItemAdd(bb_interact, id);

  bool hovered, held;
  bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
  if (is_clipped)
    return pressed;

  // Render
  // FIXME: Clarify this mess
  U32 col = GetColorU32(held ? Col_ButtonActive : Col_ButtonHovered);
  Vec2 center = bb.GetCenter();
  if (hovered)
    window->DrawList->AddCircleFilled(center,
                                      Max(2.0f, g.FontSize * 0.5f + 1.0f), col);

  float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
  U32 cross_col = GetColorU32(Col_Text);
  center -= Vec2(0.5f, 0.5f);
  window->DrawList->AddLine(center + Vec2(+cross_extent, +cross_extent),
                            center + Vec2(-cross_extent, -cross_extent),
                            cross_col, 1.0f);
  window->DrawList->AddLine(center + Vec2(+cross_extent, -cross_extent),
                            center + Vec2(-cross_extent, +cross_extent),
                            cross_col, 1.0f);

  return pressed;
}

// The Collapse button also functions as a Dock Menu button.
bool Gui::CollapseButton(ID id, const Vec2 &pos, DockNode *dock_node) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  Rect bb(pos, pos + Vec2(g.FontSize, g.FontSize));
  bool is_clipped = !ItemAdd(bb, id);
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, ButtonFlags_None);
  if (is_clipped)
    return pressed;

  // Render
  // bool is_dock_menu = (window->DockNodeAsHost && !window->Collapsed);
  U32 bg_col = GetColorU32((held && hovered) ? Col_ButtonActive
                           : hovered         ? Col_ButtonHovered
                                             : Col_Button);
  U32 text_col = GetColorU32(Col_Text);
  if (hovered || held)
    window->DrawList->AddCircleFilled(bb.GetCenter() + Vec2(0.0f, -0.5f),
                                      g.FontSize * 0.5f + 1.0f, bg_col);

  if (dock_node)
    RenderArrowDockMenu(window->DrawList, bb.Min, g.FontSize, text_col);
  else
    RenderArrow(window->DrawList, bb.Min, text_col,
                window->Collapsed ? Dir_Right : Dir_Down, 1.0f);

  // Switch to moving the window after mouse is moved beyond the initial drag
  // threshold
  if (IsItemActive() && IsMouseDragging(0))
    StartMouseMovingWindowOrNode(
        window, dock_node, true); // Undock from window/collapse menu button

  return pressed;
}

ID Gui::GetWindowScrollbarID(Window *window, Axis axis) {
  return window->GetID(axis == Axis_X ? "#SCROLLX" : "#SCROLLY");
}

// Return scrollbar rectangle, must only be called for corresponding axis if
// window->ScrollbarX/Y is set.
Rect Gui::GetWindowScrollbarRect(Window *window, Axis axis) {
  const Rect outer_rect = window->Rect();
  const Rect inner_rect = window->InnerRect;
  const float border_size = window->WindowBorderSize;
  const float scrollbar_size =
      window->ScrollbarSizes[axis ^
                             1]; // (ScrollbarSizes.x = width of Y scrollbar;
                                 // ScrollbarSizes.y = height of X scrollbar)
  ASSERT(scrollbar_size > 0.0f);
  if (axis == Axis_X)
    return Rect(
        inner_rect.Min.x,
        Max(outer_rect.Min.y, outer_rect.Max.y - border_size - scrollbar_size),
        inner_rect.Max.x - border_size, outer_rect.Max.y - border_size);
  else
    return Rect(
        Max(outer_rect.Min.x, outer_rect.Max.x - border_size - scrollbar_size),
        inner_rect.Min.y, outer_rect.Max.x - border_size,
        inner_rect.Max.y - border_size);
}

void Gui::Scrollbar(Axis axis) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  const ID id = GetWindowScrollbarID(window, axis);

  // Calculate scrollbar bounding box
  Rect bb = GetWindowScrollbarRect(window, axis);
  DrawFlags rounding_corners = DrawFlags_RoundCornersNone;
  if (axis == Axis_X) {
    rounding_corners |= DrawFlags_RoundCornersBottomLeft;
    if (!window->ScrollbarY)
      rounding_corners |= DrawFlags_RoundCornersBottomRight;
  } else {
    if ((window->Flags & WindowFlags_NoTitleBar) &&
        !(window->Flags & WindowFlags_MenuBar))
      rounding_corners |= DrawFlags_RoundCornersTopRight;
    if (!window->ScrollbarX)
      rounding_corners |= DrawFlags_RoundCornersBottomRight;
  }
  float size_avail = window->InnerRect.Max[axis] - window->InnerRect.Min[axis];
  float size_contents =
      window->ContentSize[axis] + window->WindowPadding[axis] * 2.0f;
  S64 scroll = (S64)window->Scroll[axis];
  ScrollbarEx(bb, id, axis, &scroll, (S64)size_avail, (S64)size_contents,
              rounding_corners);
  window->Scroll[axis] = (float)scroll;
}

// Vertical/Horizontal scrollbar
// The entire piece of code below is rather confusing because:
// - We handle absolute seeking (when first clicking outside the grab) and
// relative manipulation (afterward or when clicking inside the grab)
// - We store values as normalized ratio and in a form that allows the window
// content to change while we are holding on a scrollbar
// - We handle both horizontal and vertical scrollbars, which makes the
// terminology not ideal. Still, the code should probably be made simpler..
bool Gui::ScrollbarEx(const Rect &bb_frame, ID id, Axis axis, S64 *p_scroll_v,
                      S64 size_avail_v, S64 size_contents_v, DrawFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  const float bb_frame_width = bb_frame.GetWidth();
  const float bb_frame_height = bb_frame.GetHeight();
  if (bb_frame_width <= 0.0f || bb_frame_height <= 0.0f)
    return false;

  // When we are too small, start hiding and disabling the grab (this reduce
  // visual noise on very small window and facilitate using the window resize
  // grab)
  float alpha = 1.0f;
  if ((axis == Axis_Y) &&
      bb_frame_height < g.FontSize + g.Style.FramePadding.y * 2.0f)
    alpha = Saturate((bb_frame_height - g.FontSize) /
                     (g.Style.FramePadding.y * 2.0f));
  if (alpha <= 0.0f)
    return false;

  const Style &style = g.Style;
  const bool allow_interaction = (alpha >= 1.0f);

  Rect bb = bb_frame;
  bb.Expand(Vec2(-Clamp(TRUNC((bb_frame_width - 2.0f) * 0.5f), 0.0f, 3.0f),
                 -Clamp(TRUNC((bb_frame_height - 2.0f) * 0.5f), 0.0f, 3.0f)));

  // V denote the main, longer axis of the scrollbar (= height for a vertical
  // scrollbar)
  const float scrollbar_size_v =
      (axis == Axis_X) ? bb.GetWidth() : bb.GetHeight();

  // Calculate the height of our grabbable box. It generally represent the
  // amount visible (vs the total scrollable amount) But we maintain a minimum
  // size in pixel to allow for the user to still aim inside.
  ASSERT(Max(size_contents_v, size_avail_v) >
         0.0f); // Adding this assert to check if the Max(XXX,1.0f) is
                // still needed. PLEASE CONTACT ME if this triggers.
  const S64 win_size_v = Max(Max(size_contents_v, size_avail_v), (S64)1);
  const float grab_h_pixels =
      Clamp(scrollbar_size_v * ((float)size_avail_v / (float)win_size_v),
            style.GrabMinSize, scrollbar_size_v);
  const float grab_h_norm = grab_h_pixels / scrollbar_size_v;

  // Handle input right away. None of the code of Begin() is relying on
  // scrolling position before calling Scrollbar().
  bool held = false;
  bool hovered = false;
  ItemAdd(bb_frame, id, NULL, ItemFlags_NoNav);
  ButtonBehavior(bb, id, &hovered, &held, ButtonFlags_NoNavFocus);

  const S64 scroll_max = Max((S64)1, size_contents_v - size_avail_v);
  float scroll_ratio = Saturate((float)*p_scroll_v / (float)scroll_max);
  float grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) /
                      scrollbar_size_v; // Grab position in normalized space
  if (held && allow_interaction && grab_h_norm < 1.0f) {
    const float scrollbar_pos_v = bb.Min[axis];
    const float mouse_pos_v = g.IO.MousePos[axis];

    // Click position in scrollbar normalized space (0.0f->1.0f)
    const float clicked_v_norm =
        Saturate((mouse_pos_v - scrollbar_pos_v) / scrollbar_size_v);
    SetHoveredID(id);

    bool seek_absolute = false;
    if (g.ActiveIdIsJustActivated) {
      // On initial click calculate the distance between mouse and the center of
      // the grab
      seek_absolute = (clicked_v_norm < grab_v_norm ||
                       clicked_v_norm > grab_v_norm + grab_h_norm);
      if (seek_absolute)
        g.ScrollbarClickDeltaToGrabCenter = 0.0f;
      else
        g.ScrollbarClickDeltaToGrabCenter =
            clicked_v_norm - grab_v_norm - grab_h_norm * 0.5f;
    }

    // Apply scroll (p_scroll_v will generally point on one member of
    // window->Scroll) It is ok to modify Scroll here because we are being
    // called in Begin() after the calculation of ContentSize and before setting
    // up our starting position
    const float scroll_v_norm =
        Saturate((clicked_v_norm - g.ScrollbarClickDeltaToGrabCenter -
                  grab_h_norm * 0.5f) /
                 (1.0f - grab_h_norm));
    *p_scroll_v = (S64)(scroll_v_norm * scroll_max);

    // Update values for rendering
    scroll_ratio = Saturate((float)*p_scroll_v / (float)scroll_max);
    grab_v_norm =
        scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;

    // Update distance to grab now that we have seeked and saturated
    if (seek_absolute)
      g.ScrollbarClickDeltaToGrabCenter =
          clicked_v_norm - grab_v_norm - grab_h_norm * 0.5f;
  }

  // Render
  const U32 bg_col = GetColorU32(Col_ScrollbarBg);
  const U32 grab_col = GetColorU32(held      ? Col_ScrollbarGrabActive
                                   : hovered ? Col_ScrollbarGrabHovered
                                             : Col_ScrollbarGrab,
                                   alpha);
  window->DrawList->AddRectFilled(bb_frame.Min, bb_frame.Max, bg_col,
                                  window->WindowRounding, flags);
  Rect grab_rect;
  if (axis == Axis_X)
    grab_rect =
        Rect(Lerp(bb.Min.x, bb.Max.x, grab_v_norm), bb.Min.y,
             Lerp(bb.Min.x, bb.Max.x, grab_v_norm) + grab_h_pixels, bb.Max.y);
  else
    grab_rect = Rect(bb.Min.x, Lerp(bb.Min.y, bb.Max.y, grab_v_norm), bb.Max.x,
                     Lerp(bb.Min.y, bb.Max.y, grab_v_norm) + grab_h_pixels);
  window->DrawList->AddRectFilled(grab_rect.Min, grab_rect.Max, grab_col,
                                  style.ScrollbarRounding);

  return held;
}

void Gui::Image(TextureID user_texture_id, const Vec2 &image_size,
                const Vec2 &uv0, const Vec2 &uv1, const Vec4 &tint_col,
                const Vec4 &border_col) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  const float border_size = (border_col.w > 0.0f) ? 1.0f : 0.0f;
  const Vec2 padding(border_size, border_size);
  const Rect bb(window->DC.CursorPos,
                window->DC.CursorPos + image_size + padding * 2.0f);
  ItemSize(bb);
  if (!ItemAdd(bb, 0))
    return;

  // Render
  if (border_size > 0.0f)
    window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f,
                              DrawFlags_None, border_size);
  window->DrawList->AddImage(user_texture_id, bb.Min + padding,
                             bb.Max - padding, uv0, uv1, GetColorU32(tint_col));
}

// ImageButton() is flawed as 'id' is always derived from 'texture_id' (see
// #2464 #1390) We provide this internal helper to write your own variant while
// we figure out how to redesign the public ImageButton() API.
bool Gui::ImageButtonEx(ID id, TextureID texture_id, const Vec2 &image_size,
                        const Vec2 &uv0, const Vec2 &uv1, const Vec4 &bg_col,
                        const Vec4 &tint_col, ButtonFlags flags) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  const Vec2 padding = g.Style.FramePadding;
  const Rect bb(window->DC.CursorPos,
                window->DC.CursorPos + image_size + padding * 2.0f);
  ItemSize(bb);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  // Render
  const U32 col = GetColorU32((held && hovered) ? Col_ButtonActive
                              : hovered         ? Col_ButtonHovered
                                                : Col_Button);
  RenderNavHighlight(bb, id);
  RenderFrame(
      bb.Min, bb.Max, col, true,
      Clamp((float)Min(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
  if (bg_col.w > 0.0f)
    window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding,
                                    GetColorU32(bg_col));
  window->DrawList->AddImage(texture_id, bb.Min + padding, bb.Max - padding,
                             uv0, uv1, GetColorU32(tint_col));

  return pressed;
}

// Note that ImageButton() adds style.FramePadding*2.0f to provided size. This
// is in order to facilitate fitting an image in a button.
bool Gui::ImageButton(const char *str_id, TextureID user_texture_id,
                      const Vec2 &image_size, const Vec2 &uv0, const Vec2 &uv1,
                      const Vec4 &bg_col, const Vec4 &tint_col) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  return ImageButtonEx(window->GetID(str_id), user_texture_id, image_size, uv0,
                       uv1, bg_col, tint_col);
}

#ifndef DISABLE_OBSOLETE_FUNCTIONS
// Legacy API obsoleted in 1.89. Two differences with new ImageButton()
// - new ImageButton() requires an explicit 'const char* str_id'    Old
// ImageButton() used opaque imTextureId (created issue with: multiple buttons
// with same image, transient texture id values, opaque computation of ID)
// - new ImageButton() always use style.FramePadding                Old
// ImageButton() had an override argument. If you need to change padding with
// new ImageButton() you can use PushStyleVar(StyleVar_FramePadding,
// value), consistent with other Button functions.
bool Gui::ImageButton(TextureID user_texture_id, const Vec2 &size,
                      const Vec2 &uv0, const Vec2 &uv1, int frame_padding,
                      const Vec4 &bg_col, const Vec4 &tint_col) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  // Default to using texture ID as ID. User can still push string/integer
  // prefixes.
  PushID((void *)(intptr_t)user_texture_id);
  const ID id = window->GetID("#image");
  PopID();

  if (frame_padding >= 0)
    PushStyleVar(StyleVar_FramePadding,
                 Vec2((float)frame_padding, (float)frame_padding));
  bool ret =
      ImageButtonEx(id, user_texture_id, size, uv0, uv1, bg_col, tint_col);
  if (frame_padding >= 0)
    PopStyleVar();
  return ret;
}
#endif // #ifndef DISABLE_OBSOLETE_FUNCTIONS

bool Gui::Checkbox(const char *label, bool *v) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const ID id = window->GetID(label);
  const Vec2 label_size = CalcTextSize(label, NULL, true);

  const float square_sz = GetFrameHeight();
  const Vec2 pos = window->DC.CursorPos;
  const Rect total_bb(
      pos, pos + Vec2(square_sz + (label_size.x > 0.0f
                                       ? style.ItemInnerSpacing.x + label_size.x
                                       : 0.0f),
                      label_size.y + style.FramePadding.y * 2.0f));
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id)) {
    TEST_ENGINE_ITEM_INFO(id, label,
                          g.LastItemData.StatusFlags |
                              ItemStatusFlags_Checkable |
                              (*v ? ItemStatusFlags_Checked : 0));
    return false;
  }

  bool hovered, held;
  bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
  if (pressed) {
    *v = !(*v);
    MarkItemEdited(id);
  }

  const Rect check_bb(pos, pos + Vec2(square_sz, square_sz));
  RenderNavHighlight(total_bb, id);
  RenderFrame(check_bb.Min, check_bb.Max,
              GetColorU32((held && hovered) ? Col_FrameBgActive
                          : hovered         ? Col_FrameBgHovered
                                            : Col_FrameBg),
              true, style.FrameRounding);
  U32 check_col = GetColorU32(Col_CheckMark);
  bool mixed_value = (g.LastItemData.InFlags & ItemFlags_MixedValue) != 0;
  if (mixed_value) {
    // Undocumented tristate/mixed/indeterminate checkbox (#2644)
    // This may seem awkwardly designed because the aim is to make
    // ItemFlags_MixedValue supported by all widgets (not just checkbox)
    Vec2 pad(Max(1.0f, TRUNC(square_sz / 3.6f)),
             Max(1.0f, TRUNC(square_sz / 3.6f)));
    window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad,
                                    check_col, style.FrameRounding);
  } else if (*v) {
    const float pad = Max(1.0f, TRUNC(square_sz / 6.0f));
    RenderCheckMark(window->DrawList, check_bb.Min + Vec2(pad, pad), check_col,
                    square_sz - pad * 2.0f);
  }

  Vec2 label_pos = Vec2(check_bb.Max.x + style.ItemInnerSpacing.x,
                        check_bb.Min.y + style.FramePadding.y);
  if (g.LogEnabled)
    LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
  if (label_size.x > 0.0f)
    RenderText(label_pos, label);

  TEST_ENGINE_ITEM_INFO(id, label,
                        g.LastItemData.StatusFlags | ItemStatusFlags_Checkable |
                            (*v ? ItemStatusFlags_Checked : 0));
  return pressed;
}

template <typename T>
bool Gui::CheckboxFlagsT(const char *label, T *flags, T flags_value) {
  bool all_on = (*flags & flags_value) == flags_value;
  bool any_on = (*flags & flags_value) != 0;
  bool pressed;
  if (!all_on && any_on) {
    Context &g = *GGui;
    g.NextItemData.ItemFlags |= ItemFlags_MixedValue;
    pressed = Checkbox(label, &all_on);
  } else {
    pressed = Checkbox(label, &all_on);
  }
  if (pressed) {
    if (all_on)
      *flags |= flags_value;
    else
      *flags &= ~flags_value;
  }
  return pressed;
}

bool Gui::CheckboxFlags(const char *label, int *flags, int flags_value) {
  return CheckboxFlagsT(label, flags, flags_value);
}

bool Gui::CheckboxFlags(const char *label, unsigned int *flags,
                        unsigned int flags_value) {
  return CheckboxFlagsT(label, flags, flags_value);
}

bool Gui::CheckboxFlags(const char *label, S64 *flags, S64 flags_value) {
  return CheckboxFlagsT(label, flags, flags_value);
}

bool Gui::CheckboxFlags(const char *label, U64 *flags, U64 flags_value) {
  return CheckboxFlagsT(label, flags, flags_value);
}

bool Gui::RadioButton(const char *label, bool active) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const ID id = window->GetID(label);
  const Vec2 label_size = CalcTextSize(label, NULL, true);

  const float square_sz = GetFrameHeight();
  const Vec2 pos = window->DC.CursorPos;
  const Rect check_bb(pos, pos + Vec2(square_sz, square_sz));
  const Rect total_bb(
      pos, pos + Vec2(square_sz + (label_size.x > 0.0f
                                       ? style.ItemInnerSpacing.x + label_size.x
                                       : 0.0f),
                      label_size.y + style.FramePadding.y * 2.0f));
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id))
    return false;

  Vec2 center = check_bb.GetCenter();
  center.x = ROUND(center.x);
  center.y = ROUND(center.y);
  const float radius = (square_sz - 1.0f) * 0.5f;

  bool hovered, held;
  bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
  if (pressed)
    MarkItemEdited(id);

  RenderNavHighlight(total_bb, id);
  const int num_segment = window->DrawList->_CalcCircleAutoSegmentCount(radius);
  window->DrawList->AddCircleFilled(center, radius,
                                    GetColorU32((held && hovered)
                                                    ? Col_FrameBgActive
                                                : hovered ? Col_FrameBgHovered
                                                          : Col_FrameBg),
                                    num_segment);
  if (active) {
    const float pad = Max(1.0f, TRUNC(square_sz / 6.0f));
    window->DrawList->AddCircleFilled(center, radius - pad,
                                      GetColorU32(Col_CheckMark));
  }

  if (style.FrameBorderSize > 0.0f) {
    window->DrawList->AddCircle(center + Vec2(1, 1), radius,
                                GetColorU32(Col_BorderShadow), num_segment,
                                style.FrameBorderSize);
    window->DrawList->AddCircle(center, radius, GetColorU32(Col_Border),
                                num_segment, style.FrameBorderSize);
  }

  Vec2 label_pos = Vec2(check_bb.Max.x + style.ItemInnerSpacing.x,
                        check_bb.Min.y + style.FramePadding.y);
  if (g.LogEnabled)
    LogRenderedText(&label_pos, active ? "(x)" : "( )");
  if (label_size.x > 0.0f)
    RenderText(label_pos, label);

  TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
  return pressed;
}

// FIXME: This would work nicely if it was a public template, e.g. 'template<T>
// RadioButton(const char* label, T* v, T v_button)', but I'm not sure how we
// would expose it..
bool Gui::RadioButton(const char *label, int *v, int v_button) {
  const bool pressed = RadioButton(label, *v == v_button);
  if (pressed)
    *v = v_button;
  return pressed;
}

// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified
// size
void Gui::ProgressBar(float fraction, const Vec2 &size_arg,
                      const char *overlay) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  Context &g = *GGui;
  const Style &style = g.Style;

  Vec2 pos = window->DC.CursorPos;
  Vec2 size = CalcItemSize(size_arg, CalcItemWidth(),
                           g.FontSize + style.FramePadding.y * 2.0f);
  Rect bb(pos, pos + size);
  ItemSize(size, style.FramePadding.y);
  if (!ItemAdd(bb, 0))
    return;

  // Render
  fraction = Saturate(fraction);
  RenderFrame(bb.Min, bb.Max, GetColorU32(Col_FrameBg), true,
              style.FrameRounding);
  bb.Expand(Vec2(-style.FrameBorderSize, -style.FrameBorderSize));
  const Vec2 fill_br = Vec2(Lerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
  RenderRectFilledRangeH(window->DrawList, bb, GetColorU32(Col_PlotHistogram),
                         0.0f, fraction, style.FrameRounding);

  // Default displaying the fraction as percentage string, but user can override
  // it
  char overlay_buf[32];
  if (!overlay) {
    FormatString(overlay_buf, ARRAYSIZE(overlay_buf), "%.0f%%",
                 fraction * 100 + 0.01f);
    overlay = overlay_buf;
  }

  Vec2 overlay_size = CalcTextSize(overlay, NULL);
  if (overlay_size.x > 0.0f)
    RenderTextClipped(
        Vec2(Clamp(fill_br.x + style.ItemSpacing.x, bb.Min.x,
                   bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x),
             bb.Min.y),
        bb.Max, overlay, NULL, &overlay_size, Vec2(0.0f, 0.5f), &bb);
}

void Gui::Bullet() {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  Context &g = *GGui;
  const Style &style = g.Style;
  const float line_height =
      Max(Min(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2),
          g.FontSize);
  const Rect bb(window->DC.CursorPos,
                window->DC.CursorPos + Vec2(g.FontSize, line_height));
  ItemSize(bb);
  if (!ItemAdd(bb, 0)) {
    SameLine(0, style.FramePadding.x * 2);
    return;
  }

  // Render and stay on same line
  U32 text_col = GetColorU32(Col_Text);
  RenderBullet(window->DrawList,
               bb.Min + Vec2(style.FramePadding.x + g.FontSize * 0.5f,
                             line_height * 0.5f),
               text_col);
  SameLine(0, style.FramePadding.x * 2.0f);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Low-level Layout helpers
//-------------------------------------------------------------------------
// - Spacing()
// - Dummy()
// - NewLine()
// - AlignTextToFramePadding()
// - SeparatorEx() [Internal]
// - Separator()
// - SplitterBehavior() [Internal]
// - ShrinkWidths() [Internal]
//-------------------------------------------------------------------------

void Gui::Spacing() {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;
  ItemSize(Vec2(0, 0));
}

void Gui::Dummy(const Vec2 &size) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  const Rect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  ItemSize(size);
  ItemAdd(bb, 0);
}

void Gui::NewLine() {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  Context &g = *GGui;
  const LayoutType backup_layout_type = window->DC.LayoutType;
  window->DC.LayoutType = LayoutType_Vertical;
  window->DC.IsSameLine = false;
  if (window->DC.CurrLineSize.y >
      0.0f) // In the event that we are on a line with items that is smaller
            // that FontSize high, we will preserve its height.
    ItemSize(Vec2(0, 0));
  else
    ItemSize(Vec2(0.0f, g.FontSize));
  window->DC.LayoutType = backup_layout_type;
}

void Gui::AlignTextToFramePadding() {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  Context &g = *GGui;
  window->DC.CurrLineSize.y =
      Max(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2);
  window->DC.CurrLineTextBaseOffset =
      Max(window->DC.CurrLineTextBaseOffset, g.Style.FramePadding.y);
}

// Horizontal/vertical separating line
// FIXME: Surprisingly, this seemingly trivial widget is a victim of many
// different legacy/tricky layout issues. Note how thickness == 1.0f is handled
// specifically as not moving CursorPos by 'thickness', but other values are.
void Gui::SeparatorEx(SeparatorFlags flags, float thickness) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  Context &g = *GGui;
  ASSERT(IsPowerOfTwo(
      flags &
      (SeparatorFlags_Horizontal |
       SeparatorFlags_Vertical))); // Check that only 1 option is selected
  ASSERT(thickness > 0.0f);

  if (flags & SeparatorFlags_Vertical) {
    // Vertical separator, for menu bars (use current line height).
    float y1 = window->DC.CursorPos.y;
    float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
    const Rect bb(Vec2(window->DC.CursorPos.x, y1),
                  Vec2(window->DC.CursorPos.x + thickness, y2));
    ItemSize(Vec2(thickness, 0.0f));
    if (!ItemAdd(bb, 0))
      return;

    // Draw
    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(Col_Separator));
    if (g.LogEnabled)
      LogText(" |");
  } else if (flags & SeparatorFlags_Horizontal) {
    // Horizontal Separator
    float x1 = window->DC.CursorPos.x;
    float x2 = window->WorkRect.Max.x;

    // Preserve legacy behavior inside Columns()
    // Before Tables API happened, we relied on Separator() to span all columns
    // of a Columns() set. We currently don't need to provide the same feature
    // for tables because tables naturally have border features.
    OldColumns *columns = (flags & SeparatorFlags_SpanAllColumns)
                              ? window->DC.CurrentColumns
                              : NULL;
    if (columns) {
      x1 = window->Pos.x +
           window->DC.Indent.x; // Used to be Pos.x before 2023/10/03
      x2 = window->Pos.x + window->Size.x;
      PushColumnsBackground();
    }

    // We don't provide our width to the layout so that it doesn't get feed back
    // into AutoFit
    // FIXME: This prevents ->CursorMaxPos based bounding box evaluation from
    // working (e.g. TableEndCell)
    const float thickness_for_layout =
        (thickness == 1.0f)
            ? 0.0f
            : thickness; // FIXME: See 1.70/1.71 Separator() change: makes
                         // legacy 1-px separator not affect layout yet. Should
                         // change.
    const Rect bb(Vec2(x1, window->DC.CursorPos.y),
                  Vec2(x2, window->DC.CursorPos.y + thickness));
    ItemSize(Vec2(0.0f, thickness_for_layout));

    if (ItemAdd(bb, 0)) {
      // Draw
      window->DrawList->AddRectFilled(bb.Min, bb.Max,
                                      GetColorU32(Col_Separator));
      if (g.LogEnabled)
        LogRenderedText(&bb.Min, "--------------------------------\n");
    }
    if (columns) {
      PopColumnsBackground();
      columns->LineMinY = window->DC.CursorPos.y;
    }
  }
}

void Gui::Separator() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  // Those flags should eventually be configurable by the user
  // FIXME: We cannot g.Style.SeparatorTextBorderSize for thickness as it
  // relates to SeparatorText() which is a decorated separator, not defaulting
  // to 1.0f.
  SeparatorFlags flags = (window->DC.LayoutType == LayoutType_Horizontal)
                             ? SeparatorFlags_Vertical
                             : SeparatorFlags_Horizontal;

  // Only applies to legacy Columns() api as they relied on Separator() a lot.
  if (window->DC.CurrentColumns)
    flags |= SeparatorFlags_SpanAllColumns;

  SeparatorEx(flags, 1.0f);
}

void Gui::SeparatorTextEx(ID id, const char *label, const char *label_end,
                          float extra_w) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Style &style = g.Style;

  const Vec2 label_size = CalcTextSize(label, label_end, false);
  const Vec2 pos = window->DC.CursorPos;
  const Vec2 padding = style.SeparatorTextPadding;

  const float separator_thickness = style.SeparatorTextBorderSize;
  const Vec2 min_size(
      label_size.x + extra_w + padding.x * 2.0f,
      Max(label_size.y + padding.y * 2.0f, separator_thickness));
  const Rect bb(pos, Vec2(window->WorkRect.Max.x, pos.y + min_size.y));
  const float text_baseline_y =
      Trunc((bb.GetHeight() - label_size.y) * style.SeparatorTextAlign.y +
            0.99999f); // Max(padding.y, Trunc((style.SeparatorTextSize -
                       // label_size.y) * 0.5f));
  ItemSize(min_size, text_baseline_y);
  if (!ItemAdd(bb, id))
    return;

  const float sep1_x1 = pos.x;
  const float sep2_x2 = bb.Max.x;
  const float seps_y = Trunc((bb.Min.y + bb.Max.y) * 0.5f + 0.99999f);

  const float label_avail_w = Max(0.0f, sep2_x2 - sep1_x1 - padding.x * 2.0f);
  const Vec2 label_pos(pos.x + padding.x +
                           Max(0.0f, (label_avail_w - label_size.x - extra_w) *
                                         style.SeparatorTextAlign.x),
                       pos.y + text_baseline_y); // FIXME-ALIGN

  // This allows using SameLine() to position something in the 'extra_w'
  window->DC.CursorPosPrevLine.x = label_pos.x + label_size.x;

  const U32 separator_col = GetColorU32(Col_Separator);
  if (label_size.x > 0.0f) {
    const float sep1_x2 = label_pos.x - style.ItemSpacing.x;
    const float sep2_x1 =
        label_pos.x + label_size.x + extra_w + style.ItemSpacing.x;
    if (sep1_x2 > sep1_x1 && separator_thickness > 0.0f)
      window->DrawList->AddLine(Vec2(sep1_x1, seps_y), Vec2(sep1_x2, seps_y),
                                separator_col, separator_thickness);
    if (sep2_x2 > sep2_x1 && separator_thickness > 0.0f)
      window->DrawList->AddLine(Vec2(sep2_x1, seps_y), Vec2(sep2_x2, seps_y),
                                separator_col, separator_thickness);
    if (g.LogEnabled)
      LogSetNextTextDecoration("---", NULL);
    RenderTextEllipsis(window->DrawList, label_pos,
                       Vec2(bb.Max.x, bb.Max.y + style.ItemSpacing.y), bb.Max.x,
                       bb.Max.x, label, label_end, &label_size);
  } else {
    if (g.LogEnabled)
      LogText("---");
    if (separator_thickness > 0.0f)
      window->DrawList->AddLine(Vec2(sep1_x1, seps_y), Vec2(sep2_x2, seps_y),
                                separator_col, separator_thickness);
  }
}

void Gui::SeparatorText(const char *label) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  // The SeparatorText() vs SeparatorTextEx() distinction is designed to be
  // considerate that we may want:
  // - allow separator-text to be draggable items (would require a stable ID + a
  // noticeable highlight)
  // - this high-level entry point to allow formatting? (which in turns may
  // require ID separate from formatted string)
  // - because of this we probably can't turn 'const char* label' into 'const
  // char* fmt, ...' Otherwise, we can decide that users wanting to drag this
  // would layout a dedicated drag-item, and then we can turn this into a format
  // function.
  SeparatorTextEx(0, label, FindRenderedTextEnd(label), 0.0f);
}

// Using 'hover_visibility_delay' allows us to hide the highlight and mouse
// cursor for a short time, which can be convenient to reduce visual noise.
bool Gui::SplitterBehavior(const Rect &bb, ID id, Axis axis, float *size1,
                           float *size2, float min_size1, float min_size2,
                           float hover_extend, float hover_visibility_delay,
                           U32 bg_col) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  if (!ItemAdd(bb, id, NULL, ItemFlags_NoNav))
    return false;

  // FIXME: AFAIK the only leftover reason for passing
  // ButtonFlags_AllowOverlap here is to allow caller of SplitterBehavior()
  // to call SetItemAllowOverlap() after the item. Nowadays we would instead
  // want to use SetNextItemAllowOverlap() before the item.
  ButtonFlags button_flags = ButtonFlags_FlattenChildren;
#ifndef DISABLE_OBSOLETE_FUNCTIONS
  button_flags |= ButtonFlags_AllowOverlap;
#endif

  bool hovered, held;
  Rect bb_interact = bb;
  bb_interact.Expand(axis == Axis_Y ? Vec2(0.0f, hover_extend)
                                    : Vec2(hover_extend, 0.0f));
  ButtonBehavior(bb_interact, id, &hovered, &held, button_flags);
  if (hovered)
    g.LastItemData.StatusFlags |=
        ItemStatusFlags_HoveredRect; // for IsItemHovered(), because
                                     // bb_interact is larger than bb

  if (held || (hovered && g.HoveredIdPreviousFrame == id &&
               g.HoveredIdTimer >= hover_visibility_delay))
    SetMouseCursor(axis == Axis_Y ? MouseCursor_ResizeNS
                                  : MouseCursor_ResizeEW);

  Rect bb_render = bb;
  if (held) {
    float mouse_delta =
        (g.IO.MousePos - g.ActiveIdClickOffset - bb_interact.Min)[axis];

    // Minimum pane size
    float size_1_maximum_delta = Max(0.0f, *size1 - min_size1);
    float size_2_maximum_delta = Max(0.0f, *size2 - min_size2);
    if (mouse_delta < -size_1_maximum_delta)
      mouse_delta = -size_1_maximum_delta;
    if (mouse_delta > size_2_maximum_delta)
      mouse_delta = size_2_maximum_delta;

    // Apply resize
    if (mouse_delta != 0.0f) {
      *size1 = Max(*size1 + mouse_delta, min_size1);
      *size2 = Max(*size2 - mouse_delta, min_size2);
      bb_render.Translate((axis == Axis_X) ? Vec2(mouse_delta, 0.0f)
                                           : Vec2(0.0f, mouse_delta));
      MarkItemEdited(id);
    }
  }

  // Render at new position
  if (bg_col & COL32_A_MASK)
    window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, bg_col, 0.0f);
  const U32 col =
      GetColorU32(held ? Col_SeparatorActive
                  : (hovered && g.HoveredIdTimer >= hover_visibility_delay)
                      ? Col_SeparatorHovered
                      : Col_Separator);
  window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, col, 0.0f);

  return held;
}

static int CDECL ShrinkWidthItemComparer(const void *lhs, const void *rhs) {
  const ShrinkWidthItem *a = (const ShrinkWidthItem *)lhs;
  const ShrinkWidthItem *b = (const ShrinkWidthItem *)rhs;
  if (int d = (int)(b->Width - a->Width))
    return d;
  return (b->Index - a->Index);
}

// Shrink excess width from a set of item, by removing width from the larger
// items first. Set items Width to -1.0f to disable shrinking this item.
void Gui::ShrinkWidths(ShrinkWidthItem *items, int count, float width_excess) {
  if (count == 1) {
    if (items[0].Width >= 0.0f)
      items[0].Width = Max(items[0].Width - width_excess, 1.0f);
    return;
  }
  Qsort(items, (size_t)count, sizeof(ShrinkWidthItem), ShrinkWidthItemComparer);
  int count_same_width = 1;
  while (width_excess > 0.0f && count_same_width < count) {
    while (count_same_width < count &&
           items[0].Width <= items[count_same_width].Width)
      count_same_width++;
    float max_width_to_remove_per_item =
        (count_same_width < count && items[count_same_width].Width >= 0.0f)
            ? (items[0].Width - items[count_same_width].Width)
            : (items[0].Width - 1.0f);
    if (max_width_to_remove_per_item <= 0.0f)
      break;
    float width_to_remove_per_item =
        Min(width_excess / count_same_width, max_width_to_remove_per_item);
    for (int item_n = 0; item_n < count_same_width; item_n++)
      items[item_n].Width -= width_to_remove_per_item;
    width_excess -= width_to_remove_per_item * count_same_width;
  }

  // Round width and redistribute remainder
  // Ensure that e.g. the right-most tab of a shrunk tab-bar always reaches
  // exactly at the same distance from the right-most edge of the tab bar
  // separator.
  width_excess = 0.0f;
  for (int n = 0; n < count; n++) {
    float width_rounded = Trunc(items[n].Width);
    width_excess += items[n].Width - width_rounded;
    items[n].Width = width_rounded;
  }
  while (width_excess > 0.0f)
    for (int n = 0; n < count && width_excess > 0.0f; n++) {
      float width_to_add = Min(items[n].InitialWidth - items[n].Width, 1.0f);
      items[n].Width += width_to_add;
      width_excess -= width_to_add;
    }
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ComboBox
//-------------------------------------------------------------------------
// - CalcMaxPopupHeightFromItemCount() [Internal]
// - BeginCombo()
// - BeginComboPopup() [Internal]
// - EndCombo()
// - BeginComboPreview() [Internal]
// - EndComboPreview() [Internal]
// - Combo()
//-------------------------------------------------------------------------

static float CalcMaxPopupHeightFromItemCount(int items_count) {
  Context &g = *GGui;
  if (items_count <= 0)
    return FLT_MAX;
  return (g.FontSize + g.Style.ItemSpacing.y) * items_count -
         g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool Gui::BeginCombo(const char *label, const char *preview_value,
                     ComboFlags flags) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();

  NextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
  g.NextWindowData
      .ClearFlags(); // We behave like Begin() and need to consume those values
  if (window->SkipItems)
    return false;

  const Style &style = g.Style;
  const ID id = window->GetID(label);
  ASSERT((flags & (ComboFlags_NoArrowButton | ComboFlags_NoPreview)) !=
         (ComboFlags_NoArrowButton |
          ComboFlags_NoPreview)); // Can't use both flags together
  if (flags & ComboFlags_WidthFitPreview)
    ASSERT((flags & (ComboFlags_NoPreview |
                     (ComboFlags)ComboFlags_CustomPreview)) == 0);

  const float arrow_size =
      (flags & ComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
  const Vec2 label_size = CalcTextSize(label, NULL, true);
  const float preview_width =
      ((flags & ComboFlags_WidthFitPreview) && (preview_value != NULL))
          ? CalcTextSize(preview_value, NULL, true).x
          : 0.0f;
  const float w =
      (flags & ComboFlags_NoPreview)
          ? arrow_size
          : ((flags & ComboFlags_WidthFitPreview)
                 ? (arrow_size + preview_width + style.FramePadding.x * 2.0f)
                 : CalcItemWidth());
  const Rect bb(window->DC.CursorPos,
                window->DC.CursorPos +
                    Vec2(w, label_size.y + style.FramePadding.y * 2.0f));
  const Rect total_bb(
      bb.Min, bb.Max + Vec2(label_size.x > 0.0f
                                ? style.ItemInnerSpacing.x + label_size.x
                                : 0.0f,
                            0.0f));
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id, &bb))
    return false;

  // Open on click
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held);
  const ID popup_id = HashStr("##ComboPopup", 0, id);
  bool popup_open = IsPopupOpen(popup_id, PopupFlags_None);
  if (pressed && !popup_open) {
    OpenPopupEx(popup_id, PopupFlags_None);
    popup_open = true;
  }

  // Render shape
  const U32 frame_col = GetColorU32(hovered ? Col_FrameBgHovered : Col_FrameBg);
  const float value_x2 = Max(bb.Min.x, bb.Max.x - arrow_size);
  RenderNavHighlight(bb, id);
  if (!(flags & ComboFlags_NoPreview))
    window->DrawList->AddRectFilled(
        bb.Min, Vec2(value_x2, bb.Max.y), frame_col, style.FrameRounding,
        (flags & ComboFlags_NoArrowButton) ? DrawFlags_RoundCornersAll
                                           : DrawFlags_RoundCornersLeft);
  if (!(flags & ComboFlags_NoArrowButton)) {
    U32 bg_col =
        GetColorU32((popup_open || hovered) ? Col_ButtonHovered : Col_Button);
    U32 text_col = GetColorU32(Col_Text);
    window->DrawList->AddRectFilled(
        Vec2(value_x2, bb.Min.y), bb.Max, bg_col, style.FrameRounding,
        (w <= arrow_size) ? DrawFlags_RoundCornersAll
                          : DrawFlags_RoundCornersRight);
    if (value_x2 + arrow_size - style.FramePadding.x <= bb.Max.x)
      RenderArrow(window->DrawList,
                  Vec2(value_x2 + style.FramePadding.y,
                       bb.Min.y + style.FramePadding.y),
                  text_col, Dir_Down, 1.0f);
  }
  RenderFrameBorder(bb.Min, bb.Max, style.FrameRounding);

  // Custom preview
  if (flags & ComboFlags_CustomPreview) {
    g.ComboPreviewData.PreviewRect =
        Rect(bb.Min.x, bb.Min.y, value_x2, bb.Max.y);
    ASSERT(preview_value == NULL || preview_value[0] == 0);
    preview_value = NULL;
  }

  // Render preview and label
  if (preview_value != NULL && !(flags & ComboFlags_NoPreview)) {
    if (g.LogEnabled)
      LogSetNextTextDecoration("{", "}");
    RenderTextClipped(bb.Min + style.FramePadding, Vec2(value_x2, bb.Max.y),
                      preview_value, NULL, NULL);
  }
  if (label_size.x > 0)
    RenderText(Vec2(bb.Max.x + style.ItemInnerSpacing.x,
                    bb.Min.y + style.FramePadding.y),
               label);

  if (!popup_open)
    return false;

  g.NextWindowData.Flags = backup_next_window_data_flags;
  return BeginComboPopup(popup_id, bb, flags);
}

bool Gui::BeginComboPopup(ID popup_id, const Rect &bb, ComboFlags flags) {
  Context &g = *GGui;
  if (!IsPopupOpen(popup_id, PopupFlags_None)) {
    g.NextWindowData.ClearFlags();
    return false;
  }

  // Set popup size
  float w = bb.GetWidth();
  if (g.NextWindowData.Flags & NextWindowDataFlags_HasSizeConstraint) {
    g.NextWindowData.SizeConstraintRect.Min.x =
        Max(g.NextWindowData.SizeConstraintRect.Min.x, w);
  } else {
    if ((flags & ComboFlags_HeightMask_) == 0)
      flags |= ComboFlags_HeightRegular;
    ASSERT(IsPowerOfTwo(flags & ComboFlags_HeightMask_)); // Only one
    int popup_max_height_in_items = -1;
    if (flags & ComboFlags_HeightRegular)
      popup_max_height_in_items = 8;
    else if (flags & ComboFlags_HeightSmall)
      popup_max_height_in_items = 4;
    else if (flags & ComboFlags_HeightLarge)
      popup_max_height_in_items = 20;
    Vec2 constraint_min(0.0f, 0.0f), constraint_max(FLT_MAX, FLT_MAX);
    if ((g.NextWindowData.Flags & NextWindowDataFlags_HasSize) == 0 ||
        g.NextWindowData.SizeVal.x <=
            0.0f) // Don't apply constraints if user specified a size
      constraint_min.x = w;
    if ((g.NextWindowData.Flags & NextWindowDataFlags_HasSize) == 0 ||
        g.NextWindowData.SizeVal.y <= 0.0f)
      constraint_max.y =
          CalcMaxPopupHeightFromItemCount(popup_max_height_in_items);
    SetNextWindowSizeConstraints(constraint_min, constraint_max);
  }

  // This is essentially a specialized version of BeginPopupEx()
  char name[16];
  FormatString(name, ARRAYSIZE(name), "##Combo_%02d",
               g.BeginPopupStack.Size); // Recycle windows based on depth

  // Set position given a custom constraint (peak into expected window size so
  // we can position it)
  // FIXME: This might be easier to express with an hypothetical
  // SetNextWindowPosConstraints() function?
  // FIXME: This might be moved to Begin() or at least around the same spot
  // where Tooltips and other Popups are calling FindBestWindowPosForPopupEx()?
  if (Window *popup_window = FindWindowByName(name))
    if (popup_window->WasActive) {
      // Always override 'AutoPosLastDirection' to not leave a chance for a past
      // value to affect us.
      Vec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
      popup_window->AutoPosLastDirection =
          (flags & ComboFlags_PopupAlignLeft)
              ? Dir_Left
              : Dir_Down; // Left = "Below, Toward Left", Down = "Below,
                          // Toward Right (default)"
      Rect r_outer = GetPopupAllowedExtentRect(popup_window);
      Vec2 pos = FindBestWindowPosForPopupEx(
          bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection,
          r_outer, bb, PopupPositionPolicy_ComboBox);
      SetNextWindowPos(pos);
    }

  // We don't use BeginPopupEx() solely because we have a custom name string,
  // which we could make an argument to BeginPopupEx()
  WindowFlags window_flags = WindowFlags_AlwaysAutoResize | WindowFlags_Popup |
                             WindowFlags_NoTitleBar | WindowFlags_NoResize |
                             WindowFlags_NoSavedSettings | WindowFlags_NoMove;
  PushStyleVar(
      StyleVar_WindowPadding,
      Vec2(g.Style.FramePadding.x,
           g.Style.WindowPadding
               .y)); // Horizontally align ourselves with the framed text
  bool ret = Begin(name, NULL, window_flags);
  PopStyleVar();
  if (!ret) {
    EndPopup();
    ASSERT(0); // This should never happen as we tested for IsPopupOpen() above
    return false;
  }
  return true;
}

void Gui::EndCombo() { EndPopup(); }

// Call directly after the BeginCombo/EndCombo block. The preview is designed to
// only host non-interactive elements (Experimental, see GitHub issues: #1658,
// #4168)
bool Gui::BeginComboPreview() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ComboPreviewData *preview_data = &g.ComboPreviewData;

  if (window->SkipItems ||
      !(g.LastItemData.StatusFlags & ItemStatusFlags_Visible))
    return false;
  ASSERT(g.LastItemData.Rect.Min.x == preview_data->PreviewRect.Min.x &&
         g.LastItemData.Rect.Min.y ==
             preview_data->PreviewRect.Min
                 .y); // Didn't call after BeginCombo/EndCombo block or
                      // forgot to pass ComboFlags_CustomPreview flag?
  if (!window->ClipRect.Overlaps(
          preview_data->PreviewRect)) // Narrower test (optional)
    return false;

  // FIXME: This could be contained in a PushWorkRect() api
  preview_data->BackupCursorPos = window->DC.CursorPos;
  preview_data->BackupCursorMaxPos = window->DC.CursorMaxPos;
  preview_data->BackupCursorPosPrevLine = window->DC.CursorPosPrevLine;
  preview_data->BackupPrevLineTextBaseOffset =
      window->DC.PrevLineTextBaseOffset;
  preview_data->BackupLayout = window->DC.LayoutType;
  window->DC.CursorPos = preview_data->PreviewRect.Min + g.Style.FramePadding;
  window->DC.CursorMaxPos = window->DC.CursorPos;
  window->DC.LayoutType = LayoutType_Horizontal;
  window->DC.IsSameLine = false;
  PushClipRect(preview_data->PreviewRect.Min, preview_data->PreviewRect.Max,
               true);

  return true;
}

void Gui::EndComboPreview() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ComboPreviewData *preview_data = &g.ComboPreviewData;

  // FIXME: Using CursorMaxPos approximation instead of correct AABB which we
  // will store in DrawCmd in the future
  DrawList *draw_list = window->DrawList;
  if (window->DC.CursorMaxPos.x < preview_data->PreviewRect.Max.x &&
      window->DC.CursorMaxPos.y < preview_data->PreviewRect.Max.y)
    if (draw_list->CmdBuffer.Size >
        1) // Unlikely case that the PushClipRect() didn't create a command
    {
      draw_list->_CmdHeader.ClipRect =
          draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ClipRect =
              draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 2].ClipRect;
      draw_list->_TryMergeDrawCmds();
    }
  PopClipRect();
  window->DC.CursorPos = preview_data->BackupCursorPos;
  window->DC.CursorMaxPos =
      Max(window->DC.CursorMaxPos, preview_data->BackupCursorMaxPos);
  window->DC.CursorPosPrevLine = preview_data->BackupCursorPosPrevLine;
  window->DC.PrevLineTextBaseOffset =
      preview_data->BackupPrevLineTextBaseOffset;
  window->DC.LayoutType = preview_data->BackupLayout;
  window->DC.IsSameLine = false;
  preview_data->PreviewRect = Rect();
}

// Getter for the old Combo() API: const char*[]
static const char *Items_ArrayGetter(void *data, int idx) {
  const char *const *items = (const char *const *)data;
  return items[idx];
}

// Getter for the old Combo() API: "item1\0item2\0item3\0"
static const char *Items_SingleStringGetter(void *data, int idx) {
  const char *items_separated_by_zeros = (const char *)data;
  int items_count = 0;
  const char *p = items_separated_by_zeros;
  while (*p) {
    if (idx == items_count)
      break;
    p += strlen(p) + 1;
    items_count++;
  }
  return *p ? p : NULL;
}

// Old API, prefer using BeginCombo() nowadays if you can.
bool Gui::Combo(const char *label, int *current_item,
                const char *(*getter)(void *user_data, int idx),
                void *user_data, int items_count,
                int popup_max_height_in_items) {
  Context &g = *GGui;

  // Call the getter to obtain the preview string which is a parameter to
  // BeginCombo()
  const char *preview_value = NULL;
  if (*current_item >= 0 && *current_item < items_count)
    preview_value = getter(user_data, *current_item);

  // The old Combo() API exposed "popup_max_height_in_items". The new more
  // general BeginCombo() API doesn't have/need it, but we emulate it here.
  if (popup_max_height_in_items != -1 &&
      !(g.NextWindowData.Flags & NextWindowDataFlags_HasSizeConstraint))
    SetNextWindowSizeConstraints(Vec2(0, 0),
                                 Vec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(
                                                   popup_max_height_in_items)));

  if (!BeginCombo(label, preview_value, ComboFlags_None))
    return false;

  // Display items
  // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to
  // make sure our call to SetItemDefaultFocus() is processed)
  bool value_changed = false;
  for (int i = 0; i < items_count; i++) {
    const char *item_text = getter(user_data, i);
    if (item_text == NULL)
      item_text = "*Unknown item*";

    PushID(i);
    const bool item_selected = (i == *current_item);
    if (Selectable(item_text, item_selected) && *current_item != i) {
      value_changed = true;
      *current_item = i;
    }
    if (item_selected)
      SetItemDefaultFocus();
    PopID();
  }

  EndCombo();

  if (value_changed)
    MarkItemEdited(g.LastItemData.ID);

  return value_changed;
}

// Combo box helper allowing to pass an array of strings.
bool Gui::Combo(const char *label, int *current_item, const char *const items[],
                int items_count, int height_in_items) {
  const bool value_changed = Combo(label, current_item, Items_ArrayGetter,
                                   (void *)items, items_count, height_in_items);
  return value_changed;
}

// Combo box helper allowing to pass all items in a single string literal
// holding multiple zero-terminated items "item1\0item2\0"
bool Gui::Combo(const char *label, int *current_item,
                const char *items_separated_by_zeros, int height_in_items) {
  int items_count = 0;
  const char *p =
      items_separated_by_zeros; // FIXME-OPT: Avoid computing this, or at least
                                // only when combo is open
  while (*p) {
    p += strlen(p) + 1;
    items_count++;
  }
  bool value_changed =
      Combo(label, current_item, Items_SingleStringGetter,
            (void *)items_separated_by_zeros, items_count, height_in_items);
  return value_changed;
}

#ifndef DISABLE_OBSOLETE_FUNCTIONS

struct GetNameFromIndexOldToNewCallbackData {
  void *UserData;
  bool (*OldCallback)(void *, int, const char **);
};
static const char *GetNameFromIndexOldToNewCallback(void *user_data, int idx) {
  GetNameFromIndexOldToNewCallbackData *data =
      (GetNameFromIndexOldToNewCallbackData *)user_data;
  const char *s = NULL;
  data->OldCallback(data->UserData, idx, &s);
  return s;
}

bool Gui::ListBox(const char *label, int *current_item,
                  bool (*old_getter)(void *, int, const char **),
                  void *user_data, int items_count, int height_in_items) {
  GetNameFromIndexOldToNewCallbackData old_to_new_data = {user_data,
                                                          old_getter};
  return ListBox(label, current_item, GetNameFromIndexOldToNewCallback,
                 &old_to_new_data, items_count, height_in_items);
}
bool Gui::Combo(const char *label, int *current_item,
                bool (*old_getter)(void *, int, const char **), void *user_data,
                int items_count, int popup_max_height_in_items) {
  GetNameFromIndexOldToNewCallbackData old_to_new_data = {user_data,
                                                          old_getter};
  return Combo(label, current_item, GetNameFromIndexOldToNewCallback,
               &old_to_new_data, items_count, popup_max_height_in_items);
}

#endif

//-------------------------------------------------------------------------
// [SECTION] Data Type and Data Formatting Helpers [Internal]
//-------------------------------------------------------------------------
// - DataTypeGetInfo()
// - DataTypeFormatString()
// - DataTypeApplyOp()
// - DataTypeApplyOpFromText()
// - DataTypeCompare()
// - DataTypeClamp()
// - GetMinimumStepAtDecimalPrecision
// - RoundScalarWithFormat<>()
//-------------------------------------------------------------------------

static const DataTypeInfo GDataTypeInfo[] = {
    {sizeof(char), "S8", "%d", "%d"}, // DataType_S8
    {sizeof(unsigned char), "U8", "%u", "%u"},
    {sizeof(short), "S16", "%d", "%d"}, // DataType_S16
    {sizeof(unsigned short), "U16", "%u", "%u"},
    {sizeof(int), "S32", "%d", "%d"}, // DataType_S32
    {sizeof(unsigned int), "U32", "%u", "%u"},
#ifdef _MSC_VER
    {sizeof(S64), "S64", "%I64d", "%I64d"}, // DataType_S64
    {sizeof(U64), "U64", "%I64u", "%I64u"},
#else
    {sizeof(S64), "S64", "%lld", "%lld"}, // DataType_S64
    {sizeof(U64), "U64", "%llu", "%llu"},
#endif
    {sizeof(float), "float", "%.3f",
     "%f"}, // DataType_Float (float are promoted to double in va_arg)
    {sizeof(double), "double", "%f", "%lf"}, // DataType_Double
};
STATIC_ASSERT(ARRAYSIZE(GDataTypeInfo) == DataType_COUNT);

const DataTypeInfo *Gui::DataTypeGetInfo(DataType data_type) {
  ASSERT(data_type >= 0 && data_type < DataType_COUNT);
  return &GDataTypeInfo[data_type];
}

int Gui::DataTypeFormatString(char *buf, int buf_size, DataType data_type,
                              const void *p_data, const char *format) {
  // Signedness doesn't matter when pushing integer arguments
  if (data_type == DataType_S32 || data_type == DataType_U32)
    return FormatString(buf, buf_size, format, *(const U32 *)p_data);
  if (data_type == DataType_S64 || data_type == DataType_U64)
    return FormatString(buf, buf_size, format, *(const U64 *)p_data);
  if (data_type == DataType_Float)
    return FormatString(buf, buf_size, format, *(const float *)p_data);
  if (data_type == DataType_Double)
    return FormatString(buf, buf_size, format, *(const double *)p_data);
  if (data_type == DataType_S8)
    return FormatString(buf, buf_size, format, *(const S8 *)p_data);
  if (data_type == DataType_U8)
    return FormatString(buf, buf_size, format, *(const U8 *)p_data);
  if (data_type == DataType_S16)
    return FormatString(buf, buf_size, format, *(const S16 *)p_data);
  if (data_type == DataType_U16)
    return FormatString(buf, buf_size, format, *(const U16 *)p_data);
  ASSERT(0);
  return 0;
}

void Gui::DataTypeApplyOp(DataType data_type, int op, void *output,
                          const void *arg1, const void *arg2) {
  ASSERT(op == '+' || op == '-');
  switch (data_type) {
  case DataType_S8:
    if (op == '+') {
      *(S8 *)output = AddClampOverflow(*(const S8 *)arg1, *(const S8 *)arg2,
                                       S8_MIN, S8_MAX);
    }
    if (op == '-') {
      *(S8 *)output = SubClampOverflow(*(const S8 *)arg1, *(const S8 *)arg2,
                                       S8_MIN, S8_MAX);
    }
    return;
  case DataType_U8:
    if (op == '+') {
      *(U8 *)output = AddClampOverflow(*(const U8 *)arg1, *(const U8 *)arg2,
                                       U8_MIN, U8_MAX);
    }
    if (op == '-') {
      *(U8 *)output = SubClampOverflow(*(const U8 *)arg1, *(const U8 *)arg2,
                                       U8_MIN, U8_MAX);
    }
    return;
  case DataType_S16:
    if (op == '+') {
      *(S16 *)output = AddClampOverflow(*(const S16 *)arg1, *(const S16 *)arg2,
                                        S16_MIN, S16_MAX);
    }
    if (op == '-') {
      *(S16 *)output = SubClampOverflow(*(const S16 *)arg1, *(const S16 *)arg2,
                                        S16_MIN, S16_MAX);
    }
    return;
  case DataType_U16:
    if (op == '+') {
      *(U16 *)output = AddClampOverflow(*(const U16 *)arg1, *(const U16 *)arg2,
                                        U16_MIN, U16_MAX);
    }
    if (op == '-') {
      *(U16 *)output = SubClampOverflow(*(const U16 *)arg1, *(const U16 *)arg2,
                                        U16_MIN, U16_MAX);
    }
    return;
  case DataType_S32:
    if (op == '+') {
      *(S32 *)output = AddClampOverflow(*(const S32 *)arg1, *(const S32 *)arg2,
                                        S32_MIN, S32_MAX);
    }
    if (op == '-') {
      *(S32 *)output = SubClampOverflow(*(const S32 *)arg1, *(const S32 *)arg2,
                                        S32_MIN, S32_MAX);
    }
    return;
  case DataType_U32:
    if (op == '+') {
      *(U32 *)output = AddClampOverflow(*(const U32 *)arg1, *(const U32 *)arg2,
                                        U32_MIN, U32_MAX);
    }
    if (op == '-') {
      *(U32 *)output = SubClampOverflow(*(const U32 *)arg1, *(const U32 *)arg2,
                                        U32_MIN, U32_MAX);
    }
    return;
  case DataType_S64:
    if (op == '+') {
      *(S64 *)output = AddClampOverflow(*(const S64 *)arg1, *(const S64 *)arg2,
                                        S64_MIN, S64_MAX);
    }
    if (op == '-') {
      *(S64 *)output = SubClampOverflow(*(const S64 *)arg1, *(const S64 *)arg2,
                                        S64_MIN, S64_MAX);
    }
    return;
  case DataType_U64:
    if (op == '+') {
      *(U64 *)output = AddClampOverflow(*(const U64 *)arg1, *(const U64 *)arg2,
                                        U64_MIN, U64_MAX);
    }
    if (op == '-') {
      *(U64 *)output = SubClampOverflow(*(const U64 *)arg1, *(const U64 *)arg2,
                                        U64_MIN, U64_MAX);
    }
    return;
  case DataType_Float:
    if (op == '+') {
      *(float *)output = *(const float *)arg1 + *(const float *)arg2;
    }
    if (op == '-') {
      *(float *)output = *(const float *)arg1 - *(const float *)arg2;
    }
    return;
  case DataType_Double:
    if (op == '+') {
      *(double *)output = *(const double *)arg1 + *(const double *)arg2;
    }
    if (op == '-') {
      *(double *)output = *(const double *)arg1 - *(const double *)arg2;
    }
    return;
  case DataType_COUNT:
    break;
  }
  ASSERT(0);
}

// User can input math operators (e.g. +100) to edit a numerical values.
// NB: This is _not_ a full expression evaluator. We should probably add one and
// replace this dumb mess..
bool Gui::DataTypeApplyFromText(const char *buf, DataType data_type,
                                void *p_data, const char *format) {
  while (CharIsBlankA(*buf))
    buf++;
  if (!buf[0])
    return false;

  // Copy the value in an opaque buffer so we can compare at the end of the
  // function if it changed at all.
  const DataTypeInfo *type_info = DataTypeGetInfo(data_type);
  DataTypeTempStorage data_backup;
  memcpy(&data_backup, p_data, type_info->Size);

  // Sanitize format
  // - For float/double we have to ignore format with precision (e.g. "%.2f")
  // because sscanf doesn't take them in, so force them into %f and %lf
  // - In theory could treat empty format as using default, but this would only
  // cover rare/bizarre case of using InputScalar() + integer + format string
  // without %.
  char format_sanitized[32];
  if (data_type == DataType_Float || data_type == DataType_Double)
    format = type_info->ScanFmt;
  else
    format = ParseFormatSanitizeForScanning(format, format_sanitized,
                                            ARRAYSIZE(format_sanitized));

  // Small types need a 32-bit buffer to receive the result from scanf()
  int v32 = 0;
  if (sscanf(buf, format, type_info->Size >= 4 ? p_data : &v32) < 1)
    return false;
  if (type_info->Size < 4) {
    if (data_type == DataType_S8)
      *(S8 *)p_data = (S8)Clamp(v32, (int)S8_MIN, (int)S8_MAX);
    else if (data_type == DataType_U8)
      *(U8 *)p_data = (U8)Clamp(v32, (int)U8_MIN, (int)U8_MAX);
    else if (data_type == DataType_S16)
      *(S16 *)p_data = (S16)Clamp(v32, (int)S16_MIN, (int)S16_MAX);
    else if (data_type == DataType_U16)
      *(U16 *)p_data = (U16)Clamp(v32, (int)U16_MIN, (int)U16_MAX);
    else
      ASSERT(0);
  }

  return memcmp(&data_backup, p_data, type_info->Size) != 0;
}

template <typename T> static int DataTypeCompareT(const T *lhs, const T *rhs) {
  if (*lhs < *rhs)
    return -1;
  if (*lhs > *rhs)
    return +1;
  return 0;
}

int Gui::DataTypeCompare(DataType data_type, const void *arg_1,
                         const void *arg_2) {
  switch (data_type) {
  case DataType_S8:
    return DataTypeCompareT<S8>((const S8 *)arg_1, (const S8 *)arg_2);
  case DataType_U8:
    return DataTypeCompareT<U8>((const U8 *)arg_1, (const U8 *)arg_2);
  case DataType_S16:
    return DataTypeCompareT<S16>((const S16 *)arg_1, (const S16 *)arg_2);
  case DataType_U16:
    return DataTypeCompareT<U16>((const U16 *)arg_1, (const U16 *)arg_2);
  case DataType_S32:
    return DataTypeCompareT<S32>((const S32 *)arg_1, (const S32 *)arg_2);
  case DataType_U32:
    return DataTypeCompareT<U32>((const U32 *)arg_1, (const U32 *)arg_2);
  case DataType_S64:
    return DataTypeCompareT<S64>((const S64 *)arg_1, (const S64 *)arg_2);
  case DataType_U64:
    return DataTypeCompareT<U64>((const U64 *)arg_1, (const U64 *)arg_2);
  case DataType_Float:
    return DataTypeCompareT<float>((const float *)arg_1, (const float *)arg_2);
  case DataType_Double:
    return DataTypeCompareT<double>((const double *)arg_1,
                                    (const double *)arg_2);
  case DataType_COUNT:
    break;
  }
  ASSERT(0);
  return 0;
}

template <typename T>
static bool DataTypeClampT(T *v, const T *v_min, const T *v_max) {
  // Clamp, both sides are optional, return true if modified
  if (v_min && *v < *v_min) {
    *v = *v_min;
    return true;
  }
  if (v_max && *v > *v_max) {
    *v = *v_max;
    return true;
  }
  return false;
}

bool Gui::DataTypeClamp(DataType data_type, void *p_data, const void *p_min,
                        const void *p_max) {
  switch (data_type) {
  case DataType_S8:
    return DataTypeClampT<S8>((S8 *)p_data, (const S8 *)p_min,
                              (const S8 *)p_max);
  case DataType_U8:
    return DataTypeClampT<U8>((U8 *)p_data, (const U8 *)p_min,
                              (const U8 *)p_max);
  case DataType_S16:
    return DataTypeClampT<S16>((S16 *)p_data, (const S16 *)p_min,
                               (const S16 *)p_max);
  case DataType_U16:
    return DataTypeClampT<U16>((U16 *)p_data, (const U16 *)p_min,
                               (const U16 *)p_max);
  case DataType_S32:
    return DataTypeClampT<S32>((S32 *)p_data, (const S32 *)p_min,
                               (const S32 *)p_max);
  case DataType_U32:
    return DataTypeClampT<U32>((U32 *)p_data, (const U32 *)p_min,
                               (const U32 *)p_max);
  case DataType_S64:
    return DataTypeClampT<S64>((S64 *)p_data, (const S64 *)p_min,
                               (const S64 *)p_max);
  case DataType_U64:
    return DataTypeClampT<U64>((U64 *)p_data, (const U64 *)p_min,
                               (const U64 *)p_max);
  case DataType_Float:
    return DataTypeClampT<float>((float *)p_data, (const float *)p_min,
                                 (const float *)p_max);
  case DataType_Double:
    return DataTypeClampT<double>((double *)p_data, (const double *)p_min,
                                  (const double *)p_max);
  case DataType_COUNT:
    break;
  }
  ASSERT(0);
  return false;
}

static float GetMinimumStepAtDecimalPrecision(int decimal_precision) {
  static const float min_steps[10] = {
      1.0f,     0.1f,      0.01f,      0.001f,      0.0001f,
      0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f};
  if (decimal_precision < 0)
    return FLT_MIN;
  return (decimal_precision < ARRAYSIZE(min_steps))
             ? min_steps[decimal_precision]
             : Pow(10.0f, (float)-decimal_precision);
}

template <typename TYPE>
TYPE Gui::RoundScalarWithFormatT(const char *format, DataType data_type,
                                 TYPE v) {
  UNUSED(data_type);
  ASSERT(data_type == DataType_Float || data_type == DataType_Double);
  const char *fmt_start = ParseFormatFindStart(format);
  if (fmt_start[0] != '%' ||
      fmt_start[1] ==
          '%') // Don't apply if the value is not visible in the format string
    return v;

  // Sanitize format
  char fmt_sanitized[32];
  ParseFormatSanitizeForPrinting(fmt_start, fmt_sanitized,
                                 ARRAYSIZE(fmt_sanitized));
  fmt_start = fmt_sanitized;

  // Format value with our rounding, and read back
  char v_str[64];
  FormatString(v_str, ARRAYSIZE(v_str), fmt_start, v);
  const char *p = v_str;
  while (*p == ' ')
    p++;
  v = (TYPE)Atof(p);

  return v;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
//-------------------------------------------------------------------------
// - DragBehaviorT<>() [Internal]
// - DragBehavior() [Internal]
// - DragScalar()
// - DragScalarN()
// - DragFloat()
// - DragFloat2()
// - DragFloat3()
// - DragFloat4()
// - DragFloatRange2()
// - DragInt()
// - DragInt2()
// - DragInt3()
// - DragInt4()
// - DragIntRange2()
//-------------------------------------------------------------------------

// This is called by DragBehavior() when the widget is active (held by mouse or
// being manipulated with Nav controls)
template <typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool Gui::DragBehaviorT(DataType data_type, TYPE *v, float v_speed,
                        const TYPE v_min, const TYPE v_max, const char *format,
                        SliderFlags flags) {
  Context &g = *GGui;
  const Axis axis = (flags & SliderFlags_Vertical) ? Axis_Y : Axis_X;
  const bool is_clamped = (v_min < v_max);
  const bool is_logarithmic = (flags & SliderFlags_Logarithmic) != 0;
  const bool is_floating_point =
      (data_type == DataType_Float) || (data_type == DataType_Double);

  // Default tweak speed
  if (v_speed == 0.0f && is_clamped && (v_max - v_min < FLT_MAX))
    v_speed = (float)((v_max - v_min) * g.DragSpeedDefaultRatio);

  // Inputs accumulates into g.DragCurrentAccum, which is flushed into the
  // current value as soon as it makes a difference with our precision settings
  float adjust_delta = 0.0f;
  if (g.ActiveIdSource == InputSource_Mouse && IsMousePosValid() &&
      IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold *
                                      DRAG_MOUSE_THRESHOLD_FACTOR)) {
    adjust_delta = g.IO.MouseDelta[axis];
    if (g.IO.KeyAlt)
      adjust_delta *= 1.0f / 100.0f;
    if (g.IO.KeyShift)
      adjust_delta *= 10.0f;
  } else if (g.ActiveIdSource == InputSource_Keyboard ||
             g.ActiveIdSource == InputSource_Gamepad) {
    const int decimal_precision =
        is_floating_point ? ParseFormatPrecision(format, 3) : 0;
    const bool tweak_slow = IsKeyDown((g.NavInputSource == InputSource_Gamepad)
                                          ? Key_NavGamepadTweakSlow
                                          : Key_NavKeyboardTweakSlow);
    const bool tweak_fast = IsKeyDown((g.NavInputSource == InputSource_Gamepad)
                                          ? Key_NavGamepadTweakFast
                                          : Key_NavKeyboardTweakFast);
    const float tweak_factor = tweak_slow   ? 1.0f / 1.0f
                               : tweak_fast ? 10.0f
                                            : 1.0f;
    adjust_delta = GetNavTweakPressedAmount(axis) * tweak_factor;
    v_speed = Max(v_speed, GetMinimumStepAtDecimalPrecision(decimal_precision));
  }
  adjust_delta *= v_speed;

  // For vertical drag we currently assume that Up=higher value (like we do with
  // vertical sliders). This may become a parameter.
  if (axis == Axis_Y)
    adjust_delta = -adjust_delta;

  // For logarithmic use our range is effectively 0..1 so scale the delta into
  // that range
  if (is_logarithmic && (v_max - v_min < FLT_MAX) &&
      ((v_max - v_min) > 0.000001f)) // Epsilon to avoid /0
    adjust_delta /= (float)(v_max - v_min);

  // Clear current value on activation
  // Avoid altering values and clamping when we are _already_ past the limits
  // and heading in the same direction, so e.g. if range is 0..255, current
  // value is 300 and we are pushing to the right side, keep the 300.
  bool is_just_activated = g.ActiveIdIsJustActivated;
  bool is_already_past_limits_and_pushing_outward =
      is_clamped && ((*v >= v_max && adjust_delta > 0.0f) ||
                     (*v <= v_min && adjust_delta < 0.0f));
  if (is_just_activated || is_already_past_limits_and_pushing_outward) {
    g.DragCurrentAccum = 0.0f;
    g.DragCurrentAccumDirty = false;
  } else if (adjust_delta != 0.0f) {
    g.DragCurrentAccum += adjust_delta;
    g.DragCurrentAccumDirty = true;
  }

  if (!g.DragCurrentAccumDirty)
    return false;

  TYPE v_cur = *v;
  FLOATTYPE v_old_ref_for_accum_remainder = (FLOATTYPE)0.0f;

  float logarithmic_zero_epsilon =
      0.0f; // Only valid when is_logarithmic is true
  const float zero_deadzone_halfsize =
      0.0f; // Drag widgets have no deadzone (as it doesn't make sense)
  if (is_logarithmic) {
    // When using logarithmic sliders, we need to clamp to avoid hitting zero,
    // but our choice of clamp value greatly affects slider precision. We
    // attempt to use the specified precision to estimate a good lower bound.
    const int decimal_precision =
        is_floating_point ? ParseFormatPrecision(format, 3) : 1;
    logarithmic_zero_epsilon = Pow(0.1f, (float)decimal_precision);

    // Convert to parametric space, apply delta, convert back
    float v_old_parametric = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
        data_type, v_cur, v_min, v_max, is_logarithmic,
        logarithmic_zero_epsilon, zero_deadzone_halfsize);
    float v_new_parametric = v_old_parametric + g.DragCurrentAccum;
    v_cur = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(
        data_type, v_new_parametric, v_min, v_max, is_logarithmic,
        logarithmic_zero_epsilon, zero_deadzone_halfsize);
    v_old_ref_for_accum_remainder = v_old_parametric;
  } else {
    v_cur += (SIGNEDTYPE)g.DragCurrentAccum;
  }

  // Round to user desired precision based on format string
  if (is_floating_point && !(flags & SliderFlags_NoRoundToFormat))
    v_cur = RoundScalarWithFormatT<TYPE>(format, data_type, v_cur);

  // Preserve remainder after rounding has been applied. This also allow slow
  // tweaking of values.
  g.DragCurrentAccumDirty = false;
  if (is_logarithmic) {
    // Convert to parametric space, apply delta, convert back
    float v_new_parametric = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
        data_type, v_cur, v_min, v_max, is_logarithmic,
        logarithmic_zero_epsilon, zero_deadzone_halfsize);
    g.DragCurrentAccum -=
        (float)(v_new_parametric - v_old_ref_for_accum_remainder);
  } else {
    g.DragCurrentAccum -= (float)((SIGNEDTYPE)v_cur - (SIGNEDTYPE)*v);
  }

  // Lose zero sign for float/double
  if (v_cur == (TYPE)-0)
    v_cur = (TYPE)0;

  // Clamp values (+ handle overflow/wrap-around for integer types)
  if (*v != v_cur && is_clamped) {
    if (v_cur < v_min ||
        (v_cur > *v && adjust_delta < 0.0f && !is_floating_point))
      v_cur = v_min;
    if (v_cur > v_max ||
        (v_cur < *v && adjust_delta > 0.0f && !is_floating_point))
      v_cur = v_max;
  }

  // Apply result
  if (*v == v_cur)
    return false;
  *v = v_cur;
  return true;
}

bool Gui::DragBehavior(ID id, DataType data_type, void *p_v, float v_speed,
                       const void *p_min, const void *p_max, const char *format,
                       SliderFlags flags) {
  // Read gui.cpp "API BREAKING CHANGES" section for 1.78 if you hit this
  // assert.
  ASSERT((flags == 1 || (flags & SliderFlags_InvalidMask_) == 0) &&
         "Invalid SliderFlags flags! Has the 'float power' argument "
         "been mistakenly cast to flags? Call function with "
         "SliderFlags_Logarithmic flags instead.");

  Context &g = *GGui;
  if (g.ActiveId == id) {
    // Those are the things we can do easily outside the DragBehaviorT<>
    // template, saves code generation.
    if (g.ActiveIdSource == InputSource_Mouse && !g.IO.MouseDown[0])
      ClearActiveID();
    else if ((g.ActiveIdSource == InputSource_Keyboard ||
              g.ActiveIdSource == InputSource_Gamepad) &&
             g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
      ClearActiveID();
  }
  if (g.ActiveId != id)
    return false;
  if ((g.LastItemData.InFlags & ItemFlags_ReadOnly) ||
      (flags & SliderFlags_ReadOnly))
    return false;

  switch (data_type) {
  case DataType_S8: {
    S32 v32 = (S32) * (S8 *)p_v;
    bool r = DragBehaviorT<S32, S32, float>(
        DataType_S32, &v32, v_speed, p_min ? *(const S8 *)p_min : S8_MIN,
        p_max ? *(const S8 *)p_max : S8_MAX, format, flags);
    if (r)
      *(S8 *)p_v = (S8)v32;
    return r;
  }
  case DataType_U8: {
    U32 v32 = (U32) * (U8 *)p_v;
    bool r = DragBehaviorT<U32, S32, float>(
        DataType_U32, &v32, v_speed, p_min ? *(const U8 *)p_min : U8_MIN,
        p_max ? *(const U8 *)p_max : U8_MAX, format, flags);
    if (r)
      *(U8 *)p_v = (U8)v32;
    return r;
  }
  case DataType_S16: {
    S32 v32 = (S32) * (S16 *)p_v;
    bool r = DragBehaviorT<S32, S32, float>(
        DataType_S32, &v32, v_speed, p_min ? *(const S16 *)p_min : S16_MIN,
        p_max ? *(const S16 *)p_max : S16_MAX, format, flags);
    if (r)
      *(S16 *)p_v = (S16)v32;
    return r;
  }
  case DataType_U16: {
    U32 v32 = (U32) * (U16 *)p_v;
    bool r = DragBehaviorT<U32, S32, float>(
        DataType_U32, &v32, v_speed, p_min ? *(const U16 *)p_min : U16_MIN,
        p_max ? *(const U16 *)p_max : U16_MAX, format, flags);
    if (r)
      *(U16 *)p_v = (U16)v32;
    return r;
  }
  case DataType_S32:
    return DragBehaviorT<S32, S32, float>(
        data_type, (S32 *)p_v, v_speed, p_min ? *(const S32 *)p_min : S32_MIN,
        p_max ? *(const S32 *)p_max : S32_MAX, format, flags);
  case DataType_U32:
    return DragBehaviorT<U32, S32, float>(
        data_type, (U32 *)p_v, v_speed, p_min ? *(const U32 *)p_min : U32_MIN,
        p_max ? *(const U32 *)p_max : U32_MAX, format, flags);
  case DataType_S64:
    return DragBehaviorT<S64, S64, double>(
        data_type, (S64 *)p_v, v_speed, p_min ? *(const S64 *)p_min : S64_MIN,
        p_max ? *(const S64 *)p_max : S64_MAX, format, flags);
  case DataType_U64:
    return DragBehaviorT<U64, S64, double>(
        data_type, (U64 *)p_v, v_speed, p_min ? *(const U64 *)p_min : U64_MIN,
        p_max ? *(const U64 *)p_max : U64_MAX, format, flags);
  case DataType_Float:
    return DragBehaviorT<float, float, float>(
        data_type, (float *)p_v, v_speed,
        p_min ? *(const float *)p_min : -FLT_MAX,
        p_max ? *(const float *)p_max : FLT_MAX, format, flags);
  case DataType_Double:
    return DragBehaviorT<double, double, double>(
        data_type, (double *)p_v, v_speed,
        p_min ? *(const double *)p_min : -DBL_MAX,
        p_max ? *(const double *)p_max : DBL_MAX, format, flags);
  case DataType_COUNT:
    break;
  }
  ASSERT(0);
  return false;
}

// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the
// data. For a Drag widget, p_min and p_max are optional. Read code of e.g.
// DragFloat(), DragInt() etc. or examples in 'Demo->Widgets->Data Types' to
// understand how to use this function directly.
bool Gui::DragScalar(const char *label, DataType data_type, void *p_data,
                     float v_speed, const void *p_min, const void *p_max,
                     const char *format, SliderFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const ID id = window->GetID(label);
  const float w = CalcItemWidth();

  const Vec2 label_size = CalcTextSize(label, NULL, true);
  const Rect frame_bb(window->DC.CursorPos,
                      window->DC.CursorPos +
                          Vec2(w, label_size.y + style.FramePadding.y * 2.0f));
  const Rect total_bb(frame_bb.Min,
                      frame_bb.Max +
                          Vec2(label_size.x > 0.0f
                                   ? style.ItemInnerSpacing.x + label_size.x
                                   : 0.0f,
                               0.0f));

  const bool temp_input_allowed = (flags & SliderFlags_NoInput) == 0;
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id, &frame_bb,
               temp_input_allowed ? ItemFlags_Inputable : 0))
    return false;

  // Default format string when passing NULL
  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;

  const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
  bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
  if (!temp_input_is_active) {
    // Tabbing or CTRL-clicking on Drag turns it into an InputText
    const bool clicked = hovered && IsMouseClicked(0, id);
    const bool double_clicked = (hovered && g.IO.MouseClickedCount[0] == 2 &&
                                 TestKeyOwner(Key_MouseLeft, id));
    const bool make_active =
        (clicked || double_clicked || g.NavActivateId == id);
    if (make_active && (clicked || double_clicked))
      SetKeyOwner(Key_MouseLeft, id);
    if (make_active && temp_input_allowed)
      if ((clicked && g.IO.KeyCtrl) || double_clicked ||
          (g.NavActivateId == id &&
           (g.NavActivateFlags & ActivateFlags_PreferInput)))
        temp_input_is_active = true;

    // (Optional) simple click (without moving) turns Drag into an InputText
    if (g.IO.ConfigDragClickToInputText && temp_input_allowed &&
        !temp_input_is_active)
      if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] &&
          !IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold *
                                           DRAG_MOUSE_THRESHOLD_FACTOR)) {
        g.NavActivateId = id;
        g.NavActivateFlags = ActivateFlags_PreferInput;
        temp_input_is_active = true;
      }

    if (make_active && !temp_input_is_active) {
      SetActiveID(id, window);
      SetFocusID(id, window);
      FocusWindow(window);
      g.ActiveIdUsingNavDirMask = (1 << Dir_Left) | (1 << Dir_Right);
    }
  }

  if (temp_input_is_active) {
    // Only clamp CTRL+Click input when SliderFlags_AlwaysClamp is set
    const bool is_clamp_input = (flags & SliderFlags_AlwaysClamp) != 0 &&
                                (p_min == NULL || p_max == NULL ||
                                 DataTypeCompare(data_type, p_min, p_max) < 0);
    return TempInputScalar(frame_bb, id, label, data_type, p_data, format,
                           is_clamp_input ? p_min : NULL,
                           is_clamp_input ? p_max : NULL);
  }

  // Draw frame
  const U32 frame_col = GetColorU32(g.ActiveId == id ? Col_FrameBgActive
                                    : hovered        ? Col_FrameBgHovered
                                                     : Col_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

  // Drag behavior
  const bool value_changed =
      DragBehavior(id, data_type, p_data, v_speed, p_min, p_max, format, flags);
  if (value_changed)
    MarkItemEdited(id);

  // Display value using user-provided display format so user can add
  // prefix/suffix/decorations to the value.
  char value_buf[64];
  const char *value_buf_end =
      value_buf + DataTypeFormatString(value_buf, ARRAYSIZE(value_buf),
                                       data_type, p_data, format);
  if (g.LogEnabled)
    LogSetNextTextDecoration("{", "}");
  RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL,
                    Vec2(0.5f, 0.5f));

  if (label_size.x > 0.0f)
    RenderText(Vec2(frame_bb.Max.x + style.ItemInnerSpacing.x,
                    frame_bb.Min.y + style.FramePadding.y),
               label);

  TEST_ENGINE_ITEM_INFO(
      id, label,
      g.LastItemData.StatusFlags |
          (temp_input_allowed ? ItemStatusFlags_Inputable : 0));
  return value_changed;
}

bool Gui::DragScalarN(const char *label, DataType data_type, void *p_data,
                      int components, float v_speed, const void *p_min,
                      const void *p_max, const char *format,
                      SliderFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  bool value_changed = false;
  BeginGroup();
  PushID(label);
  PushMultiItemsWidths(components, CalcItemWidth());
  size_t type_size = GDataTypeInfo[data_type].Size;
  for (int i = 0; i < components; i++) {
    PushID(i);
    if (i > 0)
      SameLine(0, g.Style.ItemInnerSpacing.x);
    value_changed |=
        DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);
    PopID();
    PopItemWidth();
    p_data = (void *)((char *)p_data + type_size);
  }
  PopID();

  const char *label_end = FindRenderedTextEnd(label);
  if (label != label_end) {
    SameLine(0, g.Style.ItemInnerSpacing.x);
    TextEx(label, label_end);
  }

  EndGroup();
  return value_changed;
}

bool Gui::DragFloat(const char *label, float *v, float v_speed, float v_min,
                    float v_max, const char *format, SliderFlags flags) {
  return DragScalar(label, DataType_Float, v, v_speed, &v_min, &v_max, format,
                    flags);
}

bool Gui::DragFloat2(const char *label, float v[2], float v_speed, float v_min,
                     float v_max, const char *format, SliderFlags flags) {
  return DragScalarN(label, DataType_Float, v, 2, v_speed, &v_min, &v_max,
                     format, flags);
}

bool Gui::DragFloat3(const char *label, float v[3], float v_speed, float v_min,
                     float v_max, const char *format, SliderFlags flags) {
  return DragScalarN(label, DataType_Float, v, 3, v_speed, &v_min, &v_max,
                     format, flags);
}

bool Gui::DragFloat4(const char *label, float v[4], float v_speed, float v_min,
                     float v_max, const char *format, SliderFlags flags) {
  return DragScalarN(label, DataType_Float, v, 4, v_speed, &v_min, &v_max,
                     format, flags);
}

// NB: You likely want to specify the SliderFlags_AlwaysClamp when using
// this.
bool Gui::DragFloatRange2(const char *label, float *v_current_min,
                          float *v_current_max, float v_speed, float v_min,
                          float v_max, const char *format,
                          const char *format_max, SliderFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  PushID(label);
  BeginGroup();
  PushMultiItemsWidths(2, CalcItemWidth());

  float min_min = (v_min >= v_max) ? -FLT_MAX : v_min;
  float min_max =
      (v_min >= v_max) ? *v_current_max : Min(v_max, *v_current_max);
  SliderFlags min_flags =
      flags | ((min_min == min_max) ? SliderFlags_ReadOnly : 0);
  bool value_changed =
      DragScalar("##min", DataType_Float, v_current_min, v_speed, &min_min,
                 &min_max, format, min_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing.x);

  float max_min =
      (v_min >= v_max) ? *v_current_min : Max(v_min, *v_current_min);
  float max_max = (v_min >= v_max) ? FLT_MAX : v_max;
  SliderFlags max_flags =
      flags | ((max_min == max_max) ? SliderFlags_ReadOnly : 0);
  value_changed |=
      DragScalar("##max", DataType_Float, v_current_max, v_speed, &max_min,
                 &max_max, format_max ? format_max : format, max_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing.x);

  TextEx(label, FindRenderedTextEnd(label));
  EndGroup();
  PopID();

  return value_changed;
}

// NB: v_speed is float to allow adjusting the drag speed with more precision
bool Gui::DragInt(const char *label, int *v, float v_speed, int v_min,
                  int v_max, const char *format, SliderFlags flags) {
  return DragScalar(label, DataType_S32, v, v_speed, &v_min, &v_max, format,
                    flags);
}

bool Gui::DragInt2(const char *label, int v[2], float v_speed, int v_min,
                   int v_max, const char *format, SliderFlags flags) {
  return DragScalarN(label, DataType_S32, v, 2, v_speed, &v_min, &v_max, format,
                     flags);
}

bool Gui::DragInt3(const char *label, int v[3], float v_speed, int v_min,
                   int v_max, const char *format, SliderFlags flags) {
  return DragScalarN(label, DataType_S32, v, 3, v_speed, &v_min, &v_max, format,
                     flags);
}

bool Gui::DragInt4(const char *label, int v[4], float v_speed, int v_min,
                   int v_max, const char *format, SliderFlags flags) {
  return DragScalarN(label, DataType_S32, v, 4, v_speed, &v_min, &v_max, format,
                     flags);
}

// NB: You likely want to specify the SliderFlags_AlwaysClamp when using
// this.
bool Gui::DragIntRange2(const char *label, int *v_current_min,
                        int *v_current_max, float v_speed, int v_min, int v_max,
                        const char *format, const char *format_max,
                        SliderFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  PushID(label);
  BeginGroup();
  PushMultiItemsWidths(2, CalcItemWidth());

  int min_min = (v_min >= v_max) ? INT_MIN : v_min;
  int min_max = (v_min >= v_max) ? *v_current_max : Min(v_max, *v_current_max);
  SliderFlags min_flags =
      flags | ((min_min == min_max) ? SliderFlags_ReadOnly : 0);
  bool value_changed = DragInt("##min", v_current_min, v_speed, min_min,
                               min_max, format, min_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing.x);

  int max_min = (v_min >= v_max) ? *v_current_min : Max(v_min, *v_current_min);
  int max_max = (v_min >= v_max) ? INT_MAX : v_max;
  SliderFlags max_flags =
      flags | ((max_min == max_max) ? SliderFlags_ReadOnly : 0);
  value_changed |= DragInt("##max", v_current_max, v_speed, max_min, max_max,
                           format_max ? format_max : format, max_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing.x);

  TextEx(label, FindRenderedTextEnd(label));
  EndGroup();
  PopID();

  return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
//-------------------------------------------------------------------------
// - ScaleRatioFromValueT<> [Internal]
// - ScaleValueFromRatioT<> [Internal]
// - SliderBehaviorT<>() [Internal]
// - SliderBehavior() [Internal]
// - SliderScalar()
// - SliderScalarN()
// - SliderFloat()
// - SliderFloat2()
// - SliderFloat3()
// - SliderFloat4()
// - SliderAngle()
// - SliderInt()
// - SliderInt2()
// - SliderInt3()
// - SliderInt4()
// - VSliderScalar()
// - VSliderFloat()
// - VSliderInt()
//-------------------------------------------------------------------------

// Convert a value v in the output space of a slider into a parametric position
// on the slider itself (the logical opposite of ScaleValueFromRatioT)
template <typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
float Gui::ScaleRatioFromValueT(DataType data_type, TYPE v, TYPE v_min,
                                TYPE v_max, bool is_logarithmic,
                                float logarithmic_zero_epsilon,
                                float zero_deadzone_halfsize) {
  if (v_min == v_max)
    return 0.0f;
  UNUSED(data_type);

  const TYPE v_clamped =
      (v_min < v_max) ? Clamp(v, v_min, v_max) : Clamp(v, v_max, v_min);
  if (is_logarithmic) {
    bool flipped = v_max < v_min;

    if (flipped) // Handle the case where the range is backwards
      Swap(v_min, v_max);

    // Fudge min/max to avoid getting close to log(0)
    FLOATTYPE v_min_fudged = (Abs((FLOATTYPE)v_min) < logarithmic_zero_epsilon)
                                 ? ((v_min < 0.0f) ? -logarithmic_zero_epsilon
                                                   : logarithmic_zero_epsilon)
                                 : (FLOATTYPE)v_min;
    FLOATTYPE v_max_fudged = (Abs((FLOATTYPE)v_max) < logarithmic_zero_epsilon)
                                 ? ((v_max < 0.0f) ? -logarithmic_zero_epsilon
                                                   : logarithmic_zero_epsilon)
                                 : (FLOATTYPE)v_max;

    // Awkward special cases - we need ranges of the form (-100 .. 0) to convert
    // to (-100 .. -epsilon), not (-100 .. epsilon)
    if ((v_min == 0.0f) && (v_max < 0.0f))
      v_min_fudged = -logarithmic_zero_epsilon;
    else if ((v_max == 0.0f) && (v_min < 0.0f))
      v_max_fudged = -logarithmic_zero_epsilon;

    float result;
    if (v_clamped <= v_min_fudged)
      result =
          0.0f; // Workaround for values that are in-range but below our fudge
    else if (v_clamped >= v_max_fudged)
      result =
          1.0f; // Workaround for values that are in-range but above our fudge
    else if ((v_min * v_max) <
             0.0f) // Range crosses zero, so split into two portions
    {
      float zero_point_center =
          (-(float)v_min) /
          ((float)v_max -
           (float)v_min); // The zero point in parametric space.  There's an
                          // argument we should take the logarithmic nature into
                          // account when calculating this, but for now this
                          // should do (and the most common case of a
                          // symmetrical range works fine)
      float zero_point_snap_L = zero_point_center - zero_deadzone_halfsize;
      float zero_point_snap_R = zero_point_center + zero_deadzone_halfsize;
      if (v == 0.0f)
        result = zero_point_center; // Special case for exactly zero
      else if (v < 0.0f)
        result =
            (1.0f -
             (float)(Log(-(FLOATTYPE)v_clamped / logarithmic_zero_epsilon) /
                     Log(-v_min_fudged / logarithmic_zero_epsilon))) *
            zero_point_snap_L;
      else
        result = zero_point_snap_R +
                 ((float)(Log((FLOATTYPE)v_clamped / logarithmic_zero_epsilon) /
                          Log(v_max_fudged / logarithmic_zero_epsilon)) *
                  (1.0f - zero_point_snap_R));
    } else if ((v_min < 0.0f) || (v_max < 0.0f)) // Entirely negative slider
      result = 1.0f - (float)(Log(-(FLOATTYPE)v_clamped / -v_max_fudged) /
                              Log(-v_min_fudged / -v_max_fudged));
    else
      result = (float)(Log((FLOATTYPE)v_clamped / v_min_fudged) /
                       Log(v_max_fudged / v_min_fudged));

    return flipped ? (1.0f - result) : result;
  } else {
    // Linear slider
    return (float)((FLOATTYPE)(SIGNEDTYPE)(v_clamped - v_min) /
                   (FLOATTYPE)(SIGNEDTYPE)(v_max - v_min));
  }
}

// Convert a parametric position on a slider into a value v in the output space
// (the logical opposite of ScaleRatioFromValueT)
template <typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
TYPE Gui::ScaleValueFromRatioT(DataType data_type, float t, TYPE v_min,
                               TYPE v_max, bool is_logarithmic,
                               float logarithmic_zero_epsilon,
                               float zero_deadzone_halfsize) {
  // We special-case the extents because otherwise our logarithmic fudging can
  // lead to "mathematically correct" but non-intuitive behaviors like a
  // fully-left slider not actually reaching the minimum value. Also generally
  // simpler.
  if (t <= 0.0f || v_min == v_max)
    return v_min;
  if (t >= 1.0f)
    return v_max;

  TYPE result = (TYPE)0;
  if (is_logarithmic) {
    // Fudge min/max to avoid getting silly results close to zero
    FLOATTYPE v_min_fudged = (Abs((FLOATTYPE)v_min) < logarithmic_zero_epsilon)
                                 ? ((v_min < 0.0f) ? -logarithmic_zero_epsilon
                                                   : logarithmic_zero_epsilon)
                                 : (FLOATTYPE)v_min;
    FLOATTYPE v_max_fudged = (Abs((FLOATTYPE)v_max) < logarithmic_zero_epsilon)
                                 ? ((v_max < 0.0f) ? -logarithmic_zero_epsilon
                                                   : logarithmic_zero_epsilon)
                                 : (FLOATTYPE)v_max;

    const bool flipped = v_max < v_min; // Check if range is "backwards"
    if (flipped)
      Swap(v_min_fudged, v_max_fudged);

    // Awkward special case - we need ranges of the form (-100 .. 0) to convert
    // to (-100 .. -epsilon), not (-100 .. epsilon)
    if ((v_max == 0.0f) && (v_min < 0.0f))
      v_max_fudged = -logarithmic_zero_epsilon;

    float t_with_flip =
        flipped ? (1.0f - t) : t; // t, but flipped if necessary to account for
                                  // us flipping the range

    if ((v_min * v_max) <
        0.0f) // Range crosses zero, so we have to do this in two parts
    {
      float zero_point_center =
          (-(float)Min(v_min, v_max)) /
          Abs((float)v_max -
              (float)v_min); // The zero point in parametric space
      float zero_point_snap_L = zero_point_center - zero_deadzone_halfsize;
      float zero_point_snap_R = zero_point_center + zero_deadzone_halfsize;
      if (t_with_flip >= zero_point_snap_L && t_with_flip <= zero_point_snap_R)
        result = (TYPE)0.0f; // Special case to make getting exactly zero
                             // possible (the epsilon prevents it otherwise)
      else if (t_with_flip < zero_point_center)
        result = (TYPE) -
                 (logarithmic_zero_epsilon *
                  Pow(-v_min_fudged / logarithmic_zero_epsilon,
                      (FLOATTYPE)(1.0f - (t_with_flip / zero_point_snap_L))));
      else
        result = (TYPE)(logarithmic_zero_epsilon *
                        Pow(v_max_fudged / logarithmic_zero_epsilon,
                            (FLOATTYPE)((t_with_flip - zero_point_snap_R) /
                                        (1.0f - zero_point_snap_R))));
    } else if ((v_min < 0.0f) || (v_max < 0.0f)) // Entirely negative slider
      result = (TYPE) - (-v_max_fudged * Pow(-v_min_fudged / -v_max_fudged,
                                             (FLOATTYPE)(1.0f - t_with_flip)));
    else
      result = (TYPE)(v_min_fudged *
                      Pow(v_max_fudged / v_min_fudged, (FLOATTYPE)t_with_flip));
  } else {
    // Linear slider
    const bool is_floating_point =
        (data_type == DataType_Float) || (data_type == DataType_Double);
    if (is_floating_point) {
      result = Lerp(v_min, v_max, t);
    } else if (t < 1.0) {
      // - For integer values we want the clicking position to match the grab
      // box so we round above
      //   This code is carefully tuned to work with large values (e.g. high
      //   ranges of U64) while preserving this property..
      // - Not doing a *1.0 multiply at the end of a range as it tends to be
      // lossy. While absolute aiming at a large s64/u64
      //   range is going to be imprecise anyway, with this check we at least
      //   make the edge values matches expected limits.
      FLOATTYPE v_new_off_f = (SIGNEDTYPE)(v_max - v_min) * t;
      result = (TYPE)((SIGNEDTYPE)v_min +
                      (SIGNEDTYPE)(v_new_off_f +
                                   (FLOATTYPE)(v_min > v_max ? -0.5 : 0.5)));
    }
  }

  return result;
}

// FIXME: Try to move more of the code into shared SliderBehavior()
template <typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool Gui::SliderBehaviorT(const Rect &bb, ID id, DataType data_type, TYPE *v,
                          const TYPE v_min, const TYPE v_max,
                          const char *format, SliderFlags flags,
                          Rect *out_grab_bb) {
  Context &g = *GGui;
  const Style &style = g.Style;

  const Axis axis = (flags & SliderFlags_Vertical) ? Axis_Y : Axis_X;
  const bool is_logarithmic = (flags & SliderFlags_Logarithmic) != 0;
  const bool is_floating_point =
      (data_type == DataType_Float) || (data_type == DataType_Double);
  const float v_range_f =
      (float)(v_min < v_max ? v_max - v_min
                            : v_min - v_max); // We don't need high precision
                                              // for what we do with it.

  // Calculate bounds
  const float grab_padding = 2.0f; // FIXME: Should be part of style.
  const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
  float grab_sz = style.GrabMinSize;
  if (!is_floating_point &&
      v_range_f >= 0.0f) // v_range_f < 0 may happen on integer overflows
    grab_sz = Max(slider_sz / (v_range_f + 1),
                  style.GrabMinSize); // For integer sliders: if possible have
                                      // the grab size represent 1 unit
  grab_sz = Min(grab_sz, slider_sz);
  const float slider_usable_sz = slider_sz - grab_sz;
  const float slider_usable_pos_min =
      bb.Min[axis] + grab_padding + grab_sz * 0.5f;
  const float slider_usable_pos_max =
      bb.Max[axis] - grab_padding - grab_sz * 0.5f;

  float logarithmic_zero_epsilon =
      0.0f;                            // Only valid when is_logarithmic is true
  float zero_deadzone_halfsize = 0.0f; // Only valid when is_logarithmic is true
  if (is_logarithmic) {
    // When using logarithmic sliders, we need to clamp to avoid hitting zero,
    // but our choice of clamp value greatly affects slider precision. We
    // attempt to use the specified precision to estimate a good lower bound.
    const int decimal_precision =
        is_floating_point ? ParseFormatPrecision(format, 3) : 1;
    logarithmic_zero_epsilon = Pow(0.1f, (float)decimal_precision);
    zero_deadzone_halfsize =
        (style.LogSliderDeadzone * 0.5f) / Max(slider_usable_sz, 1.0f);
  }

  // Process interacting with the slider
  bool value_changed = false;
  if (g.ActiveId == id) {
    bool set_new_value = false;
    float clicked_t = 0.0f;
    if (g.ActiveIdSource == InputSource_Mouse) {
      if (!g.IO.MouseDown[0]) {
        ClearActiveID();
      } else {
        const float mouse_abs_pos = g.IO.MousePos[axis];
        if (g.ActiveIdIsJustActivated) {
          float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
              data_type, *v, v_min, v_max, is_logarithmic,
              logarithmic_zero_epsilon, zero_deadzone_halfsize);
          if (axis == Axis_Y)
            grab_t = 1.0f - grab_t;
          const float grab_pos =
              Lerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
          const bool clicked_around_grab =
              (mouse_abs_pos >= grab_pos - grab_sz * 0.5f - 1.0f) &&
              (mouse_abs_pos <= grab_pos + grab_sz * 0.5f +
                                    1.0f); // No harm being extra generous here.
          g.SliderGrabClickOffset = (clicked_around_grab && is_floating_point)
                                        ? mouse_abs_pos - grab_pos
                                        : 0.0f;
        }
        if (slider_usable_sz > 0.0f)
          clicked_t = Saturate((mouse_abs_pos - g.SliderGrabClickOffset -
                                slider_usable_pos_min) /
                               slider_usable_sz);
        if (axis == Axis_Y)
          clicked_t = 1.0f - clicked_t;
        set_new_value = true;
      }
    } else if (g.ActiveIdSource == InputSource_Keyboard ||
               g.ActiveIdSource == InputSource_Gamepad) {
      if (g.ActiveIdIsJustActivated) {
        g.SliderCurrentAccum =
            0.0f; // Reset any stored nav delta upon activation
        g.SliderCurrentAccumDirty = false;
      }

      float input_delta = (axis == Axis_X) ? GetNavTweakPressedAmount(axis)
                                           : -GetNavTweakPressedAmount(axis);
      if (input_delta != 0.0f) {
        const bool tweak_slow =
            IsKeyDown((g.NavInputSource == InputSource_Gamepad)
                          ? Key_NavGamepadTweakSlow
                          : Key_NavKeyboardTweakSlow);
        const bool tweak_fast =
            IsKeyDown((g.NavInputSource == InputSource_Gamepad)
                          ? Key_NavGamepadTweakFast
                          : Key_NavKeyboardTweakFast);
        const int decimal_precision =
            is_floating_point ? ParseFormatPrecision(format, 3) : 0;
        if (decimal_precision > 0) {
          input_delta /=
              100.0f; // Gamepad/keyboard tweak speeds in % of slider bounds
          if (tweak_slow)
            input_delta /= 10.0f;
        } else {
          if ((v_range_f >= -100.0f && v_range_f <= 100.0f &&
               v_range_f != 0.0f) ||
              tweak_slow)
            input_delta =
                ((input_delta < 0.0f) ? -1.0f : +1.0f) /
                v_range_f; // Gamepad/keyboard tweak speeds in integer steps
          else
            input_delta /= 100.0f;
        }
        if (tweak_fast)
          input_delta *= 10.0f;

        g.SliderCurrentAccum += input_delta;
        g.SliderCurrentAccumDirty = true;
      }

      float delta = g.SliderCurrentAccum;
      if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated) {
        ClearActiveID();
      } else if (g.SliderCurrentAccumDirty) {
        clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
            data_type, *v, v_min, v_max, is_logarithmic,
            logarithmic_zero_epsilon, zero_deadzone_halfsize);

        if ((clicked_t >= 1.0f && delta > 0.0f) ||
            (clicked_t <= 0.0f &&
             delta < 0.0f)) // This is to avoid applying the saturation when
                            // already past the limits
        {
          set_new_value = false;
          g.SliderCurrentAccum = 0.0f; // If pushing up against the limits,
                                       // don't continue to accumulate
        } else {
          set_new_value = true;
          float old_clicked_t = clicked_t;
          clicked_t = Saturate(clicked_t + delta);

          // Calculate what our "new" clicked_t will be, and thus how far we
          // actually moved the slider, and subtract this from the accumulator
          TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(
              data_type, clicked_t, v_min, v_max, is_logarithmic,
              logarithmic_zero_epsilon, zero_deadzone_halfsize);
          if (is_floating_point && !(flags & SliderFlags_NoRoundToFormat))
            v_new = RoundScalarWithFormatT<TYPE>(format, data_type, v_new);
          float new_clicked_t =
              ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
                  data_type, v_new, v_min, v_max, is_logarithmic,
                  logarithmic_zero_epsilon, zero_deadzone_halfsize);

          if (delta > 0)
            g.SliderCurrentAccum -= Min(new_clicked_t - old_clicked_t, delta);
          else
            g.SliderCurrentAccum -= Max(new_clicked_t - old_clicked_t, delta);
        }

        g.SliderCurrentAccumDirty = false;
      }
    }

    if (set_new_value)
      if ((g.LastItemData.InFlags & ItemFlags_ReadOnly) ||
          (flags & SliderFlags_ReadOnly))
        set_new_value = false;

    if (set_new_value) {
      TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(
          data_type, clicked_t, v_min, v_max, is_logarithmic,
          logarithmic_zero_epsilon, zero_deadzone_halfsize);

      // Round to user desired precision based on format string
      if (is_floating_point && !(flags & SliderFlags_NoRoundToFormat))
        v_new = RoundScalarWithFormatT<TYPE>(format, data_type, v_new);

      // Apply result
      if (*v != v_new) {
        *v = v_new;
        value_changed = true;
      }
    }
  }

  if (slider_sz < 1.0f) {
    *out_grab_bb = Rect(bb.Min, bb.Min);
  } else {
    // Output grab position so it can be displayed by the caller
    float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
        data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon,
        zero_deadzone_halfsize);
    if (axis == Axis_Y)
      grab_t = 1.0f - grab_t;
    const float grab_pos =
        Lerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
    if (axis == Axis_X)
      *out_grab_bb = Rect(grab_pos - grab_sz * 0.5f, bb.Min.y + grab_padding,
                          grab_pos + grab_sz * 0.5f, bb.Max.y - grab_padding);
    else
      *out_grab_bb = Rect(bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f,
                          bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f);
  }

  return value_changed;
}

// For 32-bit and larger types, slider bounds are limited to half the natural
// type range. So e.g. an integer Slider between INT_MAX-10 and INT_MAX will
// fail, but an integer Slider between INT_MAX/2-10 and INT_MAX/2 will be ok. It
// would be possible to lift that limitation with some work but it doesn't seem
// to be worth it for sliders.
bool Gui::SliderBehavior(const Rect &bb, ID id, DataType data_type, void *p_v,
                         const void *p_min, const void *p_max,
                         const char *format, SliderFlags flags,
                         Rect *out_grab_bb) {
  // Read gui.cpp "API BREAKING CHANGES" section for 1.78 if you hit this
  // assert.
  ASSERT((flags == 1 || (flags & SliderFlags_InvalidMask_) == 0) &&
         "Invalid SliderFlags flag!  Has the 'float power' argument "
         "been mistakenly cast to flags? Call function with "
         "SliderFlags_Logarithmic flags instead.");

  switch (data_type) {
  case DataType_S8: {
    S32 v32 = (S32) * (S8 *)p_v;
    bool r = SliderBehaviorT<S32, S32, float>(
        bb, id, DataType_S32, &v32, *(const S8 *)p_min, *(const S8 *)p_max,
        format, flags, out_grab_bb);
    if (r)
      *(S8 *)p_v = (S8)v32;
    return r;
  }
  case DataType_U8: {
    U32 v32 = (U32) * (U8 *)p_v;
    bool r = SliderBehaviorT<U32, S32, float>(
        bb, id, DataType_U32, &v32, *(const U8 *)p_min, *(const U8 *)p_max,
        format, flags, out_grab_bb);
    if (r)
      *(U8 *)p_v = (U8)v32;
    return r;
  }
  case DataType_S16: {
    S32 v32 = (S32) * (S16 *)p_v;
    bool r = SliderBehaviorT<S32, S32, float>(
        bb, id, DataType_S32, &v32, *(const S16 *)p_min, *(const S16 *)p_max,
        format, flags, out_grab_bb);
    if (r)
      *(S16 *)p_v = (S16)v32;
    return r;
  }
  case DataType_U16: {
    U32 v32 = (U32) * (U16 *)p_v;
    bool r = SliderBehaviorT<U32, S32, float>(
        bb, id, DataType_U32, &v32, *(const U16 *)p_min, *(const U16 *)p_max,
        format, flags, out_grab_bb);
    if (r)
      *(U16 *)p_v = (U16)v32;
    return r;
  }
  case DataType_S32:
    ASSERT(*(const S32 *)p_min >= S32_MIN / 2 &&
           *(const S32 *)p_max <= S32_MAX / 2);
    return SliderBehaviorT<S32, S32, float>(
        bb, id, data_type, (S32 *)p_v, *(const S32 *)p_min, *(const S32 *)p_max,
        format, flags, out_grab_bb);
  case DataType_U32:
    ASSERT(*(const U32 *)p_max <= U32_MAX / 2);
    return SliderBehaviorT<U32, S32, float>(
        bb, id, data_type, (U32 *)p_v, *(const U32 *)p_min, *(const U32 *)p_max,
        format, flags, out_grab_bb);
  case DataType_S64:
    ASSERT(*(const S64 *)p_min >= S64_MIN / 2 &&
           *(const S64 *)p_max <= S64_MAX / 2);
    return SliderBehaviorT<S64, S64, double>(
        bb, id, data_type, (S64 *)p_v, *(const S64 *)p_min, *(const S64 *)p_max,
        format, flags, out_grab_bb);
  case DataType_U64:
    ASSERT(*(const U64 *)p_max <= U64_MAX / 2);
    return SliderBehaviorT<U64, S64, double>(
        bb, id, data_type, (U64 *)p_v, *(const U64 *)p_min, *(const U64 *)p_max,
        format, flags, out_grab_bb);
  case DataType_Float:
    ASSERT(*(const float *)p_min >= -FLT_MAX / 2.0f &&
           *(const float *)p_max <= FLT_MAX / 2.0f);
    return SliderBehaviorT<float, float, float>(
        bb, id, data_type, (float *)p_v, *(const float *)p_min,
        *(const float *)p_max, format, flags, out_grab_bb);
  case DataType_Double:
    ASSERT(*(const double *)p_min >= -DBL_MAX / 2.0f &&
           *(const double *)p_max <= DBL_MAX / 2.0f);
    return SliderBehaviorT<double, double, double>(
        bb, id, data_type, (double *)p_v, *(const double *)p_min,
        *(const double *)p_max, format, flags, out_grab_bb);
  case DataType_COUNT:
    break;
  }
  ASSERT(0);
  return false;
}

// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the
// data. For a slider, they are all required. Read code of e.g. SliderFloat(),
// SliderInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how
// to use this function directly.
bool Gui::SliderScalar(const char *label, DataType data_type, void *p_data,
                       const void *p_min, const void *p_max, const char *format,
                       SliderFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const ID id = window->GetID(label);
  const float w = CalcItemWidth();

  const Vec2 label_size = CalcTextSize(label, NULL, true);
  const Rect frame_bb(window->DC.CursorPos,
                      window->DC.CursorPos +
                          Vec2(w, label_size.y + style.FramePadding.y * 2.0f));
  const Rect total_bb(frame_bb.Min,
                      frame_bb.Max +
                          Vec2(label_size.x > 0.0f
                                   ? style.ItemInnerSpacing.x + label_size.x
                                   : 0.0f,
                               0.0f));

  const bool temp_input_allowed = (flags & SliderFlags_NoInput) == 0;
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id, &frame_bb,
               temp_input_allowed ? ItemFlags_Inputable : 0))
    return false;

  // Default format string when passing NULL
  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;

  const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
  bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
  if (!temp_input_is_active) {
    // Tabbing or CTRL-clicking on Slider turns it into an input box
    const bool clicked = hovered && IsMouseClicked(0, id);
    const bool make_active = (clicked || g.NavActivateId == id);
    if (make_active && clicked)
      SetKeyOwner(Key_MouseLeft, id);
    if (make_active && temp_input_allowed)
      if ((clicked && g.IO.KeyCtrl) ||
          (g.NavActivateId == id &&
           (g.NavActivateFlags & ActivateFlags_PreferInput)))
        temp_input_is_active = true;

    if (make_active && !temp_input_is_active) {
      SetActiveID(id, window);
      SetFocusID(id, window);
      FocusWindow(window);
      g.ActiveIdUsingNavDirMask |= (1 << Dir_Left) | (1 << Dir_Right);
    }
  }

  if (temp_input_is_active) {
    // Only clamp CTRL+Click input when SliderFlags_AlwaysClamp is set
    const bool is_clamp_input = (flags & SliderFlags_AlwaysClamp) != 0;
    return TempInputScalar(frame_bb, id, label, data_type, p_data, format,
                           is_clamp_input ? p_min : NULL,
                           is_clamp_input ? p_max : NULL);
  }

  // Draw frame
  const U32 frame_col = GetColorU32(g.ActiveId == id ? Col_FrameBgActive
                                    : hovered        ? Col_FrameBgHovered
                                                     : Col_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true,
              g.Style.FrameRounding);

  // Slider behavior
  Rect grab_bb;
  const bool value_changed = SliderBehavior(
      frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
  if (value_changed)
    MarkItemEdited(id);

  // Render grab
  if (grab_bb.Max.x > grab_bb.Min.x)
    window->DrawList->AddRectFilled(
        grab_bb.Min, grab_bb.Max,
        GetColorU32(g.ActiveId == id ? Col_SliderGrabActive : Col_SliderGrab),
        style.GrabRounding);

  // Display value using user-provided display format so user can add
  // prefix/suffix/decorations to the value.
  char value_buf[64];
  const char *value_buf_end =
      value_buf + DataTypeFormatString(value_buf, ARRAYSIZE(value_buf),
                                       data_type, p_data, format);
  if (g.LogEnabled)
    LogSetNextTextDecoration("{", "}");
  RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL,
                    Vec2(0.5f, 0.5f));

  if (label_size.x > 0.0f)
    RenderText(Vec2(frame_bb.Max.x + style.ItemInnerSpacing.x,
                    frame_bb.Min.y + style.FramePadding.y),
               label);

  TEST_ENGINE_ITEM_INFO(
      id, label,
      g.LastItemData.StatusFlags |
          (temp_input_allowed ? ItemStatusFlags_Inputable : 0));
  return value_changed;
}

// Add multiple sliders on 1 line for compact edition of multiple components
bool Gui::SliderScalarN(const char *label, DataType data_type, void *v,
                        int components, const void *v_min, const void *v_max,
                        const char *format, SliderFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  bool value_changed = false;
  BeginGroup();
  PushID(label);
  PushMultiItemsWidths(components, CalcItemWidth());
  size_t type_size = GDataTypeInfo[data_type].Size;
  for (int i = 0; i < components; i++) {
    PushID(i);
    if (i > 0)
      SameLine(0, g.Style.ItemInnerSpacing.x);
    value_changed |=
        SliderScalar("", data_type, v, v_min, v_max, format, flags);
    PopID();
    PopItemWidth();
    v = (void *)((char *)v + type_size);
  }
  PopID();

  const char *label_end = FindRenderedTextEnd(label);
  if (label != label_end) {
    SameLine(0, g.Style.ItemInnerSpacing.x);
    TextEx(label, label_end);
  }

  EndGroup();
  return value_changed;
}

bool Gui::SliderFloat(const char *label, float *v, float v_min, float v_max,
                      const char *format, SliderFlags flags) {
  return SliderScalar(label, DataType_Float, v, &v_min, &v_max, format, flags);
}

bool Gui::SliderFloat2(const char *label, float v[2], float v_min, float v_max,
                       const char *format, SliderFlags flags) {
  return SliderScalarN(label, DataType_Float, v, 2, &v_min, &v_max, format,
                       flags);
}

bool Gui::SliderFloat3(const char *label, float v[3], float v_min, float v_max,
                       const char *format, SliderFlags flags) {
  return SliderScalarN(label, DataType_Float, v, 3, &v_min, &v_max, format,
                       flags);
}

bool Gui::SliderFloat4(const char *label, float v[4], float v_min, float v_max,
                       const char *format, SliderFlags flags) {
  return SliderScalarN(label, DataType_Float, v, 4, &v_min, &v_max, format,
                       flags);
}

bool Gui::SliderAngle(const char *label, float *v_rad, float v_degrees_min,
                      float v_degrees_max, const char *format,
                      SliderFlags flags) {
  if (format == NULL)
    format = "%.0f deg";
  float v_deg = (*v_rad) * 360.0f / (2 * PI);
  bool value_changed =
      SliderFloat(label, &v_deg, v_degrees_min, v_degrees_max, format, flags);
  *v_rad = v_deg * (2 * PI) / 360.0f;
  return value_changed;
}

bool Gui::SliderInt(const char *label, int *v, int v_min, int v_max,
                    const char *format, SliderFlags flags) {
  return SliderScalar(label, DataType_S32, v, &v_min, &v_max, format, flags);
}

bool Gui::SliderInt2(const char *label, int v[2], int v_min, int v_max,
                     const char *format, SliderFlags flags) {
  return SliderScalarN(label, DataType_S32, v, 2, &v_min, &v_max, format,
                       flags);
}

bool Gui::SliderInt3(const char *label, int v[3], int v_min, int v_max,
                     const char *format, SliderFlags flags) {
  return SliderScalarN(label, DataType_S32, v, 3, &v_min, &v_max, format,
                       flags);
}

bool Gui::SliderInt4(const char *label, int v[4], int v_min, int v_max,
                     const char *format, SliderFlags flags) {
  return SliderScalarN(label, DataType_S32, v, 4, &v_min, &v_max, format,
                       flags);
}

bool Gui::VSliderScalar(const char *label, const Vec2 &size, DataType data_type,
                        void *p_data, const void *p_min, const void *p_max,
                        const char *format, SliderFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const ID id = window->GetID(label);

  const Vec2 label_size = CalcTextSize(label, NULL, true);
  const Rect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
  const Rect bb(frame_bb.Min,
                frame_bb.Max +
                    Vec2(label_size.x > 0.0f
                             ? style.ItemInnerSpacing.x + label_size.x
                             : 0.0f,
                         0.0f));

  ItemSize(bb, style.FramePadding.y);
  if (!ItemAdd(frame_bb, id))
    return false;

  // Default format string when passing NULL
  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;

  const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
  const bool clicked = hovered && IsMouseClicked(0, id);
  if (clicked || g.NavActivateId == id) {
    if (clicked)
      SetKeyOwner(Key_MouseLeft, id);
    SetActiveID(id, window);
    SetFocusID(id, window);
    FocusWindow(window);
    g.ActiveIdUsingNavDirMask |= (1 << Dir_Up) | (1 << Dir_Down);
  }

  // Draw frame
  const U32 frame_col = GetColorU32(g.ActiveId == id ? Col_FrameBgActive
                                    : hovered        ? Col_FrameBgHovered
                                                     : Col_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true,
              g.Style.FrameRounding);

  // Slider behavior
  Rect grab_bb;
  const bool value_changed =
      SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format,
                     flags | SliderFlags_Vertical, &grab_bb);
  if (value_changed)
    MarkItemEdited(id);

  // Render grab
  if (grab_bb.Max.y > grab_bb.Min.y)
    window->DrawList->AddRectFilled(
        grab_bb.Min, grab_bb.Max,
        GetColorU32(g.ActiveId == id ? Col_SliderGrabActive : Col_SliderGrab),
        style.GrabRounding);

  // Display value using user-provided display format so user can add
  // prefix/suffix/decorations to the value. For the vertical slider we allow
  // centered text to overlap the frame padding
  char value_buf[64];
  const char *value_buf_end =
      value_buf + DataTypeFormatString(value_buf, ARRAYSIZE(value_buf),
                                       data_type, p_data, format);
  RenderTextClipped(Vec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y),
                    frame_bb.Max, value_buf, value_buf_end, NULL,
                    Vec2(0.5f, 0.0f));
  if (label_size.x > 0.0f)
    RenderText(Vec2(frame_bb.Max.x + style.ItemInnerSpacing.x,
                    frame_bb.Min.y + style.FramePadding.y),
               label);

  return value_changed;
}

bool Gui::VSliderFloat(const char *label, const Vec2 &size, float *v,
                       float v_min, float v_max, const char *format,
                       SliderFlags flags) {
  return VSliderScalar(label, size, DataType_Float, v, &v_min, &v_max, format,
                       flags);
}

bool Gui::VSliderInt(const char *label, const Vec2 &size, int *v, int v_min,
                     int v_max, const char *format, SliderFlags flags) {
  return VSliderScalar(label, size, DataType_S32, v, &v_min, &v_max, format,
                       flags);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
//-------------------------------------------------------------------------
// - ParseFormatFindStart() [Internal]
// - ParseFormatFindEnd() [Internal]
// - ParseFormatTrimDecorations() [Internal]
// - ParseFormatSanitizeForPrinting() [Internal]
// - ParseFormatSanitizeForScanning() [Internal]
// - ParseFormatPrecision() [Internal]
// - TempInputTextScalar() [Internal]
// - InputScalar()
// - InputScalarN()
// - InputFloat()
// - InputFloat2()
// - InputFloat3()
// - InputFloat4()
// - InputInt()
// - InputInt2()
// - InputInt3()
// - InputInt4()
// - InputDouble()
//-------------------------------------------------------------------------

// We don't use strchr() because our strings are usually very short and often
// start with '%'
const char *ParseFormatFindStart(const char *fmt) {
  while (char c = fmt[0]) {
    if (c == '%' && fmt[1] != '%')
      return fmt;
    else if (c == '%')
      fmt++;
    fmt++;
  }
  return fmt;
}

const char *ParseFormatFindEnd(const char *fmt) {
  // Printf/scanf types modifiers: I/L/h/j/l/t/w/z. Other uppercase letters
  // qualify as types aka end of the format.
  if (fmt[0] != '%')
    return fmt;
  const unsigned int ignored_uppercase_mask =
      (1 << ('I' - 'A')) | (1 << ('L' - 'A'));
  const unsigned int ignored_lowercase_mask =
      (1 << ('h' - 'a')) | (1 << ('j' - 'a')) | (1 << ('l' - 'a')) |
      (1 << ('t' - 'a')) | (1 << ('w' - 'a')) | (1 << ('z' - 'a'));
  for (char c; (c = *fmt) != 0; fmt++) {
    if (c >= 'A' && c <= 'Z' &&
        ((1 << (c - 'A')) & ignored_uppercase_mask) == 0)
      return fmt + 1;
    if (c >= 'a' && c <= 'z' &&
        ((1 << (c - 'a')) & ignored_lowercase_mask) == 0)
      return fmt + 1;
  }
  return fmt;
}

// Extract the format out of a format string with leading or trailing
// decorations
//  fmt = "blah blah"  -> return ""
//  fmt = "%.3f"       -> return fmt
//  fmt = "hello %.3f" -> return fmt + 6
//  fmt = "%.3f hello" -> return buf written with "%.3f"
const char *ParseFormatTrimDecorations(const char *fmt, char *buf,
                                       size_t buf_size) {
  const char *fmt_start = ParseFormatFindStart(fmt);
  if (fmt_start[0] != '%')
    return "";
  const char *fmt_end = ParseFormatFindEnd(fmt_start);
  if (fmt_end[0] ==
      0) // If we only have leading decoration, we don't need to copy the data.
    return fmt_start;
  Strncpy(buf, fmt_start, Min((size_t)(fmt_end - fmt_start) + 1, buf_size));
  return buf;
}

// Sanitize format
// - Zero terminate so extra characters after format (e.g. "%f123") don't
// confuse atof/atoi
// - sprintf.h supports several new modifiers which format numbers in a way
// that also makes them incompatible atof/atoi.
void ParseFormatSanitizeForPrinting(const char *fmt_in, char *fmt_out,
                                    size_t fmt_out_size) {
  const char *fmt_end = ParseFormatFindEnd(fmt_in);
  UNUSED(fmt_out_size);
  ASSERT(
      (size_t)(fmt_end - fmt_in + 1) <
      fmt_out_size); // Format is too long, let us know if this happens to you!
  while (fmt_in < fmt_end) {
    char c = *fmt_in++;
    if (c != '\'' && c != '$' &&
        c != '_') // Custom flags provided by sprintf.h. POSIX 2008 also
                  // supports '.
      *(fmt_out++) = c;
  }
  *fmt_out = 0; // Zero-terminate
}

// - For scanning we need to remove all width and precision fields and flags
// "%+3.7f" -> "%f". BUT don't strip types like "%I64d" which includes digits. !
// "%07I64d" -> "%I64d"
const char *ParseFormatSanitizeForScanning(const char *fmt_in, char *fmt_out,
                                           size_t fmt_out_size) {
  const char *fmt_end = ParseFormatFindEnd(fmt_in);
  const char *fmt_out_begin = fmt_out;
  UNUSED(fmt_out_size);
  ASSERT(
      (size_t)(fmt_end - fmt_in + 1) <
      fmt_out_size); // Format is too long, let us know if this happens to you!
  bool has_type = false;
  while (fmt_in < fmt_end) {
    char c = *fmt_in++;
    if (!has_type &&
        ((c >= '0' && c <= '9') || c == '.' || c == '+' || c == '#'))
      continue;
    has_type |= ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z')); // Stop skipping digits
    if (c != '\'' && c != '$' &&
        c != '_') // Custom flags provided by sprintf.h. POSIX 2008 also
                  // supports '.
      *(fmt_out++) = c;
  }
  *fmt_out = 0; // Zero-terminate
  return fmt_out_begin;
}

template <typename TYPE>
static const char *Atoi(const char *src, TYPE *output) {
  int negative = 0;
  if (*src == '-') {
    negative = 1;
    src++;
  }
  if (*src == '+') {
    src++;
  }
  TYPE v = 0;
  while (*src >= '0' && *src <= '9')
    v = (v * 10) + (*src++ - '0');
  *output = negative ? -v : v;
  return src;
}

// Parse display precision back from the display format string
// FIXME: This is still used by some navigation code path to infer a minimum
// tweak step, but we should aim to rework widgets so it isn't needed.
int ParseFormatPrecision(const char *fmt, int default_precision) {
  fmt = ParseFormatFindStart(fmt);
  if (fmt[0] != '%')
    return default_precision;
  fmt++;
  while (*fmt >= '0' && *fmt <= '9')
    fmt++;
  int precision = INT_MAX;
  if (*fmt == '.') {
    fmt = Atoi<int>(fmt + 1, &precision);
    if (precision < 0 || precision > 99)
      precision = default_precision;
  }
  if (*fmt == 'e' || *fmt == 'E') // Maximum precision with scientific notation
    precision = -1;
  if ((*fmt == 'g' || *fmt == 'G') && precision == INT_MAX)
    precision = -1;
  return (precision == INT_MAX) ? default_precision : precision;
}

// Create text input in place of another active widget (e.g. used when doing a
// CTRL+Click on drag/slider widgets)
// FIXME: Facilitate using this in variety of other situations.
bool Gui::TempInputText(const Rect &bb, ID id, const char *label, char *buf,
                        int buf_size, InputTextFlags flags) {
  // On the first frame, g.TempInputTextId == 0, then on subsequent frames it
  // becomes == id. We clear ActiveID on the first frame to allow the
  // InputText() taking it back.
  Context &g = *GGui;
  const bool init = (g.TempInputId != id);
  if (init)
    ClearActiveID();

  g.CurrentWindow->DC.CursorPos = bb.Min;
  bool value_changed = InputTextEx(label, NULL, buf, buf_size, bb.GetSize(),
                                   flags | InputTextFlags_MergedItem);
  if (init) {
    // First frame we started displaying the InputText widget, we expect it to
    // take the active id.
    ASSERT(g.ActiveId == id);
    g.TempInputId = g.ActiveId;
  }
  return value_changed;
}

// Note that Drag/Slider functions are only forwarding the min/max values
// clamping values if the SliderFlags_AlwaysClamp flag is set! This is
// intended: this way we allow CTRL+Click manual input to set a value out of
// bounds, for maximum flexibility. However this may not be ideal for all uses,
// as some user code may break on out of bound values.
bool Gui::TempInputScalar(const Rect &bb, ID id, const char *label,
                          DataType data_type, void *p_data, const char *format,
                          const void *p_clamp_min, const void *p_clamp_max) {
  // FIXME: May need to clarify display behavior if format doesn't contain %.
  // "%d" -> "%d" / "There are %d items" -> "%d" / "items" -> "%d" (fallback).
  // Also see #6405
  const DataTypeInfo *type_info = DataTypeGetInfo(data_type);
  char fmt_buf[32];
  char data_buf[32];
  format = ParseFormatTrimDecorations(format, fmt_buf, ARRAYSIZE(fmt_buf));
  if (format[0] == 0)
    format = type_info->PrintFmt;
  DataTypeFormatString(data_buf, ARRAYSIZE(data_buf), data_type, p_data,
                       format);
  StrTrimBlanks(data_buf);

  InputTextFlags flags = InputTextFlags_AutoSelectAll |
                         (InputTextFlags)InputTextFlags_NoMarkEdited;

  bool value_changed = false;
  if (TempInputText(bb, id, label, data_buf, ARRAYSIZE(data_buf), flags)) {
    // Backup old value
    size_t data_type_size = type_info->Size;
    DataTypeTempStorage data_backup;
    memcpy(&data_backup, p_data, data_type_size);

    // Apply new value (or operations) then clamp
    DataTypeApplyFromText(data_buf, data_type, p_data, format);
    if (p_clamp_min || p_clamp_max) {
      if (p_clamp_min && p_clamp_max &&
          DataTypeCompare(data_type, p_clamp_min, p_clamp_max) > 0)
        Swap(p_clamp_min, p_clamp_max);
      DataTypeClamp(data_type, p_data, p_clamp_min, p_clamp_max);
    }

    // Only mark as edited if new value is different
    value_changed = memcmp(&data_backup, p_data, data_type_size) != 0;
    if (value_changed)
      MarkItemEdited(id);
  }
  return value_changed;
}

// Note: p_data, p_step, p_step_fast are _pointers_ to a memory address holding
// the data. For an Input widget, p_step and p_step_fast are optional. Read code
// of e.g. InputFloat(), InputInt() etc. or examples in 'Demo->Widgets->Data
// Types' to understand how to use this function directly.
bool Gui::InputScalar(const char *label, DataType data_type, void *p_data,
                      const void *p_step, const void *p_step_fast,
                      const char *format, InputTextFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  Style &style = g.Style;

  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;

  char buf[64];
  DataTypeFormatString(buf, ARRAYSIZE(buf), data_type, p_data, format);

  flags |= InputTextFlags_AutoSelectAll |
           (InputTextFlags)
               InputTextFlags_NoMarkEdited; // We call MarkItemEdited()
                                            // ourselves by comparing the actual
                                            // data rather than the string.

  bool value_changed = false;
  if (p_step == NULL) {
    if (InputText(label, buf, ARRAYSIZE(buf), flags))
      value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
  } else {
    const float button_size = GetFrameHeight();

    BeginGroup(); // The only purpose of the group here is to allow the caller
                  // to query item data e.g. IsItemActive()
    PushID(label);
    SetNextItemWidth(Max(
        1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
    if (InputText("", buf, ARRAYSIZE(buf),
                  flags)) // PushId(label) + "" gives us the expected ID from
                          // outside point of view
      value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
    TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label,
                          g.LastItemData.StatusFlags |
                              ItemStatusFlags_Inputable);

    // Step buttons
    const Vec2 backup_frame_padding = style.FramePadding;
    style.FramePadding.x = style.FramePadding.y;
    ButtonFlags button_flags = ButtonFlags_Repeat | ButtonFlags_DontClosePopups;
    if (flags & InputTextFlags_ReadOnly)
      BeginDisabled();
    SameLine(0, style.ItemInnerSpacing.x);
    if (ButtonEx("-", Vec2(button_size, button_size), button_flags)) {
      DataTypeApplyOp(data_type, '-', p_data, p_data,
                      g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
      value_changed = true;
    }
    SameLine(0, style.ItemInnerSpacing.x);
    if (ButtonEx("+", Vec2(button_size, button_size), button_flags)) {
      DataTypeApplyOp(data_type, '+', p_data, p_data,
                      g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
      value_changed = true;
    }
    if (flags & InputTextFlags_ReadOnly)
      EndDisabled();

    const char *label_end = FindRenderedTextEnd(label);
    if (label != label_end) {
      SameLine(0, style.ItemInnerSpacing.x);
      TextEx(label, label_end);
    }
    style.FramePadding = backup_frame_padding;

    PopID();
    EndGroup();
  }
  if (value_changed)
    MarkItemEdited(g.LastItemData.ID);

  return value_changed;
}

bool Gui::InputScalarN(const char *label, DataType data_type, void *p_data,
                       int components, const void *p_step,
                       const void *p_step_fast, const char *format,
                       InputTextFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  bool value_changed = false;
  BeginGroup();
  PushID(label);
  PushMultiItemsWidths(components, CalcItemWidth());
  size_t type_size = GDataTypeInfo[data_type].Size;
  for (int i = 0; i < components; i++) {
    PushID(i);
    if (i > 0)
      SameLine(0, g.Style.ItemInnerSpacing.x);
    value_changed |=
        InputScalar("", data_type, p_data, p_step, p_step_fast, format, flags);
    PopID();
    PopItemWidth();
    p_data = (void *)((char *)p_data + type_size);
  }
  PopID();

  const char *label_end = FindRenderedTextEnd(label);
  if (label != label_end) {
    SameLine(0.0f, g.Style.ItemInnerSpacing.x);
    TextEx(label, label_end);
  }

  EndGroup();
  return value_changed;
}

bool Gui::InputFloat(const char *label, float *v, float step, float step_fast,
                     const char *format, InputTextFlags flags) {
  return InputScalar(
      label, DataType_Float, (void *)v, (void *)(step > 0.0f ? &step : NULL),
      (void *)(step_fast > 0.0f ? &step_fast : NULL), format, flags);
}

bool Gui::InputFloat2(const char *label, float v[2], const char *format,
                      InputTextFlags flags) {
  return InputScalarN(label, DataType_Float, v, 2, NULL, NULL, format, flags);
}

bool Gui::InputFloat3(const char *label, float v[3], const char *format,
                      InputTextFlags flags) {
  return InputScalarN(label, DataType_Float, v, 3, NULL, NULL, format, flags);
}

bool Gui::InputFloat4(const char *label, float v[4], const char *format,
                      InputTextFlags flags) {
  return InputScalarN(label, DataType_Float, v, 4, NULL, NULL, format, flags);
}

bool Gui::InputInt(const char *label, int *v, int step, int step_fast,
                   InputTextFlags flags) {
  // Hexadecimal input provided as a convenience but the flag name is awkward.
  // Typically you'd use InputText() to parse your own data, if you want to
  // handle prefixes.
  const char *format =
      (flags & InputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
  return InputScalar(
      label, DataType_S32, (void *)v, (void *)(step > 0 ? &step : NULL),
      (void *)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool Gui::InputInt2(const char *label, int v[2], InputTextFlags flags) {
  return InputScalarN(label, DataType_S32, v, 2, NULL, NULL, "%d", flags);
}

bool Gui::InputInt3(const char *label, int v[3], InputTextFlags flags) {
  return InputScalarN(label, DataType_S32, v, 3, NULL, NULL, "%d", flags);
}

bool Gui::InputInt4(const char *label, int v[4], InputTextFlags flags) {
  return InputScalarN(label, DataType_S32, v, 4, NULL, NULL, "%d", flags);
}

bool Gui::InputDouble(const char *label, double *v, double step,
                      double step_fast, const char *format,
                      InputTextFlags flags) {
  return InputScalar(
      label, DataType_Double, (void *)v, (void *)(step > 0.0 ? &step : NULL),
      (void *)(step_fast > 0.0 ? &step_fast : NULL), format, flags);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: InputText, InputTextMultiline, InputTextWithHint
//-------------------------------------------------------------------------
// - InputText()
// - InputTextWithHint()
// - InputTextMultiline()
// - InputTextGetCharInfo() [Internal]
// - InputTextReindexLines() [Internal]
// - InputTextReindexLinesRange() [Internal]
// - InputTextEx() [Internal]
// - DebugNodeInputTextState() [Internal]
//-------------------------------------------------------------------------

bool Gui::InputText(const char *label, char *buf, size_t buf_size,
                    InputTextFlags flags, InputTextCallback callback,
                    void *user_data) {
  ASSERT(!(flags & InputTextFlags_Multiline)); // call InputTextMultiline()
  return InputTextEx(label, NULL, buf, (int)buf_size, Vec2(0, 0), flags,
                     callback, user_data);
}

bool Gui::InputTextMultiline(const char *label, char *buf, size_t buf_size,
                             const Vec2 &size, InputTextFlags flags,
                             InputTextCallback callback, void *user_data) {
  return InputTextEx(label, NULL, buf, (int)buf_size, size,
                     flags | InputTextFlags_Multiline, callback, user_data);
}

bool Gui::InputTextWithHint(const char *label, const char *hint, char *buf,
                            size_t buf_size, InputTextFlags flags,
                            InputTextCallback callback, void *user_data) {
  ASSERT(!(flags & InputTextFlags_Multiline)); // call InputTextMultiline() or
                                               // InputTextEx() manually if
                                               // you need multi-line + hint.
  return InputTextEx(label, hint, buf, (int)buf_size, Vec2(0, 0), flags,
                     callback, user_data);
}

static int InputTextCalcTextLenAndLineCount(const char *text_begin,
                                            const char **out_text_end) {
  int line_count = 0;
  const char *s = text_begin;
  while (
      char c =
          *s++) // We are only matching for \n so we can ignore UTF-8 decoding
    if (c == '\n')
      line_count++;
  s--;
  if (s[0] != '\n' && s[0] != '\r')
    line_count++;
  *out_text_end = s;
  return line_count;
}

static Vec2 InputTextCalcTextSizeW(Context *ctx, const Wchar *text_begin,
                                   const Wchar *text_end,
                                   const Wchar **remaining, Vec2 *out_offset,
                                   bool stop_on_new_line) {
  Context &g = *ctx;
  Font *font = g.Font;
  const float line_height = g.FontSize;
  const float scale = line_height / font->FontSize;

  Vec2 text_size = Vec2(0, 0);
  float line_width = 0.0f;

  const Wchar *s = text_begin;
  while (s < text_end) {
    unsigned int c = (unsigned int)(*s++);
    if (c == '\n') {
      text_size.x = Max(text_size.x, line_width);
      text_size.y += line_height;
      line_width = 0.0f;
      if (stop_on_new_line)
        break;
      continue;
    }
    if (c == '\r')
      continue;

    const float char_width = font->GetCharAdvance((Wchar)c) * scale;
    line_width += char_width;
  }

  if (text_size.x < line_width)
    text_size.x = line_width;

  if (out_offset)
    *out_offset =
        Vec2(line_width,
             text_size.y + line_height); // offset allow for the possibility
                                         // of sitting after a trailing \n

  if (line_width > 0 ||
      text_size.y == 0.0f) // whereas size.y will ignore the trailing \n
    text_size.y += line_height;

  if (remaining)
    *remaining = s;

  return text_size;
}

// Wrapper for textedit.h to edit text (our wrapper is for: statically sized
// buffer, single-line, wchar characters. InputText converts between UTF-8 and
// wchar)
namespace Stb {

static int TEXTEDIT_STRINGLEN(const InputTextState *obj) {
  return obj->CurLenW;
}
static Wchar TEXTEDIT_GETCHAR(const InputTextState *obj, int idx) {
  ASSERT(idx <= obj->CurLenW);
  return obj->TextW[idx];
}
static float TEXTEDIT_GETWIDTH(InputTextState *obj, int line_start_idx,
                               int char_idx) {
  Wchar c = obj->TextW[line_start_idx + char_idx];
  if (c == '\n')
    return TEXTEDIT_GETWIDTH_NEWLINE;
  Context &g = *obj->Ctx;
  return g.Font->GetCharAdvance(c) * (g.FontSize / g.Font->FontSize);
}
static int TEXTEDIT_KEYTOTEXT(int key) { return key >= 0x200000 ? 0 : key; }
static Wchar TEXTEDIT_NEWLINE = '\n';
static void TEXTEDIT_LAYOUTROW(StbTexteditRow *r, InputTextState *obj,
                               int line_start_idx) {
  const Wchar *text = obj->TextW.Data;
  const Wchar *text_remaining = NULL;
  const Vec2 size =
      InputTextCalcTextSizeW(obj->Ctx, text + line_start_idx,
                             text + obj->CurLenW, &text_remaining, NULL, true);
  r->x0 = 0.0f;
  r->x1 = size.x;
  r->baseline_y_delta = size.y;
  r->ymin = 0.0f;
  r->ymax = size.y;
  r->num_chars = (int)(text_remaining - (text + line_start_idx));
}

static bool is_separator(unsigned int c) {
  return c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' ||
         c == '[' || c == ']' || c == '|' || c == '\n' || c == '\r' ||
         c == '.' || c == '!';
}

static int is_word_boundary_from_right(InputTextState *obj, int idx) {
  // When InputTextFlags_Password is set, we don't want actions such as
  // CTRL+Arrow to leak the fact that underlying data are blanks or separators.
  if ((obj->Flags & InputTextFlags_Password) || idx <= 0)
    return 0;

  bool prev_white = CharIsBlankW(obj->TextW[idx - 1]);
  bool prev_separ = is_separator(obj->TextW[idx - 1]);
  bool curr_white = CharIsBlankW(obj->TextW[idx]);
  bool curr_separ = is_separator(obj->TextW[idx]);
  return ((prev_white || prev_separ) && !(curr_separ || curr_white)) ||
         (curr_separ && !prev_separ);
}
static int is_word_boundary_from_left(InputTextState *obj, int idx) {
  if ((obj->Flags & InputTextFlags_Password) || idx <= 0)
    return 0;

  bool prev_white = CharIsBlankW(obj->TextW[idx]);
  bool prev_separ = is_separator(obj->TextW[idx]);
  bool curr_white = CharIsBlankW(obj->TextW[idx - 1]);
  bool curr_separ = is_separator(obj->TextW[idx - 1]);
  return ((prev_white) && !(curr_separ || curr_white)) ||
         (curr_separ && !prev_separ);
}
static int TEXTEDIT_MOVEWORDLEFT_IMPL(InputTextState *obj, int idx) {
  idx--;
  while (idx >= 0 && !is_word_boundary_from_right(obj, idx))
    idx--;
  return idx < 0 ? 0 : idx;
}
static int TEXTEDIT_MOVEWORDRIGHT_MAC(InputTextState *obj, int idx) {
  idx++;
  int len = obj->CurLenW;
  while (idx < len && !is_word_boundary_from_left(obj, idx))
    idx++;
  return idx > len ? len : idx;
}
static int TEXTEDIT_MOVEWORDRIGHT_WIN(InputTextState *obj, int idx) {
  idx++;
  int len = obj->CurLenW;
  while (idx < len && !is_word_boundary_from_right(obj, idx))
    idx++;
  return idx > len ? len : idx;
}
static int TEXTEDIT_MOVEWORDRIGHT_IMPL(InputTextState *obj, int idx) {
  Context &g = *obj->Ctx;
  if (g.IO.ConfigMacOSXBehaviors)
    return TEXTEDIT_MOVEWORDRIGHT_MAC(obj, idx);
  else
    return TEXTEDIT_MOVEWORDRIGHT_WIN(obj, idx);
}
#define TEXTEDIT_MOVEWORDLEFT                                                  \
  TEXTEDIT_MOVEWORDLEFT_IMPL // They need to be #define for textedit.h
#define TEXTEDIT_MOVEWORDRIGHT TEXTEDIT_MOVEWORDRIGHT_IMPL

static void TEXTEDIT_DELETECHARS(InputTextState *obj, int pos, int n) {
  Wchar *dst = obj->TextW.Data + pos;

  // We maintain our buffer length in both UTF-8 and wchar formats
  obj->Edited = true;
  obj->CurLenA -= TextCountUtf8BytesFromStr(dst, dst + n);
  obj->CurLenW -= n;

  // Offset remaining text (FIXME-OPT: Use memmove)
  const Wchar *src = obj->TextW.Data + pos + n;
  while (Wchar c = *src++)
    *dst++ = c;
  *dst = '\0';
}

static bool TEXTEDIT_INSERTCHARS(InputTextState *obj, int pos,
                                 const Wchar *new_text, int new_text_len) {
  const bool is_resizable = (obj->Flags & InputTextFlags_CallbackResize) != 0;
  const int text_len = obj->CurLenW;
  ASSERT(pos <= text_len);

  const int new_text_len_utf8 =
      TextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
  if (!is_resizable &&
      (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufCapacityA))
    return false;

  // Grow internal buffer if needed
  if (new_text_len + text_len + 1 > obj->TextW.Size) {
    if (!is_resizable)
      return false;
    ASSERT(text_len < obj->TextW.Size);
    obj->TextW.resize(text_len +
                      Clamp(new_text_len * 4, 32, Max(256, new_text_len)) + 1);
  }

  Wchar *text = obj->TextW.Data;
  if (pos != text_len)
    memmove(text + pos + new_text_len, text + pos,
            (size_t)(text_len - pos) * sizeof(Wchar));
  memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(Wchar));

  obj->Edited = true;
  obj->CurLenW += new_text_len;
  obj->CurLenA += new_text_len_utf8;
  obj->TextW[obj->CurLenW] = '\0';

  return true;
}

// We don't use an enum so we can build even with conflicting symbols (if
// another user of textedit.h leak their TEXTEDIT_K_* symbols)
#define TEXTEDIT_K_LEFT 0x200000  // keyboard input to move cursor left
#define TEXTEDIT_K_RIGHT 0x200001 // keyboard input to move cursor right
#define TEXTEDIT_K_UP 0x200002    // keyboard input to move cursor up
#define TEXTEDIT_K_DOWN 0x200003  // keyboard input to move cursor down
#define TEXTEDIT_K_LINESTART                                                   \
  0x200004 // keyboard input to move cursor to start of line
#define TEXTEDIT_K_LINEEND                                                     \
  0x200005 // keyboard input to move cursor to end of line
#define TEXTEDIT_K_TEXTSTART                                                   \
  0x200006 // keyboard input to move cursor to start of text
#define TEXTEDIT_K_TEXTEND                                                     \
  0x200007 // keyboard input to move cursor to end of text
#define TEXTEDIT_K_DELETE                                                      \
  0x200008 // keyboard input to delete selection or character under cursor
#define TEXTEDIT_K_BACKSPACE                                                   \
  0x200009 // keyboard input to delete selection or character left of cursor
#define TEXTEDIT_K_UNDO 0x20000A // keyboard input to perform undo
#define TEXTEDIT_K_REDO 0x20000B // keyboard input to perform redo
#define TEXTEDIT_K_WORDLEFT                                                    \
  0x20000C // keyboard input to move cursor left one word
#define TEXTEDIT_K_WORDRIGHT                                                   \
  0x20000D                       // keyboard input to move cursor right one word
#define TEXTEDIT_K_PGUP 0x20000E // keyboard input to move cursor up a page
#define TEXTEDIT_K_PGDOWN 0x20000F // keyboard input to move cursor down a page
#define TEXTEDIT_K_SHIFT 0x400000

#define TEXTEDIT_IMPLEMENTATION
#define TEXTEDIT_memmove memmove
#include "textedit.hpp"

// textedit internally allows for a single undo record to do addition and
// deletion, but somehow, calling the textedit_paste() function creates two
// separate records, so we perform it manually. (FIXME: Report to nothings/stb?)
static void textedit_replace(InputTextState *str, TexteditState *state,
                             const TEXTEDIT_CHARTYPE *text, int text_len) {
  text_makeundo_replace(str, state, 0, str->CurLenW, text_len);
  Stb::TEXTEDIT_DELETECHARS(str, 0, str->CurLenW);
  state->cursor = state->select_start = state->select_end = 0;
  if (text_len <= 0)
    return;
  if (Stb::TEXTEDIT_INSERTCHARS(str, 0, text, text_len)) {
    state->cursor = state->select_start = state->select_end = text_len;
    state->has_preferred_x = 0;
    return;
  }
  ASSERT(0); // Failed to insert character, normally shouldn't happen because
             // of how we currently use textedit_replace()
}

} // namespace Stb

void InputTextState::OnKeyPressed(int key) {
  textedit_key(this, &Stb, key);
  CursorFollow = true;
  CursorAnimReset();
}

InputTextCallbackData::InputTextCallbackData() {
  memset(this, 0, sizeof(*this));
}

// Public API to manipulate UTF-8 text
// We expose UTF-8 to the user (unlike the TEXTEDIT_* functions which are
// manipulating wchar)
// FIXME: The existence of this rarely exercised code path is a bit of a
// nuisance.
void InputTextCallbackData::DeleteChars(int pos, int bytes_count) {
  ASSERT(pos + bytes_count <= BufTextLen);
  char *dst = Buf + pos;
  const char *src = Buf + pos + bytes_count;
  while (char c = *src++)
    *dst++ = c;
  *dst = '\0';

  if (CursorPos >= pos + bytes_count)
    CursorPos -= bytes_count;
  else if (CursorPos >= pos)
    CursorPos = pos;
  SelectionStart = SelectionEnd = CursorPos;
  BufDirty = true;
  BufTextLen -= bytes_count;
}

void InputTextCallbackData::InsertChars(int pos, const char *new_text,
                                        const char *new_text_end) {
  // Accept null ranges
  if (new_text == new_text_end)
    return;

  const bool is_resizable = (Flags & InputTextFlags_CallbackResize) != 0;
  const int new_text_len =
      new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
  if (new_text_len + BufTextLen >= BufSize) {
    if (!is_resizable)
      return;

    // Contrary to TEXTEDIT_INSERTCHARS() this is working in the UTF8
    // buffer, hence the mildly similar code (until we remove the U16 buffer
    // altogether!)
    Context &g = *Ctx;
    InputTextState *edit_state = &g.InputTextState;
    ASSERT(edit_state->ID != 0 && g.ActiveId == edit_state->ID);
    ASSERT(Buf == edit_state->TextA.Data);
    int new_buf_size =
        BufTextLen + Clamp(new_text_len * 4, 32, Max(256, new_text_len)) + 1;
    edit_state->TextA.reserve(new_buf_size + 1);
    Buf = edit_state->TextA.Data;
    BufSize = edit_state->BufCapacityA = new_buf_size;
  }

  if (BufTextLen != pos)
    memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
  memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
  Buf[BufTextLen + new_text_len] = '\0';

  if (CursorPos >= pos)
    CursorPos += new_text_len;
  SelectionStart = SelectionEnd = CursorPos;
  BufDirty = true;
  BufTextLen += new_text_len;
}

// Return false to discard a character.
static bool InputTextFilterCharacter(Context *ctx, unsigned int *p_char,
                                     InputTextFlags flags,
                                     InputTextCallback callback,
                                     void *user_data,
                                     InputSource input_source) {
  ASSERT(input_source == InputSource_Keyboard ||
         input_source == InputSource_Clipboard);
  unsigned int c = *p_char;

  // Filter non-printable (NB: isprint is unreliable! see #2467)
  bool apply_named_filters = true;
  if (c < 0x20) {
    bool pass = false;
    pass |=
        (c == '\n') && (flags & InputTextFlags_Multiline) !=
                           0; // Note that an Enter KEY will emit \r and be
                              // ignored (we poll for KEY in InputText() code)
    pass |= (c == '\t') && (flags & InputTextFlags_AllowTabInput) != 0;
    if (!pass)
      return false;
    apply_named_filters = false; // Override named filters below so newline and
                                 // tabs can still be inserted.
  }

  if (input_source != InputSource_Clipboard) {
    // We ignore Ascii representation of delete (emitted from Backspace on OSX,
    // see #2578, #2817)
    if (c == 127)
      return false;

    // Filter private Unicode range. GLFW on OSX seems to send private
    // characters for special keys like arrow keys (FIXME)
    if (c >= 0xE000 && c <= 0xF8FF)
      return false;
  }

  // Filter Unicode ranges we are not handling in this build
  if (c > UNICODE_CODEPOINT_MAX)
    return false;

  // Generic named filters
  if (apply_named_filters &&
      (flags & (InputTextFlags_CharsDecimal | InputTextFlags_CharsHexadecimal |
                InputTextFlags_CharsUppercase | InputTextFlags_CharsNoBlank |
                InputTextFlags_CharsScientific))) {
    // The libc allows overriding locale, with e.g. 'setlocale(LC_NUMERIC,
    // "de_DE.UTF-8");' which affect the output/input of printf/scanf to use
    // e.g. ',' instead of '.'. The standard mandate that programs starts in the
    // "C" locale where the decimal point is '.'. We don't really intend to
    // provide widespread support for it, but out of empathy for people stuck
    // with using odd API, we support the bare minimum aka overriding the
    // decimal point. Change the default decimal_point with:
    //   Gui::GetIO()->PlatformLocaleDecimalPoint =
    //   *localeconv()->decimal_point;
    // Users of non-default decimal point (in particular ',') may be affected by
    // word-selection logic
    // (is_word_boundary_from_right/is_word_boundary_from_left) functions.
    Context &g = *ctx;
    const unsigned c_decimal_point =
        (unsigned int)g.IO.PlatformLocaleDecimalPoint;
    if (flags & (InputTextFlags_CharsDecimal | InputTextFlags_CharsScientific))
      if (c == '.' || c == ',')
        c = c_decimal_point;

    // Full-width -> half-width conversion for numeric fields
    // (https://en.wikipedia.org/wiki/Halfwidth_and_Fullwidth_Forms_(Unicode_block)
    // While this is mostly convenient, this has the side-effect for uninformed
    // users accidentally inputting full-width characters that they may scratch
    // their head as to why it works in numerical fields vs in generic text
    // fields it would require support in the font.
    if (flags & (InputTextFlags_CharsDecimal | InputTextFlags_CharsScientific |
                 InputTextFlags_CharsHexadecimal))
      if (c >= 0xFF01 && c <= 0xFF5E)
        c = c - 0xFF01 + 0x21;

    // Allow 0-9 . - + * /
    if (flags & InputTextFlags_CharsDecimal)
      if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') &&
          (c != '+') && (c != '*') && (c != '/'))
        return false;

    // Allow 0-9 . - + * / e E
    if (flags & InputTextFlags_CharsScientific)
      if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') &&
          (c != '+') && (c != '*') && (c != '/') && (c != 'e') && (c != 'E'))
        return false;

    // Allow 0-9 a-F A-F
    if (flags & InputTextFlags_CharsHexadecimal)
      if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') &&
          !(c >= 'A' && c <= 'F'))
        return false;

    // Turn a-z into A-Z
    if (flags & InputTextFlags_CharsUppercase)
      if (c >= 'a' && c <= 'z')
        c += (unsigned int)('A' - 'a');

    if (flags & InputTextFlags_CharsNoBlank)
      if (CharIsBlankW(c))
        return false;

    *p_char = c;
  }

  // Custom callback filter
  if (flags & InputTextFlags_CallbackCharFilter) {
    Context &g = *GGui;
    InputTextCallbackData callback_data;
    callback_data.Ctx = &g;
    callback_data.EventFlag = InputTextFlags_CallbackCharFilter;
    callback_data.EventChar = (Wchar)c;
    callback_data.Flags = flags;
    callback_data.UserData = user_data;
    if (callback(&callback_data) != 0)
      return false;
    *p_char = callback_data.EventChar;
    if (!callback_data.EventChar)
      return false;
  }

  return true;
}

// Find the shortest single replacement we can make to get the new text from the
// old text. Important: needs to be run before TextW is rewritten with the new
// characters because calling TEXTEDIT_GETCHAR() at the end.
// FIXME: Ideally we should transition toward (1) making
// InsertChars()/DeleteChars() update undo-stack (2) discourage (and keep
// reconcile) or obsolete (and remove reconcile) accessing buffer directly.
static void InputTextReconcileUndoStateAfterUserCallback(InputTextState *state,
                                                         const char *new_buf_a,
                                                         int new_length_a) {
  Context &g = *GGui;
  const Wchar *old_buf = state->TextW.Data;
  const int old_length = state->CurLenW;
  const int new_length =
      TextCountCharsFromUtf8(new_buf_a, new_buf_a + new_length_a);
  g.TempBuffer.reserve_discard((new_length + 1) * sizeof(Wchar));
  Wchar *new_buf = (Wchar *)(void *)g.TempBuffer.Data;
  TextStrFromUtf8(new_buf, new_length + 1, new_buf_a, new_buf_a + new_length_a);

  const int shorter_length = Min(old_length, new_length);
  int first_diff;
  for (first_diff = 0; first_diff < shorter_length; first_diff++)
    if (old_buf[first_diff] != new_buf[first_diff])
      break;
  if (first_diff == old_length && first_diff == new_length)
    return;

  int old_last_diff = old_length - 1;
  int new_last_diff = new_length - 1;
  for (; old_last_diff >= first_diff && new_last_diff >= first_diff;
       old_last_diff--, new_last_diff--)
    if (old_buf[old_last_diff] != new_buf[new_last_diff])
      break;

  const int insert_len = new_last_diff - first_diff + 1;
  const int delete_len = old_last_diff - first_diff + 1;
  if (insert_len > 0 || delete_len > 0)
    if (TEXTEDIT_CHARTYPE *p = text_createundo(
            &state->Stb.undostate, first_diff, delete_len, insert_len))
      for (int i = 0; i < delete_len; i++)
        p[i] = Stb::TEXTEDIT_GETCHAR(state, first_diff + i);
}

// As InputText() retain textual data and we currently provide a path for user
// to not retain it (via local variables) we need some form of hook to reapply
// data back to user buffer on deactivation frame. (#4714) It would be more
// desirable that we discourage users from taking advantage of the "user not
// retaining data" trick, but that more likely be attractive when we do have
// _NoLiveEdit flag available.
void Gui::InputTextDeactivateHook(ID id) {
  Context &g = *GGui;
  InputTextState *state = &g.InputTextState;
  if (id == 0 || state->ID != id)
    return;
  g.InputTextDeactivatedState.ID = state->ID;
  if (state->Flags & InputTextFlags_ReadOnly) {
    g.InputTextDeactivatedState.TextA.resize(
        0); // In theory this data won't be used, but clear to be neat.
  } else {
    ASSERT(state->TextA.Data != 0);
    g.InputTextDeactivatedState.TextA.resize(state->CurLenA + 1);
    memcpy(g.InputTextDeactivatedState.TextA.Data, state->TextA.Data,
           state->CurLenA + 1);
  }
}

// Edit a string of text
// - buf_size account for the zero-terminator, so a buf_size of 6 can hold
// "Hello" but not "Hello!".
//   This is so we can easily call InputText() on static arrays using
//   ARRAYSIZE() and to match Note that in std::string world, capacity() would
//   omit 1 byte used by the zero-terminator.
// - When active, hold on a privately held copy of the text (and apply back to
// 'buf'). So changing 'buf' while the InputText is active has no effect.
// - If you want to use Gui::InputText() with std::string, see
// misc/cpp/stdlib.h (FIXME: Rather confusing and messy function, among
// the worse part of our codebase, expecting to rewrite a V2 at some point..
// Partly because we are
//  doing UTF8 > U16 > UTF8 conversions on the go to easily interface with
//  textedit. Ideally should stay in UTF-8 all the time. See
//  https://github.com/nothings/stb/issues/188)
bool Gui::InputTextEx(const char *label, const char *hint, char *buf,
                      int buf_size, const Vec2 &size_arg, InputTextFlags flags,
                      InputTextCallback callback, void *callback_user_data) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ASSERT(buf != NULL && buf_size >= 0);
  ASSERT(!((flags & InputTextFlags_CallbackHistory) &&
           (flags & InputTextFlags_Multiline))); // Can't use both together
                                                 // (they both use up/down keys)
  ASSERT(!((flags & InputTextFlags_CallbackCompletion) &&
           (flags & InputTextFlags_AllowTabInput))); // Can't use both together
                                                     // (they both use tab key)

  Context &g = *GGui;
  IO &io = g.IO;
  const Style &style = g.Style;

  const bool RENDER_SELECTION_WHEN_INACTIVE = false;
  const bool is_multiline = (flags & InputTextFlags_Multiline) != 0;

  if (is_multiline) // Open group before calling GetID() because groups tracks
                    // id created within their scope (including the scrollbar)
    BeginGroup();
  const ID id = window->GetID(label);
  const Vec2 label_size = CalcTextSize(label, NULL, true);
  const Vec2 frame_size = CalcItemSize(
      size_arg, CalcItemWidth(),
      (is_multiline ? g.FontSize * 8.0f : label_size.y) +
          style.FramePadding.y *
              2.0f); // Arbitrary default of 8 lines high for multi-line
  const Vec2 total_size =
      Vec2(frame_size.x + (label_size.x > 0.0f
                               ? style.ItemInnerSpacing.x + label_size.x
                               : 0.0f),
           frame_size.y);

  const Rect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
  const Rect total_bb(frame_bb.Min, frame_bb.Min + total_size);

  Window *draw_window = window;
  Vec2 inner_size = frame_size;
  LastItemData item_data_backup;
  if (is_multiline) {
    Vec2 backup_pos = window->DC.CursorPos;
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb, ItemFlags_Inputable)) {
      EndGroup();
      return false;
    }
    item_data_backup = g.LastItemData;
    window->DC.CursorPos = backup_pos;

    // Prevent NavActivation from Tabbing when our widget accepts Tab inputs:
    // this allows cycling through widgets without stopping.
    if (g.NavActivateId == id &&
        (g.NavActivateFlags & ActivateFlags_FromTabbing) &&
        (flags & InputTextFlags_AllowTabInput))
      g.NavActivateId = 0;

    // Prevent NavActivate reactivating in BeginChild() when we are already
    // active.
    const ID backup_activate_id = g.NavActivateId;
    if (g.ActiveId == id) // Prevent reactivation
      g.NavActivateId = 0;

    // We reproduce the contents of BeginChildFrame() in order to provide
    // 'label' so our window internal data are easier to read/debug.
    PushStyleColor(Col_ChildBg, style.Colors[Col_FrameBg]);
    PushStyleVar(StyleVar_ChildRounding, style.FrameRounding);
    PushStyleVar(StyleVar_ChildBorderSize, style.FrameBorderSize);
    PushStyleVar(StyleVar_WindowPadding,
                 Vec2(0, 0)); // Ensure no clip rect so mouse hover can reach
                              // FramePadding edges
    bool child_visible =
        BeginChildEx(label, id, frame_bb.GetSize(), true, WindowFlags_NoMove);
    g.NavActivateId = backup_activate_id;
    PopStyleVar(3);
    PopStyleColor();
    if (!child_visible) {
      EndChild();
      EndGroup();
      return false;
    }
    draw_window = g.CurrentWindow; // Child window
    draw_window->DC.NavLayersActiveMaskNext |=
        (1 << draw_window->DC
                  .NavLayerCurrent); // This is to ensure that EndChild() will
                                     // display a navigation highlight so we can
                                     // "enter" into it.
    draw_window->DC.CursorPos += style.FramePadding;
    inner_size.x -= draw_window->ScrollbarSizes.x;
  } else {
    // Support for internal InputTextFlags_MergedItem flag, which could be
    // redesigned as an ItemFlags if needed (with test performed in ItemAdd)
    ItemSize(total_bb, style.FramePadding.y);
    if (!(flags & InputTextFlags_MergedItem))
      if (!ItemAdd(total_bb, id, &frame_bb, ItemFlags_Inputable))
        return false;
  }
  const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
  if (hovered)
    g.MouseCursor = MouseCursor_TextInput;

  // We are only allowed to access the state if we are already the active
  // widget.
  InputTextState *state = GetInputTextState(id);

  if (g.LastItemData.InFlags & ItemFlags_ReadOnly)
    flags |= InputTextFlags_ReadOnly;
  const bool is_readonly = (flags & InputTextFlags_ReadOnly) != 0;
  const bool is_password = (flags & InputTextFlags_Password) != 0;
  const bool is_undoable = (flags & InputTextFlags_NoUndoRedo) == 0;
  const bool is_resizable = (flags & InputTextFlags_CallbackResize) != 0;
  if (is_resizable)
    ASSERT(callback != NULL); // Must provide a callback if you set the
                              // InputTextFlags_CallbackResize flag!

  const bool input_requested_by_nav =
      (g.ActiveId != id) &&
      ((g.NavActivateId == id) &&
       ((g.NavActivateFlags & ActivateFlags_PreferInput) ||
        (g.NavInputSource == InputSource_Keyboard)));

  const bool user_clicked = hovered && io.MouseClicked[0];
  const bool user_scroll_finish =
      is_multiline && state != NULL && g.ActiveId == 0 &&
      g.ActiveIdPreviousFrame == GetWindowScrollbarID(draw_window, Axis_Y);
  const bool user_scroll_active =
      is_multiline && state != NULL &&
      g.ActiveId == GetWindowScrollbarID(draw_window, Axis_Y);
  bool clear_active_id = false;
  bool select_all = false;

  float scroll_y = is_multiline ? draw_window->Scroll.y : FLT_MAX;

  const bool init_changed_specs =
      (state != NULL &&
       state->Stb.single_line !=
           !is_multiline); // state != NULL means its our state.
  const bool init_make_active =
      (user_clicked || user_scroll_finish || input_requested_by_nav);
  const bool init_state = (init_make_active || user_scroll_active);
  if ((init_state && g.ActiveId != id) || init_changed_specs) {
    // Access state even if we don't own it yet.
    state = &g.InputTextState;
    state->CursorAnimReset();

    // Backup state of deactivating item so they'll have a chance to do a write
    // to output buffer on the same frame they report IsItemDeactivatedAfterEdit
    // (#4714)
    InputTextDeactivateHook(state->ID);

    // Take a copy of the initial buffer value (both in original UTF-8 format
    // and converted to wchar) From the moment we focused we are ignoring the
    // content of 'buf' (unless we are in read-only mode)
    const int buf_len = (int)strlen(buf);
    state->InitialTextA.resize(
        buf_len + 1); // UTF-8. we use +1 to make sure that .Data is always
                      // pointing to at least an empty string.
    memcpy(state->InitialTextA.Data, buf, buf_len + 1);

    // Preserve cursor position and undo/redo stack if we come back to same
    // widget
    // FIXME: Since we reworked this on 2022/06, may want to differenciate
    // recycle_cursor vs recycle_undostate?
    bool recycle_state = (state->ID == id && !init_changed_specs);
    if (recycle_state && (state->CurLenA != buf_len ||
                          (state->TextAIsValid &&
                           strncmp(state->TextA.Data, buf, buf_len) != 0)))
      recycle_state = false;

    // Start edition
    const char *buf_end = NULL;
    state->ID = id;
    state->TextW.resize(
        buf_size +
        1); // wchar count <= UTF-8 count. we use +1 to make sure that .Data is
            // always pointing to at least an empty string.
    state->TextA.resize(0);
    state->TextAIsValid =
        false; // TextA is not valid yet (we will display buf until then)
    state->CurLenW =
        TextStrFromUtf8(state->TextW.Data, buf_size, buf, NULL, &buf_end);
    state->CurLenA =
        (int)(buf_end -
              buf); // We can't get the result from Strncpy() above because it
                    // is not UTF-8 aware. Here we'll cut off malformed UTF-8.

    if (recycle_state) {
      // Recycle existing cursor/selection/undo stack but clamp position
      // Note a single mouse click will override the cursor/position immediately
      // by calling textedit_click handler.
      state->CursorClamp();
    } else {
      state->ScrollX = 0.0f;
      textedit_initialize_state(&state->Stb, !is_multiline);
    }

    if (!is_multiline) {
      if (flags & InputTextFlags_AutoSelectAll)
        select_all = true;
      if (input_requested_by_nav &&
          (!recycle_state ||
           !(g.NavActivateFlags & ActivateFlags_TryToPreserveState)))
        select_all = true;
      if (user_clicked && io.KeyCtrl)
        select_all = true;
    }

    if (flags & InputTextFlags_AlwaysOverwrite)
      state->Stb.insert_mode =
          1; // stb field name is indeed incorrect (see #2863)
  }

  const bool is_osx = io.ConfigMacOSXBehaviors;
  if (g.ActiveId != id && init_make_active) {
    ASSERT(state && state->ID == id);
    SetActiveID(id, window);
    SetFocusID(id, window);
    FocusWindow(window);
  }
  if (g.ActiveId == id) {
    // Declare some inputs, the other are registered and polled via Shortcut()
    // routing system.
    if (user_clicked)
      SetKeyOwner(Key_MouseLeft, id);
    g.ActiveIdUsingNavDirMask |= (1 << Dir_Left) | (1 << Dir_Right);
    if (is_multiline || (flags & InputTextFlags_CallbackHistory))
      g.ActiveIdUsingNavDirMask |= (1 << Dir_Up) | (1 << Dir_Down);
    SetKeyOwner(Key_Home, id);
    SetKeyOwner(Key_End, id);
    if (is_multiline) {
      SetKeyOwner(Key_PageUp, id);
      SetKeyOwner(Key_PageDown, id);
    }
    if (is_osx)
      SetKeyOwner(Mod_Alt, id);
    if (flags &
        (InputTextFlags_CallbackCompletion |
         InputTextFlags_AllowTabInput)) // Disable keyboard tabbing out as
                                        // we will use the \t character.
      SetShortcutRouting(Key_Tab, id);
  }

  // We have an edge case if ActiveId was set through another widget (e.g.
  // widget being swapped), clear id immediately (don't wait until the end of
  // the function)
  if (g.ActiveId == id && state == NULL)
    ClearActiveID();

  // Release focus when we click outside
  if (g.ActiveId == id && io.MouseClicked[0] && !init_state &&
      !init_make_active) //-V560
    clear_active_id = true;

  // Lock the decision of whether we are going to take the path displaying the
  // cursor or selection
  bool render_cursor = (g.ActiveId == id) || (state && user_scroll_active);
  bool render_selection = state && (state->HasSelection() || select_all) &&
                          (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
  bool value_changed = false;
  bool validated = false;

  // When read-only we always use the live data passed to the function
  // FIXME-OPT: Because our selection/cursor code currently needs the wide text
  // we need to convert it when active, which is not ideal :(
  if (is_readonly && state != NULL && (render_cursor || render_selection)) {
    const char *buf_end = NULL;
    state->TextW.resize(buf_size + 1);
    state->CurLenW = TextStrFromUtf8(state->TextW.Data, state->TextW.Size, buf,
                                     NULL, &buf_end);
    state->CurLenA = (int)(buf_end - buf);
    state->CursorClamp();
    render_selection &= state->HasSelection();
  }

  // Select the buffer to render.
  const bool buf_display_from_state =
      (render_cursor || render_selection || g.ActiveId == id) && !is_readonly &&
      state && state->TextAIsValid;
  const bool is_displaying_hint =
      (hint != NULL &&
       (buf_display_from_state ? state->TextA.Data : buf)[0] == 0);

  // Password pushes a temporary font with only a fallback glyph
  if (is_password && !is_displaying_hint) {
    const FontGlyph *glyph = g.Font->FindGlyph('*');
    Font *password_font = &g.InputTextPasswordFont;
    password_font->FontSize = g.Font->FontSize;
    password_font->Scale = g.Font->Scale;
    password_font->Ascent = g.Font->Ascent;
    password_font->Descent = g.Font->Descent;
    password_font->ContainerAtlas = g.Font->ContainerAtlas;
    password_font->FallbackGlyph = glyph;
    password_font->FallbackAdvanceX = glyph->AdvanceX;
    ASSERT(password_font->Glyphs.empty() &&
           password_font->IndexAdvanceX.empty() &&
           password_font->IndexLookup.empty());
    PushFont(password_font);
  }

  // Process mouse inputs and character inputs
  int backup_current_text_length = 0;
  if (g.ActiveId == id) {
    ASSERT(state != NULL);
    backup_current_text_length = state->CurLenA;
    state->Edited = false;
    state->BufCapacityA = buf_size;
    state->Flags = flags;

    // Although we are active we don't prevent mouse from hovering other
    // elements unless we are interacting right now with the widget. Down the
    // line we should have a cleaner library-wide concept of Selected vs Active.
    g.ActiveIdAllowOverlap = !io.MouseDown[0];

    // Edit in progress
    const float mouse_x =
        (io.MousePos.x - frame_bb.Min.x - style.FramePadding.x) +
        state->ScrollX;
    const float mouse_y =
        (is_multiline ? (io.MousePos.y - draw_window->DC.CursorPos.y)
                      : (g.FontSize * 0.5f));

    if (select_all) {
      state->SelectAll();
      state->SelectedAllMouseLock = true;
    } else if (hovered && io.MouseClickedCount[0] >= 2 && !io.KeyShift) {
      textedit_click(state, &state->Stb, mouse_x, mouse_y);
      const int multiclick_count = (io.MouseClickedCount[0] - 2);
      if ((multiclick_count % 2) == 0) {
        // Double-click: Select word
        // We always use the "Mac" word advance for double-click select vs
        // CTRL+Right which use the platform dependent variant:
        // FIXME: There are likely many ways to improve this behavior, but
        // there's no "right" behavior (depends on use-case, software, OS)
        const bool is_bol =
            (state->Stb.cursor == 0) ||
            Stb::TEXTEDIT_GETCHAR(state, state->Stb.cursor - 1) == '\n';
        if (TEXT_HAS_SELECTION(&state->Stb) || !is_bol)
          state->OnKeyPressed(TEXTEDIT_K_WORDLEFT);
        // state->OnKeyPressed(TEXTEDIT_K_WORDRIGHT | TEXTEDIT_K_SHIFT);
        if (!TEXT_HAS_SELECTION(&state->Stb))
          Stb::textedit_prep_selection_at_cursor(&state->Stb);
        state->Stb.cursor =
            Stb::TEXTEDIT_MOVEWORDRIGHT_MAC(state, state->Stb.cursor);
        state->Stb.select_end = state->Stb.cursor;
        Stb::textedit_clamp(state, &state->Stb);
      } else {
        // Triple-click: Select line
        const bool is_eol =
            Stb::TEXTEDIT_GETCHAR(state, state->Stb.cursor) == '\n';
        state->OnKeyPressed(TEXTEDIT_K_LINESTART);
        state->OnKeyPressed(TEXTEDIT_K_LINEEND | TEXTEDIT_K_SHIFT);
        state->OnKeyPressed(TEXTEDIT_K_RIGHT | TEXTEDIT_K_SHIFT);
        if (!is_eol && is_multiline) {
          Swap(state->Stb.select_start, state->Stb.select_end);
          state->Stb.cursor = state->Stb.select_end;
        }
        state->CursorFollow = false;
      }
      state->CursorAnimReset();
    } else if (io.MouseClicked[0] && !state->SelectedAllMouseLock) {
      if (hovered) {
        if (io.KeyShift)
          textedit_drag(state, &state->Stb, mouse_x, mouse_y);
        else
          textedit_click(state, &state->Stb, mouse_x, mouse_y);
        state->CursorAnimReset();
      }
    } else if (io.MouseDown[0] && !state->SelectedAllMouseLock &&
               (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)) {
      textedit_drag(state, &state->Stb, mouse_x, mouse_y);
      state->CursorAnimReset();
      state->CursorFollow = true;
    }
    if (state->SelectedAllMouseLock && !io.MouseDown[0])
      state->SelectedAllMouseLock = false;

    // We expect backends to emit a Tab key but some also emit a Tab character
    // which we ignore (#2467, #1336) (For Tab and Enter: Win32/SFML/Allegro are
    // sending both keys and chars, GLFW and SDL are only sending keys. For
    // Space they all send all threes)
    if ((flags & InputTextFlags_AllowTabInput) &&
        Shortcut(Key_Tab, id, InputFlags_Repeat) && !is_readonly) {
      unsigned int c = '\t'; // Insert TAB
      if (InputTextFilterCharacter(&g, &c, flags, callback, callback_user_data,
                                   InputSource_Keyboard))
        state->OnKeyPressed((int)c);
    }

    // Process regular text input (before we check for Return because using some
    // IME will effectively send a Return?) We ignore CTRL inputs, but need to
    // allow ALT+CTRL as some keyboards (e.g. German) use AltGR (which _is_
    // Alt+Ctrl) to input certain characters.
    const bool ignore_char_inputs =
        (io.KeyCtrl && !io.KeyAlt) || (is_osx && io.KeySuper);
    if (io.InputQueueCharacters.Size > 0) {
      if (!ignore_char_inputs && !is_readonly && !input_requested_by_nav)
        for (int n = 0; n < io.InputQueueCharacters.Size; n++) {
          // Insert character if they pass filtering
          unsigned int c = (unsigned int)io.InputQueueCharacters[n];
          if (c == '\t') // Skip Tab, see above.
            continue;
          if (InputTextFilterCharacter(&g, &c, flags, callback,
                                       callback_user_data,
                                       InputSource_Keyboard))
            state->OnKeyPressed((int)c);
        }

      // Consume characters
      io.InputQueueCharacters.resize(0);
    }
  }

  // Process other shortcuts/key-presses
  bool revert_edit = false;
  if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id) {
    ASSERT(state != NULL);

    const int row_count_per_page =
        Max((int)((inner_size.y - style.FramePadding.y) / g.FontSize), 1);
    state->Stb.row_count_per_page = row_count_per_page;

    const int k_mask = (io.KeyShift ? TEXTEDIT_K_SHIFT : 0);
    const bool is_wordmove_key_down =
        is_osx ? io.KeyAlt : io.KeyCtrl; // OS X style: Text editing cursor
                                         // movement using Alt instead of Ctrl
    const bool is_startend_key_down =
        is_osx && io.KeySuper && !io.KeyCtrl &&
        !io.KeyAlt; // OS X style: Line/Text Start and End using Cmd+Arrows
                    // instead of Home/End

    // Using Shortcut() with InputFlags_RouteFocused (default policy) to
    // allow routing operations for other code (e.g. calling window trying to
    // use CTRL+A and CTRL+B: formet would be handled by InputText) Otherwise we
    // could simply assume that we own the keys as we are active.
    const InputFlags f_repeat = InputFlags_Repeat;
    const bool is_cut = (Shortcut(Mod_Shortcut | Key_X, id, f_repeat) ||
                         Shortcut(Mod_Shift | Key_Delete, id, f_repeat)) &&
                        !is_readonly && !is_password &&
                        (!is_multiline || state->HasSelection());
    const bool is_copy = (Shortcut(Mod_Shortcut | Key_C, id) ||
                          Shortcut(Mod_Ctrl | Key_Insert, id)) &&
                         !is_password &&
                         (!is_multiline || state->HasSelection());
    const bool is_paste = (Shortcut(Mod_Shortcut | Key_V, id, f_repeat) ||
                           Shortcut(Mod_Shift | Key_Insert, id, f_repeat)) &&
                          !is_readonly;
    const bool is_undo = (Shortcut(Mod_Shortcut | Key_Z, id, f_repeat)) &&
                         !is_readonly && is_undoable;
    const bool is_redo = (Shortcut(Mod_Shortcut | Key_Y, id, f_repeat) ||
                          (is_osx && Shortcut(Mod_Shortcut | Mod_Shift | Key_Z,
                                              id, f_repeat))) &&
                         !is_readonly && is_undoable;
    const bool is_select_all = Shortcut(Mod_Shortcut | Key_A, id);

    // We allow validate/cancel with Nav source (gamepad) to makes it easier to
    // undo an accidental NavInput press with no keyboard wired, but otherwise
    // it isn't very useful.
    const bool nav_gamepad_active =
        (io.ConfigFlags & ConfigFlags_NavEnableGamepad) != 0 &&
        (io.BackendFlags & BackendFlags_HasGamepad) != 0;
    const bool is_enter_pressed =
        IsKeyPressed(Key_Enter, true) || IsKeyPressed(Key_KeypadEnter, true);
    const bool is_gamepad_validate =
        nav_gamepad_active && (IsKeyPressed(Key_NavGamepadActivate, false) ||
                               IsKeyPressed(Key_NavGamepadInput, false));
    const bool is_cancel =
        Shortcut(Key_Escape, id, f_repeat) ||
        (nav_gamepad_active && Shortcut(Key_NavGamepadCancel, id, f_repeat));

    // FIXME: Should use more Shortcut() and reduce
    // IsKeyPressed()+SetKeyOwner(), but requires modifiers combination to be
    // taken account of.
    if (IsKeyPressed(Key_LeftArrow)) {
      state->OnKeyPressed((is_startend_key_down   ? TEXTEDIT_K_LINESTART
                           : is_wordmove_key_down ? TEXTEDIT_K_WORDLEFT
                                                  : TEXTEDIT_K_LEFT) |
                          k_mask);
    } else if (IsKeyPressed(Key_RightArrow)) {
      state->OnKeyPressed((is_startend_key_down   ? TEXTEDIT_K_LINEEND
                           : is_wordmove_key_down ? TEXTEDIT_K_WORDRIGHT
                                                  : TEXTEDIT_K_RIGHT) |
                          k_mask);
    } else if (IsKeyPressed(Key_UpArrow) && is_multiline) {
      if (io.KeyCtrl)
        SetScrollY(draw_window, Max(draw_window->Scroll.y - g.FontSize, 0.0f));
      else
        state->OnKeyPressed(
            (is_startend_key_down ? TEXTEDIT_K_TEXTSTART : TEXTEDIT_K_UP) |
            k_mask);
    } else if (IsKeyPressed(Key_DownArrow) && is_multiline) {
      if (io.KeyCtrl)
        SetScrollY(draw_window,
                   Min(draw_window->Scroll.y + g.FontSize, GetScrollMaxY()));
      else
        state->OnKeyPressed(
            (is_startend_key_down ? TEXTEDIT_K_TEXTEND : TEXTEDIT_K_DOWN) |
            k_mask);
    } else if (IsKeyPressed(Key_PageUp) && is_multiline) {
      state->OnKeyPressed(TEXTEDIT_K_PGUP | k_mask);
      scroll_y -= row_count_per_page * g.FontSize;
    } else if (IsKeyPressed(Key_PageDown) && is_multiline) {
      state->OnKeyPressed(TEXTEDIT_K_PGDOWN | k_mask);
      scroll_y += row_count_per_page * g.FontSize;
    } else if (IsKeyPressed(Key_Home)) {
      state->OnKeyPressed(io.KeyCtrl ? TEXTEDIT_K_TEXTSTART | k_mask
                                     : TEXTEDIT_K_LINESTART | k_mask);
    } else if (IsKeyPressed(Key_End)) {
      state->OnKeyPressed(io.KeyCtrl ? TEXTEDIT_K_TEXTEND | k_mask
                                     : TEXTEDIT_K_LINEEND | k_mask);
    } else if (IsKeyPressed(Key_Delete) && !is_readonly && !is_cut) {
      if (!state->HasSelection()) {
        // OSX doesn't seem to have Super+Delete to delete until end-of-line, so
        // we don't emulate that (as opposed to Super+Backspace)
        if (is_wordmove_key_down)
          state->OnKeyPressed(TEXTEDIT_K_WORDRIGHT | TEXTEDIT_K_SHIFT);
      }
      state->OnKeyPressed(TEXTEDIT_K_DELETE | k_mask);
    } else if (IsKeyPressed(Key_Backspace) && !is_readonly) {
      if (!state->HasSelection()) {
        if (is_wordmove_key_down)
          state->OnKeyPressed(TEXTEDIT_K_WORDLEFT | TEXTEDIT_K_SHIFT);
        else if (is_osx && io.KeySuper && !io.KeyAlt && !io.KeyCtrl)
          state->OnKeyPressed(TEXTEDIT_K_LINESTART | TEXTEDIT_K_SHIFT);
      }
      state->OnKeyPressed(TEXTEDIT_K_BACKSPACE | k_mask);
    } else if (is_enter_pressed || is_gamepad_validate) {
      // Determine if we turn Enter into a \n character
      bool ctrl_enter_for_new_line =
          (flags & InputTextFlags_CtrlEnterForNewLine) != 0;
      if (!is_multiline || is_gamepad_validate ||
          (ctrl_enter_for_new_line && !io.KeyCtrl) ||
          (!ctrl_enter_for_new_line && io.KeyCtrl)) {
        validated = true;
        if (io.ConfigInputTextEnterKeepActive && !is_multiline)
          state->SelectAll(); // No need to scroll
        else
          clear_active_id = true;
      } else if (!is_readonly) {
        unsigned int c = '\n'; // Insert new line
        if (InputTextFilterCharacter(&g, &c, flags, callback,
                                     callback_user_data, InputSource_Keyboard))
          state->OnKeyPressed((int)c);
      }
    } else if (is_cancel) {
      if (flags & InputTextFlags_EscapeClearsAll) {
        if (buf[0] != 0) {
          revert_edit = true;
        } else {
          render_cursor = render_selection = false;
          clear_active_id = true;
        }
      } else {
        clear_active_id = revert_edit = true;
        render_cursor = render_selection = false;
      }
    } else if (is_undo || is_redo) {
      state->OnKeyPressed(is_undo ? TEXTEDIT_K_UNDO : TEXTEDIT_K_REDO);
      state->ClearSelection();
    } else if (is_select_all) {
      state->SelectAll();
      state->CursorFollow = true;
    } else if (is_cut || is_copy) {
      // Cut, Copy
      if (io.SetClipboardTextFn) {
        const int ib = state->HasSelection()
                           ? Min(state->Stb.select_start, state->Stb.select_end)
                           : 0;
        const int ie = state->HasSelection()
                           ? Max(state->Stb.select_start, state->Stb.select_end)
                           : state->CurLenW;
        const int clipboard_data_len =
            TextCountUtf8BytesFromStr(state->TextW.Data + ib,
                                      state->TextW.Data + ie) +
            1;
        char *clipboard_data = (char *)ALLOC(clipboard_data_len * sizeof(char));
        TextStrToUtf8(clipboard_data, clipboard_data_len,
                      state->TextW.Data + ib, state->TextW.Data + ie);
        SetClipboardText(clipboard_data);
        MemFree(clipboard_data);
      }
      if (is_cut) {
        if (!state->HasSelection())
          state->SelectAll();
        state->CursorFollow = true;
        textedit_cut(state, &state->Stb);
      }
    } else if (is_paste) {
      if (const char *clipboard = GetClipboardText()) {
        // Filter pasted buffer
        const int clipboard_len = (int)strlen(clipboard);
        Wchar *clipboard_filtered =
            (Wchar *)ALLOC((clipboard_len + 1) * sizeof(Wchar));
        int clipboard_filtered_len = 0;
        for (const char *s = clipboard; *s != 0;) {
          unsigned int c;
          s += TextCharFromUtf8(&c, s, NULL);
          if (!InputTextFilterCharacter(&g, &c, flags, callback,
                                        callback_user_data,
                                        InputSource_Clipboard))
            continue;
          clipboard_filtered[clipboard_filtered_len++] = (Wchar)c;
        }
        clipboard_filtered[clipboard_filtered_len] = 0;
        if (clipboard_filtered_len >
            0) // If everything was filtered, ignore the pasting operation
        {
          textedit_paste(state, &state->Stb, clipboard_filtered,
                         clipboard_filtered_len);
          state->CursorFollow = true;
        }
        MemFree(clipboard_filtered);
      }
    }

    // Update render selection flag after events have been handled, so selection
    // highlight can be displayed during the same frame.
    render_selection |= state->HasSelection() &&
                        (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
  }

  // Process callbacks and apply result back to user's buffer.
  const char *apply_new_text = NULL;
  int apply_new_text_length = 0;
  if (g.ActiveId == id) {
    ASSERT(state != NULL);
    if (revert_edit && !is_readonly) {
      if (flags & InputTextFlags_EscapeClearsAll) {
        // Clear input
        ASSERT(buf[0] != 0);
        apply_new_text = "";
        apply_new_text_length = 0;
        value_changed = true;
        TEXTEDIT_CHARTYPE empty_string;
        textedit_replace(state, &state->Stb, &empty_string, 0);
      } else if (strcmp(buf, state->InitialTextA.Data) != 0) {
        // Restore initial value. Only return true if restoring to the initial
        // value changes the current buffer contents. Push records into the undo
        // stack so we can CTRL+Z the revert operation itself
        apply_new_text = state->InitialTextA.Data;
        apply_new_text_length = state->InitialTextA.Size - 1;
        value_changed = true;
        Vector<Wchar> w_text;
        if (apply_new_text_length > 0) {
          w_text.resize(
              TextCountCharsFromUtf8(apply_new_text,
                                     apply_new_text + apply_new_text_length) +
              1);
          TextStrFromUtf8(w_text.Data, w_text.Size, apply_new_text,
                          apply_new_text + apply_new_text_length);
        }
        textedit_replace(state, &state->Stb, w_text.Data,
                         (apply_new_text_length > 0) ? (w_text.Size - 1) : 0);
      }
    }

    // Apply ASCII value
    if (!is_readonly) {
      state->TextAIsValid = true;
      state->TextA.resize(state->TextW.Size * 4 + 1);
      TextStrToUtf8(state->TextA.Data, state->TextA.Size, state->TextW.Data,
                    NULL);
    }

    // When using 'InputTextFlags_EnterReturnsTrue' as a special case we
    // reapply the live buffer back to the input buffer before clearing
    // ActiveId, even though strictly speaking it wasn't modified on this frame.
    // If we didn't do that, code like InputInt() with
    // InputTextFlags_EnterReturnsTrue would fail. This also allows the
    // user to use InputText() with InputTextFlags_EnterReturnsTrue without
    // maintaining any user-side storage (please note that if you use this
    // property along InputTextFlags_CallbackResize you can end up with
    // your temporary string object unnecessarily allocating once a frame,
    // either store your string data, either if you don't then don't use
    // InputTextFlags_CallbackResize).
    const bool apply_edit_back_to_user_buffer =
        !revert_edit ||
        (validated && (flags & InputTextFlags_EnterReturnsTrue) != 0);
    if (apply_edit_back_to_user_buffer) {
      // Apply new value immediately - copy modified buffer back
      // Note that as soon as the input box is active, the in-widget value gets
      // priority over any underlying modification of the input buffer
      // FIXME: We actually always render 'buf' when calling DrawList->AddText,
      // making the comment above incorrect.
      // FIXME-OPT: CPU waste to do this every time the widget is active, should
      // mark dirty state from the textedit callbacks.

      // User callback
      if ((flags &
           (InputTextFlags_CallbackCompletion | InputTextFlags_CallbackHistory |
            InputTextFlags_CallbackEdit | InputTextFlags_CallbackAlways)) !=
          0) {
        ASSERT(callback != NULL);

        // The reason we specify the usage semantic (Completion/History) is that
        // Completion needs to disable keyboard TABBING at the moment.
        InputTextFlags event_flag = 0;
        Key event_key = Key_None;
        if ((flags & InputTextFlags_CallbackCompletion) != 0 &&
            Shortcut(Key_Tab, id)) {
          event_flag = InputTextFlags_CallbackCompletion;
          event_key = Key_Tab;
        } else if ((flags & InputTextFlags_CallbackHistory) != 0 &&
                   IsKeyPressed(Key_UpArrow)) {
          event_flag = InputTextFlags_CallbackHistory;
          event_key = Key_UpArrow;
        } else if ((flags & InputTextFlags_CallbackHistory) != 0 &&
                   IsKeyPressed(Key_DownArrow)) {
          event_flag = InputTextFlags_CallbackHistory;
          event_key = Key_DownArrow;
        } else if ((flags & InputTextFlags_CallbackEdit) && state->Edited) {
          event_flag = InputTextFlags_CallbackEdit;
        } else if (flags & InputTextFlags_CallbackAlways) {
          event_flag = InputTextFlags_CallbackAlways;
        }

        if (event_flag) {
          InputTextCallbackData callback_data;
          callback_data.Ctx = &g;
          callback_data.EventFlag = event_flag;
          callback_data.Flags = flags;
          callback_data.UserData = callback_user_data;

          char *callback_buf = is_readonly ? buf : state->TextA.Data;
          callback_data.EventKey = event_key;
          callback_data.Buf = callback_buf;
          callback_data.BufTextLen = state->CurLenA;
          callback_data.BufSize = state->BufCapacityA;
          callback_data.BufDirty = false;

          // We have to convert from wchar-positions to UTF-8-positions, which
          // can be pretty slow (an incentive to ditch the Wchar buffer, see
          // https://github.com/nothings/stb/issues/188)
          Wchar *text = state->TextW.Data;
          const int utf8_cursor_pos = callback_data.CursorPos =
              TextCountUtf8BytesFromStr(text, text + state->Stb.cursor);
          const int utf8_selection_start = callback_data.SelectionStart =
              TextCountUtf8BytesFromStr(text, text + state->Stb.select_start);
          const int utf8_selection_end = callback_data.SelectionEnd =
              TextCountUtf8BytesFromStr(text, text + state->Stb.select_end);

          // Call user code
          callback(&callback_data);

          // Read back what user may have modified
          callback_buf =
              is_readonly
                  ? buf
                  : state->TextA.Data; // Pointer may have been invalidated by a
                                       // resize callback
          ASSERT(callback_data.Buf ==
                 callback_buf); // Invalid to modify those fields
          ASSERT(callback_data.BufSize == state->BufCapacityA);
          ASSERT(callback_data.Flags == flags);
          const bool buf_dirty = callback_data.BufDirty;
          if (callback_data.CursorPos != utf8_cursor_pos || buf_dirty) {
            state->Stb.cursor = TextCountCharsFromUtf8(
                callback_data.Buf, callback_data.Buf + callback_data.CursorPos);
            state->CursorFollow = true;
          }
          if (callback_data.SelectionStart != utf8_selection_start ||
              buf_dirty) {
            state->Stb.select_start =
                (callback_data.SelectionStart == callback_data.CursorPos)
                    ? state->Stb.cursor
                    : TextCountCharsFromUtf8(callback_data.Buf,
                                             callback_data.Buf +
                                                 callback_data.SelectionStart);
          }
          if (callback_data.SelectionEnd != utf8_selection_end || buf_dirty) {
            state->Stb.select_end =
                (callback_data.SelectionEnd == callback_data.SelectionStart)
                    ? state->Stb.select_start
                    : TextCountCharsFromUtf8(callback_data.Buf,
                                             callback_data.Buf +
                                                 callback_data.SelectionEnd);
          }
          if (buf_dirty) {
            ASSERT(!is_readonly);
            ASSERT(callback_data.BufTextLen ==
                   (int)strlen(
                       callback_data.Buf)); // You need to maintain BufTextLen
                                            // if you change the text!
            InputTextReconcileUndoStateAfterUserCallback(
                state, callback_data.Buf,
                callback_data
                    .BufTextLen); // FIXME: Move the rest of this block
                                  // inside function and rename to
                                  // InputTextReconcileStateAfterUserCallback()
                                  // ?
            if (callback_data.BufTextLen > backup_current_text_length &&
                is_resizable)
              state->TextW.resize(
                  state->TextW.Size +
                  (callback_data.BufTextLen -
                   backup_current_text_length)); // Worse case scenario resize
            state->CurLenW = TextStrFromUtf8(
                state->TextW.Data, state->TextW.Size, callback_data.Buf, NULL);
            state->CurLenA =
                callback_data
                    .BufTextLen; // Assume correct length and valid UTF-8 from
                                 // user, saves us an extra strlen()
            state->CursorAnimReset();
          }
        }
      }

      // Will copy result string if modified
      if (!is_readonly && strcmp(state->TextA.Data, buf) != 0) {
        apply_new_text = state->TextA.Data;
        apply_new_text_length = state->CurLenA;
        value_changed = true;
      }
    }
  }

  // Handle reapplying final data on deactivation (see InputTextDeactivateHook()
  // for details)
  if (g.InputTextDeactivatedState.ID == id) {
    if (g.ActiveId != id && IsItemDeactivatedAfterEdit() && !is_readonly &&
        strcmp(g.InputTextDeactivatedState.TextA.Data, buf) != 0) {
      apply_new_text = g.InputTextDeactivatedState.TextA.Data;
      apply_new_text_length = g.InputTextDeactivatedState.TextA.Size - 1;
      value_changed = true;
      // DEBUG_LOG("InputText(): apply Deactivated data for 0x%08X:
      // \"%.*s\".\n", id, apply_new_text_length, apply_new_text);
    }
    g.InputTextDeactivatedState.ID = 0;
  }

  // Copy result to user buffer. This can currently only happen when (g.ActiveId
  // == id)
  if (apply_new_text != NULL) {
    // We cannot test for 'backup_current_text_length != apply_new_text_length'
    // here because we have no guarantee that the size of our owned buffer
    // matches the size of the string object held by the user, and by design we
    // allow InputText() to be used without any storage on user's side.
    ASSERT(apply_new_text_length >= 0);
    if (is_resizable) {
      InputTextCallbackData callback_data;
      callback_data.Ctx = &g;
      callback_data.EventFlag = InputTextFlags_CallbackResize;
      callback_data.Flags = flags;
      callback_data.Buf = buf;
      callback_data.BufTextLen = apply_new_text_length;
      callback_data.BufSize = Max(buf_size, apply_new_text_length + 1);
      callback_data.UserData = callback_user_data;
      callback(&callback_data);
      buf = callback_data.Buf;
      buf_size = callback_data.BufSize;
      apply_new_text_length = Min(callback_data.BufTextLen, buf_size - 1);
      ASSERT(apply_new_text_length <= buf_size);
    }
    // DEBUG_PRINT("InputText(\"%s\"): apply_new_text length %d\n", label,
    // apply_new_text_length);

    // If the underlying buffer resize was denied or not carried to the next
    // frame, apply_new_text_length+1 may be >= buf_size.
    Strncpy(buf, apply_new_text, Min(apply_new_text_length + 1, buf_size));
  }

  // Release active ID at the end of the function (so e.g. pressing Return still
  // does a final application of the value) Otherwise request text input ahead
  // for next frame.
  if (g.ActiveId == id && clear_active_id)
    ClearActiveID();
  else if (g.ActiveId == id)
    g.WantTextInputNextFrame = 1;

  // Render frame
  if (!is_multiline) {
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(Col_FrameBg), true,
                style.FrameRounding);
  }

  const Vec4 clip_rect(
      frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + inner_size.x,
      frame_bb.Min.y +
          inner_size.y); // Not using frame_bb.Max because we have adjusted size
  Vec2 draw_pos = is_multiline ? draw_window->DC.CursorPos
                               : frame_bb.Min + style.FramePadding;
  Vec2 text_size(0.0f, 0.0f);

  // Set upper limit of single-line InputTextEx() at 2 million characters
  // strings. The current pathological worst case is a long line without any
  // carriage return, which would makes Font::RenderText() reserve too many
  // vertices and probably crash. Avoid it altogether. Note that we only use
  // this limit on single-line InputText(), so a pathologically large line on a
  // InputTextMultiline() would still crash.
  const int buf_display_max_length = 2 * 1024 * 1024;
  const char *buf_display =
      buf_display_from_state ? state->TextA.Data : buf; //-V595
  const char *buf_display_end =
      NULL; // We have specialized paths below for setting the length
  if (is_displaying_hint) {
    buf_display = hint;
    buf_display_end = hint + strlen(hint);
  }

  // Render text. We currently only render selection when the widget is active
  // or while scrolling.
  // FIXME: We could remove the '&& render_cursor' to keep rendering selection
  // when inactive.
  if (render_cursor || render_selection) {
    ASSERT(state != NULL);
    if (!is_displaying_hint)
      buf_display_end = buf_display + state->CurLenA;

    // Render text (with cursor and selection)
    // This is going to be messy. We need to:
    // - Display the text (this alone can be more easily clipped)
    // - Handle scrolling, highlight selection, display cursor (those all
    // requires some form of 1d->2d cursor position calculation)
    // - Measure text height (for scrollbar)
    // We are attempting to do most of that in **one main pass** to minimize the
    // computation cost (non-negligible for large amount of text) + 2nd pass for
    // selection rendering (we could merge them by an extra refactoring effort)
    // FIXME: This should occur on buf_display but we'd need to maintain
    // cursor/select_start/select_end for UTF-8.
    const Wchar *text_begin = state->TextW.Data;
    Vec2 cursor_offset, select_start_offset;

    {
      // Find lines numbers straddling 'cursor' (slot 0) and 'select_start'
      // (slot 1) positions.
      const Wchar *searches_input_ptr[2] = {NULL, NULL};
      int searches_result_line_no[2] = {-1000, -1000};
      int searches_remaining = 0;
      if (render_cursor) {
        searches_input_ptr[0] = text_begin + state->Stb.cursor;
        searches_result_line_no[0] = -1;
        searches_remaining++;
      }
      if (render_selection) {
        searches_input_ptr[1] =
            text_begin + Min(state->Stb.select_start, state->Stb.select_end);
        searches_result_line_no[1] = -1;
        searches_remaining++;
      }

      // Iterate all lines to find our line numbers
      // In multi-line mode, we never exit the loop until all lines are counted,
      // so add one extra to the searches_remaining counter.
      searches_remaining += is_multiline ? 1 : 0;
      int line_count = 0;
      // for (const Wchar* s = text_begin; (s = (const Wchar*)wcschr((const
      // wchar_t*)s, (wchar_t)'\n')) != NULL; s++)  // FIXME-OPT: Could use this
      // when wchar_t are 16-bit
      for (const Wchar *s = text_begin; *s != 0; s++)
        if (*s == '\n') {
          line_count++;
          if (searches_result_line_no[0] == -1 && s >= searches_input_ptr[0]) {
            searches_result_line_no[0] = line_count;
            if (--searches_remaining <= 0)
              break;
          }
          if (searches_result_line_no[1] == -1 && s >= searches_input_ptr[1]) {
            searches_result_line_no[1] = line_count;
            if (--searches_remaining <= 0)
              break;
          }
        }
      line_count++;
      if (searches_result_line_no[0] == -1)
        searches_result_line_no[0] = line_count;
      if (searches_result_line_no[1] == -1)
        searches_result_line_no[1] = line_count;

      // Calculate 2d position by finding the beginning of the line and
      // measuring distance
      cursor_offset.x =
          InputTextCalcTextSizeW(&g, StrbolW(searches_input_ptr[0], text_begin),
                                 searches_input_ptr[0])
              .x;
      cursor_offset.y = searches_result_line_no[0] * g.FontSize;
      if (searches_result_line_no[1] >= 0) {
        select_start_offset.x =
            InputTextCalcTextSizeW(&g,
                                   StrbolW(searches_input_ptr[1], text_begin),
                                   searches_input_ptr[1])
                .x;
        select_start_offset.y = searches_result_line_no[1] * g.FontSize;
      }

      // Store text height (note that we haven't calculated text width at all,
      // see GitHub issues #383, #1224)
      if (is_multiline)
        text_size = Vec2(inner_size.x, line_count * g.FontSize);
    }

    // Scroll
    if (render_cursor && state->CursorFollow) {
      // Horizontal scroll in chunks of quarter width
      if (!(flags & InputTextFlags_NoHorizontalScroll)) {
        const float scroll_increment_x = inner_size.x * 0.25f;
        const float visible_width = inner_size.x - style.FramePadding.x;
        if (cursor_offset.x < state->ScrollX)
          state->ScrollX =
              TRUNC(Max(0.0f, cursor_offset.x - scroll_increment_x));
        else if (cursor_offset.x - visible_width >= state->ScrollX)
          state->ScrollX =
              TRUNC(cursor_offset.x - visible_width + scroll_increment_x);
      } else {
        state->ScrollX = 0.0f;
      }

      // Vertical scroll
      if (is_multiline) {
        // Test if cursor is vertically visible
        if (cursor_offset.y - g.FontSize < scroll_y)
          scroll_y = Max(0.0f, cursor_offset.y - g.FontSize);
        else if (cursor_offset.y -
                     (inner_size.y - style.FramePadding.y * 2.0f) >=
                 scroll_y)
          scroll_y =
              cursor_offset.y - inner_size.y + style.FramePadding.y * 2.0f;
        const float scroll_max_y = Max(
            (text_size.y + style.FramePadding.y * 2.0f) - inner_size.y, 0.0f);
        scroll_y = Clamp(scroll_y, 0.0f, scroll_max_y);
        draw_pos.y += (draw_window->Scroll.y -
                       scroll_y); // Manipulate cursor pos immediately avoid a
                                  // frame of lag
        draw_window->Scroll.y = scroll_y;
      }

      state->CursorFollow = false;
    }

    // Draw selection
    const Vec2 draw_scroll = Vec2(state->ScrollX, 0.0f);
    if (render_selection) {
      const Wchar *text_selected_begin =
          text_begin + Min(state->Stb.select_start, state->Stb.select_end);
      const Wchar *text_selected_end =
          text_begin + Max(state->Stb.select_start, state->Stb.select_end);

      U32 bg_color = GetColorU32(
          Col_TextSelectedBg,
          render_cursor ? 1.0f
                        : 0.6f); // FIXME: current code flow mandate that
                                 // render_cursor is always true here, we are
                                 // leaving the transparent one for tests.
      float bg_offy_up =
          is_multiline
              ? 0.0f
              : -1.0f; // FIXME: those offsets should be part of the style? they
                       // don't play so well with multi-line selection.
      float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
      Vec2 rect_pos = draw_pos + select_start_offset - draw_scroll;
      for (const Wchar *p = text_selected_begin; p < text_selected_end;) {
        if (rect_pos.y > clip_rect.w + g.FontSize)
          break;
        if (rect_pos.y < clip_rect.y) {
          // p = (const Wchar*)wmemchr((const wchar_t*)p, '\n',
          // text_selected_end - p);  // FIXME-OPT: Could use this when wchar_t
          // are 16-bit p = p ? p + 1 : text_selected_end;
          while (p < text_selected_end)
            if (*p++ == '\n')
              break;
        } else {
          Vec2 rect_size =
              InputTextCalcTextSizeW(&g, p, text_selected_end, &p, NULL, true);
          if (rect_size.x <= 0.0f)
            rect_size.x = TRUNC(g.Font->GetCharAdvance((Wchar)' ') *
                                0.50f); // So we can see selected empty lines
          Rect rect(rect_pos + Vec2(0.0f, bg_offy_up - g.FontSize),
                    rect_pos + Vec2(rect_size.x, bg_offy_dn));
          rect.ClipWith(clip_rect);
          if (rect.Overlaps(clip_rect))
            draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
        }
        rect_pos.x = draw_pos.x - draw_scroll.x;
        rect_pos.y += g.FontSize;
      }
    }

    // We test for 'buf_display_max_length' as a way to avoid some pathological
    // cases (e.g. single-line 1 MB string) which would make DrawList crash.
    if (is_multiline ||
        (buf_display_end - buf_display) < buf_display_max_length) {
      U32 col = GetColorU32(is_displaying_hint ? Col_TextDisabled : Col_Text);
      draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos - draw_scroll,
                                     col, buf_display, buf_display_end, 0.0f,
                                     is_multiline ? NULL : &clip_rect);
    }

    // Draw blinking cursor
    if (render_cursor) {
      state->CursorAnim += io.DeltaTime;
      bool cursor_is_visible = (!g.IO.ConfigInputTextCursorBlink) ||
                               (state->CursorAnim <= 0.0f) ||
                               Fmod(state->CursorAnim, 1.20f) <= 0.80f;
      Vec2 cursor_screen_pos = Trunc(draw_pos + cursor_offset - draw_scroll);
      Rect cursor_screen_rect(
          cursor_screen_pos.x, cursor_screen_pos.y - g.FontSize + 0.5f,
          cursor_screen_pos.x + 1.0f, cursor_screen_pos.y - 1.5f);
      if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
        draw_window->DrawList->AddLine(cursor_screen_rect.Min,
                                       cursor_screen_rect.GetBL(),
                                       GetColorU32(Col_Text));

      // Notify OS of text input position for advanced IME (-1 x offset so that
      // Windows IME can cover our cursor. Bit of an extra nicety.)
      if (!is_readonly) {
        g.PlatformImeData.WantVisible = true;
        g.PlatformImeData.InputPos =
            Vec2(cursor_screen_pos.x - 1.0f, cursor_screen_pos.y - g.FontSize);
        g.PlatformImeData.InputLineHeight = g.FontSize;
        g.PlatformImeViewport = window->Viewport->ID;
      }
    }
  } else {
    // Render text only (no selection, no cursor)
    if (is_multiline)
      text_size =
          Vec2(inner_size.x,
               InputTextCalcTextLenAndLineCount(buf_display, &buf_display_end) *
                   g.FontSize); // We don't need width
    else if (!is_displaying_hint && g.ActiveId == id)
      buf_display_end = buf_display + state->CurLenA;
    else if (!is_displaying_hint)
      buf_display_end = buf_display + strlen(buf_display);

    if (is_multiline ||
        (buf_display_end - buf_display) < buf_display_max_length) {
      U32 col = GetColorU32(is_displaying_hint ? Col_TextDisabled : Col_Text);
      draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos, col,
                                     buf_display, buf_display_end, 0.0f,
                                     is_multiline ? NULL : &clip_rect);
    }
  }

  if (is_password && !is_displaying_hint)
    PopFont();

  if (is_multiline) {
    // For focus requests to work on our multiline we need to ensure our child
    // ItemAdd() call specifies the ItemFlags_Inputable (ref issue
    // #4761)...
    Dummy(Vec2(text_size.x, text_size.y + style.FramePadding.y));
    g.NextItemData.ItemFlags |= ItemFlags_Inputable | ItemFlags_NoTabStop;
    EndChild();
    item_data_backup.StatusFlags |=
        (g.LastItemData.StatusFlags & ItemStatusFlags_HoveredWindow);

    // ...and then we need to undo the group overriding last item data, which
    // gets a bit messy as EndGroup() tries to forward scrollbar being active...
    // FIXME: This quite messy/tricky, should attempt to get rid of the child
    // window.
    EndGroup();
    if (g.LastItemData.ID == 0) {
      g.LastItemData.ID = id;
      g.LastItemData.InFlags = item_data_backup.InFlags;
      g.LastItemData.StatusFlags = item_data_backup.StatusFlags;
    }
  }

  // Log as text
  if (g.LogEnabled && (!is_password || is_displaying_hint)) {
    LogSetNextTextDecoration("{", "}");
    LogRenderedText(&draw_pos, buf_display, buf_display_end);
  }

  if (label_size.x > 0)
    RenderText(Vec2(frame_bb.Max.x + style.ItemInnerSpacing.x,
                    frame_bb.Min.y + style.FramePadding.y),
               label);

  if (value_changed && !(flags & InputTextFlags_NoMarkEdited))
    MarkItemEdited(id);

  TEST_ENGINE_ITEM_INFO(id, label,
                        g.LastItemData.StatusFlags | ItemStatusFlags_Inputable);
  if ((flags & InputTextFlags_EnterReturnsTrue) != 0)
    return validated;
  else
    return value_changed;
}

void Gui::DebugNodeInputTextState(InputTextState *state) {
#ifndef DISABLE_DEBUG_TOOLS
  Context &g = *GGui;
  Stb::TexteditState *state = &state->Stb;
  Stb::StbUndoState *undo_state = &state->undostate;
  Text("ID: 0x%08X, ActiveID: 0x%08X", state->ID, g.ActiveId);
  DebugLocateItemOnHover(state->ID);
  Text("CurLenW: %d, CurLenA: %d, Cursor: %d, Selection: %d..%d",
       state->CurLenW, state->CurLenA, state->cursor, state->select_start,
       state->select_end);
  Text("has_preferred_x: %d (%.2f)", state->has_preferred_x,
       state->preferred_x);
  Text("undo_point: %d, redo_point: %d, undo_char_point: %d, redo_char_point: "
       "%d",
       undo_state->undo_point, undo_state->redo_point,
       undo_state->undo_char_point, undo_state->redo_char_point);
  if (BeginChild("undopoints", Vec2(0.0f, GetTextLineHeight() * 15),
                 ChildFlags_Border)) // Visualize undo state
  {
    PushStyleVar(StyleVar_ItemSpacing, Vec2(0, 0));
    for (int n = 0; n < TEXTEDIT_UNDOSTATECOUNT; n++) {
      Stb::StbUndoRecord *undo_rec = &undo_state->undo_rec[n];
      const char undo_rec_type = (n < undo_state->undo_point)    ? 'u'
                                 : (n >= undo_state->redo_point) ? 'r'
                                                                 : ' ';
      if (undo_rec_type == ' ')
        BeginDisabled();
      char buf[64] = "";
      if (undo_rec_type != ' ' && undo_rec->char_storage != -1)
        TextStrToUtf8(buf, ARRAYSIZE(buf),
                      undo_state->undo_char + undo_rec->char_storage,
                      undo_state->undo_char + undo_rec->char_storage +
                          undo_rec->insert_length);
      Text("%c [%02d] where %03d, insert %03d, delete %03d, char_storage %03d "
           "\"%s\"",
           undo_rec_type, n, undo_rec->where, undo_rec->insert_length,
           undo_rec->delete_length, undo_rec->char_storage, buf);
      if (undo_rec_type == ' ')
        EndDisabled();
    }
    PopStyleVar();
  }
  EndChild();
#else
  UNUSED(state);
#endif
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
//-------------------------------------------------------------------------
// - ColorEdit3()
// - ColorEdit4()
// - ColorPicker3()
// - RenderColorRectWithAlphaCheckerboard() [Internal]
// - ColorPicker4()
// - ColorButton()
// - SetColorEditOptions()
// - ColorTooltip() [Internal]
// - ColorEditOptionsPopup() [Internal]
// - ColorPickerOptionsPopup() [Internal]
//-------------------------------------------------------------------------

bool Gui::ColorEdit3(const char *label, float col[3], ColorEditFlags flags) {
  return ColorEdit4(label, col, flags | ColorEditFlags_NoAlpha);
}

static void ColorEditRestoreH(const float *col, float *H) {
  Context &g = *GGui;
  ASSERT(g.ColorEditCurrentID != 0);
  if (g.ColorEditSavedID != g.ColorEditCurrentID ||
      g.ColorEditSavedColor !=
          Gui::ColorConvertFloat4ToU32(Vec4(col[0], col[1], col[2], 0)))
    return;
  *H = g.ColorEditSavedHue;
}

// ColorEdit supports RGB and HSV inputs. In case of RGB input resulting color
// may have undefined hue and/or saturation. Since widget displays both RGB and
// HSV values we must preserve hue and saturation to prevent these values
// resetting.
static void ColorEditRestoreHS(const float *col, float *H, float *S, float *V) {
  Context &g = *GGui;
  ASSERT(g.ColorEditCurrentID != 0);
  if (g.ColorEditSavedID != g.ColorEditCurrentID ||
      g.ColorEditSavedColor !=
          Gui::ColorConvertFloat4ToU32(Vec4(col[0], col[1], col[2], 0)))
    return;

  // When S == 0, H is undefined.
  // When H == 1 it wraps around to 0.
  if (*S == 0.0f || (*H == 0.0f && g.ColorEditSavedHue == 1))
    *H = g.ColorEditSavedHue;

  // When V == 0, S is undefined.
  if (*V == 0.0f)
    *S = g.ColorEditSavedSat;
}

// Edit colors components (each component in 0.0f..1.0f range).
// See enum ColorEditFlags_ for available options. e.g. Only access 3
// floats if ColorEditFlags_NoAlpha flag is set. With typical options:
// Left-click on color square to open color picker. Right-click to open option
// menu. CTRL-Click over input fields to edit them and TAB to go to next item.
bool Gui::ColorEdit4(const char *label, float col[4], ColorEditFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const float square_sz = GetFrameHeight();
  const char *label_display_end = FindRenderedTextEnd(label);
  float w_full = CalcItemWidth();
  g.NextItemData.ClearFlags();

  BeginGroup();
  PushID(label);
  const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
  if (set_current_color_edit_id)
    g.ColorEditCurrentID = window->IDStack.back();

  // If we're not showing any slider there's no point in doing any HSV
  // conversions
  const ColorEditFlags flags_untouched = flags;
  if (flags & ColorEditFlags_NoInputs)
    flags = (flags & (~ColorEditFlags_DisplayMask_)) |
            ColorEditFlags_DisplayRGB | ColorEditFlags_NoOptions;

  // Context menu: display and modify options (before defaults are applied)
  if (!(flags & ColorEditFlags_NoOptions))
    ColorEditOptionsPopup(col, flags);

  // Read stored options
  if (!(flags & ColorEditFlags_DisplayMask_))
    flags |= (g.ColorEditOptions & ColorEditFlags_DisplayMask_);
  if (!(flags & ColorEditFlags_DataTypeMask_))
    flags |= (g.ColorEditOptions & ColorEditFlags_DataTypeMask_);
  if (!(flags & ColorEditFlags_PickerMask_))
    flags |= (g.ColorEditOptions & ColorEditFlags_PickerMask_);
  if (!(flags & ColorEditFlags_InputMask_))
    flags |= (g.ColorEditOptions & ColorEditFlags_InputMask_);
  flags |= (g.ColorEditOptions &
            ~(ColorEditFlags_DisplayMask_ | ColorEditFlags_DataTypeMask_ |
              ColorEditFlags_PickerMask_ | ColorEditFlags_InputMask_));
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_DisplayMask_)); // Check that only 1 is selected
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_InputMask_)); // Check that only 1 is selected

  const bool alpha = (flags & ColorEditFlags_NoAlpha) == 0;
  const bool hdr = (flags & ColorEditFlags_HDR) != 0;
  const int components = alpha ? 4 : 3;
  const float w_button = (flags & ColorEditFlags_NoSmallPreview)
                             ? 0.0f
                             : (square_sz + style.ItemInnerSpacing.x);
  const float w_inputs = Max(w_full - w_button, 1.0f);
  w_full = w_inputs + w_button;

  // Convert to the formats we need
  float f[4] = {col[0], col[1], col[2], alpha ? col[3] : 1.0f};
  if ((flags & ColorEditFlags_InputHSV) && (flags & ColorEditFlags_DisplayRGB))
    ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
  else if ((flags & ColorEditFlags_InputRGB) &&
           (flags & ColorEditFlags_DisplayHSV)) {
    // Hue is lost when converting from grayscale rgb (saturation=0). Restore
    // it.
    ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
    ColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
  }
  int i[4] = {F32_TO_INT8_UNBOUND(f[0]), F32_TO_INT8_UNBOUND(f[1]),
              F32_TO_INT8_UNBOUND(f[2]), F32_TO_INT8_UNBOUND(f[3])};

  bool value_changed = false;
  bool value_changed_as_float = false;

  const Vec2 pos = window->DC.CursorPos;
  const float inputs_offset_x =
      (style.ColorButtonPosition == Dir_Left) ? w_button : 0.0f;
  window->DC.CursorPos.x = pos.x + inputs_offset_x;

  if ((flags & (ColorEditFlags_DisplayRGB | ColorEditFlags_DisplayHSV)) != 0 &&
      (flags & ColorEditFlags_NoInputs) == 0) {
    // RGB/HSV 0..255 Sliders
    const float w_items =
        w_inputs - style.ItemInnerSpacing.x * (components - 1);

    const bool hide_prefix =
        (TRUNC(w_items / components) <=
         CalcTextSize((flags & ColorEditFlags_Float) ? "M:0.000" : "M:000").x);
    static const char *ids[4] = {"##X", "##Y", "##Z", "##W"};
    static const char *fmt_table_int[3][4] = {
        {"%3d", "%3d", "%3d", "%3d"},         // Short display
        {"R:%3d", "G:%3d", "B:%3d", "A:%3d"}, // Long display for RGBA
        {"H:%3d", "S:%3d", "V:%3d", "A:%3d"}  // Long display for HSVA
    };
    static const char *fmt_table_float[3][4] = {
        {"%0.3f", "%0.3f", "%0.3f", "%0.3f"},         // Short display
        {"R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f"}, // Long display for RGBA
        {"H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f"}  // Long display for HSVA
    };
    const int fmt_idx = hide_prefix                           ? 0
                        : (flags & ColorEditFlags_DisplayHSV) ? 2
                                                              : 1;

    float prev_split = 0.0f;
    for (int n = 0; n < components; n++) {
      if (n > 0)
        SameLine(0, style.ItemInnerSpacing.x);
      float next_split = TRUNC(w_items * (n + 1) / components);
      SetNextItemWidth(Max(next_split - prev_split, 1.0f));
      prev_split = next_split;

      // FIXME: When ColorEditFlags_HDR flag is passed HS values snap in
      // weird ways when SV values go below 0.
      if (flags & ColorEditFlags_Float) {
        value_changed |=
            DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f,
                      fmt_table_float[fmt_idx][n]);
        value_changed_as_float |= value_changed;
      } else {
        value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255,
                                 fmt_table_int[fmt_idx][n]);
      }
      if (!(flags & ColorEditFlags_NoOptions))
        OpenPopupOnItemClick("context", PopupFlags_MouseButtonRight);
    }
  } else if ((flags & ColorEditFlags_DisplayHex) != 0 &&
             (flags & ColorEditFlags_NoInputs) == 0) {
    // RGB Hexadecimal Input
    char buf[64];
    if (alpha)
      FormatString(buf, ARRAYSIZE(buf), "#%02X%02X%02X%02X",
                   Clamp(i[0], 0, 255), Clamp(i[1], 0, 255),
                   Clamp(i[2], 0, 255), Clamp(i[3], 0, 255));
    else
      FormatString(buf, ARRAYSIZE(buf), "#%02X%02X%02X", Clamp(i[0], 0, 255),
                   Clamp(i[1], 0, 255), Clamp(i[2], 0, 255));
    SetNextItemWidth(w_inputs);
    if (InputText("##Text", buf, ARRAYSIZE(buf),
                  InputTextFlags_CharsUppercase)) {
      value_changed = true;
      char *p = buf;
      while (*p == '#' || CharIsBlankA(*p))
        p++;
      i[0] = i[1] = i[2] = 0;
      i[3] = 0xFF; // alpha default to 255 is not parsed by scanf (e.g.
                   // inputting #FFFFFF omitting alpha)
      int r;
      if (alpha)
        r = sscanf(p, "%02X%02X%02X%02X", (unsigned int *)&i[0],
                   (unsigned int *)&i[1], (unsigned int *)&i[2],
                   (unsigned int *)&i[3]); // Treat at unsigned (%X is unsigned)
      else
        r = sscanf(p, "%02X%02X%02X", (unsigned int *)&i[0],
                   (unsigned int *)&i[1], (unsigned int *)&i[2]);
      UNUSED(r); // Fixes C6031: Return value ignored: 'sscanf'.
    }
    if (!(flags & ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context", PopupFlags_MouseButtonRight);
  }

  Window *picker_active_window = NULL;
  if (!(flags & ColorEditFlags_NoSmallPreview)) {
    const float button_offset_x = ((flags & ColorEditFlags_NoInputs) ||
                                   (style.ColorButtonPosition == Dir_Left))
                                      ? 0.0f
                                      : w_inputs + style.ItemInnerSpacing.x;
    window->DC.CursorPos = Vec2(pos.x + button_offset_x, pos.y);

    const Vec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
    if (ColorButton("##ColorButton", col_v4, flags)) {
      if (!(flags & ColorEditFlags_NoPicker)) {
        // Store current color and open a picker
        g.ColorPickerRef = col_v4;
        OpenPopup("picker");
        SetNextWindowPos(g.LastItemData.Rect.GetBL() +
                         Vec2(0.0f, style.ItemSpacing.y));
      }
    }
    if (!(flags & ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context", PopupFlags_MouseButtonRight);

    if (BeginPopup("picker")) {
      if (g.CurrentWindow->BeginCount == 1) {
        picker_active_window = g.CurrentWindow;
        if (label != label_display_end) {
          TextEx(label, label_display_end);
          Spacing();
        }
        ColorEditFlags picker_flags_to_forward =
            ColorEditFlags_DataTypeMask_ | ColorEditFlags_PickerMask_ |
            ColorEditFlags_InputMask_ | ColorEditFlags_HDR |
            ColorEditFlags_NoAlpha | ColorEditFlags_AlphaBar;
        ColorEditFlags picker_flags =
            (flags_untouched & picker_flags_to_forward) |
            ColorEditFlags_DisplayMask_ | ColorEditFlags_NoLabel |
            ColorEditFlags_AlphaPreviewHalf;
        SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
        value_changed |=
            ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
      }
      EndPopup();
    }
  }

  if (label != label_display_end && !(flags & ColorEditFlags_NoLabel)) {
    // Position not necessarily next to last submitted button (e.g. if
    // style.ColorButtonPosition == Dir_Left), but we need to use
    // SameLine() to setup baseline correctly. Might want to refactor SameLine()
    // to simplify this.
    SameLine(0.0f, style.ItemInnerSpacing.x);
    window->DC.CursorPos.x = pos.x + ((flags & ColorEditFlags_NoInputs)
                                          ? w_button
                                          : w_full + style.ItemInnerSpacing.x);
    TextEx(label, label_display_end);
  }

  // Convert back
  if (value_changed && picker_active_window == NULL) {
    if (!value_changed_as_float)
      for (int n = 0; n < 4; n++)
        f[n] = i[n] / 255.0f;
    if ((flags & ColorEditFlags_DisplayHSV) &&
        (flags & ColorEditFlags_InputRGB)) {
      g.ColorEditSavedHue = f[0];
      g.ColorEditSavedSat = f[1];
      ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
      g.ColorEditSavedID = g.ColorEditCurrentID;
      g.ColorEditSavedColor =
          ColorConvertFloat4ToU32(Vec4(f[0], f[1], f[2], 0));
    }
    if ((flags & ColorEditFlags_DisplayRGB) &&
        (flags & ColorEditFlags_InputHSV))
      ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

    col[0] = f[0];
    col[1] = f[1];
    col[2] = f[2];
    if (alpha)
      col[3] = f[3];
  }

  if (set_current_color_edit_id)
    g.ColorEditCurrentID = 0;
  PopID();
  EndGroup();

  // Drag and Drop Target
  // NB: The flag test is merely an optional micro-optimization,
  // BeginDragDropTarget() does the same test.
  if ((g.LastItemData.StatusFlags & ItemStatusFlags_HoveredRect) &&
      !(g.LastItemData.InFlags & ItemFlags_ReadOnly) &&
      !(flags & ColorEditFlags_NoDragDrop) && BeginDragDropTarget()) {
    bool accepted_drag_drop = false;
    if (const Payload *payload = AcceptDragDropPayload(PAYLOAD_TYPE_COLOR_3F)) {
      memcpy((float *)col, payload->Data,
             sizeof(float) * 3); // Preserve alpha if any //-V512 //-V1086
      value_changed = accepted_drag_drop = true;
    }
    if (const Payload *payload = AcceptDragDropPayload(PAYLOAD_TYPE_COLOR_4F)) {
      memcpy((float *)col, payload->Data, sizeof(float) * components);
      value_changed = accepted_drag_drop = true;
    }

    // Drag-drop payloads are always RGB
    if (accepted_drag_drop && (flags & ColorEditFlags_InputHSV))
      ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
    EndDragDropTarget();
  }

  // When picker is being actively used, use its active id so IsItemActive()
  // will function on ColorEdit4().
  if (picker_active_window && g.ActiveId != 0 &&
      g.ActiveIdWindow == picker_active_window)
    g.LastItemData.ID = g.ActiveId;

  if (value_changed &&
      g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup()
                              // won't catch g.ActiveId
    MarkItemEdited(g.LastItemData.ID);

  return value_changed;
}

bool Gui::ColorPicker3(const char *label, float col[3], ColorEditFlags flags) {
  float col4[4] = {col[0], col[1], col[2], 1.0f};
  if (!ColorPicker4(label, col4, flags | ColorEditFlags_NoAlpha))
    return false;
  col[0] = col4[0];
  col[1] = col4[1];
  col[2] = col4[2];
  return true;
}

// Helper for ColorPicker4()
static void RenderArrowsForVerticalBar(DrawList *draw_list, Vec2 pos,
                                       Vec2 half_sz, float bar_w, float alpha) {
  U32 alpha8 = F32_TO_INT8_SAT(alpha);
  Gui::RenderArrowPointingAt(draw_list, Vec2(pos.x + half_sz.x + 1, pos.y),
                             Vec2(half_sz.x + 2, half_sz.y + 1), Dir_Right,
                             COL32(0, 0, 0, alpha8));
  Gui::RenderArrowPointingAt(draw_list, Vec2(pos.x + half_sz.x, pos.y), half_sz,
                             Dir_Right, COL32(255, 255, 255, alpha8));
  Gui::RenderArrowPointingAt(
      draw_list, Vec2(pos.x + bar_w - half_sz.x - 1, pos.y),
      Vec2(half_sz.x + 2, half_sz.y + 1), Dir_Left, COL32(0, 0, 0, alpha8));
  Gui::RenderArrowPointingAt(draw_list, Vec2(pos.x + bar_w - half_sz.x, pos.y),
                             half_sz, Dir_Left, COL32(255, 255, 255, alpha8));
}

// Note: ColorPicker4() only accesses 3 floats if ColorEditFlags_NoAlpha
// flag is set. (In C++ the 'float col[4]' notation for a function argument is
// equivalent to 'float* col', we only specify a size to facilitate
// understanding of the code.)
// FIXME: we adjust the big color square height based on item width, which may
// cause a flickering feedback loop (if automatic height makes a vertical
// scrollbar appears, affecting automatic width..)
// FIXME: this is trying to be aware of style.Alpha but not fully correct. Also,
// the color wheel will have overlapping glitches with (style.Alpha < 1.0)
bool Gui::ColorPicker4(const char *label, float col[4], ColorEditFlags flags,
                       const float *ref_col) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  DrawList *draw_list = window->DrawList;
  Style &style = g.Style;
  IO &io = g.IO;

  const float width = CalcItemWidth();
  const bool is_readonly = ((g.NextItemData.ItemFlags | g.CurrentItemFlags) &
                            ItemFlags_ReadOnly) != 0;
  g.NextItemData.ClearFlags();

  PushID(label);
  const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
  if (set_current_color_edit_id)
    g.ColorEditCurrentID = window->IDStack.back();
  BeginGroup();

  if (!(flags & ColorEditFlags_NoSidePreview))
    flags |= ColorEditFlags_NoSmallPreview;

  // Context menu: display and store options.
  if (!(flags & ColorEditFlags_NoOptions))
    ColorPickerOptionsPopup(col, flags);

  // Read stored options
  if (!(flags & ColorEditFlags_PickerMask_))
    flags |= ((g.ColorEditOptions & ColorEditFlags_PickerMask_)
                  ? g.ColorEditOptions
                  : ColorEditFlags_DefaultOptions_) &
             ColorEditFlags_PickerMask_;
  if (!(flags & ColorEditFlags_InputMask_))
    flags |= ((g.ColorEditOptions & ColorEditFlags_InputMask_)
                  ? g.ColorEditOptions
                  : ColorEditFlags_DefaultOptions_) &
             ColorEditFlags_InputMask_;
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_PickerMask_)); // Check that only 1 is selected
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_InputMask_)); // Check that only 1 is selected
  if (!(flags & ColorEditFlags_NoOptions))
    flags |= (g.ColorEditOptions & ColorEditFlags_AlphaBar);

  // Setup
  int components = (flags & ColorEditFlags_NoAlpha) ? 3 : 4;
  bool alpha_bar =
      (flags & ColorEditFlags_AlphaBar) && !(flags & ColorEditFlags_NoAlpha);
  Vec2 picker_pos = window->DC.CursorPos;
  float square_sz = GetFrameHeight();
  float bars_width =
      square_sz; // Arbitrary smallish width of Hue/Alpha picking bars
  float sv_picker_size = Max(
      bars_width * 1,
      width - (alpha_bar ? 2 : 1) *
                  (bars_width +
                   style.ItemInnerSpacing.x)); // Saturation/Value picking box
  float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
  float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
  float bars_triangles_half_sz = TRUNC(bars_width * 0.20f);

  float backup_initial_col[4];
  memcpy(backup_initial_col, col, components * sizeof(float));

  float wheel_thickness = sv_picker_size * 0.08f;
  float wheel_r_outer = sv_picker_size * 0.50f;
  float wheel_r_inner = wheel_r_outer - wheel_thickness;
  Vec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width) * 0.5f,
                    picker_pos.y + sv_picker_size * 0.5f);

  // Note: the triangle is displayed rotated with triangle_pa pointing to Hue,
  // but most coordinates stays unrotated for logic.
  float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
  Vec2 triangle_pa = Vec2(triangle_r, 0.0f); // Hue point.
  Vec2 triangle_pb =
      Vec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
  Vec2 triangle_pc =
      Vec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

  float H = col[0], S = col[1], V = col[2];
  float R = col[0], G = col[1], B = col[2];
  if (flags & ColorEditFlags_InputRGB) {
    // Hue is lost when converting from grayscale rgb (saturation=0). Restore
    // it.
    ColorConvertRGBtoHSV(R, G, B, H, S, V);
    ColorEditRestoreHS(col, &H, &S, &V);
  } else if (flags & ColorEditFlags_InputHSV) {
    ColorConvertHSVtoRGB(H, S, V, R, G, B);
  }

  bool value_changed = false, value_changed_h = false, value_changed_sv = false;

  PushItemFlag(ItemFlags_NoNav, true);
  if (flags & ColorEditFlags_PickerHueWheel) {
    // Hue wheel + SV triangle logic
    InvisibleButton("hsv",
                    Vec2(sv_picker_size + style.ItemInnerSpacing.x + bars_width,
                         sv_picker_size));
    if (IsItemActive() && !is_readonly) {
      Vec2 initial_off = g.IO.MouseClickedPos[0] - wheel_center;
      Vec2 current_off = g.IO.MousePos - wheel_center;
      float initial_dist2 = LengthSqr(initial_off);
      if (initial_dist2 >= (wheel_r_inner - 1) * (wheel_r_inner - 1) &&
          initial_dist2 <= (wheel_r_outer + 1) * (wheel_r_outer + 1)) {
        // Interactive with Hue wheel
        H = Atan2(current_off.y, current_off.x) / PI * 0.5f;
        if (H < 0.0f)
          H += 1.0f;
        value_changed = value_changed_h = true;
      }
      float cos_hue_angle = Cos(-H * 2.0f * PI);
      float sin_hue_angle = Sin(-H * 2.0f * PI);
      if (TriangleContainsPoint(
              triangle_pa, triangle_pb, triangle_pc,
              Rotate(initial_off, cos_hue_angle, sin_hue_angle))) {
        // Interacting with SV triangle
        Vec2 current_off_unrotated =
            Rotate(current_off, cos_hue_angle, sin_hue_angle);
        if (!TriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc,
                                   current_off_unrotated))
          current_off_unrotated = TriangleClosestPoint(
              triangle_pa, triangle_pb, triangle_pc, current_off_unrotated);
        float uu, vv, ww;
        TriangleBarycentricCoords(triangle_pa, triangle_pb, triangle_pc,
                                  current_off_unrotated, uu, vv, ww);
        V = Clamp(1.0f - vv, 0.0001f, 1.0f);
        S = Clamp(uu / V, 0.0001f, 1.0f);
        value_changed = value_changed_sv = true;
      }
    }
    if (!(flags & ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context", PopupFlags_MouseButtonRight);
  } else if (flags & ColorEditFlags_PickerHueBar) {
    // SV rectangle logic
    InvisibleButton("sv", Vec2(sv_picker_size, sv_picker_size));
    if (IsItemActive() && !is_readonly) {
      S = Saturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
      V = 1.0f -
          Saturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
      ColorEditRestoreH(
          col, &H); // Greatly reduces hue jitter and reset to 0 when hue == 255
                    // and color is rapidly modified using SV square.
      value_changed = value_changed_sv = true;
    }
    if (!(flags & ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context", PopupFlags_MouseButtonRight);

    // Hue bar logic
    SetCursorScreenPos(Vec2(bar0_pos_x, picker_pos.y));
    InvisibleButton("hue", Vec2(bars_width, sv_picker_size));
    if (IsItemActive() && !is_readonly) {
      H = Saturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
      value_changed = value_changed_h = true;
    }
  }

  // Alpha bar logic
  if (alpha_bar) {
    SetCursorScreenPos(Vec2(bar1_pos_x, picker_pos.y));
    InvisibleButton("alpha", Vec2(bars_width, sv_picker_size));
    if (IsItemActive()) {
      col[3] = 1.0f -
               Saturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
      value_changed = true;
    }
  }
  PopItemFlag(); // ItemFlags_NoNav

  if (!(flags & ColorEditFlags_NoSidePreview)) {
    SameLine(0, style.ItemInnerSpacing.x);
    BeginGroup();
  }

  if (!(flags & ColorEditFlags_NoLabel)) {
    const char *label_display_end = FindRenderedTextEnd(label);
    if (label != label_display_end) {
      if ((flags & ColorEditFlags_NoSidePreview))
        SameLine(0, style.ItemInnerSpacing.x);
      TextEx(label, label_display_end);
    }
  }

  if (!(flags & ColorEditFlags_NoSidePreview)) {
    PushItemFlag(ItemFlags_NoNavDefaultFocus, true);
    Vec4 col_v4(col[0], col[1], col[2],
                (flags & ColorEditFlags_NoAlpha) ? 1.0f : col[3]);
    if ((flags & ColorEditFlags_NoLabel))
      Text("Current");

    ColorEditFlags sub_flags_to_forward =
        ColorEditFlags_InputMask_ | ColorEditFlags_HDR |
        ColorEditFlags_AlphaPreview | ColorEditFlags_AlphaPreviewHalf |
        ColorEditFlags_NoTooltip;
    ColorButton("##current", col_v4, (flags & sub_flags_to_forward),
                Vec2(square_sz * 3, square_sz * 2));
    if (ref_col != NULL) {
      Text("Original");
      Vec4 ref_col_v4(ref_col[0], ref_col[1], ref_col[2],
                      (flags & ColorEditFlags_NoAlpha) ? 1.0f : ref_col[3]);
      if (ColorButton("##original", ref_col_v4, (flags & sub_flags_to_forward),
                      Vec2(square_sz * 3, square_sz * 2))) {
        memcpy(col, ref_col, components * sizeof(float));
        value_changed = true;
      }
    }
    PopItemFlag();
    EndGroup();
  }

  // Convert back color to RGB
  if (value_changed_h || value_changed_sv) {
    if (flags & ColorEditFlags_InputRGB) {
      ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
      g.ColorEditSavedHue = H;
      g.ColorEditSavedSat = S;
      g.ColorEditSavedID = g.ColorEditCurrentID;
      g.ColorEditSavedColor =
          ColorConvertFloat4ToU32(Vec4(col[0], col[1], col[2], 0));
    } else if (flags & ColorEditFlags_InputHSV) {
      col[0] = H;
      col[1] = S;
      col[2] = V;
    }
  }

  // R,G,B and H,S,V slider color editor
  bool value_changed_fix_hue_wrap = false;
  if ((flags & ColorEditFlags_NoInputs) == 0) {
    PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width -
                  picker_pos.x);
    ColorEditFlags sub_flags_to_forward =
        ColorEditFlags_DataTypeMask_ | ColorEditFlags_InputMask_ |
        ColorEditFlags_HDR | ColorEditFlags_NoAlpha | ColorEditFlags_NoOptions |
        ColorEditFlags_NoTooltip | ColorEditFlags_NoSmallPreview |
        ColorEditFlags_AlphaPreview | ColorEditFlags_AlphaPreviewHalf;
    ColorEditFlags sub_flags =
        (flags & sub_flags_to_forward) | ColorEditFlags_NoPicker;
    if (flags & ColorEditFlags_DisplayRGB ||
        (flags & ColorEditFlags_DisplayMask_) == 0)
      if (ColorEdit4("##rgb", col, sub_flags | ColorEditFlags_DisplayRGB)) {
        // FIXME: Hackily differentiating using the DragInt (ActiveId != 0 &&
        // !ActiveIdAllowOverlap) vs. using the InputText or DropTarget. For the
        // later we don't want to run the hue-wrap canceling code. If you are
        // well versed in HSV picker please provide your input! (See #2050)
        value_changed_fix_hue_wrap =
            (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
        value_changed = true;
      }
    if (flags & ColorEditFlags_DisplayHSV ||
        (flags & ColorEditFlags_DisplayMask_) == 0)
      value_changed |=
          ColorEdit4("##hsv", col, sub_flags | ColorEditFlags_DisplayHSV);
    if (flags & ColorEditFlags_DisplayHex ||
        (flags & ColorEditFlags_DisplayMask_) == 0)
      value_changed |=
          ColorEdit4("##hex", col, sub_flags | ColorEditFlags_DisplayHex);
    PopItemWidth();
  }

  // Try to cancel hue wrap (after ColorEdit4 call), if any
  if (value_changed_fix_hue_wrap && (flags & ColorEditFlags_InputRGB)) {
    float new_H, new_S, new_V;
    ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
    if (new_H <= 0 && H > 0) {
      if (new_V <= 0 && V != new_V)
        ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0],
                             col[1], col[2]);
      else if (new_S <= 0)
        ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0],
                             col[1], col[2]);
    }
  }

  if (value_changed) {
    if (flags & ColorEditFlags_InputRGB) {
      R = col[0];
      G = col[1];
      B = col[2];
      ColorConvertRGBtoHSV(R, G, B, H, S, V);
      ColorEditRestoreHS(
          col, &H, &S,
          &V); // Fix local Hue as display below will use it immediately.
    } else if (flags & ColorEditFlags_InputHSV) {
      H = col[0];
      S = col[1];
      V = col[2];
      ColorConvertHSVtoRGB(H, S, V, R, G, B);
    }
  }

  const int style_alpha8 = F32_TO_INT8_SAT(style.Alpha);
  const U32 col_black = COL32(0, 0, 0, style_alpha8);
  const U32 col_white = COL32(255, 255, 255, style_alpha8);
  const U32 col_midgrey = COL32(128, 128, 128, style_alpha8);
  const U32 col_hues[6 + 1] = {
      COL32(255, 0, 0, style_alpha8), COL32(255, 255, 0, style_alpha8),
      COL32(0, 255, 0, style_alpha8), COL32(0, 255, 255, style_alpha8),
      COL32(0, 0, 255, style_alpha8), COL32(255, 0, 255, style_alpha8),
      COL32(255, 0, 0, style_alpha8)};

  Vec4 hue_color_f(1, 1, 1, style.Alpha);
  ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
  U32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
  U32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(
      Vec4(R, G, B, style.Alpha)); // Important: this is still including the
                                   // main rendering/style alpha!!

  Vec2 sv_cursor_pos;

  if (flags & ColorEditFlags_PickerHueWheel) {
    // Render Hue Wheel
    const float aeps =
        0.5f /
        wheel_r_outer; // Half a pixel arc length in radians (2pi cancels out).
    const int segment_per_arc = Max(4, (int)wheel_r_outer / 12);
    for (int n = 0; n < 6; n++) {
      const float a0 = (n) / 6.0f * 2.0f * PI - aeps;
      const float a1 = (n + 1.0f) / 6.0f * 2.0f * PI + aeps;
      const int vert_start_idx = draw_list->VtxBuffer.Size;
      draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer) * 0.5f,
                           a0, a1, segment_per_arc);
      draw_list->PathStroke(col_white, 0, wheel_thickness);
      const int vert_end_idx = draw_list->VtxBuffer.Size;

      // Paint colors over existing vertices
      Vec2 gradient_p0(wheel_center.x + Cos(a0) * wheel_r_inner,
                       wheel_center.y + Sin(a0) * wheel_r_inner);
      Vec2 gradient_p1(wheel_center.x + Cos(a1) * wheel_r_inner,
                       wheel_center.y + Sin(a1) * wheel_r_inner);
      ShadeVertsLinearColorGradientKeepAlpha(
          draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1,
          col_hues[n], col_hues[n + 1]);
    }

    // Render Cursor + preview on Hue Wheel
    float cos_hue_angle = Cos(H * 2.0f * PI);
    float sin_hue_angle = Sin(H * 2.0f * PI);
    Vec2 hue_cursor_pos(
        wheel_center.x + cos_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f,
        wheel_center.y +
            sin_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f);
    float hue_cursor_rad =
        value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
    int hue_cursor_segments = draw_list->_CalcCircleAutoSegmentCount(
        hue_cursor_rad); // Lock segment count so the +1 one matches others.
    draw_list->AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32,
                               hue_cursor_segments);
    draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad + 1, col_midgrey,
                         hue_cursor_segments);
    draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad, col_white,
                         hue_cursor_segments);

    // Render SV triangle (rotated according to hue)
    Vec2 tra = wheel_center + Rotate(triangle_pa, cos_hue_angle, sin_hue_angle);
    Vec2 trb = wheel_center + Rotate(triangle_pb, cos_hue_angle, sin_hue_angle);
    Vec2 trc = wheel_center + Rotate(triangle_pc, cos_hue_angle, sin_hue_angle);
    Vec2 uv_white = GetFontTexUvWhitePixel();
    draw_list->PrimReserve(3, 3);
    draw_list->PrimVtx(tra, uv_white, hue_color32);
    draw_list->PrimVtx(trb, uv_white, col_black);
    draw_list->PrimVtx(trc, uv_white, col_white);
    draw_list->AddTriangle(tra, trb, trc, col_midgrey, 1.5f);
    sv_cursor_pos = Lerp(Lerp(trc, tra, Saturate(S)), trb, Saturate(1 - V));
  } else if (flags & ColorEditFlags_PickerHueBar) {
    // Render SV Square
    draw_list->AddRectFilledMultiColor(
        picker_pos, picker_pos + Vec2(sv_picker_size, sv_picker_size),
        col_white, hue_color32, hue_color32, col_white);
    draw_list->AddRectFilledMultiColor(
        picker_pos, picker_pos + Vec2(sv_picker_size, sv_picker_size), 0, 0,
        col_black, col_black);
    RenderFrameBorder(picker_pos,
                      picker_pos + Vec2(sv_picker_size, sv_picker_size), 0.0f);
    sv_cursor_pos.x = Clamp(
        ROUND(picker_pos.x + Saturate(S) * sv_picker_size), picker_pos.x + 2,
        picker_pos.x + sv_picker_size -
            2); // Sneakily prevent the circle to stick out too much
    sv_cursor_pos.y =
        Clamp(ROUND(picker_pos.y + Saturate(1 - V) * sv_picker_size),
              picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

    // Render Hue Bar
    for (int i = 0; i < 6; ++i)
      draw_list->AddRectFilledMultiColor(
          Vec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)),
          Vec2(bar0_pos_x + bars_width,
               picker_pos.y + (i + 1) * (sv_picker_size / 6)),
          col_hues[i], col_hues[i], col_hues[i + 1], col_hues[i + 1]);
    float bar0_line_y = ROUND(picker_pos.y + H * sv_picker_size);
    RenderFrameBorder(
        Vec2(bar0_pos_x, picker_pos.y),
        Vec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);
    RenderArrowsForVerticalBar(
        draw_list, Vec2(bar0_pos_x - 1, bar0_line_y),
        Vec2(bars_triangles_half_sz + 1, bars_triangles_half_sz),
        bars_width + 2.0f, style.Alpha);
  }

  // Render cursor/preview circle (clamp S/V within 0..1 range because floating
  // points colors may lead HSV values to be out of range)
  float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
  int sv_cursor_segments = draw_list->_CalcCircleAutoSegmentCount(
      sv_cursor_rad); // Lock segment count so the +1 one matches others.
  draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad,
                             user_col32_striped_of_alpha, sv_cursor_segments);
  draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1, col_midgrey,
                       sv_cursor_segments);
  draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, col_white,
                       sv_cursor_segments);

  // Render alpha bar
  if (alpha_bar) {
    float alpha = Saturate(col[3]);
    Rect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width,
                 picker_pos.y + sv_picker_size);
    RenderColorRectWithAlphaCheckerboard(draw_list, bar1_bb.Min, bar1_bb.Max, 0,
                                         bar1_bb.GetWidth() / 2.0f,
                                         Vec2(0.0f, 0.0f));
    draw_list->AddRectFilledMultiColor(
        bar1_bb.Min, bar1_bb.Max, user_col32_striped_of_alpha,
        user_col32_striped_of_alpha,
        user_col32_striped_of_alpha & ~COL32_A_MASK,
        user_col32_striped_of_alpha & ~COL32_A_MASK);
    float bar1_line_y = ROUND(picker_pos.y + (1.0f - alpha) * sv_picker_size);
    RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
    RenderArrowsForVerticalBar(
        draw_list, Vec2(bar1_pos_x - 1, bar1_line_y),
        Vec2(bars_triangles_half_sz + 1, bars_triangles_half_sz),
        bars_width + 2.0f, style.Alpha);
  }

  EndGroup();

  if (value_changed &&
      memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
    value_changed = false;
  if (value_changed &&
      g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup()
                              // won't catch g.ActiveId
    MarkItemEdited(g.LastItemData.ID);

  if (set_current_color_edit_id)
    g.ColorEditCurrentID = 0;
  PopID();

  return value_changed;
}

// A little color square. Return true when clicked.
// FIXME: May want to display/ignore the alpha component in the color display?
// Yet show it in the tooltip. 'desc_id' is not called 'label' because we don't
// display it next to the button, but only in the tooltip. Note that 'col' may
// be encoded in HSV if ColorEditFlags_InputHSV is set.
bool Gui::ColorButton(const char *desc_id, const Vec4 &col,
                      ColorEditFlags flags, const Vec2 &size_arg) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const ID id = window->GetID(desc_id);
  const float default_size = GetFrameHeight();
  const Vec2 size(size_arg.x == 0.0f ? default_size : size_arg.x,
                  size_arg.y == 0.0f ? default_size : size_arg.y);
  const Rect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held);

  if (flags & ColorEditFlags_NoAlpha)
    flags &= ~(ColorEditFlags_AlphaPreview | ColorEditFlags_AlphaPreviewHalf);

  Vec4 col_rgb = col;
  if (flags & ColorEditFlags_InputHSV)
    ColorConvertHSVtoRGB(col_rgb.x, col_rgb.y, col_rgb.z, col_rgb.x, col_rgb.y,
                         col_rgb.z);

  Vec4 col_rgb_without_alpha(col_rgb.x, col_rgb.y, col_rgb.z, 1.0f);
  float grid_step = Min(size.x, size.y) / 2.99f;
  float rounding = Min(g.Style.FrameRounding, grid_step * 0.5f);
  Rect bb_inner = bb;
  float off = 0.0f;
  if ((flags & ColorEditFlags_NoBorder) == 0) {
    off = -0.75f; // The border (using Col_FrameBg) tends to look off when color
                  // is near-opaque and rounding is enabled. This offset seemed
                  // like a good middle ground to reduce those artifacts.
    bb_inner.Expand(off);
  }
  if ((flags & ColorEditFlags_AlphaPreviewHalf) && col_rgb.w < 1.0f) {
    float mid_x = ROUND((bb_inner.Min.x + bb_inner.Max.x) * 0.5f);
    RenderColorRectWithAlphaCheckerboard(
        window->DrawList, Vec2(bb_inner.Min.x + grid_step, bb_inner.Min.y),
        bb_inner.Max, GetColorU32(col_rgb), grid_step,
        Vec2(-grid_step + off, off), rounding, DrawFlags_RoundCornersRight);
    window->DrawList->AddRectFilled(bb_inner.Min, Vec2(mid_x, bb_inner.Max.y),
                                    GetColorU32(col_rgb_without_alpha),
                                    rounding, DrawFlags_RoundCornersLeft);
  } else {
    // Because GetColorU32() multiplies by the global style Alpha and we don't
    // want to display a checkerboard if the source code had no alpha
    Vec4 col_source =
        (flags & ColorEditFlags_AlphaPreview) ? col_rgb : col_rgb_without_alpha;
    if (col_source.w < 1.0f)
      RenderColorRectWithAlphaCheckerboard(
          window->DrawList, bb_inner.Min, bb_inner.Max, GetColorU32(col_source),
          grid_step, Vec2(off, off), rounding);
    else
      window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max,
                                      GetColorU32(col_source), rounding);
  }
  RenderNavHighlight(bb, id);
  if ((flags & ColorEditFlags_NoBorder) == 0) {
    if (g.Style.FrameBorderSize > 0.0f)
      RenderFrameBorder(bb.Min, bb.Max, rounding);
    else
      window->DrawList->AddRect(
          bb.Min, bb.Max, GetColorU32(Col_FrameBg),
          rounding); // Color button are often in need of some sort of border
  }

  // Drag and Drop Source
  // NB: The ActiveId test is merely an optional micro-optimization,
  // BeginDragDropSource() does the same test.
  if (g.ActiveId == id && !(flags & ColorEditFlags_NoDragDrop) &&
      BeginDragDropSource()) {
    if (flags & ColorEditFlags_NoAlpha)
      SetDragDropPayload(PAYLOAD_TYPE_COLOR_3F, &col_rgb, sizeof(float) * 3,
                         Cond_Once);
    else
      SetDragDropPayload(PAYLOAD_TYPE_COLOR_4F, &col_rgb, sizeof(float) * 4,
                         Cond_Once);
    ColorButton(desc_id, col, flags);
    SameLine();
    TextEx("Color");
    EndDragDropSource();
  }

  // Tooltip
  if (!(flags & ColorEditFlags_NoTooltip) && hovered &&
      IsItemHovered(HoveredFlags_ForTooltip))
    ColorTooltip(desc_id, &col.x,
                 flags & (ColorEditFlags_InputMask_ | ColorEditFlags_NoAlpha |
                          ColorEditFlags_AlphaPreview |
                          ColorEditFlags_AlphaPreviewHalf));

  return pressed;
}

// Initialize/override default color options
void Gui::SetColorEditOptions(ColorEditFlags flags) {
  Context &g = *GGui;
  if ((flags & ColorEditFlags_DisplayMask_) == 0)
    flags |= ColorEditFlags_DefaultOptions_ & ColorEditFlags_DisplayMask_;
  if ((flags & ColorEditFlags_DataTypeMask_) == 0)
    flags |= ColorEditFlags_DefaultOptions_ & ColorEditFlags_DataTypeMask_;
  if ((flags & ColorEditFlags_PickerMask_) == 0)
    flags |= ColorEditFlags_DefaultOptions_ & ColorEditFlags_PickerMask_;
  if ((flags & ColorEditFlags_InputMask_) == 0)
    flags |= ColorEditFlags_DefaultOptions_ & ColorEditFlags_InputMask_;
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_DisplayMask_)); // Check only 1 option is selected
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_DataTypeMask_)); // Check only 1 option is selected
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_PickerMask_)); // Check only 1 option is selected
  ASSERT(IsPowerOfTwo(
      flags & ColorEditFlags_InputMask_)); // Check only 1 option is selected
  g.ColorEditOptions = flags;
}

// Note: only access 3 floats if ColorEditFlags_NoAlpha flag is set.
void Gui::ColorTooltip(const char *text, const float *col,
                       ColorEditFlags flags) {
  Context &g = *GGui;

  if (!BeginTooltipEx(TooltipFlags_OverridePrevious, WindowFlags_None))
    return;
  const char *text_end = text ? FindRenderedTextEnd(text, NULL) : text;
  if (text_end > text) {
    TextEx(text, text_end);
    Separator();
  }

  Vec2 sz(g.FontSize * 3 + g.Style.FramePadding.y * 2,
          g.FontSize * 3 + g.Style.FramePadding.y * 2);
  Vec4 cf(col[0], col[1], col[2],
          (flags & ColorEditFlags_NoAlpha) ? 1.0f : col[3]);
  int cr = F32_TO_INT8_SAT(col[0]), cg = F32_TO_INT8_SAT(col[1]),
      cb = F32_TO_INT8_SAT(col[2]),
      ca = (flags & ColorEditFlags_NoAlpha) ? 255 : F32_TO_INT8_SAT(col[3]);
  ColorButton("##preview", cf,
              (flags & (ColorEditFlags_InputMask_ | ColorEditFlags_NoAlpha |
                        ColorEditFlags_AlphaPreview |
                        ColorEditFlags_AlphaPreviewHalf)) |
                  ColorEditFlags_NoTooltip,
              sz);
  SameLine();
  if ((flags & ColorEditFlags_InputRGB) ||
      !(flags & ColorEditFlags_InputMask_)) {
    if (flags & ColorEditFlags_NoAlpha)
      Text("#%02X%02X%02X\nR: %d, G: %d, B: %d\n(%.3f, %.3f, %.3f)", cr, cg, cb,
           cr, cg, cb, col[0], col[1], col[2]);
    else
      Text(
          "#%02X%02X%02X%02X\nR:%d, G:%d, B:%d, A:%d\n(%.3f, %.3f, %.3f, %.3f)",
          cr, cg, cb, ca, cr, cg, cb, ca, col[0], col[1], col[2], col[3]);
  } else if (flags & ColorEditFlags_InputHSV) {
    if (flags & ColorEditFlags_NoAlpha)
      Text("H: %.3f, S: %.3f, V: %.3f", col[0], col[1], col[2]);
    else
      Text("H: %.3f, S: %.3f, V: %.3f, A: %.3f", col[0], col[1], col[2],
           col[3]);
  }
  EndTooltip();
}

void Gui::ColorEditOptionsPopup(const float *col, ColorEditFlags flags) {
  bool allow_opt_inputs = !(flags & ColorEditFlags_DisplayMask_);
  bool allow_opt_datatype = !(flags & ColorEditFlags_DataTypeMask_);
  if ((!allow_opt_inputs && !allow_opt_datatype) || !BeginPopup("context"))
    return;
  Context &g = *GGui;
  g.LockMarkEdited++;
  ColorEditFlags opts = g.ColorEditOptions;
  if (allow_opt_inputs) {
    if (RadioButton("RGB", (opts & ColorEditFlags_DisplayRGB) != 0))
      opts = (opts & ~ColorEditFlags_DisplayMask_) | ColorEditFlags_DisplayRGB;
    if (RadioButton("HSV", (opts & ColorEditFlags_DisplayHSV) != 0))
      opts = (opts & ~ColorEditFlags_DisplayMask_) | ColorEditFlags_DisplayHSV;
    if (RadioButton("Hex", (opts & ColorEditFlags_DisplayHex) != 0))
      opts = (opts & ~ColorEditFlags_DisplayMask_) | ColorEditFlags_DisplayHex;
  }
  if (allow_opt_datatype) {
    if (allow_opt_inputs)
      Separator();
    if (RadioButton("0..255", (opts & ColorEditFlags_Uint8) != 0))
      opts = (opts & ~ColorEditFlags_DataTypeMask_) | ColorEditFlags_Uint8;
    if (RadioButton("0.00..1.00", (opts & ColorEditFlags_Float) != 0))
      opts = (opts & ~ColorEditFlags_DataTypeMask_) | ColorEditFlags_Float;
  }

  if (allow_opt_inputs || allow_opt_datatype)
    Separator();
  if (Button("Copy as..", Vec2(-1, 0)))
    OpenPopup("Copy");
  if (BeginPopup("Copy")) {
    int cr = F32_TO_INT8_SAT(col[0]), cg = F32_TO_INT8_SAT(col[1]),
        cb = F32_TO_INT8_SAT(col[2]),
        ca = (flags & ColorEditFlags_NoAlpha) ? 255 : F32_TO_INT8_SAT(col[3]);
    char buf[64];
    FormatString(buf, ARRAYSIZE(buf), "(%.3ff, %.3ff, %.3ff, %.3ff)", col[0],
                 col[1], col[2],
                 (flags & ColorEditFlags_NoAlpha) ? 1.0f : col[3]);
    if (Selectable(buf))
      SetClipboardText(buf);
    FormatString(buf, ARRAYSIZE(buf), "(%d,%d,%d,%d)", cr, cg, cb, ca);
    if (Selectable(buf))
      SetClipboardText(buf);
    FormatString(buf, ARRAYSIZE(buf), "#%02X%02X%02X", cr, cg, cb);
    if (Selectable(buf))
      SetClipboardText(buf);
    if (!(flags & ColorEditFlags_NoAlpha)) {
      FormatString(buf, ARRAYSIZE(buf), "#%02X%02X%02X%02X", cr, cg, cb, ca);
      if (Selectable(buf))
        SetClipboardText(buf);
    }
    EndPopup();
  }

  g.ColorEditOptions = opts;
  EndPopup();
  g.LockMarkEdited--;
}

void Gui::ColorPickerOptionsPopup(const float *ref_col, ColorEditFlags flags) {
  bool allow_opt_picker = !(flags & ColorEditFlags_PickerMask_);
  bool allow_opt_alpha_bar =
      !(flags & ColorEditFlags_NoAlpha) && !(flags & ColorEditFlags_AlphaBar);
  if ((!allow_opt_picker && !allow_opt_alpha_bar) || !BeginPopup("context"))
    return;
  Context &g = *GGui;
  g.LockMarkEdited++;
  if (allow_opt_picker) {
    Vec2 picker_size(
        g.FontSize * 8,
        Max(g.FontSize * 8 - (GetFrameHeight() + g.Style.ItemInnerSpacing.x),
            1.0f)); // FIXME: Picker size copied from main picker function
    PushItemWidth(picker_size.x);
    for (int picker_type = 0; picker_type < 2; picker_type++) {
      // Draw small/thumbnail version of each picker type (over an invisible
      // button for selection)
      if (picker_type > 0)
        Separator();
      PushID(picker_type);
      ColorEditFlags picker_flags =
          ColorEditFlags_NoInputs | ColorEditFlags_NoOptions |
          ColorEditFlags_NoLabel | ColorEditFlags_NoSidePreview |
          (flags & ColorEditFlags_NoAlpha);
      if (picker_type == 0)
        picker_flags |= ColorEditFlags_PickerHueBar;
      if (picker_type == 1)
        picker_flags |= ColorEditFlags_PickerHueWheel;
      Vec2 backup_pos = GetCursorScreenPos();
      if (Selectable("##selectable", false, 0,
                     picker_size)) // By default, Selectable() is closing popup
        g.ColorEditOptions =
            (g.ColorEditOptions & ~ColorEditFlags_PickerMask_) |
            (picker_flags & ColorEditFlags_PickerMask_);
      SetCursorScreenPos(backup_pos);
      Vec4 previewing_ref_col;
      memcpy(&previewing_ref_col, ref_col,
             sizeof(float) * ((picker_flags & ColorEditFlags_NoAlpha) ? 3 : 4));
      ColorPicker4("##previewing_picker", &previewing_ref_col.x, picker_flags);
      PopID();
    }
    PopItemWidth();
  }
  if (allow_opt_alpha_bar) {
    if (allow_opt_picker)
      Separator();
    CheckboxFlags("Alpha Bar", &g.ColorEditOptions, ColorEditFlags_AlphaBar);
  }
  EndPopup();
  g.LockMarkEdited--;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
//-------------------------------------------------------------------------
// - TreeNode()
// - TreeNodeV()
// - TreeNodeEx()
// - TreeNodeExV()
// - TreeNodeBehavior() [Internal]
// - TreePush()
// - TreePop()
// - GetTreeNodeToLabelSpacing()
// - SetNextItemOpen()
// - CollapsingHeader()
//-------------------------------------------------------------------------

bool Gui::TreeNode(const char *str_id, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(str_id, 0, fmt, args);
  va_end(args);
  return is_open;
}

bool Gui::TreeNode(const void *ptr_id, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(ptr_id, 0, fmt, args);
  va_end(args);
  return is_open;
}

bool Gui::TreeNode(const char *label) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;
  return TreeNodeBehavior(window->GetID(label), 0, label, NULL);
}

bool Gui::TreeNodeV(const char *str_id, const char *fmt, va_list args) {
  return TreeNodeExV(str_id, 0, fmt, args);
}

bool Gui::TreeNodeV(const void *ptr_id, const char *fmt, va_list args) {
  return TreeNodeExV(ptr_id, 0, fmt, args);
}

bool Gui::TreeNodeEx(const char *label, TreeNodeFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  return TreeNodeBehavior(window->GetID(label), flags, label, NULL);
}

bool Gui::TreeNodeEx(const char *str_id, TreeNodeFlags flags, const char *fmt,
                     ...) {
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(str_id, flags, fmt, args);
  va_end(args);
  return is_open;
}

bool Gui::TreeNodeEx(const void *ptr_id, TreeNodeFlags flags, const char *fmt,
                     ...) {
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(ptr_id, flags, fmt, args);
  va_end(args);
  return is_open;
}

bool Gui::TreeNodeExV(const char *str_id, TreeNodeFlags flags, const char *fmt,
                      va_list args) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  const char *label, *label_end;
  FormatStringToTempBufferV(&label, &label_end, fmt, args);
  return TreeNodeBehavior(window->GetID(str_id), flags, label, label_end);
}

bool Gui::TreeNodeExV(const void *ptr_id, TreeNodeFlags flags, const char *fmt,
                      va_list args) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  const char *label, *label_end;
  FormatStringToTempBufferV(&label, &label_end, fmt, args);
  return TreeNodeBehavior(window->GetID(ptr_id), flags, label, label_end);
}

void Gui::TreeNodeSetOpen(ID id, bool open) {
  Context &g = *GGui;
  Storage *storage = g.CurrentWindow->DC.StateStorage;
  storage->SetInt(id, open ? 1 : 0);
}

bool Gui::TreeNodeUpdateNextOpen(ID id, TreeNodeFlags flags) {
  if (flags & TreeNodeFlags_Leaf)
    return true;

  // We only write to the tree storage if the user clicks (or explicitly use the
  // SetNextItemOpen function)
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Storage *storage = window->DC.StateStorage;

  bool is_open;
  if (g.NextItemData.Flags & NextItemDataFlags_HasOpen) {
    if (g.NextItemData.OpenCond & Cond_Always) {
      is_open = g.NextItemData.OpenVal;
      TreeNodeSetOpen(id, is_open);
    } else {
      // We treat Cond_Once and Cond_FirstUseEver the same because
      // tree node state are not saved persistently.
      const int stored_value = storage->GetInt(id, -1);
      if (stored_value == -1) {
        is_open = g.NextItemData.OpenVal;
        TreeNodeSetOpen(id, is_open);
      } else {
        is_open = stored_value != 0;
      }
    }
  } else {
    is_open =
        storage->GetInt(id, (flags & TreeNodeFlags_DefaultOpen) ? 1 : 0) != 0;
  }

  // When logging is enabled, we automatically expand tree nodes (but *NOT*
  // collapsing headers.. seems like sensible behavior). NB- If we are above max
  // depth we still allow manually opened nodes to be logged.
  if (g.LogEnabled && !(flags & TreeNodeFlags_NoAutoOpenOnLog) &&
      (window->DC.TreeDepth - g.LogDepthRef) < g.LogDepthToExpand)
    is_open = true;

  return is_open;
}

bool Gui::TreeNodeBehavior(ID id, TreeNodeFlags flags, const char *label,
                           const char *label_end) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const bool display_frame = (flags & TreeNodeFlags_Framed) != 0;
  const Vec2 padding =
      (display_frame || (flags & TreeNodeFlags_FramePadding))
          ? style.FramePadding
          : Vec2(style.FramePadding.x,
                 Min(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

  if (!label_end)
    label_end = FindRenderedTextEnd(label);
  const Vec2 label_size = CalcTextSize(label, label_end, false);

  // We vertically grow up to current line height up the typical widget height.
  const float frame_height =
      Max(Min(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2),
          label_size.y + padding.y * 2);
  const bool span_all_columns =
      (flags & TreeNodeFlags_SpanAllColumns) != 0 && (g.CurrentTable != NULL);
  Rect frame_bb;
  frame_bb.Min.x = span_all_columns ? window->ParentWorkRect.Min.x
                   : (flags & TreeNodeFlags_SpanFullWidth)
                       ? window->WorkRect.Min.x
                       : window->DC.CursorPos.x;
  frame_bb.Min.y = window->DC.CursorPos.y;
  frame_bb.Max.x =
      span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
  frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
  if (display_frame) {
    // Framed header expand a little outside the default padding, to the edge of
    // InnerClipRect (FIXME: May remove this at some point and make
    // InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
    frame_bb.Min.x -= TRUNC(window->WindowPadding.x * 0.5f - 1.0f);
    frame_bb.Max.x += TRUNC(window->WindowPadding.x * 0.5f);
  }

  const float text_offset_x =
      g.FontSize + (display_frame
                        ? padding.x * 3
                        : padding.x * 2); // Collapsing arrow width + Spacing
  const float text_offset_y = Max(
      padding.y,
      window->DC.CurrLineTextBaseOffset); // Latch before ItemSize changes it
  const float text_width =
      g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2
                                        : 0.0f); // Include collapsing
  Vec2 text_pos(window->DC.CursorPos.x + text_offset_x,
                window->DC.CursorPos.y + text_offset_y);
  ItemSize(Vec2(text_width, frame_height), padding.y);

  // For regular tree nodes, we arbitrary allow to click past 2 worth of
  // ItemSpacing
  Rect interact_bb = frame_bb;
  if (!display_frame &&
      (flags & (TreeNodeFlags_SpanAvailWidth | TreeNodeFlags_SpanFullWidth |
                TreeNodeFlags_SpanAllColumns)) == 0)
    interact_bb.Max.x =
        frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;

  // Modify ClipRect for the ItemAdd(), faster than doing a
  // PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
  const float backup_clip_rect_min_x = window->ClipRect.Min.x;
  const float backup_clip_rect_max_x = window->ClipRect.Max.x;
  if (span_all_columns) {
    window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
    window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
  }

  // Compute open and multi-select states before ItemAdd() as it clear NextItem
  // data.
  bool is_open = TreeNodeUpdateNextOpen(id, flags);
  bool item_add = ItemAdd(interact_bb, id);
  g.LastItemData.StatusFlags |= ItemStatusFlags_HasDisplayRect;
  g.LastItemData.DisplayRect = frame_bb;

  if (span_all_columns) {
    window->ClipRect.Min.x = backup_clip_rect_min_x;
    window->ClipRect.Max.x = backup_clip_rect_max_x;
  }

  // If a NavLeft request is happening and
  // TreeNodeFlags_NavLeftJumpsBackHere enabled: Store data for the current
  // depth to allow returning to this node from any child item. For this purpose
  // we essentially compare if g.NavIdIsAlive went from 0 to 1 between
  // TreeNode() and TreePop(). It will become tempting to enable
  // TreeNodeFlags_NavLeftJumpsBackHere by default or move it to
  // Style. Currently only supports 32 level deep and we are fine with (1
  // << Depth) overflowing into a zero, easy to increase.
  if (is_open && !g.NavIdIsAlive &&
      (flags & TreeNodeFlags_NavLeftJumpsBackHere) &&
      !(flags & TreeNodeFlags_NoTreePushOnOpen))
    if (g.NavMoveDir == Dir_Left && g.NavWindow == window &&
        NavMoveRequestButNoResultYet()) {
      g.NavTreeNodeStack.resize(g.NavTreeNodeStack.Size + 1);
      NavTreeNodeData *nav_tree_node_data = &g.NavTreeNodeStack.back();
      nav_tree_node_data->ID = id;
      nav_tree_node_data->InFlags = g.LastItemData.InFlags;
      nav_tree_node_data->NavRect = g.LastItemData.NavRect;
      window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);
    }

  const bool is_leaf = (flags & TreeNodeFlags_Leaf) != 0;
  if (!item_add) {
    if (is_open && !(flags & TreeNodeFlags_NoTreePushOnOpen))
      TreePushOverrideID(id);
    TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label,
                          g.LastItemData.StatusFlags |
                              (is_leaf ? 0 : ItemStatusFlags_Openable) |
                              (is_open ? ItemStatusFlags_Opened : 0));
    return is_open;
  }

  if (span_all_columns) {
    TablePushBackgroundChannel();
    g.LastItemData.StatusFlags |= ItemStatusFlags_HasClipRect;
    g.LastItemData.ClipRect = window->ClipRect;
  }

  ButtonFlags button_flags = TreeNodeFlags_None;
  if ((flags & TreeNodeFlags_AllowOverlap) ||
      (g.LastItemData.InFlags & ItemFlags_AllowOverlap))
    button_flags |= ButtonFlags_AllowOverlap;
  if (!is_leaf)
    button_flags |= ButtonFlags_PressedOnDragDropHold;

  // We allow clicking on the arrow section with keyboard modifiers held, in
  // order to easily allow browsing a tree while preserving selection with code
  // implementing multi-selection patterns. When clicking on the rest of the
  // tree node we always disallow keyboard modifiers.
  const float arrow_hit_x1 =
      (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
  const float arrow_hit_x2 = (text_pos.x - text_offset_x) +
                             (g.FontSize + padding.x * 2.0f) +
                             style.TouchExtraPadding.x;
  const bool is_mouse_x_over_arrow =
      (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
  if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
    button_flags |= ButtonFlags_NoKeyModifiers;

  // Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick
  // flags. Some alteration have subtle effects (e.g. toggle on MouseUp vs
  // MouseDown events) due to requirements for multi-selection and drag and drop
  // support.
  // - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
  // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
  // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
  // - Double-click on label = Toggle on MouseDoubleClick (when
  // _OpenOnDoubleClick=1)
  // - Double-click on arrow = Toggle on MouseDoubleClick (when
  // _OpenOnDoubleClick=1 and _OpenOnArrow=0) It is rather standard that arrow
  // click react on Down rather than Up. We set
  // ButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want
  // the item to be active on the initial MouseDown in order for drag and drop
  // to work.
  if (is_mouse_x_over_arrow)
    button_flags |= ButtonFlags_PressedOnClick;
  else if (flags & TreeNodeFlags_OpenOnDoubleClick)
    button_flags |=
        ButtonFlags_PressedOnClickRelease | ButtonFlags_PressedOnDoubleClick;
  else
    button_flags |= ButtonFlags_PressedOnClickRelease;

  bool selected = (flags & TreeNodeFlags_Selected) != 0;
  const bool was_selected = selected;

  bool hovered, held;
  bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
  bool toggled = false;
  if (!is_leaf) {
    if (pressed && g.DragDropHoldJustPressedId != id) {
      if ((flags & (TreeNodeFlags_OpenOnArrow |
                    TreeNodeFlags_OpenOnDoubleClick)) == 0 ||
          (g.NavActivateId == id))
        toggled = true;
      if (flags & TreeNodeFlags_OpenOnArrow)
        toggled |=
            is_mouse_x_over_arrow &&
            !g.NavDisableMouseHover; // Lightweight equivalent of
                                     // IsMouseHoveringRect() since
                                     // ButtonBehavior() already did the job
      if ((flags & TreeNodeFlags_OpenOnDoubleClick) &&
          g.IO.MouseClickedCount[0] == 2)
        toggled = true;
    } else if (pressed && g.DragDropHoldJustPressedId == id) {
      ASSERT(button_flags & ButtonFlags_PressedOnDragDropHold);
      if (!is_open) // When using Drag and Drop "hold to open" we keep the node
                    // highlighted after opening, but never close it again.
        toggled = true;
    }

    if (g.NavId == id && g.NavMoveDir == Dir_Left && is_open) {
      toggled = true;
      NavClearPreferredPosForAxis(Axis_X);
      NavMoveRequestCancel();
    }
    if (g.NavId == id && g.NavMoveDir == Dir_Right &&
        !is_open) // If there's something upcoming on the line we may want to
                  // give it the priority?
    {
      toggled = true;
      NavClearPreferredPosForAxis(Axis_X);
      NavMoveRequestCancel();
    }

    if (toggled) {
      is_open = !is_open;
      window->DC.StateStorage->SetInt(id, is_open);
      g.LastItemData.StatusFlags |= ItemStatusFlags_ToggledOpen;
    }
  }

  // In this branch, TreeNodeBehavior() cannot toggle the selection so this will
  // never trigger.
  if (selected != was_selected) //-V547
    g.LastItemData.StatusFlags |= ItemStatusFlags_ToggledSelection;

  // Render
  const U32 text_col = GetColorU32(Col_Text);
  NavHighlightFlags nav_highlight_flags = NavHighlightFlags_TypeThin;
  if (display_frame) {
    // Framed type
    const U32 bg_col = GetColorU32((held && hovered) ? Col_HeaderActive
                                   : hovered         ? Col_HeaderHovered
                                                     : Col_Header);
    RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
    RenderNavHighlight(frame_bb, id, nav_highlight_flags);
    if (flags & TreeNodeFlags_Bullet)
      RenderBullet(window->DrawList,
                   Vec2(text_pos.x - text_offset_x * 0.60f,
                        text_pos.y + g.FontSize * 0.5f),
                   text_col);
    else if (!is_leaf)
      RenderArrow(
          window->DrawList,
          Vec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col,
          is_open
              ? ((flags & TreeNodeFlags_UpsideDownArrow) ? Dir_Up : Dir_Down)
              : Dir_Right,
          1.0f);
    else // Leaf without bullet, left-adjusted text
      text_pos.x -= text_offset_x - padding.x;
    if (flags & TreeNodeFlags_ClipLabelForTrailingButton)
      frame_bb.Max.x -= g.FontSize + style.FramePadding.x;

    if (g.LogEnabled)
      LogSetNextTextDecoration("###", "###");
  } else {
    // Unframed typed for tree nodes
    if (hovered || selected) {
      const U32 bg_col = GetColorU32((held && hovered) ? Col_HeaderActive
                                     : hovered         ? Col_HeaderHovered
                                                       : Col_Header);
      RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
    }
    RenderNavHighlight(frame_bb, id, nav_highlight_flags);
    if (flags & TreeNodeFlags_Bullet)
      RenderBullet(window->DrawList,
                   Vec2(text_pos.x - text_offset_x * 0.5f,
                        text_pos.y + g.FontSize * 0.5f),
                   text_col);
    else if (!is_leaf)
      RenderArrow(window->DrawList,
                  Vec2(text_pos.x - text_offset_x + padding.x,
                       text_pos.y + g.FontSize * 0.15f),
                  text_col,
                  is_open ? ((flags & TreeNodeFlags_UpsideDownArrow) ? Dir_Up
                                                                     : Dir_Down)
                          : Dir_Right,
                  0.70f);
    if (g.LogEnabled)
      LogSetNextTextDecoration(">", NULL);
  }

  if (span_all_columns)
    TablePopBackgroundChannel();

  // Label
  if (display_frame)
    RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
  else
    RenderText(text_pos, label, label_end, false);

  if (is_open && !(flags & TreeNodeFlags_NoTreePushOnOpen))
    TreePushOverrideID(id);
  TEST_ENGINE_ITEM_INFO(id, label,
                        g.LastItemData.StatusFlags |
                            (is_leaf ? 0 : ItemStatusFlags_Openable) |
                            (is_open ? ItemStatusFlags_Opened : 0));
  return is_open;
}

void Gui::TreePush(const char *str_id) {
  Window *window = GetCurrentWindow();
  Indent();
  window->DC.TreeDepth++;
  PushID(str_id);
}

void Gui::TreePush(const void *ptr_id) {
  Window *window = GetCurrentWindow();
  Indent();
  window->DC.TreeDepth++;
  PushID(ptr_id);
}

void Gui::TreePushOverrideID(ID id) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Indent();
  window->DC.TreeDepth++;
  PushOverrideID(id);
}

void Gui::TreePop() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  Unindent();

  window->DC.TreeDepth--;
  U32 tree_depth_mask = (1 << window->DC.TreeDepth);

  // Handle Left arrow to move to parent tree node (when
  // TreeNodeFlags_NavLeftJumpsBackHere is enabled)
  if (window->DC.TreeJumpToParentOnPopMask &
      tree_depth_mask) // Only set during request
  {
    NavTreeNodeData *nav_tree_node_data = &g.NavTreeNodeStack.back();
    ASSERT(nav_tree_node_data->ID == window->IDStack.back());
    if (g.NavIdIsAlive && g.NavMoveDir == Dir_Left && g.NavWindow == window &&
        NavMoveRequestButNoResultYet())
      NavMoveRequestResolveWithPastTreeNode(&g.NavMoveResultLocal,
                                            nav_tree_node_data);
    g.NavTreeNodeStack.pop_back();
  }
  window->DC.TreeJumpToParentOnPopMask &= tree_depth_mask - 1;

  ASSERT(window->IDStack.Size >
         1); // There should always be 1 element in the IDStack (pushed
             // during window creation). If this triggers you called
             // TreePop/PopID too much.
  PopID();
}

// Horizontal distance preceding label when using TreeNode() or Bullet()
float Gui::GetTreeNodeToLabelSpacing() {
  Context &g = *GGui;
  return g.FontSize + (g.Style.FramePadding.x * 2.0f);
}

// Set next TreeNode/CollapsingHeader open state.
void Gui::SetNextItemOpen(bool is_open, Cond cond) {
  Context &g = *GGui;
  if (g.CurrentWindow->SkipItems)
    return;
  g.NextItemData.Flags |= NextItemDataFlags_HasOpen;
  g.NextItemData.OpenVal = is_open;
  g.NextItemData.OpenCond = cond ? cond : Cond_Always;
}

// CollapsingHeader returns true when opened but do not indent nor push into the
// ID stack (because of the TreeNodeFlags_NoTreePushOnOpen flag). This is
// basically the same as calling TreeNodeEx(label,
// TreeNodeFlags_CollapsingHeader). You can remove the _NoTreePushOnOpen
// flag if you want behavior closer to normal TreeNode().
bool Gui::CollapsingHeader(const char *label, TreeNodeFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  return TreeNodeBehavior(window->GetID(label),
                          flags | TreeNodeFlags_CollapsingHeader, label);
}

// p_visible == NULL                        : regular collapsing header
// p_visible != NULL && *p_visible == true  : show a small close button on the
// corner of the header, clicking the button will set *p_visible = false
// p_visible != NULL && *p_visible == false : do not show the header at all
// Do not mistake this with the Open state of the header itself, which you can
// adjust with SetNextItemOpen() or TreeNodeFlags_DefaultOpen.
bool Gui::CollapsingHeader(const char *label, bool *p_visible,
                           TreeNodeFlags flags) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  if (p_visible && !*p_visible)
    return false;

  ID id = window->GetID(label);
  flags |= TreeNodeFlags_CollapsingHeader;
  if (p_visible)
    flags |= TreeNodeFlags_AllowOverlap |
             (TreeNodeFlags)TreeNodeFlags_ClipLabelForTrailingButton;
  bool is_open = TreeNodeBehavior(id, flags, label);
  if (p_visible != NULL) {
    // Create a small overlapping close button
    // FIXME: We can evolve this into user accessible helpers to add extra
    // buttons on title bars, headers, etc.
    // FIXME: CloseButton can overlap into text, need find a way to clip the
    // text somehow.
    Context &g = *GGui;
    LastItemData last_item_backup = g.LastItemData;
    float button_size = g.FontSize;
    float button_x =
        Max(g.LastItemData.Rect.Min.x,
            g.LastItemData.Rect.Max.x - g.Style.FramePadding.x - button_size);
    float button_y = g.LastItemData.Rect.Min.y + g.Style.FramePadding.y;
    ID close_button_id = GetIDWithSeed("#CLOSE", NULL, id);
    if (CloseButton(close_button_id, Vec2(button_x, button_y)))
      *p_visible = false;
    g.LastItemData = last_item_backup;
  }

  return is_open;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Selectable
//-------------------------------------------------------------------------
// - Selectable()
//-------------------------------------------------------------------------

// Tip: pass a non-visible label (e.g. "##hello") then you can use the space to
// draw other text or image. But you need to make sure the ID is unique, e.g.
// enclose calls in PushID/PopID or use ##unique_id. With this scheme,
// SelectableFlags_SpanAllColumns and SelectableFlags_AllowOverlap are
// also frequently used flags.
// FIXME: Selectable() with (size.x == 0.0f) and (SelectableTextAlign.x > 0.0f)
// followed by SameLine() is currently not supported.
bool Gui::Selectable(const char *label, bool selected, SelectableFlags flags,
                     const Vec2 &size_arg) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;

  // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit
  // a larger/spanning rectangle.
  ID id = window->GetID(label);
  Vec2 label_size = CalcTextSize(label, NULL, true);
  Vec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x,
            size_arg.y != 0.0f ? size_arg.y : label_size.y);
  Vec2 pos = window->DC.CursorPos;
  pos.y += window->DC.CurrLineTextBaseOffset;
  ItemSize(size, 0.0f);

  // Fill horizontal space
  // We don't support (size < 0.0f) in Selectable() because the ItemSpacing
  // extension would make explicitly right-aligned sizes not visibly match other
  // widgets.
  const bool span_all_columns = (flags & SelectableFlags_SpanAllColumns) != 0;
  const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
  const float max_x =
      span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
  if (size_arg.x == 0.0f || (flags & SelectableFlags_SpanAvailWidth))
    size.x = Max(label_size.x, max_x - min_x);

  // Text stays at the submission position, but bounding box may be extended on
  // both sides
  const Vec2 text_min = pos;
  const Vec2 text_max(min_x + size.x, pos.y + size.y);

  // Selectables are meant to be tightly packed together with no click-gap, so
  // we extend their box to cover spacing between selectable.
  Rect bb(min_x, pos.y, text_max.x, text_max.y);
  if ((flags & SelectableFlags_NoPadWithHalfSpacing) == 0) {
    const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
    const float spacing_y = style.ItemSpacing.y;
    const float spacing_L = TRUNC(spacing_x * 0.50f);
    const float spacing_U = TRUNC(spacing_y * 0.50f);
    bb.Min.x -= spacing_L;
    bb.Min.y -= spacing_U;
    bb.Max.x += (spacing_x - spacing_L);
    bb.Max.y += (spacing_y - spacing_U);
  }
  // if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max,
  // COL32(0, 255, 0, 255)); }

  // Modify ClipRect for the ItemAdd(), faster than doing a
  // PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
  const float backup_clip_rect_min_x = window->ClipRect.Min.x;
  const float backup_clip_rect_max_x = window->ClipRect.Max.x;
  if (span_all_columns) {
    window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
    window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
  }

  const bool disabled_item = (flags & SelectableFlags_Disabled) != 0;
  const bool item_add = ItemAdd(
      bb, id, NULL, disabled_item ? ItemFlags_Disabled : ItemFlags_None);
  if (span_all_columns) {
    window->ClipRect.Min.x = backup_clip_rect_min_x;
    window->ClipRect.Max.x = backup_clip_rect_max_x;
  }

  if (!item_add)
    return false;

  const bool disabled_global = (g.CurrentItemFlags & ItemFlags_Disabled) != 0;
  if (disabled_item && !disabled_global) // Only testing this as an optimization
    BeginDisabled();

  // FIXME: We can standardize the behavior of those two, we could also keep the
  // fast path of override ClipRect + full push on render only, which would be
  // advantageous since most selectable are not selected.
  if (span_all_columns) {
    if (g.CurrentTable)
      TablePushBackgroundChannel();
    else if (window->DC.CurrentColumns)
      PushColumnsBackground();
    g.LastItemData.StatusFlags |= ItemStatusFlags_HasClipRect;
    g.LastItemData.ClipRect = window->ClipRect;
  }

  // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu
  // then drag to browse child entries
  ButtonFlags button_flags = 0;
  if (flags & SelectableFlags_NoHoldingActiveID) {
    button_flags |= ButtonFlags_NoHoldingActiveId;
  }
  if (flags & SelectableFlags_NoSetKeyOwner) {
    button_flags |= ButtonFlags_NoSetKeyOwner;
  }
  if (flags & SelectableFlags_SelectOnClick) {
    button_flags |= ButtonFlags_PressedOnClick;
  }
  if (flags & SelectableFlags_SelectOnRelease) {
    button_flags |= ButtonFlags_PressedOnRelease;
  }
  if (flags & SelectableFlags_AllowDoubleClick) {
    button_flags |=
        ButtonFlags_PressedOnClickRelease | ButtonFlags_PressedOnDoubleClick;
  }
  if ((flags & SelectableFlags_AllowOverlap) ||
      (g.LastItemData.InFlags & ItemFlags_AllowOverlap)) {
    button_flags |= ButtonFlags_AllowOverlap;
  }

  const bool was_selected = selected;
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

  // Auto-select when moved into
  // - This will be more fully fleshed in the range-select branch
  // - This is not exposed as it won't nicely work with some user side handling
  // of shift/control
  // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed =
  // was_selected; }' for two reasons
  //   - (1) it would require focus scope to be set, need exposing
  //   PushFocusScope() or equivalent (e.g. BeginSelection() calling
  //   PushFocusScope())
  //   - (2) usage will fail with clipped items
  //   The multi-select API aim to fix those issues, e.g. may be replaced with a
  //   BeginSelection() API.
  if ((flags & SelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 &&
      g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
    if (g.NavJustMovedToId == id)
      selected = pressed = true;

  // Update NavId when clicking or when Hovering (this doesn't happen on most
  // widgets), so navigation can be resumed with gamepad/keyboard
  if (pressed || (hovered && (flags & SelectableFlags_SetNavIdOnHover))) {
    if (!g.NavDisableMouseHover && g.NavWindow == window &&
        g.NavLayer == window->DC.NavLayerCurrent) {
      SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId,
               WindowRectAbsToRel(window, bb)); // (bb == NavRect)
      g.NavDisableHighlight = true;
    }
  }
  if (pressed)
    MarkItemEdited(id);

  // In this branch, Selectable() cannot toggle the selection so this will never
  // trigger.
  if (selected != was_selected) //-V547
    g.LastItemData.StatusFlags |= ItemStatusFlags_ToggledSelection;

  // Render
  if (hovered || selected) {
    const U32 col = GetColorU32((held && hovered) ? Col_HeaderActive
                                : hovered         ? Col_HeaderHovered
                                                  : Col_Header);
    RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
  }
  if (g.NavId == id)
    RenderNavHighlight(
        bb, id, NavHighlightFlags_TypeThin | NavHighlightFlags_NoRounding);

  if (span_all_columns) {
    if (g.CurrentTable)
      TablePopBackgroundChannel();
    else if (window->DC.CurrentColumns)
      PopColumnsBackground();
  }

  RenderTextClipped(text_min, text_max, label, NULL, &label_size,
                    style.SelectableTextAlign, &bb);

  // Automatically close popups
  if (pressed && (window->Flags & WindowFlags_Popup) &&
      !(flags & SelectableFlags_DontClosePopups) &&
      !(g.LastItemData.InFlags & ItemFlags_SelectableDontClosePopup))
    CloseCurrentPopup();

  if (disabled_item && !disabled_global)
    EndDisabled();

  TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
  return pressed; //-V1020
}

bool Gui::Selectable(const char *label, bool *p_selected, SelectableFlags flags,
                     const Vec2 &size_arg) {
  if (Selectable(label, *p_selected, flags, size_arg)) {
    *p_selected = !*p_selected;
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Typing-Select support
//-------------------------------------------------------------------------

// [Experimental] Currently not exposed in public API.
// Consume character inputs and return search request, if any.
// This would typically only be called on the focused window or location you
// want to grab inputs for, e.g.
//   if (Gui::IsWindowFocused(...))
//       if (TypingSelectRequest* req = Gui::GetTypingSelectRequest())
//           focus_idx = Gui::TypingSelectFindMatch(req, my_items.size(),
//           [](void*, int n) { return my_items[n]->Name; }, &my_items, -1);
// However the code is written in a way where calling it from multiple locations
// is safe (e.g. to obtain buffer).
TypingSelectRequest *Gui::GetTypingSelectRequest(TypingSelectFlags flags) {
  Context &g = *GGui;
  TypingSelectState *data = &g.TypingSelectState;
  TypingSelectRequest *out_request = &data->Request;

  // Clear buffer
  const float TYPING_SELECT_RESET_TIMER =
      1.80f; // FIXME: Potentially move to IO config.
  const int TYPING_SELECT_SINGLE_CHAR_COUNT_FOR_LOCK =
      4; // Lock single char matching when repeating same char 4 times
  if (data->SearchBuffer[0] != 0) {
    bool clear_buffer = false;
    clear_buffer |= (g.NavFocusScopeId != data->FocusScope);
    clear_buffer |=
        (data->LastRequestTime + TYPING_SELECT_RESET_TIMER < g.Time);
    clear_buffer |= g.NavAnyRequest;
    clear_buffer |= g.ActiveId != 0 &&
                    g.NavActivateId ==
                        0; // Allow temporary SPACE activation to not interfere
    clear_buffer |= IsKeyPressed(Key_Escape) || IsKeyPressed(Key_Enter);
    clear_buffer |= IsKeyPressed(Key_Backspace) &&
                    (flags & TypingSelectFlags_AllowBackspace) == 0;
    // if (clear_buffer) { DEBUG_LOG("GetTypingSelectRequest(): Clear
    // SearchBuffer.\n"); }
    if (clear_buffer)
      data->Clear();
  }

  // Append to buffer
  const int buffer_max_len = ARRAYSIZE(data->SearchBuffer) - 1;
  int buffer_len = (int)strlen(data->SearchBuffer);
  bool select_request = false;
  for (Wchar w : g.IO.InputQueueCharacters) {
    const int w_len = TextCountUtf8BytesFromStr(&w, &w + 1);
    if (w < 32 || (buffer_len == 0 && CharIsBlankW(w)) ||
        (buffer_len + w_len > buffer_max_len)) // Ignore leading blanks
      continue;
    char w_buf[5];
    TextCharToUtf8(w_buf, (unsigned int)w);
    if (data->SingleCharModeLock && w_len == out_request->SingleCharSize &&
        memcmp(w_buf, data->SearchBuffer, w_len) == 0) {
      select_request = true; // Same character: don't need to append to buffer.
      continue;
    }
    if (data->SingleCharModeLock) {
      data->Clear(); // Different character: clear
      buffer_len = 0;
    }
    memcpy(data->SearchBuffer + buffer_len, w_buf, w_len + 1); // Append
    buffer_len += w_len;
    select_request = true;
  }
  g.IO.InputQueueCharacters.resize(0);

  // Handle backspace
  if ((flags & TypingSelectFlags_AllowBackspace) &&
      IsKeyPressed(Key_Backspace, 0, InputFlags_Repeat)) {
    char *p = (char *)(void *)TextFindPreviousUtf8Codepoint(
        data->SearchBuffer, data->SearchBuffer + buffer_len);
    *p = 0;
    buffer_len = (int)(p - data->SearchBuffer);
  }

  // Return request if any
  if (buffer_len == 0)
    return NULL;
  if (select_request) {
    data->FocusScope = g.NavFocusScopeId;
    data->LastRequestFrame = g.FrameCount;
    data->LastRequestTime = (float)g.Time;
  }
  out_request->Flags = flags;
  out_request->SearchBufferLen = buffer_len;
  out_request->SearchBuffer = data->SearchBuffer;
  out_request->SelectRequest = (data->LastRequestFrame == g.FrameCount);
  out_request->SingleCharMode = false;
  out_request->SingleCharSize = 0;

  // Calculate if buffer contains the same character repeated.
  // - This can be used to implement a special search mode on first character.
  // - Performed on UTF-8 codepoint for correctness.
  // - SingleCharMode is always set for first input character, because it
  // usually leads to a "next".
  if (flags & TypingSelectFlags_AllowSingleCharMode) {
    const char *buf_begin = out_request->SearchBuffer;
    const char *buf_end =
        out_request->SearchBuffer + out_request->SearchBufferLen;
    const int c0_len = TextCountUtf8BytesFromChar(buf_begin, buf_end);
    const char *p = buf_begin + c0_len;
    for (; p < buf_end; p += c0_len)
      if (memcmp(buf_begin, p, (size_t)c0_len) != 0)
        break;
    const int single_char_count =
        (p == buf_end) ? (out_request->SearchBufferLen / c0_len) : 0;
    out_request->SingleCharMode =
        (single_char_count > 0 || data->SingleCharModeLock);
    out_request->SingleCharSize = (S8)c0_len;
    data->SingleCharModeLock |=
        (single_char_count >=
         TYPING_SELECT_SINGLE_CHAR_COUNT_FOR_LOCK); // From now on we stop
                                                    // search matching to lock
                                                    // to single char mode.
  }

  return out_request;
}

static int Strimatchlen(const char *s1, const char *s1_end, const char *s2) {
  int match_len = 0;
  while (s1 < s1_end && ToUpper(*s1++) == ToUpper(*s2++))
    match_len++;
  return match_len;
}

// Default handler for finding a result for typing-select. You may implement
// your own. You might want to display a tooltip to visualize the current
// request SearchBuffer When SingleCharMode is set:
// - it is better to NOT display a tooltip of other on-screen display indicator.
// - the index of the currently focused item is required.
//   if your SetNextItemSelectionData() values are indices, you can obtain it
//   from MultiSelectIO::NavIdItem, otherwise from
//   g.NavLastValidSelectionUserData.
int Gui::TypingSelectFindMatch(TypingSelectRequest *req, int items_count,
                               const char *(*get_item_name_func)(void *, int),
                               void *user_data, int nav_item_idx) {
  if (req == NULL ||
      req->SelectRequest == false) // Support NULL parameter so both calls can
                                   // be done from same spot.
    return -1;
  int idx = -1;
  if (req->SingleCharMode &&
      (req->Flags & TypingSelectFlags_AllowSingleCharMode))
    idx = TypingSelectFindNextSingleCharMatch(
        req, items_count, get_item_name_func, user_data, nav_item_idx);
  else
    idx = TypingSelectFindBestLeadingMatch(req, items_count, get_item_name_func,
                                           user_data);
  if (idx != -1)
    NavRestoreHighlightAfterMove();
  return idx;
}

// Special handling when a single character is repeated: perform search on a
// single letter and goes to next.
int Gui::TypingSelectFindNextSingleCharMatch(
    TypingSelectRequest *req, int items_count,
    const char *(*get_item_name_func)(void *, int), void *user_data,
    int nav_item_idx) {
  // FIXME: Assume selection user data is index. Would be extremely practical.
  // if (nav_item_idx == -1)
  //    nav_item_idx = (int)g.NavLastValidSelectionUserData;

  int first_match_idx = -1;
  bool return_next_match = false;
  for (int idx = 0; idx < items_count; idx++) {
    const char *item_name = get_item_name_func(user_data, idx);
    if (Strimatchlen(req->SearchBuffer, req->SearchBuffer + req->SingleCharSize,
                     item_name) < req->SingleCharSize)
      continue;
    if (return_next_match) // Return next matching item after current item.
      return idx;
    if (first_match_idx == -1 &&
        nav_item_idx == -1) // Return first match immediately if we don't have a
                            // nav_item_idx value.
      return idx;
    if (first_match_idx == -1) // Record first match for wrapping.
      first_match_idx = idx;
    if (nav_item_idx == idx) // Record that we encountering nav_item so we can
                             // return next match.
      return_next_match = true;
  }
  return first_match_idx; // First result
}

int Gui::TypingSelectFindBestLeadingMatch(
    TypingSelectRequest *req, int items_count,
    const char *(*get_item_name_func)(void *, int), void *user_data) {
  int longest_match_idx = -1;
  int longest_match_len = 0;
  for (int idx = 0; idx < items_count; idx++) {
    const char *item_name = get_item_name_func(user_data, idx);
    const int match_len = Strimatchlen(
        req->SearchBuffer, req->SearchBuffer + req->SearchBufferLen, item_name);
    if (match_len <= longest_match_len)
      continue;
    longest_match_idx = idx;
    longest_match_len = match_len;
    if (match_len == req->SearchBufferLen)
      break;
  }
  return longest_match_idx;
}

void Gui::DebugNodeTypingSelectState(TypingSelectState *data) {
#ifndef DISABLE_DEBUG_TOOLS
  Text("SearchBuffer = \"%s\"", data->SearchBuffer);
  Text("SingleCharMode = %d, Size = %d, Lock = %d",
       data->Request.SingleCharMode, data->Request.SingleCharSize,
       data->SingleCharModeLock);
  Text("LastRequest = time: %.2f, frame: %d", data->LastRequestTime,
       data->LastRequestFrame);
#else
  UNUSED(data);
#endif
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Multi-Select support
//-------------------------------------------------------------------------

void Gui::SetNextItemSelectionUserData(SelectionUserData selection_user_data) {
  // Note that flags will be cleared by ItemAdd(), so it's only useful for
  // Navigation code! This designed so widgets can also cheaply set this before
  // calling ItemAdd(), so we are not tied to MultiSelect api.
  Context &g = *GGui;
  g.NextItemData.ItemFlags |= ItemFlags_HasSelectionUserData;
  g.NextItemData.SelectionUserData = selection_user_data;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ListBox
//-------------------------------------------------------------------------
// - BeginListBox()
// - EndListBox()
// - ListBox()
//-------------------------------------------------------------------------

// This is essentially a thin wrapper to using BeginChild/EndChild with the
// ChildFlags_FrameStyle flag for stylistic changes + displaying a label.
// Tip: To have a list filling the entire window width, use size.x = -FLT_MIN
// and pass an non-visible label e.g. "##empty" Tip: If your vertical size is
// calculated from an item count (e.g. 10 * item_height) consider adding a
// fractional part to facilitate seeing scrolling boundaries (e.g. 10.25 *
// item_height).
bool Gui::BeginListBox(const char *label, const Vec2 &size_arg) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  const Style &style = g.Style;
  const ID id = GetID(label);
  const Vec2 label_size = CalcTextSize(label, NULL, true);

  // Size default to hold ~7.25 items.
  // Fractional number of items helps seeing that we can scroll down/up without
  // looking at scrollbar.
  Vec2 size = Trunc(CalcItemSize(size_arg, CalcItemWidth(),
                                 GetTextLineHeightWithSpacing() * 7.25f +
                                     style.FramePadding.y * 2.0f));
  Vec2 frame_size = Vec2(size.x, Max(size.y, label_size.y));
  Rect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
  Rect bb(frame_bb.Min,
          frame_bb.Max + Vec2(label_size.x > 0.0f
                                  ? style.ItemInnerSpacing.x + label_size.x
                                  : 0.0f,
                              0.0f));
  g.NextItemData.ClearFlags();

  if (!IsRectVisible(bb.Min, bb.Max)) {
    ItemSize(bb.GetSize(), style.FramePadding.y);
    ItemAdd(bb, 0, &frame_bb);
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume
                                   // those values
    return false;
  }

  // FIXME-OPT: We could omit the BeginGroup() if label_size.x == 0.0f but would
  // need to omit the EndGroup() as well.
  BeginGroup();
  if (label_size.x > 0.0f) {
    Vec2 label_pos = Vec2(frame_bb.Max.x + style.ItemInnerSpacing.x,
                          frame_bb.Min.y + style.FramePadding.y);
    RenderText(label_pos, label);
    window->DC.CursorMaxPos =
        Max(window->DC.CursorMaxPos, label_pos + label_size);
  }

  BeginChild(id, frame_bb.GetSize(), ChildFlags_FrameStyle);
  return true;
}

void Gui::EndListBox() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT((window->Flags & WindowFlags_ChildWindow) &&
         "Mismatched BeginListBox/EndListBox calls. Did you test the return "
         "value of BeginListBox?");
  UNUSED(window);

  EndChild();
  EndGroup(); // This is only required to be able to do IsItemXXX query on the
              // whole ListBox including label
}

bool Gui::ListBox(const char *label, int *current_item,
                  const char *const items[], int items_count,
                  int height_items) {
  const bool value_changed = ListBox(label, current_item, Items_ArrayGetter,
                                     (void *)items, items_count, height_items);
  return value_changed;
}

// This is merely a helper around BeginListBox(), EndListBox().
// Considering using those directly to submit custom data or store selection
// differently.
bool Gui::ListBox(const char *label, int *current_item,
                  const char *(*getter)(void *user_data, int idx),
                  void *user_data, int items_count, int height_in_items) {
  Context &g = *GGui;

  // Calculate size from "height_in_items"
  if (height_in_items < 0)
    height_in_items = Min(items_count, 7);
  float height_in_items_f = height_in_items + 0.25f;
  Vec2 size(0.0f, Trunc(GetTextLineHeightWithSpacing() * height_in_items_f +
                        g.Style.FramePadding.y * 2.0f));

  if (!BeginListBox(label, size))
    return false;

  // Assume all items have even height (= 1 line of text). If you need items of
  // different height, you can create a custom version of ListBox() in your code
  // without using the clipper.
  bool value_changed = false;
  ListClipper clipper;
  clipper.Begin(
      items_count,
      GetTextLineHeightWithSpacing()); // We know exactly our line height here
                                       // so we pass it as a minor optimization,
                                       // but generally you don't need to.
  while (clipper.Step())
    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
      const char *item_text = getter(user_data, i);
      if (item_text == NULL)
        item_text = "*Unknown item*";

      PushID(i);
      const bool item_selected = (i == *current_item);
      if (Selectable(item_text, item_selected)) {
        *current_item = i;
        value_changed = true;
      }
      if (item_selected)
        SetItemDefaultFocus();
      PopID();
    }
  EndListBox();

  if (value_changed)
    MarkItemEdited(g.LastItemData.ID);

  return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: PlotLines, PlotHistogram
//-------------------------------------------------------------------------
// - PlotEx() [Internal]
// - PlotLines()
// - PlotHistogram()
//-------------------------------------------------------------------------
// Plot/Graph widgets are not very good.
// Consider writing your own, or using a third-party one, see:
// - Plot https://github.com/epezent/implot
// - others https://github.com/ocornut/imgui/wiki/Useful-Extensions
//-------------------------------------------------------------------------

int Gui::PlotEx(PlotType plot_type, const char *label,
                float (*values_getter)(void *data, int idx), void *data,
                int values_count, int values_offset, const char *overlay_text,
                float scale_min, float scale_max, const Vec2 &size_arg) {
  Context &g = *GGui;
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return -1;

  const Style &style = g.Style;
  const ID id = window->GetID(label);

  const Vec2 label_size = CalcTextSize(label, NULL, true);
  const Vec2 frame_size = CalcItemSize(
      size_arg, CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);

  const Rect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
  const Rect inner_bb(frame_bb.Min + style.FramePadding,
                      frame_bb.Max - style.FramePadding);
  const Rect total_bb(frame_bb.Min,
                      frame_bb.Max +
                          Vec2(label_size.x > 0.0f
                                   ? style.ItemInnerSpacing.x + label_size.x
                                   : 0.0f,
                               0));
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, 0, &frame_bb))
    return -1;
  const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);

  // Determine scale from values if not specified
  if (scale_min == FLT_MAX || scale_max == FLT_MAX) {
    float v_min = FLT_MAX;
    float v_max = -FLT_MAX;
    for (int i = 0; i < values_count; i++) {
      const float v = values_getter(data, i);
      if (v != v) // Ignore NaN values
        continue;
      v_min = Min(v_min, v);
      v_max = Max(v_max, v);
    }
    if (scale_min == FLT_MAX)
      scale_min = v_min;
    if (scale_max == FLT_MAX)
      scale_max = v_max;
  }

  RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(Col_FrameBg), true,
              style.FrameRounding);

  const int values_count_min = (plot_type == PlotType_Lines) ? 2 : 1;
  int idx_hovered = -1;
  if (values_count >= values_count_min) {
    int res_w = Min((int)frame_size.x, values_count) +
                ((plot_type == PlotType_Lines) ? -1 : 0);
    int item_count = values_count + ((plot_type == PlotType_Lines) ? -1 : 0);

    // Tooltip on hover
    if (hovered && inner_bb.Contains(g.IO.MousePos)) {
      const float t = Clamp((g.IO.MousePos.x - inner_bb.Min.x) /
                                (inner_bb.Max.x - inner_bb.Min.x),
                            0.0f, 0.9999f);
      const int v_idx = (int)(t * item_count);
      ASSERT(v_idx >= 0 && v_idx < values_count);

      const float v0 =
          values_getter(data, (v_idx + values_offset) % values_count);
      const float v1 =
          values_getter(data, (v_idx + 1 + values_offset) % values_count);
      if (plot_type == PlotType_Lines)
        SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
      else if (plot_type == PlotType_Histogram)
        SetTooltip("%d: %8.4g", v_idx, v0);
      idx_hovered = v_idx;
    }

    const float t_step = 1.0f / (float)res_w;
    const float inv_scale =
        (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

    float v0 = values_getter(data, (0 + values_offset) % values_count);
    float t0 = 0.0f;
    Vec2 tp0 =
        Vec2(t0, 1.0f - Saturate((v0 - scale_min) *
                                 inv_scale)); // Point in the normalized space
                                              // of our target rectangle
    float histogram_zero_line_t =
        (scale_min * scale_max < 0.0f)
            ? (1 + scale_min * inv_scale)
            : (scale_min < 0.0f ? 0.0f
                                : 1.0f); // Where does the zero line stands

    const U32 col_base = GetColorU32(
        (plot_type == PlotType_Lines) ? Col_PlotLines : Col_PlotHistogram);
    const U32 col_hovered =
        GetColorU32((plot_type == PlotType_Lines) ? Col_PlotLinesHovered
                                                  : Col_PlotHistogramHovered);

    for (int n = 0; n < res_w; n++) {
      const float t1 = t0 + t_step;
      const int v1_idx = (int)(t0 * item_count + 0.5f);
      ASSERT(v1_idx >= 0 && v1_idx < values_count);
      const float v1 =
          values_getter(data, (v1_idx + values_offset + 1) % values_count);
      const Vec2 tp1 = Vec2(t1, 1.0f - Saturate((v1 - scale_min) * inv_scale));

      // NB: Draw calls are merged together by the DrawList system. Still, we
      // should render our batch are lower level to save a bit of CPU.
      Vec2 pos0 = Lerp(inner_bb.Min, inner_bb.Max, tp0);
      Vec2 pos1 = Lerp(inner_bb.Min, inner_bb.Max,
                       (plot_type == PlotType_Lines)
                           ? tp1
                           : Vec2(tp1.x, histogram_zero_line_t));
      if (plot_type == PlotType_Lines) {
        window->DrawList->AddLine(
            pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
      } else if (plot_type == PlotType_Histogram) {
        if (pos1.x >= pos0.x + 2.0f)
          pos1.x -= 1.0f;
        window->DrawList->AddRectFilled(
            pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
      }

      t0 = t1;
      tp0 = tp1;
    }
  }

  // Text overlay
  if (overlay_text)
    RenderTextClipped(
        Vec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y),
        frame_bb.Max, overlay_text, NULL, NULL, Vec2(0.5f, 0.0f));

  if (label_size.x > 0.0f)
    RenderText(Vec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y),
               label);

  // Return hovered index or -1 if none are hovered.
  // This is currently not exposed in the public API because we need a larger
  // redesign of the whole thing, but in the short-term we are making it
  // available in PlotEx().
  return idx_hovered;
}

struct PlotArrayGetterData {
  const float *Values;
  int Stride;

  PlotArrayGetterData(const float *values, int stride) {
    Values = values;
    Stride = stride;
  }
};

static float Plot_ArrayGetter(void *data, int idx) {
  PlotArrayGetterData *plot_data = (PlotArrayGetterData *)data;
  const float v =
      *(const float *)(const void *)((const unsigned char *)plot_data->Values +
                                     (size_t)idx * plot_data->Stride);
  return v;
}

void Gui::PlotLines(const char *label, const float *values, int values_count,
                    int values_offset, const char *overlay_text,
                    float scale_min, float scale_max, Vec2 graph_size,
                    int stride) {
  PlotArrayGetterData data(values, stride);
  PlotEx(PlotType_Lines, label, &Plot_ArrayGetter, (void *)&data, values_count,
         values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Gui::PlotLines(const char *label,
                    float (*values_getter)(void *data, int idx), void *data,
                    int values_count, int values_offset,
                    const char *overlay_text, float scale_min, float scale_max,
                    Vec2 graph_size) {
  PlotEx(PlotType_Lines, label, values_getter, data, values_count,
         values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Gui::PlotHistogram(const char *label, const float *values,
                        int values_count, int values_offset,
                        const char *overlay_text, float scale_min,
                        float scale_max, Vec2 graph_size, int stride) {
  PlotArrayGetterData data(values, stride);
  PlotEx(PlotType_Histogram, label, &Plot_ArrayGetter, (void *)&data,
         values_count, values_offset, overlay_text, scale_min, scale_max,
         graph_size);
}

void Gui::PlotHistogram(const char *label,
                        float (*values_getter)(void *data, int idx), void *data,
                        int values_count, int values_offset,
                        const char *overlay_text, float scale_min,
                        float scale_max, Vec2 graph_size) {
  PlotEx(PlotType_Histogram, label, values_getter, data, values_count,
         values_offset, overlay_text, scale_min, scale_max, graph_size);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Value helpers
// Those is not very useful, legacy API.
//-------------------------------------------------------------------------
// - Value()
//-------------------------------------------------------------------------

void Gui::Value(const char *prefix, bool b) {
  Text("%s: %s", prefix, (b ? "true" : "false"));
}

void Gui::Value(const char *prefix, int v) { Text("%s: %d", prefix, v); }

void Gui::Value(const char *prefix, unsigned int v) {
  Text("%s: %d", prefix, v);
}

void Gui::Value(const char *prefix, float v, const char *float_format) {
  if (float_format) {
    char fmt[64];
    FormatString(fmt, ARRAYSIZE(fmt), "%%s: %s", float_format);
    Text(fmt, prefix, v);
  } else {
    Text("%s: %.3f", prefix, v);
  }
}

//-------------------------------------------------------------------------
// [SECTION] MenuItem, BeginMenu, EndMenu, etc.
//-------------------------------------------------------------------------
// - MenuColumns [Internal]
// - BeginMenuBar()
// - EndMenuBar()
// - BeginMainMenuBar()
// - EndMainMenuBar()
// - BeginMenu()
// - EndMenu()
// - MenuItemEx() [Internal]
// - MenuItem()
//-------------------------------------------------------------------------

// Helpers for internal use
void MenuColumns::Update(float spacing, bool window_reappearing) {
  if (window_reappearing)
    memset(Widths, 0, sizeof(Widths));
  Spacing = (U16)spacing;
  CalcNextTotalWidth(true);
  memset(Widths, 0, sizeof(Widths));
  TotalWidth = NextTotalWidth;
  NextTotalWidth = 0;
}

void MenuColumns::CalcNextTotalWidth(bool update_offsets) {
  U16 offset = 0;
  bool want_spacing = false;
  for (int i = 0; i < ARRAYSIZE(Widths); i++) {
    U16 width = Widths[i];
    if (want_spacing && width > 0)
      offset += Spacing;
    want_spacing |= (width > 0);
    if (update_offsets) {
      if (i == 1) {
        OffsetLabel = offset;
      }
      if (i == 2) {
        OffsetShortcut = offset;
      }
      if (i == 3) {
        OffsetMark = offset;
      }
    }
    offset += width;
  }
  NextTotalWidth = offset;
}

float MenuColumns::DeclColumns(float w_icon, float w_label, float w_shortcut,
                               float w_mark) {
  Widths[0] = Max(Widths[0], (U16)w_icon);
  Widths[1] = Max(Widths[1], (U16)w_label);
  Widths[2] = Max(Widths[2], (U16)w_shortcut);
  Widths[3] = Max(Widths[3], (U16)w_mark);
  CalcNextTotalWidth(false);
  return (float)Max(TotalWidth, NextTotalWidth);
}

// FIXME: Provided a rectangle perhaps e.g. a BeginMenuBarEx() could be used
// anywhere.. Currently the main responsibility of this function being to setup
// clip-rect + horizontal layout + menu navigation layer. Ideally we also want
// this to be responsible for claiming space out of the main window scrolling
// rectangle, in which case WindowFlags_MenuBar will become unnecessary.
// Then later the same system could be used for multiple menu-bars, scrollbars,
// side-bars.
bool Gui::BeginMenuBar() {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;
  if (!(window->Flags & WindowFlags_MenuBar))
    return false;

  ASSERT(!window->DC.MenuBarAppending);
  BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a
                // group for that backup/restore
  PushID("##menubar");

  // We don't clip with current window clipping rectangle as it is already set
  // to the area below. However we clip with window full rect. We remove 1 worth
  // of rounding to Max.x to that text in long menus and small windows don't
  // tend to display over the lower-right rounded area, which looks particularly
  // glitchy.
  Rect bar_rect = window->MenuBarRect();
  Rect clip_rect(ROUND(bar_rect.Min.x + window->WindowBorderSize),
                 ROUND(bar_rect.Min.y + window->WindowBorderSize),
                 ROUND(Max(bar_rect.Min.x,
                           bar_rect.Max.x - Max(window->WindowRounding,
                                                window->WindowBorderSize))),
                 ROUND(bar_rect.Max.y));
  clip_rect.ClipWith(window->OuterRectClipped);
  PushClipRect(clip_rect.Min, clip_rect.Max, false);

  // We overwrite CursorMaxPos because BeginGroup sets it to CursorPos
  // (essentially the .EmitItem hack in EndMenuBar() would need something
  // analogous here, maybe a BeginGroupEx() with flags).
  window->DC.CursorPos = window->DC.CursorMaxPos =
      Vec2(bar_rect.Min.x + window->DC.MenuBarOffset.x,
           bar_rect.Min.y + window->DC.MenuBarOffset.y);
  window->DC.LayoutType = LayoutType_Horizontal;
  window->DC.IsSameLine = false;
  window->DC.NavLayerCurrent = NavLayer_Menu;
  window->DC.MenuBarAppending = true;
  AlignTextToFramePadding();
  return true;
}

void Gui::EndMenuBar() {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;
  Context &g = *GGui;

  // Nav: When a move request within one of our child menu failed, capture the
  // request to navigate among our siblings.
  if (NavMoveRequestButNoResultYet() &&
      (g.NavMoveDir == Dir_Left || g.NavMoveDir == Dir_Right) &&
      (g.NavWindow->Flags & WindowFlags_ChildMenu)) {
    // Try to find out if the request is for one of our child menu
    Window *nav_earliest_child = g.NavWindow;
    while (nav_earliest_child->ParentWindow &&
           (nav_earliest_child->ParentWindow->Flags & WindowFlags_ChildMenu))
      nav_earliest_child = nav_earliest_child->ParentWindow;
    if (nav_earliest_child->ParentWindow == window &&
        nav_earliest_child->DC.ParentLayoutType == LayoutType_Horizontal &&
        (g.NavMoveFlags & NavMoveFlags_Forwarded) == 0) {
      // To do so we claim focus back, restore NavId and then process the
      // movement request for yet another frame. This involve a one-frame delay
      // which isn't very problematic in this situation. We could remove it by
      // scoring in advance for multiple window (probably not worth bothering)
      const NavLayer layer = NavLayer_Menu;
      ASSERT(window->DC.NavLayersActiveMaskNext &
             (1 << layer)); // Sanity check (FIXME: Seems unnecessary)
      FocusWindow(window);
      SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
      g.NavDisableHighlight = true; // Hide highlight for the current frame so
                                    // we don't see the intermediary selection.
      g.NavDisableMouseHover = g.NavMousePosDirty = true;
      NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags,
                            g.NavMoveScrollFlags); // Repeat
    }
  }

  MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning
                               // C6011: Dereferencing NULL pointer 'window'"
  ASSERT(window->Flags & WindowFlags_MenuBar);
  ASSERT(window->DC.MenuBarAppending);
  PopClipRect();
  PopID();
  window->DC.MenuBarOffset.x =
      window->DC.CursorPos.x -
      window->Pos.x; // Save horizontal position so next append can reuse it.
                     // This is kinda equivalent to a per-layer CursorPos.

  // FIXME: Extremely confusing, cleanup by (a) working on WorkRect stack system
  // (b) not using a Group confusingly here.
  GroupData &group_data = g.GroupStack.back();
  group_data.EmitItem = false;
  Vec2 restore_cursor_max_pos = group_data.BackupCursorMaxPos;
  window->DC.IdealMaxPos.x =
      Max(window->DC.IdealMaxPos.x,
          window->DC.CursorMaxPos.x -
              window->Scroll
                  .x); // Convert ideal extents for scrolling layer equivalent.
  EndGroup(); // Restore position on layer 0 // FIXME: Misleading to use a group
              // for that backup/restore
  window->DC.LayoutType = LayoutType_Vertical;
  window->DC.IsSameLine = false;
  window->DC.NavLayerCurrent = NavLayer_Main;
  window->DC.MenuBarAppending = false;
  window->DC.CursorMaxPos = restore_cursor_max_pos;
}

// Important: calling order matters!
// FIXME: Somehow overlapping with docking tech.
// FIXME: The "rect-cut" aspect of this could be formalized into a lower-level
// helper (rect-cut: https://halt.software/dead-simple-layouts)
bool Gui::BeginViewportSideBar(const char *name, Viewport *viewport_p, Dir dir,
                               float axis_size, WindowFlags window_flags) {
  ASSERT(dir != Dir_None);

  Window *bar_window = FindWindowByName(name);
  ViewportP *viewport =
      (ViewportP *)(void *)(viewport_p ? viewport_p : GetMainViewport());
  if (bar_window == NULL || bar_window->BeginCount == 0) {
    // Calculate and set window size/position
    Rect avail_rect = viewport->GetBuildWorkRect();
    Axis axis = (dir == Dir_Up || dir == Dir_Down) ? Axis_Y : Axis_X;
    Vec2 pos = avail_rect.Min;
    if (dir == Dir_Right || dir == Dir_Down)
      pos[axis] = avail_rect.Max[axis] - axis_size;
    Vec2 size = avail_rect.GetSize();
    size[axis] = axis_size;
    SetNextWindowPos(pos);
    SetNextWindowSize(size);

    // Report our size into work area (for next frame) using actual window size
    if (dir == Dir_Up || dir == Dir_Left)
      viewport->BuildWorkOffsetMin[axis] += axis_size;
    else if (dir == Dir_Down || dir == Dir_Right)
      viewport->BuildWorkOffsetMax[axis] -= axis_size;
  }

  window_flags |= WindowFlags_NoTitleBar | WindowFlags_NoResize |
                  WindowFlags_NoMove | WindowFlags_NoDocking;
  SetNextWindowViewport(
      viewport->ID); // Enforce viewport so we don't create our own viewport
                     // when ConfigFlags_ViewportsNoMerge is set.
  PushStyleVar(StyleVar_WindowRounding, 0.0f);
  PushStyleVar(StyleVar_WindowMinSize,
               Vec2(0, 0)); // Lift normal size constraint
  bool is_open = Begin(name, NULL, window_flags);
  PopStyleVar(2);

  return is_open;
}

bool Gui::BeginMainMenuBar() {
  Context &g = *GGui;
  ViewportP *viewport = (ViewportP *)(void *)GetMainViewport();

  // Notify of viewport change so GetFrameHeight() can be accurate in case of
  // DPI change
  SetCurrentViewport(NULL, viewport);

  // For the main menu bar, which cannot be moved, we honor
  // g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
  // FIXME: This could be generalized as an opt-in way to clamp
  // window->DC.CursorStartPos to avoid SafeArea?
  // FIXME: Consider removing support for safe area down the line... it's messy.
  // Nowadays consoles have support for TV calibration in OS settings.
  g.NextWindowData.MenuBarOffsetMinVal = Vec2(
      g.Style.DisplaySafeAreaPadding.x,
      Max(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
  WindowFlags window_flags = WindowFlags_NoScrollbar |
                             WindowFlags_NoSavedSettings | WindowFlags_MenuBar;
  float height = GetFrameHeight();
  bool is_open = BeginViewportSideBar("##MainMenuBar", viewport, Dir_Up, height,
                                      window_flags);
  g.NextWindowData.MenuBarOffsetMinVal = Vec2(0.0f, 0.0f);

  if (is_open)
    BeginMenuBar();
  else
    End();
  return is_open;
}

void Gui::EndMainMenuBar() {
  EndMenuBar();

  // When the user has left the menu layer (typically: closed menus through
  // activation of an item), we restore focus to the previous window
  // FIXME: With this strategy we won't be able to restore a NULL focus.
  Context &g = *GGui;
  if (g.CurrentWindow == g.NavWindow && g.NavLayer == NavLayer_Main &&
      !g.NavAnyRequest)
    FocusTopMostWindowUnderOne(g.NavWindow, NULL, NULL,
                               FocusRequestFlags_UnlessBelowModal |
                                   FocusRequestFlags_RestoreFocusedChild);

  End();
}

static bool IsRootOfOpenMenuSet() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if ((g.OpenPopupStack.Size <= g.BeginPopupStack.Size) ||
      (window->Flags & WindowFlags_ChildMenu))
    return false;

  // Initially we used 'upper_popup->OpenParentId == window->IDStack.back()' to
  // differentiate multiple menu sets from each others (e.g. inside menu bar vs
  // loose menu items) based on parent ID. This would however prevent the use of
  // e.g. PushID() user code submitting menus. Previously this worked between
  // popup and a first child menu because the first child menu always had the
  // _ChildWindow flag, making hovering on parent popup possible while first
  // child menu was focused - but this was generally a bug with other side
  // effects. Instead we don't treat Popup specifically (in order to
  // consistently support menu features in them), maybe the first child menu of
  // a Popup doesn't have the _ChildWindow flag, and we rely on this
  // IsRootOfOpenMenuSet() check to allow hovering between root window/popup and
  // first child menu. In the end, lack of ID check made it so we could no
  // longer differentiate between separate menu sets. To compensate for that, we
  // at least check parent window nav layer. This fixes the most common case of
  // menu opening on hover when moving between window content and menu bar.
  // Multiple different menu sets in same nav layer would still open on hover,
  // but that should be a lesser problem, because if such menus are close in
  // proximity in window content then it won't feel weird and if they are far
  // apart it likely won't be a problem anyone runs into.
  const PopupData *upper_popup = &g.OpenPopupStack[g.BeginPopupStack.Size];
  if (window->DC.NavLayerCurrent != upper_popup->ParentNavLayer)
    return false;
  return upper_popup->Window &&
         (upper_popup->Window->Flags & WindowFlags_ChildMenu) &&
         Gui::IsWindowChildOf(upper_popup->Window, window, true, false);
}

bool Gui::BeginMenuEx(const char *label, const char *icon, bool enabled) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  const Style &style = g.Style;
  const ID id = window->GetID(label);
  bool menu_is_open = IsPopupOpen(id, PopupFlags_None);

  // Sub-menus are ChildWindow so that mouse can be hovering across them
  // (otherwise top-most popup menu would steal focus and not allow hovering on
  // parent menu) The first menu in a hierarchy isn't so hovering doesn't get
  // across (otherwise e.g. resizing borders with
  // ButtonFlags_FlattenChildren would react), but top-most BeginMenu()
  // will bypass that limitation.
  WindowFlags window_flags =
      WindowFlags_ChildMenu | WindowFlags_AlwaysAutoResize |
      WindowFlags_NoMove | WindowFlags_NoTitleBar |
      WindowFlags_NoSavedSettings | WindowFlags_NoNavFocus;
  if (window->Flags & WindowFlags_ChildMenu)
    window_flags |= WindowFlags_ChildWindow;

  // If a menu with same the ID was already submitted, we will append to it,
  // matching the behavior of Begin(). We are relying on a O(N) search - so O(N
  // log N) over the frame - which seems like the most efficient for the
  // expected small amount of BeginMenu() calls per frame. If somehow this is
  // ever becoming a problem we can switch to use e.g. Storage mapping key
  // to last frame used.
  if (g.MenusIdSubmittedThisFrame.contains(id)) {
    if (menu_is_open)
      menu_is_open = BeginPopupEx(
          id, window_flags); // menu_is_open can be 'false' when the popup is
                             // completely clipped (e.g. zero size display)
    else
      g.NextWindowData.ClearFlags(); // we behave like Begin() and need to
                                     // consume those values
    return menu_is_open;
  }

  // Tag menu as used. Next time BeginMenu() with same ID is called it will
  // append to existing menu
  g.MenusIdSubmittedThisFrame.push_back(id);

  Vec2 label_size = CalcTextSize(label, NULL, true);

  // Odd hack to allow hovering across menus of a same menu-set (otherwise we
  // wouldn't be able to hover parent without always being a Child window) This
  // is only done for items for the menu set and not the full parent window.
  const bool menuset_is_open = IsRootOfOpenMenuSet();
  if (menuset_is_open)
    PushItemFlag(ItemFlags_NoWindowHoverableCheck, true);

  // The reference position stored in popup_pos will be used by Begin() to find
  // a suitable position for the child menu, However the final position is going
  // to be different! It is chosen by FindBestWindowPosForPopup(). e.g. Menus
  // tend to overlap each other horizontally to amplify relative Z-ordering.
  Vec2 popup_pos, pos = window->DC.CursorPos;
  PushID(label);
  if (!enabled)
    BeginDisabled();
  const MenuColumns *offsets = &window->DC.MenuColumns;
  bool pressed;

  // We use SelectableFlags_NoSetKeyOwner to allow down on one menu item,
  // move, up on another.
  const SelectableFlags selectable_flags =
      SelectableFlags_NoHoldingActiveID | SelectableFlags_NoSetKeyOwner |
      SelectableFlags_SelectOnClick | SelectableFlags_DontClosePopups;
  if (window->DC.LayoutType == LayoutType_Horizontal) {
    // Menu inside an horizontal menu bar
    // Selectable extend their highlight by half ItemSpacing in each direction.
    // For ChildMenu, the popup position will be overwritten by the call to
    // FindBestWindowPosForPopup() in Begin()
    popup_pos = Vec2(pos.x - 1.0f - TRUNC(style.ItemSpacing.x * 0.5f),
                     pos.y - style.FramePadding.y + window->MenuBarHeight());
    window->DC.CursorPos.x += TRUNC(style.ItemSpacing.x * 0.5f);
    PushStyleVar(StyleVar_ItemSpacing,
                 Vec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
    float w = label_size.x;
    Vec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel,
                  window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    pressed =
        Selectable("", menu_is_open, selectable_flags, Vec2(w, label_size.y));
    RenderText(text_pos, label);
    PopStyleVar();
    window->DC.CursorPos.x += TRUNC(
        style.ItemSpacing.x *
        (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when
                         // Selectable() did a SameLine(). It would also work to
                         // call SameLine() ourselves after the PopStyleVar().
  } else {
    // Menu inside a regular/vertical menu
    // (In a typical menu window where all items are BeginMenu() or MenuItem()
    // calls, extra_w will always be 0.0f.
    //  Only when they are other items sticking out we're going to add spacing,
    //  yet only register minimum width into the layout system.
    popup_pos = Vec2(pos.x, pos.y - style.WindowPadding.y);
    float icon_w = (icon && icon[0]) ? CalcTextSize(icon, NULL).x : 0.0f;
    float checkmark_w = TRUNC(g.FontSize * 1.20f);
    float min_w = window->DC.MenuColumns.DeclColumns(
        icon_w, label_size.x, 0.0f, checkmark_w); // Feedback to next frame
    float extra_w = Max(0.0f, GetContentRegionAvail().x - min_w);
    Vec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel,
                  window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    pressed = Selectable("", menu_is_open,
                         selectable_flags | SelectableFlags_SpanAvailWidth,
                         Vec2(min_w, label_size.y));
    RenderText(text_pos, label);
    if (icon_w > 0.0f)
      RenderText(pos + Vec2(offsets->OffsetIcon, 0.0f), icon);
    RenderArrow(
        window->DrawList,
        pos + Vec2(offsets->OffsetMark + extra_w + g.FontSize * 0.30f, 0.0f),
        GetColorU32(Col_Text), Dir_Right);
  }
  if (!enabled)
    EndDisabled();

  const bool hovered =
      (g.HoveredId == id) && enabled && !g.NavDisableMouseHover;
  if (menuset_is_open)
    PopItemFlag();

  bool want_open = false;
  bool want_close = false;
  if (window->DC.LayoutType ==
      LayoutType_Vertical) // (window->Flags &
                           // (WindowFlags_Popup|WindowFlags_ChildMenu))
  {
    // Close menu when not hovering it anymore unless we are moving roughly in
    // the direction of the menu Implement
    // http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to
    // avoid using timers, so menus feels more reactive.
    bool moving_toward_child_menu = false;
    PopupData *child_popup = (g.BeginPopupStack.Size < g.OpenPopupStack.Size)
                                 ? &g.OpenPopupStack[g.BeginPopupStack.Size]
                                 : NULL; // Popup candidate (testing below)
    Window *child_menu_window = (child_popup && child_popup->Window &&
                                 child_popup->Window->ParentWindow == window)
                                    ? child_popup->Window
                                    : NULL;
    if (g.HoveredWindow == window && child_menu_window != NULL) {
      const float ref_unit = g.FontSize; // FIXME-DPI
      const float child_dir =
          (window->Pos.x < child_menu_window->Pos.x) ? 1.0f : -1.0f;
      const Rect next_window_rect = child_menu_window->Rect();
      Vec2 ta = (g.IO.MousePos - g.IO.MouseDelta);
      Vec2 tb = (child_dir > 0.0f) ? next_window_rect.GetTL()
                                   : next_window_rect.GetTR();
      Vec2 tc = (child_dir > 0.0f) ? next_window_rect.GetBL()
                                   : next_window_rect.GetBR();
      const float pad_farmost_h =
          Clamp(Fabs(ta.x - tb.x) * 0.30f, ref_unit * 0.5f,
                ref_unit * 2.5f); // Add a bit of extra slack.
      ta.x += child_dir * -0.5f;
      tb.x += child_dir * ref_unit;
      tc.x += child_dir * ref_unit;
      tb.y = ta.y +
             Max((tb.y - pad_farmost_h) - ta.y,
                 -ref_unit * 8.0f); // Triangle has maximum height to limit the
                                    // slope and the bias toward large sub-menus
      tc.y = ta.y + Min((tc.y + pad_farmost_h) - ta.y, +ref_unit * 8.0f);
      moving_toward_child_menu =
          TriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
      // GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc,
      // moving_toward_child_menu ? COL32(0,128,0,128) :
      // COL32(128,0,0,128)); // [DEBUG]
    }

    // The 'HovereWindow == window' check creates an inconsistency (e.g. moving
    // away from menu slowly tends to hit same window, whereas moving away fast
    // does not) But we also need to not close the top-menu menu when moving
    // over void. Perhaps we should extend the triangle check to a larger
    // polygon. (Remember to test this on BeginPopup("A")->BeginMenu("B")
    // sequence which behaves slightly differently as B isn't a Child of A and
    // hovering isn't shared.)
    if (menu_is_open && !hovered && g.HoveredWindow == window &&
        !moving_toward_child_menu && !g.NavDisableMouseHover && g.ActiveId == 0)
      want_close = true;

    // Open
    // (note: at this point 'hovered' actually includes the NavDisableMouseHover
    // == false test)
    if (!menu_is_open && pressed) // Click/activate to open
      want_open = true;
    else if (!menu_is_open && hovered &&
             !moving_toward_child_menu) // Hover to open
      want_open = true;
    else if (!menu_is_open && hovered && g.HoveredIdTimer >= 0.30f &&
             g.MouseStationaryTimer >= 0.30f) // Hover to open (timer fallback)
      want_open = true;
    if (g.NavId == id && g.NavMoveDir == Dir_Right) // Nav-Right to open
    {
      want_open = true;
      NavMoveRequestCancel();
    }
  } else {
    // Menu bar
    if (menu_is_open && pressed &&
        menuset_is_open) // Click an open menu again to close it
    {
      want_close = true;
      want_open = menu_is_open = false;
    } else if (pressed || (hovered && menuset_is_open &&
                           !menu_is_open)) // First click to open, then hover to
                                           // open others
    {
      want_open = true;
    } else if (g.NavId == id && g.NavMoveDir == Dir_Down) // Nav-Down to open
    {
      want_open = true;
      NavMoveRequestCancel();
    }
  }

  if (!enabled) // explicitly close if an open menu becomes disabled, facilitate
                // users code a lot in pattern such as 'if (BeginMenu("options",
                // has_object)) { ..use object.. }'
    want_close = true;
  if (want_close && IsPopupOpen(id, PopupFlags_None))
    ClosePopupToLevel(g.BeginPopupStack.Size, true);

  TEST_ENGINE_ITEM_INFO(id, label,
                        g.LastItemData.StatusFlags | ItemStatusFlags_Openable |
                            (menu_is_open ? ItemStatusFlags_Opened : 0));
  PopID();

  if (want_open && !menu_is_open &&
      g.OpenPopupStack.Size > g.BeginPopupStack.Size) {
    // Don't reopen/recycle same menu level in the same frame, first close the
    // other menu and yield for a frame.
    OpenPopup(label);
  } else if (want_open) {
    menu_is_open = true;
    OpenPopup(label);
  }

  if (menu_is_open) {
    LastItemData last_item_in_parent = g.LastItemData;
    SetNextWindowPos(popup_pos,
                     Cond_Always); // Note: misleading: the value will serve as
                                   // reference for FindBestWindowPosForPopup(),
                                   // not actual pos.
    PushStyleVar(StyleVar_ChildRounding,
                 style.PopupRounding); // First level will use _PopupRounding,
                                       // subsequent will use _ChildRounding
    menu_is_open = BeginPopupEx(
        id, window_flags); // menu_is_open can be 'false' when the popup is
                           // completely clipped (e.g. zero size display)
    PopStyleVar();
    if (menu_is_open) {
      // Restore LastItemData so IsItemXXXX functions can work after
      // BeginMenu()/EndMenu() (This fixes using IsItemClicked() and
      // IsItemHovered(), but IsItemHovered() also relies on its support for
      // ItemFlags_NoWindowHoverableCheck)
      g.LastItemData = last_item_in_parent;
      if (g.HoveredWindow == window)
        g.LastItemData.StatusFlags |= ItemStatusFlags_HoveredWindow;
    }
  } else {
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume
                                   // those values
  }

  return menu_is_open;
}

bool Gui::BeginMenu(const char *label, bool enabled) {
  return BeginMenuEx(label, NULL, enabled);
}

void Gui::EndMenu() {
  // Nav: When a left move request our menu failed, close ourselves.
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  ASSERT(window->Flags &
         WindowFlags_Popup); // Mismatched BeginMenu()/EndMenu() calls
  Window *parent_window =
      window->ParentWindow; // Should always be != NULL is we passed assert.
  if (window->BeginCount == window->BeginCountPreviousFrame)
    if (g.NavMoveDir == Dir_Left && NavMoveRequestButNoResultYet())
      if (g.NavWindow && (g.NavWindow->RootWindowForNav == window) &&
          parent_window->DC.LayoutType == LayoutType_Vertical) {
        ClosePopupToLevel(g.BeginPopupStack.Size - 1, true);
        NavMoveRequestCancel();
      }

  EndPopup();
}

bool Gui::MenuItemEx(const char *label, const char *icon, const char *shortcut,
                     bool selected, bool enabled) {
  Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  Context &g = *GGui;
  Style &style = g.Style;
  Vec2 pos = window->DC.CursorPos;
  Vec2 label_size = CalcTextSize(label, NULL, true);

  // See BeginMenuEx() for comments about this.
  const bool menuset_is_open = IsRootOfOpenMenuSet();
  if (menuset_is_open)
    PushItemFlag(ItemFlags_NoWindowHoverableCheck, true);

  // We've been using the equivalent of SelectableFlags_SetNavIdOnHover on
  // all Selectable() since early Nav system days (commit 43ee5d73), but I am
  // unsure whether this should be kept at all. For now moved it to be an opt-in
  // feature used by menus only.
  bool pressed;
  PushID(label);
  if (!enabled)
    BeginDisabled();

  // We use SelectableFlags_NoSetKeyOwner to allow down on one menu item,
  // move, up on another.
  const SelectableFlags selectable_flags = SelectableFlags_SelectOnRelease |
                                           SelectableFlags_NoSetKeyOwner |
                                           SelectableFlags_SetNavIdOnHover;
  const MenuColumns *offsets = &window->DC.MenuColumns;
  if (window->DC.LayoutType == LayoutType_Horizontal) {
    // Mimic the exact layout spacing of BeginMenu() to allow MenuItem() inside
    // a menu bar, which is a little misleading but may be useful Note that in
    // this situation: we don't render the shortcut, we render a highlight
    // instead of the selected tick mark.
    float w = label_size.x;
    window->DC.CursorPos.x += TRUNC(style.ItemSpacing.x * 0.5f);
    Vec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel,
                  window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    PushStyleVar(StyleVar_ItemSpacing,
                 Vec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
    pressed = Selectable("", selected, selectable_flags, Vec2(w, 0.0f));
    PopStyleVar();
    if (g.LastItemData.StatusFlags & ItemStatusFlags_Visible)
      RenderText(text_pos, label);
    window->DC.CursorPos.x += TRUNC(
        style.ItemSpacing.x *
        (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when
                         // Selectable() did a SameLine(). It would also work to
                         // call SameLine() ourselves after the PopStyleVar().
  } else {
    // Menu item inside a vertical menu
    // (In a typical menu window where all items are BeginMenu() or MenuItem()
    // calls, extra_w will always be 0.0f.
    //  Only when they are other items sticking out we're going to add spacing,
    //  yet only register minimum width into the layout system.
    float icon_w = (icon && icon[0]) ? CalcTextSize(icon, NULL).x : 0.0f;
    float shortcut_w =
        (shortcut && shortcut[0]) ? CalcTextSize(shortcut, NULL).x : 0.0f;
    float checkmark_w = TRUNC(g.FontSize * 1.20f);
    float min_w = window->DC.MenuColumns.DeclColumns(
        icon_w, label_size.x, shortcut_w,
        checkmark_w); // Feedback for next frame
    float stretch_w = Max(0.0f, GetContentRegionAvail().x - min_w);
    pressed =
        Selectable("", false, selectable_flags | SelectableFlags_SpanAvailWidth,
                   Vec2(min_w, label_size.y));
    if (g.LastItemData.StatusFlags & ItemStatusFlags_Visible) {
      RenderText(pos + Vec2(offsets->OffsetLabel, 0.0f), label);
      if (icon_w > 0.0f)
        RenderText(pos + Vec2(offsets->OffsetIcon, 0.0f), icon);
      if (shortcut_w > 0.0f) {
        PushStyleColor(Col_Text, style.Colors[Col_TextDisabled]);
        RenderText(pos + Vec2(offsets->OffsetShortcut + stretch_w, 0.0f),
                   shortcut, NULL, false);
        PopStyleColor();
      }
      if (selected)
        RenderCheckMark(
            window->DrawList,
            pos + Vec2(offsets->OffsetMark + stretch_w + g.FontSize * 0.40f,
                       g.FontSize * 0.134f * 0.5f),
            GetColorU32(Col_Text), g.FontSize * 0.866f);
    }
  }
  TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label,
                        g.LastItemData.StatusFlags | ItemStatusFlags_Checkable |
                            (selected ? ItemStatusFlags_Checked : 0));
  if (!enabled)
    EndDisabled();
  PopID();
  if (menuset_is_open)
    PopItemFlag();

  return pressed;
}

bool Gui::MenuItem(const char *label, const char *shortcut, bool selected,
                   bool enabled) {
  return MenuItemEx(label, NULL, shortcut, selected, enabled);
}

bool Gui::MenuItem(const char *label, const char *shortcut, bool *p_selected,
                   bool enabled) {
  if (MenuItemEx(label, NULL, shortcut, p_selected ? *p_selected : false,
                 enabled)) {
    if (p_selected)
      *p_selected = !*p_selected;
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
//-------------------------------------------------------------------------
// - BeginTabBar()
// - BeginTabBarEx() [Internal]
// - EndTabBar()
// - TabBarLayout() [Internal]
// - TabBarCalcTabID() [Internal]
// - TabBarCalcMaxTabWidth() [Internal]
// - TabBarFindTabById() [Internal]
// - TabBarFindTabByOrder() [Internal]
// - TabBarFindMostRecentlySelectedTabForActiveWindow() [Internal]
// - TabBarGetCurrentTab() [Internal]
// - TabBarGetTabName() [Internal]
// - TabBarAddTab() [Internal]
// - TabBarRemoveTab() [Internal]
// - TabBarCloseTab() [Internal]
// - TabBarScrollClamp() [Internal]
// - TabBarScrollToTab() [Internal]
// - TabBarQueueFocus() [Internal]
// - TabBarQueueReorder() [Internal]
// - TabBarProcessReorderFromMousePos() [Internal]
// - TabBarProcessReorder() [Internal]
// - TabBarScrollingButtons() [Internal]
// - TabBarTabListPopupButton() [Internal]
//-------------------------------------------------------------------------

struct TabBarSection {
  int TabCount;  // Number of tabs in this section.
  float Width;   // Sum of width of tabs in this section (after shrinking down)
  float Spacing; // Horizontal spacing at the end of the section.

  TabBarSection() { memset(this, 0, sizeof(*this)); }
};

namespace Gui {
static void TabBarLayout(TabBar *tab_bar);
static U32 TabBarCalcTabID(TabBar *tab_bar, const char *label,
                           Window *docked_window);
static float TabBarCalcMaxTabWidth();
static float TabBarScrollClamp(TabBar *tab_bar, float scrolling);
static void TabBarScrollToTab(TabBar *tab_bar, ID tab_id,
                              TabBarSection *sections);
static TabItem *TabBarScrollingButtons(TabBar *tab_bar);
static TabItem *TabBarTabListPopupButton(TabBar *tab_bar);
} // namespace Gui

TabBar::TabBar() {
  memset(this, 0, sizeof(*this));
  CurrFrameVisible = PrevFrameVisible = -1;
  LastTabItemIdx = -1;
}

static inline int TabItemGetSectionIdx(const TabItem *tab) {
  return (tab->Flags & TabItemFlags_Leading)    ? 0
         : (tab->Flags & TabItemFlags_Trailing) ? 2
                                                : 1;
}

static int CDECL TabItemComparerBySection(const void *lhs, const void *rhs) {
  const TabItem *a = (const TabItem *)lhs;
  const TabItem *b = (const TabItem *)rhs;
  const int a_section = TabItemGetSectionIdx(a);
  const int b_section = TabItemGetSectionIdx(b);
  if (a_section != b_section)
    return a_section - b_section;
  return (int)(a->IndexDuringLayout - b->IndexDuringLayout);
}

static int CDECL TabItemComparerByBeginOrder(const void *lhs, const void *rhs) {
  const TabItem *a = (const TabItem *)lhs;
  const TabItem *b = (const TabItem *)rhs;
  return (int)(a->BeginOrder - b->BeginOrder);
}

static TabBar *GetTabBarFromTabBarRef(const PtrOrIndex &ref) {
  Context &g = *GGui;
  return ref.Ptr ? (TabBar *)ref.Ptr : g.TabBars.GetByIndex(ref.Index);
}

static PtrOrIndex GetTabBarRefFromTabBar(TabBar *tab_bar) {
  Context &g = *GGui;
  if (g.TabBars.Contains(tab_bar))
    return PtrOrIndex(g.TabBars.GetIndex(tab_bar));
  return PtrOrIndex(tab_bar);
}

bool Gui::BeginTabBar(const char *str_id, TabBarFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  ID id = window->GetID(str_id);
  TabBar *tab_bar = g.TabBars.GetOrAddByKey(id);
  Rect tab_bar_bb = Rect(
      window->DC.CursorPos.x, window->DC.CursorPos.y, window->WorkRect.Max.x,
      window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
  tab_bar->ID = id;
  tab_bar->SeparatorMinX =
      tab_bar->BarRect.Min.x - TRUNC(window->WindowPadding.x * 0.5f);
  tab_bar->SeparatorMaxX =
      tab_bar->BarRect.Max.x + TRUNC(window->WindowPadding.x * 0.5f);
  return BeginTabBarEx(tab_bar, tab_bar_bb, flags | TabBarFlags_IsFocused);
}

bool Gui::BeginTabBarEx(TabBar *tab_bar, const Rect &tab_bar_bb,
                        TabBarFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  ASSERT(tab_bar->ID != 0);
  if ((flags & TabBarFlags_DockNode) == 0)
    PushOverrideID(tab_bar->ID);

  // Add to stack
  g.CurrentTabBarStack.push_back(GetTabBarRefFromTabBar(tab_bar));
  g.CurrentTabBar = tab_bar;

  // Append with multiple BeginTabBar()/EndTabBar() pairs.
  tab_bar->BackupCursorPos = window->DC.CursorPos;
  if (tab_bar->CurrFrameVisible == g.FrameCount) {
    window->DC.CursorPos = Vec2(tab_bar->BarRect.Min.x,
                                tab_bar->BarRect.Max.y + tab_bar->ItemSpacingY);
    tab_bar->BeginCount++;
    return true;
  }

  // Ensure correct ordering when toggling TabBarFlags_Reorderable flag, or
  // when a new tab was added while being not reorderable
  if ((flags & TabBarFlags_Reorderable) !=
          (tab_bar->Flags & TabBarFlags_Reorderable) ||
      (tab_bar->TabsAddedNew && !(flags & TabBarFlags_Reorderable)))
    if ((flags & TabBarFlags_DockNode) ==
        0) // FIXME: TabBar with DockNode can now be hybrid
      Qsort(tab_bar->Tabs.Data, tab_bar->Tabs.Size, sizeof(TabItem),
            TabItemComparerByBeginOrder);
  tab_bar->TabsAddedNew = false;

  // Flags
  if ((flags & TabBarFlags_FittingPolicyMask_) == 0)
    flags |= TabBarFlags_FittingPolicyDefault_;

  tab_bar->Flags = flags;
  tab_bar->BarRect = tab_bar_bb;
  tab_bar->WantLayout =
      true; // Layout will be done on the first call to ItemTab()
  tab_bar->PrevFrameVisible = tab_bar->CurrFrameVisible;
  tab_bar->CurrFrameVisible = g.FrameCount;
  tab_bar->PrevTabsContentsHeight = tab_bar->CurrTabsContentsHeight;
  tab_bar->CurrTabsContentsHeight = 0.0f;
  tab_bar->ItemSpacingY = g.Style.ItemSpacing.y;
  tab_bar->FramePadding = g.Style.FramePadding;
  tab_bar->TabsActiveCount = 0;
  tab_bar->LastTabItemIdx = -1;
  tab_bar->BeginCount = 1;

  // Set cursor pos in a way which only be used in the off-chance the user
  // erroneously submits item before BeginTabItem(): items will overlap
  window->DC.CursorPos = Vec2(tab_bar->BarRect.Min.x,
                              tab_bar->BarRect.Max.y + tab_bar->ItemSpacingY);

  // Draw separator
  // (it would be misleading to draw this in EndTabBar() suggesting that it may
  // be drawn over tabs, as tab bar are appendable)
  const U32 col = GetColorU32(
      (flags & TabBarFlags_IsFocused) ? Col_TabActive : Col_TabUnfocusedActive);
  if (g.Style.TabBarBorderSize > 0.0f) {
    const float y = tab_bar->BarRect.Max.y;
    window->DrawList->AddRectFilled(
        Vec2(tab_bar->SeparatorMinX, y - g.Style.TabBarBorderSize),
        Vec2(tab_bar->SeparatorMaxX, y), col);
  }
  return true;
}

void Gui::EndTabBar() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ASSERT_USER_ERROR(tab_bar != NULL, "Mismatched BeginTabBar()/EndTabBar()!");
    return;
  }

  // Fallback in case no TabItem have been submitted
  if (tab_bar->WantLayout)
    TabBarLayout(tab_bar);

  // Restore the last visible height if no tab is visible, this reduce vertical
  // flicker/movement when a tabs gets removed without calling
  // SetTabItemClosed().
  const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
  if (tab_bar->VisibleTabWasSubmitted || tab_bar->VisibleTabId == 0 ||
      tab_bar_appearing) {
    tab_bar->CurrTabsContentsHeight =
        Max(window->DC.CursorPos.y - tab_bar->BarRect.Max.y,
            tab_bar->CurrTabsContentsHeight);
    window->DC.CursorPos.y =
        tab_bar->BarRect.Max.y + tab_bar->CurrTabsContentsHeight;
  } else {
    window->DC.CursorPos.y =
        tab_bar->BarRect.Max.y + tab_bar->PrevTabsContentsHeight;
  }
  if (tab_bar->BeginCount > 1)
    window->DC.CursorPos = tab_bar->BackupCursorPos;

  tab_bar->LastTabItemIdx = -1;
  if ((tab_bar->Flags & TabBarFlags_DockNode) == 0)
    PopID();

  g.CurrentTabBarStack.pop_back();
  g.CurrentTabBar = g.CurrentTabBarStack.empty()
                        ? NULL
                        : GetTabBarFromTabBarRef(g.CurrentTabBarStack.back());
}

// Scrolling happens only in the central section (leading/trailing sections are
// not scrolling)
static float TabBarCalcScrollableWidth(TabBar *tab_bar,
                                       TabBarSection *sections) {
  return tab_bar->BarRect.GetWidth() - sections[0].Width - sections[2].Width -
         sections[1].Spacing;
}

// This is called only once a frame before by the first call to ItemTab()
// The reason we're not calling it in BeginTabBar() is to leave a chance to the
// user to call the SetTabItemClosed() functions.
static void Gui::TabBarLayout(TabBar *tab_bar) {
  Context &g = *GGui;
  tab_bar->WantLayout = false;

  // Garbage collect by compacting list
  // Detect if we need to sort out tab list (e.g. in rare case where a tab
  // changed section)
  int tab_dst_n = 0;
  bool need_sort_by_section = false;
  TabBarSection sections[3]; // Layout sections: Leading, Central, Trailing
  for (int tab_src_n = 0; tab_src_n < tab_bar->Tabs.Size; tab_src_n++) {
    TabItem *tab = &tab_bar->Tabs[tab_src_n];
    if (tab->LastFrameVisible < tab_bar->PrevFrameVisible || tab->WantClose) {
      // Remove tab
      if (tab_bar->VisibleTabId == tab->ID) {
        tab_bar->VisibleTabId = 0;
      }
      if (tab_bar->SelectedTabId == tab->ID) {
        tab_bar->SelectedTabId = 0;
      }
      if (tab_bar->NextSelectedTabId == tab->ID) {
        tab_bar->NextSelectedTabId = 0;
      }
      continue;
    }
    if (tab_dst_n != tab_src_n)
      tab_bar->Tabs[tab_dst_n] = tab_bar->Tabs[tab_src_n];

    tab = &tab_bar->Tabs[tab_dst_n];
    tab->IndexDuringLayout = (S16)tab_dst_n;

    // We will need sorting if tabs have changed section (e.g. moved from one of
    // Leading/Central/Trailing to another)
    int curr_tab_section_n = TabItemGetSectionIdx(tab);
    if (tab_dst_n > 0) {
      TabItem *prev_tab = &tab_bar->Tabs[tab_dst_n - 1];
      int prev_tab_section_n = TabItemGetSectionIdx(prev_tab);
      if (curr_tab_section_n == 0 && prev_tab_section_n != 0)
        need_sort_by_section = true;
      if (prev_tab_section_n == 2 && curr_tab_section_n != 2)
        need_sort_by_section = true;
    }

    sections[curr_tab_section_n].TabCount++;
    tab_dst_n++;
  }
  if (tab_bar->Tabs.Size != tab_dst_n)
    tab_bar->Tabs.resize(tab_dst_n);

  if (need_sort_by_section)
    Qsort(tab_bar->Tabs.Data, tab_bar->Tabs.Size, sizeof(TabItem),
          TabItemComparerBySection);

  // Calculate spacing between sections
  sections[0].Spacing = sections[0].TabCount > 0 && (sections[1].TabCount +
                                                     sections[2].TabCount) > 0
                            ? g.Style.ItemInnerSpacing.x
                            : 0.0f;
  sections[1].Spacing = sections[1].TabCount > 0 && sections[2].TabCount > 0
                            ? g.Style.ItemInnerSpacing.x
                            : 0.0f;

  // Setup next selected tab
  ID scroll_to_tab_id = 0;
  if (tab_bar->NextSelectedTabId) {
    tab_bar->SelectedTabId = tab_bar->NextSelectedTabId;
    tab_bar->NextSelectedTabId = 0;
    scroll_to_tab_id = tab_bar->SelectedTabId;
  }

  // Process order change request (we could probably process it when requested
  // but it's just saner to do it in a single spot).
  if (tab_bar->ReorderRequestTabId != 0) {
    if (TabBarProcessReorder(tab_bar))
      if (tab_bar->ReorderRequestTabId == tab_bar->SelectedTabId)
        scroll_to_tab_id = tab_bar->ReorderRequestTabId;
    tab_bar->ReorderRequestTabId = 0;
  }

  // Tab List Popup (will alter tab_bar->BarRect and therefore the available
  // width!)
  const bool tab_list_popup_button =
      (tab_bar->Flags & TabBarFlags_TabListPopupButton) != 0;
  if (tab_list_popup_button)
    if (TabItem *tab_to_select =
            TabBarTabListPopupButton(tab_bar)) // NB: Will alter BarRect.Min.x!
      scroll_to_tab_id = tab_bar->SelectedTabId = tab_to_select->ID;

  // Leading/Trailing tabs will be shrink only if central one aren't visible
  // anymore, so layout the shrink data as: leading, trailing, central (whereas
  // our tabs are stored as: leading, central, trailing)
  int shrink_buffer_indexes[3] = {
      0, sections[0].TabCount + sections[2].TabCount, sections[0].TabCount};
  g.ShrinkWidthBuffer.resize(tab_bar->Tabs.Size);

  // Compute ideal tabs widths + store them into shrink buffer
  TabItem *most_recently_selected_tab = NULL;
  int curr_section_n = -1;
  bool found_selected_tab_id = false;
  for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++) {
    TabItem *tab = &tab_bar->Tabs[tab_n];
    ASSERT(tab->LastFrameVisible >= tab_bar->PrevFrameVisible);

    if ((most_recently_selected_tab == NULL ||
         most_recently_selected_tab->LastFrameSelected <
             tab->LastFrameSelected) &&
        !(tab->Flags & TabItemFlags_Button))
      most_recently_selected_tab = tab;
    if (tab->ID == tab_bar->SelectedTabId)
      found_selected_tab_id = true;
    if (scroll_to_tab_id == 0 && g.NavJustMovedToId == tab->ID)
      scroll_to_tab_id = tab->ID;

    // Refresh tab width immediately, otherwise changes of style e.g.
    // style.FramePadding.x would noticeably lag in the tab bar. Additionally,
    // when using TabBarAddTab() to manipulate tab bar order we occasionally
    // insert new tabs that don't have a width yet, and we cannot wait for the
    // next BeginTabItem() call. We cannot compute this width within
    // TabBarAddTab() because font size depends on the active window.
    const char *tab_name = TabBarGetTabName(tab_bar, tab);
    const bool has_close_button_or_unsaved_marker =
        (tab->Flags & TabItemFlags_NoCloseButton) == 0 ||
        (tab->Flags & TabItemFlags_UnsavedDocument);
    tab->ContentWidth =
        (tab->RequestedWidth >= 0.0f)
            ? tab->RequestedWidth
            : TabItemCalcSize(tab_name, has_close_button_or_unsaved_marker).x;

    int section_n = TabItemGetSectionIdx(tab);
    TabBarSection *section = &sections[section_n];
    section->Width +=
        tab->ContentWidth +
        (section_n == curr_section_n ? g.Style.ItemInnerSpacing.x : 0.0f);
    curr_section_n = section_n;

    // Store data so we can build an array sorted by width if we need to shrink
    // tabs down
    MSVC_WARNING_SUPPRESS(6385);
    ShrinkWidthItem *shrink_width_item =
        &g.ShrinkWidthBuffer[shrink_buffer_indexes[section_n]++];
    shrink_width_item->Index = tab_n;
    shrink_width_item->Width = shrink_width_item->InitialWidth =
        tab->ContentWidth;
    tab->Width = Max(tab->ContentWidth, 1.0f);
  }

  // Compute total ideal width (used for e.g. auto-resizing a window)
  tab_bar->WidthAllTabsIdeal = 0.0f;
  for (int section_n = 0; section_n < 3; section_n++)
    tab_bar->WidthAllTabsIdeal +=
        sections[section_n].Width + sections[section_n].Spacing;

  // Horizontal scrolling buttons
  // (note that TabBarScrollButtons() will alter BarRect.Max.x)
  if ((tab_bar->WidthAllTabsIdeal > tab_bar->BarRect.GetWidth() &&
       tab_bar->Tabs.Size > 1) &&
      !(tab_bar->Flags & TabBarFlags_NoTabListScrollingButtons) &&
      (tab_bar->Flags & TabBarFlags_FittingPolicyScroll))
    if (TabItem *scroll_and_select_tab = TabBarScrollingButtons(tab_bar)) {
      scroll_to_tab_id = scroll_and_select_tab->ID;
      if ((scroll_and_select_tab->Flags & TabItemFlags_Button) == 0)
        tab_bar->SelectedTabId = scroll_to_tab_id;
    }

  // Shrink widths if full tabs don't fit in their allocated space
  float section_0_w = sections[0].Width + sections[0].Spacing;
  float section_1_w = sections[1].Width + sections[1].Spacing;
  float section_2_w = sections[2].Width + sections[2].Spacing;
  bool central_section_is_visible =
      (section_0_w + section_2_w) < tab_bar->BarRect.GetWidth();
  float width_excess;
  if (central_section_is_visible)
    width_excess = Max(
        section_1_w - (tab_bar->BarRect.GetWidth() - section_0_w - section_2_w),
        0.0f); // Excess used to shrink central section
  else
    width_excess =
        (section_0_w + section_2_w) -
        tab_bar->BarRect
            .GetWidth(); // Excess used to shrink leading/trailing section

  // With TabBarFlags_FittingPolicyScroll policy, we will only shrink
  // leading/trailing if the central section is not visible anymore
  if (width_excess >= 1.0f &&
      ((tab_bar->Flags & TabBarFlags_FittingPolicyResizeDown) ||
       !central_section_is_visible)) {
    int shrink_data_count = (central_section_is_visible
                                 ? sections[1].TabCount
                                 : sections[0].TabCount + sections[2].TabCount);
    int shrink_data_offset = (central_section_is_visible
                                  ? sections[0].TabCount + sections[2].TabCount
                                  : 0);
    ShrinkWidths(g.ShrinkWidthBuffer.Data + shrink_data_offset,
                 shrink_data_count, width_excess);

    // Apply shrunk values into tabs and sections
    for (int tab_n = shrink_data_offset;
         tab_n < shrink_data_offset + shrink_data_count; tab_n++) {
      TabItem *tab = &tab_bar->Tabs[g.ShrinkWidthBuffer[tab_n].Index];
      float shrinked_width = TRUNC(g.ShrinkWidthBuffer[tab_n].Width);
      if (shrinked_width < 0.0f)
        continue;

      shrinked_width = Max(1.0f, shrinked_width);
      int section_n = TabItemGetSectionIdx(tab);
      sections[section_n].Width -= (tab->Width - shrinked_width);
      tab->Width = shrinked_width;
    }
  }

  // Layout all active tabs
  int section_tab_index = 0;
  float tab_offset = 0.0f;
  tab_bar->WidthAllTabs = 0.0f;
  for (int section_n = 0; section_n < 3; section_n++) {
    TabBarSection *section = &sections[section_n];
    if (section_n == 2)
      tab_offset = Min(Max(0.0f, tab_bar->BarRect.GetWidth() - section->Width),
                       tab_offset);

    for (int tab_n = 0; tab_n < section->TabCount; tab_n++) {
      TabItem *tab = &tab_bar->Tabs[section_tab_index + tab_n];
      tab->Offset = tab_offset;
      tab->NameOffset = -1;
      tab_offset +=
          tab->Width +
          (tab_n < section->TabCount - 1 ? g.Style.ItemInnerSpacing.x : 0.0f);
    }
    tab_bar->WidthAllTabs += Max(section->Width + section->Spacing, 0.0f);
    tab_offset += section->Spacing;
    section_tab_index += section->TabCount;
  }

  // Clear name buffers
  tab_bar->TabsNames.Buf.resize(0);

  // If we have lost the selected tab, select the next most recently active one
  if (found_selected_tab_id == false)
    tab_bar->SelectedTabId = 0;
  if (tab_bar->SelectedTabId == 0 && tab_bar->NextSelectedTabId == 0 &&
      most_recently_selected_tab != NULL)
    scroll_to_tab_id = tab_bar->SelectedTabId = most_recently_selected_tab->ID;

  // Lock in visible tab
  tab_bar->VisibleTabId = tab_bar->SelectedTabId;
  tab_bar->VisibleTabWasSubmitted = false;

  // CTRL+TAB can override visible tab temporarily
  if (g.NavWindowingTarget != NULL && g.NavWindowingTarget->DockNode &&
      g.NavWindowingTarget->DockNode->TabBar == tab_bar)
    tab_bar->VisibleTabId = scroll_to_tab_id = g.NavWindowingTarget->TabId;

  // Apply request requests
  if (scroll_to_tab_id != 0)
    TabBarScrollToTab(tab_bar, scroll_to_tab_id, sections);
  else if ((tab_bar->Flags & TabBarFlags_FittingPolicyScroll) &&
           IsMouseHoveringRect(tab_bar->BarRect.Min, tab_bar->BarRect.Max,
                               true) &&
           IsWindowContentHoverable(g.CurrentWindow)) {
    const float wheel =
        g.IO.MouseWheelRequestAxisSwap ? g.IO.MouseWheel : g.IO.MouseWheelH;
    const Key wheel_key =
        g.IO.MouseWheelRequestAxisSwap ? Key_MouseWheelY : Key_MouseWheelX;
    if (TestKeyOwner(wheel_key, tab_bar->ID) && wheel != 0.0f) {
      const float scroll_step =
          wheel * TabBarCalcScrollableWidth(tab_bar, sections) / 3.0f;
      tab_bar->ScrollingTargetDistToVisibility = 0.0f;
      tab_bar->ScrollingTarget =
          TabBarScrollClamp(tab_bar, tab_bar->ScrollingTarget - scroll_step);
    }
    SetKeyOwner(wheel_key, tab_bar->ID);
  }

  // Update scrolling
  tab_bar->ScrollingAnim = TabBarScrollClamp(tab_bar, tab_bar->ScrollingAnim);
  tab_bar->ScrollingTarget =
      TabBarScrollClamp(tab_bar, tab_bar->ScrollingTarget);
  if (tab_bar->ScrollingAnim != tab_bar->ScrollingTarget) {
    // Scrolling speed adjust itself so we can always reach our target in 1/3
    // seconds. Teleport if we are aiming far off the visible line
    tab_bar->ScrollingSpeed = Max(tab_bar->ScrollingSpeed, 70.0f * g.FontSize);
    tab_bar->ScrollingSpeed =
        Max(tab_bar->ScrollingSpeed,
            Fabs(tab_bar->ScrollingTarget - tab_bar->ScrollingAnim) / 0.3f);
    const bool teleport =
        (tab_bar->PrevFrameVisible + 1 < g.FrameCount) ||
        (tab_bar->ScrollingTargetDistToVisibility > 10.0f * g.FontSize);
    tab_bar->ScrollingAnim =
        teleport ? tab_bar->ScrollingTarget
                 : LinearSweep(tab_bar->ScrollingAnim, tab_bar->ScrollingTarget,
                               g.IO.DeltaTime * tab_bar->ScrollingSpeed);
  } else {
    tab_bar->ScrollingSpeed = 0.0f;
  }
  tab_bar->ScrollingRectMinX =
      tab_bar->BarRect.Min.x + sections[0].Width + sections[0].Spacing;
  tab_bar->ScrollingRectMaxX =
      tab_bar->BarRect.Max.x - sections[2].Width - sections[1].Spacing;

  // Actual layout in host window (we don't do it in BeginTabBar() so as not to
  // waste an extra frame)
  Window *window = g.CurrentWindow;
  window->DC.CursorPos = tab_bar->BarRect.Min;
  ItemSize(Vec2(tab_bar->WidthAllTabs, tab_bar->BarRect.GetHeight()),
           tab_bar->FramePadding.y);
  window->DC.IdealMaxPos.x =
      Max(window->DC.IdealMaxPos.x,
          tab_bar->BarRect.Min.x + tab_bar->WidthAllTabsIdeal);
}

// Dockable windows uses Name/ID in the global namespace. Non-dockable items use
// the ID stack.
static U32 Gui::TabBarCalcTabID(TabBar *tab_bar, const char *label,
                                Window *docked_window) {
  if (docked_window != NULL) {
    UNUSED(tab_bar);
    ASSERT(tab_bar->Flags & TabBarFlags_DockNode);
    ID id = docked_window->TabId;
    KeepAliveID(id);
    return id;
  } else {
    Window *window = GGui->CurrentWindow;
    return window->GetID(label);
  }
}

static float Gui::TabBarCalcMaxTabWidth() {
  Context &g = *GGui;
  return g.FontSize * 20.0f;
}

TabItem *Gui::TabBarFindTabByID(TabBar *tab_bar, ID tab_id) {
  if (tab_id != 0)
    for (int n = 0; n < tab_bar->Tabs.Size; n++)
      if (tab_bar->Tabs[n].ID == tab_id)
        return &tab_bar->Tabs[n];
  return NULL;
}

// Order = visible order, not submission order! (which is tab->BeginOrder)
TabItem *Gui::TabBarFindTabByOrder(TabBar *tab_bar, int order) {
  if (order < 0 || order >= tab_bar->Tabs.Size)
    return NULL;
  return &tab_bar->Tabs[order];
}

// FIXME: See references to #2304 in TODO.txt
TabItem *
Gui::TabBarFindMostRecentlySelectedTabForActiveWindow(TabBar *tab_bar) {
  TabItem *most_recently_selected_tab = NULL;
  for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++) {
    TabItem *tab = &tab_bar->Tabs[tab_n];
    if (most_recently_selected_tab == NULL ||
        most_recently_selected_tab->LastFrameSelected < tab->LastFrameSelected)
      if (tab->Window && tab->Window->WasActive)
        most_recently_selected_tab = tab;
  }
  return most_recently_selected_tab;
}

TabItem *Gui::TabBarGetCurrentTab(TabBar *tab_bar) {
  if (tab_bar->LastTabItemIdx <= 0 ||
      tab_bar->LastTabItemIdx >= tab_bar->Tabs.Size)
    return NULL;
  return &tab_bar->Tabs[tab_bar->LastTabItemIdx];
}

const char *Gui::TabBarGetTabName(TabBar *tab_bar, TabItem *tab) {
  if (tab->Window)
    return tab->Window->Name;
  if (tab->NameOffset == -1)
    return "N/A";
  ASSERT(tab->NameOffset < tab_bar->TabsNames.Buf.Size);
  return tab_bar->TabsNames.Buf.Data + tab->NameOffset;
}

// The purpose of this call is to register tab in advance so we can control
// their order at the time they appear. Otherwise calling this is unnecessary as
// tabs are appending as needed by the BeginTabItem() function.
void Gui::TabBarAddTab(TabBar *tab_bar, TabItemFlags tab_flags,
                       Window *window) {
  Context &g = *GGui;
  ASSERT(TabBarFindTabByID(tab_bar, window->TabId) == NULL);
  ASSERT(g.CurrentTabBar !=
         tab_bar); // Can't work while the tab bar is active as our tab doesn't
                   // have an X offset yet, in theory we could/should test
                   // something like (tab_bar->CurrFrameVisible < g.FrameCount)
                   // but we'd need to solve why triggers the commented
                   // early-out assert in BeginTabBarEx() (probably dock node
                   // going from implicit to explicit in same frame)

  if (!window->HasCloseButton)
    tab_flags |= TabItemFlags_NoCloseButton; // Set _NoCloseButton immediately
                                             // because it will be used for
                                             // first-frame width calculation.

  TabItem new_tab;
  new_tab.ID = window->TabId;
  new_tab.Flags = tab_flags;
  new_tab.LastFrameVisible =
      tab_bar
          ->CurrFrameVisible; // Required so BeginTabBar() doesn't ditch the tab
  if (new_tab.LastFrameVisible == -1)
    new_tab.LastFrameVisible = g.FrameCount - 1;
  new_tab.Window = window; // Required so tab bar layout can compute the tab
                           // width before tab submission
  tab_bar->Tabs.push_back(new_tab);
}

// The *TabId fields are already set by the docking system _before_ the actual
// TabItem was created, so we clear them regardless.
void Gui::TabBarRemoveTab(TabBar *tab_bar, ID tab_id) {
  if (TabItem *tab = TabBarFindTabByID(tab_bar, tab_id))
    tab_bar->Tabs.erase(tab);
  if (tab_bar->VisibleTabId == tab_id) {
    tab_bar->VisibleTabId = 0;
  }
  if (tab_bar->SelectedTabId == tab_id) {
    tab_bar->SelectedTabId = 0;
  }
  if (tab_bar->NextSelectedTabId == tab_id) {
    tab_bar->NextSelectedTabId = 0;
  }
}

// Called on manual closure attempt
void Gui::TabBarCloseTab(TabBar *tab_bar, TabItem *tab) {
  if (tab->Flags & TabItemFlags_Button)
    return; // A button appended with TabItemButton().

  if ((tab->Flags &
       (TabItemFlags_UnsavedDocument | TabItemFlags_NoAssumedClosure)) == 0) {
    // This will remove a frame of lag for selecting another tab on closure.
    // However we don't run it in the case where the 'Unsaved' flag is set, so
    // user gets a chance to fully undo the closure
    tab->WantClose = true;
    if (tab_bar->VisibleTabId == tab->ID) {
      tab->LastFrameVisible = -1;
      tab_bar->SelectedTabId = tab_bar->NextSelectedTabId = 0;
    }
  } else {
    // Actually select before expecting closure attempt (on an UnsavedDocument
    // tab user is expect to e.g. show a popup)
    if (tab_bar->VisibleTabId != tab->ID)
      TabBarQueueFocus(tab_bar, tab);
  }
}

static float Gui::TabBarScrollClamp(TabBar *tab_bar, float scrolling) {
  scrolling =
      Min(scrolling, tab_bar->WidthAllTabs - tab_bar->BarRect.GetWidth());
  return Max(scrolling, 0.0f);
}

// Note: we may scroll to tab that are not selected! e.g. using keyboard arrow
// keys
static void Gui::TabBarScrollToTab(TabBar *tab_bar, ID tab_id,
                                   TabBarSection *sections) {
  TabItem *tab = TabBarFindTabByID(tab_bar, tab_id);
  if (tab == NULL)
    return;
  if (tab->Flags & TabItemFlags_SectionMask_)
    return;

  Context &g = *GGui;
  float margin =
      g.FontSize * 1.0f; // When to scroll to make Tab N+1 visible always make a
                         // bit of N visible to suggest more scrolling area
                         // (since we don't have a scrollbar)
  int order = TabBarGetTabOrder(tab_bar, tab);

  // Scrolling happens only in the central section (leading/trailing sections
  // are not scrolling)
  float scrollable_width = TabBarCalcScrollableWidth(tab_bar, sections);

  // We make all tabs positions all relative Sections[0].Width to make code
  // simpler
  float tab_x1 = tab->Offset - sections[0].Width +
                 (order > sections[0].TabCount - 1 ? -margin : 0.0f);
  float tab_x2 =
      tab->Offset - sections[0].Width + tab->Width +
      (order + 1 < tab_bar->Tabs.Size - sections[2].TabCount ? margin : 1.0f);
  tab_bar->ScrollingTargetDistToVisibility = 0.0f;
  if (tab_bar->ScrollingTarget > tab_x1 ||
      (tab_x2 - tab_x1 >= scrollable_width)) {
    // Scroll to the left
    tab_bar->ScrollingTargetDistToVisibility =
        Max(tab_bar->ScrollingAnim - tab_x2, 0.0f);
    tab_bar->ScrollingTarget = tab_x1;
  } else if (tab_bar->ScrollingTarget < tab_x2 - scrollable_width) {
    // Scroll to the right
    tab_bar->ScrollingTargetDistToVisibility =
        Max((tab_x1 - scrollable_width) - tab_bar->ScrollingAnim, 0.0f);
    tab_bar->ScrollingTarget = tab_x2 - scrollable_width;
  }
}

void Gui::TabBarQueueFocus(TabBar *tab_bar, TabItem *tab) {
  tab_bar->NextSelectedTabId = tab->ID;
}

void Gui::TabBarQueueReorder(TabBar *tab_bar, TabItem *tab, int offset) {
  ASSERT(offset != 0);
  ASSERT(tab_bar->ReorderRequestTabId == 0);
  tab_bar->ReorderRequestTabId = tab->ID;
  tab_bar->ReorderRequestOffset = (S16)offset;
}

void Gui::TabBarQueueReorderFromMousePos(TabBar *tab_bar, TabItem *src_tab,
                                         Vec2 mouse_pos) {
  Context &g = *GGui;
  ASSERT(tab_bar->ReorderRequestTabId == 0);
  if ((tab_bar->Flags & TabBarFlags_Reorderable) == 0)
    return;

  const bool is_central_section =
      (src_tab->Flags & TabItemFlags_SectionMask_) == 0;
  const float bar_offset = tab_bar->BarRect.Min.x -
                           (is_central_section ? tab_bar->ScrollingTarget : 0);

  // Count number of contiguous tabs we are crossing over
  const int dir = (bar_offset + src_tab->Offset) > mouse_pos.x ? -1 : +1;
  const int src_idx = tab_bar->Tabs.index_from_ptr(src_tab);
  int dst_idx = src_idx;
  for (int i = src_idx; i >= 0 && i < tab_bar->Tabs.Size; i += dir) {
    // Reordered tabs must share the same section
    const TabItem *dst_tab = &tab_bar->Tabs[i];
    if (dst_tab->Flags & TabItemFlags_NoReorder)
      break;
    if ((dst_tab->Flags & TabItemFlags_SectionMask_) !=
        (src_tab->Flags & TabItemFlags_SectionMask_))
      break;
    dst_idx = i;

    // Include spacing after tab, so when mouse cursor is between tabs we would
    // not continue checking further tabs that are not hovered.
    const float x1 = bar_offset + dst_tab->Offset - g.Style.ItemInnerSpacing.x;
    const float x2 = bar_offset + dst_tab->Offset + dst_tab->Width +
                     g.Style.ItemInnerSpacing.x;
    // GetForegroundDrawList()->AddRect(Vec2(x1, tab_bar->BarRect.Min.y),
    // Vec2(x2, tab_bar->BarRect.Max.y), COL32(255, 0, 0, 255));
    if ((dir < 0 && mouse_pos.x > x1) || (dir > 0 && mouse_pos.x < x2))
      break;
  }

  if (dst_idx != src_idx)
    TabBarQueueReorder(tab_bar, src_tab, dst_idx - src_idx);
}

bool Gui::TabBarProcessReorder(TabBar *tab_bar) {
  TabItem *tab1 = TabBarFindTabByID(tab_bar, tab_bar->ReorderRequestTabId);
  if (tab1 == NULL || (tab1->Flags & TabItemFlags_NoReorder))
    return false;

  // ASSERT(tab_bar->Flags & TabBarFlags_Reorderable); // <- this may
  // happen when using debug tools
  int tab2_order =
      TabBarGetTabOrder(tab_bar, tab1) + tab_bar->ReorderRequestOffset;
  if (tab2_order < 0 || tab2_order >= tab_bar->Tabs.Size)
    return false;

  // Reordered tabs must share the same section
  // (Note: TabBarQueueReorderFromMousePos() also has a similar test but since
  // we allow direct calls to TabBarQueueReorder() we do it here too)
  TabItem *tab2 = &tab_bar->Tabs[tab2_order];
  if (tab2->Flags & TabItemFlags_NoReorder)
    return false;
  if ((tab1->Flags & TabItemFlags_SectionMask_) !=
      (tab2->Flags & TabItemFlags_SectionMask_))
    return false;

  TabItem item_tmp = *tab1;
  TabItem *src_tab = (tab_bar->ReorderRequestOffset > 0) ? tab1 + 1 : tab2;
  TabItem *dst_tab = (tab_bar->ReorderRequestOffset > 0) ? tab1 : tab2 + 1;
  const int move_count = (tab_bar->ReorderRequestOffset > 0)
                             ? tab_bar->ReorderRequestOffset
                             : -tab_bar->ReorderRequestOffset;
  memmove(dst_tab, src_tab, move_count * sizeof(TabItem));
  *tab2 = item_tmp;

  if (tab_bar->Flags & TabBarFlags_SaveSettings)
    MarkIniSettingsDirty();
  return true;
}

static TabItem *Gui::TabBarScrollingButtons(TabBar *tab_bar) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  const Vec2 arrow_button_size(g.FontSize - 2.0f,
                               g.FontSize + g.Style.FramePadding.y * 2.0f);
  const float scrolling_buttons_width = arrow_button_size.x * 2.0f;

  const Vec2 backup_cursor_pos = window->DC.CursorPos;
  // window->DrawList->AddRect(Vec2(tab_bar->BarRect.Max.x -
  // scrolling_buttons_width, tab_bar->BarRect.Min.y),
  // Vec2(tab_bar->BarRect.Max.x, tab_bar->BarRect.Max.y),
  // COL32(255,0,0,255));

  int select_dir = 0;
  Vec4 arrow_col = g.Style.Colors[Col_Text];
  arrow_col.w *= 0.5f;

  PushStyleColor(Col_Text, arrow_col);
  PushStyleColor(Col_Button, Vec4(0, 0, 0, 0));
  const float backup_repeat_delay = g.IO.KeyRepeatDelay;
  const float backup_repeat_rate = g.IO.KeyRepeatRate;
  g.IO.KeyRepeatDelay = 0.250f;
  g.IO.KeyRepeatRate = 0.200f;
  float x = Max(tab_bar->BarRect.Min.x,
                tab_bar->BarRect.Max.x - scrolling_buttons_width);
  window->DC.CursorPos = Vec2(x, tab_bar->BarRect.Min.y);
  if (ArrowButtonEx("##<", Dir_Left, arrow_button_size,
                    ButtonFlags_PressedOnClick | ButtonFlags_Repeat))
    select_dir = -1;
  window->DC.CursorPos = Vec2(x + arrow_button_size.x, tab_bar->BarRect.Min.y);
  if (ArrowButtonEx("##>", Dir_Right, arrow_button_size,
                    ButtonFlags_PressedOnClick | ButtonFlags_Repeat))
    select_dir = +1;
  PopStyleColor(2);
  g.IO.KeyRepeatRate = backup_repeat_rate;
  g.IO.KeyRepeatDelay = backup_repeat_delay;

  TabItem *tab_to_scroll_to = NULL;
  if (select_dir != 0)
    if (TabItem *tab_item =
            TabBarFindTabByID(tab_bar, tab_bar->SelectedTabId)) {
      int selected_order = TabBarGetTabOrder(tab_bar, tab_item);
      int target_order = selected_order + select_dir;

      // Skip tab item buttons until another tab item is found or end is reached
      while (tab_to_scroll_to == NULL) {
        // If we are at the end of the list, still scroll to make our tab
        // visible
        tab_to_scroll_to =
            &tab_bar
                 ->Tabs[(target_order >= 0 && target_order < tab_bar->Tabs.Size)
                            ? target_order
                            : selected_order];

        // Cross through buttons
        // (even if first/last item is a button, return it so we can update the
        // scroll)
        if (tab_to_scroll_to->Flags & TabItemFlags_Button) {
          target_order += select_dir;
          selected_order += select_dir;
          tab_to_scroll_to =
              (target_order < 0 || target_order >= tab_bar->Tabs.Size)
                  ? tab_to_scroll_to
                  : NULL;
        }
      }
    }
  window->DC.CursorPos = backup_cursor_pos;
  tab_bar->BarRect.Max.x -= scrolling_buttons_width + 1.0f;

  return tab_to_scroll_to;
}

static TabItem *Gui::TabBarTabListPopupButton(TabBar *tab_bar) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;

  // We use g.Style.FramePadding.y to match the square ArrowButton size
  const float tab_list_popup_button_width = g.FontSize + g.Style.FramePadding.y;
  const Vec2 backup_cursor_pos = window->DC.CursorPos;
  window->DC.CursorPos = Vec2(tab_bar->BarRect.Min.x - g.Style.FramePadding.y,
                              tab_bar->BarRect.Min.y);
  tab_bar->BarRect.Min.x += tab_list_popup_button_width;

  Vec4 arrow_col = g.Style.Colors[Col_Text];
  arrow_col.w *= 0.5f;
  PushStyleColor(Col_Text, arrow_col);
  PushStyleColor(Col_Button, Vec4(0, 0, 0, 0));
  bool open =
      BeginCombo("##v", NULL, ComboFlags_NoPreview | ComboFlags_HeightLargest);
  PopStyleColor(2);

  TabItem *tab_to_select = NULL;
  if (open) {
    for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++) {
      TabItem *tab = &tab_bar->Tabs[tab_n];
      if (tab->Flags & TabItemFlags_Button)
        continue;

      const char *tab_name = TabBarGetTabName(tab_bar, tab);
      if (Selectable(tab_name, tab_bar->SelectedTabId == tab->ID))
        tab_to_select = tab;
    }
    EndCombo();
  }

  window->DC.CursorPos = backup_cursor_pos;
  return tab_to_select;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.
//-------------------------------------------------------------------------
// - BeginTabItem()
// - EndTabItem()
// - TabItemButton()
// - TabItemEx() [Internal]
// - SetTabItemClosed()
// - TabItemCalcSize() [Internal]
// - TabItemBackground() [Internal]
// - TabItemLabelAndCloseButton() [Internal]
//-------------------------------------------------------------------------

bool Gui::BeginTabItem(const char *label, bool *p_open, TabItemFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ASSERT_USER_ERROR(
        tab_bar, "Needs to be called between BeginTabBar() and EndTabBar()!");
    return false;
  }
  ASSERT((flags & TabItemFlags_Button) ==
         0); // BeginTabItem() Can't be used with button flags, use
             // TabItemButton() instead!

  bool ret = TabItemEx(tab_bar, label, p_open, flags, NULL);
  if (ret && !(flags & TabItemFlags_NoPushId)) {
    TabItem *tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
    PushOverrideID(tab->ID); // We already hashed 'label' so push into the ID
                             // stack directly instead of doing another hash
                             // through PushID(label)
  }
  return ret;
}

void Gui::EndTabItem() {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ASSERT_USER_ERROR(
        tab_bar != NULL,
        "Needs to be called between BeginTabBar() and EndTabBar()!");
    return;
  }
  ASSERT(tab_bar->LastTabItemIdx >= 0);
  TabItem *tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
  if (!(tab->Flags & TabItemFlags_NoPushId))
    PopID();
}

bool Gui::TabItemButton(const char *label, TabItemFlags flags) {
  Context &g = *GGui;
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ASSERT_USER_ERROR(
        tab_bar != NULL,
        "Needs to be called between BeginTabBar() and EndTabBar()!");
    return false;
  }
  return TabItemEx(tab_bar, label, NULL,
                   flags | TabItemFlags_Button | TabItemFlags_NoReorder, NULL);
}

bool Gui::TabItemEx(TabBar *tab_bar, const char *label, bool *p_open,
                    TabItemFlags flags, Window *docked_window) {
  // Layout whole tab bar if not already done
  Context &g = *GGui;
  if (tab_bar->WantLayout) {
    NextItemData backup_next_item_data = g.NextItemData;
    TabBarLayout(tab_bar);
    g.NextItemData = backup_next_item_data;
  }
  Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  const Style &style = g.Style;
  const ID id = TabBarCalcTabID(tab_bar, label, docked_window);

  // If the user called us with *p_open == false, we early out and don't render.
  // We make a call to ItemAdd() so that attempts to use a contextual popup menu
  // with an implicit ID won't use an older ID.
  TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
  if (p_open && !*p_open) {
    ItemAdd(Rect(), id, NULL, ItemFlags_NoNav);
    return false;
  }

  ASSERT(!p_open || !(flags & TabItemFlags_Button));
  ASSERT((flags & (TabItemFlags_Leading | TabItemFlags_Trailing)) !=
         (TabItemFlags_Leading |
          TabItemFlags_Trailing)); // Can't use both Leading and Trailing

  // Store into TabItemFlags_NoCloseButton, also honor
  // TabItemFlags_NoCloseButton passed by user (although not documented)
  if (flags & TabItemFlags_NoCloseButton)
    p_open = NULL;
  else if (p_open == NULL)
    flags |= TabItemFlags_NoCloseButton;

  // Acquire tab data
  TabItem *tab = TabBarFindTabByID(tab_bar, id);
  bool tab_is_new = false;
  if (tab == NULL) {
    tab_bar->Tabs.push_back(TabItem());
    tab = &tab_bar->Tabs.back();
    tab->ID = id;
    tab_bar->TabsAddedNew = tab_is_new = true;
  }
  tab_bar->LastTabItemIdx = (S16)tab_bar->Tabs.index_from_ptr(tab);

  // Calculate tab contents size
  Vec2 size = TabItemCalcSize(
      label, (p_open != NULL) || (flags & TabItemFlags_UnsavedDocument));
  tab->RequestedWidth = -1.0f;
  if (g.NextItemData.Flags & NextItemDataFlags_HasWidth)
    size.x = tab->RequestedWidth = g.NextItemData.Width;
  if (tab_is_new)
    tab->Width = Max(1.0f, size.x);
  tab->ContentWidth = size.x;
  tab->BeginOrder = tab_bar->TabsActiveCount++;

  const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
  const bool tab_bar_focused = (tab_bar->Flags & TabBarFlags_IsFocused) != 0;
  const bool tab_appearing = (tab->LastFrameVisible + 1 < g.FrameCount);
  const bool tab_just_unsaved = (flags & TabItemFlags_UnsavedDocument) &&
                                !(tab->Flags & TabItemFlags_UnsavedDocument);
  const bool is_tab_button = (flags & TabItemFlags_Button) != 0;
  tab->LastFrameVisible = g.FrameCount;
  tab->Flags = flags;
  tab->Window = docked_window;

  // Append name _WITH_ the zero-terminator
  // (regular tabs are permitted in a DockNode tab bar, but window tabs not
  // permitted in a non-DockNode tab bar)
  if (docked_window != NULL) {
    ASSERT(tab_bar->Flags & TabBarFlags_DockNode);
    tab->NameOffset = -1;
  } else {
    tab->NameOffset = (S32)tab_bar->TabsNames.size();
    tab_bar->TabsNames.append(label, label + strlen(label) + 1);
  }

  // Update selected tab
  if (!is_tab_button) {
    if (tab_appearing && (tab_bar->Flags & TabBarFlags_AutoSelectNewTabs) &&
        tab_bar->NextSelectedTabId == 0)
      if (!tab_bar_appearing || tab_bar->SelectedTabId == 0)
        TabBarQueueFocus(tab_bar, tab); // New tabs gets activated
    if ((flags & TabItemFlags_SetSelected) &&
        (tab_bar->SelectedTabId !=
         id)) // _SetSelected can only be passed on explicit tab bar
      TabBarQueueFocus(tab_bar, tab);
  }

  // Lock visibility
  // (Note: tab_contents_visible != tab_selected... because CTRL+TAB operations
  // may preview some tabs without selecting them!)
  bool tab_contents_visible = (tab_bar->VisibleTabId == id);
  if (tab_contents_visible)
    tab_bar->VisibleTabWasSubmitted = true;

  // On the very first frame of a tab bar we let first tab contents be visible
  // to minimize appearing glitches
  if (!tab_contents_visible && tab_bar->SelectedTabId == 0 &&
      tab_bar_appearing && docked_window == NULL)
    if (tab_bar->Tabs.Size == 1 &&
        !(tab_bar->Flags & TabBarFlags_AutoSelectNewTabs))
      tab_contents_visible = true;

  // Note that tab_is_new is not necessarily the same as tab_appearing! When a
  // tab bar stops being submitted and then gets submitted again, the tabs will
  // have 'tab_appearing=true' but 'tab_is_new=false'.
  if (tab_appearing && (!tab_bar_appearing || tab_is_new)) {
    ItemAdd(Rect(), id, NULL, ItemFlags_NoNav);
    if (is_tab_button)
      return false;
    return tab_contents_visible;
  }

  if (tab_bar->SelectedTabId == id)
    tab->LastFrameSelected = g.FrameCount;

  // Backup current layout position
  const Vec2 backup_main_cursor_pos = window->DC.CursorPos;

  // Layout
  const bool is_central_section = (tab->Flags & TabItemFlags_SectionMask_) == 0;
  size.x = tab->Width;
  if (is_central_section)
    window->DC.CursorPos =
        tab_bar->BarRect.Min +
        Vec2(TRUNC(tab->Offset - tab_bar->ScrollingAnim), 0.0f);
  else
    window->DC.CursorPos = tab_bar->BarRect.Min + Vec2(tab->Offset, 0.0f);
  Vec2 pos = window->DC.CursorPos;
  Rect bb(pos, pos + size);

  // We don't have CPU clipping primitives to clip the CloseButton (until it
  // becomes a texture), so need to add an extra draw call (temporary in the
  // case of vertical animation)
  const bool want_clip_rect =
      is_central_section && (bb.Min.x < tab_bar->ScrollingRectMinX ||
                             bb.Max.x > tab_bar->ScrollingRectMaxX);
  if (want_clip_rect)
    PushClipRect(Vec2(Max(bb.Min.x, tab_bar->ScrollingRectMinX), bb.Min.y - 1),
                 Vec2(tab_bar->ScrollingRectMaxX, bb.Max.y), true);

  Vec2 backup_cursor_max_pos = window->DC.CursorMaxPos;
  ItemSize(bb.GetSize(), style.FramePadding.y);
  window->DC.CursorMaxPos = backup_cursor_max_pos;

  if (!ItemAdd(bb, id)) {
    if (want_clip_rect)
      PopClipRect();
    window->DC.CursorPos = backup_main_cursor_pos;
    return tab_contents_visible;
  }

  // Click to Select a tab
  ButtonFlags button_flags = ((is_tab_button ? ButtonFlags_PressedOnClickRelease
                                             : ButtonFlags_PressedOnClick) |
                              ButtonFlags_AllowOverlap);
  if (g.DragDropActive &&
      !g.DragDropPayload.IsDataType(
          PAYLOAD_TYPE_WINDOW)) // FIXME: May be an opt-in property of the
                                // payload to disable this
    button_flags |= ButtonFlags_PressedOnDragDropHold;
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);
  if (pressed && !is_tab_button)
    TabBarQueueFocus(tab_bar, tab);

  // Transfer active id window so the active id is not owned by the dock host
  // (as StartMouseMovingWindow() will only do it on the drag). This allows
  // FocusWindow() to be more conservative in how it clears active id.
  if (held && docked_window && g.ActiveId == id && g.ActiveIdIsJustActivated)
    g.ActiveIdWindow = docked_window;

  // Drag and drop a single floating window node moves it
  DockNode *node = docked_window ? docked_window->DockNode : NULL;
  const bool single_floating_window_node =
      node && node->IsFloatingNode() && (node->Windows.Size == 1);
  if (held && single_floating_window_node && IsMouseDragging(0, 0.0f)) {
    // Move
    StartMouseMovingWindow(docked_window);
  } else if (held && !tab_appearing && IsMouseDragging(0)) {
    // Drag and drop: re-order tabs
    int drag_dir = 0;
    float drag_distance_from_edge_x = 0.0f;
    if (!g.DragDropActive && ((tab_bar->Flags & TabBarFlags_Reorderable) ||
                              (docked_window != NULL))) {
      // While moving a tab it will jump on the other side of the mouse, so we
      // also test for MouseDelta.x
      if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < bb.Min.x) {
        drag_dir = -1;
        drag_distance_from_edge_x = bb.Min.x - g.IO.MousePos.x;
        TabBarQueueReorderFromMousePos(tab_bar, tab, g.IO.MousePos);
      } else if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > bb.Max.x) {
        drag_dir = +1;
        drag_distance_from_edge_x = g.IO.MousePos.x - bb.Max.x;
        TabBarQueueReorderFromMousePos(tab_bar, tab, g.IO.MousePos);
      }
    }

    // Extract a Dockable window out of it's tab bar
    const bool can_undock = docked_window != NULL &&
                            !(docked_window->Flags & WindowFlags_NoMove) &&
                            !(node->MergedFlags & DockNodeFlags_NoUndocking);
    if (can_undock) {
      // We use a variable threshold to distinguish dragging tabs within a tab
      // bar and extracting them out of the tab bar
      bool undocking_tab =
          (g.DragDropActive && g.DragDropPayload.SourceId == id);
      if (!undocking_tab) //&& (!g.IO.ConfigDockingWithShift || g.IO.KeyShift)
      {
        float threshold_base = g.FontSize;
        float threshold_x = (threshold_base * 2.2f);
        float threshold_y = (threshold_base * 1.5f) +
                            Clamp((Fabs(g.IO.MouseDragMaxDistanceAbs[0].x) -
                                   threshold_base * 2.0f) *
                                      0.20f,
                                  0.0f, threshold_base * 4.0f);
        // GetForegroundDrawList()->AddRect(Vec2(bb.Min.x - threshold_x,
        // bb.Min.y - threshold_y), Vec2(bb.Max.x + threshold_x, bb.Max.y +
        // threshold_y), COL32_WHITE); // [DEBUG]

        float distance_from_edge_y =
            Max(bb.Min.y - g.IO.MousePos.y, g.IO.MousePos.y - bb.Max.y);
        if (distance_from_edge_y >= threshold_y)
          undocking_tab = true;
        if (drag_distance_from_edge_x > threshold_x)
          if ((drag_dir < 0 && TabBarGetTabOrder(tab_bar, tab) == 0) ||
              (drag_dir > 0 &&
               TabBarGetTabOrder(tab_bar, tab) == tab_bar->Tabs.Size - 1))
            undocking_tab = true;
      }

      if (undocking_tab) {
        // Undock
        // FIXME: refactor to share more code with e.g. StartMouseMovingWindow
        DockContextQueueUndockWindow(&g, docked_window);
        g.MovingWindow = docked_window;
        SetActiveID(g.MovingWindow->MoveId, g.MovingWindow);
        g.ActiveIdClickOffset -= g.MovingWindow->Pos - bb.Min;
        g.ActiveIdNoClearOnFocusLoss = true;
        SetActiveIdUsingAllKeyboardKeys();
      }
    }
  }

#if 0
    if (hovered && g.HoveredIdNotActiveTimer > TOOLTIP_DELAY && bb.GetWidth() < tab->ContentWidth)
    {
        // Enlarge tab display when hovering
        bb.Max.x = bb.Min.x + TRUNC(Lerp(bb.GetWidth(), tab->ContentWidth, Saturate((g.HoveredIdNotActiveTimer - 0.40f) * 6.0f)));
        display_draw_list = GetForegroundDrawList(window);
        TabItemBackground(display_draw_list, bb, flags, GetColorU32(Col_TitleBgActive));
    }
#endif

  // Render tab shape
  DrawList *display_draw_list = window->DrawList;
  const U32 tab_col = GetColorU32(
      (held || hovered) ? Col_TabHovered
      : tab_contents_visible
          ? (tab_bar_focused ? Col_TabActive : Col_TabUnfocusedActive)
          : (tab_bar_focused ? Col_Tab : Col_TabUnfocused));
  TabItemBackground(display_draw_list, bb, flags, tab_col);
  RenderNavHighlight(bb, id);

  // Select with right mouse button. This is so the common idiom for context
  // menu automatically highlight the current widget.
  const bool hovered_unblocked =
      IsItemHovered(HoveredFlags_AllowWhenBlockedByPopup);
  if (hovered_unblocked && (IsMouseClicked(1) || IsMouseReleased(1)) &&
      !is_tab_button)
    TabBarQueueFocus(tab_bar, tab);

  if (tab_bar->Flags & TabBarFlags_NoCloseWithMiddleMouseButton)
    flags |= TabItemFlags_NoCloseWithMiddleMouseButton;

  // Render tab label, process close button
  const ID close_button_id =
      p_open ? GetIDWithSeed("#CLOSE", NULL,
                             docked_window ? docked_window->ID : id)
             : 0;
  bool just_closed;
  bool text_clipped;
  TabItemLabelAndCloseButton(
      display_draw_list, bb,
      tab_just_unsaved ? (flags & ~TabItemFlags_UnsavedDocument) : flags,
      tab_bar->FramePadding, label, id, close_button_id, tab_contents_visible,
      &just_closed, &text_clipped);
  if (just_closed && p_open != NULL) {
    *p_open = false;
    TabBarCloseTab(tab_bar, tab);
  }

  // Forward Hovered state so IsItemHovered() after Begin() can work (even
  // though we are technically hovering our parent) That state is copied to
  // window->DockTabItemStatusFlags by our caller.
  if (docked_window && (hovered || g.HoveredId == close_button_id))
    g.LastItemData.StatusFlags |= ItemStatusFlags_HoveredWindow;

  // Restore main window position so user can draw there
  if (want_clip_rect)
    PopClipRect();
  window->DC.CursorPos = backup_main_cursor_pos;

  // Tooltip
  // (Won't work over the close button because ItemOverlap systems messes up
  // with HoveredIdTimer-> seems ok) (We test IsItemHovered() to discard e.g.
  // when another item is active or drag and drop over the tab bar, which
  // g.HoveredId ignores)
  // FIXME: This is a mess.
  // FIXME: We may want disabled tab to still display the tooltip?
  if (text_clipped && g.HoveredId == id && !held)
    if (!(tab_bar->Flags & TabBarFlags_NoTooltip) &&
        !(tab->Flags & TabItemFlags_NoTooltip))
      SetItemTooltip("%.*s", (int)(FindRenderedTextEnd(label) - label), label);

  ASSERT(!is_tab_button ||
         !(tab_bar->SelectedTabId == tab->ID &&
           is_tab_button)); // TabItemButton should not be selected
  if (is_tab_button)
    return pressed;
  return tab_contents_visible;
}

// [Public] This is call is 100% optional but it allows to remove some one-frame
// glitches when a tab has been unexpectedly removed. To use it to need to call
// the function SetTabItemClosed() between BeginTabBar() and EndTabBar(). Tabs
// closed by the close button will automatically be flagged to avoid this issue.
void Gui::SetTabItemClosed(const char *label) {
  Context &g = *GGui;
  bool is_within_manual_tab_bar =
      g.CurrentTabBar && !(g.CurrentTabBar->Flags & TabBarFlags_DockNode);
  if (is_within_manual_tab_bar) {
    TabBar *tab_bar = g.CurrentTabBar;
    ID tab_id = TabBarCalcTabID(tab_bar, label, NULL);
    if (TabItem *tab = TabBarFindTabByID(tab_bar, tab_id))
      tab->WantClose = true; // Will be processed by next call to TabBarLayout()
  } else if (Window *window = FindWindowByName(label)) {
    if (window->DockIsActive)
      if (DockNode *node = window->DockNode) {
        ID tab_id = TabBarCalcTabID(node->TabBar, label, window);
        TabBarRemoveTab(node->TabBar, tab_id);
        window->DockTabWantClose = true;
      }
  }
}

Vec2 Gui::TabItemCalcSize(const char *label,
                          bool has_close_button_or_unsaved_marker) {
  Context &g = *GGui;
  Vec2 label_size = CalcTextSize(label, NULL, true);
  Vec2 size = Vec2(label_size.x + g.Style.FramePadding.x,
                   label_size.y + g.Style.FramePadding.y * 2.0f);
  if (has_close_button_or_unsaved_marker)
    size.x +=
        g.Style.FramePadding.x +
        (g.Style.ItemInnerSpacing.x +
         g.FontSize); // We use Y intentionally to fit the close button circle.
  else
    size.x += g.Style.FramePadding.x + 1.0f;
  return Vec2(Min(size.x, TabBarCalcMaxTabWidth()), size.y);
}

Vec2 Gui::TabItemCalcSize(Window *window) {
  return TabItemCalcSize(window->Name,
                         window->HasCloseButton ||
                             (window->Flags & WindowFlags_UnsavedDocument));
}

void Gui::TabItemBackground(DrawList *draw_list, const Rect &bb,
                            TabItemFlags flags, U32 col) {
  // While rendering tabs, we trim 1 pixel off the top of our bounding box so
  // they can fit within a regular frame height while looking "detached" from
  // it.
  Context &g = *GGui;
  const float width = bb.GetWidth();
  UNUSED(flags);
  ASSERT(width > 0.0f);
  const float rounding =
      Max(0.0f, Min((flags & TabItemFlags_Button) ? g.Style.FrameRounding
                                                  : g.Style.TabRounding,
                    width * 0.5f - 1.0f));
  const float y1 = bb.Min.y + 1.0f;
  const float y2 = bb.Max.y - g.Style.TabBarBorderSize;
  draw_list->PathLineTo(Vec2(bb.Min.x, y2));
  draw_list->PathArcToFast(Vec2(bb.Min.x + rounding, y1 + rounding), rounding,
                           6, 9);
  draw_list->PathArcToFast(Vec2(bb.Max.x - rounding, y1 + rounding), rounding,
                           9, 12);
  draw_list->PathLineTo(Vec2(bb.Max.x, y2));
  draw_list->PathFillConvex(col);
  if (g.Style.TabBorderSize > 0.0f) {
    draw_list->PathLineTo(Vec2(bb.Min.x + 0.5f, y2));
    draw_list->PathArcToFast(
        Vec2(bb.Min.x + rounding + 0.5f, y1 + rounding + 0.5f), rounding, 6, 9);
    draw_list->PathArcToFast(
        Vec2(bb.Max.x - rounding - 0.5f, y1 + rounding + 0.5f), rounding, 9,
        12);
    draw_list->PathLineTo(Vec2(bb.Max.x - 0.5f, y2));
    draw_list->PathStroke(GetColorU32(Col_Border), 0, g.Style.TabBorderSize);
  }
}

// Render text label (with custom clipping) + Unsaved Document marker + Close
// Button logic We tend to lock style.FramePadding for a given tab-bar, hence
// the 'frame_padding' parameter.
void Gui::TabItemLabelAndCloseButton(
    DrawList *draw_list, const Rect &bb, TabItemFlags flags, Vec2 frame_padding,
    const char *label, ID tab_id, ID close_button_id, bool is_contents_visible,
    bool *out_just_closed, bool *out_text_clipped) {
  Context &g = *GGui;
  Vec2 label_size = CalcTextSize(label, NULL, true);

  if (out_just_closed)
    *out_just_closed = false;
  if (out_text_clipped)
    *out_text_clipped = false;

  if (bb.GetWidth() <= 1.0f)
    return;

    // In Style V2 we'll have full override of all colors per state (e.g.
    // focused, selected) But right now if you want to alter text color of tabs
    // this is what you need to do.
#if 0
    const float backup_alpha = g.Style.Alpha;
    if (!is_contents_visible)
        g.Style.Alpha *= 0.7f;
#endif

  // Render text label (with clipping + alpha gradient) + unsaved marker
  Rect text_pixel_clip_bb(bb.Min.x + frame_padding.x,
                          bb.Min.y + frame_padding.y,
                          bb.Max.x - frame_padding.x, bb.Max.y);
  Rect text_ellipsis_clip_bb = text_pixel_clip_bb;

  // Return clipped state ignoring the close button
  if (out_text_clipped) {
    *out_text_clipped =
        (text_ellipsis_clip_bb.Min.x + label_size.x) > text_pixel_clip_bb.Max.x;
    // draw_list->AddCircle(text_ellipsis_clip_bb.Min, 3.0f, *out_text_clipped ?
    // COL32(255, 0, 0, 255) : COL32(0, 255, 0, 255));
  }

  const float button_sz = g.FontSize;
  const Vec2 button_pos(Max(bb.Min.x, bb.Max.x - frame_padding.x - button_sz),
                        bb.Min.y + frame_padding.y);

  // Close Button & Unsaved Marker
  // We are relying on a subtle and confusing distinction between 'hovered' and
  // 'g.HoveredId' which happens because we are using
  // ButtonFlags_AllowOverlapMode + SetItemAllowOverlap()
  //  'hovered' will be true when hovering the Tab but NOT when hovering the
  //  close button 'g.HoveredId==id' will be true when hovering the Tab
  //  including when hovering the close button 'g.ActiveId==close_button_id'
  //  will be true when we are holding on the close button, in which case both
  //  hovered booleans are false
  bool close_button_pressed = false;
  bool close_button_visible = false;
  if (close_button_id != 0)
    if (is_contents_visible ||
        bb.GetWidth() >= Max(button_sz, g.Style.TabMinWidthForCloseButton))
      if (g.HoveredId == tab_id || g.HoveredId == close_button_id ||
          g.ActiveId == tab_id || g.ActiveId == close_button_id)
        close_button_visible = true;
  bool unsaved_marker_visible = (flags & TabItemFlags_UnsavedDocument) != 0 &&
                                (button_pos.x + button_sz <= bb.Max.x);

  if (close_button_visible) {
    LastItemData last_item_backup = g.LastItemData;
    if (CloseButton(close_button_id, button_pos))
      close_button_pressed = true;
    g.LastItemData = last_item_backup;

    // Close with middle mouse button
    if (!(flags & TabItemFlags_NoCloseWithMiddleMouseButton) &&
        IsMouseClicked(2))
      close_button_pressed = true;
  } else if (unsaved_marker_visible) {
    const Rect bullet_bb(button_pos, button_pos + Vec2(button_sz, button_sz));
    RenderBullet(draw_list, bullet_bb.GetCenter(), GetColorU32(Col_Text));
  }

  // This is all rather complicated
  // (the main idea is that because the close button only appears on hover, we
  // don't want it to alter the ellipsis position)
  // FIXME: if FramePadding is noticeably large, ellipsis_max_x will be wrong
  // here (e.g. #3497), maybe for consistency that parameter of
  // RenderTextEllipsis() shouldn't exist..
  float ellipsis_max_x =
      close_button_visible ? text_pixel_clip_bb.Max.x : bb.Max.x - 1.0f;
  if (close_button_visible || unsaved_marker_visible) {
    text_pixel_clip_bb.Max.x -=
        close_button_visible ? (button_sz) : (button_sz * 0.80f);
    text_ellipsis_clip_bb.Max.x -=
        unsaved_marker_visible ? (button_sz * 0.80f) : 0.0f;
    ellipsis_max_x = text_pixel_clip_bb.Max.x;
  }
  RenderTextEllipsis(draw_list, text_ellipsis_clip_bb.Min,
                     text_ellipsis_clip_bb.Max, text_pixel_clip_bb.Max.x,
                     ellipsis_max_x, label, NULL, &label_size);

#if 0
    if (!is_contents_visible)
        g.Style.Alpha = backup_alpha;
#endif

  if (out_just_closed)
    *out_just_closed = close_button_pressed;
}

#endif // #ifndef DISABLE
