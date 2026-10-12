#include "ui/widgets/info_row.hxx"

#include <algorithm>

namespace loader::ui
{
namespace
{

constexpr float kTileSize = 40.0F;
constexpr float kTileRadius = 12.0F;
constexpr float kGap = 14.0F;
constexpr float kRowHeight = 44.0F;
constexpr float kLabelLine = 16.0F;
constexpr float kValueLine = 22.0F;

}

InfoRow::InfoRow(UiContext& context, char32_t icon, Str label, Str value) : Widget(context)
{
    style().height = kRowHeight;
    text::TextStyle iconStyle = context_.theme().type.icon;
    iconStyle.size = 19.0F;
    iconStyle.lineHeight = kTileSize;
    icon_ = text::shapeText(context_.fonts(), iconStyle, std::u32string(1, icon));

    reshapeLabel(context_.localization().text(label));
    labelBinding_ = TextBinding(context_.localization(),
                                label,
                                [this](std::u32string_view text)
                                {
                                    reshapeLabel(text);
                                    invalidate();
                                });
    reshapeValue(context_.localization().text(value));
    valueBinding_ = TextBinding(context_.localization(),
                                value,
                                [this](std::u32string_view text)
                                {
                                    reshapeValue(text);
                                    invalidate();
                                });
}

InfoRow::InfoRow(UiContext& context, char32_t icon, Str label, const std::u32string& value)
    : InfoRow(context, icon, label, Str::DetectionNever)
{
    valueBinding_ = TextBinding();
    reshapeValue(value);
}

void InfoRow::setValue(Str value)
{
    reshapeValue(context_.localization().text(value));
    valueBinding_ = TextBinding(context_.localization(),
                                value,
                                [this](std::u32string_view text)
                                {
                                    reshapeValue(text);
                                    invalidate();
                                });
    invalidate();
}

void InfoRow::reshapeLabel(std::u32string_view text)
{
    text::TextStyle style = context_.theme().type.caption;
    style.lineHeight = kLabelLine;
    label_ = text::shapeText(context_.fonts(), style, text);
}

void InfoRow::reshapeValue(std::u32string_view text)
{
    valueText_ = std::u32string(text);
    text::TextStyle style = context_.theme().type.body;
    style.weight = 600.0F;
    style.lineHeight = kValueLine;
    value_ = text::shapeText(context_.fonts(), style, text);
}

core::Size InfoRow::measureContent()
{
    return {std::max(label_.width, value_.width) + kGap + kTileSize, kRowHeight};
}

void InfoRow::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();

    const core::Rect tile{
        box.right() - kTileSize, box.y + (box.height - kTileSize) * 0.5F, kTileSize, kTileSize};
    context.useFront();
    context.draw().addRect({tile,
                            render::Corners::uniform(kTileRadius),
                            core::Color::fromHex(0xFFFFFF, 0.11F),
                            core::Color::fromHex(0xFFFFFF, 0.24F),
                            1.0F});
    text::ShapedText glyph = icon_;
    glyph.lineHeight = kTileSize;
    context.draw().addText(
        glyph, {tile.x + (kTileSize - icon_.width) * 0.5F, tile.y}, theme.colors.textPrimary);

    const float textRight = tile.x - kGap;
    const float top = box.y + (box.height - kLabelLine - kValueLine) * 0.5F;

    text::ShapedText label = label_;
    label.lineHeight = kLabelLine;
    context.draw().addTextWithShadow(label, {textRight - label_.width, top}, theme.colors.textSecondary);

    text::ShapedText value = value_;
    value.lineHeight = kValueLine;
    context.draw().addTextWithShadow(
        value, {textRight - value_.width, top + kLabelLine}, theme.colors.textPrimary);
}

}
