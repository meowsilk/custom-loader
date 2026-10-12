#include "ui/widgets/inline_message.hxx"

#include "ui/icons.hxx"

namespace loader::ui
{

InlineMessage::InlineMessage(UiContext& context) : Widget(context)
{
    setVisible(false);
    icon_ = text::shapeText(
        context_.fonts(), context_.theme().type.iconSmall, std::u32string(1, icons::kErrorWarning));
    languageSubscription_ = context_.localization().subscribe(
        [this]
        {
            if (!id_)
                return;
            message_ = text::shapeText(
                context_.fonts(), context_.theme().type.caption, context_.localization().text(*id_));
            invalidate();
        });
}

void InlineMessage::show(const std::u32string& message, TextTone tone)
{
    id_.reset();
    present(message, tone);
}

void InlineMessage::show(Str id, TextTone tone)
{
    id_ = id;
    present(context_.localization().text(id), tone);
}

void InlineMessage::present(std::u32string_view message, TextTone tone)
{
    message_ = text::shapeText(context_.fonts(), context_.theme().type.caption, message);
    tone_ = tone;
    icon_ = text::shapeText(
        context_.fonts(),
        context_.theme().type.iconSmall,
        std::u32string(1, tone == TextTone::Success ? icons::kCheckCircle : icons::kErrorWarning));
    setVisible(true);
    reveal_.animateTo(1.0F, context_.theme().motion.base);
    invalidate();
}

void InlineMessage::clear()
{
    reveal_.animateTo(0.0F, context_.theme().motion.fast);
    invalidate();
}

core::Size InlineMessage::measureContent()
{
    const float height = context_.theme().type.caption.lineHeight;
    return {message_.width + icon_.width + context_.theme().space.xs, height * reveal_.value()};
}

bool InlineMessage::updateSelf(float deltaSeconds)
{
    const bool animating = reveal_.update(deltaSeconds);
    if (!animating && reveal_.value() <= 0.0F && visible())
        setVisible(false);
    if (animating)
        invalidate();
    return animating;
}

void InlineMessage::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const float lineHeight = theme.type.caption.lineHeight;
    const core::Color color = toneColor(theme, tone_).scaledAlpha(reveal_.value());

    context.useFront();

    text::ShapedText icon = icon_;
    icon.lineHeight = lineHeight;
    context.draw().addText(icon, {box.x, box.y}, color);

    text::ShapedText message = message_;
    message.lineHeight = lineHeight;
    context.draw().addTextWithShadow(message, {box.x + icon_.width + theme.space.xs, box.y}, color);
}

}
