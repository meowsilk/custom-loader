#pragma once

#include "text/shaper.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"
#include "ui/widgets/text_tone.hxx"

#include <string_view>
#include <vector>

namespace loader::ui
{

class ParagraphLabel : public Widget
{
public:
    ParagraphLabel(UiContext& context, Str id, const text::TextStyle& style, TextTone tone, float maxWidth);

    std::size_t lineCount() const { return lines_.size(); }

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;

private:
    void rewrap(std::u32string_view text);

    text::TextStyle style_;
    TextTone tone_;
    float maxWidth_;
    std::vector<text::ShapedText> lines_;
    TextBinding binding_;
};

}
