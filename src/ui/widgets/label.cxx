#include "ui/widgets/label.hxx"

namespace loader::ui
{

Label::Label(UiContext& context,
             std::u32string text,
             const text::TextStyle& style,
             TextTone tone,
             HorizontalAlign align)
    : Widget(context), text_(std::move(text)), style_(style), tone_(tone), align_(align)
{
    reshape();
}

Label::Label(UiContext& context, Str id, const text::TextStyle& style, TextTone tone, HorizontalAlign align)
    : Label(context, std::u32string(context.localization().text(id)), style, tone, align)
{
    binding_ = TextBinding(
        context.localization(), id, [this](std::u32string_view text) { setText(std::u32string(text)); });
}

void Label::reshape()
{
    shaped_ = text::shapeText(context_.fonts(), style_, text_);
}

void Label::setText(std::u32string text)
{
    if (text_ == text)
        return;
    text_ = std::move(text);
    reshape();
    invalidate();
}

void Label::setTone(TextTone tone)
{
    if (tone_ == tone)
        return;
    tone_ = tone;
    invalidate();
}

core::Size Label::measureContent()
{
    return {shaped_.width, style_.lineHeight};
}

void Label::paintSelf(PaintContext& context)
{
    const core::Rect box = bounds();
    float x = box.x;
    if (align_ == HorizontalAlign::Center)
        x += (box.width - shaped_.width) * 0.5F;
    else if (align_ == HorizontalAlign::End)
        x += box.width - shaped_.width;

    text::ShapedText line = shaped_;
    line.lineHeight = box.height;
    context.useFront();
    context.draw().addTextWithShadow(line, {x, box.y}, toneColor(context.theme(), tone_));
}

}
