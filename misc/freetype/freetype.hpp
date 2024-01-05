// dear imgui: FreeType font builder (used as a replacement for the truetype
// builder) (headers)

#pragma once
#include "../../gui.hpp" // API
#ifndef DISABLE

// Forward declarations
struct FontAtlas;
struct FontBuilderIO;

// Hinting greatly impacts visuals (and glyph sizes).
// - By default, hinting is enabled and the font's native hinter is preferred
// over the auto-hinter.
// - When disabled, FreeType generates blurrier glyphs, more or less matches the
// truetype.h
// - The Default hinting mode usually looks good, but may distort glyphs in an
// unusual way.
// - The Light hinting mode generates fuzzier glyphs but better matches
// Microsoft's rasterizer. You can set those flags globaly in
// FontAtlas::FontBuilderFlags You can set those flags on a per font basis in
// FontConfig::FontBuilderFlags
enum FreeTypeBuilderFlags {
  FreeTypeBuilderFlags_NoHinting =
      1 << 0, // Disable hinting. This generally generates 'blurrier' bitmap
              // glyphs when the glyph are rendered in any of the anti-aliased
              // modes.
  FreeTypeBuilderFlags_NoAutoHint = 1 << 1, // Disable auto-hinter.
  FreeTypeBuilderFlags_ForceAutoHint =
      1 << 2, // Indicates that the auto-hinter is preferred over the font's
              // native hinter.
  FreeTypeBuilderFlags_LightHinting =
      1 << 3, // A lighter hinting algorithm for gray-level modes. Many
              // generated glyphs are fuzzier but better resemble their original
              // shape. This is achieved by snapping glyphs to the pixel grid
              // only vertically (Y-axis), as is done by Microsoft's ClearType
              // and Adobe's proprietary font renderer. This preserves
              // inter-glyph spacing in horizontal text.
  FreeTypeBuilderFlags_MonoHinting =
      1 << 4, // Strong hinting algorithm that should only be used for
              // monochrome output.
  FreeTypeBuilderFlags_Bold =
      1 << 5, // Styling: Should we artificially embolden the font?
  FreeTypeBuilderFlags_Oblique =
      1 << 6, // Styling: Should we slant the font, emulating italic style?
  FreeTypeBuilderFlags_Monochrome = 1
                                    << 7, // Disable anti-aliasing. Combine this
                                          // with MonoHinting for best results!
  FreeTypeBuilderFlags_LoadColor = 1
                                   << 8, // Enable FreeType color-layered glyphs
  FreeTypeBuilderFlags_Bitmap = 1 << 9   // Enable FreeType bitmap glyphs
};

namespace FreeType {
// This is automatically assigned when using '#define ENABLE_FREETYPE'.
// If you need to dynamically select between multiple builders:
// - you can manually assign this builder with 'atlas->FontBuilderIO =
// FreeType::GetBuilderForFreeType()'
// - prefer deep-copying this into your own FontBuilderIO instance if you use
// hot-reloading that messes up static data.
API const FontBuilderIO *GetBuilderForFreeType();

// Override allocators. By default FreeType will use ALLOC()/FREE()
// However, as FreeType does lots of allocations we provide a way for the user
// to redirect it to a separate memory heap if desired.
API void SetAllocatorFunctions(void *(*alloc_func)(size_t sz, void *user_data),
                               void (*free_func)(void *ptr, void *user_data),
                               void *user_data = nullptr);

// Obsolete names (will be removed soon)
#ifndef DISABLE_OBSOLETE_FUNCTIONS
// static inline bool BuildFontAtlas(FontAtlas* atlas, unsigned int flags = 0)
// { atlas->FontBuilderIO = GetBuilderForFreeType(); atlas->FontBuilderFlags =
// flags; return atlas->Build(); } // Prefer using '#define ENABLE_FREETYPE'
#endif
} // namespace FreeType

#endif // #ifndef DISABLE
