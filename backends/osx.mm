// gui: Platform Backend for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan,
// Metal..)
// - Not well tested. If you want a portable application, prefer using the GLFW
// or SDL platform Backends on Mac.
// - Requires linking with the GameController framework ("-framework
// GameController").

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with
//  'io.ConfigFlags |= ConfigFlags_NoMouseCursorChange'. [X] Platform:
//  Mouse support. Can discriminate Mouse/Pen. [X] Platform: Keyboard support.
//  Since 1.87 we are using the io.AddKeyEvent() function. Pass Key values
//  to all key functions e.g. Gui::IsKeyPressed(Key_Space). [Legacy kVK_*
//  values will also be supported unless DISABLE_OBSOLETE_KEYIO is set] [X]
//  Platform: OSX clipboard is supported within core Gui (no specific
//  code in this backend). [X] Platform: Gamepad support. Enabled with
//  'io.ConfigFlags |= ConfigFlags_NavEnableGamepad'. [X] Platform: IME
//  support. [X] Platform: Multi-viewport / platform windows.

#import "../gui.hpp"
#ifndef DISABLE
#import "osx.hpp"
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>
#import <time.h>

#define APPLE_HAS_BUTTON_OPTIONS                                               \
  (__IPHONE_OS_VERSION_MIN_REQUIRED >= 130000 ||                               \
   __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500 ||                                \
   __TV_OS_VERSION_MIN_REQUIRED >= 130000)
#define APPLE_HAS_CONTROLLER                                                   \
  (__IPHONE_OS_VERSION_MIN_REQUIRED >= 140000 ||                               \
   __MAC_OS_X_VERSION_MIN_REQUIRED >= 110000 ||                                \
   __TV_OS_VERSION_MIN_REQUIRED >= 140000)
#define APPLE_HAS_THUMBSTICKS                                                  \
  (__IPHONE_OS_VERSION_MIN_REQUIRED >= 120100 ||                               \
   __MAC_OS_X_VERSION_MIN_REQUIRED >= 101401 ||                                \
   __TV_OS_VERSION_MIN_REQUIRED >= 120100)

@class Observer;
@class KeyEventResponder;

// Data
struct OSX_Data {
  CFTimeInterval Time;
  NSCursor *MouseCursors[MouseCursor_COUNT];
  bool MouseCursorHidden;
  Observer *Observer;
  KeyEventResponder *KeyEventResponder;
  NSTextInputContext *InputContext;
  id Monitor;
  NSWindow *Window;

  OSX_Data() { memset(this, 0, sizeof(*this)); }
};

static OSX_Data *OSX_CreateBackendData() { return NEW(OSX_Data)(); }
static OSX_Data *OSX_GetBackendData() {
  return (OSX_Data *)Gui::GetIO().BackendPlatformUserData;
}
static void OSX_DestroyBackendData() { DELETE(OSX_GetBackendData()); }

static inline CFTimeInterval GetMachAbsoluteTimeInSeconds() {
  return (CFTimeInterval)(double)(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) /
                                  1e9);
}

// Forward Declarations
static void OSX_InitPlatformInterface();
static void OSX_ShutdownPlatformInterface();
static void OSX_UpdateMonitors();
static void OSX_AddTrackingArea(NSView *_Nonnull view);
static bool OSX_HandleEvent(NSEvent *event, NSView *view);

// Undocumented methods for creating cursors.
@interface NSCursor ()
+ (id)_windowResizeNorthWestSouthEastCursor;
+ (id)_windowResizeNorthEastSouthWestCursor;
+ (id)_windowResizeNorthSouthCursor;
+ (id)_windowResizeEastWestCursor;
@end

/**
 KeyEventResponder implements the NSTextInputClient protocol as is required by
 the macOS text input manager.

 The macOS text input manager is invoked by calling the interpretKeyEvents
 method from the keyDown method. Keyboard events are then evaluated by the macOS
 input manager and valid text input is passed back via the
 insertText:replacementRange method.

 This is the same approach employed by other cross-platform libraries such as
 SDL2:
  https://github.com/spurious/SDL-mirror/blob/e17aacbd09e65a4fd1e166621e011e581fb017a8/src/video/cocoa/SDL_cocoakeyboard.m#L53
 and GLFW:
  https://github.com/glfw/glfw/blob/b55a517ae0c7b5127dffa79a64f5406021bf9076/src/cocoa_window.m#L722-L723
 */
@interface KeyEventResponder : NSView <NSTextInputClient>
@end

@implementation KeyEventResponder {
  float _posX;
  float _posY;
  NSRect _imeRect;
}

#pragma mark - Public

- (void)setImePosX:(float)posX imePosY:(float)posY {
  _posX = posX;
  _posY = posY;
}

- (void)updateImePosWithView:(NSView *)view {
  NSWindow *window = view.window;
  if (!window)
    return;
  NSRect contentRect = [window contentRectForFrameRect:window.frame];
  NSRect rect = NSMakeRect(_posX, contentRect.size.height - _posY, 0, 0);
  _imeRect = [window convertRectToScreen:rect];
}

- (void)viewDidMoveToWindow {
  // Ensure self is a first responder to receive the input events.
  [self.window makeFirstResponder:self];
}

- (void)keyDown:(NSEvent *)event {
  if (!OSX_HandleEvent(event, self))
    [super keyDown:event];

  // Call to the macOS input manager system.
  [self interpretKeyEvents:@[ event ]];
}

- (void)keyUp:(NSEvent *)event {
  if (!OSX_HandleEvent(event, self))
    [super keyUp:event];
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange {
  IO &io = Gui::GetIO();

  NSString *characters;
  if ([aString isKindOfClass:[NSAttributedString class]])
    characters = [aString string];
  else
    characters = (NSString *)aString;

  io.AddInputCharactersUTF8(characters.UTF8String);
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)doCommandBySelector:(SEL)myselector {
}

- (nullable NSAttributedString *)
    attributedSubstringForProposedRange:(NSRange)range
                            actualRange:(nullable NSRangePointer)actualRange {
  return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
  return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(nullable NSRangePointer)actualRange {
  return _imeRect;
}

- (BOOL)hasMarkedText {
  return NO;
}

- (NSRange)markedRange {
  return NSMakeRange(NSNotFound, 0);
}

- (NSRange)selectedRange {
  return NSMakeRange(NSNotFound, 0);
}

- (void)setMarkedText:(nonnull id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange {
}

- (void)unmarkText {
}

- (nonnull NSArray<NSAttributedStringKey> *)validAttributesForMarkedText {
  return @[];
}

@end

@interface Observer : NSObject

- (void)onApplicationBecomeActive:(NSNotification *)aNotification;
- (void)onApplicationBecomeInactive:(NSNotification *)aNotification;
- (void)displaysDidChange:(NSNotification *)aNotification;

@end

@implementation Observer

- (void)onApplicationBecomeActive:(NSNotification *)aNotification {
  IO &io = Gui::GetIO();
  io.AddFocusEvent(true);
}

- (void)onApplicationBecomeInactive:(NSNotification *)aNotification {
  IO &io = Gui::GetIO();
  io.AddFocusEvent(false);
}

- (void)displaysDidChange:(NSNotification *)aNotification {
  OSX_UpdateMonitors();
}

@end

// Functions
static Key OSX_KeyCodeToKey(int key_code) {
  switch (key_code) {
  case kVK_ANSI_A:
    return Key_A;
  case kVK_ANSI_S:
    return Key_S;
  case kVK_ANSI_D:
    return Key_D;
  case kVK_ANSI_F:
    return Key_F;
  case kVK_ANSI_H:
    return Key_H;
  case kVK_ANSI_G:
    return Key_G;
  case kVK_ANSI_Z:
    return Key_Z;
  case kVK_ANSI_X:
    return Key_X;
  case kVK_ANSI_C:
    return Key_C;
  case kVK_ANSI_V:
    return Key_V;
  case kVK_ANSI_B:
    return Key_B;
  case kVK_ANSI_Q:
    return Key_Q;
  case kVK_ANSI_W:
    return Key_W;
  case kVK_ANSI_E:
    return Key_E;
  case kVK_ANSI_R:
    return Key_R;
  case kVK_ANSI_Y:
    return Key_Y;
  case kVK_ANSI_T:
    return Key_T;
  case kVK_ANSI_1:
    return Key_1;
  case kVK_ANSI_2:
    return Key_2;
  case kVK_ANSI_3:
    return Key_3;
  case kVK_ANSI_4:
    return Key_4;
  case kVK_ANSI_6:
    return Key_6;
  case kVK_ANSI_5:
    return Key_5;
  case kVK_ANSI_Equal:
    return Key_Equal;
  case kVK_ANSI_9:
    return Key_9;
  case kVK_ANSI_7:
    return Key_7;
  case kVK_ANSI_Minus:
    return Key_Minus;
  case kVK_ANSI_8:
    return Key_8;
  case kVK_ANSI_0:
    return Key_0;
  case kVK_ANSI_RightBracket:
    return Key_RightBracket;
  case kVK_ANSI_O:
    return Key_O;
  case kVK_ANSI_U:
    return Key_U;
  case kVK_ANSI_LeftBracket:
    return Key_LeftBracket;
  case kVK_ANSI_I:
    return Key_I;
  case kVK_ANSI_P:
    return Key_P;
  case kVK_ANSI_L:
    return Key_L;
  case kVK_ANSI_J:
    return Key_J;
  case kVK_ANSI_Quote:
    return Key_Apostrophe;
  case kVK_ANSI_K:
    return Key_K;
  case kVK_ANSI_Semicolon:
    return Key_Semicolon;
  case kVK_ANSI_Backslash:
    return Key_Backslash;
  case kVK_ANSI_Comma:
    return Key_Comma;
  case kVK_ANSI_Slash:
    return Key_Slash;
  case kVK_ANSI_N:
    return Key_N;
  case kVK_ANSI_M:
    return Key_M;
  case kVK_ANSI_Period:
    return Key_Period;
  case kVK_ANSI_Grave:
    return Key_GraveAccent;
  case kVK_ANSI_KeypadDecimal:
    return Key_KeypadDecimal;
  case kVK_ANSI_KeypadMultiply:
    return Key_KeypadMultiply;
  case kVK_ANSI_KeypadPlus:
    return Key_KeypadAdd;
  case kVK_ANSI_KeypadClear:
    return Key_NumLock;
  case kVK_ANSI_KeypadDivide:
    return Key_KeypadDivide;
  case kVK_ANSI_KeypadEnter:
    return Key_KeypadEnter;
  case kVK_ANSI_KeypadMinus:
    return Key_KeypadSubtract;
  case kVK_ANSI_KeypadEquals:
    return Key_KeypadEqual;
  case kVK_ANSI_Keypad0:
    return Key_Keypad0;
  case kVK_ANSI_Keypad1:
    return Key_Keypad1;
  case kVK_ANSI_Keypad2:
    return Key_Keypad2;
  case kVK_ANSI_Keypad3:
    return Key_Keypad3;
  case kVK_ANSI_Keypad4:
    return Key_Keypad4;
  case kVK_ANSI_Keypad5:
    return Key_Keypad5;
  case kVK_ANSI_Keypad6:
    return Key_Keypad6;
  case kVK_ANSI_Keypad7:
    return Key_Keypad7;
  case kVK_ANSI_Keypad8:
    return Key_Keypad8;
  case kVK_ANSI_Keypad9:
    return Key_Keypad9;
  case kVK_Return:
    return Key_Enter;
  case kVK_Tab:
    return Key_Tab;
  case kVK_Space:
    return Key_Space;
  case kVK_Delete:
    return Key_Backspace;
  case kVK_Escape:
    return Key_Escape;
  case kVK_CapsLock:
    return Key_CapsLock;
  case kVK_Control:
    return Key_LeftCtrl;
  case kVK_Shift:
    return Key_LeftShift;
  case kVK_Option:
    return Key_LeftAlt;
  case kVK_Command:
    return Key_LeftSuper;
  case kVK_RightControl:
    return Key_RightCtrl;
  case kVK_RightShift:
    return Key_RightShift;
  case kVK_RightOption:
    return Key_RightAlt;
  case kVK_RightCommand:
    return Key_RightSuper;
    //      case kVK_Function: return Key_;
    //      case kVK_VolumeUp: return Key_;
    //      case kVK_VolumeDown: return Key_;
    //      case kVK_Mute: return Key_;
  case kVK_F1:
    return Key_F1;
  case kVK_F2:
    return Key_F2;
  case kVK_F3:
    return Key_F3;
  case kVK_F4:
    return Key_F4;
  case kVK_F5:
    return Key_F5;
  case kVK_F6:
    return Key_F6;
  case kVK_F7:
    return Key_F7;
  case kVK_F8:
    return Key_F8;
  case kVK_F9:
    return Key_F9;
  case kVK_F10:
    return Key_F10;
  case kVK_F11:
    return Key_F11;
  case kVK_F12:
    return Key_F12;
  case kVK_F13:
    return Key_F13;
  case kVK_F14:
    return Key_F14;
  case kVK_F15:
    return Key_F15;
  case kVK_F16:
    return Key_F16;
  case kVK_F17:
    return Key_F17;
  case kVK_F18:
    return Key_F18;
  case kVK_F19:
    return Key_F19;
  case kVK_F20:
    return Key_F20;
  case 0x6E:
    return Key_Menu;
  case kVK_Help:
    return Key_Insert;
  case kVK_Home:
    return Key_Home;
  case kVK_PageUp:
    return Key_PageUp;
  case kVK_ForwardDelete:
    return Key_Delete;
  case kVK_End:
    return Key_End;
  case kVK_PageDown:
    return Key_PageDown;
  case kVK_LeftArrow:
    return Key_LeftArrow;
  case kVK_RightArrow:
    return Key_RightArrow;
  case kVK_DownArrow:
    return Key_DownArrow;
  case kVK_UpArrow:
    return Key_UpArrow;
  default:
    return Key_None;
  }
}

#ifdef METAL_CPP_EXTENSIONS

API bool OSX_Init(void *_Nonnull view) {
  return OSX_Init((__bridge NSView *)(view));
}

API void OSX_NewFrame(void *_Nullable view) {
  return OSX_NewFrame((__bridge NSView *)(view));
}

#endif

bool OSX_Init(NSView *view) {
  IO &io = Gui::GetIO();
  OSX_Data *bd = OSX_CreateBackendData();
  io.BackendPlatformUserData = (void *)bd;

  // Setup backend capabilities flags
  io.BackendFlags |=
      BackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
                                    // values (optional)
  // io.BackendFlags |= BackendFlags_HasSetMousePos;          // We can
  // honor io.WantSetMousePos requests (optional, rarely used)
  io.BackendFlags |=
      BackendFlags_PlatformHasViewports; // We can create multi-viewports
                                         // on the Platform side (optional)
  // io.BackendFlags |= BackendFlags_HasMouseHoveredViewport; // We can
  // call io.AddMouseViewportEvent() with correct data (optional)
  io.BackendPlatformName = "osx";

  bd->Observer = [Observer new];
  bd->Window = view.window ?: NSApp.orderedWindows.firstObject;
  Viewport *main_viewport = Gui::GetMainViewport();
  main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw =
      (__bridge_retained void *)bd->Window;
  if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
    OSX_InitPlatformInterface();

  // Load cursors. Some of them are undocumented.
  bd->MouseCursorHidden = false;
  bd->MouseCursors[MouseCursor_Arrow] = [NSCursor arrowCursor];
  bd->MouseCursors[MouseCursor_TextInput] = [NSCursor IBeamCursor];
  bd->MouseCursors[MouseCursor_ResizeAll] = [NSCursor closedHandCursor];
  bd->MouseCursors[MouseCursor_Hand] = [NSCursor pointingHandCursor];
  bd->MouseCursors[MouseCursor_NotAllowed] =
      [NSCursor operationNotAllowedCursor];
  bd->MouseCursors[MouseCursor_ResizeNS] =
      [NSCursor respondsToSelector:@selector(_windowResizeNorthSouthCursor)]
          ? [NSCursor _windowResizeNorthSouthCursor]
          : [NSCursor resizeUpDownCursor];
  bd->MouseCursors[MouseCursor_ResizeEW] =
      [NSCursor respondsToSelector:@selector(_windowResizeEastWestCursor)]
          ? [NSCursor _windowResizeEastWestCursor]
          : [NSCursor resizeLeftRightCursor];
  bd->MouseCursors[MouseCursor_ResizeNESW] =
      [NSCursor
          respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)]
          ? [NSCursor _windowResizeNorthEastSouthWestCursor]
          : [NSCursor closedHandCursor];
  bd->MouseCursors[MouseCursor_ResizeNWSE] =
      [NSCursor
          respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)]
          ? [NSCursor _windowResizeNorthWestSouthEastCursor]
          : [NSCursor closedHandCursor];

  // Note that gui.cpp also include default OSX clipboard handlers which can
  // be enabled by adding '#define ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS' in
  // config.hpp and adding '-framework ApplicationServices' to your linker
  // command-line. Since we are already in ObjC land here, it is easy for us to
  // add a clipboard handler using the NSPasteboard api.
  io.SetClipboardTextFn = [](void *, const char *str) -> void {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString]
                       owner:nil];
    [pasteboard setString:[NSString stringWithUTF8String:str]
                  forType:NSPasteboardTypeString];
  };

  io.GetClipboardTextFn = [](void *) -> const char * {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSString *available = [pasteboard
        availableTypeFromArray:[NSArray
                                   arrayWithObject:NSPasteboardTypeString]];
    if (![available isEqualToString:NSPasteboardTypeString])
      return nullptr;

    NSString *string = [pasteboard stringForType:NSPasteboardTypeString];
    if (string == nil)
      return nullptr;

    const char *string_c = (const char *)[string UTF8String];
    size_t string_len = strlen(string_c);
    static Vector<char> s_clipboard;
    s_clipboard.resize((int)string_len + 1);
    strcpy(s_clipboard.Data, string_c);
    return s_clipboard.Data;
  };

  [[NSNotificationCenter defaultCenter]
      addObserver:bd->Observer
         selector:@selector(onApplicationBecomeActive:)
             name:NSApplicationDidBecomeActiveNotification
           object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:bd->Observer
         selector:@selector(onApplicationBecomeInactive:)
             name:NSApplicationDidResignActiveNotification
           object:nil];

  // Add the NSTextInputClient to the view hierarchy,
  // to receive keyboard events and translate them to input text.
  bd->KeyEventResponder = [[KeyEventResponder alloc] initWithFrame:NSZeroRect];
  bd->InputContext =
      [[NSTextInputContext alloc] initWithClient:bd->KeyEventResponder];
  [view addSubview:bd->KeyEventResponder];
  OSX_AddTrackingArea(view);

  io.SetPlatformImeDataFn = [](Viewport *viewport,
                               PlatformImeData *data) -> void {
    OSX_Data *bd = OSX_GetBackendData();
    if (data->WantVisible) {
      [bd->InputContext activate];
    } else {
      [bd->InputContext discardMarkedText];
      [bd->InputContext invalidateCharacterCoordinates];
      [bd->InputContext deactivate];
    }
    [bd->KeyEventResponder setImePosX:data->InputPos.x
                              imePosY:data->InputPos.y + data->InputLineHeight];
  };

  return true;
}

void OSX_Shutdown() {
  OSX_Data *bd = OSX_GetBackendData();
  assert(bd != nullptr &&
         "No platform backend to shutdown, or already shutdown?");

  bd->Observer = nullptr;
  if (bd->Monitor != nullptr) {
    [NSEvent removeMonitor:bd->Monitor];
    bd->Monitor = nullptr;
  }

  OSX_ShutdownPlatformInterface();
  OSX_DestroyBackendData();
  IO &io = Gui::GetIO();
  io.BackendPlatformName = nullptr;
  io.BackendPlatformUserData = nullptr;
  io.BackendFlags &= ~(BackendFlags_HasMouseCursors | BackendFlags_HasGamepad |
                       BackendFlags_PlatformHasViewports);
}

static void OSX_UpdateMouseCursor() {
  OSX_Data *bd = OSX_GetBackendData();
  IO &io = Gui::GetIO();
  if (io.ConfigFlags & ConfigFlags_NoMouseCursorChange)
    return;

  int cursor = Gui::GetMouseCursor();
  if (io.MouseDrawCursor || cursor == MouseCursor_None) {
    // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
    if (!bd->MouseCursorHidden) {
      bd->MouseCursorHidden = true;
      [NSCursor hide];
    }
  } else {
    NSCursor *desired =
        bd->MouseCursors[cursor] ?: bd->MouseCursors[MouseCursor_Arrow];
    // -[NSCursor set] generates measureable overhead if called unconditionally.
    if (desired != NSCursor.currentCursor) {
      [desired set];
    }
    if (bd->MouseCursorHidden) {
      bd->MouseCursorHidden = false;
      [NSCursor unhide];
    }
  }
}

static void OSX_UpdateGamepads() {
  IO &io = Gui::GetIO();
  memset(io.NavInputs, 0, sizeof(io.NavInputs));
  if ((io.ConfigFlags & ConfigFlags_NavEnableGamepad) ==
      0) // FIXME: Technically feeding gamepad shouldn't depend on this now that
         // they are regular inputs.
    return;

#if APPLE_HAS_CONTROLLER
  GCController *controller = GCController.current;
#else
  GCController *controller = GCController.controllers.firstObject;
#endif
  if (controller == nil || controller.extendedGamepad == nil) {
    io.BackendFlags &= ~BackendFlags_HasGamepad;
    return;
  }

  GCExtendedGamepad *gp = controller.extendedGamepad;

// Update gamepad inputs
#define SATURATE(V) (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
#define MAP_BUTTON(KEY_NO, BUTTON_NAME)                                        \
  { io.AddKeyEvent(KEY_NO, gp.BUTTON_NAME.isPressed); }
#define MAP_ANALOG(KEY_NO, AXIS_NAME, V0, V1)                                  \
  {                                                                            \
    float vn = (float)(gp.AXIS_NAME.value - V0) / (float)(V1 - V0);            \
    vn = SATURATE(vn);                                                         \
    io.AddKeyAnalogEvent(KEY_NO, vn > 0.1f, vn);                               \
  }
  const float thumb_dead_zone = 0.0f;

#if APPLE_HAS_BUTTON_OPTIONS
  MAP_BUTTON(Key_GamepadBack, buttonOptions);
#endif
  MAP_BUTTON(Key_GamepadFaceLeft, buttonX);  // Xbox X, PS Square
  MAP_BUTTON(Key_GamepadFaceRight, buttonB); // Xbox B, PS Circle
  MAP_BUTTON(Key_GamepadFaceUp, buttonY);    // Xbox Y, PS Triangle
  MAP_BUTTON(Key_GamepadFaceDown, buttonA);  // Xbox A, PS Cross
  MAP_BUTTON(Key_GamepadDpadLeft, dpad.left);
  MAP_BUTTON(Key_GamepadDpadRight, dpad.right);
  MAP_BUTTON(Key_GamepadDpadUp, dpad.up);
  MAP_BUTTON(Key_GamepadDpadDown, dpad.down);
  MAP_ANALOG(Key_GamepadL1, leftShoulder, 0.0f, 1.0f);
  MAP_ANALOG(Key_GamepadR1, rightShoulder, 0.0f, 1.0f);
  MAP_ANALOG(Key_GamepadL2, leftTrigger, 0.0f, 1.0f);
  MAP_ANALOG(Key_GamepadR2, rightTrigger, 0.0f, 1.0f);
#if APPLE_HAS_THUMBSTICKS
  MAP_BUTTON(Key_GamepadL3, leftThumbstickButton);
  MAP_BUTTON(Key_GamepadR3, rightThumbstickButton);
#endif
  MAP_ANALOG(Key_GamepadLStickLeft, leftThumbstick.xAxis, -thumb_dead_zone,
             -1.0f);
  MAP_ANALOG(Key_GamepadLStickRight, leftThumbstick.xAxis, +thumb_dead_zone,
             +1.0f);
  MAP_ANALOG(Key_GamepadLStickUp, leftThumbstick.yAxis, +thumb_dead_zone,
             +1.0f);
  MAP_ANALOG(Key_GamepadLStickDown, leftThumbstick.yAxis, -thumb_dead_zone,
             -1.0f);
  MAP_ANALOG(Key_GamepadRStickLeft, rightThumbstick.xAxis, -thumb_dead_zone,
             -1.0f);
  MAP_ANALOG(Key_GamepadRStickRight, rightThumbstick.xAxis, +thumb_dead_zone,
             +1.0f);
  MAP_ANALOG(Key_GamepadRStickUp, rightThumbstick.yAxis, +thumb_dead_zone,
             +1.0f);
  MAP_ANALOG(Key_GamepadRStickDown, rightThumbstick.yAxis, -thumb_dead_zone,
             -1.0f);
#undef MAP_BUTTON
#undef MAP_ANALOG

  io.BackendFlags |= BackendFlags_HasGamepad;
}

static void OSX_UpdateImePosWithView(NSView *view) {
  OSX_Data *bd = OSX_GetBackendData();
  IO &io = Gui::GetIO();
  if (io.WantTextInput)
    [bd->KeyEventResponder updateImePosWithView:view];
}

void OSX_NewFrame(NSView *view) {
  OSX_Data *bd = OSX_GetBackendData();
  IO &io = Gui::GetIO();

  // Setup display size
  if (view) {
    const float dpi = (float)[view.window backingScaleFactor];
    io.DisplaySize =
        Vec2((float)view.bounds.size.width, (float)view.bounds.size.height);
    io.DisplayFramebufferScale = Vec2(dpi, dpi);
  }

  // Setup time step
  if (bd->Time == 0.0)
    bd->Time = GetMachAbsoluteTimeInSeconds();

  double current_time = GetMachAbsoluteTimeInSeconds();
  io.DeltaTime = (float)(current_time - bd->Time);
  bd->Time = current_time;

  OSX_UpdateMouseCursor();
  OSX_UpdateGamepads();
  OSX_UpdateImePosWithView(view);
}

// Must only be called for a mouse event, otherwise an exception occurs
// (Note that NSEventTypeScrollWheel is considered "other input". Oddly enough
// an exception does not occur with it, but the value will sometimes be wrong!)
static MouseSource GetMouseSource(NSEvent *event) {
  switch (event.subtype) {
  case NSEventSubtypeTabletPoint:
    return MouseSource_Pen;
  // macOS considers input from relative touch devices (like the trackpad or
  // Apple Magic Mouse) to be touch input. This doesn't really make sense for
  // Gui, which expects absolute touch devices only. There does not seem
  // to be a simple way to disambiguate things here so we consider
  // NSEventSubtypeTouch events to always come from mice. See
  // https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/EventOverview/HandlingTouchEvents/HandlingTouchEvents.html#//apple_ref/doc/uid/10000060i-CH13-SW24
  // case NSEventSubtypeTouch:
  //    return MouseSource_TouchScreen;
  case NSEventSubtypeMouseEvent:
  default:
    return MouseSource_Mouse;
  }
}

static bool OSX_HandleEvent(NSEvent *event, NSView *view) {
  IO &io = Gui::GetIO();

  if (event.type == NSEventTypeLeftMouseDown ||
      event.type == NSEventTypeRightMouseDown ||
      event.type == NSEventTypeOtherMouseDown) {
    int button = (int)[event buttonNumber];
    if (button >= 0 && button < MouseButton_COUNT) {
      io.AddMouseSourceEvent(GetMouseSource(event));
      io.AddMouseButtonEvent(button, true);
    }
    return io.WantCaptureMouse;
  }

  if (event.type == NSEventTypeLeftMouseUp ||
      event.type == NSEventTypeRightMouseUp ||
      event.type == NSEventTypeOtherMouseUp) {
    int button = (int)[event buttonNumber];
    if (button >= 0 && button < MouseButton_COUNT) {
      io.AddMouseSourceEvent(GetMouseSource(event));
      io.AddMouseButtonEvent(button, false);
    }
    return io.WantCaptureMouse;
  }

  if (event.type == NSEventTypeMouseMoved ||
      event.type == NSEventTypeLeftMouseDragged ||
      event.type == NSEventTypeRightMouseDragged ||
      event.type == NSEventTypeOtherMouseDragged) {
    NSPoint mousePoint;
    if (io.ConfigFlags & ConfigFlags_ViewportsEnable) {
      mousePoint = NSEvent.mouseLocation;
      mousePoint.y =
          CGDisplayPixelsHigh(kCGDirectMainDisplay) -
          mousePoint.y; // Normalize y coordinate to top-left of main display.
    } else {
      mousePoint = event.locationInWindow;
      if (event.window == nil)
        mousePoint = [[view window] convertPointFromScreen:mousePoint];
      mousePoint =
          [view convertPoint:mousePoint
                    fromView:nil]; // Convert to local coordinates of view
      if ([view isFlipped])
        mousePoint = NSMakePoint(mousePoint.x, mousePoint.y);
      else
        mousePoint =
            NSMakePoint(mousePoint.x, view.bounds.size.height - mousePoint.y);
    }
    io.AddMouseSourceEvent(GetMouseSource(event));
    io.AddMousePosEvent((float)mousePoint.x, (float)mousePoint.y);
    return io.WantCaptureMouse;
  }

  if (event.type == NSEventTypeScrollWheel) {
    // Ignore canceled events.
    //
    // From macOS 12.1, scrolling with two fingers and then decelerating
    // by tapping two fingers results in two events appearing:
    //
    // 1. A scroll wheel NSEvent, with a phase == NSEventPhaseMayBegin, when the
    // user taps two fingers to decelerate or stop the scroll events.
    //
    // 2. A scroll wheel NSEvent, with a phase == NSEventPhaseCancelled, when
    // the user releases the two-finger tap. It is this event that sometimes
    // contains large values for scrollingDeltaX and scrollingDeltaY. When these
    // are added to the current x and y positions of the scrolling view, it
    // appears to jump up or down. It can be observed in Preview, various
    // JetBrains IDEs and here.
    if (event.phase == NSEventPhaseCancelled)
      return false;

    double wheel_dx = 0.0;
    double wheel_dy = 0.0;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6) {
      wheel_dx = [event scrollingDeltaX];
      wheel_dy = [event scrollingDeltaY];
      if ([event hasPreciseScrollingDeltas]) {
        wheel_dx *= 0.01;
        wheel_dy *= 0.01;
      }
    } else
#endif // MAC_OS_X_VERSION_MAX_ALLOWED
    {
      wheel_dx = [event deltaX] * 0.1;
      wheel_dy = [event deltaY] * 0.1;
    }
    if (wheel_dx != 0.0 || wheel_dy != 0.0)
      io.AddMouseWheelEvent((float)wheel_dx, (float)wheel_dy);

    return io.WantCaptureMouse;
  }

  if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp) {
    if ([event isARepeat])
      return io.WantCaptureKeyboard;

    int key_code = (int)[event keyCode];
    Key key = OSX_KeyCodeToKey(key_code);
    io.AddKeyEvent(key, event.type == NSEventTypeKeyDown);
    io.SetKeyEventNativeData(
        key, key_code, -1); // To support legacy indexing (<1.87 user code)

    return io.WantCaptureKeyboard;
  }

  if (event.type == NSEventTypeFlagsChanged) {
    unsigned short key_code = [event keyCode];
    NSEventModifierFlags modifier_flags = [event modifierFlags];

    io.AddKeyEvent(Mod_Shift, (modifier_flags & NSEventModifierFlagShift) != 0);
    io.AddKeyEvent(Mod_Ctrl,
                   (modifier_flags & NSEventModifierFlagControl) != 0);
    io.AddKeyEvent(Mod_Alt, (modifier_flags & NSEventModifierFlagOption) != 0);
    io.AddKeyEvent(Mod_Super,
                   (modifier_flags & NSEventModifierFlagCommand) != 0);

    Key key = OSX_KeyCodeToKey(key_code);
    if (key != Key_None) {
      // macOS does not generate down/up event for modifiers. We're trying
      // to use hardware dependent masks to extract that information.
      // 'mask' is left as a fallback.
      NSEventModifierFlags mask = 0;
      switch (key) {
      case Key_LeftCtrl:
        mask = 0x0001;
        break;
      case Key_RightCtrl:
        mask = 0x2000;
        break;
      case Key_LeftShift:
        mask = 0x0002;
        break;
      case Key_RightShift:
        mask = 0x0004;
        break;
      case Key_LeftSuper:
        mask = 0x0008;
        break;
      case Key_RightSuper:
        mask = 0x0010;
        break;
      case Key_LeftAlt:
        mask = 0x0020;
        break;
      case Key_RightAlt:
        mask = 0x0040;
        break;
      default:
        return io.WantCaptureKeyboard;
      }

      NSEventModifierFlags modifier_flags = [event modifierFlags];
      io.AddKeyEvent(key, (modifier_flags & mask) != 0);
      io.SetKeyEventNativeData(
          key, key_code, -1); // To support legacy indexing (<1.87 user code)
    }

    return io.WantCaptureKeyboard;
  }

  return false;
}

static void OSX_AddTrackingArea(NSView *_Nonnull view) {
  // If we want to receive key events, we either need to be in the responder
  // chain of the key view, or else we can install a local monitor. The
  // consequence of this heavy-handed approach is that we receive events for all
  // controls, not just Gui widgets. If we had native controls in our
  // window, we'd want to be much more careful than just ingesting the complete
  // event stream. To match the behavior of other backends, we pass every event
  // down to the OS.
  OSX_Data *bd = OSX_GetBackendData();
  if (bd->Monitor)
    return;
  NSEventMask eventMask = 0;
  eventMask |= NSEventMaskMouseMoved | NSEventMaskScrollWheel;
  eventMask |= NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp |
               NSEventMaskLeftMouseDragged;
  eventMask |= NSEventMaskRightMouseDown | NSEventMaskRightMouseUp |
               NSEventMaskRightMouseDragged;
  eventMask |= NSEventMaskOtherMouseDown | NSEventMaskOtherMouseUp |
               NSEventMaskOtherMouseDragged;
  eventMask |= NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;
  bd->Monitor = [NSEvent
      addLocalMonitorForEventsMatchingMask:eventMask
                                   handler:^NSEvent *_Nullable(NSEvent *event) {
                                     OSX_HandleEvent(event, view);
                                     return event;
                                   }];
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create
// and handle multiple viewports simultaneously. If you are new to gui or
// creating a new binding for gui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct ViewportDataOSX {
  NSWindow *Window;
  bool WindowOwned;

  ViewportDataOSX() { WindowOwned = false; }
  ~ViewportDataOSX() { assert(Window == nil); }
};

@interface OSX_Window : NSWindow
@end

@implementation OSX_Window

- (BOOL)canBecomeKeyWindow {
  return YES;
}

@end

static void ConvertNSRect(NSScreen *screen, NSRect *r) {
  r->origin.y = screen.frame.size.height - r->origin.y - r->size.height;
}

static void OSX_CreateWindow(Viewport *viewport) {
  OSX_Data *bd = OSX_GetBackendData();
  ViewportDataOSX *data = NEW(ViewportDataOSX)();
  viewport->PlatformUserData = data;

  NSScreen *screen = bd->Window.screen;
  NSRect rect = NSMakeRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x,
                           viewport->Size.y);
  ConvertNSRect(screen, &rect);

  NSWindowStyleMask styleMask = 0;
  if (viewport->Flags & ViewportFlags_NoDecoration)
    styleMask |= NSWindowStyleMaskBorderless;
  else
    styleMask |= NSWindowStyleMaskTitled | NSWindowStyleMaskResizable |
                 NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

  NSWindow *window =
      [[OSX_Window alloc] initWithContentRect:rect
                                    styleMask:styleMask
                                      backing:NSBackingStoreBuffered
                                        defer:YES
                                       screen:screen];
  if (viewport->Flags & ViewportFlags_TopMost)
    [window setLevel:NSFloatingWindowLevel];

  window.title = @"Untitled";
  window.opaque = YES;

  KeyEventResponder *view = [[KeyEventResponder alloc] initWithFrame:rect];
  if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
    [view setWantsBestResolutionOpenGLSurface:YES];

  window.contentView = view;

  data->Window = window;
  data->WindowOwned = true;
  viewport->PlatformRequestResize = false;
  viewport->PlatformHandle = viewport->PlatformHandleRaw =
      (__bridge_retained void *)window;
}

static void OSX_DestroyWindow(Viewport *viewport) {
  NSWindow *window = (__bridge_transfer NSWindow *)viewport->PlatformHandleRaw;
  window = nil;

  if (ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData) {
    NSWindow *window = data->Window;
    if (window != nil && data->WindowOwned) {
      window.contentView = nil;
      window.contentViewController = nil;
      [window orderOut:nil];
    }
    data->Window = nil;
    DELETE(data);
  }
  viewport->PlatformUserData = viewport->PlatformHandle =
      viewport->PlatformHandleRaw = nullptr;
}

static void OSX_ShowWindow(Viewport *viewport) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  if (viewport->Flags & ViewportFlags_NoFocusOnAppearing)
    [data->Window orderFront:nil];
  else
    [data->Window makeKeyAndOrderFront:nil];

  [data->Window setIsVisible:YES];
}

static Vec2 OSX_GetWindowPos(Viewport *viewport) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  NSWindow *window = data->Window;
  NSScreen *screen = window.screen;
  NSSize size = screen.frame.size;
  NSRect frame = window.frame;
  NSRect rect = window.contentLayoutRect;
  return Vec2(frame.origin.x, size.height - frame.origin.y - rect.size.height);
}

static void OSX_SetWindowPos(Viewport *viewport, Vec2 pos) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  NSWindow *window = data->Window;
  NSSize size = window.frame.size;

  NSRect r = NSMakeRect(pos.x, pos.y, size.width, size.height);
  ConvertNSRect(window.screen, &r);
  [window setFrameOrigin:r.origin];
}

static Vec2 OSX_GetWindowSize(Viewport *viewport) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  NSWindow *window = data->Window;
  NSSize size = window.contentLayoutRect.size;
  return Vec2(size.width, size.height);
}

static void OSX_SetWindowSize(Viewport *viewport, Vec2 size) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  NSWindow *window = data->Window;
  NSRect rect = window.frame;
  rect.origin.y -= (size.y - rect.size.height);
  rect.size.width = size.x;
  rect.size.height = size.y;
  [window setFrame:rect display:YES];
}

static void OSX_SetWindowFocus(Viewport *viewport) {
  OSX_Data *bd = OSX_GetBackendData();
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);
  [data->Window makeKeyAndOrderFront:bd->Window];
}

static bool OSX_GetWindowFocus(Viewport *viewport) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  return data->Window.isKeyWindow;
}

static bool OSX_GetWindowMinimized(Viewport *viewport) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  return data->Window.isMiniaturized;
}

static void OSX_SetWindowTitle(Viewport *viewport, const char *title) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  data->Window.title = [NSString stringWithUTF8String:title];
}

static void OSX_SetWindowAlpha(Viewport *viewport, float alpha) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);
  assert(alpha >= 0.0f && alpha <= 1.0f);

  data->Window.alphaValue = alpha;
}

static float OSX_GetWindowDpiScale(Viewport *viewport) {
  ViewportDataOSX *data = (ViewportDataOSX *)viewport->PlatformUserData;
  assert(data->Window != 0);

  return data->Window.backingScaleFactor;
}

static void OSX_UpdateMonitors() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Monitors.resize(0);

  for (NSScreen *screen in NSScreen.screens) {
    NSRect frame = screen.frame;
    NSRect visibleFrame = screen.visibleFrame;

    PlatformMonitor monitor;
    monitor.MainPos = Vec2(frame.origin.x, frame.origin.y);
    monitor.MainSize = Vec2(frame.size.width, frame.size.height);
    monitor.WorkPos = Vec2(visibleFrame.origin.x, visibleFrame.origin.y);
    monitor.WorkSize = Vec2(visibleFrame.size.width, visibleFrame.size.height);
    monitor.DpiScale = screen.backingScaleFactor;
    monitor.PlatformHandle = (__bridge_retained void *)screen;

    platform_io.Monitors.push_back(monitor);
  }
}

static void OSX_InitPlatformInterface() {
  OSX_Data *bd = OSX_GetBackendData();
  OSX_UpdateMonitors();

  // Register platform interface (will be coupled with a renderer interface)
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Platform_CreateWindow = OSX_CreateWindow;
  platform_io.Platform_DestroyWindow = OSX_DestroyWindow;
  platform_io.Platform_ShowWindow = OSX_ShowWindow;
  platform_io.Platform_SetWindowPos = OSX_SetWindowPos;
  platform_io.Platform_GetWindowPos = OSX_GetWindowPos;
  platform_io.Platform_SetWindowSize = OSX_SetWindowSize;
  platform_io.Platform_GetWindowSize = OSX_GetWindowSize;
  platform_io.Platform_SetWindowFocus = OSX_SetWindowFocus;
  platform_io.Platform_GetWindowFocus = OSX_GetWindowFocus;
  platform_io.Platform_GetWindowMinimized = OSX_GetWindowMinimized;
  platform_io.Platform_SetWindowTitle = OSX_SetWindowTitle;
  platform_io.Platform_SetWindowAlpha = OSX_SetWindowAlpha;
  platform_io.Platform_GetWindowDpiScale = OSX_GetWindowDpiScale; // FIXME-DPI

  // Register main window handle (which is owned by the main application, not by
  // us)
  Viewport *main_viewport = Gui::GetMainViewport();
  ViewportDataOSX *data = NEW(ViewportDataOSX)();
  data->Window = bd->Window;
  data->WindowOwned = false;
  main_viewport->PlatformUserData = data;
  main_viewport->PlatformHandle = (__bridge void *)bd->Window;

  [NSNotificationCenter.defaultCenter
      addObserver:bd->Observer
         selector:@selector(displaysDidChange:)
             name:NSApplicationDidChangeScreenParametersNotification
           object:nil];
}

static void OSX_ShutdownPlatformInterface() {
  OSX_Data *bd = OSX_GetBackendData();
  [NSNotificationCenter.defaultCenter
      removeObserver:bd->Observer
                name:NSApplicationDidChangeScreenParametersNotification
              object:nil];
  bd->Observer = nullptr;
  bd->Window = nullptr;
  if (bd->Monitor != nullptr) {
    [NSEvent removeMonitor:bd->Monitor];
    bd->Monitor = nullptr;
  }

  Viewport *main_viewport = Gui::GetMainViewport();
  ViewportDataOSX *data = (ViewportDataOSX *)main_viewport->PlatformUserData;
  DELETE(data);
  main_viewport->PlatformUserData = nullptr;
  Gui::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
