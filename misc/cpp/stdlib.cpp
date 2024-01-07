// gui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

#include "stdlib.h"
#include "../../gui.hpp"
#include <string>

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wsign-conversion" // warning: implicit conversion changes signedness
#endif

struct InputTextCallback_UserData {
  std::string *Str;
  InputTextCallback ChainCallback;
  void *ChainCallbackUserData;
};

static int InputTextCallback(InputTextCallbackData *data) {
  InputTextCallback_UserData *user_data =
      (InputTextCallback_UserData *)data->UserData;
  if (data->EventFlag == InputTextFlags_CallbackResize) {
    // Resize string callback
    // If for some reason we refuse the new length (BufTextLen) and/or capacity
    // (BufSize) we need to set them back to what we want.
    std::string *str = user_data->Str;
    ASSERT(data->Buf == str->c_str());
    str->resize(data->BufTextLen);
    data->Buf = (char *)str->c_str();
  } else if (user_data->ChainCallback) {
    // Forward to user callback, if any
    data->UserData = user_data->ChainCallbackUserData;
    return user_data->ChainCallback(data);
  }
  return 0;
}

bool Gui::InputText(const char *label, std::string *str, int flags,
                    InputTextCallback callback, void *user_data) {
  ASSERT((flags & InputTextFlags_CallbackResize) == 0);
  flags |= InputTextFlags_CallbackResize;

  InputTextCallback_UserData cb_user_data;
  cb_user_data.Str = str;
  cb_user_data.ChainCallback = callback;
  cb_user_data.ChainCallbackUserData = user_data;
  return InputText(label, (char *)str->c_str(), str->capacity() + 1, flags,
                   InputTextCallback, &cb_user_data);
}

bool Gui::InputTextMultiline(const char *label, std::string *str,
                             const Vec2 &size, int flags,
                             InputTextCallback callback, void *user_data) {
  ASSERT((flags & InputTextFlags_CallbackResize) == 0);
  flags |= InputTextFlags_CallbackResize;

  InputTextCallback_UserData cb_user_data;
  cb_user_data.Str = str;
  cb_user_data.ChainCallback = callback;
  cb_user_data.ChainCallbackUserData = user_data;
  return InputTextMultiline(label, (char *)str->c_str(), str->capacity() + 1,
                            size, flags, InputTextCallback, &cb_user_data);
}

bool Gui::InputTextWithHint(const char *label, const char *hint,
                            std::string *str, int flags,
                            InputTextCallback callback, void *user_data) {
  ASSERT((flags & InputTextFlags_CallbackResize) == 0);
  flags |= InputTextFlags_CallbackResize;

  InputTextCallback_UserData cb_user_data;
  cb_user_data.Str = str;
  cb_user_data.ChainCallback = callback;
  cb_user_data.ChainCallbackUserData = user_data;
  return InputTextWithHint(label, hint, (char *)str->c_str(),
                           str->capacity() + 1, flags, InputTextCallback,
                           &cb_user_data);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
