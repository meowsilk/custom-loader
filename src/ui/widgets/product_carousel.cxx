#include "ui/widgets/product_carousel.hxx"

#include "ui/icons.hxx"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <numeric>

namespace loader::ui
{
namespace
{

constexpr float kAspect = 2.0F / 3.0F;
constexpr float kMaxFrontHeight = 430.0F;
constexpr float kMinFrontHeight = 160.0F;
constexpr float kDotsArea = 44.0F;
constexpr float kBackScale = 0.62F;
constexpr float kStepAngle = 2.0F * std::numbers::pi_v<float> / 5.0F;
constexpr float kSpread = 0.9F;
constexpr float kSideDim = 0.38F;
constexpr float kCornerRadius = 20.0F;
constexpr float kInset = 18.0F;
constexpr float kGrow = 5.0F;
constexpr float kLift = 7.0F;
constexpr float kPressShrink = 0.02F;
constexpr float kArrowSize = 34.0F;
constexpr float kDotSize = 6.0F;
constexpr float kDotStretch = 14.0F;
constexpr float kDotSlot = 22.0F;
constexpr float kMinTitleSize = 13.0F;
constexpr float kSpringDamping = 0.86F;
constexpr float kSpringFrequency = 12.0F;
constexpr float kHoverIn = 0.16F;
constexpr float kBackDrop = 0.04F;
constexpr float kMinHitOpacity = 0.3F;
constexpr float kMinPaintOpacity = 0.01F;
constexpr float kFadeStart = 0.12F;
constexpr float kFadeEnd = 0.5F;
constexpr float kMaxDim = 0.6F;
constexpr float kFrontStart = 0.55F;
constexpr float kDotsOffset = 14.0F;
constexpr float kArrowBleed = 4.0F;
constexpr float kArrowRise = 4.0F;
constexpr float kArrowTint = 0.38F;
constexpr float kShadowOffset = 12.0F;
constexpr float kShadowBlur = 16.0F;
constexpr float kShadowHoverGain = 10.0F;
constexpr float kShadowAlpha = 0.40F;
constexpr float kShadowAlphaGain = 0.18F;
constexpr float kScrim = 0.9F;
constexpr float kHoverUnDim = 0.55F;
constexpr float kIdleRimAlpha = 0.14F;
constexpr float kHoverRimAlpha = 0.46F;
constexpr float kTitleHoverLift = 2.0F;
constexpr float kDotIdleAlpha = 0.34F;
constexpr float kDotNearAlpha = 0.58F;
constexpr float kDotHoverAlpha = 0.2F;

float smoothstep(float low, float high, float value)
{
    const float t = std::clamp((value - low) / (high - low), 0.0F, 1.0F);
    return t * t * (3.0F - 2.0F * t);
}

float wrappedDistance(float raw, float count)
{
    return raw - count * std::floor(raw / count + 0.5F);
}

}

ProductCarousel::ProductCarousel(UiContext& context) : Widget(context)
{
    setFocusable(true);
    arrow_ = text::shapeText(
        context_.fonts(), context_.theme().type.icon, std::u32string(1, icons::kArrowUpRight));
}

void ProductCarousel::addProduct(std::u32string title, render::ArtImage image)
{
    Item item;
    item.title = std::move(title);
    item.image = image;
    item.shaped = text::shapeText(context_.fonts(), context_.theme().type.cardTitle, item.title);
    items_.push_back(std::move(item));
    invalidate();
}

float ProductCarousel::frontHeight() const
{
    return std::clamp(bounds().height - kDotsArea, kMinFrontHeight, kMaxFrontHeight);
}

float ProductCarousel::offsetOf(std::size_t index) const
{
    return wrappedDistance(static_cast<float>(index) - position_.value(), static_cast<float>(items_.size()));
}

ProductCarousel::Placement ProductCarousel::placementFor(std::size_t index) const
{
    const core::Rect area = bounds();
    const float height = frontHeight();
    const float width = height * kAspect;

    const float angle =
        std::clamp(offsetOf(index) * kStepAngle, -std::numbers::pi_v<float>, std::numbers::pi_v<float>);
    const float depth = std::cos(angle);
    const float lateral = std::sin(angle);
    const float nearness = (depth + 1.0F) * 0.5F;

    const float scale = kBackScale + (1.0F - kBackScale) * nearness;
    const float cardWidth = width * scale;
    const float cardHeight = height * scale;
    const float centerX = area.center().x + lateral * width * kSpread;
    const float centerY = area.y + height * 0.5F + (1.0F - nearness) * height * kBackDrop;

    Placement placement;
    placement.rect = {centerX - cardWidth * 0.5F, centerY - cardHeight * 0.5F, cardWidth, cardHeight};
    placement.depth = depth;
    placement.opacity = smoothstep(kFadeStart, kFadeEnd, nearness);
    placement.dim = std::clamp((1.0F - depth) * kSideDim, 0.0F, kMaxDim);
    return placement;
}

core::Rect ProductCarousel::cardRect(std::size_t index) const
{
    return placementFor(index).rect;
}

std::optional<std::size_t> ProductCarousel::cardAt(core::Vec2 position) const
{
    std::optional<std::size_t> best;
    float bestDepth = -2.0F;
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Placement placement = placementFor(i);
        if (placement.opacity < kMinHitOpacity || !placement.rect.contains(position))
            continue;
        if (placement.depth > bestDepth)
        {
            bestDepth = placement.depth;
            best = i;
        }
    }
    return best;
}

core::Rect ProductCarousel::dotSlot(std::size_t index) const
{
    const core::Rect area = bounds();
    const float total = kDotSlot * static_cast<float>(items_.size());
    return {area.center().x - total * 0.5F + kDotSlot * static_cast<float>(index),
            area.y + frontHeight() + kDotsOffset,
            kDotSlot,
            kDotSlot};
}

std::optional<std::size_t> ProductCarousel::dotAt(core::Vec2 position) const
{
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (dotSlot(i).contains(position))
            return i;
    }
    return std::nullopt;
}

void ProductCarousel::select(std::size_t index)
{
    if (items_.empty())
        return;
    index %= items_.size();

    const float delta = wrappedDistance(static_cast<float>(index) - static_cast<float>(selected_),
                                        static_cast<float>(items_.size()));
    selected_ = index;
    position_.animateTo(position_.target() + delta, kSpringDamping, kSpringFrequency);
    invalidate();
}

void ProductCarousel::jumpTo(std::size_t index)
{
    if (items_.empty())
        return;
    selected_ = index % items_.size();
    position_.set(static_cast<float>(selected_));
    invalidate();
}

void ProductCarousel::rotate(int steps)
{
    if (items_.empty())
        return;
    const auto count = static_cast<int>(items_.size());
    const int next = ((static_cast<int>(selected_) + steps) % count + count) % count;
    select(static_cast<std::size_t>(next));
}

void ProductCarousel::activate()
{
    if (onActivate_ && !items_.empty())
        onActivate_(selected_);
}

void ProductCarousel::trackPointer(core::Vec2 position)
{
    hovered_ = cardAt(position);
    hoveredDot_ = hovered_ ? std::nullopt : dotAt(position);

    const theme::Motion& motion = context_.theme().motion;
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const bool over = hovered_ == i;
        items_[i].hover.animateTo(over ? 1.0F : 0.0F, over ? kHoverIn : motion.base);
    }
    invalidate();
}

bool ProductCarousel::onPointerDown(core::Vec2 position, core::input::PointerButton button)
{
    if (button != core::input::PointerButton::Left)
        return false;

    pressedCard_ = cardAt(position);
    pressedDot_ = pressedCard_ ? std::nullopt : dotAt(position);
    if (pressedCard_)
        items_[*pressedCard_].press.animateTo(1.0F, context_.theme().motion.fast);
    return true;
}

void ProductCarousel::onPointerMove(core::Vec2 position)
{
    trackPointer(position);
}

void ProductCarousel::onHoverMove(core::Vec2 position)
{
    trackPointer(position);
}

void ProductCarousel::onPointerUp(core::Vec2 position, core::input::PointerButton button)
{
    if (button != core::input::PointerButton::Left)
        return;

    const std::optional<std::size_t> card = cardAt(position);
    const std::optional<std::size_t> dot = card ? std::nullopt : dotAt(position);

    if (card && card == pressedCard_)
    {
        if (*card == selected_)
            activate();
        else
            select(*card);
    }
    else if (dot && dot == pressedDot_)
    {
        select(*dot);
    }
}

void ProductCarousel::onScroll(float notches)
{
    scrollRemainder_ += notches;
    while (scrollRemainder_ >= 1.0F)
    {
        rotate(-1);
        scrollRemainder_ -= 1.0F;
    }
    while (scrollRemainder_ <= -1.0F)
    {
        rotate(1);
        scrollRemainder_ += 1.0F;
    }
}

bool ProductCarousel::onKeyDown(const core::input::KeyEvent& event)
{
    switch (event.key)
    {
    case core::input::Key::Left:
        rotate(-1);
        return true;
    case core::input::Key::Right:
        rotate(1);
        return true;
    case core::input::Key::Enter:
    case core::input::Key::Space:
        activate();
        return true;
    default:
        return false;
    }
}

core::input::Cursor ProductCarousel::cursorKind() const
{
    return hovered_ || hoveredDot_ ? core::input::Cursor::Hand : core::input::Cursor::Arrow;
}

void ProductCarousel::onStateChanged()
{
    const theme::Motion& motion = context_.theme().motion;
    if (!hovered())
    {
        hovered_.reset();
        hoveredDot_.reset();
        for (Item& item : items_)
            item.hover.animateTo(0.0F, motion.base);
    }
    if (!pressed())
    {
        pressedCard_.reset();
        pressedDot_.reset();
        for (Item& item : items_)
            item.press.animateTo(0.0F, motion.fast);
    }
    focus_.animateTo(focusVisible() ? 1.0F : 0.0F, motion.fast);
}

void ProductCarousel::fitTitles()
{
    const float available = frontHeight() * kAspect - kInset * 2.0F;
    for (Item& item : items_)
    {
        text::TextStyle style = context_.theme().type.cardTitle;
        item.shaped = text::shapeText(context_.fonts(), style, item.title);
        if (item.shaped.width > available && available > 0.0F)
        {
            style.size = std::max(kMinTitleSize, std::floor(style.size * available / item.shaped.width));
            item.shaped = text::shapeText(context_.fonts(), style, item.title);
        }
    }
    fittedWidth_ = available;
}

void ProductCarousel::layoutChildren(core::Rect)
{
    if (frontHeight() * kAspect - kInset * 2.0F != fittedWidth_)
        fitTitles();
}

bool ProductCarousel::updateSelf(float deltaSeconds)
{
    bool animating = position_.update(deltaSeconds);
    animating = focus_.update(deltaSeconds) || animating;
    for (Item& item : items_)
    {
        animating = item.hover.update(deltaSeconds) || animating;
        animating = item.press.update(deltaSeconds) || animating;
    }
    return animating;
}

void ProductCarousel::paintSelf(PaintContext& context)
{
    std::vector<Placement> placements;
    placements.reserve(items_.size());
    for (std::size_t i = 0; i < items_.size(); ++i)
        placements.push_back(placementFor(i));

    std::vector<std::size_t> order(items_.size());
    std::ranges::iota(order, std::size_t{0});
    std::ranges::sort(
        order, [&](std::size_t a, std::size_t b) { return placements[a].depth < placements[b].depth; });

    for (const std::size_t index : order)
    {
        if (placements[index].opacity >= kMinPaintOpacity)
            paintCard(context, index, placements[index]);
    }

    paintDots(context);
}

void ProductCarousel::paintCard(PaintContext& context, std::size_t index, const Placement& placement)
{
    const theme::Theme& theme = context.theme();
    const Item& item = items_[index];
    const float hover = item.hover.value();
    const float press = item.press.value();
    const float frontness = smoothstep(kFrontStart, 1.0F, placement.depth);
    const float focus = index == selected_ ? focus_.value() : 0.0F;

    const float grow = kGrow * hover - placement.rect.width * kPressShrink * press;
    const core::Rect box{placement.rect.x - grow,
                         placement.rect.y - grow - kLift * hover,
                         placement.rect.width + grow * 2.0F,
                         placement.rect.height + grow * 2.0F};
    const float radius = kCornerRadius * (box.width / (frontHeight() * kAspect));
    const render::Corners corners = render::Corners::uniform(radius);

    context.draw().pushTransform({}, placement.opacity);

    context.useBack();
    context.draw().addShadow({box,
                              radius,
                              {0.0F, kShadowOffset + kShadowHoverGain * hover},
                              kShadowBlur + kShadowHoverGain * hover,
                              core::Color::fromHex(0x000000, kShadowAlpha + kShadowAlphaGain * hover)});
    context.draw().addArt(
        {box, corners, item.image, hover, placement.dim * (1.0F - kHoverUnDim * hover), kScrim});

    const core::Color idleRim = core::Color::fromHex(0xFFFFFF, kIdleRimAlpha);
    const core::Color hoverRim = core::Color::fromHex(0xFFFFFF, kHoverRimAlpha);
    const core::Color rim =
        core::Color::lerp(core::Color::lerp(idleRim, hoverRim, hover), theme.colors.glassRimFocus, focus);
    context.draw().addRect({box, corners, core::Color::transparent(), rim, 1.0F});

    if (frontness > 0.02F)
    {
        paintArrow(context, box, hover, frontness);

        context.useFront();
        text::ShapedText title = item.shaped;
        title.lineHeight = theme.type.cardTitle.lineHeight;
        const float titleY = box.bottom() - kInset - title.lineHeight - kTitleHoverLift * hover;
        context.draw().addTextWithShadow(
            title, {box.x + kInset, titleY}, theme.colors.textPrimary.scaledAlpha(frontness));
    }

    context.draw().popTransform();
}

void ProductCarousel::paintArrow(PaintContext& context,
                                 const core::Rect& box,
                                 float hover,
                                 float frontness) const
{
    const float alpha = hover * frontness;
    if (alpha <= 0.01F)
        return;

    const theme::Theme& theme = context.theme();
    const core::Rect arrow{box.right() - kInset + kArrowBleed - kArrowSize,
                           box.y + kInset - kArrowBleed,
                           kArrowSize,
                           kArrowSize};

    context.draw().pushTransform({0.0F, kArrowRise * (1.0F - hover)}, alpha);
    context.useGlass();
    context.draw().addGlass({arrow,
                             render::Corners::uniform(theme.radii.control - 2.0F),
                             core::Color::fromHex(0x000000, kArrowTint),
                             theme.colors.glassRim,
                             theme.glass.bevel,
                             theme.glass.refraction,
                             theme.glass.dispersion,
                             theme.glass.specular,
                             theme.glass.frost});
    context.useFront();
    text::ShapedText glyph = arrow_;
    glyph.lineHeight = arrow.height;
    context.draw().addText(
        glyph, {arrow.x + (arrow.width - arrow_.width) * 0.5F, arrow.y}, theme.colors.textPrimary);
    context.draw().popTransform();
}

void ProductCarousel::paintDots(PaintContext& context) const
{
    context.useFront();
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const core::Rect slot = dotSlot(i);
        const float closeness = std::clamp(1.0F - std::fabs(offsetOf(i)), 0.0F, 1.0F);
        const float width = kDotSize + kDotStretch * closeness;
        const core::Rect dot{
            slot.center().x - width * 0.5F, slot.center().y - kDotSize * 0.5F, width, kDotSize};
        const float alpha =
            kDotIdleAlpha + kDotNearAlpha * closeness + (hoveredDot_ == i ? kDotHoverAlpha : 0.0F);
        context.draw().addRect({dot,
                                render::Corners::uniform(kDotSize * 0.5F),
                                core::Color::fromHex(0xFFFFFF, std::min(alpha, 1.0F))});
    }
}

}
