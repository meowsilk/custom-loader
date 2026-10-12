#pragma once

#include "core/color.hxx"
#include "ui/theme/theme.hxx"

namespace loader::ui
{

enum class TextTone
{
    Primary,
    Secondary,
    Tertiary,
    Disabled,
    Danger,
    Success,
};

inline core::Color toneColor(const theme::Theme& theme, TextTone tone)
{
    switch (tone)
    {
    case TextTone::Primary:
        return theme.colors.textPrimary;
    case TextTone::Secondary:
        return theme.colors.textSecondary;
    case TextTone::Tertiary:
        return theme.colors.textTertiary;
    case TextTone::Disabled:
        return theme.colors.textDisabled;
    case TextTone::Danger:
        return theme.colors.danger;
    case TextTone::Success:
        return theme.colors.success;
    }
    return theme.colors.textPrimary;
}

enum class HorizontalAlign
{
    Start,
    Center,
    End,
};

}
