#pragma once

#include "text/shaper.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"
#include "ui/widgets/text_tone.hxx"

#include <string>

namespace loader::ui
{

class Label : public Widget
{
public:
    Label(UiContext& context,
          std::u32string text,
          const text::TextStyle& style,
          TextTone tone = TextTone::Primary,
          HorizontalAlign align = HorizontalAlign::Start);

    Label(UiContext& context,
          Str id,
          const text::TextStyle& style,
          TextTone tone = TextTone::Primary,
          HorizontalAlign align = HorizontalAlign::Start);

    void setText(std::u32string text);
    void setTone(TextTone tone);
    const std::u32string& text() const { return text_; }

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;

private:
    void reshape();

    std::u32string text_;
    text::TextStyle style_;
    TextTone tone_;
    HorizontalAlign align_;
    text::ShapedText shaped_;
    TextBinding binding_;
};

}
