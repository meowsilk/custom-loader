#pragma once

#include "core/color.hxx"
#include "text/shaper.hxx"

namespace loader::ui::theme
{

struct Palette
{
    core::Color panelBorder;
    core::Color shadow;
    core::Color glassTint;
    core::Color glassTintHover;
    core::Color glassTintPressed;
    core::Color fieldTint;
    core::Color fieldTintHover;
    core::Color glassRim;
    core::Color glassRimFocus;
    core::Color primaryTint;
    core::Color primaryTintHover;
    core::Color primaryTintPressed;
    core::Color textPrimary;
    core::Color textSecondary;
    core::Color textTertiary;
    core::Color textDisabled;
    core::Color danger;
    core::Color success;
    core::Color heroTint;
};

struct TypeScale
{
    text::TextStyle headerTitle;
    text::TextStyle brand;
    text::TextStyle display;
    text::TextStyle heading;
    text::TextStyle cardTitle;
    text::TextStyle tag;
    text::TextStyle title;
    text::TextStyle body;
    text::TextStyle label;
    text::TextStyle button;
    text::TextStyle caption;
    text::TextStyle icon;
    text::TextStyle iconSmall;
};

struct Spacing
{
    float xs = 4.0F;
    float s = 8.0F;
    float m = 12.0F;
    float l = 16.0F;
    float xl = 24.0F;
};

struct Radii
{
    float small = 6.0F;
    float control = 12.0F;
    float inset = 12.0F;
    float panel = 20.0F;
};

struct Motion
{
    float fast = 0.12F;
    float base = 0.20F;
    float slow = 0.32F;
    float morph = 0.70F;
};

struct Glass
{
    float bevel = 8.0F;
    float refraction = 6.0F;
    float dispersion = 0.02F;
    float specular = 0.18F;
    float frost = 0.8F;
    float shadowBlur = 16.0F;
    float shadowOffset = 12.0F;
};

struct Backdrop
{
    float seed = 3.0F;
    float brightness = 0.6F;
    float fade = 0.62F;
    float patternScale = 210.0F;
};

struct Metrics
{
    float controlHeight = 44.0F;
    float headerHeight = 44.0F;
    float headerInset = 8.0F;
    float windowButton = 32.0F;
    float iconTile = 28.0F;
    float checkbox = 18.0F;
};

struct Theme
{
    Palette colors;
    TypeScale type;
    Spacing space;
    Radii radii;
    Motion motion;
    Glass glass;
    Backdrop backdrop;
    Metrics metrics;
};

Theme makeDarkGlassTheme();

}
