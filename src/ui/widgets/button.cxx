#include "ui/widgets/button.hxx"

#include "ui/widgets/spinner.hxx"

#include <cmath>

namespace loader::ui
{

Button::Button(UiContext& context, std::u32string label, Variant variant)
    : Widget(context), label_(std::move(label)), variant_(variant)
{
    setFocusable(true);
    shaped_ = text::shapeText(context_.fonts(), context_.theme().type.button, label_);
}

Button::Button(UiContext& context, Str id, Variant variant)
    : Button(context, std::u32string(context.localization().text(id)), variant)
{
    binding_ = TextBinding(
        context.localization(), id, [this](std::u32string_view text) { setLabel(std::u32string(text)); });
}

void Button::setLabel(std::u32string label)
{
    label_ = std::move(label);
    shaped_ = text::shapeText(context_.fonts(), context_.theme().type.button, label_);
    invalidate();
}

void Button::setLoading(bool loading)
{
    if (loading_ == loading)
        return;
    loading_ = loading;
    busy_.animateTo(loading ? 1.0F : 0.0F, context_.theme().motion.base);
    invalidate();
}

void Button::onClick()
{
    if (!loading_ && onClick_)
        onClick_();
}

bool Button::onKeyDown(const core::input::KeyEvent& event)
{
    if (event.key == core::input::Key::Enter || event.key == core::input::Key::Space)
    {
        onClick();
        return true;
    }
    return false;
}

core::Size Button::measureContent()
{
    const theme::Theme& theme = context_.theme();
    return {shaped_.width + theme.space.xl * 2.0F, theme.metrics.controlHeight};
}

void Button::onStateChanged()
{
    const float duration = context_.theme().motion.fast;
    hover_.animateTo(hovered() ? 1.0F : 0.0F, duration);
    press_.animateTo(pressed() ? 1.0F : 0.0F, duration);
    focus_.animateTo(focusVisible() ? 1.0F : 0.0F, duration);
}

bool Button::updateSelf(float deltaSeconds)
{
    bool animating = hover_.update(deltaSeconds);
    animating = press_.update(deltaSeconds) || animating;
    animating = focus_.update(deltaSeconds) || animating;
    animating = busy_.update(deltaSeconds) || animating;

    if (loading_)
    {
        spinnerPhase_ += deltaSeconds * 1.1F;
        spinnerPhase_ -= std::floor(spinnerPhase_);
        animating = true;
    }
    return animating;
}

void Button::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const bool primary = variant_ == Variant::Primary;
    const core::Rect box = bounds();

    const core::Color base = primary ? theme.colors.primaryTint : theme.colors.glassTint;
    const core::Color hover = primary ? theme.colors.primaryTintHover : theme.colors.glassTintHover;
    const core::Color pressed = primary ? theme.colors.primaryTintPressed : theme.colors.glassTintPressed;

    core::Color tint = core::Color::lerp(base, hover, hover_.value());
    tint = core::Color::lerp(tint, pressed, press_.value());
    const core::Color rim =
        core::Color::lerp(theme.colors.glassRim, theme.colors.glassRimFocus, focus_.value());

    context.useGlass();
    context.draw().addGlass({box,
                             render::Corners::uniform(box.height * 0.5F),
                             tint,
                             rim,
                             theme.glass.bevel,
                             theme.glass.refraction,
                             theme.glass.dispersion,
                             theme.glass.specular,
                             theme.glass.frost});

    const core::Color textColor = enabled() ? theme.colors.textPrimary : theme.colors.textDisabled;
    const float labelOpacity = 1.0F - busy_.value();
    if (labelOpacity > 0.01F)
    {
        text::ShapedText line = shaped_;
        line.lineHeight = box.height;
        context.useFront();
        context.draw().addText(
            line, {box.x + (box.width - shaped_.width) * 0.5F, box.y}, textColor.scaledAlpha(labelOpacity));
    }
    if (busy_.value() > 0.01F)
        paintSpinner(context, box.center(), 18.0F, spinnerPhase_, textColor.scaledAlpha(busy_.value()));
}

}
