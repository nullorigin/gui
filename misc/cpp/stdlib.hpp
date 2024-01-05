// dear imgui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with
// std::string

// See more C++ related extension (fmt, RAII, syntaxis sugar) on Wiki:
//   https://github.com/ocornut/imgui/wiki/Useful-Extensions#cness

#pragma once

#include <string>

namespace Gui {
// Gui::InputText() with std::string
// Because text input needs dynamic resizing, we need to setup a callback to
// grow the capacity
API bool InputText(const char *label, std::string *str,
                   InputTextFlags flags = 0,
                   InputTextCallback callback = nullptr,
                   void *user_data = nullptr);
API bool InputTextMultiline(const char *label, std::string *str,
                            const Vec2 &size = Vec2(0, 0),
                            InputTextFlags flags = 0,
                            InputTextCallback callback = nullptr,
                            void *user_data = nullptr);
API bool InputTextWithHint(const char *label, const char *hint,
                           std::string *str, InputTextFlags flags = 0,
                           InputTextCallback callback = nullptr,
                           void *user_data = nullptr);
} // namespace Gui
