#include "ui/widgets/paragraph_label.hxx"

#include <string>

namespace loader::ui
{

ParagraphLabel::ParagraphLabel(
    UiContext& context, Str id, const text::TextStyle& style, TextTone tone, float maxWidth)
    : Widget(context), style_(style), tone_(tone), maxWidth_(maxWidth)
{
    rewrap(context_.localization().text(id));
    binding_ = TextBinding(context_.localization(),
                           id,
                           [this](std::u32string_view text)
                           {
                               rewrap(text);
                               invalidate();
                           });
}

void ParagraphLabel::rewrap(std::u32string_view text)
{
    lines_.clear();

    std::u32string current;
    std::size_t position = 0;
    while (position < text.size())
    {
        const std::size_t end = text.find(U' ', position);
        const std::u32string word(
            text.substr(position, end == std::u32string_view::npos ? end : end - position));
        position = end == std::u32string_view::npos ? text.size() : end + 1;
        if (word.empty())
            continue;

        const std::u32string candidate = current.empty() ? word : (current + U' ').append(word);
        if (!current.empty() && text::shapeText(context_.fonts(), style_, candidate).width > maxWidth_)
        {
            lines_.push_back(text::shapeText(context_.fonts(), style_, current));
            current = word;
        }
        else
        {
            current = candidate;
        }
    }
    if (!current.empty())
        lines_.push_back(text::shapeText(context_.fonts(), style_, current));
}

core::Size ParagraphLabel::measureContent()
{
    return {maxWidth_, style_.lineHeight * static_cast<float>(lines_.size())};
}

void ParagraphLabel::paintSelf(PaintContext& context)
{
    const core::Rect box = bounds();
    context.useFront();
    float y = box.y;
    for (const text::ShapedText& line : lines_)
    {
        text::ShapedText shaped = line;
        shaped.lineHeight = style_.lineHeight;
        context.draw().addTextWithShadow(shaped, {box.x, y}, toneColor(context.theme(), tone_));
        y += style_.lineHeight;
    }
}

}
