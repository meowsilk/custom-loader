#include "ui/theme/theme.hxx"

namespace loader::ui::theme
{
namespace
{

using core::Color;
using text::FontFamily;
using text::TextStyle;

Color white(float alpha)
{
    return Color::fromHex(0xFFFFFF, alpha);
}

}

Theme makeDarkGlassTheme()
{
    Theme theme;

    theme.colors = {
        .panelBorder = white(0.10F),
        .shadow = Color::fromHex(0x000000, 0.55F),
        .glassTint = white(0.05F),
        .glassTintHover = white(0.09F),
        .glassTintPressed = white(0.03F),
        .fieldTint = Color::fromHex(0x000000, 0.42F),
        .fieldTintHover = Color::fromHex(0x000000, 0.32F),
        .glassRim = white(0.55F),
        .glassRimFocus = white(0.95F),
        .primaryTint = white(0.14F),
        .primaryTintHover = white(0.19F),
        .primaryTintPressed = white(0.10F),
        .textPrimary = Color::fromHex(0xF2F2F4),
        .textSecondary = Color::fromHex(0xDCDDE2),
        .textTertiary = Color::fromHex(0xB4B5BC),
        .textDisabled = Color::fromHex(0x4B4C52),
        .danger = Color::fromHex(0xFF6369),
        .success = Color::fromHex(0x4CC38A),
        .heroTint = Color::fromHex(0xFFFFFF),
    };

    theme.type = {
        .headerTitle = TextStyle{FontFamily::Inter, 14.0F, 600.0F, 44.0F, 0.0F},
        .brand = TextStyle{FontFamily::Inter, 15.0F, 700.0F, 22.0F, -0.1F},
        .display = TextStyle{FontFamily::Inter, 34.0F, 700.0F, 42.0F, -0.7F},
        .heading = TextStyle{FontFamily::Inter, 15.0F, 600.0F, 22.0F, 0.0F},
        .cardTitle = TextStyle{FontFamily::Inter, 19.0F, 700.0F, 24.0F, -0.3F},
        .tag = TextStyle{FontFamily::Inter, 10.5F, 600.0F, 14.0F, 0.6F},
        .title = TextStyle{FontFamily::Inter, 20.0F, 600.0F, 28.0F, -0.2F},
        .body = TextStyle{FontFamily::Inter, 14.0F, 400.0F, 20.0F, 0.0F},
        .label = TextStyle{FontFamily::Inter, 13.0F, 500.0F, 20.0F, 0.0F},
        .button = TextStyle{FontFamily::Inter, 14.0F, 500.0F, 20.0F, 0.0F},
        .caption = TextStyle{FontFamily::Inter, 12.0F, 400.0F, 16.0F, 0.0F},
        .icon = TextStyle{FontFamily::Icons, 16.0F, 400.0F, 24.0F, 0.0F},
        .iconSmall = TextStyle{FontFamily::Icons, 14.0F, 400.0F, 20.0F, 0.0F},
    };

    return theme;
}

}
