#include "ui/widgets/icon_button.hxx"

#include <algorithm>

namespace loader::ui
{

IconButton::IconButton(UiContext& context, char32_t glyph, Tone tone) : Widget(context), tone_(tone)
{
    setFocusable(true);
    shaped_ = text::shapeText(context_.fonts(), context_.theme().type.icon, std::u32string(1, glyph));
}

void IconButton::onClick()
{
    if (onClick_)
        onClick_();
}

bool IconButton::onKeyDown(const core::input::KeyEvent& event)
{
    if (event.key == core::input::Key::Enter || event.key == core::input::Key::Space)
    {
        onClick();
        return true;
    }
    return false;
}

core::Size IconButton::measureContent()
{
    const float size = context_.theme().metrics.windowButton;
    return {size, size};
}

void IconButton::onStateChanged()
{
    const float duration = context_.theme().motion.fast;
    hover_.animateTo(hovered() ? 1.0F : 0.0F, duration);
    press_.animateTo(pressed() ? 1.0F : 0.0F, duration);
    focus_.animateTo(focusVisible() ? 1.0F : 0.0F, duration);
}

bool IconButton::updateSelf(float deltaSeconds)
{
    bool animating = hover_.update(deltaSeconds);
    animating = press_.update(deltaSeconds) || animating;
    animating = focus_.update(deltaSeconds) || animating;
    return animating;
}

void IconButton::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const bool danger = tone_ == Tone::Danger;

    const float presence = std::max(hover_.value(), focus_.value());
    if (presence > 0.01F)
    {
        const core::Color hoverTint =
            danger ? theme.colors.danger.withAlpha(0.32F) : theme.colors.glassTintHover;
        const core::Color pressTint =
            danger ? theme.colors.danger.withAlpha(0.20F) : theme.colors.glassTintPressed;
        const core::Color tint = core::Color::lerp(hoverTint, pressTint, press_.value());
        const core::Color rim =
            core::Color::lerp(theme.colors.glassRim, theme.colors.glassRimFocus, focus_.value());

        context.draw().pushTransform({}, presence);
        context.useGlass();
        context.draw().addGlass({box,
                                 render::Corners::uniform(theme.radii.small + 2.0F),
                                 tint,
                                 rim,
                                 theme.glass.bevel,
                                 theme.glass.refraction,
                                 theme.glass.dispersion,
                                 theme.glass.specular,
                                 theme.glass.frost});
        context.draw().popTransform();
    }

    text::ShapedText line = shaped_;
    line.lineHeight = box.height;
    context.useFront();
    context.draw().addText(
        line,
        {box.x + (box.width - shaped_.width) * 0.5F, box.y},
        core::Color::lerp(theme.colors.textSecondary, theme.colors.textPrimary, hover_.value()));
}

}
