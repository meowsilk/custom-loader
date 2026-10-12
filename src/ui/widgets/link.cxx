#include "ui/widgets/link.hxx"

namespace loader::ui
{

Link::Link(UiContext& context, const std::u32string& text) : Widget(context)
{
    setFocusable(true);
    shaped_ = text::shapeText(context_.fonts(), context_.theme().type.body, text);
}

Link::Link(UiContext& context, Str id) : Link(context, std::u32string(context.localization().text(id)))
{
    binding_ = TextBinding(
        context.localization(), id, [this](std::u32string_view text) { setText(std::u32string(text)); });
}

void Link::setText(const std::u32string& text)
{
    shaped_ = text::shapeText(context_.fonts(), context_.theme().type.body, text);
    invalidate();
}

void Link::onClick()
{
    if (onClick_)
        onClick_();
}

bool Link::onKeyDown(const core::input::KeyEvent& event)
{
    if (event.key == core::input::Key::Enter || event.key == core::input::Key::Space)
    {
        onClick();
        return true;
    }
    return false;
}

core::Size Link::measureContent()
{
    return {shaped_.width, shaped_.lineHeight};
}

void Link::onStateChanged()
{
    const float duration = context_.theme().motion.fast;
    hover_.animateTo(hovered() || pressed() ? 1.0F : 0.0F, duration);
    focus_.animateTo(focusVisible() ? 1.0F : 0.0F, duration);
}

bool Link::updateSelf(float deltaSeconds)
{
    const bool hoverAnimating = hover_.update(deltaSeconds);
    const bool focusAnimating = focus_.update(deltaSeconds);
    return hoverAnimating || focusAnimating;
}

void Link::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();

    text::ShapedText line = shaped_;
    line.lineHeight = box.height;
    const core::Color color =
        core::Color::lerp(theme.colors.textSecondary, theme.colors.textPrimary, hover_.value());

    context.useFront();
    context.draw().addTextWithShadow(line, {box.x, box.y}, color);

    const float underlineY = box.y + line.baselineOffset() + 2.0F;
    const float underline = std::max(hover_.value(), focus_.value());
    context.draw().addRect({{box.x, underlineY, shaped_.width, 1.0F}, {}, color.scaledAlpha(underline)});
}

}
