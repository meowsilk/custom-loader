#include "ui/widgets/checkbox.hxx"

#include "ui/icons.hxx"

namespace loader::ui
{

Checkbox::Checkbox(UiContext& context, const std::u32string& label) : Widget(context)
{
    setFocusable(true);
    const theme::Theme& theme = context_.theme();
    label_ = text::shapeText(context_.fonts(), theme.type.body, label);
    tick_ = text::shapeText(context_.fonts(), theme.type.iconSmall, std::u32string(1, icons::kCheck));
}

Checkbox::Checkbox(UiContext& context, Str id)
    : Checkbox(context, std::u32string(context.localization().text(id)))
{
    binding_ = TextBinding(
        context.localization(), id, [this](std::u32string_view text) { setLabel(std::u32string(text)); });
}

void Checkbox::setLabel(const std::u32string& label)
{
    label_ = text::shapeText(context_.fonts(), context_.theme().type.body, label);
    invalidate();
}

void Checkbox::setChecked(bool checked)
{
    if (checked_ == checked)
        return;
    checked_ = checked;
    check_.animateTo(checked ? 1.0F : 0.0F, context_.theme().motion.fast);
    invalidate();
}

void Checkbox::toggle()
{
    setChecked(!checked_);
    if (onChanged_)
        onChanged_(checked_);
}

void Checkbox::onClick()
{
    toggle();
}

bool Checkbox::onKeyDown(const core::input::KeyEvent& event)
{
    if (event.key == core::input::Key::Space)
    {
        toggle();
        return true;
    }
    return false;
}

core::Size Checkbox::measureContent()
{
    const theme::Theme& theme = context_.theme();
    return {theme.metrics.checkbox + theme.space.s + label_.width, theme.type.body.lineHeight};
}

void Checkbox::onStateChanged()
{
    const float duration = context_.theme().motion.fast;
    hover_.animateTo(hovered() ? 1.0F : 0.0F, duration);
    focus_.animateTo(focusVisible() ? 1.0F : 0.0F, duration);
}

bool Checkbox::updateSelf(float deltaSeconds)
{
    bool animating = hover_.update(deltaSeconds);
    animating = focus_.update(deltaSeconds) || animating;
    animating = check_.update(deltaSeconds) || animating;
    return animating;
}

void Checkbox::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const float size = theme.metrics.checkbox;
    const core::Rect square{box.x, box.y + (box.height - size) * 0.5F, size, size};

    const core::Color tint = core::Color::lerp(
        theme.colors.glassTint.withAlpha(0.04F), theme.colors.glassTintHover, hover_.value());
    const core::Color rim =
        core::Color::lerp(theme.colors.glassRim, theme.colors.glassRimFocus, focus_.value());
    context.useGlass();
    context.draw().addGlass({square,
                             render::Corners::uniform(5.0F),
                             tint,
                             rim,
                             4.0F,
                             2.0F,
                             theme.glass.dispersion,
                             theme.glass.specular,
                             theme.glass.frost});

    context.useFront();
    const float checked = check_.value();
    if (checked > 0.01F)
    {
        context.draw().addRect(
            {square, render::Corners::uniform(5.0F), theme.colors.textPrimary.scaledAlpha(0.9F * checked)});

        text::ShapedText tick = tick_;
        tick.lineHeight = square.height;
        context.draw().addText(tick,
                               {square.x + (square.width - tick_.width) * 0.5F, square.y},
                               core::Color::fromHex(0x101113).scaledAlpha(checked));
    }

    text::ShapedText label = label_;
    label.lineHeight = box.height;
    context.draw().addTextWithShadow(
        label, {box.x + size + theme.space.s, box.y}, theme.colors.textSecondary);
}

}
