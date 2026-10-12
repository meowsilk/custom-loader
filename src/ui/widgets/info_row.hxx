#pragma once

#include "text/shaper.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"

#include <optional>
#include <string>

namespace loader::ui
{

class InfoRow : public Widget
{
public:
    InfoRow(UiContext& context, char32_t icon, Str label, Str value);
    InfoRow(UiContext& context, char32_t icon, Str label, const std::u32string& value);

    const std::u32string& valueText() const { return valueText_; }
    void setValue(Str value);

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;

private:
    void reshapeLabel(std::u32string_view text);
    void reshapeValue(std::u32string_view text);

    text::ShapedText icon_;
    text::ShapedText label_;
    text::ShapedText value_;
    std::u32string valueText_;
    TextBinding labelBinding_;
    TextBinding valueBinding_;
};

}
