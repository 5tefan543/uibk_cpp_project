#pragma once

namespace view {

// Used as index in UI.fonts. Fonts defined here should be loaded in UI() accordingly!
enum FontType { Default };

inline const char *toString(const FontType font)
{
    switch (font) {
    case FontType::Default:
        return "Default";
    default:
        return "Unknown";
    }
}

} // namespace view