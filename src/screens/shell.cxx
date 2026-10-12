#include "screens/shell.hxx"

#include "ui/animation/easing.hxx"
#include "ui/icons.hxx"
#include "ui/widgets/icon_button.hxx"
#include "ui/widgets/label.hxx"
#include "ui/widgets/stack.hxx"

#include <algorithm>

namespace loader::screens
{
namespace
{

constexpr float kSlideSeconds = 0.26F;
constexpr float kReducedSeconds = 0.24F;
constexpr float kReducedDamping = 1.0F;
constexpr float kReducedFrequency = 17.0F;
constexpr float kMorphDamping = 0.82F;
constexpr float kMorphFrequency = 10.5F;
constexpr float kSlideDamping = 0.9F;
constexpr float kSlideFrequency = 14.0F;
constexpr float kReducedHeaderSwap = 0.4F;
constexpr float kReducedHeaderFade = 0.6F;
constexpr float kSlideHeaderSwap = 0.3F;
constexpr float kSlideOutShare = 0.5F;
constexpr float kSlideInStart = 0.25F;
constexpr float kSlideInShare = 0.75F;
constexpr float kSlideDistance = 16.0F;
constexpr float kStaggerStart = 0.18F;
constexpr float kStaggerStep = 0.04F;
constexpr float kStaggerFade = 0.20F;
constexpr float kStaggerRise = 8.0F;
constexpr float kMorphFadeOut = 0.12F;
constexpr float kHeaderFadeIn = 0.18F;
const core::Insets kTitleHeaderPadding{12.0F, 6.0F, 6.0F, 6.0F};
const core::Insets kCustomHeaderPadding{10.0F, 6.0F, 10.0F, 6.0F};

float progress(float elapsed, float start, float length)
{
    return std::clamp((elapsed - start) / length, 0.0F, 1.0F);
}

}

Shell::Shell(ui::UiContext& context, WindowControls& controls) : Widget(context)
{
    const ui::theme::Theme& theme = context.theme();

    panel_ = &add<ui::PanelSurface>();
    panel_->style().axis = ui::layout::Axis::Column;
    panel_->style().padding = core::Insets::uniform(theme.metrics.headerInset);
    panel_->style().gap = theme.metrics.headerInset;

    header_ = &panel_->add<ui::Stack>(
        ui::layout::Axis::Row, theme.space.xs, kTitleHeaderPadding, ui::layout::Align::Center);
    header_->style().height = theme.metrics.headerHeight;

    headerSlot_ =
        &header_->add<ui::Stack>(ui::layout::Axis::Row, 0.0F, core::Insets{}, ui::layout::Align::Center);
    headerSlot_->style().flexGrow = 1.0F;
    title_ = &headerSlot_->add<ui::Label>(U"loader.gg", theme.type.headerTitle, ui::TextTone::Primary);
    title_->style().flexGrow = 1.0F;

    auto& minimize = header_->add<ui::IconButton>(ui::icons::kMinimize, ui::IconButton::Tone::Neutral);
    minimize.setOnClick([&controls] { controls.minimizeWindow(); });
    minimize_ = &minimize;
    auto& close = header_->add<ui::IconButton>(ui::icons::kClose, ui::IconButton::Tone::Danger);
    close.setOnClick([&controls] { controls.closeWindow(); });
    close_ = &close;

    contentHost_ = &panel_->add<ui::OverlayStack>();
    contentHost_->style().flexGrow = 1.0F;

    languageSelector_ = &add<ui::LanguageSelector>();
}

void Shell::setBackdropAvailable(bool available)
{
    panel_->setBackdropAvailable(available);
}

void Shell::applyHeader(std::unique_ptr<ui::Widget> header)
{
    const ui::theme::Theme& theme = context_.theme();

    if (headerContent_ != nullptr)
    {
        retired_.push_back(headerSlot_->detach(*headerContent_));
        headerContent_ = nullptr;
    }

    const bool custom = header != nullptr;
    title_->setVisible(!custom);
    minimize_->setVisible(!custom);
    close_->setVisible(!custom);
    header_->style().height = custom ? Shell::kCustomHeaderHeight : theme.metrics.headerHeight;
    header_->style().padding = custom ? kCustomHeaderPadding : kTitleHeaderPadding;
    if (custom)
    {
        header->style().flexGrow = 1.0F;
        headerContent_ = &headerSlot_->attach(std::move(header));
    }
    invalidate();
}

core::Size Shell::hostSizeFor(core::Size panel, bool customHeader) const
{
    const ui::theme::Theme& theme = context_.theme();
    const float inset = theme.metrics.headerInset;
    const float header = customHeader ? Shell::kCustomHeaderHeight : theme.metrics.headerHeight;
    return {panel.width - inset * 2.0F, panel.height - inset * 3.0F - header};
}

void Shell::setMaxPanelSize(core::Size size)
{
    if (size == maxPanel_)
        return;
    maxPanel_ = size;
    if (requestedPanel_.width > 0.0F)
        resizePanel(requestedPanel_, false, false);
    invalidate();
}

void Shell::resizePanel(core::Size size, bool animate, bool morph)
{
    requestedPanel_ = size;
    size = {std::min(size.width, maxPanel_.width), std::min(size.height, maxPanel_.height)};

    if (!animate || !sized_)
    {
        width_.set(size.width);
        height_.set(size.height);
        sized_ = true;
        return;
    }

    const float damping = reducedMotion_ ? kReducedDamping : (morph ? kMorphDamping : kSlideDamping);
    const float frequency = reducedMotion_ ? kReducedFrequency : (morph ? kMorphFrequency : kSlideFrequency);
    width_.animateTo(size.width, damping, frequency);
    height_.animateTo(size.height, damping, frequency);
}

void Shell::showInitial(std::unique_ptr<ui::Widget> content,
                        core::Size panelSize,
                        std::unique_ptr<ui::Widget> header)
{
    applyHeader(std::move(header));
    resizePanel(panelSize, false, false);
    contentHost_->attach(std::move(content));
    invalidate();
}

void Shell::transitionTo(std::unique_ptr<ui::Widget> content,
                         core::Size panelSize,
                         TransitionKind kind,
                         std::unique_ptr<ui::Widget> header)
{
    if (transition_.active)
        finishTransition();

    ui::Widget* outgoing = contentHost_->children().empty() ? nullptr : contentHost_->children().back().get();
    if (outgoing != nullptr)
        contentHost_->pinSize(*outgoing, {outgoing->bounds().width, outgoing->bounds().height});
    ui::Widget& incoming = contentHost_->attach(std::move(content));

    resizePanel(panelSize, true, kind == TransitionKind::Morph);

    const core::Size finalPanel{std::min(panelSize.width, maxPanel_.width),
                                std::min(panelSize.height, maxPanel_.height)};
    contentHost_->pinSize(incoming, hostSizeFor(finalPanel, header != nullptr));

    transition_ = {};
    transition_.headerChange = header != nullptr || headerContent_ != nullptr;
    transition_.header = std::move(header);
    transition_.active = true;
    transition_.kind = kind;
    transition_.outgoing = outgoing;
    transition_.incoming = &incoming;
    transition_.duration =
        reducedMotion_ ? kReducedSeconds
                       : (kind == TransitionKind::Morph ? context_.theme().motion.morph : kSlideSeconds);
    applyTransitionFrame();
    invalidate();
}

void Shell::applyTransitionFrame()
{
    if (!transition_.active)
        return;

    const float t = transition_.elapsed;
    const bool reduced = reducedMotion_;

    if (transition_.headerChange)
    {
        const float swapAt =
            reduced ? transition_.duration * kReducedHeaderSwap
                    : (transition_.kind == TransitionKind::Morph ? kMorphFadeOut
                                                                 : transition_.duration * kSlideHeaderSwap);
        const float fadeIn = reduced ? transition_.duration * kReducedHeaderFade : kHeaderFadeIn;
        if (!transition_.headerApplied && t >= swapAt)
        {
            applyHeader(std::move(transition_.header));
            transition_.headerApplied = true;
        }

        const float opacity =
            transition_.headerApplied ? progress(t, swapAt, fadeIn) : 1.0F - progress(t, 0.0F, swapAt);
        headerSlot_->setPresentation({}, opacity);
    }

    if (reduced)
    {
        const float mix = progress(t, 0.0F, transition_.duration);
        if (transition_.outgoing != nullptr)
            transition_.outgoing->setPresentation({}, 1.0F - mix);
        transition_.incoming->setPresentation({}, mix);
        return;
    }

    if (transition_.kind == TransitionKind::Morph)
    {
        if (transition_.outgoing != nullptr)
            transition_.outgoing->setPresentation({}, 1.0F - progress(t, 0.0F, kMorphFadeOut));

        transition_.incoming->setPresentation({}, 1.0F);
        int index = 0;
        for (const auto& child : transition_.incoming->children())
        {
            const float local = ui::animation::easing::enter(
                progress(t, kStaggerStart + kStaggerStep * static_cast<float>(index), kStaggerFade));
            child->setPresentation({0.0F, kStaggerRise * (1.0F - local)}, local);
            ++index;
        }
        return;
    }

    const float direction = transition_.kind == TransitionKind::SlideForward ? 1.0F : -1.0F;
    const float out = ui::animation::easing::exit(progress(t, 0.0F, transition_.duration * kSlideOutShare));
    const float in = ui::animation::easing::enter(
        progress(t, transition_.duration * kSlideInStart, transition_.duration * kSlideInShare));
    if (transition_.outgoing != nullptr)
        transition_.outgoing->setPresentation({-direction * kSlideDistance * out, 0.0F}, 1.0F - out);
    transition_.incoming->setPresentation({direction * kSlideDistance * (1.0F - in), 0.0F}, in);
}

void Shell::finishTransition()
{
    if (!transition_.active)
        return;

    contentHost_->unpin();
    if (transition_.outgoing != nullptr)
        contentHost_->detach(*transition_.outgoing);
    if (transition_.headerChange && !transition_.headerApplied)
        applyHeader(std::move(transition_.header));
    headerSlot_->setPresentation({}, 1.0F);

    transition_.incoming->setPresentation({}, 1.0F);
    for (const auto& child : transition_.incoming->children())
        child->setPresentation({}, 1.0F);
    transition_ = {};
    invalidate();
}

void Shell::updateTransition(float deltaSeconds)
{
    if (!transition_.active)
        return;

    transition_.elapsed += deltaSeconds;
    if (transition_.elapsed >= transition_.duration && !width_.animating() && !height_.animating())
    {
        finishTransition();
        return;
    }
    applyTransitionFrame();
}

bool Shell::updateSelf(float deltaSeconds)
{
    retired_.clear();
    bool animating = width_.update(deltaSeconds);
    animating = height_.update(deltaSeconds) || animating;

    updateTransition(deltaSeconds);
    return animating || transition_.active;
}

void Shell::layoutChildren(core::Rect bounds)
{
    const core::Vec2 center = bounds.center();
    const float width = width_.value();
    const float height = height_.value();
    const core::Rect panel{center.x - width * 0.5F, center.y - height * 0.5F, width, height};
    panel_->layout(panel);
    languageSelector_->layout(panel);
}

}
