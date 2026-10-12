#include "ui/widgets/text_field.hxx"

#include "text/caret_metrics.hxx"
#include "ui/icons.hxx"

#include <algorithm>
#include <cmath>

namespace loader::ui
{
namespace
{

constexpr float kBlinkSeconds = 0.53F;
constexpr float kTileInset = 8.0F;
constexpr float kTextGap = 12.0F;
constexpr float kCaretWidth = 1.5F;
constexpr float kCaretHeight = 20.0F;
constexpr float kDotDiameter = 6.0F;
constexpr float kDotAdvance = 10.0F;
constexpr float kDotBlurSigma = 1.7F;
constexpr std::size_t kSavedSecretDots = 12;
constexpr float kSecretTrailingSpace = 40.0F;
constexpr float kPlainTrailingSpace = 14.0F;

std::unique_ptr<text_edit::TextStorage> makeStorage(TextField::Kind kind)
{
    if (kind == TextField::Kind::Secret)
        return std::make_unique<text_edit::SecureTextStorage>();
    return std::make_unique<text_edit::PlainTextStorage>();
}

text_edit::EditorOptions makeOptions(TextField::Kind kind)
{
    text_edit::EditorOptions options;
    if (kind == TextField::Kind::Secret)
    {
        options.allowClipboardWrite = false;
        options.keepUndoHistory = false;
        options.maxLength = 128;
    }
    return options;
}

}

TextField::TextField(UiContext& context, Kind kind, char32_t leadingIcon, std::u32string placeholder)
    : Widget(context), kind_(kind), leadingIcon_(leadingIcon), placeholder_(std::move(placeholder)),
      editor_(makeStorage(kind), makeOptions(kind))
{
    setFocusable(true);
    iconShape_ =
        text::shapeText(context_.fonts(), context_.theme().type.icon, std::u32string(1, leadingIcon_));
    placeholderShape_ = text::shapeText(context_.fonts(), textStyle(), placeholder_);
    if (secret())
        eyeShape_ = text::shapeText(
            context_.fonts(), context_.theme().type.iconSmall, std::u32string(1, icons::kEye));
}

const text::TextStyle& TextField::textStyle() const
{
    return context_.theme().type.body;
}

void TextField::dropSavedSecret()
{
    savedSecret_ = false;
    savedSource_ = nullptr;
}

bool TextField::canReveal() const
{
    return secret() && (!savedSecret_ || savedSource_ != nullptr);
}

std::u32string_view TextField::visibleSource() const
{
    return savedSecret_ && savedSource_ != nullptr ? savedSource_->view() : editor_.text();
}

void TextField::setText(std::u32string_view text)
{
    editor_.setText(text);
    dropSavedSecret();
    scrollX_ = 0.0F;
    invalidate();
}

void TextField::clear()
{
    editor_.clear();
    dropSavedSecret();
    scrollX_ = 0.0F;
    invalidate();
}

void TextField::setSavedSecret(bool present, const core::SecretString* source)
{
    savedSecret_ = present && secret();
    savedSource_ = savedSecret_ ? source : nullptr;
    if (savedSecret_)
        editor_.clear();
    invalidate();
}

void TextField::setPlaceholder(std::u32string placeholder)
{
    placeholder_ = std::move(placeholder);
    placeholderShape_ = text::shapeText(context_.fonts(), textStyle(), placeholder_);
    invalidate();
}

void TextField::bindPlaceholder(Str id)
{
    setPlaceholder(std::u32string(context_.localization().text(id)));
    placeholderBinding_ =
        TextBinding(context_.localization(),
                    id,
                    [this](std::u32string_view text) { setPlaceholder(std::u32string(text)); });
}

void TextField::setError(bool error)
{
    error_.animateTo(error ? 1.0F : 0.0F, context_.theme().motion.base);
    invalidate();
}

void TextField::setReadOnly(bool readOnly)
{
    readOnly_ = readOnly;
    invalidate();
}

core::Size TextField::measureContent()
{
    return {0.0F, context_.theme().metrics.controlHeight};
}

TextField::Layout TextField::computeLayout() const
{
    const theme::Theme& theme = context_.theme();
    const core::Rect box = bounds();
    const float tileSize = theme.metrics.iconTile;

    Layout layout;
    layout.tile = {box.x + kTileInset, box.y + (box.height - tileSize) * 0.5F, tileSize, tileSize};
    const float left = layout.tile.right() + kTextGap;
    const float right = box.right() - (secret() ? kSecretTrailingSpace : kPlainTrailingSpace);
    layout.textArea = {left, box.y, std::max(0.0F, right - left), box.height};
    return layout;
}

text::ShapedText TextField::shapeVisibleText() const
{
    text::TextStyle style = textStyle();
    style.lineHeight = context_.theme().metrics.controlHeight;
    return text::shapeText(context_.fonts(), style, visibleSource());
}

float TextField::hiddenAdvance() const
{
    return kDotAdvance;
}

std::size_t TextField::hiddenCount() const
{
    if (!savedSecret_)
        return editor_.length();
    return savedSource_ != nullptr ? savedSource_->view().size() : kSavedSecretDots;
}

std::vector<float> TextField::displayOffsets() const
{
    const bool hidden = secret() && (reveal_.value() < 0.5F || !canReveal());
    if (hidden)
    {
        std::vector<float> offsets(hiddenCount() + 1);
        for (std::size_t i = 0; i < offsets.size(); ++i)
            offsets[i] = static_cast<float>(i) * hiddenAdvance();
        return offsets;
    }
    return text::caretOffsets(shapeVisibleText(), visibleSource().size());
}

void TextField::ensureCaretVisible(const std::vector<float>& offsets, float areaWidth)
{
    const std::size_t index = std::min(editor_.caret(), offsets.size() - 1);
    const float caretX = offsets[index];

    if (caretX - scrollX_ > areaWidth - 2.0F)
        scrollX_ = caretX - areaWidth + 2.0F;
    if (caretX - scrollX_ < 0.0F)
        scrollX_ = caretX;
    if (offsets.back() <= areaWidth)
        scrollX_ = 0.0F;
    scrollX_ = std::max(0.0F, scrollX_);
}

void TextField::restartBlink()
{
    blinkTimer_ = 0.0F;
    caretVisible_ = true;
    invalidate();
}

void TextField::afterEdit(unsigned previousRevision)
{
    if (editor_.revision() == previousRevision)
        return;
    restartBlink();
    if (onChanged_)
        onChanged_();
}

core::Rect TextField::caretRect() const
{
    const Layout layout = computeLayout();
    const std::vector<float> offsets = displayOffsets();
    const float x = layout.textArea.x - scrollX_ + offsets[std::min(editor_.caret(), offsets.size() - 1)];
    const core::Rect box = bounds();
    return {x, box.y + (box.height - kCaretHeight) * 0.5F, kCaretWidth, kCaretHeight};
}

bool TextField::onPointerDown(core::Vec2 position, core::input::PointerButton button)
{
    if (button != core::input::PointerButton::Left || savedSecret_)
        return true;

    const Layout layout = computeLayout();
    const std::vector<float> offsets = displayOffsets();
    editor_.moveCaret(text::caretIndexAt(offsets, position.x - layout.textArea.x + scrollX_), false);
    dragging_ = true;
    restartBlink();
    return true;
}

void TextField::onPointerMove(core::Vec2 position)
{
    if (!dragging_ || savedSecret_)
        return;

    const Layout layout = computeLayout();
    const std::vector<float> offsets = displayOffsets();
    editor_.moveCaret(text::caretIndexAt(offsets, position.x - layout.textArea.x + scrollX_), true);
    restartBlink();
}

void TextField::onPointerUp(core::Vec2, core::input::PointerButton)
{
    dragging_ = false;
}

bool TextField::onKeyDown(const core::input::KeyEvent& event)
{
    using core::input::Key;

    if (event.key == Key::Enter)
    {
        if (!onSubmit_)
            return false;
        onSubmit_();
        return true;
    }

    if (readOnly_)
        return false;

    if (savedSecret_ && (event.key == Key::Backspace || event.key == Key::Delete ||
                         (event.key == Key::V && event.modifiers.control)))
    {
        dropSavedSecret();
        invalidate();
        if (event.key != Key::V)
        {
            if (onChanged_)
                onChanged_();
            return true;
        }
    }

    const unsigned before = editor_.revision();
    const bool handled = editor_.handleKey(event, context_.clipboard());
    afterEdit(before);
    if (handled)
        restartBlink();
    return handled;
}

void TextField::onChar(char32_t codePoint)
{
    if (readOnly_)
        return;

    dropSavedSecret();
    const unsigned before = editor_.revision();
    editor_.insert(std::u32string(1, codePoint));
    afterEdit(before);
}

core::Rect TextField::eyeZone() const
{
    const core::Rect box = bounds();
    const float centerX = box.right() - kTileInset - 4.0F - eyeShape_.width * 0.5F;
    return {centerX - 15.0F, box.y + (box.height - 28.0F) * 0.5F, 30.0F, 28.0F};
}

void TextField::onHoverMove(core::Vec2 position)
{
    const bool hot = secret() && eyeZone().contains(position);
    if (hot == eyeHot_)
        return;
    eyeHot_ = hot;
    eyeHover_.animateTo(hot ? 1.0F : 0.0F, context_.theme().motion.fast);
    invalidate();
}

void TextField::onStateChanged()
{
    const float duration = context_.theme().motion.fast;
    if (!hovered() && eyeHot_)
    {
        eyeHot_ = false;
        eyeHover_.animateTo(0.0F, duration);
    }
    focus_.animateTo(focused() ? 1.0F : 0.0F, duration);
    hover_.animateTo(hovered() ? 1.0F : 0.0F, duration);
    reveal_.animateTo(canReveal() && pressed() ? 1.0F : 0.0F, duration);
    if (!focused())
        dragging_ = false;
    else
        restartBlink();

    const bool lostFocus = wasFocused_ && !focused();
    wasFocused_ = focused();
    if (lostFocus && onBlur_)
        onBlur_();
}

bool TextField::updateSelf(float deltaSeconds)
{
    bool animating = focus_.update(deltaSeconds);
    animating = hover_.update(deltaSeconds) || animating;
    animating = error_.update(deltaSeconds) || animating;
    animating = reveal_.update(deltaSeconds) || animating;
    animating = eyeHover_.update(deltaSeconds) || animating;

    if (focused() && !readOnly_)
    {
        blinkTimer_ += deltaSeconds;
        if (blinkTimer_ >= kBlinkSeconds)
        {
            blinkTimer_ = std::fmod(blinkTimer_, kBlinkSeconds);
            caretVisible_ = !caretVisible_;
            invalidate();
        }
    }
    return animating;
}

std::optional<float> TextField::idleWakeSeconds() const
{
    if (!focused() || readOnly_)
        return std::nullopt;
    return std::max(0.01F, kBlinkSeconds - blinkTimer_);
}

void TextField::paintHiddenDots(PaintContext& context, const core::Rect& area, float opacity)
{
    if (opacity <= 0.01F)
        return;

    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const std::size_t count = hiddenCount();
    const core::Color color = theme.colors.textPrimary.scaledAlpha(0.92F * opacity);

    for (std::size_t i = 0; i < count; ++i)
    {
        const float centerX = area.x - scrollX_ + (static_cast<float>(i) + 0.5F) * hiddenAdvance();
        const core::Rect dot{centerX - kDotDiameter * 0.5F,
                             box.y + (box.height - kDotDiameter) * 0.5F,
                             kDotDiameter,
                             kDotDiameter};
        context.draw().addShadow({dot, kDotDiameter * 0.5F, {0.0F, 0.0F}, kDotBlurSigma, color});
    }
}

void TextField::paintVisibleText(PaintContext& context, const core::Rect& area, float opacity)
{
    if (opacity <= 0.01F || visibleSource().empty())
        return;

    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const text::ShapedText shaped = shapeVisibleText();
    const std::vector<float> offsets = text::caretOffsets(shaped, visibleSource().size());

    if (editor_.hasSelection())
    {
        const auto [begin, end] = editor_.selection();
        const float left = area.x - scrollX_ + offsets[begin];
        const float right = area.x - scrollX_ + offsets[end];
        context.draw().addRect(
            {{left, box.y + (box.height - kCaretHeight) * 0.5F, right - left, kCaretHeight},
             render::Corners::uniform(3.0F),
             core::Color::fromHex(0xFFFFFF, 0.22F * opacity)});
    }

    context.draw().addText(shaped, {area.x - scrollX_, box.y}, theme.colors.textPrimary.scaledAlpha(opacity));
}

void TextField::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const Layout layout = computeLayout();

    const std::vector<float> offsets = displayOffsets();
    ensureCaretVisible(offsets, layout.textArea.width);

    const core::Color tint =
        core::Color::lerp(theme.colors.fieldTint, theme.colors.fieldTintHover, hover_.value());
    core::Color rim = core::Color::lerp(theme.colors.glassRim, theme.colors.glassRimFocus, focus_.value());
    rim = core::Color::lerp(rim, theme.colors.glassRimFocus, hover_.value() * 0.4F * (1.0F - focus_.value()));
    rim = core::Color::lerp(rim, theme.colors.danger.withAlpha(0.95F), error_.value());

    context.useGlass();
    context.draw().addGlass({box,
                             render::Corners::uniform(theme.radii.control),
                             tint,
                             rim,
                             theme.glass.bevel,
                             theme.glass.refraction,
                             theme.glass.dispersion,
                             theme.glass.specular,
                             theme.glass.frost});

    context.useFront();
    context.draw().addRect({layout.tile,
                            render::Corners::uniform(theme.radii.small + 2.0F),
                            core::Color::fromHex(0xFFFFFF, 0.07F + 0.05F * hover_.value())});
    text::ShapedText icon = iconShape_;
    icon.lineHeight = layout.tile.height;
    context.draw().addText(
        icon,
        {layout.tile.x + (layout.tile.width - iconShape_.width) * 0.5F, layout.tile.y},
        core::Color::lerp(theme.colors.textSecondary, theme.colors.danger, error_.value()));

    if (secret())
    {
        text::ShapedText line = eyeShape_;
        line.lineHeight = box.height;
        const float emphasis = std::max(eyeHover_.value(), reveal_.value());
        const core::Color eyeColor =
            core::Color::lerp(theme.colors.textTertiary, theme.colors.textPrimary, emphasis);
        context.draw().addRect({eyeZone(),
                                render::Corners::uniform(theme.radii.small + 2.0F),
                                core::Color::fromHex(0xFFFFFF, 0.10F * emphasis)});
        context.draw().addText(line, {box.right() - kTileInset - 4.0F - eyeShape_.width, box.y}, eyeColor);
    }

    context.draw().pushClip(layout.textArea, 0.0F);

    if (editor_.empty() && !savedSecret_)
    {
        text::ShapedText placeholder = placeholderShape_;
        placeholder.lineHeight = box.height;
        context.draw().addText(placeholder, {layout.textArea.x, box.y}, theme.colors.textTertiary);
    }
    else if (secret())
    {
        paintHiddenDots(context, layout.textArea, canReveal() ? 1.0F - reveal_.value() : 1.0F);
        paintVisibleText(context, layout.textArea, reveal_.value());
    }
    else
    {
        paintVisibleText(context, layout.textArea, 1.0F);
    }

    if (focused() && caretVisible_ && !readOnly_ && !savedSecret_)
    {
        const float x = layout.textArea.x - scrollX_ + offsets[std::min(editor_.caret(), offsets.size() - 1)];
        context.draw().addRect({{x, box.y + (box.height - kCaretHeight) * 0.5F, kCaretWidth, kCaretHeight},
                                {},
                                theme.colors.textPrimary});
    }

    context.draw().popClip();
}

}
