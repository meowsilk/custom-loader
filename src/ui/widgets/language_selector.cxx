#include "ui/widgets/language_selector.hxx"

#include "render/art_images.hxx"
#include "ui/icons.hxx"

#include <algorithm>

namespace loader::ui
{
namespace
{

constexpr float kMargin = 16.0F;
constexpr float kChipHeight = 38.0F;
constexpr float kChipPadding = 11.0F;
constexpr float kChipRadius = 12.0F;
constexpr float kFlagWidth = 24.0F;
constexpr float kFlagHeight = 18.0F;
constexpr float kRowFlagWidth = 28.0F;
constexpr float kRowFlagHeight = 21.0F;
constexpr float kFlagRadius = 4.0F;
constexpr float kGap = 9.0F;
constexpr float kChevronWidth = 16.0F;
constexpr float kPopupGap = 8.0F;
constexpr float kPopupPadding = 6.0F;
constexpr float kRowHeight = 36.0F;
constexpr float kRowRadius = 10.0F;
constexpr float kPopupRadius = 16.0F;
constexpr float kPopupMinWidth = 188.0F;
constexpr float kPopupRise = 10.0F;
constexpr float kOpenSeconds = 0.18F;
constexpr float kCloseSeconds = 0.12F;
constexpr float kPopupBlur = 12.0F;

constexpr core::Color kChipTint = core::Color::fromHex(0x000000, 0.34F);
constexpr core::Color kChipTintHover = core::Color::fromHex(0x000000, 0.2F);
constexpr core::Color kPopupTint = core::Color::fromHex(0x08090C, 0.66F);

static_assert(render::kFlagImageCount == kLanguageCount, "one flag per language");

render::FlagImage flagFor(std::size_t index)
{
    return static_cast<render::FlagImage>(index);
}

}

LanguageSelector::LanguageSelector(UiContext& context) : Widget(context)
{
    setFocusable(true);

    const theme::Theme& theme = context_.theme();
    for (std::size_t i = 0; i < kLanguageCount; ++i)
        names_[i] =
            text::shapeText(context_.fonts(), theme.type.label, std::u32string(languageName(kLanguages[i])));
    chevronUp_ =
        text::shapeText(context_.fonts(), theme.type.iconSmall, std::u32string(1, icons::kChevronUp));
    chevronDown_ =
        text::shapeText(context_.fonts(), theme.type.iconSmall, std::u32string(1, icons::kChevronDown));
    check_ = text::shapeText(context_.fonts(), theme.type.iconSmall, std::u32string(1, icons::kCheck));

    languageSubscription_ = context_.localization().subscribe([this] { invalidate(); });
}

std::size_t LanguageSelector::currentIndex() const
{
    return static_cast<std::size_t>(context_.localization().language());
}

float LanguageSelector::collapsedWidth() const
{
    return kChipPadding * 2.0F + kFlagWidth + kGap + kChevronWidth;
}

float LanguageSelector::expandedWidth() const
{
    float widest = 0.0F;
    for (const text::ShapedText& name : names_)
        widest = std::max(widest, name.width);
    return collapsedWidth() + widest + kGap;
}

float LanguageSelector::expansion() const
{
    return std::max({chipHover_.value(), openAnimation_.value(), focus_.value()});
}

core::Rect LanguageSelector::chipRect() const
{
    const core::Rect box = bounds();
    const float width = collapsedWidth() + (expandedWidth() - collapsedWidth()) * expansion();
    return {box.x + kMargin, box.bottom() - kMargin - kChipHeight, width, kChipHeight};
}

core::Rect LanguageSelector::popupRect() const
{
    const core::Rect chip = chipRect();
    const float width = std::max(kPopupMinWidth, expandedWidth());
    const float height = kPopupPadding * 2.0F + kRowHeight * static_cast<float>(kLanguageCount);
    return {chip.x, chip.y - kPopupGap - height, width, height};
}

core::Rect LanguageSelector::rowRect(std::size_t index) const
{
    const core::Rect popup = popupRect();
    return {popup.x + kPopupPadding,
            popup.y + kPopupPadding + kRowHeight * static_cast<float>(index),
            popup.width - kPopupPadding * 2.0F,
            kRowHeight};
}

std::optional<std::size_t> LanguageSelector::rowAt(core::Vec2 position) const
{
    if (!open_)
        return std::nullopt;
    for (std::size_t i = 0; i < kLanguageCount; ++i)
    {
        if (rowRect(i).contains(position))
            return i;
    }
    return std::nullopt;
}

Widget* LanguageSelector::hitTest(core::Vec2 point)
{
    if (!visible() || !bounds().contains(point))
        return nullptr;
    if (open_ || chipRect().contains(point))
        return this;
    return nullptr;
}

void LanguageSelector::setOpen(bool open)
{
    if (open_ == open)
        return;
    open_ = open;
    if (open_)
        highlighted_ = currentIndex();
    else
        hoveredRow_.reset();
    openAnimation_.animateTo(open_ ? 1.0F : 0.0F, open_ ? kOpenSeconds : kCloseSeconds);
    invalidate();
}

void LanguageSelector::choose(std::size_t index)
{
    context_.localization().setLanguage(kLanguages[index % kLanguageCount]);
}

void LanguageSelector::track(core::Vec2 position)
{
    const bool onChip = chipRect().contains(position);
    if (onChip != pointerOnChip_)
    {
        pointerOnChip_ = onChip;
        chipHover_.animateTo(onChip ? 1.0F : 0.0F, context_.theme().motion.fast);
    }

    const std::optional<std::size_t> row = rowAt(position);
    if (row != hoveredRow_)
    {
        hoveredRow_ = row;
        if (row)
            highlighted_ = *row;
        invalidate();
    }
}

void LanguageSelector::onHoverMove(core::Vec2 position)
{
    track(position);
}

void LanguageSelector::onPointerMove(core::Vec2 position)
{
    track(position);
}

bool LanguageSelector::onPointerDown(core::Vec2 position, core::input::PointerButton button)
{
    if (button != core::input::PointerButton::Left)
        return false;
    pressedChip_ = chipRect().contains(position);
    pressedRow_ = rowAt(position);
    return true;
}

void LanguageSelector::onPointerUp(core::Vec2 position, core::input::PointerButton button)
{
    if (button != core::input::PointerButton::Left)
        return;

    const bool onChip = chipRect().contains(position);
    const std::optional<std::size_t> row = rowAt(position);

    if (!open_)
    {
        if (onChip && pressedChip_)
            setOpen(true);
    }
    else if (row && row == pressedRow_)
    {
        choose(*row);
        setOpen(false);
    }
    else if ((onChip && pressedChip_) || !popupRect().contains(position))
    {
        setOpen(false);
    }

    pressedChip_ = false;
    pressedRow_.reset();
}

bool LanguageSelector::onKeyDown(const core::input::KeyEvent& event)
{
    using core::input::Key;

    if (!open_)
    {
        if (event.key == Key::Enter || event.key == Key::Space || event.key == Key::Up ||
            event.key == Key::Down)
        {
            setOpen(true);
            return true;
        }
        return false;
    }

    switch (event.key)
    {
    case Key::Up:
        highlighted_ = (highlighted_ + kLanguageCount - 1) % kLanguageCount;
        invalidate();
        return true;
    case Key::Down:
        highlighted_ = (highlighted_ + 1) % kLanguageCount;
        invalidate();
        return true;
    case Key::Enter:
    case Key::Space:
        choose(highlighted_);
        setOpen(false);
        return true;
    case Key::Escape:
        setOpen(false);
        return true;
    default:
        return false;
    }
}

core::input::Cursor LanguageSelector::cursorKind() const
{
    return pointerOnChip_ || hoveredRow_ ? core::input::Cursor::Hand : core::input::Cursor::Arrow;
}

void LanguageSelector::onStateChanged()
{
    if (!hovered() && pointerOnChip_)
    {
        pointerOnChip_ = false;
        chipHover_.animateTo(0.0F, context_.theme().motion.fast);
    }
    if (!focused())
        setOpen(false);
    focus_.animateTo(focusVisible() ? 1.0F : 0.0F, context_.theme().motion.fast);
}

bool LanguageSelector::updateSelf(float deltaSeconds)
{
    bool animating = openAnimation_.update(deltaSeconds);
    animating = chipHover_.update(deltaSeconds) || animating;
    animating = focus_.update(deltaSeconds) || animating;
    return animating;
}

void LanguageSelector::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect chip = chipRect();
    const float hover = std::max(chipHover_.value(), openAnimation_.value());
    const std::size_t current = currentIndex();

    const core::Color rim = core::Color::lerp(
        core::Color::lerp(theme.colors.glassRim.scaledAlpha(0.5F), theme.colors.glassRim, hover),
        theme.colors.glassRimFocus,
        focus_.value());
    context.useGlass();
    context.draw().addGlass({chip,
                             render::Corners::uniform(kChipRadius),
                             core::Color::lerp(kChipTint, kChipTintHover, hover),
                             rim,
                             theme.glass.bevel,
                             theme.glass.refraction,
                             theme.glass.dispersion,
                             theme.glass.specular,
                             theme.glass.frost});

    context.useFront();

    const core::Rect flag{
        chip.x + kChipPadding, chip.y + (chip.height - kFlagHeight) * 0.5F, kFlagWidth, kFlagHeight};
    context.draw().addFlag(
        {flag, render::Corners::uniform(kFlagRadius), flagFor(current), 1.0F + 0.08F * hover});

    const float reveal = std::clamp((expansion() - 0.35F) / 0.65F, 0.0F, 1.0F);
    if (reveal > 0.01F)
    {
        text::ShapedText name = names_[current];
        name.lineHeight = chip.height;
        context.draw().pushClip(chip, kChipRadius);
        context.draw().addText(name,
                               {flag.right() + kGap, chip.y},
                               core::Color::lerp(theme.colors.textSecondary, theme.colors.textPrimary, hover)
                                   .scaledAlpha(reveal));
        context.draw().popClip();
    }

    const text::ShapedText& chevron = open_ ? chevronDown_ : chevronUp_;
    text::ShapedText chevronLine = chevron;
    chevronLine.lineHeight = chip.height;
    context.draw().addText(chevronLine,
                           {chip.right() - kChipPadding - chevron.width, chip.y},
                           core::Color::lerp(theme.colors.textTertiary, theme.colors.textPrimary, hover));

    const float t = openAnimation_.value();
    if (t <= 0.01F)
        return;

    const core::Rect popup = popupRect();
    context.draw().pushTransform({0.0F, kPopupRise * (1.0F - t)}, t);
    context.useOverlayGlass();
    context.draw().addGlass({popup,
                             render::Corners::uniform(kPopupRadius),
                             kPopupTint,
                             theme.colors.glassRim,
                             theme.glass.bevel,
                             theme.glass.refraction,
                             theme.glass.dispersion,
                             theme.glass.specular,
                             theme.glass.frost,
                             kPopupBlur});
    context.useOverlayFront();

    for (std::size_t i = 0; i < kLanguageCount; ++i)
    {
        const core::Rect row = rowRect(i);
        const bool isCurrent = i == current;
        const bool isActive = open_ && i == highlighted_;

        if (isActive || isCurrent)
        {
            context.draw().addRect({row,
                                    render::Corners::uniform(kRowRadius),
                                    core::Color::fromHex(0xFFFFFF, isActive ? 0.11F : 0.06F)});
        }

        const core::Rect rowFlag{
            row.x + 10.0F, row.y + (row.height - kRowFlagHeight) * 0.5F, kRowFlagWidth, kRowFlagHeight};
        context.draw().addFlag(
            {rowFlag, render::Corners::uniform(kFlagRadius), flagFor(i), isActive ? 1.08F : 1.0F});

        text::ShapedText label = names_[i];
        label.lineHeight = row.height;
        context.draw().addText(label,
                               {rowFlag.right() + 12.0F, row.y},
                               isCurrent || isActive ? theme.colors.textPrimary : theme.colors.textSecondary);

        if (isCurrent)
        {
            text::ShapedText tick = check_;
            tick.lineHeight = row.height;
            context.draw().addText(
                tick, {row.right() - 12.0F - check_.width, row.y}, theme.colors.textPrimary);
        }
    }
    context.draw().popTransform();
}

}
